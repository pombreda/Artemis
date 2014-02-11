/*
 * Copyright 2012 Aarhus University
 *
 * Licensed under the GNU General Public License, Version 3 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *          http://www.gnu.org/licenses/gpl-3.0.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <sstream>
#include <cstdlib>
#include <math.h>

#include <QDebug>
#include <QDateTime>

#include "util/loggingutil.h"
#include "statistics/statsstorage.h"

#include "cvc4regexcompiler.h"

#include "cvc4.h"

namespace artemis
{

CVC4ConstraintWriter::CVC4ConstraintWriter()
    : SMTConstraintWriter()
{

}

void CVC4ConstraintWriter::preVisitPathConditionsHook()
{
    mOutput << "(set-logic QF_S)" << std::endl;
    mOutput << "(set-option :produce-models true)" << std::endl;
    mOutput << "(set-option :strings-exp true)" << std::endl << std::endl;
    mOutput << "(define-fun str.contains ((?in String) (?s String)) Bool\n"
               "    (exists ((?a String) (?b String))\n"
               "        (= ?in (str.++ ?a ?s ?b)))\n"
               ")\n"
               "\n"
               "(define-fun str.replace ((?x String) (?y String) (?s String) (?r String)) Bool\n"
               "    (ite (str.contains ?x ?s)\n"
               "         (or\n"
               "             (and (= ?s ?r)\n"
               "                  (= ?x ?y))\n"
               "             (exists ((?a String) (?b String))\n"
               "                 (and  (= ?s (str.++ ?a ?x ?b))\n"
               "                 (= ?r (str.++ ?a ?y ?b)))))\n"
               "         (= ?s ?r))\n"
               ")" << std::endl << std::endl;
}

void CVC4ConstraintWriter::postVisitPathConditionsHook()
{
    mOutput << "\n";
    mOutput << "(check-sat)\n";
    mOutput << "(get-model)\n";
}

void CVC4ConstraintWriter::visit(Symbolic::SymbolicString* symbolicstring, void* args)
{
    // If we are coercing from an input (string) to an integer, then this is a special case.
    // Instead of returning a symbolic string (which would raise an error) we just silently ignore the coercion and record
    // the variable as an integer instead of a string.
    if (args != NULL) {

        CoercionPromise* promise = (CoercionPromise*)args;

        if (promise->coerceTo == Symbolic::INT) {
            promise->isCoerced = true;

            recordAndEmitType(symbolicstring->getSource(), Symbolic::INT);
            mExpressionBuffer = SMTConstraintWriter::encodeIdentifier(symbolicstring->getSource().getIdentifier());
            mExpressionType = Symbolic::INT;

            return;

        }
    }

    // Checks this symbolic value is of type STRING and raises an error otherwise.
    recordAndEmitType(symbolicstring->getSource(), Symbolic::STRING);

    mExpressionBuffer = SMTConstraintWriter::encodeIdentifier(symbolicstring->getSource().getIdentifier());
    mExpressionType = Symbolic::STRING;
}

void CVC4ConstraintWriter::visit(Symbolic::ConstantString* constantstring, void* args)
{
    std::ostringstream strs;

    strs << "\"" << *constantstring->getValue() << "\"";

    mExpressionBuffer = strs.str();
    mExpressionType = Symbolic::STRING;
}

void CVC4ConstraintWriter::visit(Symbolic::StringBinaryOperation* stringbinaryoperation, void* args)
{
    static const char* op[] = {
        "(str.++ ", "(= ", "(= (= ", "_", "_", "_", "_", "(= ", "(= (= "
    };

    static const char* opclose[] = {
        ")", ")", ") false)", "_", "_", "_", "_", ")", ") false)"
    };

    switch (stringbinaryoperation->getOp()) {
    case Symbolic::CONCAT:
    case Symbolic::STRING_EQ:
    case Symbolic::STRING_NEQ:
    case Symbolic::STRING_SEQ:
    case Symbolic::STRING_SNEQ:
        break; // these are supported
    default:
        error("Unsupported operation on strings");
        return;
    }

    stringbinaryoperation->getLhs()->accept(this);
    std::string lhs = mExpressionBuffer;
    if(!checkType(Symbolic::STRING)){
        error("String operation with incorrectly typed LHS");
        return;
    }

    stringbinaryoperation->getRhs()->accept(this);
    std::string rhs = mExpressionBuffer;
    if(!checkType(Symbolic::STRING)){
        error("String operation with incorrectly typed RHS");
        return;
    }

    std::ostringstream strs;
    strs << op[stringbinaryoperation->getOp()] << lhs << " " << rhs << opclose[stringbinaryoperation->getOp()];
    mExpressionBuffer = strs.str();
    mExpressionType = opGetType(stringbinaryoperation->getOp());
}

void CVC4ConstraintWriter::visit(Symbolic::StringCoercion* stringcoercion, void* args)
{
    CoercionPromise promise(Symbolic::STRING);
    stringcoercion->getExpression()->accept(this);

    if (!promise.isCoerced) {
        coercetype(mExpressionType, Symbolic::STRING, mExpressionBuffer); // Sets mExpressionBuffer and Type.
    }
}

void CVC4ConstraintWriter::visit(Symbolic::StringCharAt* stringcharat, void* arg)
{
    stringcharat->getSource()->accept(this);
    if(!checkType(Symbolic::STRING)){
        error("String char at operation on non-string");
        return;
    }

    // CVC4 requires the length of mExpressionBuffer to be at least getPosition+1.
    mOutput << "(assert (> (str.len " << mExpressionBuffer << ") " << stringcharat->getPosition() << "))" << std::endl;

    std::ostringstream strs;
    strs << "(str.at " << mExpressionBuffer << " " << stringcharat->getPosition() << ")";
    mExpressionBuffer = strs.str();
    mExpressionType = Symbolic::STRING;
}

void CVC4ConstraintWriter::visit(Symbolic::StringRegexReplace* regex, void* args)
{
    // special case input filtering (filters matching X and replacing with "")
    if (regex->getReplace()->compare("") == 0) {

        // right now, only support a very limited number of whitespace filters

        bool replaceSpaces = regex->getRegexpattern()->compare("/ /g") == 0 ||
                regex->getRegexpattern()->compare("/ /") == 0;
        bool replaceNewlines = regex->getRegexpattern()->compare("/\\n/g") == 0 ||
                regex->getRegexpattern()->compare("/\\r/") == 0 ||
                regex->getRegexpattern()->compare("/\\r\\n/") == 0;

        if (replaceSpaces || replaceNewlines || true) { // TODO: Hack, always filter away these for now

            regex->getSource()->accept(this, args); // send args through, allow local coercions

            // You could use the following block to prevent certain characters to be used,
            // but this would be problematic wrt. possible coercions, so we just ignore these filtering regexes.

            //if(!checkType(Symbolic::STRING)){
            //    error("String regex operation on non-string");
            //    return;
            //}
            //
            //if(replaceSpaces){
            //    mOutput << "(assert (= (Contains " << mExpressionBuffer << " \" \") false))\n";
            //    mConstriantLog << "(assert (= (Contains " << mExpressionBuffer << " \" \") false))\n";
            //}

            // In fact the solver currently cannot return results which contain newlines,
            // so we can completely ignore the case of replaceNewlines.

            // to be explicit, we just let the parent buffer flow down
            mExpressionBuffer = mExpressionBuffer;
            mExpressionType = mExpressionType;

            statistics()->accumulate("Concolic::Solver::RegexSuccessfullyTranslated", 1);

            return;
        }

    }


    statistics()->accumulate("Concolic::Solver::RegexNotTranslated", 1);
    error("Regex constraints not supported");
}

void CVC4ConstraintWriter::visit(Symbolic::StringReplace* replace, void* args)
{
    replace->getSource()->accept(this);
    if(!checkType(Symbolic::STRING)){
        error("String replace operation on non-string");
        return;
    }

    std::string temporary = emitAndReturnNewTemporary(Symbolic::STRING);

    mOutput << "(assert (str.replace " << "\"" << *replace->getPattern() << "\" \"" << *replace->getReplace() << "\" " << mExpressionBuffer << " " << temporary << "))";

    mExpressionBuffer = temporary;
    mExpressionType = Symbolic::STRING;
}

void CVC4ConstraintWriter::visit(Symbolic::StringRegexSubmatch* submatch, void* args)
{
    submatch->getSource()->accept(this);
    if(!checkType(Symbolic::STRING)){
        error("String char at operation on non-string");
        return;
    }

    std::string pre = this->emitAndReturnNewTemporary(Symbolic::STRING);
    std::string match = this->emitAndReturnNewTemporary(Symbolic::STRING);
    std::string post = this->emitAndReturnNewTemporary(Symbolic::STRING);

    mOutput << "(assert (= " << mExpressionBuffer << " (str.++ " << pre << " " << match << " " << post << ")))" << std::endl;

    std::ostringstream strs;

    try {
        strs << "(str.in.re " << match << " " << CVC4RegexCompiler::compile(*submatch->getRegexpattern()) << ")";
        mExpressionBuffer = strs.str();

    } catch (CVC4RegexCompilerException ex) {
        std::stringstream err;
        err << "The CVC4RegexCompiler failed when compiling the regex " << *submatch->getRegexpattern() << " with the error message: " << ex.what();
        error(err.str());
    }

    mExpressionType = Symbolic::BOOL;
}

void CVC4ConstraintWriter::visit(Symbolic::StringRegexSubmatchIndex* submatchIndex, void* args)
{
    submatchIndex->getSource()->accept(this);
    if(!checkType(Symbolic::STRING)){
        error("String submatch index operation on non-string");
        return;
    }

    /*
     * TODO this is currently not supported, or we don't know how to support it, in CVC4.
     */

    error("SUBMATCH INDEX NOT SUPPORTED");
}

void CVC4ConstraintWriter::visit(Symbolic::StringLength* stringlength, void* args)
{
    stringlength->getString()->accept(this);
    if(!checkType(Symbolic::STRING)){
        error("String length operation on non-string");
        return;
    }

    std::ostringstream strs;
    strs << "(str.len " << mExpressionBuffer << ")";
    mExpressionBuffer = strs.str();
    mExpressionType = Symbolic::INT;
}

}