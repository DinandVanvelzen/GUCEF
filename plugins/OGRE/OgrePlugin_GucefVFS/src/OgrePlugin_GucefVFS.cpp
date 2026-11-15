/*
 *  OgrePlugin_GucefVFS: Ogre plugin that registers the archive factories which read through gucefVFS
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreRoot.h"

#ifndef GUCEF_OGREPLUGIN_GUCEFVFS_CGUCEFVFSOGREPLUGIN_H
#include "OgrePlugin_GucefVFS_CGucefVfsOgrePlugin.h"
#define GUCEF_OGREPLUGIN_GUCEFVFS_CGUCEFVFSOGREPLUGIN_H
#endif /* GUCEF_OGREPLUGIN_GUCEFVFS_CGUCEFVFSOGREPLUGIN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The entry points of an Ogre plugin module, the way every Ogre plugin has them. They are not there in builds where
 *  the plugin is linked into the application (OGRE_STATIC_LIB): the application then makes a CGucefVfsOgrePlugin,
 *  gives it to COgrePluginManager::RegisterStaticOgrePlugin() of pluginglueOGRE and lets the bootstrap
 *  configuration install it.
 */
#ifndef OGRE_STATIC_LIB

static GUCEF::PLUGINGLUE::OGRE::CGucefVfsOgrePlugin* g_plugin = GUCEF_NULL;

extern "C" GUCEF_OGREPLUGIN_GUCEFVFS_EXPORT_C void
dllStartPlugin( void )
{GUCEF_TRACE;

    g_plugin = new GUCEF::PLUGINGLUE::OGRE::CGucefVfsOgrePlugin();
    Ogre::Root::getSingleton().installPlugin( g_plugin );
}

/*-------------------------------------------------------------------------*/

extern "C" GUCEF_OGREPLUGIN_GUCEFVFS_EXPORT_C void
dllStopPlugin( void )
{GUCEF_TRACE;

    Ogre::Root::getSingleton().uninstallPlugin( g_plugin );
    delete g_plugin;
    g_plugin = GUCEF_NULL;
}

#endif /* OGRE_STATIC_LIB ? */

/*-------------------------------------------------------------------------*/
