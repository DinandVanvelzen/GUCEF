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

#include "gucefCOM_CServiceAdminClient.h"

#ifndef GUCEF_COM_CSERVICEADMINCONTRACTS_H
#include "gucefCOM_CServiceAdminContracts.h"
#define GUCEF_COM_CSERVICEADMINCONTRACTS_H
#endif /* GUCEF_COM_CSERVICEADMINCONTRACTS_H ? */

#ifndef GUCEF_COM_CSERVICEADMINENDPOINT_H
#include "gucefCOM_CServiceAdminEndpoint.h"
#define GUCEF_COM_CSERVICEADMINENDPOINT_H
#endif /* GUCEF_COM_CSERVICEADMINENDPOINT_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINT_H
#include "gucefCOMCORE_CRpcEndpoint.h"
#define GUCEF_COMCORE_CRPCENDPOINT_H
#endif /* GUCEF_COMCORE_CRPCENDPOINT_H ? */

#ifndef GUCEF_COMCORE_CRPCCALLRESULT_H
#include "gucefCOMCORE_CRpcCallResult.h"
#define GUCEF_COMCORE_CRPCCALLRESULT_H
#endif /* GUCEF_COMCORE_CRPCCALLRESULT_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

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

namespace {

UInt64
AdminClientNowMs( void )
{GUCEF_TRACE;

    const UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    if ( 0 == ticksPerMs )
        return MT::PrecisionTickCount();
    return MT::PrecisionTickCount() / ticksPerMs;
}

/** Pulses the client's own pulse generator until the condition holds or the deadline passed */
class CAdminWait
{
    public:
    virtual ~CAdminWait() {}
    virtual bool IsDone( void ) const = 0;
};

bool
PumpOwnUntil( const CORE::PulseGeneratorPtr& pulseGenerator, const CAdminWait& wait, UInt64 deadlineMs )
{GUCEF_TRACE;

    CORE::PulseGeneratorPtr pulse( pulseGenerator );
    while ( !wait.IsDone() )
    {
        if ( AdminClientNowMs() >= deadlineMs )
            return false;
        pulse->ForceDirectPulse();
        if ( !wait.IsDone() )
            MT::PrecisionDelay( 1 );
    }
    return true;
}

class CBoundWait : public CAdminWait
{
    public:
    const COMCORE::CRpcEndpointPtr& endpoint;
    explicit CBoundWait( const COMCORE::CRpcEndpointPtr& e ) : endpoint( e ) {}
    virtual bool IsDone( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return endpoint->IsContractBound( CServiceAdminContracts::ContractName, 1 ) || COMCORE::RPC_SESSIONSTATE_CLOSED == endpoint->GetState();
    }
};

class CFutureWait : public CAdminWait
{
    public:
    const CORE::CFutureResult& future;
    explicit CFutureWait( const CORE::CFutureResult& f ) : future( f ) {}
    virtual bool IsDone( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return future.IsReady() || future.HasNoFuture();
    }
};

bool
HexToBytes( const CORE::CString& hex, CORE::CDynamicBuffer& bytes )
{GUCEF_TRACE;

    const UInt32 length = hex.Length();
    if ( 0 != ( length % 2 ) )
        return false;
    bytes.SetDataSize( length / 2 );
    UInt8* out = static_cast< UInt8* >( bytes.GetBufferPtr() );
    for ( UInt32 i=0; i<length/2; ++i )
    {
        UInt32 value = 0;
        for ( UInt32 k=0; k<2; ++k )
        {
            const UInt32 c = hex[ i * 2 + k ];
            UInt32 digit = 0;
            if ( c >= '0' && c <= '9' )
                digit = c - '0';
            else
            if ( c >= 'a' && c <= 'f' )
                digit = c - 'a' + 10;
            else
            if ( c >= 'A' && c <= 'F' )
                digit = c - 'A' + 10;
            else
                return false;
            value = value * 16 + digit;
        }
        out[ i ] = (UInt8) value;
    }
    return true;
}

}

/*-------------------------------------------------------------------------*/

bool
CServiceAdminClient::Run( UInt16 port                     ,
                          const CORE::CString& tokenHex   ,
                          const CORE::CString& command    ,
                          const TArgumentMap& arguments   ,
                          CORE::CDataNode& result         ,
                          COMCORE::CRpcStatus& status     ,
                          UInt32 timeoutMs                )
{GUCEF_TRACE;

    result.Clear();
    const UInt64 deadlineMs = AdminClientNowMs() + ( 0 == timeoutMs ? 30000 : timeoutMs );

    CORE::CDynamicBuffer token;
    if ( !HexToBytes( tokenHex, token ) )
    {
        status.Set( COMCORE::RPC_STATUS_INVALID_ARGUMENT, "the admin token is not hexadecimal" );
        return false;
    }
    if ( CServiceAdminContracts::GetContract().IsNULL() )
    {
        status.Set( COMCORE::RPC_STATUS_INTERNAL, "the contract gucef.service.admin cannot be built" );
        return false;
    }

    // a pulse generator of this call alone, driven by this thread: nothing else is pumped here
    CORE::PulseGeneratorPtr pulse = CORE::CPulseGenerator::CreateSharedObj();
    pulse->SetPulseDriverThreadId( MT::GetCurrentThreadID() );

    COMCORE::CIRpcTransportPtr transport = COMCORE::CComCoreGlobal::Instance()->GetRpcTransportFactory().Create( "tcp" );
    const CORE::CString uri = "tcp://127.0.0.1:" + CORE::ToString( (UInt32) port );
    if ( transport.IsNULL() || !transport->SetPulseGenerator( pulse ) || !transport->Connect( CORE::CUri( uri ) ) )
    {
        status.Set( COMCORE::RPC_STATUS_UNAVAILABLE, "the admin endpoint at " + uri + " cannot be reached" );
        return false;
    }

    COMCORE::CRpcEndpointSettings settings;
    settings.nodeName = "admin-client";
    settings.autoReconnect = false;
    COMCORE::CRpcEndpointPtr endpoint = COMCORE::CRpcEndpoint::Create( pulse, settings );
    endpoint->AddWantedContract( CServiceAdminContracts::ContractName, 1 );
    bool answered = false;
    if ( !endpoint->Start( transport, true ) )
    {
        status.Set( COMCORE::RPC_STATUS_UNAVAILABLE, "the RPC session with the admin endpoint cannot be started" );
    }
    else
    if ( !PumpOwnUntil( pulse, CBoundWait( endpoint ), deadlineMs ) || !endpoint->IsContractBound( CServiceAdminContracts::ContractName, 1 ) )
    {
        status.Set( COMCORE::RPC_STATUS_UNAVAILABLE, "the admin endpoint at " + uri + " did not answer (is the service running?)" );
    }
    else
    {
        COMCORE::CRpcMethodRef ref;
        if ( !endpoint->ResolveMethod( CServiceAdminContracts::RunMethodPath, ref, status ) )
        {
            // status says why
        }
        else
        {
            COMCORE::CRpcMessage request = endpoint->NewRequest( ref );
            request.SetBytes( 1, token );
            request.SetString( 2, command );
            TArgumentMap::const_iterator a = arguments.begin();
            while ( a != arguments.end() )
            {
                request.SetMapEntry( 3, CORE::CVariant( (*a).first ), CORE::CVariant( (*a).second ) );
                ++a;
            }
            COMCORE::CRpcCallOptions options;
            const UInt64 now = AdminClientNowMs();
            options.deadlineMs = (UInt32) ( deadlineMs > now ? deadlineMs - now : 1 );
            options.waitForReady = false;
            CORE::CFutureResult future = endpoint->CallAsync( ref, request, options );
            if ( !PumpOwnUntil( pulse, CFutureWait( future ), deadlineMs + 1000 ) )
            {
                future.RequestCancellation();
                status.Set( COMCORE::RPC_STATUS_DEADLINE_EXCEEDED, "the admin endpoint did not answer in time" );
            }
            else
            {
                COMCORE::CRpcCallResult callResult;
                if ( !COMCORE::CRpcEndpoint::TryGetCallResult( future, callResult ) )
                {
                    status.Set( COMCORE::RPC_STATUS_UNAVAILABLE, "the call ended without an answer" );
                }
                else
                {
                    answered = true;
                    status = callResult.status;
                    if ( status.IsOk() )
                        callResult.response.GetDataNode( 1, result );
                }
            }
        }
    }

    // memset of the token copy: the request held it too, it goes with the request
    memset( token.GetBufferPtr(), 0, token.GetDataSize() );
    endpoint->Close( false );
    for ( UInt32 i=0; i<20; ++i )
    {
        pulse->ForceDirectPulse();
        MT::PrecisionDelay( 1 );
    }
    endpoint.Unlink();
    transport.Unlink();
    pulse->ForceDirectPulse();
    return answered;
}

/*-------------------------------------------------------------------------*/

bool
CServiceAdminClient::RunWithTokenFile( UInt16 port                        ,
                                       const CORE::CString& stateDirectory ,
                                       const CORE::CString& command        ,
                                       const TArgumentMap& arguments       ,
                                       CORE::CDataNode& result             ,
                                       COMCORE::CRpcStatus& status         ,
                                       UInt32 timeoutMs                    )
{GUCEF_TRACE;

    CORE::CString tokenHex;
    CORE::CString error;
    if ( !CServiceAdminEndpoint::ReadTokenFile( stateDirectory, tokenHex, error ) )
    {
        status.Set( COMCORE::RPC_STATUS_UNAUTHENTICATED, error );
        return false;
    }
    return Run( port, tokenHex, command, arguments, result, status, timeoutMs );
}

/*-------------------------------------------------------------------------*/

bool
CServiceAdminClient::ParseCommandLine( const CORE::CString::StringVector& words, CORE::CString& command, TArgumentMap& arguments, CORE::CString& error )
{GUCEF_TRACE;

    command.Clear();
    arguments.clear();
    if ( words.empty() || words[ 0 ].IsNULLOrEmpty() || words[ 0 ].StartsWith( "--" ) )
    {
        error = "no command";
        return false;
    }
    command = words[ 0 ];

    UInt32 i = 1;
    while ( i < (UInt32) words.size() )
    {
        const CORE::CString& word = words[ i ];
        if ( !word.StartsWith( "--" ) || 2 >= word.Length() )
        {
            error = "\"" + word + "\" is not an argument name (--name)";
            return false;
        }
        const CORE::CString name = word.CutChars( 2, true );
        if ( i + 1 < (UInt32) words.size() && !words[ i + 1 ].StartsWith( "--" ) )
        {
            arguments[ name ] = words[ i + 1 ];
            i += 2;
        }
        else
        {
            arguments[ name ] = "true";
            ++i;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
