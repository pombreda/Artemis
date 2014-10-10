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

 // AUTO GENERATED - DO NOT MODIFY

#ifndef SYMBOLIC_SYMBOLICOBJECT_H
#define SYMBOLIC_SYMBOLICOBJECT_H

#include <string>

#include <list>

#include "visitor.h"
#include "objectexpression.h"

#ifdef ARTEMIS

namespace Symbolic
{

class SymbolicObject : public ObjectExpression
{
public:
    explicit SymbolicObject(int id);
    void accept(Visitor* visitor);
    void accept(Visitor* visitor, void* arg);

	inline int getId() {
		return m_id;
	}

private:
	int m_id;

};
}

#endif
#endif // SYMBOLIC_SYMBOLICOBJECT_H