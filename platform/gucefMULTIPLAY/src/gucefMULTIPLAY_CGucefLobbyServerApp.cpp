/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMULTIPLAY_CGucefLobbyServerApp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CGucefLobbyServerApp::SettingsNodeName = "GucefLobbyServer";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGucefLobbyServerApp::CGucefLobbyServerApp( void )
    : m_server()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGucefLobbyServerApp::~CGucefLobbyServerApp()
{GUCEF_TRACE;

    Stop();
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyServerApp::LoadConfigFile( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString resolvedPath = CORE::RelativePath( path );
    CORE::CString extension = CORE::ExtractFileExtention( resolvedPath ).Lowercase();
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( extension, codec, false ) || codec.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServerApp: There is no codec for \"" << extension << "\" files to read " << resolvedPath );
        return false;
    }
    CORE::CDataNode tree;
    if ( !codec->BuildDataTree( &tree, resolvedPath ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServerApp: Could not read the config file " << resolvedPath );
        return false;
    }
    return LoadConfig( tree );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyServerApp::LoadConfig( const CORE::CDataNode& tree )
{GUCEF_TRACE;

    const CORE::CDataNode* settingsNode = tree.Find( SettingsNodeName );
    if ( GUCEF_NULL == settingsNode )
        settingsNode = &tree;
    CGucefLobbyServerSettings settings;
    if ( !settings.LoadConfig( *settingsNode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServerApp: The settings are not valid" );
        return false;
    }
    if ( settings.node.listenEndpoints.IsEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServerApp: The settings name no endpoints to listen on" );
        return false;
    }
    m_server.SetSettings( settings );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyServerApp::Start( void )
{GUCEF_TRACE;

    return m_server.Start();
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyServerApp::Stop( void )
{GUCEF_TRACE;

    m_server.Stop();
}

/*-------------------------------------------------------------------------*/

int
CGucefLobbyServerApp::Run( int argc, char** argv )
{GUCEF_TRACE;

    if ( !Start() )
        return 1;
    int result = CORE::CCoreGlobal::Instance()->GetApplication().main( argc, argv, true );
    Stop();
    return result;
}

/*-------------------------------------------------------------------------*/

CGucefLobbyServer&
CGucefLobbyServerApp::GetServer( void )
{GUCEF_TRACE;

    return m_server;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
