/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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
#include <algorithm>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_GENERICPLUGINAPI_H
#include "gucefCORE_GenericPluginAPI.h"
#define GUCEF_CORE_GENERICPLUGINAPI_H
#endif /* GUCEF_CORE_GENERICPLUGINAPI_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CPLUGINMETADATA_H
#include "gucefCORE_CPluginMetaData.h"
#define GUCEF_CORE_CPLUGINMETADATA_H
#endif /* GUCEF_CORE_CPLUGINMETADATA_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#include "gucefASSETSTORE_CAssetStoreRegistry.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */

#include "gucefASSETSTORE_CAssetStorePluginLoader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetStorePluginLoader::StorePluginDependencyName = "gucefASSETSTORE";
const CORE::CString CAssetStorePluginLoader::PluginGroupName = "AssetStorePrivate";

/*-------------------------------------------------------------------------*/

namespace {

/** The host of the system: the dynamic module loading of the platform to look, the plugin control of gucefCORE to load */
class CSystemPluginHost : public CIAssetStorePluginHost
{
    public:

    virtual TAssetStorePluginKind InspectModule( const CORE::CString& modulePath, CORE::CString& reason ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        void* module = CORE::LoadModuleDynamicly( modulePath, reason );
        if ( GUCEF_NULL == module )
        {
            if ( reason.IsNULLOrEmpty() )
                reason = "the library cannot be loaded";
            return ASSETSTORE_PLUGINKIND_BROKEN;
        }
        TAssetStorePluginKind kind = ASSETSTORE_PLUGINKIND_NOT_A_PLUGIN;
        const CORE::TAnyPointer entry = CORE::GetFunctionAddress( module, "GUCEFPlugin_GetLinkBackModuleDependencies", 0 );
        if ( GUCEF_NULL != entry.funcPtr )
        {
            kind = ASSETSTORE_PLUGINKIND_OTHER_PLUGIN;
            const char* dependencies = reinterpret_cast< CORE::TGUCEFGENERICPLUGFPTR_GetLinkBackModuleDependencies >( entry.funcPtr )();
            if ( GUCEF_NULL != dependencies && GUCEF_NULL != strstr( dependencies, CAssetStorePluginLoader::StorePluginDependencyName.C_String() ) )
                kind = ASSETSTORE_PLUGINKIND_STORE_PLUGIN;
        }
        CORE::UnloadModuleDynamicly( module );
        return kind;
    }

    virtual bool LoadPlugin( const CORE::CString& directory, const CORE::CString& moduleFileName, CORE::CString& reason ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CORE::CPluginMetaData metaData;
        metaData.SetPluginType( "GucefGenericPlugin" );
        metaData.SetFullModulePath( directory );
        metaData.SetModuleFilename( moduleFileName );
        metaData.SetLoadFailAllowed( true );
        if ( !CORE::CCoreGlobal::Instance()->GetPluginControl().AddPluginMetaData( metaData, CAssetStorePluginLoader::PluginGroupName, true ) )
        {
            reason = "the plugin did not load or did not start (the log says why)";
            return false;
        }
        return true;
    }
};

bool
HasLibraryExtension( const CORE::CString& fileName )
{GUCEF_TRACE;

    const CORE::CString extension = CORE::ExtractFileExtention( fileName ).Lowercase();
    return "so" == extension || "dll" == extension || "dylib" == extension;
}

}

/*-------------------------------------------------------------------------*/

CAssetStorePluginLoader::CAssetStorePluginLoader( CIAssetStorePluginHost* host )
    : m_host( GUCEF_NULL != host ? host : &GetSystemHost() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStorePluginLoader::~CAssetStorePluginLoader()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIAssetStorePluginHost&
CAssetStorePluginLoader::GetHost( void )
{GUCEF_TRACE;

    return *m_host;
}

/*-------------------------------------------------------------------------*/

CIAssetStorePluginHost&
CAssetStorePluginLoader::GetSystemHost( void )
{GUCEF_TRACE;

    static CSystemPluginHost systemHost;
    return systemHost;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStorePluginLoader::LoadFromDirectory( const CORE::CString& directory, CAssetStorePluginLoadReport& report )
{GUCEF_TRACE;

    report.Clear();
    report.directory = directory;
    report.directoryGiven = !directory.IsNULLOrEmpty();
    if ( !report.directoryGiven )
        return false;
    const CORE::CString resolved = CORE::ResolveVars( directory );
    report.directoryFound = CORE::DirExists( resolved );
    if ( !report.directoryFound )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CAssetStorePluginLoader: The folder of private store plugins does not exist: " << directory );
        return false;
    }

    // the libraries of the folder, in the order of their names
    CORE::CString::StringVector files;
    struct CORE::SDI_Data* did = CORE::DI_First_Dir_Entry( resolved.C_String() );
    if ( GUCEF_NULL != did )
    {
        do
        {
            if ( 1 == CORE::DI_Is_It_A_File( did ) )
            {
                const CORE::CString fileName = CORE::DI_Name( did );
                if ( HasLibraryExtension( fileName ) )
                    files.push_back( fileName );
            }
        }
        while ( 0 != CORE::DI_Next_Dir_Entry( did ) );
        CORE::DI_Cleanup( did );
    }
    std::sort( files.begin(), files.end() );
    report.libraryCount = (UInt32) files.size();

    CORE::CString::StringVector before;
    CAssetStoreGlobal::Instance()->GetRegistry().GetProviderIds( before );

    for ( size_t i=0; i<files.size(); ++i )
    {
        CORE::CString reason;
        const TAssetStorePluginKind kind = m_host->InspectModule( CORE::CombinePath( resolved, files[ i ] ), reason );
        switch ( kind )
        {
            case ASSETSTORE_PLUGINKIND_STORE_PLUGIN:
            {
                if ( m_host->LoadPlugin( resolved, files[ i ], reason ) )
                {
                    report.loaded.push_back( files[ i ] );
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CAssetStorePluginLoader: Loaded the asset store plugin " << files[ i ] );
                }
                else
                {
                    report.refused.push_back( files[ i ] + ": " + reason );
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CAssetStorePluginLoader: Refused the asset store plugin " << files[ i ] << ": " << reason );
                }
                break;
            }
            case ASSETSTORE_PLUGINKIND_BROKEN:
            {
                report.refused.push_back( files[ i ] + ": " + reason );
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CAssetStorePluginLoader: Refused the library " << files[ i ] << ": " << reason );
                break;
            }
            default:
            {
                report.ignored.push_back( files[ i ] );
                GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CAssetStorePluginLoader: Left the library " << files[ i ] << " alone, it is no asset store plugin" );
                break;
            }
        }
    }

    // the stores the plugins registered
    CORE::CString::StringVector after;
    CAssetStoreGlobal::Instance()->GetRegistry().GetProviderIds( after );
    for ( size_t i=0; i<after.size(); ++i )
        if ( std::find( before.begin(), before.end(), after[ i ] ) == before.end() )
            report.stores.push_back( after[ i ] );
    if ( !report.loaded.empty() && report.stores.empty() )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CAssetStorePluginLoader: The asset store plugins of " << directory << " loaded but registered no new store" );
    return report.IsClean();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
