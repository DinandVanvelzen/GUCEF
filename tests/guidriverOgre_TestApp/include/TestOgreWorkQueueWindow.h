/*
 *  guidriverOgre_TestApp: Test application for the window backend Ogre (guidriverOgre)
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef GUIDRIVEROGRE_TESTAPP_TESTOGREWORKQUEUEWINDOW_H
#define GUIDRIVEROGRE_TESTAPP_TESTOGREWORKQUEUEWINDOW_H

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

namespace GUCEF { namespace GUIDRIVEROGRE { class COgreWindowContext; } }

/** The threads of the process whose name starts with the given text (Linux: /proc/self/task/<id>/comm), 0 elsewhere */
GUCEF::UInt32 CountProcessThreadsNamed( const char* namePrefix );

/**
 *  The tests of Ogre's work queue on the GUCEF thread pool with a render window: the start with the first window,
 *  main thread tasks in the frames of the window, a texture loaded in the background through the future bridge, and
 *  the registration of the worker threads with the render system
 */
void PerformOgreWorkQueueWindowTests( GUCEF::GUIDRIVEROGRE::COgreWindowContext* window );

#endif /* GUIDRIVEROGRE_TESTAPP_TESTOGREWORKQUEUEWINDOW_H ? */
