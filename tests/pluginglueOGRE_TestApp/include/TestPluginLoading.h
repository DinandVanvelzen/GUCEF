/*
 *  pluginglueOGRE_TestApp: Test application for the Ogre glue modules (pluginglueOGRE, OgrePlugin_GucefVFS, OgreCodec_GucefIMAGE)
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

#ifndef GUCEF_PLUGINGLUE_OGRE_TESTAPP_TESTPLUGINLOADING_H
#define GUCEF_PLUGINGLUE_OGRE_TESTAPP_TESTPLUGINLOADING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Makes the global Ogre systems of the support library (log adapter, Root, plugin manager of the type OgrePlugin)
 *  and loads the Ogre plugins OgrePlugin_GucefVFS and OgreCodec_GucefIMAGE through GUCEF plugin control, as the
 *  bootstrap configuration of an application does. This is the environment of the archive and codec tests.
 *  Also tests the loading: the plugins end up installed in the Root and take part in the Ogre systems.
 */
void
PerformPluginLoadingTests( void );

/** Unloads the plugins and takes the global Ogre systems down, tests that the Ogre side is clean afterwards */
void
PerformPluginUnloadingTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_OGRE_TESTAPP_TESTPLUGINLOADING_H ? */
