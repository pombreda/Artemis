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

 // AUTO GENERATED - DO NOT MODIFY

#ifndef SYMBOLIC_STRINGREGEXREPLACE_H
#define SYMBOLIC_STRINGREGEXREPLACE_H

#include <string>

#include "JavaScriptCore/wtf/ExportMacros.h"
#include "JavaScriptCore/runtime/UString.h"

#include "visitor.h"
#include "stringexpression.h"

#ifdef ARTEMIS

namespace Symbolic
{

class StringRegexReplace : public StringExpression
{
public:
    explicit StringRegexReplace(StringExpression* source, JSC::UString regexpattern, JSC::UString replace);
    void accept(Visitor* visitor);

	inline StringExpression* getSource() {
		return m_source;
	}
	inline JSC::UString getRegexpattern() {
		return m_regexpattern;
	}
	inline JSC::UString getReplace() {
		return m_replace;
	}

private:
	StringExpression* m_source;
	JSC::UString m_regexpattern;
	JSC::UString m_replace;

};
}

#endif
#endif // SYMBOLIC_STRINGREGEXREPLACE_H