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

#include "traceunexploredqueued.h"

namespace artemis {

QSharedPointer<TraceUnexploredQueued>* TraceUnexploredQueued::mInstance = new QSharedPointer<TraceUnexploredQueued>(new TraceUnexploredQueued());

QSharedPointer<TraceUnexploredQueued> TraceUnexploredQueued::getInstance()
{
    return *TraceUnexploredQueued::mInstance;
}

void TraceUnexploredQueued::accept(TraceVisitor* visitor)
{
    visitor->visit(this);
}

bool TraceUnexploredQueued::isEqualShallow(const QSharedPointer<const TraceNode>& other)
{
    return !other.dynamicCast<const TraceUnexploredQueued>().isNull();
}

}