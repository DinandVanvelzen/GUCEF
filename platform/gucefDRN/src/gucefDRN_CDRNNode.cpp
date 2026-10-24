/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#ifndef GUCEF_DRN_CIDRNACCESSPOLICY_H
#include "gucefDRN_CIDRNAccessPolicy.h"
#define GUCEF_DRN_CIDRNACCESSPOLICY_H
#endif /* GUCEF_DRN_CIDRNACCESSPOLICY_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNRELAYSERVICE_H
#include "gucefDRN_CDRNRelayService.h"
#define GUCEF_DRN_CDRNRELAYSERVICE_H
#endif /* GUCEF_DRN_CDRNRELAYSERVICE_H ? */

#ifndef GUCEF_DRN_CDRNRELAYCHANNEL_H
#include "gucefDRN_CDRNRelayChannel.h"
#define GUCEF_DRN_CDRNRELAYCHANNEL_H
#endif /* GUCEF_DRN_CDRNRELAYCHANNEL_H ? */

#ifndef GUCEF_DRN_CDRNWIREWRITER_H
#include "gucefDRN_CDRNWireWriter.h"
#define GUCEF_DRN_CDRNWIREWRITER_H
#endif /* GUCEF_DRN_CDRNWIREWRITER_H ? */

#ifndef GUCEF_DRN_CIDRNBANLIST_H
#include "gucefDRN_CIDRNBanList.h"
#define GUCEF_DRN_CIDRNBANLIST_H
#endif /* GUCEF_DRN_CIDRNBANLIST_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#include "gucefDRN_CDRNTokenAuthenticator.h"
#define GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#endif /* GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H ? */

#include "gucefDRN_CDRNNode.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CDRNNode::LinkOperationalEvent = "GUCEF::DRN::CDRNNode::LinkOperationalEvent";
const CORE::CEvent CDRNNode::LinkClosedEvent = "GUCEF::DRN::CDRNNode::LinkClosedEvent";
const CORE::CEvent CDRNNode::LinkRefusedEvent = "GUCEF::DRN::CDRNNode::LinkRefusedEvent";
const CORE::CEvent CDRNNode::LatencyUpdateEvent = "GUCEF::DRN::CDRNNode::LatencyUpdateEvent";
const CORE::CEvent CDRNNode::RelayBoundEvent = "GUCEF::DRN::CDRNNode::RelayBoundEvent";
const CORE::CEvent CDRNNode::RelayBindingLostEvent = "GUCEF::DRN::CDRNNode::RelayBindingLostEvent";
const CORE::CEvent CDRNNode::PeerLimitExceededEvent = "GUCEF::DRN::CDRNNode::PeerLimitExceededEvent";
const CORE::CEvent CDRNNode::ClockAnomalyEvent = "GUCEF::DRN::CDRNNode::ClockAnomalyEvent";
const CORE::CEvent CDRNNode::TrafficAnomalyEvent = "GUCEF::DRN::CDRNNode::TrafficAnomalyEvent";

/** The window of the handshakes per address */
static const UInt64 HandshakeWindowUs = 60000000;

/** At most this many addresses are tracked for the handshake rate */
static const UInt32 MaxTrackedAddresses = 65536;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The thread of a node that runs on a thread of its own: pulses the node's pulse generator under the node lock
 */
class CDRNNodeThread : public MT::CActiveObject
{
    public:

    explicit CDRNNodeThread( CDRNNode* node )
        : MT::CActiveObject()
        , m_node( node )
        , m_lock()
    {GUCEF_TRACE;
    }

    virtual ~CDRNNodeThread()
    {GUCEF_TRACE;
    }

    protected:

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return this;
    }

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_lock.Lock( lockWaitTimeoutInMs );
    }

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_lock.Unlock();
    }

    virtual bool OnThreadStart( void* /* taskdata */ ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_node->m_pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentTaskID() );
        m_node->m_channelReaper->SetOwnerThreadId( MT::GetCurrentTaskID() );
        return true;
    }

    virtual bool OnThreadCycle( void* /* taskdata */ ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_node->RunThreadCycle();
        return false;
    }

    private:

    CDRNNode* m_node;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNNodeLinkInfo::CDRNNodeLinkInfo( void )
    : link()
    , closedInfo()
    , latencyInfo()
    , anomaly()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDRNNode::CInternalObserver::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_node )
        m_node->OnInternalNotify( notifier, eventid, eventdata );
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::RegisterEvents( void )
{GUCEF_TRACE;

    LinkOperationalEvent.Initialize();
    LinkClosedEvent.Initialize();
    LinkRefusedEvent.Initialize();
    LatencyUpdateEvent.Initialize();
    RelayBoundEvent.Initialize();
    RelayBindingLostEvent.Initialize();
    PeerLimitExceededEvent.Initialize();
    ClockAnomalyEvent.Initialize();
    TrafficAnomalyEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CDRNNode::CDRNNode( void )
    : CORE::CTSGNotifier( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), true, true )
    , m_settings()
    , m_linkSettings()
    , m_authenticator()
    , m_channelReaper( CDRNChannelReaper::CreateSharedObj() )
    , m_appPulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() )
    , m_pulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() )
    , m_thread( GUCEF_NULL )
    , m_relayClient( GUCEF_NULL )
    , m_relayService( GUCEF_NULL )
    , m_advertisedEndpoints()
    , m_observer()
    , m_listeners()
    , m_links()
    , m_closedLinks()
    , m_connectors()
    , m_sessions()
    , m_accessPolicy( GUCEF_NULL )
    , m_defaultAccessPolicy( GUCEF_NULL )
    , m_nextLinkId( 1 )
    , m_stopping( false )
    , m_banList( GUCEF_NULL )
    , m_haveStaticKey( false )
    , m_handshakeWindows()
    , m_nextWindowPurge( 0 )
    , m_refusedForPolicy( 0 )
    , m_refusedForBan( 0 )
    , m_refusedForPending( 0 )
    , m_refusedForRate( 0 )
    , m_nodeLock()
{GUCEF_TRACE;

    Construct();
}

/*-------------------------------------------------------------------------*/

CDRNNode::CDRNNode( const CORE::PulseGeneratorPtr& appPulseGenerator )
    : CORE::CTSGNotifier( appPulseGenerator, true, true )
    , m_settings()
    , m_linkSettings()
    , m_authenticator()
    , m_channelReaper( CDRNChannelReaper::CreateSharedObj() )
    , m_appPulseGenerator( appPulseGenerator )
    , m_pulseGenerator( appPulseGenerator )
    , m_thread( GUCEF_NULL )
    , m_relayClient( GUCEF_NULL )
    , m_relayService( GUCEF_NULL )
    , m_advertisedEndpoints()
    , m_observer()
    , m_listeners()
    , m_links()
    , m_closedLinks()
    , m_connectors()
    , m_sessions()
    , m_accessPolicy( GUCEF_NULL )
    , m_defaultAccessPolicy( GUCEF_NULL )
    , m_nextLinkId( 1 )
    , m_stopping( false )
    , m_banList( GUCEF_NULL )
    , m_haveStaticKey( false )
    , m_handshakeWindows()
    , m_nextWindowPurge( 0 )
    , m_refusedForPolicy( 0 )
    , m_refusedForBan( 0 )
    , m_refusedForPending( 0 )
    , m_refusedForRate( 0 )
    , m_nodeLock()
{GUCEF_TRACE;

    Construct();
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::Construct( void )
{GUCEF_TRACE;

    CDRNGlobal::Instance();
    RegisterEvents();
    memset( m_staticPrivateKey, 0, sizeof( m_staticPrivateKey ) );
    memset( m_staticPublicKey, 0, sizeof( m_staticPublicKey ) );
    m_observer.m_node = this;
    m_defaultAccessPolicy = GUCEF_NEW CDRNDefaultAccessPolicy();
    m_accessPolicy = m_defaultAccessPolicy;
    m_authenticator = m_settings.auth.CreateAuthenticator();
    UpdateLinkSettings();
    m_relayClient = GUCEF_NEW CDRNRelayClient( *this );
    if ( m_settings.relayEnabled )
        m_relayService = GUCEF_NEW CDRNRelayService( *this );
    m_observer.SubscribeTo( m_pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent );
    m_pulseGenerator->RequestPeriodicPulses( this, m_settings.pulseIntervalMs );
}

/*-------------------------------------------------------------------------*/

CDRNNode::~CDRNNode()
{GUCEF_TRACE;

    Stop();

    MT::CScopeMutex lock( m_nodeLock );
    TSessionVector sessions;
    sessions.swap( m_sessions );
    for ( UInt32 i=0; i<sessions.size(); ++i )
        GUCEF_DELETE sessions[ i ];

    m_observer.m_node = GUCEF_NULL;
    m_observer.UnsubscribeAllFromObserver();
    if ( !m_pulseGenerator.IsNULL() )
        m_pulseGenerator->RequestStopOfPeriodicUpdates( this );

    GUCEF_DELETE m_defaultAccessPolicy;
    m_defaultAccessPolicy = GUCEF_NULL;
    m_accessPolicy = GUCEF_NULL;
    GUCEF_DELETE m_relayService;
    m_relayService = GUCEF_NULL;
    GUCEF_DELETE m_relayClient;
    m_relayClient = GUCEF_NULL;
    memset( m_staticPrivateKey, 0, sizeof( m_staticPrivateKey ) );
    memset( m_linkSettings.staticPrivateKey, 0, sizeof( m_linkSettings.staticPrivateKey ) );
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::UpdateLinkSettings( void )
{GUCEF_TRACE;

    m_linkSettings.nodeId = m_settings.nodeId;
    m_linkSettings.serviceName = m_settings.serviceName;
    m_linkSettings.handshakeTimeoutMs = m_settings.handshakeTimeoutMs;
    m_linkSettings.linkTimeoutMs = m_settings.linkTimeoutMs;
    m_linkSettings.echoIntervalMs = m_settings.echoIntervalMs;
    m_linkSettings.latencyUpdateIntervalMs = m_settings.latencyUpdateIntervalMs;
    m_linkSettings.ackDelayMs = m_settings.ackDelayMs;
    m_linkSettings.firstSequence = 0 != m_settings.firstSequence ? m_settings.firstSequence : 1;
    m_linkSettings.limits = m_settings.securityPolicy.limits;
    m_linkSettings.anomalies = m_settings.securityPolicy.anomalies;
    m_linkSettings.banList = m_banList;
    m_settings.securityPolicy.GetPolicyHash( m_linkSettings.policyHash );
    m_linkSettings.requireEncryption = m_settings.securityPolicy.requireEncryption;
    m_linkSettings.requireTokenPolicyHash = m_settings.securityPolicy.requireTokenPolicyHash;
    m_linkSettings.ciphers = m_settings.securityPolicy.ciphers;

    // the static key, when there is key agreement; it may have come with a plugin since the last call
    EnsureStaticKey();
    m_linkSettings.haveStaticKey = m_haveStaticKey;
    memcpy( m_linkSettings.staticPrivateKey, m_staticPrivateKey, DRN_HANDSHAKE_KEY_SIZE );
    memcpy( m_linkSettings.staticPublicKey, m_staticPublicKey, DRN_HANDSHAKE_KEY_SIZE );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNode::EnsureStaticKey( void )
{GUCEF_TRACE;

    if ( m_haveStaticKey )
        return true;
    if ( !CDRNCrypto::IsKeyAgreementAvailable() )
        return false;

    bool haveKey = false;
    if ( !m_settings.staticPrivateKey.IsNULLOrEmpty() )
    {
        haveKey = CDRNCrypto::FromHex( m_settings.staticPrivateKey.Trim( true ).Trim( false ), m_staticPrivateKey, DRN_HANDSHAKE_KEY_SIZE );
        if ( !haveKey )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNNode: staticPrivateKey is not a key of 32 bytes in hex" );
    }
    else
    if ( !m_settings.staticKeyFile.IsNULLOrEmpty() )
    {
        if ( CORE::FileExists( m_settings.staticKeyFile ) )
        {
            CORE::CString content;
            haveKey = CORE::LoadTextFileAsString( m_settings.staticKeyFile, content ) &&
                      CDRNCrypto::FromHex( content.RemoveChar( '\r' ).RemoveChar( '\n' ).Trim( true ).Trim( false ), m_staticPrivateKey, DRN_HANDSHAKE_KEY_SIZE );
            if ( !haveKey )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNNode: " << m_settings.staticKeyFile << " holds no static key" );
        }
        else
        {
            UInt8 publicKey[ DRN_HANDSHAKE_KEY_SIZE ];
            haveKey = CDRNCrypto::GenerateKeyAgreementKeyPair( m_staticPrivateKey, publicKey ) &&
                      CORE::WriteStringAsTextFile( m_settings.staticKeyFile, CDRNCrypto::ToHex( m_staticPrivateKey, DRN_HANDSHAKE_KEY_SIZE ) + "\n" );
            if ( haveKey )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRNNode: Made a new static key in " << m_settings.staticKeyFile );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNNode: Cannot make the static key file " << m_settings.staticKeyFile );
            }
        }
    }
    else
    {
        UInt8 publicKey[ DRN_HANDSHAKE_KEY_SIZE ];
        haveKey = CDRNCrypto::GenerateKeyAgreementKeyPair( m_staticPrivateKey, publicKey );
    }

    if ( haveKey )
        haveKey = CDRNCrypto::GetKeyAgreementPublicKey( m_staticPrivateKey, m_staticPublicKey );
    if ( !haveKey )
    {
        memset( m_staticPrivateKey, 0, sizeof( m_staticPrivateKey ) );
        memset( m_staticPublicKey, 0, sizeof( m_staticPublicKey ) );
    }
    m_haveStaticKey = haveKey;
    return haveKey;
}

/*-------------------------------------------------------------------------*/

bool
CDRNNode::GetPublicKey( UInt8* publicKey ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );
    if ( !m_haveStaticKey )
        return false;
    memcpy( publicKey, m_staticPublicKey, DRN_HANDSHAKE_KEY_SIZE );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNNode::GetPublicKeyString( void ) const
{GUCEF_TRACE;

    UInt8 publicKey[ DRN_HANDSHAKE_KEY_SIZE ];
    if ( !GetPublicKey( publicKey ) )
        return CORE::CString::Empty;
    return CDRNCrypto::ToHex( publicKey, DRN_HANDSHAKE_KEY_SIZE );
}

/*-------------------------------------------------------------------------*/

CDRNEndpointList
CDRNNode::GetPublishedEndpoints( void ) const
{GUCEF_TRACE;

    CDRNEndpointList advertised = GetAdvertisedEndpoints();
    CORE::CString key = GetPublicKeyString();
    if ( key.IsNULLOrEmpty() )
        return advertised;
    CDRNEndpointList published;
    for ( UInt32 i=0; i<advertised.GetCount(); ++i )
    {
        CDRNEndpoint endpoint = advertised.Get( i );
        if ( endpoint.GetChannelType() != CDRNEndpoint::ChannelTypeRelay )
        {
            // the relay endpoints carry what the relay advertised, these what this node does
            endpoint.GetParams().Set( "key", CORE::CVariant( key ) );
            endpoint.GetParams().Set( "policy", CORE::CVariant( m_settings.securityPolicy.GetPolicyHashString() ) );
        }
        else
        {
            endpoint.GetParams().Set( "key", CORE::CVariant( key ) );
        }
        published.Add( endpoint );
    }
    return published;
}

/*-------------------------------------------------------------------------*/

bool
CDRNNode::SetSettings( const CDRNNodeSettings& settings )
{GUCEF_TRACE;

    if ( settings.nodeId.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNNode: A node needs a node id" );
        return false;
    }
    if ( !settings.securityPolicy.IsAuthenticatorAllowed( settings.auth.authenticatorType ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNNode: The security policy does not allow the authenticator \"" << settings.auth.authenticatorType << "\"" );
        return false;
    }

    bool threadingChange = settings.ownThread != m_settings.ownThread;
    if ( threadingChange )
    {
        MT::CScopeMutex lock( m_nodeLock );
        if ( !m_links.empty() || !m_listeners.empty() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNNode: The threading of a node cannot change while it has listeners or links" );
            return false;
        }
    }

    if ( threadingChange && !settings.ownThread )
        StopOwnThread();

    {
        MT::CScopeMutex lock( m_nodeLock );
        CIDRNAuthenticatorPtr authenticator = settings.auth.CreateAuthenticator();
        if ( authenticator.IsNULL() )
            return false;
        if ( settings.securityPolicy.requireSignedTokens && settings.auth.authenticatorType == CDRNTokenAuthenticator::MethodName &&
             !static_cast< CDRNTokenAuthenticator* >( authenticator.GetPointerAlways() )->IsVerifyingSignedTokens() &&
             !settings.auth.authenticatorConfig.GetAttributeValueOrChildValueByName( "token" ).IsInitialized() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNNode: The security policy requires signed tokens and the token authenticator has no \"issuerPublicKey\"" );
            return false;
        }

        bool keyChange = settings.staticPrivateKey != m_settings.staticPrivateKey || settings.staticKeyFile != m_settings.staticKeyFile;
        m_settings = settings;
        if ( keyChange )
        {
            m_haveStaticKey = false;
            memset( m_staticPrivateKey, 0, sizeof( m_staticPrivateKey ) );
            memset( m_staticPublicKey, 0, sizeof( m_staticPublicKey ) );
        }
        m_authenticator = authenticator;
        UpdateLinkSettings();
        if ( m_settings.relayEnabled && GUCEF_NULL == m_relayService )
            m_relayService = GUCEF_NEW CDRNRelayService( *this );
        else
        if ( !m_settings.relayEnabled && GUCEF_NULL != m_relayService )
        {
            GUCEF_DELETE m_relayService;
            m_relayService = GUCEF_NULL;
        }
        if ( !m_pulseGenerator.IsNULL() )
            m_pulseGenerator->RequestPeriodicPulses( this, m_settings.pulseIntervalMs );
    }

    if ( threadingChange && settings.ownThread )
        return StartOwnThread();
    return true;
}

/*-------------------------------------------------------------------------*/

const CDRNNodeSettings&
CDRNNode::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CDRNNode::StartOwnThread( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_thread )
        return true;

    {
        MT::CScopeMutex lock( m_nodeLock );
        if ( !m_pulseGenerator.IsNULL() )
        {
            m_pulseGenerator->RequestStopOfPeriodicUpdates( this );
            m_observer.UnsubscribeFrom( *m_pulseGenerator.GetPointerAlways() );
        }
        m_pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
        m_observer.SubscribeTo( m_pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent );
    }

    m_thread = GUCEF_NEW CDRNNodeThread( this );
    if ( !m_thread->Activate( GUCEF_NULL, 1, 1 ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNNode: Failed to start the thread of the node" );
        GUCEF_DELETE m_thread;
        m_thread = GUCEF_NULL;
        return false;
    }
    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNNode: Running on a thread of its own" );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::StopOwnThread( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_thread )
        return;

    m_thread->Deactivate( false, true );
    GUCEF_DELETE m_thread;
    m_thread = GUCEF_NULL;

    // the channels let go of on other threads are destroyed now, from now on wherever they are let go of
    m_channelReaper->SetOwnerThreadId( 0 );
    m_channelReaper->Reap();

    // from now on the node works on the calling thread
    if ( !m_pulseGenerator.IsNULL() )
        m_pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentTaskID() );
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::RunThreadCycle( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );
    m_pulseGenerator->ForceDirectPulse();
}

/*-------------------------------------------------------------------------*/

bool
CDRNNode::Start( void )
{GUCEF_TRACE;

    CORE::CString whyNot;
    if ( !m_settings.securityPolicy.CanBeHonoured( whyNot ) || m_settings.nodeId.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNNode: Cannot start: " << ( whyNot.IsNULLOrEmpty() ? CORE::CString( "no node id" ) : whyNot ) );
        return false;
    }

    if ( m_settings.ownThread && GUCEF_NULL == m_thread )
    {
        if ( !StartOwnThread() )
            return false;
    }

    bool totalSuccess = true;
    const CDRNEndpointList::TEndpointVector& endpoints = m_settings.listenEndpoints.GetEndpoints();
    for ( UInt32 i=0; i<endpoints.size(); ++i )
        totalSuccess = Listen( endpoints[ i ] ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::Stop( void )
{GUCEF_TRACE;

    // the thread takes the node lock, it has to end before the clean up
    StopOwnThread();

    MT::CScopeMutex lock( m_nodeLock );
    m_stopping = true;

    for ( UInt32 i=0; i<m_sessions.size(); ++i )
        m_sessions[ i ]->Shutdown();

    if ( GUCEF_NULL != m_relayClient )
        m_relayClient->Shutdown();

    for ( UInt32 i=0; i<m_connectors.size(); ++i )
    {
        m_observer.UnsubscribeFrom( *m_connectors[ i ].GetPointerAlways() );
        m_connectors[ i ]->Cancel();
    }
    m_connectors.clear();

    TDRNPeerLinkVector links;
    links.swap( m_links );
    for ( UInt32 i=0; i<links.size(); ++i )
    {
        CDRNPeerLinkPtr& link = links[ i ];
        m_observer.UnsubscribeFrom( *link.GetPointerAlways() );
        if ( !link->IsClosed() )
        {
            link->Close( DRN_LINKREASON_SHUTDOWN, "the node is shutting down" );
            link->Flush();
        }
        link->SetSessionBinding( GUCEF_NULL, 0 );
    }
    links.clear();
    m_closedLinks.clear();

    TListenerVector listeners;
    listeners.swap( m_listeners );
    for ( UInt32 i=0; i<listeners.size(); ++i )
    {
        m_observer.UnsubscribeFrom( *listeners[ i ].GetPointerAlways() );
        listeners[ i ]->StopListening();
    }
    listeners.clear();

    m_stopping = false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNNode::Listen( const CDRNEndpoint& local )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );

    CORE::CString whyNot;
    if ( !m_settings.securityPolicy.CanBeHonoured( whyNot ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNNode: Cannot listen on " << local.ToString() << ": " << whyNot );
        return false;
    }
    if ( !m_settings.securityPolicy.IsChannelTypeAllowed( local.GetChannelType() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNNode: The security policy does not allow channel type \"" << local.GetChannelType() << "\"" );
        return false;
    }

    CIDRNChannelListenerPtr listener( CDRNGlobal::Instance()->GetChannelListenerFactory().Create( local.GetChannelType() ) );
    if ( listener.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNNode: No listener for channel type \"" << local.GetChannelType() << "\"" );
        return false;
    }
    listener->SetPulseGenerator( m_pulseGenerator );
    listener->ApplySecurityPolicy( m_settings.securityPolicy );
    m_observer.SubscribeTo( listener.GetPointerAlways(), CIDRNChannelListener::ChannelAcceptedEvent );
    if ( !listener->Listen( local ) )
    {
        m_observer.UnsubscribeFrom( *listener.GetPointerAlways() );
        return false;
    }
    m_listeners.push_back( listener );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::StopListening( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );
    for ( UInt32 i=0; i<m_listeners.size(); ++i )
        m_listeners[ i ]->StopListening();
}

/*-------------------------------------------------------------------------*/

CDRNEndpointList
CDRNNode::GetListenEndpoints( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );
    CDRNEndpointList endpoints;
    for ( UInt32 i=0; i<m_listeners.size(); ++i )
    {
        if ( m_listeners[ i ]->IsListening() )
            endpoints.Add( m_listeners[ i ]->GetListenEndpoint() );
    }
    // the relay comes last: a peer that can reach the node directly does so
    if ( GUCEF_NULL != m_relayClient )
        m_relayClient->GetBoundEndpoints( endpoints );
    return endpoints;
}

/*-------------------------------------------------------------------------*/

CDRNConnectorPtr
CDRNNode::Connect( const CDRNEndpointList& remote, const CDRNAuthSettings* credentials )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );

    CDRNConnectorPtr connector = CDRNConnector::CreateSharedObj();
    connector->Initialize( this, remote, GUCEF_NULL != credentials ? *credentials : m_settings.auth, m_settings.connectTimeoutMs );
    m_connectors.push_back( connector );
    connector->Start();
    return connector;
}

/*-------------------------------------------------------------------------*/

CDRNConnectorPtr
CDRNNode::ConnectToService( const CDRNEndpointList& remote, const UInt8* servicePolicyHash )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );

    CDRNConnectorPtr connector = CDRNConnector::CreateSharedObj();
    connector->Initialize( this, remote, m_settings.auth, m_settings.connectTimeoutMs );
    connector->SetServicePolicyHash( servicePolicyHash );
    m_connectors.push_back( connector );
    connector->Start();
    return connector;
}

/*-------------------------------------------------------------------------*/

CDRNPeerLinkPtr
CDRNNode::CreateLink( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );

    CDRNPeerLinkPtr link = CDRNPeerLink::CreateSharedObj();
    link->SetChannelReaper( m_channelReaper );
    m_observer.SubscribeTo( link.GetPointerAlways() );
    m_links.push_back( link );
    return link;
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::GetLinks( TDRNPeerLinkVector& links ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );
    links.clear();
    for ( UInt32 i=0; i<m_links.size(); ++i )
    {
        if ( !m_links[ i ]->IsClosed() )
            links.push_back( m_links[ i ] );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNode::GetLinkCount( bool operationalOnly ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );
    UInt32 count = 0;
    for ( UInt32 i=0; i<m_links.size(); ++i )
    {
        if ( operationalOnly ? m_links[ i ]->IsOperational() : !m_links[ i ]->IsClosed() )
            ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

CDRNSession*
CDRNNode::CreateSession( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );
    CDRNSession* session = GUCEF_NEW CDRNSession( *this );
    m_sessions.push_back( session );
    return session;
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::DestroySession( CDRNSession* session )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );
    for ( UInt32 i=0; i<m_sessions.size(); ++i )
    {
        if ( m_sessions[ i ] == session )
        {
            m_sessions.erase( m_sessions.begin() + i );
            session->Shutdown();
            GUCEF_DELETE session;
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

const CDRNNode::TSessionVector&
CDRNNode::GetSessions( void ) const
{GUCEF_TRACE;

    return m_sessions;
}

/*-------------------------------------------------------------------------*/

CDRNSession*
CDRNNode::FindSession( const CORE::CString& sessionName ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_sessions.size(); ++i )
    {
        CDRNSession* session = m_sessions[ i ];
        if ( session->GetName() == sessionName && session->IsActive() )
            return session;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::SetAccessPolicy( CIDRNAccessPolicy* policy )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );
    m_accessPolicy = GUCEF_NULL != policy ? policy : m_defaultAccessPolicy;
}

/*-------------------------------------------------------------------------*/

CIDRNAccessPolicy&
CDRNNode::GetAccessPolicy( void )
{GUCEF_TRACE;

    return *m_accessPolicy;
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::SetBanList( CIDRNBanList* banList )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );
    m_banList = banList;
    m_linkSettings.banList = banList;
}

/*-------------------------------------------------------------------------*/

CIDRNBanList*
CDRNNode::GetBanList( void ) const
{GUCEF_TRACE;

    return m_banList;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNode::GetPendingHandshakeCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );
    UInt32 count = 0;
    for ( UInt32 i=0; i<m_links.size(); ++i )
    {
        const CDRNPeerLink& link = *m_links[ i ];
        if ( !link.IsInitiator() && !link.IsClosed() && !link.IsOperational() && CDRNPeerLink::LINKSTATE_CLOSING != link.GetState() )
            ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNode::GetRefusedAcceptCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );
    return m_refusedForPolicy + m_refusedForBan + m_refusedForPending + m_refusedForRate;
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::GetTelemetry( CORE::CDataNode& telemetry ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );
    telemetry.SetName( "DRNNodeTelemetry" );
    telemetry.SetAttribute( "nodeId", m_settings.nodeId );
    telemetry.SetAttribute( "policyHash", m_settings.securityPolicy.GetPolicyHashString() );
    telemetry.SetAttribute( "links", (UInt32) m_links.size() );
    telemetry.SetAttribute( "pendingHandshakes", GetPendingHandshakeCount() );
    telemetry.SetAttribute( "refusedForPolicy", m_refusedForPolicy );
    telemetry.SetAttribute( "refusedForBan", m_refusedForBan );
    telemetry.SetAttribute( "refusedForPendingHandshakes", m_refusedForPending );
    telemetry.SetAttribute( "refusedForHandshakeRate", m_refusedForRate );
    for ( UInt32 i=0; i<m_links.size(); ++i )
    {
        CORE::CDataNode* linkNode = telemetry.AddChild( "Link" );
        if ( GUCEF_NULL != linkNode )
            m_links[ i ]->GetTelemetry( *linkNode );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNNode::GetAddressKey( const CDRNEndpoint& remote )
{GUCEF_TRACE;

    // the host without the port; a relayed peer is known by its relay address only
    if ( remote.GetChannelType().Equals( CDRNEndpoint::ChannelTypeRelay, false ) )
        return remote.GetAddress();
    CORE::CString host = remote.GetHost();
    return host.IsNULLOrEmpty() ? remote.GetAddress() : host;
}

/*-------------------------------------------------------------------------*/

bool
CDRNNode::TakeHandshakeSlot( const CORE::CString& address, UInt64 nodeNow )
{GUCEF_TRACE;

    UInt32 maxPerMinute = m_settings.securityPolicy.limits.maxHandshakesPerAddressPerMinute;
    if ( 0 == maxPerMinute )
        return true;

    if ( m_handshakeWindows.size() >= MaxTrackedAddresses && m_handshakeWindows.find( address ) == m_handshakeWindows.end() )
    {
        PurgeHandshakeWindows( nodeNow );
        if ( m_handshakeWindows.size() >= MaxTrackedAddresses )
            return false;   // too many addresses at once: no new ones until the windows run out
    }

    CHandshakeWindow& window = m_handshakeWindows[ address ];
    if ( 0 == window.count || nodeNow >= window.windowStart + HandshakeWindowUs )
    {
        window.windowStart = nodeNow;
        window.count = 0;
    }
    if ( window.count >= maxPerMinute )
        return false;
    ++window.count;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::PurgeHandshakeWindows( UInt64 nodeNow )
{GUCEF_TRACE;

    THandshakeWindowMap::iterator i = m_handshakeWindows.begin();
    while ( i != m_handshakeWindows.end() )
    {
        if ( nodeNow >= (*i).second.windowStart + HandshakeWindowUs )
            m_handshakeWindows.erase( i++ );
        else
            ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNNode::MayAccept( const CIDRNChannelPtr& channel, UInt64 nodeNow )
{GUCEF_TRACE;

    const CDRNSecurityPolicy& policy = m_settings.securityPolicy;
    CDRNEndpoint remote = channel->GetRemoteEndpoint();
    if ( !policy.IsChannelTypeAllowed( channel->GetChannelType() ) )
    {
        ++m_refusedForPolicy;
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNNode: Refused a channel of type \"" << channel->GetChannelType() << "\" from " << remote.ToString() << ": not allowed by the security policy" );
        return false;
    }

    CORE::CString address = GetAddressKey( remote );
    CDRNBanEntry ban;
    if ( GUCEF_NULL != m_banList && m_banList->IsAddressBanned( address, &ban ) )
    {
        ++m_refusedForBan;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRNNode: Refused a channel from banned address " << address << ": " << ban.reason );
        return false;
    }

    UInt32 maxPending = policy.limits.maxPendingHandshakes;
    if ( 0 != maxPending && GetPendingHandshakeCount() >= maxPending )
    {
        ++m_refusedForPending;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNNode: Refused a channel from " << remote.ToString() << ": " << maxPending << " handshakes are pending already" );
        return false;
    }

    if ( !TakeHandshakeSlot( address, nodeNow ) )
    {
        ++m_refusedForRate;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNNode: Refused a channel from " << address << ": too many handshakes from that address" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CDRNNode::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CDRNNode::GetAppPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_appPulseGenerator;
}

/*-------------------------------------------------------------------------*/

bool
CDRNNode::IsRunningOwnThread( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_thread;
}

/*-------------------------------------------------------------------------*/

MT::CMutex&
CDRNNode::GetLock( void ) const
{GUCEF_TRACE;

    return m_nodeLock;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNNode::GetNodeTime( void ) const
{GUCEF_TRACE;

    return CDRNClock::NowInMicroseconds();
}

/*-------------------------------------------------------------------------*/

const CDRNLinkSettings&
CDRNNode::GetLinkSettings( void ) const
{GUCEF_TRACE;

    return m_linkSettings;
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::OnChannelAccepted( const CIDRNChannelPtr& channel )
{GUCEF_TRACE;

    if ( m_stopping || channel.IsNULL() )
        return;

    // nothing of a peer is kept before it passes the policy, the ban list and the handshake limits
    if ( !MayAccept( channel, CDRNClock::NowInMicroseconds() ) )
    {
        CIDRNChannelPtr refused = channel;
        refused->Close();
        return;
    }

    if ( !m_haveStaticKey )
        UpdateLinkSettings();
    CDRNPeerLinkPtr link = CreateLink();
    CDRNLinkSettings settings = m_linkSettings;
    settings.linkId = m_nextLinkId++;
    link->StartAccept( channel, settings, m_authenticator );
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::OnLinkMessage( CDRNPeerLink* link, const CDRNLinkMessage& message )
{GUCEF_TRACE;

    switch ( message.msgClass )
    {
        case DRN_MSGCLASS_SESSION:
        case DRN_MSGCLASS_APP:
        case DRN_MSGCLASS_REPL:
        case DRN_MSGCLASS_STREAM:
        case DRN_MSGCLASS_BULK:
        {
            CDRNSession* session = link->GetSession();
            if ( GUCEF_NULL != session )
            {
                session->OnLinkMessage( link, message );
                return;
            }

            if ( DRN_MSGCLASS_SESSION == message.msgClass && DRN_SESSION_JOIN == message.opcode )
            {
                CDRNWireReader reader( message.payload );
                CORE::CString sessionName;
                reader.ReadString( sessionName );
                session = FindSession( sessionName );
                if ( GUCEF_NULL != session )
                {
                    session->OnJoinRequest( link, message );
                    return;
                }

                CORE::CDynamicBuffer reply;
                CDRNWireWriter writer( reply );
                writer.WriteVarUInt32( DRN_JOIN_NO_SUCH_SESSION );
                writer.WriteString( "no session \"" + sessionName + "\" here" );
                link->Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_SESSION, DRN_SESSION_JOINED, reply );
                link->Flush();
            }
            return;
        }
        case DRN_MSGCLASS_RELAY:
        {
            if ( GUCEF_NULL != m_relayClient && m_relayClient->OnLinkMessage( link, message ) )
                return;
            if ( GUCEF_NULL != m_relayService && m_relayService->OnLinkMessage( link, message ) )
                return;
            if ( DRN_RELAY_BIND == message.opcode )
            {
                CORE::CDynamicBuffer reply;
                CDRNWireWriter writer( reply );
                writer.WriteVarUInt32( DRN_RELAY_BIND_NO_SERVICE );
                writer.WriteVarUInt32( 0 );
                writer.WriteString( CORE::CString::Empty );
                link->Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_RELAY, DRN_RELAY_BIND, reply );
            }
            return;
        }
        default:
        {
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::OnRelayBindingChanged( const CDRNEndpoint& endpoint, bool bound, const CORE::CString& reason )
{GUCEF_TRACE;

    CORE::TCloneableString eventData( bound ? endpoint.ToString() : reason );
    if ( IsRunningOwnThread() )
        NotifyObserversFromThread( bound ? RelayBoundEvent : RelayBindingLostEvent, &eventData );
    else
        NotifyObservers( bound ? RelayBoundEvent : RelayBindingLostEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNode::BindAtRelay( const CDRNEndpoint& relayService, const CORE::CString& realm, UInt32 requestedId, const CORE::CString* token )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );
    return m_relayClient->Bind( relayService, realm, requestedId, GUCEF_NULL != token ? *token : m_settings.relayToken );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNode::IsBoundAtRelay( const CDRNEndpoint& relayService, const CORE::CString& realm ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );
    return m_relayClient->IsBound( relayService, realm );
}

/*-------------------------------------------------------------------------*/

CDRNEndpointList
CDRNNode::GetRelayEndpoints( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );
    CDRNEndpointList endpoints;
    m_relayClient->GetBoundEndpoints( endpoints );
    return endpoints;
}

/*-------------------------------------------------------------------------*/

CDRNRelayService*
CDRNNode::GetRelayService( void ) const
{GUCEF_TRACE;

    return m_relayService;
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::SetAdvertisedEndpoints( const CDRNEndpointList& endpoints )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );
    m_advertisedEndpoints = endpoints;
}

/*-------------------------------------------------------------------------*/

CDRNEndpointList
CDRNNode::GetAdvertisedEndpoints( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );
    if ( m_advertisedEndpoints.IsEmpty() )
        return GetListenEndpoints();
    return m_advertisedEndpoints;
}

/*-------------------------------------------------------------------------*/

CIDRNChannelPtr
CDRNNode::CreateChannel( const CDRNEndpoint& remote )
{GUCEF_TRACE;

    if ( remote.GetChannelType() == CDRNEndpoint::ChannelTypeRelay )
        return m_relayClient->CreateChannel( remote );
    return CIDRNChannelPtr( CDRNGlobal::Instance()->GetChannelFactory().Create( remote.GetChannelType() ) );
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::OnLinkClosed( CDRNPeerLink* link, const CDRNLinkClosedInfo& info )
{GUCEF_TRACE;

    CDRNPeerLinkPtr linkPtr = link->GetSharedPtr();
    CDRNSession* session = link->GetSession();
    if ( GUCEF_NULL != session )
        session->OnLinkClosed( link );
    if ( GUCEF_NULL != m_relayService )
        m_relayService->OnLinkClosed( link );
    if ( GUCEF_NULL != m_relayClient )
        m_relayClient->OnLinkClosed( link );

    CDRNNodeLinkEventData eventData;
    eventData.GetData().link = linkPtr;
    eventData.GetData().closedInfo = info;
    if ( IsRunningOwnThread() )
        NotifyObserversFromThread( LinkClosedEvent, &eventData );
    else
        NotifyObservers( LinkClosedEvent, &eventData );

    // let go of the link at the next pulse, not from within its own notification
    m_closedLinks.push_back( linkPtr );
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::OnPulse( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_nodeLock );
    UInt64 now = CDRNClock::NowInMicroseconds();

    m_channelReaper->Reap();

    if ( now >= m_nextWindowPurge )
    {
        m_nextWindowPurge = now + 10000000;
        PurgeHandshakeWindows( now );
    }

    if ( GUCEF_NULL != m_relayService )
        m_relayService->Tick( now );
    if ( GUCEF_NULL != m_relayClient )
        m_relayClient->Tick( now );

    // links that closed since the last pulse go now
    if ( !m_closedLinks.empty() )
    {
        TDRNPeerLinkVector closedLinks;
        closedLinks.swap( m_closedLinks );
        for ( UInt32 i=0; i<closedLinks.size(); ++i )
        {
            for ( UInt32 n=0; n<m_links.size(); ++n )
            {
                if ( m_links[ n ] == closedLinks[ i ] )
                {
                    m_observer.UnsubscribeFrom( *m_links[ n ].GetPointerAlways() );
                    m_links.erase( m_links.begin() + n );
                    break;
                }
            }
        }
        closedLinks.clear();
    }

    for ( UInt32 i=0; i<m_connectors.size(); )
    {
        CDRNConnectorPtr connector = m_connectors[ i ];
        connector->Tick( now );
        if ( connector->IsDone() )
        {
            if ( i < m_connectors.size() && m_connectors[ i ] == connector )
                m_connectors.erase( m_connectors.begin() + i );
        }
        else
        {
            ++i;
        }
    }

    for ( UInt32 i=0; i<m_sessions.size(); ++i )
        m_sessions[ i ]->Tick( now );

    for ( UInt32 i=0; i<m_links.size(); ++i )
    {
        CDRNPeerLinkPtr link = m_links[ i ];
        link->Tick( now );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::OnInternalNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( CORE::CPulseGenerator::PulseEvent == eventid )
    {
        OnPulse();
        return;
    }

    MT::CScopeMutex lock( m_nodeLock );

    if ( CDRNPeerLink::MessageReceivedEvent == eventid )
    {
        if ( GUCEF_NULL != eventdata )
            OnLinkMessage( static_cast< CDRNPeerLink* >( notifier ), static_cast< CDRNLinkMessageEventData* >( eventdata )->GetData() );
    }
    else
    if ( CDRNPeerLink::LinkOperationalEvent == eventid )
    {
        CDRNNodeLinkEventData eventData;
        eventData.GetData().link = static_cast< CDRNPeerLink* >( notifier )->GetSharedPtr();
        if ( IsRunningOwnThread() )
            NotifyObserversFromThread( LinkOperationalEvent, &eventData );
        else
            NotifyObservers( LinkOperationalEvent, &eventData );
    }
    else
    if ( CDRNPeerLink::LinkClosedEvent == eventid )
    {
        if ( GUCEF_NULL != eventdata )
            OnLinkClosed( static_cast< CDRNPeerLink* >( notifier ), static_cast< CDRNLinkClosedEventData* >( eventdata )->GetData() );
    }
    else
    if ( CDRNPeerLink::LinkRefusedEvent == eventid )
    {
        CDRNNodeLinkEventData eventData;
        eventData.GetData().link = static_cast< CDRNPeerLink* >( notifier )->GetSharedPtr();
        if ( GUCEF_NULL != eventdata )
            eventData.GetData().closedInfo = static_cast< CDRNLinkClosedEventData* >( eventdata )->GetData();
        if ( IsRunningOwnThread() )
            NotifyObserversFromThread( LinkRefusedEvent, &eventData );
        else
            NotifyObservers( LinkRefusedEvent, &eventData );
    }
    else
    if ( CDRNPeerLink::LatencyUpdateEvent == eventid )
    {
        CDRNNodeLinkEventData eventData;
        eventData.GetData().link = static_cast< CDRNPeerLink* >( notifier )->GetSharedPtr();
        if ( GUCEF_NULL != eventdata )
            eventData.GetData().latencyInfo = static_cast< CDRNLatencyInfoEventData* >( eventdata )->GetData();
        if ( IsRunningOwnThread() )
            NotifyObserversFromThread( LatencyUpdateEvent, &eventData );
        else
            NotifyObservers( LatencyUpdateEvent, &eventData );
    }
    else
    if ( CDRNPeerLink::PeerLimitExceededEvent == eventid )
    {
        ForwardLinkEvent( PeerLimitExceededEvent, static_cast< CDRNPeerLink* >( notifier ), eventdata );
    }
    else
    if ( CDRNPeerLink::ClockAnomalyEvent == eventid )
    {
        ForwardLinkEvent( ClockAnomalyEvent, static_cast< CDRNPeerLink* >( notifier ), eventdata );
    }
    else
    if ( CDRNPeerLink::TrafficAnomalyEvent == eventid )
    {
        ForwardLinkEvent( TrafficAnomalyEvent, static_cast< CDRNPeerLink* >( notifier ), eventdata );
    }
    else
    if ( CIDRNChannelListener::ChannelAcceptedEvent == eventid )
    {
        if ( GUCEF_NULL != eventdata )
            OnChannelAccepted( static_cast< CDRNChannelEventData* >( eventdata )->GetData() );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNNode::ForwardLinkEvent( const CORE::CEvent& nodeEvent, CDRNPeerLink* link, CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    CDRNNodeLinkEventData eventData;
    eventData.GetData().link = link->GetSharedPtr();
    if ( GUCEF_NULL != eventdata )
        eventData.GetData().anomaly = static_cast< CDRNLinkAnomalyEventData* >( eventdata )->GetData();
    if ( IsRunningOwnThread() )
        NotifyObserversFromThread( nodeEvent, &eventData );
    else
        NotifyObservers( nodeEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNNode::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNNode";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
