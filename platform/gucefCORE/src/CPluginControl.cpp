/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"          /* framework string implementation */
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"   /* Additional utilities for manipulating string class objects */
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CPLUGINMANAGER_H
#include "CPluginManager.h"    /* base class for all framework plugin managers */
#define GUCEF_CORE_CPLUGINMANAGER_H
#endif /* GUCEF_CORE_CPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_CLOADERDELEGATEDPLUGINLOADLOGIC_H
#include "gucefCORE_CLoaderDelegatedPluginLoadLogic.h"
#define GUCEF_CORE_CLOADERDELEGATEDPLUGINLOADLOGIC_H
#endif /* GUCEF_CORE_CLOADERDELEGATEDPLUGINLOADLOGIC_H ? */

#ifndef GUCEF_CORE_CSIMPLISTICPLUGINLOADLOGIC_H
#include "gucefCORE_CSimplisticPluginLoadLogic.h"
#define GUCEF_CORE_CSIMPLISTICPLUGINLOADLOGIC_H
#endif /* GUCEF_CORE_CSIMPLISTICPLUGINLOADLOGIC_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"        
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#include "CPluginControl.h"     /* definition of the class implemented here */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static CLoaderDelegatedPluginLoadLogic loaderDelegatedPluginLoadLogic;
static CSimplisticPluginLoadLogic simplisticPluginLoadLogic;

const CEvent CPluginControl::LoadingOfAllPluginsStartedEvent = "GUCEF::CORE::CPluginControl::LoadingOfAllPluginsStartedEvent";
const CEvent CPluginControl::LoadingOfAllPluginsFinishedEvent = "GUCEF::CORE::CPluginControl::LoadingOfAllPluginsFinishedEvent";
const CEvent CPluginControl::RegisteringOfAllPluginsStartedEvent = "GUCEF::CORE::CPluginControl::RegisteringOfAllPluginsStartedEvent";
const CEvent CPluginControl::RegisteringOfAllPluginsFinishedEvent = "GUCEF::CORE::CPluginControl::RegisteringOfAllPluginsFinishedEvent";
const CEvent CPluginControl::PluginLoadedEvent = "GUCEF::CORE::CPluginControl::PluginLoadedEvent";
const CEvent CPluginControl::PluginUnregisterStartedEvent = "GUCEF::CORE::CPluginControl::PluginUnregisterStartedEvent";
const CEvent CPluginControl::PluginUnregisteredEvent = "GUCEF::CORE::CPluginControl::PluginUnregisteredEvent";
const CEvent CPluginControl::PluginUnloadStartedEvent = "GUCEF::CORE::CPluginControl::PluginUnloadStartedEvent";
const CEvent CPluginControl::PluginUnloadedEvent = "GUCEF::CORE::CPluginControl::PluginUnloadedEvent";
const CEvent CPluginControl::UnregisterOfAllPluginsStartedEvent = "GUCEF::CORE::CPluginControl::UnregisterOfAllPluginsStartedEvent";
const CEvent CPluginControl::UnloadOfAllPluginsStartedEvent = "GUCEF::CORE::CPluginControl::UnloadOfAllPluginsStartedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CPluginControl::RegisterEvents( void )
{GUCEF_TRACE;

    LoadingOfAllPluginsStartedEvent.Initialize();
    LoadingOfAllPluginsFinishedEvent.Initialize();
    RegisteringOfAllPluginsStartedEvent.Initialize();
    RegisteringOfAllPluginsFinishedEvent.Initialize();
    PluginLoadedEvent.Initialize();
    PluginUnregisterStartedEvent.Initialize();
    PluginUnregisteredEvent.Initialize();
    PluginUnloadStartedEvent.Initialize();
    PluginUnloadedEvent.Initialize();
    UnregisterOfAllPluginsStartedEvent.Initialize();
    UnloadOfAllPluginsStartedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CPluginControl::~CPluginControl()
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    UnloadAll();

    // Unregister the load logic implementations available by default
    UnregisterPluginLoadLogic( loaderDelegatedPluginLoadLogic.GetLoaderLogicTypeName() );
    UnregisterPluginLoadLogic( simplisticPluginLoadLogic.GetLoaderLogicTypeName() );
}

/*-------------------------------------------------------------------------*/

CPluginControl::CPluginControl( void )
    : CTSGNotifier()
    , CGloballyConfigurable( true )
    , m_pluginLoadLogicProviders()
    , m_defaultPluginLoadLogicType( "Simplistic" )
    , m_pluginGroupsByPriority()
    , m_pluginGroupsByName()
    , m_rootDirs()
    , m_pluginManagers()
    , m_listedLinkBackModules()
{GUCEF_TRACE;

    // Register the load logic implementations available by default
    RegisterPluginLoadLogic( &simplisticPluginLoadLogic );
    RegisterPluginLoadLogic( &loaderDelegatedPluginLoadLogic );

    RegisterEvents();

    SubscribeTo( &CCoreGlobal::Instance()->GetApplication() );
}

/*-------------------------------------------------------------------------*/

void
CPluginControl::OnPumpedNotify( CNotifier* notifier    ,
                                const CEvent& eventid  ,
                                CICloneable* eventdata )
{GUCEF_TRACE;

    if ( CGUCEFApplication::AppShutdownCompleteEvent == eventid )
    {
        UnloadAll();
    }
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::LoadAllPluginsOfType( const CString& pluginTypeToLoad )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    bool success = true;

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Loading all modules of type \"" + pluginTypeToLoad + '\"' );

    // Loop trough groups looking for modules of the given type
    TPluginGroupMap::iterator i = m_pluginGroupsByPriority.begin();
    while ( i != m_pluginGroupsByPriority.end() )
    {
        TStringToPluginGroupMap& pluginGroups = (*i).second;
        TStringToPluginGroupMap::iterator n = pluginGroups.begin();
        while ( n != pluginGroups.end() )
        {
            TPluginGroupPtr& pluginGroup = (*n).second;
            CPluginGroup::TPluginMetaDataSet& pluginMetaDataSet = pluginGroup->GetPluginMetaData();

            CPluginGroup::TPluginMetaDataSet::iterator m = pluginMetaDataSet.begin();
            while ( m != pluginMetaDataSet.end() )
            {
                TPluginMetaDataStoragePtr pluginMetaData = (*m);

                if ( pluginTypeToLoad == pluginMetaData->GetPluginType() )
                {
                    // Try to load this module since it is of the type we are looking for
                    success = success && LoadPlugin( pluginMetaData ,
                                                     *pluginGroup   ,
                                                     (*n).first     ,
                                                     true           );
                }
                ++m;
            }
            ++n;
        }
        ++i;
    }

    return success;
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::LoadAllPluginsOfTypeInGroup( const CString& pluginTypeToLoad ,
                                             const CString& groupName        )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    bool success = true;

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Loading all modules in group \"" + groupName + "\" of type \"" + pluginTypeToLoad + '\"' );

    // Loop trough groups looking for modules of the given type
    TStringToPluginGroupMap::iterator i = m_pluginGroupsByName.find( groupName );
    if ( i != m_pluginGroupsByName.end() )
    {
        TPluginGroupPtr& pluginGroup = (*i).second;
        CPluginGroup::TPluginMetaDataSet& pluginMetaDataSet = pluginGroup->GetPluginMetaData();

        CPluginGroup::TPluginMetaDataSet::iterator n = pluginMetaDataSet.begin();
        while ( n != pluginMetaDataSet.end() )
        {
            TPluginMetaDataStoragePtr pluginMetaData = (*n);

            if ( pluginTypeToLoad == pluginMetaData->GetPluginType() )
            {
                // Try to load this module since it is of the type we are looking for
                success = success && LoadPlugin( pluginMetaData ,
                                                 *pluginGroup   ,
                                                 (*i).first     ,
                                                 true           );
            }
            ++n;
        }
    }

    return success;
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::LoadPluginGroup( const CString& groupName )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    bool success = true;

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Loading all modules in group \"" + groupName + '\"' );

    // Loop trough groups looking for modules of the given type
    TStringToPluginGroupMap::iterator i = m_pluginGroupsByName.find( groupName );
    if ( i != m_pluginGroupsByName.end() )
    {
        // Pull a copy of the plugin group info, it will get altered as we load invalidating the collection
        // hence we have to copy it here
        TPluginGroupPtr& pluginGroup = (*i).second;
        CPluginGroup::TPluginMetaDataSet pluginMetaDataSet = pluginGroup->GetPluginMetaData();

        CPluginGroup::TPluginMetaDataSet::iterator n = pluginMetaDataSet.begin();
        while ( n != pluginMetaDataSet.end() )
        {
            TPluginMetaDataStoragePtr pluginMetaData = (*n);

            // Try to load this module since it is of the type we are looking for
            success = success && LoadPlugin( pluginMetaData ,
                                             *pluginGroup   ,
                                             (*i).first     ,
                                             true           );
            ++n;
        }
    }

    return success;
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::LoadAll( bool registerAll                  ,
                         bool loadOnlyIfLoadImmediatelySet )
{GUCEF_TRACE;

    
    bool success = true;
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Loading all modules" );
    if ( !NotifyObservers( LoadingOfAllPluginsStartedEvent ) ) return false;
    
    MT::CObjectScopeLock lock( this );

    // Loop trough all groups by order of load priority
    TPluginGroupMap::iterator i = m_pluginGroupsByPriority.begin();
    while ( i != m_pluginGroupsByPriority.end() )
    {
        TStringToPluginGroupMap& pluginGroups = (*i).second;
        TStringToPluginGroupMap::iterator n = pluginGroups.begin();
        while ( n != pluginGroups.end() )
        {
            TPluginGroupPtr& pluginGroup = (*n).second;
            CPluginGroup::TPluginMetaDataSet pluginMetaDataSet = pluginGroup->GetPluginMetaData();

            CPluginGroup::TPluginMetaDataSet::iterator m = pluginMetaDataSet.begin();
            while ( m != pluginMetaDataSet.end() )
            {
                TPluginMetaDataStoragePtr pluginMetaData = (*m);

                if ( !loadOnlyIfLoadImmediatelySet || ( pluginMetaData->GetLoadImmediately() && loadOnlyIfLoadImmediatelySet ) )
                {
                    // Try to load this module since it is of the type we are looking for
                    // Note that we are not registering yet, we defer that
                    bool loadSuccess = LoadPlugin( pluginMetaData ,
                                                   *pluginGroup   ,
                                                   (*n).first     ,
                                                   false          );
                    if ( !loadSuccess )
                        if ( pluginMetaData->GetLoadFailAllowed() )
                            loadSuccess = true; // ignore
                    success = success && loadSuccess;
                }
                ++m;
            }
            ++n;
        }
        ++i;
    }
    
    lock.EarlyUnlock();

    NotifyObservers( LoadingOfAllPluginsFinishedEvent );    
    
    if ( registerAll )
        return RegisterAll( loadOnlyIfLoadImmediatelySet ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::RegisterAll( bool registerOnlyIfLoadImmediatelySet )
{GUCEF_TRACE;

    
    bool success = true;
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Registering all modules" );
    if ( !NotifyObservers( RegisteringOfAllPluginsStartedEvent ) ) return false;

    MT::CObjectScopeLock lock( this );
    
    // Loop trough all groups by order of load priority
    TPluginGroupMap::iterator i = m_pluginGroupsByPriority.begin();
    while ( i != m_pluginGroupsByPriority.end() )
    {
        TStringToPluginGroupMap& pluginGroups = (*i).second;
        TStringToPluginGroupMap::iterator n = pluginGroups.begin();
        while ( n != pluginGroups.end() )
        {
            TPluginGroupPtr& pluginGroup = (*n).second;
            
            // note that we need to pull a copy of the plugin group info, it will get altered as we register invalidating the collection
            CPluginGroup::TPluginMetaDataSet pluginMetaDataSet = pluginGroup->GetPluginMetaData();
            CPluginGroup::TPluginMetaDataSet::iterator m = pluginMetaDataSet.begin();
            while ( m != pluginMetaDataSet.end() )
            {
                TPluginMetaDataStoragePtr pluginMetaData = (*m);

                // Registration is a follow-on step to module loading.
                // As such we do not consider entries here which are not loaded
                if ( GUCEF_NULL != pluginMetaData->GetModulePointer() )
                {
                    if ( !registerOnlyIfLoadImmediatelySet || ( pluginMetaData->GetLoadImmediately() && registerOnlyIfLoadImmediatelySet ) )
                    {
                        // Try to register the functionality in the module
                        bool registerSuccess = RegisterPlugin( pluginMetaData ,
                                                               *pluginGroup   ,
                                                               (*n).first     );
                        if ( !registerSuccess )
                            if ( pluginMetaData->GetLoadFailAllowed() )
                                registerSuccess = true; // ignore

                        success = success && registerSuccess;
                    }
                }
                ++m;
            }
            ++n;
        }
        ++i;
    }
    
    lock.EarlyUnlock();
    
    NotifyObservers( RegisteringOfAllPluginsFinishedEvent );    
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::LoadPlugin( TPluginMetaDataStoragePtr& pluginMetaData ,
                            CPluginGroup& pluginGroup                 ,
                            const CString& groupName                  ,
                            bool registerPlugin                       )
{GUCEF_TRACE;

    if ( pluginMetaData.IsNULL() )
        return false;

    if ( GUCEF_NULL != pluginMetaData->GetModulePointer() )
    {
        // The module is already loaded
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "PluginControl: Module with name \"" +
                                pluginMetaData->GetModuleFilename() + "\", alt name \"" + pluginMetaData->GetAltModuleFilename() +
                                "\", and group \"" + groupName + "\" is already loaded" );
        if ( registerPlugin )
        {
            return RegisterPlugin( pluginMetaData, pluginGroup, groupName );
        }
        return true;
    }
    
    MT::CObjectScopeLock lock( this );

    // Check to see whether the module has a specific loader logic type defined
    // if not we will use the default.
    CString pluginLoaderLogicToUse = pluginMetaData->GetLoaderLogicTypeName().IsNULLOrEmpty() ?
            m_defaultPluginLoadLogicType : pluginMetaData->GetLoaderLogicTypeName();

    TPluginLoadLogicMap::iterator m = m_pluginLoadLogicProviders.find( pluginLoaderLogicToUse );
    if ( m != m_pluginLoadLogicProviders.end() )
    {
        // Check if the module has its own full path defined
        void* modulePtr = GUCEF_NULL;
        if ( pluginMetaData->GetFullModulePath().IsNULLOrEmpty() )
        {
            // Use the globally defined module root dirs to try and load the module
            TVersion versionInfo = pluginMetaData->GetVersion();
            TStringSet::iterator i = m_rootDirs.begin();
            while ( i != m_rootDirs.end() && modulePtr == NULL )
            {
                CString filename = pluginMetaData->GetModuleFilename();

                modulePtr = (*m).second->LoadPlugin( RelativePath( (*i) ) ,
                                                     filename             ,
                                                     groupName            ,
                                                     &versionInfo         );

                if ( GUCEF_NULL == modulePtr )
                {
                    filename = pluginMetaData->GetAltModuleFilename();
                    if ( !filename.IsNULLOrEmpty() )
                    {
                        modulePtr = (*m).second->LoadPlugin( RelativePath( (*i) ) ,
                                                             filename             ,
                                                             groupName            ,
                                                             &versionInfo         );
                    }
                }

                ++i;
            }

            if ( GUCEF_NULL == modulePtr )
            {
                // We were not able to load the module, no point in proceeding
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginControl: Failed to load module with name \"" +
                                        pluginMetaData->GetModuleFilename() + "\", alt name \"" + pluginMetaData->GetAltModuleFilename() +
                                        "\", and group \"" + groupName + "\" and version " +
                                        VersionToString( pluginMetaData->GetVersion() ) );
                return false;
            }
        }
        else
        {
            // Use the path defined with the plugin as the root to load the module from
            TVersion versionInfo = pluginMetaData->GetVersion();
            modulePtr = (*m).second->LoadPlugin( RelativePath( pluginMetaData->GetFullModulePath() ) ,
                                                 pluginMetaData->GetModuleFilename()                 ,
                                                 groupName                                           ,
                                                 &versionInfo                                        );

            if ( GUCEF_NULL == modulePtr )
            {
                CString filename = pluginMetaData->GetAltModuleFilename();
                if ( !filename.IsNULLOrEmpty() )
                {
                    modulePtr = (*m).second->LoadPlugin( RelativePath( pluginMetaData->GetFullModulePath() ) ,
                                                         filename                                            ,
                                                         groupName                                           ,
                                                         &versionInfo                                        );
                }
            }

            if ( GUCEF_NULL == modulePtr )
            {
                // We were not able to load the module, no point in proceeding
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginControl: Failed to load module with name \"" +
                                        pluginMetaData->GetModuleFilename() + "\", alt name \"" + pluginMetaData->GetAltModuleFilename() +
                                        "\", and group \"" + groupName + "\" and version " +
                                        VersionToString( pluginMetaData->GetVersion() ) + " using provided path " + pluginMetaData->GetFullModulePath() );
                return false;
            }
        }

        // We loaded the module
        pluginMetaData->SetModulePointer( modulePtr );
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Loaded module with name \"" +
                                pluginMetaData->GetModuleFilename() + "\" and group \"" + groupName + "\" and version " +
                                VersionToString( pluginMetaData->GetVersion() ) );

        TPluginLoadedEventData pluginLoadedEventData( CString::StringPair( groupName, pluginMetaData->GetModuleFilename() ) );
        if ( !NotifyObservers( PluginLoadedEvent, &pluginLoadedEventData ) ) return false;
        
        if ( registerPlugin )
        {
            if ( !RegisterPlugin( pluginMetaData, pluginGroup, groupName ) )
            {
                // We were not able to register the module, no point in proceeding
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginControl: Failed to register module with name \"" +
                                        pluginMetaData->GetModuleFilename() + "\" and group \"" + groupName + "\" and version " +
                                        VersionToString( pluginMetaData->GetVersion() ) );
                
                // Undo the loading of the module
                pluginMetaData->SetModulePointer( GUCEF_NULL );
                (*m).second->UnloadPlugin( modulePtr );
                
                return false;
            
            }
        }
        return true;        
    }
    return false;
}


/*-------------------------------------------------------------------------*/

bool
CPluginControl::RegisterPlugin( TPluginMetaDataStoragePtr& pluginMetaData ,
                                CPluginGroup& pluginGroup                 ,
                                const CString& groupName                  )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    if ( pluginMetaData.IsNULL() )
        return false;

    if ( GUCEF_NULL == pluginMetaData->GetModulePointer() )
        return false;

    // Now that we verified that the module itself has been successfully loaded we can hand of the module specifics to the appropriate
    // plugin manager which in turn can check and load all symbols etc. The plugin manager will hand us a CIPlugin based
    // object in exchange for our module pointer upon a successfull registration.
    CString pluginType = pluginMetaData->GetPluginType();

    if ( pluginType.IsNULLOrEmpty() )
    {
        // Safety 2: Only when no plugin type was set in the metadata.
        // GUCEFPlugin_GetLinkBackModuleDependencies is an export unique to generic plugins.
        // If present, we can definitively identify the plugin type without trial and error,
        // and set it in the metadata so the typed registration path is used instead.
        if ( GUCEF_NULL != GetFunctionAddress( pluginMetaData->GetModulePointer()              ,
                                               "GUCEFPlugin_GetLinkBackModuleDependencies"     ,
                                               0                                                ).funcPtr )
        {
            pluginType = "GucefGenericPlugin";
            pluginMetaData->SetPluginType( pluginType );
            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Module \"" + pluginMetaData->GetModuleFilename() +
                    "\" exports GUCEFPlugin_GetLinkBackModuleDependencies, definitively identified as GucefGenericPlugin" );
        }
    }

    if ( pluginType.IsNULLOrEmpty() )
    {
        // Without a plugin type predefined we will have to use trial and error.
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Module with name \"" +
                                pluginMetaData->GetModuleFilename() + "\" and group \"" + groupName + "\" and version " +
                                VersionToString( pluginMetaData->GetVersion() ) +
                                " does not have a plugin type defined, an attempt will be made to register at any of the available plugin managers via trial and error" );

        TPluginManagerSet::iterator w = m_pluginManagers.begin();
        while ( w != m_pluginManagers.end() )
        {
            TPluginPtr pluginPtr = (*w)->RegisterPlugin( pluginMetaData->GetModulePointer(), pluginMetaData );
            if ( pluginPtr )
            {
                pluginGroup.GetPluginMetaData().erase( pluginMetaData );
                pluginGroup.GetPlugins().insert( pluginPtr );
                GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Registered module with name \"" +
                                        pluginMetaData->GetModuleFilename() + "\" and group \"" + groupName + "\" and version " +
                                        VersionToString( pluginMetaData->GetVersion() ) + " with plugin manager of type \"" + (*w)->GetPluginType() + "\"" );
                return true;
            }
            ++w;
        }

        // We were not able to register the module with any of the managers even though we could load it
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginControl: Failed to register module with name \"" +
                                pluginMetaData->GetModuleFilename() + "\" and group \"" + groupName + "\" and version " +
                                VersionToString( pluginMetaData->GetVersion() ) + " with any plugin manager" );
        return false;
    }

    // A plugin type was defined (either originally or identified via Safety 2) so we know what manager to use
    CPluginManager* pluginManager = GetPluginManagerForType( pluginType );
    if ( pluginManager != GUCEF_NULL )
    {
        TPluginPtr pluginPtr = pluginManager->RegisterPlugin( pluginMetaData->GetModulePointer(), pluginMetaData );
        if ( pluginPtr )
        {
            pluginGroup.GetPluginMetaData().erase( pluginMetaData );
            pluginGroup.GetPlugins().insert( pluginPtr );
            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Registered module with name \"" +
                                    pluginMetaData->GetModuleFilename() + "\" and group \"" + groupName + "\" and version " +
                                    VersionToString( pluginMetaData->GetVersion() ) + " with plugin manager of type \"" + pluginType + "\"" );
            return true;
        }

        // We were not able to register the module even though we could load it
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginControl: Failed to register module with name \"" +
                                pluginMetaData->GetModuleFilename() + "\" and group \"" + groupName + "\" and version " +
                                VersionToString( pluginMetaData->GetVersion() ) + " with plugin manager of type \"" + pluginType + "\"" );
    }
    else
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginControl: Failed to register module with name \"" +
                                pluginMetaData->GetModuleFilename() + "\" and group \"" + groupName + "\" and version " +
                                VersionToString( pluginMetaData->GetVersion() ) + " with plugin manager of type \"" + pluginType +
                                "\", no plugin manager is registered capable of handeling the given type" );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CPluginManager*
CPluginControl::GetPluginManagerForType( const CString& pluginType )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    TPluginManagerSet::iterator w = m_pluginManagers.begin();
    while ( w != m_pluginManagers.end() )
    {
        if ( (*w)->GetPluginType() == pluginType )
        {
            return (*w);
        }
        ++w;
    }
    return NULL;
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::UnregisterPlugin( const CString& groupName  ,
                                  const CString& moduleName )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    TStringToPluginGroupMap::iterator i = m_pluginGroupsByName.find( groupName );
    if ( i != m_pluginGroupsByName.end() )
    {
        TPluginGroupPtr& pluginGroup = (*i).second;
        TPluginPtr plugin = pluginGroup->FindPluginWithModuleName( moduleName );

        // Is the plugin even loaded in the first place?
        if ( !plugin.IsNULL() )
        {
            // It is, proceed with unregister
            return UnregisterPlugin( plugin, *pluginGroup, groupName );
        }
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::UnloadPlugin( const CString& groupName  ,
                              const CString& moduleName )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    TStringToPluginGroupMap::iterator i = m_pluginGroupsByName.find( groupName );
    if ( i != m_pluginGroupsByName.end() )
    {
        TPluginGroupPtr& pluginGroup = (*i).second;
        TPluginPtr plugin = pluginGroup->FindPluginWithModuleName( moduleName );

        // Is the plugin even loaded in the first place?
        if ( !plugin.IsNULL() )
        {
            // It is, proceed with unload
            return UnloadPlugin( plugin, *pluginGroup, groupName );
        }
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Module \"" + moduleName + "\" in group \"" + groupName + "\" is not loaded. Intent to unload is satisfied." );
        return true;
    }
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Group \"" + groupName + "\" not found when trying to unload module \"" + moduleName + "\". Intent to unload is satisfied." );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::UnregisterPlugin( TPluginPtr& pluginPtr     ,
                                  CPluginGroup& pluginGroup ,
                                  const CString& groupName  )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    TPluginMetaDataPtr pluginMetaData = pluginPtr->GetMetaData();
    if ( !pluginMetaData )
    {
        // This plugin does not have any meta data set thus we cannot unload it
        // It will have to stay in process memmory
        return false;
    }

    // Note that we make a copy of the meta data since the interface we have might be pointing to a implementation which needs
    // the module to be loaded which is the plugin managers right to do so.
    TPluginMetaDataPtr metaDataCopyPtr( GUCEF_NEW CPluginMetaData( *pluginMetaData ) );

    // First we unregister the plugin with its plugin manager
    CString pluginType = metaDataCopyPtr->GetPluginType();
    CPluginManager* pluginManager = GetPluginManagerForType( pluginType );
    if ( pluginManager != GUCEF_NULL )
    {
        // This plugin manager can handle a module of this type.
        // We will try to unregister the module here as a plugin
        if ( pluginManager->UnregisterPlugin( pluginPtr ) )
        {
            // We unregistered the module
            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Unregistered module with name \"" +
                                    metaDataCopyPtr->GetModuleFilename() + "\" and group \"" + groupName + "\" and version " +
                                    VersionToString( metaDataCopyPtr->GetVersion() ) + " with plugin manager of type \"" + pluginType + "\"" );

        }
        else
        {
            // We unregistered the module
            GUCEF_ERROR_LOG( LOGLEVEL_CRITICAL, "PluginControl: plugin manager failed to unregistered module with name \"" +
                                    metaDataCopyPtr->GetModuleFilename() + "\" and group \"" + groupName + "\" and version " +
                                    VersionToString( metaDataCopyPtr->GetVersion() ) + " with plugin manager of type \"" + pluginType + "\"" );
            return false;
        }
    }
    else
    {
        // this should not happen but unload the module anyway
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginControl: Failed to find a plugin manager capable of handling type \"" + pluginType + "\" when unregistering module with name \"" +
                            pluginMetaData->GetModuleFilename() + "\" and group \"" + groupName + "\" and version " + VersionToString( metaDataCopyPtr->GetVersion() ) );
        return false;
    }

    // Remove the plugin entry and instead add a metadata plugin entry
    pluginGroup.GetPlugins().erase( pluginPtr );
    pluginGroup.GetPluginMetaData().insert( metaDataCopyPtr );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::UnloadPlugin( TPluginPtr& pluginPtr     ,
                              CPluginGroup& pluginGroup ,
                              const CString& groupName  )
{GUCEF_TRACE;

    if ( pluginPtr.IsNULL() )
        return true;

    TPluginMetaDataPtr pluginMetaData = pluginPtr->GetMetaData();
    if ( pluginMetaData.IsNULL() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginControl: Plugin in group \"" + groupName + "\" has no metadata; cannot unload" );
        return false;
    }

    CString moduleFilename      = pluginMetaData->GetModuleFilename();
    void*   modulePtr           = pluginPtr->GetModulePointer();
    TVersion version            = pluginMetaData->GetVersion();
    CString pluginLoaderLogicToUse = pluginMetaData->GetLoaderLogicTypeName().IsNULLOrEmpty() ?
            m_defaultPluginLoadLogicType : pluginMetaData->GetLoaderLogicTypeName();

    if ( !UnregisterPlugin( pluginPtr   ,
                            pluginGroup ,
                            groupName   ) )
    {
        // Could not unregister via the plugin manager (e.g. no manager registered for this type).
        // Log the issue but proceed with the physical DLL unload as requested.
        // The module's own unload callback will handle any remaining self-deregistration.
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "PluginControl: Unable to cleanly unregister module \"" +
                moduleFilename + "\" in group \"" + groupName + "\". Forcing DLL unload." );

        // UnregisterPlugin normally removes the entry from m_plugins; do it manually here
        pluginGroup.GetPlugins().erase( pluginPtr );
    }
    else
    {
        // Unregister succeeded. The metadata copy was placed back into m_pluginMetaData;
        // re-acquire it in case the plugin manager updated the loader logic type name.
        TPluginMetaDataPtr postUnregisterMeta = pluginGroup.FindPluginMetaDataWithModuleName( moduleFilename );
        if ( !postUnregisterMeta.IsNULL() && !postUnregisterMeta->GetLoaderLogicTypeName().IsNULLOrEmpty() )
            pluginLoaderLogicToUse = postUnregisterMeta->GetLoaderLogicTypeName();
    }

    if ( GUCEF_NULL == modulePtr )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginControl: Module pointer is NULL for \"" + moduleFilename +
                "\" in group \"" + groupName + "\"; cannot physically unload DLL" );
        return false;
    }

    // Check to see whether the module has a specific loader logic type defined
    // if not we will use the default.
    TPluginLoadLogicMap::iterator m = m_pluginLoadLogicProviders.find( pluginLoaderLogicToUse );
    if ( m != m_pluginLoadLogicProviders.end() )
    {
        TPluginUnloadStartedEventData pluginUnloadStartedEventData( CString::StringPair( groupName, moduleFilename ) );
        if ( !NotifyObservers( PluginUnloadStartedEvent, &pluginUnloadStartedEventData ) ) return false;

        (*m).second->UnloadPlugin( modulePtr );

        // We will unload the module
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Unloaded module with name \"" +
                                moduleFilename + "\" and group \"" + groupName + "\" and version " +
                                VersionToString( version ) + " using plugin loader logic of type \"" + pluginLoaderLogicToUse + '\"' );

        TPluginUnloadedEventData pluginUnloadedEventData( CString::StringPair( groupName, moduleFilename ) );
        if ( !NotifyObservers( PluginUnloadedEvent, &pluginUnloadedEventData ) ) return false;
        return true;
    }

    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginControl: Failed to find plugin loader logic of type \"" + pluginLoaderLogicToUse +
            "\" for unloading a plugin of type \"" + pluginMetaData->GetPluginType() + "\" and name \"" +
            moduleFilename + "\" and group \"" + groupName + "\" and version " + VersionToString( version ) );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::UnloadAllPluginsOfTypeInGroup( const CString& pluginTypeToLoad ,
                                               const CString& groupName        )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    bool success = true;

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Unloading all modules in group \"" + groupName + "\" of type \"" + pluginTypeToLoad + '\"' );

    // Find the groups specified
    TStringToPluginGroupMap::iterator i = m_pluginGroupsByName.find( groupName );
    if ( i != m_pluginGroupsByName.end() )
    {
        TPluginGroupPtr& pluginGroup = (*i).second;
        CPluginGroup::TPluginSet& pluginSet = pluginGroup->GetPlugins();

        // Look for a plugin of the given type
        CPluginGroup::TPluginSet::iterator n = pluginSet.begin();
        while ( n != pluginSet.end() )
        {
            TPluginPtr plugin = (*n);
            TPluginMetaDataPtr pluginMetaData = plugin->GetMetaData();

            if ( pluginMetaData )
            {
                if ( pluginTypeToLoad == pluginMetaData->GetPluginType() )
                {
                    // Try to unload this module since it is of the type we are looking for
                    success = success && UnloadPlugin( plugin       ,
                                                       *pluginGroup ,
                                                       (*i).first  );
                }
            }
            ++n;
        }
    }

    return success;
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::UnloadAllPluginsOfType( const CString& pluginTypeToUnload )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    bool success = true;

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Unloading all modules of type \"" + pluginTypeToUnload + '\"' );

    // Loop trough groups looking for modules of the given type
    TPluginGroupMap::reverse_iterator i = m_pluginGroupsByPriority.rbegin();
    while ( i != m_pluginGroupsByPriority.rend() )
    {
        TStringToPluginGroupMap& pluginGroups = (*i).second;
        TStringToPluginGroupMap::iterator n = pluginGroups.begin();
        while ( n != pluginGroups.end() )
        {
            const CString& pluginGroupName = (*n).first;
            TPluginGroupPtr& pluginGroup = (*n).second;
            CPluginGroup::TPluginSet& pluginSet = pluginGroup->GetPlugins();

            CPluginGroup::TPluginSet::iterator m = pluginSet.begin();
            while ( m != pluginSet.end() )
            {
                TPluginPtr plugin = (*m);

                if ( pluginTypeToUnload == plugin->GetMetaData()->GetPluginType() )
                {
                    // Try to unload this module since it is of the type we are looking for
                    success = success && UnloadPlugin( plugin          ,
                                                       *pluginGroup    ,
                                                       pluginGroupName );
                }
                ++m;
            }
            ++n;
        }
        ++i;
    }

    return success;
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::UnloadPluginGroup( const CString& groupName )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    bool success = true;

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Unloading all modules in group \"" + groupName + '\"' );

    // Loop trough groups looking for modules of the given type
    TStringToPluginGroupMap::iterator i = m_pluginGroupsByName.find( groupName );
    if ( i != m_pluginGroupsByName.end() )
    {
        // must make a copy as unregister can mess with the iterators
        TPluginGroupPtr pluginGroup = (*i).second;
        CPluginGroup::TPluginSet pluginSet = pluginGroup->GetPlugins();

        if ( pluginSet.empty() )
        {
            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Group \"" + groupName + "\" has no loaded modules. Intent to unload is satisfied." );
        }

        CPluginGroup::TPluginSet::iterator n = pluginSet.begin();
        while ( n != pluginSet.end() )
        {
            TPluginPtr plugin = (*n);

            // Try to unload this module since it is of the type we are looking for
            success = success && UnloadPlugin( plugin       ,
                                               *pluginGroup ,
                                               (*i).first  );
            ++n;
        }
    }
    else
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Group \"" + groupName + "\" not found. Intent to unload is satisfied." );
    }

    return success;
}

/*-------------------------------------------------------------------------*/

void
CPluginControl::GetAvailablePluginGroups( TStringSet& groupNames ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    TStringToPluginGroupMap::const_iterator i = m_pluginGroupsByName.begin();
    while ( i != m_pluginGroupsByName.end() )
    {
        groupNames.insert( (*i).first );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CPluginControl::GetAvailablePluginsForGroup( const CString& groupName ,
                                             TStringSet& modules      ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    TStringToPluginGroupMap::const_iterator i = m_pluginGroupsByName.find( groupName );
    if ( i != m_pluginGroupsByName.end() )
    {
        const TPluginGroupPtr& pluginGroup = (*i).second;

        // Get modules not loaded yet
        const CPluginGroup::TPluginMetaDataSet& metaData = pluginGroup->GetPluginMetaData();
        CPluginGroup::TPluginMetaDataSet::const_iterator n = metaData.begin();
        while ( n != metaData.end() )
        {
            modules.insert( (*n)->GetModuleFilename() );
            ++n;
        }

        // Get modules already loaded
        const CPluginGroup::TPluginSet& plugins = pluginGroup->GetPlugins();
        CPluginGroup::TPluginSet::const_iterator p = plugins.begin();
        while ( p != plugins.end() )
        {
            modules.insert( (*p)->GetMetaData()->GetModuleFilename() );
            ++p;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::UnregisterAll( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    bool success = true;

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Unregistering all modules" );

    if ( !NotifyObservers( UnregisterOfAllPluginsStartedEvent ) ) return false;

    // When we unload all we have to take into account intra-dependencies
    // as such we unload indirectly via group and module names vs accessing
    // the modules directly. That administration might get invalided as we
    // start unloading

    TStringSet groupNames;
    GetAvailablePluginGroups( groupNames );

    TStringSet::iterator i = groupNames.begin();
    while ( i != groupNames.end() )
    {
        TStringSet modules;
        GetAvailablePluginsForGroup( (*i), modules );

        TStringSet::iterator n = modules.begin();
        while ( n != modules.end() )
        {
            success = success && UnregisterPlugin( (*i), (*n) );
            ++n;
        }
        ++i;
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::UnloadAll( void )
{GUCEF_TRACE;

    // When we unload all we have to take into account intra-dependencies
    // as such we explicitly unregister all plugins first before even beginning the unloading of any modules
    // this causes Unload functions to be called on all the plugins before any module is unloaded
    if ( !UnregisterAll() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginControl: Cannot unload all plugins since we failed to unregister all modules" );
        return false;
    }

    MT::CObjectScopeLock lock( this );
    bool success = true;

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Unloading all modules" );

    if ( !NotifyObservers( UnloadOfAllPluginsStartedEvent ) ) return false;

    // When we unload all we have to take into account intra-dependencies
    // as such we unload indirectly via group and module names vs accessing
    // the modules directly. That administration might get invalided as we
    // start unloading

    TStringSet groupNames;
    GetAvailablePluginGroups( groupNames );

    TStringSet::iterator i = groupNames.begin();
    while ( i != groupNames.end() )
    {
        TStringSet modules;
        GetAvailablePluginsForGroup( (*i), modules );

        TStringSet::iterator n = modules.begin();
        while ( n != modules.end() )
        {
            success = success && UnloadPlugin( (*i), (*n) );
            ++n;
        }
        ++i;
    }
    return success;
}

/*-------------------------------------------------------------------------*/

void
CPluginControl::ClearMetaDataFromGroup( const CString& groupName )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    TStringToPluginGroupMap::iterator i = m_pluginGroupsByName.find( groupName );
    if ( i != m_pluginGroupsByName.end() )
    {
        TPluginGroupPtr& pluginGroup = (*i).second;
        pluginGroup->GetPluginMetaData().clear();

        // If no plugins are loaded & we have no metadata then get rid of the group alltogether
        if ( pluginGroup->GetPlugins().empty() )
        {
            m_pluginGroupsByPriority[ pluginGroup->GetGroupPriority() ].erase( groupName );
            m_pluginGroupsByName.erase( i );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CPluginControl::GetSupportedPluginTypes( TStringVector& supportedTypes ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    TPluginManagerSet::const_iterator i = m_pluginManagers.begin();
    while ( i != m_pluginManagers.end() )
    {
        supportedTypes.push_back( (*i)->GetPluginType() );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CPluginControl::Register( CPluginManager* pman )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_pluginManagers.insert( pman );
}

/*-------------------------------------------------------------------------*/

void
CPluginControl::Unregister( CPluginManager* pman )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_pluginManagers.erase( pman );
}

/*-------------------------------------------------------------------------*/

void
CPluginControl::RegisterPluginLoadLogic( CIPluginLoadLogic* pluginLoaderLogic )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_pluginLoadLogicProviders[ pluginLoaderLogic->GetLoaderLogicTypeName() ] = pluginLoaderLogic;
}

/*-------------------------------------------------------------------------*/

void
CPluginControl::UnregisterPluginLoadLogic( const CString& loaderLogicTypeName )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    TPluginLoadLogicMap::iterator i = m_pluginLoadLogicProviders.find( loaderLogicTypeName );
    if ( i != m_pluginLoadLogicProviders.end() )
    {
        m_pluginLoadLogicProviders.erase( i );
    }
}

/*-------------------------------------------------------------------------*/

void
CPluginControl::SetDefaultPluginLoadLogicType( const CString& loaderLogicTypeName )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_defaultPluginLoadLogicType = loaderLogicTypeName;
}

/*-------------------------------------------------------------------------*/

CString
CPluginControl::GetDefaultPluginLoadLogicType( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    CString returnValue = m_defaultPluginLoadLogicType;
    return returnValue;
}

/*-------------------------------------------------------------------------*/

void
CPluginControl::AddPluginDir( const CString& path )
{GUCEF_TRACE;

    CString absPath( RelativePath( path ) );
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Adding path as plugin dir: " + absPath );

    MT::CObjectScopeLock lock( this );
    m_rootDirs.insert( absPath );
}

/*-------------------------------------------------------------------------*/

void
CPluginControl::RemovePluginDir( const CString& path )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Removing path as plugin dir: " + path );

    MT::CObjectScopeLock lock( this );
    m_rootDirs.erase( RelativePath( path ) );
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::AddPluginFromDir( const CString& pluginPath ,
                                  const CString& groupName  ,
                                  bool loadImmediatly       )
{GUCEF_TRACE;

    CString pathToPlugin = RelativePath( pluginPath );

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Defining file " + pathToPlugin + " as a plugin" );

    // All we can define using this method is the directory and the filename
    // The module will have to be matched to a plugin manager via trial and error
    CString filename = ExtractFilename( pathToPlugin );
    CPluginMetaData pluginMetaData;
    pluginMetaData.SetFullModulePath( pathToPlugin.CutChars( filename.Length(), false ) );
    pluginMetaData.SetModuleFilename( filename );

    return AddPluginMetaData( pluginMetaData ,
                              groupName      ,
                              loadImmediatly );
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::AddAllPluginsFromDir( const CString& pluginDir ,
                                      const CString& groupName ,
                                      bool loadImmediatly      )
{GUCEF_TRACE;

    bool success = true;
    struct SDI_Data* did = DI_First_Dir_Entry( pluginDir.C_String() );
    if ( GUCEF_NULL != did )
    {
        do
        {
            if ( DI_Is_It_A_File( did ) == 1 )
            {
                CString filename = DI_Name( did );
                CString fileExt = ExtractFileExtention( filename );

                #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
                if ( fileExt.Lowercase() == "dll" )
                #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
                if ( fileExt.Lowercase() == "so" )
                #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS )
                if ( fileExt.Lowercase() == "dynlib" )
                #endif
                {
                    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Defining file " + filename + " as a plugin" );

                    // All we can define using this method is the directory and the filename
                    // The module will have to be matched to a plugin manager via trial and error
                    CPluginMetaData pluginMetaData;
                    pluginMetaData.SetFullModulePath( pluginDir );
                    pluginMetaData.SetModuleFilename( filename );

                    success = success && AddPluginMetaData( pluginMetaData ,
                                                            groupName      ,
                                                            loadImmediatly );
                }
            }
        }
        while ( DI_Next_Dir_Entry( did ) != 0 );

        DI_Cleanup( did );
    }

    return success;
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::SearchForPluginInPluginDirs( const CString& pluginFileName ,
                                             const CString& groupName      ,
                                             bool loadImmediatly           )
{GUCEF_TRACE;

    CString fileName( pluginFileName );
    if ( 0 == ExtractFileExtention( pluginFileName ).Length() )
    {
        // use the platform default extension to complete the filename
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        fileName += ".dll";
        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
        fileName += ".so";
        #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS )
        fileName += ".dynlib";
        #endif
    }

    MT::CObjectScopeLock lock( this );
    TStringSet::iterator i = m_rootDirs.begin();
    while ( i != m_rootDirs.end() )
    {
        CString testPath = (*i);
        AppendToPath( testPath, fileName );
        if ( FileExists( testPath ) )
        {
            return AddPluginFromDir( testPath       ,
                                     groupName      ,
                                     loadImmediatly );
        }
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

TPluginGroupPtr 
CPluginControl::GetOrCreatePluginGroup( const CString& groupName, UInt32 priorityIfNotExist )
{GUCEF_TRACE;

    TPluginGroupPtr pluginGroup;
    TStringToPluginGroupMap::iterator i = m_pluginGroupsByName.find( groupName );
    if ( i != m_pluginGroupsByName.end() )
    {
        pluginGroup = (*i).second;
    }
    else
    {
        // This group does not exist yet, create it
        pluginGroup = CPluginGroup::CreateSharedObj();
        if ( pluginGroup.IsNULL() )
            return pluginGroup;

        pluginGroup->SetGroupPriority( priorityIfNotExist );
        m_pluginGroupsByName[ groupName ] = pluginGroup;
        m_pluginGroupsByPriority[ pluginGroup->GetGroupPriority() ][ groupName ] = pluginGroup;

        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Created plugin group with name \"" + groupName + "\" and priority " + ToString( priorityIfNotExist ) );
    }

    return pluginGroup;
}
/*-------------------------------------------------------------------------*/

bool
CPluginControl::AddPluginMetaDataImpl( TPluginMetaDataStoragePtr pluginMetaData ,
                                       const CString& groupName                 ,
                                       bool loadImmediately                     ,
                                       bool registerImmediately                 )
{GUCEF_TRACE;

    bool success = true;
    
    MT::CObjectScopeLock lock( this );

    TPluginGroupPtr pluginGroup = GetOrCreatePluginGroup( groupName );
    if ( pluginGroup.IsNULL() )
        return false;

    // Before adding metadata or loading: check if the same module filename is already
    // loaded and initialized in any plugin group. Calling Load() on an already-resident
    // module is unsafe. If found, the caller's intent is considered satisfied without
    // a second load.
    if ( loadImmediately )
    {
        CString moduleFilename = pluginMetaData->GetModuleFilename();
        if ( !moduleFilename.IsNULLOrEmpty() )
        {
            TStringToPluginGroupMap::iterator gi = m_pluginGroupsByName.begin();
            while ( gi != m_pluginGroupsByName.end() )
            {
                CPluginGroup::TPluginSet& loadedPlugins = (*gi).second->GetPlugins();
                CPluginGroup::TPluginSet::iterator pi = loadedPlugins.begin();
                while ( pi != loadedPlugins.end() )
                {
                    TPluginMetaDataPtr piMeta = (*pi)->GetMetaData();
                    if ( piMeta && moduleFilename == piMeta->GetModuleFilename() )
                    {
                        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Module \"" + moduleFilename +
                            "\" is already loaded and initialized in group \"" + (*gi).first +
                            "\", skipping duplicate load request. Intent is satisfied." );
                        return true;
                    }
                    ++pi;
                }
                ++gi;
            }
        }
    }

    // Add the metadata
    pluginGroup->GetPluginMetaData().insert( pluginMetaData );

    // If an immediate load is desired we invoke the load function as well
    if ( loadImmediately )
    {
        success = LoadPlugin( pluginMetaData      ,
                              *pluginGroup        ,
                              groupName           ,
                              registerImmediately );
    }

    return success;
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::AddPluginMetaData( const CIPluginMetaData& pluginMetaData ,
                                   const CString& groupName               ,
                                   bool loadImmediately                   ,
                                   bool registerImmediately               )
{GUCEF_TRACE;

    bool success = true;

    // Create a copy which plugin control will own
    TPluginMetaDataStoragePtr pluginMetaDataCopy( CPluginMetaData::CreateSharedObjWithParam( pluginMetaData ) );    
    if ( pluginMetaDataCopy.IsNULL() )
        return false;

    // set the load immediately flag which may differ from what the original meta-data had
    pluginMetaDataCopy->SetLoadImmediately( loadImmediately );

    // Now call our internal implementation for the rest of the work
    return AddPluginMetaDataImpl( pluginMetaDataCopy  ,
                                  groupName           ,
                                  loadImmediately     ,
                                  registerImmediately );
}

/*-------------------------------------------------------------------------*/

bool 
CPluginControl::AdjustGroupPriority( const CString& groupName , UInt32 newPriority )
{GUCEF_TRACE;
    
    MT::CObjectScopeLock lock( this );
    
    TStringToPluginGroupMap::iterator i = m_pluginGroupsByName.find( groupName );
    if ( i != m_pluginGroupsByName.end() )
    {
        TPluginGroupPtr& pluginGroup = (*i).second;
        UInt32 oldPriority = pluginGroup->GetGroupPriority();
    
        // Remove the group from the old priority level
        m_pluginGroupsByPriority[ oldPriority ].erase( groupName );
    
        // Set the new priority level
        pluginGroup->SetGroupPriority( newPriority );
    
        // Add the group to the new priority level
        m_pluginGroupsByPriority[ newPriority ][ groupName ] = pluginGroup;
    
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Adjusted plugin group priority for group \"" + groupName + "\" from " + ToString( oldPriority ) + " to " + ToString( newPriority ) );
        return true;
    }
    
    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "PluginControl: There is no group with name: " + groupName );
    return false;    
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::SaveConfig( CDataNode& tree ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< TPluginMetaDataStoragePtr, gucef_allocator< TPluginMetaDataStoragePtr > >  TPluginLoadEntryVector;
typedef GUCEF::map< CString, TPluginLoadEntryVector >                                             TStringToPluginLoadEntryVectorMap;
typedef GUCEF::map< UInt32, TStringToPluginLoadEntryVectorMap >                                   TPluginLoadEntryMap;

/*-------------------------------------------------------------------------*/

bool
CPluginControl::LoadConfig( const CDataNode& treeroot )
{GUCEF_TRACE;

    TPluginLoadEntryMap pluginLoadEntries;
    bool errorOccured = false;

    // first parse relevant information from the config
    // Note that group loading priority is determined by order of appearance in the config unless otherwise specified in the config

    UInt32 groupPriorityPerConfigOrder = 1;
    CDataNode::TConstDataNodeSet pluginGroupNodes( treeroot.FindChildrenOfType( "PluginGroup", true ) );
    CDataNode::TConstDataNodeSet::iterator i = pluginGroupNodes.begin();
    while ( i != pluginGroupNodes.end() )
    {
        const CDataNode* groupNode = (*i);

        bool loadPlugins = groupNode->GetAttributeValueOrChildValueByName( "LoadImmediately" ).AsBool( false, true );
        CString groupName = groupNode->GetAttributeValueOrChildValueByName( "GroupName" ).AsString( CString::Empty, true );
        UInt32 groupPriority = groupNode->GetAttributeValueOrChildValueByName( "GroupPriority" ).AsUInt32( groupPriorityPerConfigOrder, true );

        CDataNode::TConstDataNodeSet pluginMetaDataNodes( groupNode->FindChildrenOfType( "PluginMetaData", true ) );
        CDataNode::TConstDataNodeSet::iterator n = pluginMetaDataNodes.begin();
        while ( n != pluginMetaDataNodes.end() )
        {
            // Check for per-plugin override of load settings, overriding group default
            bool loadImmediately = (*n )->GetAttributeValueOrChildValueByName( "LoadImmediately" ).AsBool( loadPlugins, true );

            // Since we have to report whether loading the config settings went ok
            // Check to see if allow the loading of this plugin to fail and still report success
            bool loadFailAllowed = (*n )->GetAttributeValueOrChildValueByName( "LoadFailAllowed" ).AsBool( false, true );

            TPluginMetaDataStoragePtr metaData( CPluginMetaData::CreateSharedObj() );
            if ( metaData->LoadConfig( *(*n ) ) )
            {
                TStringToPluginLoadEntryVectorMap& groupsAtPriority = pluginLoadEntries[ groupPriority ];                
                TPluginLoadEntryVector& entriesOfGroup = groupsAtPriority[ groupName ];

                metaData->SetLoadImmediately( loadImmediately );
                metaData->SetLoadFailAllowed( loadFailAllowed );
                
                entriesOfGroup.push_back( metaData );
            }
            else
            {
                if ( !loadFailAllowed )
                {
                    errorOccured = true;
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginControl: LoadConfig failed for the given plugin meta data" );
                }
                else
                {
                    GUCEF_LOG( LOGLEVEL_NORMAL, "PluginControl: LoadConfig failed for the given plugin meta data. LoadFailAllowed=True" );
                }
            }
            ++n;
        }

        ++i;
        ++groupPriorityPerConfigOrder;
    }

    MT::CObjectScopeLock lock( this );

    // Due the the map ordering we already have all the groups in the correct order of priority
    // Begin by adding the meta-data into our plugin meta-data registry plus create any groups that dont exist yet
    TPluginLoadEntryMap::iterator m = pluginLoadEntries.begin();
    while ( m != pluginLoadEntries.end() )
    {
        UInt32 priorityLevel = (*m).first;
        TStringToPluginLoadEntryVectorMap& groupsAtPriority = (*m).second;

        TStringToPluginLoadEntryVectorMap::iterator g = groupsAtPriority.begin();
        while ( g != groupsAtPriority.end() )
        {
            const CString groupName = (*g).first;
            TPluginGroupPtr pluginGroup = GetOrCreatePluginGroup( groupName, priorityLevel );

            if ( !pluginGroup.IsNULL() )
            {
                if ( pluginGroup->GetGroupPriority() != priorityLevel )
                    if ( !AdjustGroupPriority( groupName, priorityLevel ) )
                        errorOccured = true;

                TPluginLoadEntryVector& entriesOfGroup = (*g).second;
                TPluginLoadEntryVector::iterator e = entriesOfGroup.begin();
                while ( e != entriesOfGroup.end() )
                {
                    TPluginMetaDataStoragePtr& metaData = (*e);

                    if ( AddPluginMetaDataImpl( metaData, groupName, false, false ) )
                    {
                        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "PluginControl: Loaded meta-data for plugin in group \"" + groupName + "\" with filename: " + metaData->GetModuleFilename() );
                    }
                    else
                    {
                        if ( !metaData->GetLoadFailAllowed() )
                        {
                            errorOccured = true;
                            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginControl: Failed to add plugin meta-data" );
                        }
                        else
                        {
                            GUCEF_LOG( LOGLEVEL_NORMAL, "PluginControl: Failed to add plugin meta-data. LoadFailAllowed=True" );
                        }
                    }
                    ++e;
                }
            }
            else
            {
                errorOccured = true;
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginControl: Failed to create plugin group with name \"" + groupName + "\" and priority " + ToString( priorityLevel ) );                
            }

            ++g;
        }
        ++m;
    }

    // Next we load the modules flagged as needing to be loaded right away vs merely being meta-data definitions
    // We will also register them but note that we specify that we only want to load the ones that have the 
    // load immediatly flag set. Those that dont will remain just meta-data for now
    if ( !LoadAll( true, true ) )
        errorOccured = true;

    return !errorOccured;
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::ListModuleForLinkBack( const CString& moduleName )
{GUCEF_TRACE;

    if ( moduleName.IsNULLOrEmpty() )
        return false;

    m_listedLinkBackModules.insert( moduleName );
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Module \"" + moduleName + "\" registered as a LinkBack module" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPluginControl::UnlistModuleForLinkBack( const CString& moduleName )
{GUCEF_TRACE;

    if ( moduleName.IsNULLOrEmpty() )
        return false;

    m_listedLinkBackModules.erase( moduleName );
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Module \"" + moduleName + "\" unregistered as a LinkBack module, checking for dependent generic plugins to unload" );

    // Collect all plugins that declared a dependency on this module.
    // We gather them first so we can safely unload without iterator invalidation.
    typedef GUCEF::vector< std::pair< TPluginPtr, CString > > TPluginGroupPairVector;
    TPluginGroupPairVector toUnload;

    TStringToPluginGroupMap::iterator gi = m_pluginGroupsByName.begin();
    while ( gi != m_pluginGroupsByName.end() )
    {
        const CString& groupName = (*gi).first;
        CPluginGroup::TPluginSet& plugins = (*gi).second->GetPlugins();
        CPluginGroup::TPluginSet::iterator pi = plugins.begin();
        while ( pi != plugins.end() )
        {
            TPluginMetaDataPtr meta = (*pi)->GetMetaData();
            if ( !meta.IsNULL() )
            {
                CString linkBacks = meta->GetAllLinkBackModules();
                if ( !linkBacks.IsNULLOrEmpty() )
                {
                    CString::StringSet declaredModules = linkBacks.ParseUniqueElements( ',', false );
                    if ( declaredModules.find( moduleName ) != declaredModules.end() )
                    {
                        toUnload.push_back( std::make_pair( *pi, groupName ) );
                    }
                }
            }
            ++pi;
        }
        ++gi;
    }

    if ( toUnload.empty() )
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: No generic plugins dependent on LinkBack module \"" + moduleName + "\" found" );
        return true;
    }

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Unloading " + ToString( (UInt32) toUnload.size() ) + " generic plugin(s) dependent on LinkBack module \"" + moduleName + "\"" );

    UInt32 unloadCount = 0;
    TPluginGroupPairVector::iterator ui = toUnload.begin();
    while ( ui != toUnload.end() )
    {
        TStringToPluginGroupMap::iterator gi2 = m_pluginGroupsByName.find( (*ui).second );
        if ( gi2 != m_pluginGroupsByName.end() )
        {
            if ( UnloadPlugin( (*ui).first, *(*gi2).second, (*ui).second ) )
                ++unloadCount;
        }
        ++ui;
    }

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginControl: Unloaded " + ToString( unloadCount ) + " generic plugin(s) dependent on LinkBack module \"" + moduleName + "\"" );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPluginControl::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::CORE::CPluginControl";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
