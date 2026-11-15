/*
 *  gucefCOM: GUCEF module providing communication implementations
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

#include <stdlib.h>
#include <string.h>

#include "gucefCOM_CServiceAdminEndpoint.h"

#ifndef GUCEF_COM_CSERVICEADMINCONTRACTS_H
#include "gucefCOM_CServiceAdminContracts.h"
#define GUCEF_COM_CSERVICEADMINCONTRACTS_H
#endif /* GUCEF_COM_CSERVICEADMINCONTRACTS_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVERCALL_H
#include "gucefCOMCORE_CRpcServerCall.h"
#define GUCEF_COMCORE_CRPCSERVERCALL_H
#endif /* GUCEF_COMCORE_CRPCSERVERCALL_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVICESET_H
#include "gucefCOMCORE_CRpcServiceSet.h"
#define GUCEF_COMCORE_CRPCSERVICESET_H
#endif /* GUCEF_COMCORE_CRPCSERVICESET_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* const CServiceAdminEndpoint::TokenFileName = "admin.token";

/*-------------------------------------------------------------------------*/

namespace {

const UInt32 TokenSize = 32;

/** Lower case hexadecimal of bytes */
CORE::CString
AdminToHex( const UInt8* data, UInt32 size )
{GUCEF_TRACE;

    static const char digits[] = "0123456789abcdef";
    CORE::CString text;
    char pair[ 3 ];
    pair[ 2 ] = '\0';
    for ( UInt32 i=0; i<size; ++i )
    {
        pair[ 0 ] = digits[ ( data[ i ] >> 4 ) & 0x0F ];
        pair[ 1 ] = digits[ data[ i ] & 0x0F ];
        text += pair;
    }
    return text;
}

}

/*-------------------------------------------------------------------------*/

CServiceAdminEndpoint::CServiceAdminEndpoint( CIServiceAdminHandler& handler, const CORE::PulseGeneratorPtr& pulseGenerator )
    : m_handler( handler )
    , m_pulse( pulseGenerator )
    , m_host( GUCEF_NULL )
    , m_listener()
    , m_service()
    , m_commands()
    , m_tokenHex()
    , m_tokenFilePath()
    , m_boundPort( 0 )
    , m_refused( 0 )
{GUCEF_TRACE;

    m_service.BindMethod( "Run", GUCEF_NEW THandler( this, &CServiceAdminEndpoint::OnRun ) );
}

/*-------------------------------------------------------------------------*/

CServiceAdminEndpoint::~CServiceAdminEndpoint()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

void
CServiceAdminEndpoint::SetCommands( const CORE::CString::StringSet& commands )
{GUCEF_TRACE;

    m_commands = commands;
}

/*-------------------------------------------------------------------------*/

bool
CServiceAdminEndpoint::Open( const CORE::CString& stateDirectory, UInt16 port, CORE::CString& error )
{GUCEF_TRACE;

    if ( IsOpen() )
    {
        error = "the admin endpoint is open already";
        return false;
    }
    if ( stateDirectory.IsNULLOrEmpty() )
    {
        error = "no state directory for the token file";
        return false;
    }
    if ( !CORE::DirExists( stateDirectory ) && !CORE::CreateDirs( stateDirectory ) )
    {
        error = "the state directory " + stateDirectory + " cannot be made";
        return false;
    }

    COMCORE::CRpcContractPtr contract = CServiceAdminContracts::GetContract();
    if ( contract.IsNULL() )
    {
        error = "the contract gucef.service.admin cannot be built";
        return false;
    }

    // listen first: when the port is taken (another service runs) its token file stays as it is
    COMCORE::CIRpcTransportListenerPtr listener = COMCORE::CComCoreGlobal::Instance()->GetRpcTransportListenerFactory().Create( "tcp" );
    if ( listener.IsNULL() )
    {
        error = "there is no tcp RPC transport listener";
        return false;
    }
    const CORE::CString listenUri = "tcp://127.0.0.1:" + CORE::ToString( (UInt32) port );
    if ( !listener->SetPulseGenerator( m_pulse ) || !listener->Listen( CORE::CUri( listenUri ) ) )
    {
        error = "cannot listen on " + listenUri + " (is another service using the port?)";
        return false;
    }
    const CORE::CUri bound = listener->GetListenUri();
    const UInt16 boundPort = (UInt16) CORE::StringToUInt32( bound.GetAuthorityPort() );

    COMCORE::CRpcHostSettings hostSettings;
    hostSettings.maxSessions = 8;
    COMCORE::CRpcHost* host = GUCEF_NEW COMCORE::CRpcHost( m_pulse, hostSettings );
    if ( !host->GetServiceSet()->RegisterService( contract, CServiceAdminContracts::ServiceName, &m_service, COMCORE::RPC_EXECUTION_NOTIFICATION_THREAD ) ||
         !host->Start() || !host->AddListener( listener ) )
    {
        listener->StopListening();
        host->Stop();
        GUCEF_DELETE host;
        error = "the RPC host of the admin endpoint cannot be started";
        return false;
    }

    // a new token for this run: the file of an earlier run is replaced
    UInt8 token[ TokenSize ];
    if ( !CORE::CRandom::GetRandomBytes( token, TokenSize ) )
    {
        listener->StopListening();
        host->Stop();
        GUCEF_DELETE host;
        error = "no randomness for the admin token";
        return false;
    }
    const CORE::CString tokenHex = AdminToHex( token, TokenSize );
    memset( token, 0, TokenSize );

    const CORE::CString tokenPath = CORE::CombinePath( stateDirectory, TokenFileName );
    if ( CORE::FileExists( tokenPath ) && !CORE::DeleteFile( tokenPath ) )
    {
        listener->StopListening();
        host->Stop();
        GUCEF_DELETE host;
        error = "the token file " + tokenPath + " of an earlier run cannot be removed";
        return false;
    }
    CORE::CString problem;
    if ( !CORE::CreateOwnerOnlyFile( tokenPath, tokenHex.C_String(), tokenHex.ByteSize() - 1, problem ) )
    {
        listener->StopListening();
        host->Stop();
        GUCEF_DELETE host;
        error = "the token file " + tokenPath + " cannot be made: " + problem;
        return false;
    }

    m_host = host;
    m_listener = listener;
    m_tokenHex = tokenHex;
    m_tokenFilePath = tokenPath;
    m_boundPort = boundPort;
    m_refused = 0;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServiceAdminEndpoint: listening on 127.0.0.1:" << m_boundPort << ", token file " << m_tokenFilePath );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CServiceAdminEndpoint::Close( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_host )
        return;

    m_host->Stop();
    if ( !m_listener.IsNULL() )
        m_listener->StopListening();
    GUCEF_DELETE m_host;
    m_host = GUCEF_NULL;
    m_listener.Unlink();

    if ( !m_tokenFilePath.IsNULLOrEmpty() && CORE::FileExists( m_tokenFilePath ) && !CORE::DeleteFile( m_tokenFilePath ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ServiceAdminEndpoint: the token file " << m_tokenFilePath << " cannot be removed" );
    m_tokenHex.Clear();
    m_tokenFilePath.Clear();
    m_boundPort = 0;
}

/*-------------------------------------------------------------------------*/

bool
CServiceAdminEndpoint::IsOpen( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_host;
}

/*-------------------------------------------------------------------------*/

UInt16
CServiceAdminEndpoint::GetBoundPort( void ) const
{GUCEF_TRACE;

    return m_boundPort;
}

/*-------------------------------------------------------------------------*/

UInt32
CServiceAdminEndpoint::GetRefusedCount( void ) const
{GUCEF_TRACE;

    return m_refused;
}

/*-------------------------------------------------------------------------*/

bool
CServiceAdminEndpoint::ReadTokenFile( const CORE::CString& stateDirectory, CORE::CString& tokenHex, CORE::CString& error )
{GUCEF_TRACE;

    const CORE::CString path = CORE::CombinePath( stateDirectory, TokenFileName );
    CORE::CDynamicBuffer content;
    CORE::CString problem;
    bool isMissing = false;
    if ( !CORE::ReadOwnerOnlyFile( path, content, problem, &isMissing ) )
    {
        error = isMissing ? "the token file " + path + " does not exist (is the service running?)" : "the token file " + path + " cannot be read: " + problem;
        return false;
    }
    CORE::CString text;
    text.Set( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() );
    text = text.Trim( true ).Trim( false );
    if ( TokenSize * 2 != text.Length() )
    {
        error = "the token file " + path + " does not hold a token";
        return false;
    }
    tokenHex = text;
    return true;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CServiceAdminEndpoint::OnRun( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    const COMCORE::CRpcMessage& request = call.GetRequest();

    // the token first, compared in a time that does not depend on where it differs
    CORE::CDynamicBuffer presented;
    request.GetBytes( 1, presented );
    bool tokenOk = false;
    UInt8 expected[ TokenSize ];
    if ( TokenSize == presented.GetDataSize() && m_tokenHex.Length() == TokenSize * 2 )
    {
        for ( UInt32 i=0; i<TokenSize; ++i )
        {
            const CORE::CString pair = m_tokenHex.SubstrFromRange( i * 2, i * 2 + 2 );
            expected[ i ] = (UInt8) strtoul( pair.C_String(), GUCEF_NULL, 16 );
        }
        tokenOk = CORE::CSha256::ConstantTimeEquals( expected, presented.GetConstBufferPtr(), TokenSize );
        memset( expected, 0, TokenSize );
    }
    if ( !tokenOk )
    {
        ++m_refused;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ServiceAdminEndpoint: refused a request without the token from " << call.GetPrincipal().subject );
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_UNAUTHENTICATED, "the admin token is missing or wrong" ) );
        return COMCORE::RPC_HANDLER_DONE;
    }

    const CORE::CString command = request.GetString( 2 );
    if ( command.IsNULLOrEmpty() )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_INVALID_ARGUMENT, "no command" ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    if ( !m_commands.empty() && m_commands.find( command ) == m_commands.end() )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_NOT_FOUND, "unknown command: " + command ) );
        return COMCORE::RPC_HANDLER_DONE;
    }

    CIServiceAdminHandler::TArgumentMap arguments;
    COMCORE::CRpcMessage::TVariantVector keys;
    request.GetMapKeys( 3, keys );
    for ( UInt32 i=0; i<keys.size(); ++i )
    {
        CORE::CVariant value;
        if ( request.GetMapEntry( 3, keys[ i ], value ) )
            arguments[ keys[ i ].AsString() ] = value.AsString();
    }

    CORE::CDataNode result;
    CORE::CString message;
    const TServiceAdminStatus status = m_handler.HandleAdminCommand( command, arguments, result, message );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServiceAdminEndpoint: " << command << " -> " << ServiceAdminStatusToString( status ) );

    switch ( status )
    {
        case SERVICEADMIN_OK:
        {
            COMCORE::CRpcMessage response = call.NewResponse();
            response.SetDataNode( 1, result );
            if ( !message.IsNULLOrEmpty() )
                response.SetString( 2, message );
            call.Reply( response );
            break;
        }
        case SERVICEADMIN_INVALID_ARGUMENT:
        {
            call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_INVALID_ARGUMENT, message ) );
            break;
        }
        case SERVICEADMIN_NOT_IMPLEMENTED:
        {
            call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_UNIMPLEMENTED, message ) );
            break;
        }
        case SERVICEADMIN_FAILED:
        default:
        {
            call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_FAILED_PRECONDITION, message ) );
            break;
        }
    }
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
