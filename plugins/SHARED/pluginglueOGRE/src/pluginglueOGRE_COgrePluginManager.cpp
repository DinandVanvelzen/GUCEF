/*
 *  pluginglueOGRE: Support library of the Ogre 1.x plugin set: Ogre Root setup, log, plugin loading
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

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREPLUGINADAPTER_H
#include "pluginglueOGRE_COgrePluginAdapter.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREPLUGINADAPTER_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREPLUGINADAPTER_H ? */

#include "pluginglueOGRE_COgrePluginManager.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef GUCEF::map< CORE::CString, Ogre::Plugin* >  TStaticPluginMap;

static MT::CMutex& StaticPluginLock( void )
{
    static MT::CMutex lock;
    return lock;
}

static TStaticPluginMap& StaticPlugins( void )
{
    static TStaticPluginMap plugins;
    return plugins;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgrePluginManager::COgrePluginManager( void )
    : CORE::CPluginManager()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COgrePluginManager::~COgrePluginManager()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
COgrePluginManager::GetPluginType( void ) const
{GUCEF_TRACE;

    return "OgrePlugin";
}

/*-------------------------------------------------------------------------*/

CORE::TPluginPtr
COgrePluginManager::RegisterPlugin( void* modulePtr                         ,
                                    CORE::TPluginMetaDataPtr pluginMetaData )
{GUCEF_TRACE;

    COgrePluginAdapter* plugin = GUCEF_NEW COgrePluginAdapter();
    if ( plugin->Link( modulePtr, pluginMetaData ) )
    {
        return CORE::TPluginPtr( plugin );
    }
    GUCEF_DELETE plugin;
    return CORE::TPluginPtr();
}

/*-------------------------------------------------------------------------*/

bool
COgrePluginManager::UnregisterPlugin( CORE::TPluginPtr plugin )
{GUCEF_TRACE;

    if ( plugin.IsNULL() )
        return true;

    TOgrePluginAdapterPtr ogrePlugin = plugin.StaticCast< COgrePluginAdapter >();
    if ( !ogrePlugin.IsNULL() )
        return ogrePlugin->Unlink();
    return false;
}

/*-------------------------------------------------------------------------*/

bool
COgrePluginManager::RegisterStaticOgrePlugin( const CORE::CString& pluginName, Ogre::Plugin* plugin )
{GUCEF_TRACE;

    if ( pluginName.IsNULLOrEmpty() || GUCEF_NULL == plugin )
        return false;

    MT::CScopeMutex lock( StaticPluginLock() );
    return StaticPlugins().insert( TStaticPluginMap::value_type( pluginName, plugin ) ).second;
}

/*-------------------------------------------------------------------------*/

bool
COgrePluginManager::UnregisterStaticOgrePlugin( const CORE::CString& pluginName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( StaticPluginLock() );
    return 0 != StaticPlugins().erase( pluginName );
}

/*-------------------------------------------------------------------------*/

Ogre::Plugin*
COgrePluginManager::GetStaticOgrePlugin( const CORE::CString& pluginName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( StaticPluginLock() );
    TStaticPluginMap::const_iterator i = StaticPlugins().find( pluginName );
    return i == StaticPlugins().end() ? GUCEF_NULL : (*i).second;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
