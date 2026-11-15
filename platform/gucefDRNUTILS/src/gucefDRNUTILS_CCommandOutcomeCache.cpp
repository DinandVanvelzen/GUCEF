/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#include "gucefDRNUTILS_CCommandOutcomeCache.h"

#include <string.h>

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_COMCORE_CRPCMETADATA_H
#include "gucefCOMCORE_CRpcMetadata.h"
#define GUCEF_COMCORE_CRPCMETADATA_H
#endif /* GUCEF_COMCORE_CRPCMETADATA_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H
#include "gucefDRNUTILS_CRemoteServiceDetailCodes.h"
#define GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCommandOutcomeCache::SOutcome::SOutcome( void )
    : status()
    , response()
    , hasResponse( false )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CCommandOutcomeCache::CCommandOutcomeCache( void )
    : m_cache()
    , m_hasKey( false )
{GUCEF_TRACE;

    memset( m_key, 0, sizeof( m_key ) );
    m_hasKey = CORE::CRandom::GetRandomBytes( m_key, sizeof( m_key ) );
    if ( !m_hasKey )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CommandOutcomeCache: no randomness for the request hash key; requests are compared by plain SHA-256" );
}

/*-------------------------------------------------------------------------*/

CCommandOutcomeCache::~CCommandOutcomeCache()
{GUCEF_TRACE;

    memset( m_key, 0, sizeof( m_key ) );
}

/*-------------------------------------------------------------------------*/

void
CCommandOutcomeCache::SetBounds( UInt64 ttlMs, UInt32 maxPerSubject, UInt32 maxTotal )
{GUCEF_TRACE;

    m_cache.SetBounds( ttlMs, maxPerSubject, maxTotal );
}

/*-------------------------------------------------------------------------*/

bool
CCommandOutcomeCache::IsValidCommandId( const CORE::CString& commandId )
{GUCEF_TRACE;

    UInt32 length = commandId.ByteSize() > 0 ? commandId.ByteSize() - 1 : 0;
    if ( 0 == length || length > 64 || length != commandId.Length() )
        return false;
    const char* c = commandId.C_String();
    for ( UInt32 i=0; i<length; ++i )
    {
        char ch = c[ i ];
        bool ok = ( ch >= 'A' && ch <= 'Z' ) || ( ch >= 'a' && ch <= 'z' ) || ( ch >= '0' && ch <= '9' ) || '.' == ch || '_' == ch || '-' == ch;
        if ( !ok )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCommandOutcomeCache::GetCommandId( const COMCORE::CRpcServerCall& call, UInt32 commandIdField, CORE::CString& commandId, CORE::CString& reason )
{GUCEF_TRACE;

    CORE::CString id;
    if ( 0 != commandIdField && call.HasRequest() && call.GetRequest().Has( commandIdField ) )
        id = call.GetRequest().GetString( commandIdField );
    if ( id.IsNULLOrEmpty() )
        id = call.GetMetadata().GetString( COMCORE::CRpcMetadata::IdempotencyKey );
    if ( id.IsNULLOrEmpty() )
    {
        reason = CORE::CString( CRemoteServiceDetailCodes::BadCommandId ) + ": the request has no command id";
        return false;
    }
    if ( !IsValidCommandId( id ) )
    {
        reason = CORE::CString( CRemoteServiceDetailCodes::BadCommandId ) + ": a command id is 1 to 64 characters of A-Z, a-z, 0-9, '.', '_' and '-'";
        return false;
    }
    commandId = id;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCommandOutcomeCache::HashBytes( const void* data, UInt32 size ) const
{GUCEF_TRACE;

    UInt8 digest[ CORE::CHmacSha256::MacSize ];
    if ( m_hasKey )
        CORE::CHmacSha256::Mac( m_key, sizeof( m_key ), data, size, digest );
    else
        CORE::CSha256::Hash( data, size, digest );
    return DRN::CDRNCrypto::ToHex( digest, sizeof( digest ) ).Lowercase();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCommandOutcomeCache::HashRequest( const COMCORE::CRpcMessage& request, UInt32 commandIdField ) const
{GUCEF_TRACE;

    COMCORE::CRpcMessage canonical( request );
    if ( 0 != commandIdField )
        canonical.Clear( commandIdField );
    CORE::CDynamicBuffer encoded;
    if ( !canonical.Encode( encoded ) )
        return CORE::CString::Empty;
    CORE::CString hash = HashBytes( encoded.GetConstBufferPtr(), encoded.GetDataSize() );
    encoded.Clear( false );
    return hash;
}

/*-------------------------------------------------------------------------*/

CCommandOutcomeCache::ELookup
CCommandOutcomeCache::Find( const CORE::CString& subject, const CORE::CString& commandId, const CORE::CString& requestHash,
                            UInt64 nowMs, SOutcome& outcome )
{GUCEF_TRACE;

    switch ( m_cache.Find( subject, commandId, requestHash, nowMs, outcome ) )
    {
        case CORE::COMMANDIDEMPOTENCY_HIT: return COMMAND_REPLAY;
        case CORE::COMMANDIDEMPOTENCY_CONFLICT: return COMMAND_REUSED;
        default: return COMMAND_NEW;
    }
}

/*-------------------------------------------------------------------------*/

void
CCommandOutcomeCache::Remember( const CORE::CString& subject, const CORE::CString& commandId, const CORE::CString& requestHash,
                                UInt64 nowMs, const SOutcome& outcome )
{GUCEF_TRACE;

    m_cache.Store( subject, commandId, requestHash, nowMs, outcome );
}

/*-------------------------------------------------------------------------*/

void
CCommandOutcomeCache::MakeOutcome( const COMCORE::CRpcStatus& status, const COMCORE::CRpcMessage* response, SOutcome& outcome )
{GUCEF_TRACE;

    outcome.status = status;
    outcome.response.Clear( false );
    outcome.hasResponse = false;
    if ( status.IsOk() && GUCEF_NULL != response )
        outcome.hasResponse = response->Encode( outcome.response );
}

/*-------------------------------------------------------------------------*/

bool
CCommandOutcomeCache::AnswerReplay( COMCORE::CRpcServerCall& call, const SOutcome& outcome, UInt32 replayedField )
{GUCEF_TRACE;

    if ( !outcome.status.IsOk() )
        return call.Fail( outcome.status );

    COMCORE::CRpcMessage response = call.NewResponse();
    if ( outcome.hasResponse )
    {
        COMCORE::CRpcStatus decodeStatus;
        if ( !response.Decode( outcome.response.GetConstBufferPtr(), outcome.response.GetDataSize(), decodeStatus ) )
            return call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_INTERNAL, "the kept outcome cannot be read" ) );
    }
    if ( 0 != replayedField )
        response.SetBool( replayedField, true );
    return call.Reply( response );
}

/*-------------------------------------------------------------------------*/

bool
CCommandOutcomeCache::RefuseReused( COMCORE::CRpcServerCall& call, const CORE::CString& commandId )
{GUCEF_TRACE;

    COMCORE::CRpcStatus status( COMCORE::RPC_STATUS_INVALID_ARGUMENT,
                                CORE::CString( CRemoteServiceDetailCodes::CommandIdReused ) + ": the command id " + commandId + " was used for another request" );
    CRemoteServiceDetailCodes::SetDetails( status.details, CRemoteServiceDetailCodes::CommandIdReused );
    return call.Fail( status );
}

/*-------------------------------------------------------------------------*/

UInt32
CCommandOutcomeCache::GetCount( void ) const
{GUCEF_TRACE;

    return m_cache.GetCount();
}

/*-------------------------------------------------------------------------*/

void
CCommandOutcomeCache::Clear( void )
{GUCEF_TRACE;

    m_cache.Clear();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
