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

#ifndef GUIDRIVEROGRE_TESTAPP_TESTOGREPLATFORM_H
#define GUIDRIVEROGRE_TESTAPP_TESTOGREPLATFORM_H

/**
 *  The tests of what the window backend Ogre does per platform, without the platform: the choice of the platform
 *  backend, the parameters of the render window of every platform, how a render window follows a resize, the pacing,
 *  and the state of the native surface over its loss and return (Android)
 */
void PerformOgrePlatformTests( void );

#endif /* GUIDRIVEROGRE_TESTAPP_TESTOGREPLATFORM_H ? */
