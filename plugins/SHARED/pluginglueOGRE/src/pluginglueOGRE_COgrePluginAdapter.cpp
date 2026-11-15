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

#include <string.h>

#include "OgreRoot.h"
#include "OgrePlugin.h"

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREPLUGINMANAGER_H
#include "pluginglueOGRE_COgrePluginManager.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREPLUGINMANAGER_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREPLUGINMANAGER_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H
#include "pluginglueOGRE_COgreGlobal.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H
#include "pluginglueOGRE_COgreRootSetup.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H ? */

#include "pluginglueOGRE_COgrePluginAdapter.h"

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TOGREPLUGFPTR_StartPlugin ) ( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TOGREPLUGFPTR_StopPlugin ) ( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgrePluginAdapter::COgrePluginAdapter( void )
    : CORE::CIPlugin()
    , m_moduleHandle( GUCEF_NULL )
    , m_staticPlugin( GUCEF_NULL )
    , m_params()
    , m_metaData()
{GUCEF_TRACE;

    memset( m_funcPointers, 0, sizeof( m_funcPointers ) );
}

/*-------------------------------------------------------------------------*/

COgrePluginAdapter::~COgrePluginAdapter()
{GUCEF_TRACE;

    Unlink();
}

/*-------------------------------------------------------------------------*/

void
COgrePluginAdapter::SetPluginParams( const CORE::CValueList& pluginParams )
{GUCEF_TRACE;

    m_params = pluginParams;
}

/*-------------------------------------------------------------------------*/

const CORE::CValueList&
COgrePluginAdapter::GetPluginParams( void ) const
{GUCEF_TRACE;

    return m_params;
}

/*-------------------------------------------------------------------------*/

CORE::TPluginMetaDataPtr
COgrePluginAdapter::GetMetaData( void ) const
{GUCEF_TRACE;

    return m_metaData;
}

/*-------------------------------------------------------------------------*/

void*
COgrePluginAdapter::GetModulePointer( void )
{GUCEF_TRACE;

    return m_moduleHandle;
}

/*-------------------------------------------------------------------------*/

bool
COgrePluginAdapter::IsLoaded( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_moduleHandle;
}

/*-------------------------------------------------------------------------*/

bool
COgrePluginAdapter::Link( void* modulePtr                         ,
                          CORE::TPluginMetaDataPtr pluginMetaData )
{GUCEF_TRACE;

    if ( IsLoaded() || GUCEF_NULL == modulePtr || pluginMetaData.IsNULL() )
        return false;

    if ( GUCEF_NULL == Ogre::Root::getSingletonPtr() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgrePluginAdapter: There is no Ogre Root to install the Ogre plugin "
            << pluginMetaData->GetModuleFilename() << " in, the support library of Ogre must be loaded first" );
        return false;
    }

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "COgrePluginAdapter: Linking Ogre plugin using module pointer: " << CORE::PointerToString( modulePtr ) );

    // The name by which an application can hand over a plugin object: the module file name without directory,
    // extension and the prefix and debug suffix of the platform, "libOgrePlugin_x_d.so" is "OgrePlugin_x"
    const CORE::CString pluginName = COgrePluginManager::GetOgrePluginNameOfModule( pluginMetaData->GetModuleFilename() );

    Ogre::Plugin* staticPlugin = COgrePluginManager::GetStaticOgrePlugin( pluginName );
    if ( GUCEF_NULL != staticPlugin )
    {
        // the application owns the plugin object and handed it over: no exports are needed
        Ogre::Root::getSingleton().installPlugin( staticPlugin );
        m_staticPlugin = staticPlugin;
    }
    else
    {
        m_funcPointers[ FUNCPTR_STARTPLUGIN ] = CORE::GetFunctionAddress( modulePtr, "dllStartPlugin", 0 ).funcPtr;
        m_funcPointers[ FUNCPTR_STOPPLUGIN ] = CORE::GetFunctionAddress( modulePtr, "dllStopPlugin", 0 ).funcPtr;
        if ( GUCEF_NULL == m_funcPointers[ FUNCPTR_STARTPLUGIN ] || GUCEF_NULL == m_funcPointers[ FUNCPTR_STOPPLUGIN ] )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgrePluginAdapter: Failed to locate dllStartPlugin and dllStopPlugin in module: "
                << CORE::PointerToString( modulePtr ) << " and no plugin object is registered under the name " << pluginName );
            memset( m_funcPointers, 0, sizeof( m_funcPointers ) );
            return false;
        }

        try
        {
            // installs the plugin in the Ogre Root
            reinterpret_cast< TOGREPLUGFPTR_StartPlugin >( m_funcPointers[ FUNCPTR_STARTPLUGIN ] )();
        }
        catch ( const std::exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgrePluginAdapter: Exception while invoking dllStartPlugin of module "
                << CORE::PointerToString( modulePtr ) << ": " << e.what() );
            memset( m_funcPointers, 0, sizeof( m_funcPointers ) );
            return false;
        }
        catch ( ... )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgrePluginAdapter: Unknown exception while invoking dllStartPlugin of module "
                << CORE::PointerToString( modulePtr ) );
            memset( m_funcPointers, 0, sizeof( m_funcPointers ) );
            return false;
        }
    }

    m_moduleHandle = modulePtr;
    m_metaData = CORE::TPluginMetaDataStoragePtr( GUCEF_NEW CORE::CPluginMetaData( *pluginMetaData ) );
    m_metaData->SetPluginType( "OgrePlugin" );
    m_metaData->SetCopyright( "Please refer to the Ogre plugin module, most are MIT License" );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "COgrePluginAdapter: Ogre plugin " << pluginName << " is installed" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COgrePluginAdapter::Unlink( void )
{GUCEF_TRACE;

    if ( IsLoaded() )
    {
        Ogre::Root* root = Ogre::Root::getSingletonPtr();
        if ( GUCEF_NULL != root && root->isInitialised() && GUCEF_NULL == m_staticPlugin )
        {
            // An initialised Root has resources, render windows and a render system made by the code of its plugins, and
            // destroys them when it shuts down: a plugin module must not go before that. Taking out a plugin of an
            // initialised Root therefore destroys the Root first (Ogre uninstalls its plugins in it), as Ogre itself
            // unloads plugins only after its shutdown
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "COgrePluginAdapter: An Ogre plugin is taken out while the Ogre Root is initialised: "
                "the Root is destroyed first" );
            COgreGlobal::Instance()->GetRootSetup().DestroyRoot();
        }
        if ( GUCEF_NULL != Ogre::Root::getSingletonPtr() )
        {
            try
            {
                if ( GUCEF_NULL != m_staticPlugin )
                    Ogre::Root::getSingleton().uninstallPlugin( m_staticPlugin );
                else
                    reinterpret_cast< TOGREPLUGFPTR_StopPlugin >( m_funcPointers[ FUNCPTR_STOPPLUGIN ] )();
            }
            catch ( const std::exception& e )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgrePluginAdapter: Exception while taking out an Ogre plugin: " << e.what() );
            }
            catch ( ... )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgrePluginAdapter: Unknown exception while taking out an Ogre plugin" );
            }
        }

        // the module is unloaded after this, nothing may call into it again
        memset( m_funcPointers, 0, sizeof( m_funcPointers ) );
        m_staticPlugin = GUCEF_NULL;
        m_moduleHandle = GUCEF_NULL;
        m_metaData.Unlink();
    }
    return true;
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
