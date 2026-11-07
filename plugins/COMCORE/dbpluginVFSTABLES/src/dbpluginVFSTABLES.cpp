/*
 *  dbpluginVFSTABLES: GUCEF database backend over table files in the VFS
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef DBPLUGIN_VFSTABLES_CVFSTABLESDATABASECLIENT_H
#include "dbpluginVFSTABLES_CVfsTablesDatabaseClient.h"
#define DBPLUGIN_VFSTABLES_CVFSTABLESDATABASECLIENT_H
#endif /* DBPLUGIN_VFSTABLES_CVFSTABLESDATABASECLIENT_H ? */

#include "dbpluginVFSTABLES.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

GUCEF::DBPLUGIN::VFSTABLES::TVfsTablesDatabaseClientFactory g_databaseClientFactory;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF::CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( GUCEF::CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    using namespace GUCEF;
    COMCORE::CDatabaseClientFactory& factory = COMCORE::CComCoreGlobal::Instance()->GetDatabaseClientFactory();
    if ( factory.IsConstructible( DBPLUGIN::VFSTABLES::CVfsTablesDatabaseClient::BackendName ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "GUCEFPlugin_Load: a database backend named \"" <<
                         DBPLUGIN::VFSTABLES::CVfsTablesDatabaseClient::BackendName << "\" is registered already" );
        return 0;
    }
    factory.RegisterConcreteFactory( DBPLUGIN::VFSTABLES::CVfsTablesDatabaseClient::BackendName, &g_databaseClientFactory );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "GUCEFPlugin_Load: database backend \"" << DBPLUGIN::VFSTABLES::CVfsTablesDatabaseClient::BackendName << "\" registered" );
    return 1;
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    using namespace GUCEF;

    // a generic plugin cannot refuse its unload: the application closes its clients first, what is still alive is named
    CORE::CString::StringVector live;
    DBPLUGIN::VFSTABLES::CVfsTablesDatabaseClient::GetLiveObjects( live );
    for ( UInt32 i=0; i<(UInt32) live.size(); ++i )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "GUCEFPlugin_Unload: dbpluginVFSTABLES is unloaded while a " << live[ i ] << " is alive" );

    COMCORE::CComCoreGlobal* comCoreGlobal = COMCORE::CComCoreGlobal::Instance();
    if ( GUCEF_NULL != comCoreGlobal && comCoreGlobal->GetDatabaseClientFactory().IsConstructible( DBPLUGIN::VFSTABLES::CVfsTablesDatabaseClient::BackendName ) )
        comCoreGlobal->GetDatabaseClientFactory().UnregisterConcreteFactory( DBPLUGIN::VFSTABLES::CVfsTablesDatabaseClient::BackendName );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "GUCEFPlugin_Unload: database backend \"" << DBPLUGIN::VFSTABLES::CVfsTablesDatabaseClient::BackendName << "\" unregistered" );
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetVersion( GUCEF::CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL != versionInfo )
    {
        versionInfo->major = 1;
        versionInfo->minor = 0;
        versionInfo->patch = 0;
        versionInfo->release = 0;
    }
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, Apache License v2";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Generic GUCEF plugin for COMCORE providing the vfstables database backend: partitioned tables as files in the VFS, read and written through the table codecs";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetLinkBackModuleDependencies( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefCORE,gucefCOMCORE,gucefVFS";
}

/*-------------------------------------------------------------------------*/
