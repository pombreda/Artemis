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

#include "easilyboredsearch.h"

namespace artemis
{


EasilyBoredSearch::EasilyBoredSearch(TraceNodePtr tree) :
    RandomAccessSearch(tree)
{
}

QPair<bool, RandomAccessSearch::ExplorationDescriptor> EasilyBoredSearch::nextTarget(QList<RandomAccessSearch::ExplorationDescriptor> possibleTargets)
{
    // If we have run out of attempts, then return false.
    if (possibleTargets.empty()) {
        return QPair<bool, ExplorationDescriptor>(false, ExplorationDescriptor());
    }

    // TODO: Choose which node from the list to return.
    return QPair<bool, RandomAccessSearch::ExplorationDescriptor>(true, possibleTargets.at(0));
}


void EasilyBoredSearch::newTraceAdded(TraceNodePtr parent, int direction, TraceNodePtr suffix)
{
    // TODO: Process the new trace suffix, if required.
}


} //namespace artemis
