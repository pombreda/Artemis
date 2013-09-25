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

#include "tracesymbolicbranch.h"

namespace artemis {

TraceSymbolicBranch::TraceSymbolicBranch(Symbolic::Expression* condition) :
    TraceBranch(),
    mCondition(condition)
{
    assert(condition != NULL);
}

void TraceSymbolicBranch::accept(TraceVisitor* visitor) {
    visitor->visit(this);
}

bool TraceSymbolicBranch::isEqualShallow(const QSharedPointer<const TraceNode>& other)
{
    QSharedPointer<const TraceSymbolicBranch> otherCasted = other.dynamicCast<const TraceSymbolicBranch>();

    if (otherCasted.isNull()) {
        return false;
    }

    // TODO, should we also check equality of the symbolic expression?

    return true;
}

}