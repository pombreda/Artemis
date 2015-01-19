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

#include <QElapsedTimer>
#include <QCoreApplication>
#include <unistd.h>

#include "delayutil.h"

namespace artemis
{

void delay(unsigned milliseconds)
{
    // Adapted from qWait in qtestsystem.h

    QElapsedTimer timer;
    timer.start();
    do {
        QCoreApplication::processEvents(QEventLoop::AllEvents, milliseconds);
        usleep(10000); // 10ms
    } while (timer.elapsed() < milliseconds);
}


}