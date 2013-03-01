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
#ifndef TARGET_DESCRIPTOR_H
#define TARGET_DESCRIPTOR_H

#include <QObject>
#include <QWebElement>

#include "runtime/input/events/eventhandlerdescriptor.h"

#include "runtime/browser/artemiswebpage.h"

namespace artemis
{

class TargetDescriptor : public QObject
{
    Q_OBJECT

public:
    TargetDescriptor(QObject* parent, const EventHandlerDescriptor* eventHandler);

    virtual QWebElement get(ArtemisWebPage* page) const = 0;

protected:
    EventHandlerDescriptor* mEventHandler;
};

}

#endif
