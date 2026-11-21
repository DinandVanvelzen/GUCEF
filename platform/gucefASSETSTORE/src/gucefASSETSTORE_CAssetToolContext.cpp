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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#ifndef GUCEF_COM_CMCPSERVER_H
#include "gucefCOM_CMcpServer.h"
#define GUCEF_COM_CMCPSERVER_H
#endif /* GUCEF_COM_CMCPSERVER_H ? */

#ifndef GUCEF_COM_CMCPJOBINFO_H
#include "gucefCOM_CMcpJobInfo.h"
#define GUCEF_COM_CMCPJOBINFO_H
#endif /* GUCEF_COM_CMCPJOBINFO_H ? */

#include "gucefASSETSTORE_CAssetToolContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 MaxPendingChanges = 512;
const UInt32 MaxPendingPerPrincipal = 32;
const UInt32 MaxNonces = 4096;
const Int64 HourMs = 3600000;
const Int64 DayMs = 86400000;

bool
IsContentBlock( const COM::CMcpContent& block )
{GUCEF_TRACE;

    return COM::CMcpContent::MCP_CONTENT_IMAGE == block.type ||
           COM::CMcpContent::MCP_CONTENT_AUDIO == block.type ||
           COM::CMcpContent::MCP_CONTENT_RESOURCE == block.type;
}

/** The largest array among the direct children of the node, GUCEF_NULL without one that has entries */
CORE::CDataNode*
FindLargestList( CORE::CDataNode& node )
{GUCEF_TRACE;

    CORE::CDataNode* largest = GUCEF_NULL;
    UInt32 largestCount = 0;
    CORE::CDataNode::iterator i = node.Begin();
    while ( i != node.End() )
    {
        CORE::CDataNode* child = (*i);
        if ( GUCEF_DATATYPE_ARRAY == child->GetNodeType() )
        {
            const UInt32 count = child->GetNrOfDirectChildNodes();
            if ( count > largestCount )
            {
                largest = child;
                largestCount = count;
            }
        }
        ++i;
    }
    return largest;
}

/** Keeps the first 'keep' entries of an array node */
void
CutList( CORE::CDataNode& list, UInt32 keep )
{GUCEF_TRACE;

    CORE::CDataNode copy( list.GetName(), GUCEF_DATATYPE_ARRAY );
    UInt32 index = 0;
    CORE::CDataNode::iterator i = list.Begin();
    while ( i != list.End() && index < keep )
    {
        copy.AddChild( *(*i) );
        ++index;
        ++i;
    }
    list.Copy( copy );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetToolContext::SContentTag::SContentTag( void )
    : contentIndex( 0 )
    , converterPackId()
    , storePackId()
    , storeAssetId()
    , stage()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetToolContext::CAssetToolContext( const TAssetStoreServicePtr& service, const CAssetToolSetSettings& settings )
    : m_service( service )
    , m_settings( settings )
    , m_exposure()
    , m_redaction()
    , m_gate( m_service.GetPointerAlways() )
    , m_callGate()
    , m_bridge( GUCEF_NULL )
    , m_server( GUCEF_NULL )
    , m_lock()
    , m_writes()
    , m_writeQuotas()
    , m_jobQuotas()
    , m_pending()
    , m_jobPacks()
    , m_jobPackOrder()
    , m_usedNonces()
    , m_nonceOrder()
{GUCEF_TRACE;

    m_writes.SetBounds( settings.requestKeyTtlMs, 256, 4096 );
    m_exposure.SetFile( settings.exposureFile );
}

/*-------------------------------------------------------------------------*/

CAssetToolContext::~CAssetToolContext()
{GUCEF_TRACE;

    m_gate.SetService( GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

CIAssetStoreService&
CAssetToolContext::GetService( void )
{GUCEF_TRACE;

    return *m_service;
}

/*-------------------------------------------------------------------------*/

const TAssetStoreServicePtr&
CAssetToolContext::GetServicePtr( void ) const
{GUCEF_TRACE;

    return m_service;
}

/*-------------------------------------------------------------------------*/

const CAssetToolSetSettings&
CAssetToolContext::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

CAssetToolsExposureControl&
CAssetToolContext::GetExposureControl( void )
{GUCEF_TRACE;

    return m_exposure;
}

/*-------------------------------------------------------------------------*/

CAssetToolsExposure
CAssetToolContext::GetExposure( void ) const
{GUCEF_TRACE;

    return m_exposure.GetState();
}

/*-------------------------------------------------------------------------*/

CAssetToolsRedaction&
CAssetToolContext::GetRedaction( void )
{GUCEF_TRACE;

    return m_redaction;
}

/*-------------------------------------------------------------------------*/

const CAssetContentExposureGate&
CAssetToolContext::GetGate( void ) const
{GUCEF_TRACE;

    return m_gate;
}

/*-------------------------------------------------------------------------*/

COM::CMcpCallGate&
CAssetToolContext::GetCallGate( void )
{GUCEF_TRACE;

    return m_callGate;
}

/*-------------------------------------------------------------------------*/

void
CAssetToolContext::SetJobBridge( COM::CMcpJobBridge* bridge )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_bridge = bridge;
}

/*-------------------------------------------------------------------------*/

COM::CMcpJobBridge*
CAssetToolContext::GetJobBridge( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_bridge;
}

/*-------------------------------------------------------------------------*/

void
CAssetToolContext::SetServer( COM::CMcpServer* server )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_server = server;
}

/*-------------------------------------------------------------------------*/

void
CAssetToolContext::NotifyResourceUpdated( const CORE::CString& uri )
{GUCEF_TRACE;

    COM::CMcpServer* server = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        server = m_server;
    }
    if ( GUCEF_NULL != server )
        server->NotifyResourceUpdated( uri );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetToolContext::GetPageLimit( UInt32 asked ) const
{GUCEF_TRACE;

    UInt32 limit = 0 == asked ? m_settings.defaultLimit : asked;
    if ( limit > m_settings.maxLimit )
        limit = m_settings.maxLimit;
    const UInt32 rows = m_exposure.GetState().maxRows;
    if ( limit > rows )
        limit = rows;
    return limit > 0 ? limit : 1;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetToolContext::GetUntrustedTextChars( void ) const
{GUCEF_TRACE;

    const UInt32 exposureChars = m_exposure.GetState().maxUntrustedTextChars;
    return exposureChars < m_settings.maxUntrustedTextChars ? exposureChars : m_settings.maxUntrustedTextChars;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolContext::Sha256Hex( const CORE::CString& text )
{GUCEF_TRACE;

    UInt8 digest[ CORE::CSha256::DigestSize ];
    CORE::CSha256::Hash( text.C_String(), text.Length(), digest );
    return CORE::ConvertBytesToHexString( digest, CORE::CSha256::DigestSize, false, false );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolContext::MakeCommandId( const CORE::CString& subject, const CORE::CString& tool, const CORE::CString& requestKey )
{GUCEF_TRACE;

    if ( requestKey.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    return "mcp-" + Sha256Hex( subject + "\n" + tool + "\n" + requestKey ).SubstrToIndex( 40, true );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolContext::DigestArguments( const CORE::CDataNode& arguments )
{GUCEF_TRACE;

    CORE::CDataNode copy( arguments );
    copy.DelAttribute( "requestKey" );
    copy.DelAttribute( "waitMs" );
    CORE::CString text;
    COM::CJsonRpcEndpoint::ToJsonText( copy, text );
    return Sha256Hex( text );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolContext::MakeHandle( void )
{GUCEF_TRACE;

    UInt8 bytes[ 16 ];
    if ( !CORE::CRandom::GetRandomBytes( bytes, 16 ) )
        return CORE::CString::Empty;
    CORE::CString handle;
    if ( !CORE::Base64UrlEncode( bytes, 16, handle, false ) )
        return CORE::CString::Empty;
    return handle;
}

/*-------------------------------------------------------------------------*/

Int64
CAssetToolContext::NowUtcMs( void )
{GUCEF_TRACE;

    return (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

/*-------------------------------------------------------------------------*/

CORE::TCommandIdempotencyLookup
CAssetToolContext::FindWrite( const CORE::CString& subject, const CORE::CString& tool, const CORE::CString& requestKey,
                              const CORE::CString& digest, CORE::CDataNode& outcome )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_writes.Find( subject, tool + "\n" + requestKey, digest, (UInt64) NowUtcMs(), outcome );
}

/*-------------------------------------------------------------------------*/

void
CAssetToolContext::StoreWrite( const CORE::CString& subject, const CORE::CString& tool, const CORE::CString& requestKey,
                               const CORE::CString& digest, const CORE::CDataNode& outcome )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_writes.Store( subject, tool + "\n" + requestKey, digest, (UInt64) NowUtcMs(), outcome );
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolContext::TakeQuota( TQuotaMap& quotas, const CORE::CString& subject, UInt32 maximum, Int64 windowMs, CORE::CString& nextAllowed )
{GUCEF_TRACE;

    const Int64 now = NowUtcMs();
    MT::CScopeMutex lock( m_lock );
    TTimeVector& times = quotas[ subject ];
    TTimeVector kept;
    for ( size_t i=0; i<times.size(); ++i )
        if ( times[ i ] > now - windowMs )
            kept.push_back( times[ i ] );
    times = kept;
    if ( times.size() >= maximum )
    {
        nextAllowed = times.empty() ? CORE::CString::Empty : COM::CMcpJobInfo::UtcMsToText( times[ 0 ] + windowMs );
        return false;
    }
    times.push_back( now );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolContext::TakeWriteQuota( const CORE::CString& subject, CORE::CString& nextAllowed )
{GUCEF_TRACE;

    return TakeQuota( m_writeQuotas, subject, m_exposure.GetState().maxWritesPerHour, HourMs, nextAllowed );
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolContext::TakeJobQuota( const CORE::CString& subject, CORE::CString& nextAllowed )
{GUCEF_TRACE;

    return TakeQuota( m_jobQuotas, subject, m_exposure.GetState().maxJobsPerDay, DayMs, nextAllowed );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolContext::AddPendingChange( const CORE::CString& subject, const CORE::CString& tool, const CORE::CString& digest,
                                     const CORE::CDataNode& payload, Int64& expiresUtcMs )
{GUCEF_TRACE;

    const CORE::CString token = MakeHandle();
    if ( token.IsNULLOrEmpty() )
        return token;
    const Int64 now = NowUtcMs();
    MT::CScopeMutex lock( m_lock );

    // expired previews go, then the oldest of the principal and in total when a bound is reached
    CORE::CString oldestOfSubject;
    Int64 oldestOfSubjectTime = 0;
    UInt32 ofSubject = 0;
    CORE::CString oldest;
    Int64 oldestTime = 0;
    TPendingMap::iterator i = m_pending.begin();
    while ( i != m_pending.end() )
    {
        if ( (*i).second.expiresUtcMs <= now )
        {
            m_pending.erase( i++ );
            continue;
        }
        if ( (*i).second.subject == subject )
        {
            ++ofSubject;
            if ( oldestOfSubject.IsNULLOrEmpty() || (*i).second.expiresUtcMs < oldestOfSubjectTime )
            {
                oldestOfSubject = (*i).first;
                oldestOfSubjectTime = (*i).second.expiresUtcMs;
            }
        }
        if ( oldest.IsNULLOrEmpty() || (*i).second.expiresUtcMs < oldestTime )
        {
            oldest = (*i).first;
            oldestTime = (*i).second.expiresUtcMs;
        }
        ++i;
    }
    if ( ofSubject >= MaxPendingPerPrincipal && !oldestOfSubject.IsNULLOrEmpty() )
        m_pending.erase( oldestOfSubject );
    else
    if ( m_pending.size() >= MaxPendingChanges && !oldest.IsNULLOrEmpty() )
        m_pending.erase( oldest );

    SPending pending;
    pending.subject = subject;
    pending.tool = tool;
    pending.digest = digest;
    pending.payload = payload;
    pending.expiresUtcMs = now + (Int64) m_settings.changeTokenTtlMs;
    m_pending[ token ] = pending;
    expiresUtcMs = pending.expiresUtcMs;
    return token;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolContext::TakePendingChange( const CORE::CString& token, const CORE::CString& subject, const CORE::CString& tool,
                                      const CORE::CString& digest, CORE::CDataNode& payload, CORE::CString& problem )
{GUCEF_TRACE;

    const Int64 now = NowUtcMs();
    MT::CScopeMutex lock( m_lock );
    TPendingMap::iterator i = m_pending.find( token );
    if ( i == m_pending.end() )
    {
        problem = "the change token is not known: it was used, it expired, or it was never given. Make a new preview";
        return false;
    }
    if ( (*i).second.expiresUtcMs <= now )
    {
        m_pending.erase( i );
        problem = "the change token expired. Make a new preview";
        return false;
    }
    if ( (*i).second.subject != subject )
    {
        problem = "the change token is not known: it was used, it expired, or it was never given. Make a new preview";
        return false;
    }
    if ( (*i).second.tool != tool || (*i).second.digest != digest )
    {
        problem = "the change token belongs to a preview with other arguments. Apply with the arguments of the preview, or make a new preview";
        return false;
    }
    payload = (*i).second.payload;
    m_pending.erase( i );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetToolContext::RememberJobPack( const CORE::CString& jobId, const CORE::CString& packId )
{GUCEF_TRACE;

    if ( jobId.IsNULLOrEmpty() || packId.IsNULLOrEmpty() )
        return;
    MT::CScopeMutex lock( m_lock );
    if ( m_jobPacks.end() == m_jobPacks.find( jobId ) )
        m_jobPackOrder.push_back( jobId );
    m_jobPacks[ jobId ] = packId;
    if ( m_jobPackOrder.size() > MaxNonces )
    {
        m_jobPacks.erase( m_jobPackOrder[ 0 ] );
        m_jobPackOrder.erase( m_jobPackOrder.begin() );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolContext::GetJobPack( const CORE::CString& jobId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CORE::CString::StringMap::const_iterator i = m_jobPacks.find( jobId );
    return i != m_jobPacks.end() ? (*i).second : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolContext::UseConfirmationNonce( const CORE::CString& nonce )
{GUCEF_TRACE;

    if ( nonce.IsNULLOrEmpty() )
        return false;
    MT::CScopeMutex lock( m_lock );
    if ( m_usedNonces.end() != m_usedNonces.find( nonce ) )
        return false;
    m_usedNonces.insert( nonce );
    m_nonceOrder.push_back( nonce );
    if ( m_nonceOrder.size() > MaxNonces )
    {
        m_usedNonces.erase( m_nonceOrder[ 0 ] );
        m_nonceOrder.erase( m_nonceOrder.begin() );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolContext::GetErrorCodeOfStatus( TAssetStoreStatus status )
{GUCEF_TRACE;

    switch ( status )
    {
        case ASSETSTORE_STATUS_NOT_FOUND:           return "not-found";
        case ASSETSTORE_STATUS_INVALID_ARGUMENT:
        case ASSETSTORE_STATUS_UNSAFE_NAME:         return "invalid-arguments";
        case ASSETSTORE_STATUS_UNSUPPORTED:         return "unsupported";
        case ASSETSTORE_STATUS_REFUSED:             return "licence-refused";
        case ASSETSTORE_STATUS_NO_SPACE:            return "no-space";
        case ASSETSTORE_STATUS_NOT_SIGNED_IN:
        case ASSETSTORE_STATUS_SESSION_EXPIRED:     return "not-signed-in";
        case ASSETSTORE_STATUS_RATE_LIMITED:        return "busy";
        case ASSETSTORE_STATUS_ALREADY_EXISTS:
        case ASSETSTORE_STATUS_DUPLICATE:           return "already-exists";
        case ASSETSTORE_STATUS_CANCELLED:           return "cancelled";
        case ASSETSTORE_STATUS_UNAVAILABLE:
        case ASSETSTORE_STATUS_NETWORK_ERROR:
        case ASSETSTORE_STATUS_SERVER_ERROR:
        case ASSETSTORE_STATUS_HTTP_ERROR:
        case ASSETSTORE_STATUS_DATABASE_ERROR:
        case ASSETSTORE_STATUS_TIMED_OUT:           return "backend-unavailable";
        default:                                    return "failed";
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetToolContext::SetError( COM::CMcpToolResult& result, const CORE::CString& code, const CORE::CString& message, bool retryable )
{GUCEF_TRACE;

    result.Clear();
    COM::CMcpJobBridge::BuildErrorResult( code, m_redaction.ForResult( message.SubstrToIndex( 600, true ) ), retryable, result );
}

/*-------------------------------------------------------------------------*/

void
CAssetToolContext::SetServiceError( COM::CMcpToolResult& result, TAssetStoreStatus status, const CORE::CString& message,
                                    const CORE::CDataNode* document )
{GUCEF_TRACE;

    CORE::CString code;
    CORE::CString text = message;
    if ( GUCEF_NULL != document )
    {
        code = document->GetAttributeValueOrChildValueByName( "errorCode" ).AsString( CORE::CString::Empty, true );
        if ( text.IsNULLOrEmpty() )
            text = document->GetAttributeValueOrChildValueByName( "message" ).AsString( CORE::CString::Empty, true );
    }
    if ( code.IsNULLOrEmpty() )
        code = GetErrorCodeOfStatus( status );
    if ( text.IsNULLOrEmpty() )
        text = CORE::CString( "The service answered " ) + AssetStoreStatusToString( status );
    if ( "unsupported" == code && 0 > text.HasSubstr( "unsupported" ) )
        text = "unsupported: " + text;
    const bool retryable = "busy" == code || "backend-unavailable" == code;
    SetError( result, code, text, retryable );
}

/*-------------------------------------------------------------------------*/

void
CAssetToolContext::BoundResult( CORE::CDataNode& structured ) const
{GUCEF_TRACE;

    CORE::CString text;
    COM::CJsonRpcEndpoint::ToJsonText( structured, text );
    bool cut = false;
    for ( UInt32 guard=0; guard<64 && text.Length() > m_settings.maxResultBytes; ++guard )
    {
        CORE::CDataNode* list = FindLargestList( structured );
        if ( GUCEF_NULL == list )
            break;
        CutList( *list, list->GetNrOfDirectChildNodes() / 2 );
        cut = true;
        COM::CJsonRpcEndpoint::ToJsonText( structured, text );
    }
    if ( cut )
        structured.SetAttribute( "truncated", true );
}

/*-------------------------------------------------------------------------*/

void
CAssetToolContext::FinishResult( COM::CMcpToolResult& result, const TContentTagVector& tags )
{GUCEF_TRACE;

    // the content filter: a block that shows a pack must carry the pack and pass the gate again
    COM::CMcpToolResult::TContentVector kept;
    UInt32 dropped = 0;
    for ( UInt32 i=0; i<(UInt32) result.content.size(); ++i )
    {
        COM::CMcpContent& block = result.content[ i ];
        if ( IsContentBlock( block ) )
        {
            const SContentTag* tag = GUCEF_NULL;
            for ( size_t t=0; t<tags.size(); ++t )
                if ( tags[ t ].contentIndex == i )
                    tag = &tags[ t ];
            if ( GUCEF_NULL == tag )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetToolContext: A content block without the pack it shows was dropped: this is a defect of the tool" );
                ++dropped;
                continue;
            }
            CORE::CString reason;
            if ( !m_gate.IsAllowed( tag->converterPackId, tag->storePackId, tag->storeAssetId, tag->stage, reason ) )
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetToolContext: A content block was withheld: " << reason );
                ++dropped;
                continue;
            }
        }
        else
        {
            block.text = m_redaction.ForResult( block.text );
            block.name = m_redaction.ForResult( block.name );
            block.title = m_redaction.ForResult( block.title );
            block.description = m_redaction.ForResult( block.description );
        }
        kept.push_back( block );
    }
    result.content = kept;
    if ( dropped > 0 && result.hasStructuredContent && GUCEF_DATATYPE_OBJECT == result.structuredContent.GetNodeType() )
        result.structuredContent.SetAttribute( "withheldContentBlocks", dropped );

    if ( result.hasStructuredContent )
    {
        m_redaction.RedactTree( result.structuredContent );
        if ( !result.isError )
        {
            BoundResult( result.structuredContent );
            CORE::CString text;
            if ( COM::CJsonRpcEndpoint::ToJsonText( result.structuredContent, text ) )
            {
                if ( text.Length() > m_settings.maxResultBytes )
                {
                    SetError( result, "too-large", "The result is too large: ask for fewer entries (limit) or read the resource it links to", false );
                    return;
                }
                result.content.insert( result.content.begin(), COM::CMcpContent::Text( text ) );
            }
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
