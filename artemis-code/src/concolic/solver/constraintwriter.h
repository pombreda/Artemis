/*
 * Copyright 2012 Aarhus University
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CONSTRAINTWRITER_H
#define CONSTRAINTWRITER_H

#include <fstream>
#include <string>
#include <map>

#include "JavaScriptCore/symbolic/expr.h"
#include "JavaScriptCore/symbolic/expression/visitor.h"

#ifdef ARTEMIS

namespace artemis
{

enum Type {
    INT, STRING, BOOL, TYPEERROR
};

/**
 * Visitor generating symbolic constraints for the
 * SMT solver Kaluza[1].
 *
 * The visitor takes a file path as an argument in
 * its construct. All constraints will be written to
 * this file.
 *
 * The commit() function should called after applying
 * the visitor to all constraints in a path condition.
 * Don't use the constraint file before calling commit().
 *
 * If commit() returns false then the constraints can
 * not be solved by Kaluza. In this case the content
 * of the constraint file is undefined and the result
 * should be interpreted as unsat.
 *
 * [1] http://webblaze.cs.berkeley.edu/2010/kaluza/
 */
class ConstraintWriter : public Symbolic::Visitor
{
public:
    ConstraintWriter(std::string output_filename);

    void visit(Symbolic::SymbolicInteger* symbolicinteger);
    void visit(Symbolic::ConstantInteger* constantinteger);
    void visit(Symbolic::IntegerBinaryOperation* integerbinaryoperation);
    void visit(Symbolic::IntegerCoercion* integercoercion);
    void visit(Symbolic::SymbolicString* symbolicstring);
    void visit(Symbolic::ConstantString* constantstring);
    void visit(Symbolic::StringBinaryOperation* stringbinaryoperation);
    void visit(Symbolic::StringCoercion* stringcoercion);
    void visit(Symbolic::StringRegexReplace* stringregexreplace);
    void visit(Symbolic::StringReplace* stringreplace);
    void visit(Symbolic::SymbolicBoolean* symbolicboolean);
    void visit(Symbolic::ConstantBoolean* constantboolean);
    void visit(Symbolic::BooleanCoercion* booleancoercion);
    void visit(Symbolic::BooleanBinaryOperation* booleanbinaryoperation);

    bool commit();

private:

    /**
     * Kaluza does not support mixing constraints on strings,
     * bools and integers. Thus, we allow type coercions but
     * we only support one type of constraint to be applied
     * to any symbol.
     *
     * E.g. an input string can be coerced into an int, and
     * we can apply as many integer constraints to it as we
     * want.
     */
    void recordType(const std::string& identifer, Type type);

    std::map<std::string, Type> mTypemap;
    std::ofstream mOutput;
    std::string mIdentifierStore;
    unsigned int mNextTemporaryIdentifier;

    bool mError; // indicates that an error occured when writing the file
    std::string mErrorReason;
};

}

#endif
#endif // CONSTRAINTWRITER_H
