/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
  #include <sys/stat.h>
#endif

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "gucefLabLink_Protocol.h"
#include "gucefLabLink_CiHub.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CCiHub::NoCiConfigured = "no CI configured";

static const UInt64 ReconcileIntervalInMs = 60 * 1000;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The content of a text file, empty when it cannot be read */
static CORE::CString
ReadTextFile( const CORE::CString& path )
{
    CORE::CDynamicBuffer content;
    if ( !CORE::FileExists( path ) || !content.LoadContentFromFile( path ) )
        return CORE::CString::Empty;
    return ( 0 == content.GetDataSize() ? CORE::CString::Empty : CORE::CString( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CCiHub::GatewayUrlOf( const SSettings& settings )
{GUCEF_TRACE;

    if ( !settings.ciGatewayUrl.IsNULLOrEmpty() )
        return StripText( settings.ciGatewayUrl );
    return "https://" + settings.clusterName + ":8792";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiHub::RegistryFileOf( const SSettings& settings )
{GUCEF_TRACE;

    if ( !settings.ciRegistryFile.IsNULLOrEmpty() )
        return settings.ciRegistryFile;
    return CORE::CombinePath( settings.stateDir, "ci.json" );
}

/*-------------------------------------------------------------------------*/

CCiHub::CCiHub( const SSettings& settings, CStore& store )
    : m_settings( settings )
    , m_store( store )
    , m_registry( RegistryFileOf( settings ), GatewayUrlOf( settings ) )
    , m_tokens( store, settings.ciGatewayTokensFile, settings.ciGatewayUser )
    , m_nextReconcileInMs( 0 )
    , m_lastReconcileProblem()
    , m_lastTokenProblem()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCiGatewayTokens&
CCiHub::GetTokens( void )
{GUCEF_TRACE;

    return m_tokens;
}

/*-------------------------------------------------------------------------*/

CCiRegistryFile&
CCiHub::GetRegistry( void )
{GUCEF_TRACE;

    return m_registry;
}

/*-------------------------------------------------------------------------*/

void
CCiHub::Start( void )
{GUCEF_TRACE;

    if ( !m_registry.GetGatewayUrl().Lowercase().StartsWith( "https://" ) )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefLabLink: The CI gateway URL " << m_registry.GetGatewayUrl() << " is not https: machines get no CI configuration" );
    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
    // a gateway that runs as another user has to be able to reach the hash file
    struct stat info;
    if ( !m_settings.ciGatewayUser.IsNULLOrEmpty() && m_settings.ciGatewayTokensFile.IsNULLOrEmpty() &&
         0 == ::stat( m_settings.stateDir.C_String(), &info ) && 0 == ( info.st_mode & S_IXOTH ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: The CI gateway user " << m_settings.ciGatewayUser << " cannot enter " << m_settings.stateDir <<
                           ", where the gateway tokens are (" << m_tokens.GetHashFilePath() << "): name a file in a directory it can enter with --ci-gateway-tokens" );
    #endif
    CORE::CString merged, problem;
    m_registry.Get( merged, problem );
    Reconcile();
    m_nextReconcileInMs = NowInMs() + ReconcileIntervalInMs;
}

/*-------------------------------------------------------------------------*/

void
CCiHub::Update( UInt64 nowInMs )
{GUCEF_TRACE;

    if ( nowInMs < m_nextReconcileInMs )
        return;
    m_nextReconcileInMs = nowInMs + ReconcileIntervalInMs;
    Reconcile();
}

/*-------------------------------------------------------------------------*/

void
CCiHub::Reconcile( void )
{GUCEF_TRACE;

    // an approval that was taken back by any means (--revoke, an edit of the file) takes its gateway token with it
    CORE::CString problem;
    GUCEF::vector< SApproval > approvals;
    if ( !m_store.LoadApprovals( approvals, problem ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: The gateway tokens of the CI are not checked, the approvals cannot be read: " << problem );
        return;
    }
    UInt32 removed = 0;
    const bool reconciled = m_tokens.Reconcile( approvals, removed, problem );
    {
        MT::CScopeMutex lock( m_lock );
        if ( !reconciled && problem != m_lastReconcileProblem )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: The gateway tokens of the CI cannot be brought in line with the approvals: " << problem );
        m_lastReconcileProblem = reconciled ? CORE::CString::Empty : problem;
    }
    if ( !reconciled )
        return;
    if ( removed > 0 )
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: " << removed << " gateway token(s) of the CI taken away: their machines are no longer approved" );
}

/*-------------------------------------------------------------------------*/

COMCORE::CRpcStatus
CCiHub::Answer( const CORE::CString& subject                ,
                const CORE::CString::StringSet& roles       ,
                const CORE::CString& peerAddress            ,
                SCiConfigAnswer& answer                     )
{GUCEF_TRACE;

    answer = SCiConfigAnswer();
    const CORE::CString address = peerAddress.IsNULLOrEmpty() ? CORE::CString( "an unknown address" ) : peerAddress;

    // approvals only: a token of the lab secret names a machine id its maker chose
    const bool approvedToken = roles.find( RoleApproved ) != roles.end() && roles.find( RoleLab ) == roles.end();
    const CORE::CString machineId = subject.StartsWith( "drn:" ) ? subject.CutChars( 4, true ) : CORE::CString::Empty;
    if ( !approvedToken || machineId.IsNULLOrEmpty() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: CiConfig refused to " << subject << " at " << address <<
                           ": only an approved machine gets the CI configuration, not a token of the lab secret" );
        return COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, "the CI configuration is for approved machines only" );
    }
    CORE::CString problem;
    GUCEF::vector< SApproval > approvals;
    if ( !m_store.LoadApprovals( approvals, problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: " << problem );
        return COMCORE::CRpcStatus( COMCORE::RPC_STATUS_UNAVAILABLE, "the cluster cannot read its approvals" );
    }
    CORE::CString name;
    bool approved = false;
    for ( size_t i=0; i<approvals.size(); ++i )
    {
        if ( approvals[ i ].machineId == machineId )
        {
            approved = approvals[ i ].state == AccessApproved;
            name = approvals[ i ].name;
        }
    }
    if ( !approved )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: CiConfig refused to " << subject << " at " << address << ": the machine is not approved" );
        return COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, "the CI configuration is for approved machines only" );
    }

    CORE::CString merged;
    switch ( m_registry.Get( merged, problem ) )
    {
        case CIREGISTRY_OK:
            break;
        case CIREGISTRY_MISSING:
            return COMCORE::CRpcStatus( COMCORE::RPC_STATUS_NOT_FOUND, NoCiConfigured );
        default:
            // the reason is in the log of the hub, said once
            return COMCORE::CRpcStatus( COMCORE::RPC_STATUS_FAILED_PRECONDITION, "the CI registry of the cluster cannot be handed out" );
    }
    if ( !m_registry.GetGatewayUrl().Lowercase().StartsWith( "https://" ) )
        return COMCORE::CRpcStatus( COMCORE::RPC_STATUS_FAILED_PRECONDITION, "the CI gateway of the cluster is not https" );

    CORE::CString token;
    bool made = false;
    bool full = false;
    if ( !m_tokens.GetOrMake( machineId, token, made, full, problem ) )
    {
        {
            MT::CScopeMutex lock( m_lock );
            if ( problem != m_lastTokenProblem )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: No gateway token of the CI for " << name << " (" << machineId << "): " << problem );
            m_lastTokenProblem = problem;
        }
        if ( full )
            return COMCORE::CRpcStatus( COMCORE::RPC_STATUS_RESOURCE_EXHAUSTED, "the cluster gives no more gateway tokens" );
        return COMCORE::CRpcStatus( COMCORE::RPC_STATUS_UNAVAILABLE, "the cluster cannot keep a gateway token" );
    }
    if ( made )
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: A gateway token of the CI is made for " << name << " (" << machineId << ") at " << address );

    answer.registry = merged;
    answer.gatewayToken = token;
    answer.caCertificate = ReadTextFile( CORE::CombinePath( m_settings.mcpDir, "tls/ca.crt" ) );
    answer.gatewayUrl = m_registry.GetGatewayUrl();
    return COMCORE::CRpcStatus( COMCORE::RPC_STATUS_OK );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
