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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */ 

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"                  /* node for building data trees */
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#include "gucefCORE_CGloballyConfigurable.h"
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"       /* DStore codec registry */
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"        
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"           /* C++ string utils */
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVSTRUTILS_H
#include "dvstrutils.h"
#define GUCEF_CORE_DVSTRUTILS_H
#endif /* GUCEF_CORE_DVSTRUTILS_H ? */

#ifndef GUCEF_CORE_CINIPARSER_H
#include "gucefCORE_CIniParser.h"
#define GUCEF_CORE_CINIPARSER_H
#endif /* GUCEF_CORE_CINIPARSER_H ? */

#include "CConfigStore.h"       /* definition of the class implemented here */

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

const CEvent CConfigStore::GlobalBootstrapConfigLoadStartingEvent = "GUCEF::CORE::CConfigStore::GlobalBootstrapConfigLoadStartingEvent";
const CEvent CConfigStore::GlobalBootstrapConfigLoadCompletedEvent = "GUCEF::CORE::CConfigStore::GlobalBootstrapConfigLoadCompletedEvent";
const CEvent CConfigStore::GlobalBootstrapConfigLoadFailedEvent = "GUCEF::CORE::CConfigStore::GlobalBootstrapConfigLoadFailedEvent";
const CEvent CConfigStore::GlobalConfigLoadStartingEvent = "GUCEF::CORE::CConfigStore::GlobalConfigLoadStartingEvent";
const CEvent CConfigStore::GlobalConfigLoadCompletedEvent = "GUCEF::CORE::CConfigStore::GlobalConfigLoadCompletedEvent";
const CEvent CConfigStore::GlobalConfigLoadFailedEvent = "GUCEF::CORE::CConfigStore::GlobalConfigLoadFailedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CConfigStore::RegisterEvents( void )
{GUCEF_TRACE;

    GlobalBootstrapConfigLoadStartingEvent.Initialize();
    GlobalBootstrapConfigLoadCompletedEvent.Initialize();
    GlobalBootstrapConfigLoadFailedEvent.Initialize();
    GlobalConfigLoadStartingEvent.Initialize();
    GlobalConfigLoadCompletedEvent.Initialize();
    GlobalConfigLoadFailedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CConfigStore::CConfigStore( void )
    : CTSGNotifier()
    , _codectype()
    , m_bootstrapConfigFile()
    , m_consolidatedConfigFile()
    , m_configfiles()
    , m_configureables()
    , m_newConfigureables()
    , m_optionalConfigurables()
    , m_configurableScopes()
    , m_configurableScopeThreadId( 0 )
    , m_isBusyLoadingConfig( false )
    , m_globalConfigIsLoaded( false )
    , m_globalBootstrapConfigIsLoaded( false )
    , m_configLocation()
    , m_configFileName()
    , m_defaultConfigFile()
    , m_usingManagedConfig( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CConfigStore::~CConfigStore()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CConfigStore::SetBootstrapConfigFile( const CString& filepath )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_bootstrapConfigFile = filepath;
}

/*-------------------------------------------------------------------------*/

CString
CConfigStore::GetBootstrapConfigFile( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return m_bootstrapConfigFile;
}

/*-------------------------------------------------------------------------*/

void
CConfigStore::SetConfigFile( const CString& filepath ,
                             bool isAddative         )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_consolidatedConfigFile = filepath;
    m_usingManagedConfig = false;       // the application says which config it uses
    if ( !isAddative )
        m_configfiles.clear();
    m_configfiles.push_back( RelativePath( filepath ) );
}

/*-------------------------------------------------------------------------*/

CString
CConfigStore::GetConfigFile( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    if ( !m_consolidatedConfigFile.IsNULLOrEmpty() )
        return m_consolidatedConfigFile;
    if ( m_configfiles.empty() )
        return CString::Empty;
    return m_configfiles.back();
}

/*-------------------------------------------------------------------------*/

CString
CConfigStore::GetConsolidatedConfigFile( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return m_consolidatedConfigFile;
}

/*-------------------------------------------------------------------------*/

CString
CConfigStore::GetConsolidatedConfigFileCodecType( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    CString codecType = ExtractFileExtention( m_consolidatedConfigFile );
    if ( codecType.IsNULLOrEmpty() )
        codecType = _codectype;
    return codecType;
}

/*-------------------------------------------------------------------------*/

void
CConfigStore::SetConfigFiles( const CString::StringVector& filepaths ,
                              bool isAddative                        )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_usingManagedConfig = false;       // the application says which config it uses
    if ( !isAddative )
        m_configfiles.clear();

    CString::StringVector::const_iterator i = filepaths.begin();
    while ( i != filepaths.end() )
    {
        m_configfiles.push_back( RelativePath( (*i) ) );
        ++i;
    }
    
    if ( m_consolidatedConfigFile.IsNULLOrEmpty() && !m_configfiles.empty() )
    {       
        // Generated a default consolidated config file name
        CString firstCfg = (*m_configfiles.begin());
        m_consolidatedConfigFile = StripFileExtention( firstCfg ) + "_consolidated." + ExtractFileExtention( firstCfg );
    }
}

/*-------------------------------------------------------------------------*/

CString::StringVector
CConfigStore::GetConfigFiles( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return m_configfiles;
}

/*-------------------------------------------------------------------------*/

void
CConfigStore::Register( CGloballyConfigurable* configobj )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    if ( !m_isBusyLoadingConfig )
        m_configureables.insert( configobj );
    else
        m_newConfigureables.insert( configobj );

    // A configurable that comes along with something that may fail is one that may fail as well
    if ( !m_configurableScopes.empty() && 0 != m_configurableScopes.back() && m_configurableScopeThreadId == MT::GetCurrentTaskID() )
        m_optionalConfigurables.insert( configobj );
}

/*-------------------------------------------------------------------------*/

void
CConfigStore::BeginConfigurableScope( bool configurablesAreOptional )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    if ( m_configurableScopes.empty() )
        m_configurableScopeThreadId = MT::GetCurrentTaskID();
    m_configurableScopes.push_back( configurablesAreOptional ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

void
CConfigStore::EndConfigurableScope( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    if ( !m_configurableScopes.empty() )
        m_configurableScopes.pop_back();
}

/*-------------------------------------------------------------------------*/

bool
CConfigStore::Unregister( CGloballyConfigurable* configobj )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    
    m_optionalConfigurables.erase( configobj );
    if ( m_isBusyLoadingConfig )
        return false;

    m_configureables.erase( configobj );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CConfigStore::SaveConsolidatedConfig( const CString& name ,
                                      bool preserve       )
{GUCEF_TRACE;
        
    MT::CObjectScopeLock lock( this );

    CString codecType = GetConsolidatedConfigFileCodecType();
    
    /*
        *      If the preserve flag is set then the old data tree
        *      will be loaded first to prevent any data loss.
        *      This basically allows you to add and/or modify data
        *      but not remove items from the tree unless you do so on purpose.
        */
    CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecType, codec, false ) || codec.IsNULL() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ConfigStore:SaveConsolidatedConfig: No codec \"" + codecType + "\" to write the config with" );
        return false;
    }

    /*
     *      If the preserve flag is set then the config as it is in the file is the starting point,
     *      so what the configurables do not write themselves is kept.
     *      This basically allows you to add and/or modify data
     *      but not remove items from the tree unless you do so on purpose.
     */
    CDataNode rootnode( name );
    if ( preserve )
    {
        CDataNode oldtree;
        if ( codec->BuildDataTree( &oldtree, m_consolidatedConfigFile ) )
            rootnode = oldtree;
    }

    TConfigurableSet::iterator i = m_configureables.begin();
    while ( i != m_configureables.end() )
    {
        // a configurable that cannot save (or has nothing to save) does not stop the others from being saved
        if ( !(*i)->SaveConfig( rootnode ) )
        {
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "ConfigStore:SaveConsolidatedConfig: A configurable of type \"" + (*i)->GetClassTypeName() + "\" did not save its config" );
        }
        ++i;
    }

    if ( codec->StoreDataTree( &rootnode, m_consolidatedConfigFile ) )
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ConfigStore:SaveConsolidatedConfig: Wrote the config using codec \"" + codecType + "\" to: " + m_consolidatedConfigFile );
        return true;
    }
    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ConfigStore:SaveConsolidatedConfig: Failed to write the config using codec \"" + codecType + "\" to: " + m_consolidatedConfigFile );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CConfigStore::SaveConsolidatedConfig( const CDataNode& newCfg )
{GUCEF_TRACE;

    try
    {
        CString codecType = GetConsolidatedConfigFileCodecType();
        CString consolidatedConfigFilePath = GetConsolidatedConfigFile();
        
        CDStoreCodecRegistry::TDStoreCodecPtr codec;
        if ( CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecType, codec, false ) && !codec.IsNULL() )
        {
            if ( codec->StoreDataTree( &newCfg                     ,
                                       consolidatedConfigFilePath  ) )
            {
                GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ConfigStore:SaveConsolidatedConfig: Wrote consolidated config using codec \"" + codecType + "\" to: " + consolidatedConfigFilePath );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ConfigStore:SaveConsolidatedConfig: Failed to write consolidated config using codec \"" + codecType + "\" to: " + consolidatedConfigFilePath );
            }
        }
        else
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ConfigStore:SaveConsolidatedConfig: Failed obtain codec \"" + codecType + "\"" );
        }
    }
    catch ( std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, CString( "ConfigStore:SaveConsolidatedConfig: Exception occured: " ) + e.what() );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CConfigStore::ApplyConsolidatedConfig( const CDataNode& newCfg )
{GUCEF_TRACE;
    
    // Before we do anything drastic make sure our logs are flushed to a known point to help diagnostics
    CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();

    MT::CObjectScopeLock lock( this );
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ConfigStore:ApplyConsolidatedConfig: commencing" );
    
    bool success = true;
    try
    {
        m_globalBootstrapConfigIsLoaded = false;
        m_globalConfigIsLoaded = false;
        if ( !NotifyObservers( GlobalConfigLoadStartingEvent ) ) 
            return false;
        
        {
            success = ApplyConfigData( newCfg, false, !m_bootstrapConfigFile.IsNULLOrEmpty() );
        }

        if ( success )
        {
            m_globalBootstrapConfigIsLoaded = true;
            m_globalConfigIsLoaded = true;
            NotifyObservers( GlobalConfigLoadCompletedEvent );
        }
        else
            NotifyObservers( GlobalConfigLoadFailedEvent );
    
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ConfigStore:ApplyConsolidatedConfig: completed" );

        lock.EarlyUnlock();

        // After anything drastic make sure our logs are flushed to a known point to help diagnostics        
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
    }
    catch ( std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, CString( "ConfigStore:SaveConsolidatedConfig: Exception occured: " ) + e.what() );
        success = false;
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CConfigStore::LoadConfig( CDataNode* loadedConfig )
{GUCEF_TRACE;

    CDataNode configTmp;
    CDataNode& config = GUCEF_NULL == loadedConfig ? configTmp : *loadedConfig; 
    bool totalSuccess = true;

    {
        MT::CObjectScopeLock lock( this );

        // the config location is what this bootstrap config says, if anything
        m_configLocation.Clear();
        m_configFileName.Clear();
        m_defaultConfigFile.Clear();
        
        // Do we have a bootstrap log? If so apply that first
        if ( !m_bootstrapConfigFile.IsNULLOrEmpty() )
        {
            m_globalConfigIsLoaded = false;
            m_globalBootstrapConfigIsLoaded = false;
            
            if ( !NotifyObservers( GlobalBootstrapConfigLoadStartingEvent ) ) 
                return false;

            totalSuccess = LoadConfigData( m_bootstrapConfigFile, config );
    
            // IF we have a bootstrap config it MUST load successfully
            if ( !totalSuccess )
            {
                NotifyObservers( GlobalBootstrapConfigLoadFailedEvent );
                return false;
            }

            totalSuccess = ApplyConfigData( config, true, true );
            ReadConfigLocation( config );

            if ( totalSuccess )
            {
                m_globalBootstrapConfigIsLoaded = true;
                NotifyObservers( GlobalBootstrapConfigLoadCompletedEvent );
            }
            else
            {
                NotifyObservers( GlobalBootstrapConfigLoadFailedEvent );
            }
        }
        else
        {
            m_globalBootstrapConfigIsLoaded = true;
        }
    }
    {
        MT::CObjectScopeLock lock( this );
    
        if ( !NotifyObservers( GlobalConfigLoadStartingEvent ) ) 
            return false;

        // the config of the user in the config location, when the bootstrap config names one and the application gave none
        if ( !m_configLocation.IsNULLOrEmpty() && ( m_configfiles.empty() || m_usingManagedConfig ) )
        {
            m_usingManagedConfig = true;
            m_configfiles.clear();
            m_configfiles.push_back( GetManagedConfigFile() );
            m_consolidatedConfigFile = GetManagedConfigFile();
            PrepareManagedConfigFile();
        }
        const CDataNode bootstrapPart( config );

        // Load and overlay configs in the order given
        CString::StringVector::const_iterator i = m_configfiles.begin();
        while ( i != m_configfiles.end() )
        {
            totalSuccess = LoadConfigData( (*i), config ) && totalSuccess;
            ++i;
        }

        // a config of the user that cannot be read is made again from the default config
        if ( !totalSuccess && m_usingManagedConfig && RestoreManagedConfigFile() )
        {
            config = bootstrapPart;
            totalSuccess = LoadConfigData( GetManagedConfigFile(), config );
        }
    
        // If multiple config files are given they MUST all load successfully
        if ( !totalSuccess )
        {
            NotifyObservers( GlobalConfigLoadFailedEvent );
            return false;
        }

        // Now we actually apply the config to globally linked configurables
        totalSuccess = ApplyConfigData( config, false, !m_bootstrapConfigFile.IsNULLOrEmpty() );

        if ( totalSuccess )
        {
            m_globalConfigIsLoaded = true;
            NotifyObservers( GlobalConfigLoadCompletedEvent );
        }
        else
            NotifyObservers( GlobalConfigLoadFailedEvent );
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

void
CConfigStore::ReadConfigLocation( const CDataNode& bootstrapConfig )
{GUCEF_TRACE;

    const CDataNode* section = bootstrapConfig.FindChild( "ConfigStore", false );
    if ( GUCEF_NULL == section )
        return;

    // with their variables: they are resolved when used, and written back as they were
    m_configLocation = section->GetAttributeValueOrChildValueByName( "ConfigLocation" ).AsString( CString::Empty, false );
    m_configFileName = section->GetAttributeValueOrChildValueByName( "ConfigFileName" ).AsString( CString::Empty, false );
    m_defaultConfigFile = section->GetAttributeValueOrChildValueByName( "DefaultConfigFile" ).AsString( CString::Empty, false );
    if ( m_configFileName.IsNULLOrEmpty() && !m_defaultConfigFile.IsNULLOrEmpty() )
        m_configFileName = ExtractFilename( m_defaultConfigFile.ReplaceChar( '\\', '/' ) );
    if ( m_configFileName.IsNULLOrEmpty() )
        m_configLocation.Clear();      // without a file name there is nothing to manage

    if ( !m_configLocation.IsNULLOrEmpty() )
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ConfigStore: The config of the application is " << m_configFileName << " in " << m_configLocation
            << " (" << GetResolvedConfigLocation() << "), made from " << m_defaultConfigFile << " when it is not there" );
}

/*-------------------------------------------------------------------------*/

CString
CConfigStore::GetManagedConfigFile( void ) const
{GUCEF_TRACE;

    return CombinePath( GetResolvedConfigLocation(), m_configFileName );
}

/*-------------------------------------------------------------------------*/

bool
CConfigStore::PrepareManagedConfigFile( void )
{GUCEF_TRACE;

    const CString configFile = GetManagedConfigFile();
    if ( FileExists( configFile ) )
        return true;

    // the first start: the user gets a config of their own, made from the default config of the application
    const CString defaultConfigFile = RelativePath( m_defaultConfigFile );
    if ( defaultConfigFile.IsNULLOrEmpty() || !FileExists( defaultConfigFile ) )
    {
        GUCEF_WARNING_LOG( LOGLEVEL_IMPORTANT, "ConfigStore: There is no config in " << configFile << " and no default config to make it from ("
            << defaultConfigFile << ")" );
        return false;
    }
    if ( !CreateDirs( GetResolvedConfigLocation() ) || !CopyFile( defaultConfigFile, configFile, false ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ConfigStore: The config " << configFile << " could not be made from the default config " << defaultConfigFile );
        return false;
    }
    GUCEF_SYSTEM_LOG( LOGLEVEL_IMPORTANT, "ConfigStore: Made the config " << configFile << " from the default config " << defaultConfigFile );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CConfigStore::RestoreManagedConfigFile( void )
{GUCEF_TRACE;

    const CString configFile = GetManagedConfigFile();
    const CString defaultConfigFile = RelativePath( m_defaultConfigFile );
    if ( defaultConfigFile.IsNULLOrEmpty() || !FileExists( defaultConfigFile ) )
        return false;

    // the one that cannot be read is kept for whoever wants to see what was wrong with it
    if ( FileExists( configFile ) && !MoveFile( configFile, configFile + ".corrupt", true ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ConfigStore: The config " << configFile << " cannot be read and cannot be moved aside" );
        return false;
    }
    if ( !CopyFile( defaultConfigFile, configFile, true ) )
        return false;
    GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ConfigStore: The config " << configFile << " could not be read: it is kept as " << configFile
        << ".corrupt and made again from the default config " << defaultConfigFile );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CConfigStore::IsConfigLocationManaged( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return !m_configLocation.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CString
CConfigStore::GetConfigLocation( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return m_configLocation;
}

/*-------------------------------------------------------------------------*/

CString
CConfigStore::GetResolvedConfigLocation( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    if ( m_configLocation.IsNULLOrEmpty() )
        return CString::Empty;
    return RelativePath( m_configLocation );
}

/*-------------------------------------------------------------------------*/

bool
CConfigStore::SetConfigLocation( const CString& location )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    if ( m_configLocation.IsNULLOrEmpty() || location.IsNULLOrEmpty() || m_bootstrapConfigFile.IsNULLOrEmpty() )
        return false;
    CString resolved = RelativePath( location ).ReplaceChar( '\\', '/' );
    while ( resolved.Length() > 1 && '/' == resolved[ resolved.Length()-1 ] )
        resolved = resolved.CutChars( 1, false );
    if ( resolved == GetResolvedConfigLocation().ReplaceChar( '\\', '/' ) )
        return true;

    // the config as it is now goes along, one that is there already is kept aside
    const CString oldConfigFile = GetManagedConfigFile();
    const CString newConfigFile = CombinePath( resolved, m_configFileName );
    if ( !CreateDirs( resolved ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ConfigStore: The config location " << resolved << " cannot be made" );
        return false;
    }
    if ( FileExists( newConfigFile ) )
        MoveFile( newConfigFile, newConfigFile + ".previous", true );
    if ( FileExists( oldConfigFile ) && !CopyFile( oldConfigFile, newConfigFile, true ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ConfigStore: The config " << oldConfigFile << " cannot be copied to " << newConfigFile );
        return false;
    }

    // the bootstrap config remembers it, with a variable when the folder is in a directory that has one
    const CString stored = UnresolveVars( resolved );
    if ( !CIniParser::UpdateValueInIniFile( RelativePath( m_bootstrapConfigFile ), "ConfigStore", "ConfigLocation", stored ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ConfigStore: The config location could not be written to the bootstrap config " << m_bootstrapConfigFile );
        return false;
    }
    m_configLocation = stored;
    if ( m_usingManagedConfig )
    {
        m_configfiles.clear();
        m_configfiles.push_back( newConfigFile );
        m_consolidatedConfigFile = newConfigFile;
    }
    GUCEF_SYSTEM_LOG( LOGLEVEL_IMPORTANT, "ConfigStore: The config is now kept in " << stored << " (" << resolved << ")" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CConfigStore::LoadConfigData( const CString& cfgFilepath , 
                              CDataNode& loadedConfig    )
{GUCEF_TRACE;
                          
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "ConfigStore:LoadConfigData: Will attempt to load all config from file: " + cfgFilepath );

    if ( !FileExists( cfgFilepath ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ConfigStore:LoadConfigData: Failed to load config because the config file does not exist: " + cfgFilepath );
        return false;
    }

    CORE::CString codectype = Extract_File_Ext( cfgFilepath.C_String() );
    if ( codectype.IsNULLOrEmpty() )
    {
        codectype = _codectype;
    }
    GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "ConfigStore:LoadConfigData: Will try to use codec \"" + codectype + "\" for the config information in file " + cfgFilepath );

    try
    {
        CDStoreCodecRegistry::TDStoreCodecPtr codec;
        if ( CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codectype, codec, false ) && !codec.IsNULL() )
        {
            if ( codec->BuildDataTree( &loadedConfig ,
                                       cfgFilepath   ) )
            {
                    GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "ConfigStore:LoadConfigData: Used codec " + codectype + " to successfully build a config data tree from file " + cfgFilepath );
                    return true;
            }
            else
            {
                GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ConfigStore:LoadConfigData: Failed to build a config data tree using codec " + codectype + " from file " + cfgFilepath );
            }
        }
        else
        {
            GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ConfigStore:LoadConfigData: Failed to load config since no codec is available for representation \"" + codectype + "\" in file " + cfgFilepath );
        }
    }
    catch ( CDStoreCodecRegistry::EUnregisteredName& )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ConfigStore:LoadConfigData: Failed to retrieve codec " + codectype + " for the config information" );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CConfigStore::ApplyConfigData( const CDataNode& loadedConfig , 
                               bool isBootstrap              ,
                               bool hasBootstrap             )
{GUCEF_TRACE;

    m_isBusyLoadingConfig = true;
                        
    bool errorOccured = false;
    TConfigurableSet::iterator i = m_configureables.begin();
    while ( i != m_configureables.end() )
    {
        if ( !hasBootstrap || ( isBootstrap == (*i)->IsIncludedInGlobalBootstrapConfigLoad() ) )
        {
            if ( !(*i)->LoadConfig( loadedConfig ) )
            {
                if ( m_optionalConfigurables.find( *i ) == m_optionalConfigurables.end() )
                {
                    errorOccured = true;
                    GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ConfigStore:ApplyConfigData: Loading of config failed for a configureable with type name \"" << (*i)->GetClassTypeName() << "\"" );
                }
                else
                {
                    GUCEF_WARNING_LOG( LOGLEVEL_IMPORTANT, "ConfigStore:ApplyConfigData: Loading of config failed for an optional configureable (it came with a plugin that may fail) with type name \"" << (*i)->GetClassTypeName() << "\"" );
                }
            }
        }
        ++i;
    }

    // Loading of config can cause more configurables to register
    // We handle those after dealing with the initial set
    while ( !m_newConfigureables.empty() )
    {
        TConfigurableSet setCopy( m_newConfigureables );
        i = setCopy.begin();
        while ( i != setCopy.end() )
        {
            if ( !hasBootstrap || ( isBootstrap == (*i)->IsIncludedInGlobalBootstrapConfigLoad() ) )
            {
                if ( !(*i)->LoadConfig( loadedConfig ) )
                {
                    if ( m_optionalConfigurables.find( *i ) == m_optionalConfigurables.end() )
                    {
                        errorOccured = true;
                        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ConfigStore:ApplyConfigData: Loading of config failed for a late-addition configureable with type name \"" << (*i)->GetClassTypeName() << "\"" );
                    }
                    else
                    {
                        GUCEF_WARNING_LOG( LOGLEVEL_IMPORTANT, "ConfigStore:ApplyConfigData: Loading of config failed for an optional late-addition configureable (it came with a plugin that may fail) with type name \"" << (*i)->GetClassTypeName() << "\"" );
                    }
                }            
            }

            m_configureables.insert( (*i) );
            m_newConfigureables.erase( (*i) );
            ++i;
        }
    }

    m_isBusyLoadingConfig = false;
                        
    if ( !errorOccured )
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "ConfigStore:ApplyConfigData: Successfully loaded all config for all globally registered configurables" );
        return true;
    }
    else
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ConfigStore:ApplyConfigData: Failed loading config for all globally registered configurables" );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool 
CConfigStore::IsGlobalBootstrapConfigLoadInProgress( void ) const
{GUCEF_TRACE;

    return m_isBusyLoadingConfig && !m_globalBootstrapConfigIsLoaded;
}

/*-------------------------------------------------------------------------*/

bool 
CConfigStore::IsGlobalConfigLoadInProgress( void ) const
{GUCEF_TRACE;

    return m_isBusyLoadingConfig && !m_globalConfigIsLoaded;
}

/*-------------------------------------------------------------------------*/

bool
CConfigStore::IsGlobalConfigLoaded( void ) const
{GUCEF_TRACE;

    return m_globalConfigIsLoaded;
}

/*-------------------------------------------------------------------------*/
    
bool 
CConfigStore::IsGlobalBootstrapConfigIsLoaded( void ) const
{GUCEF_TRACE;

    return m_globalBootstrapConfigIsLoaded;
}

/*-------------------------------------------------------------------------*/

void
CConfigStore::SetCodec( const CString& codectype )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    _codectype = codectype;
}

/*-------------------------------------------------------------------------*/

CString
CConfigStore::GetCodec( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return _codectype;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
