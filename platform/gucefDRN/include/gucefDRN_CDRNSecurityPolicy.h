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

#ifndef GUCEF_DRN_CDRNSECURITYPOLICY_H
#define GUCEF_DRN_CDRNSECURITYPOLICY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#include "gucefDRN_DRNProtocol.h"
#define GUCEF_DRN_DRNPROTOCOL_H
#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */

#ifndef GUCEF_DRN_CDRNENDPOINT_H
#include "gucefDRN_CDRNEndpoint.h"
#define GUCEF_DRN_CDRNENDPOINT_H
#endif /* GUCEF_DRN_CDRNENDPOINT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a peer may cost a node: the limits of a CDRNSecurityPolicy. 0 means "no limit" unless noted otherwise.
 */
class GUCEF_DRN_EXPORT_CPP CDRNPolicyLimits
{
    public:

    UInt32 maxMessageSize;                      /**< largest message sent or received, default 16 MB */
    UInt32 maxPreAuthPacketSize;                /**< largest packet a stream channel takes before the link is operational, default 16 KB */
    UInt32 maxMessagesPerSecond;                /**< received per peer, default 20000 */
    UInt32 maxBytesPerSecond;                   /**< received per peer, default 64 MB */
    UInt32 sustainedLimitSeconds;               /**< a peer over a receive limit in this many consecutive seconds is closed, default 5 */
    UInt32 maxPendingHandshakes;                /**< accepted links still in their handshake, per node, default 256 */
    UInt32 maxHandshakesPerAddressPerMinute;    /**< accepted channels per remote address per minute, default 120 */
    UInt32 maxFragmentGroups;                   /**< messages being joined from fragments at once, per link, default 64 */
    UInt32 maxFragmentBytes;                    /**< total size of the messages being joined, per link, default 32 MB */

    CDRNPolicyLimits( void );

    bool operator==( const CDRNPolicyLimits& other ) const;
    bool operator!=( const CDRNPolicyLimits& other ) const;
};

/*-------------------------------------------------------------------------*/

/**
 *  When a link reports a peer's behaviour as an anomaly. DRN reports, the game decides. 0 switches a check off.
 */
class GUCEF_DRN_EXPORT_CPP CDRNAnomalyThresholds
{
    public:

    UInt32 clockDriftPercent;           /**< the peer's clock running this much faster or slower than ours, default 5 */
    UInt32 clockWindowMs;               /**< the drift is measured per window of this length, default 2000 */
    UInt32 clockSustainMs;              /**< ... and reported once it lasted this long, default 6000 */
    UInt32 silenceMs;                   /**< nothing from the peer for this long ... default 1000 */
    UInt32 burstPackets;                /**< ... followed by this many packets ... default 16 */
    UInt32 burstWindowMs;               /**< ... within this time is a burst after silence (a lag switch), default 100 */
    UInt32 floodPacketsPerSecond;       /**< more packets than this in one second is a flood, default 10000 */

    CDRNAnomalyThresholds( void );

    bool operator==( const CDRNAnomalyThresholds& other ) const;
    bool operator!=( const CDRNAnomalyThresholds& other ) const;
};

/*-------------------------------------------------------------------------*/

/**
 *  The security a game demands of its network traffic, section "DRNSecurityPolicy" of a config.
 *
 *  Set by the game, in code or in the game's config, in CDRNNodeSettings (links: limits, channel types,
 *  authenticators, encryption) and CDRNSessionSettings (the session: relay only, encryption, who may join over what).
 *  It is never a player setting. Every node of a game uses the same policy: GetPolicyHash() identifies it and the
 *  handshake refuses a peer whose policy hash differs.
 *
 *  requireEncryption is true by default: links are encrypted (handshake 2, CDRNPacketProtection) and a node without
 *  key agreement ("x25519") or any of the ciphers refuses to start (CanBeHonoured()). With requireEncryption false
 *  links are still encrypted when both sides can; an empty cipher list makes them plain.
 */
class GUCEF_DRN_EXPORT_CPP CDRNSecurityPolicy : public CORE::CIConfigurable
{
    public:

    static const CORE::CString ClassTypeName;

    /** Size of the policy hash, SHA-256 */
    static const UInt32 PolicyHashSize = 32;

    bool requireEncryption;                     /**< every packet protected, default true; see the class description */
    bool requireSignedTokens;                   /**< token authenticators take only tokens signed by the issuer (ed25519), default false */
    bool requireTokenPolicyHash;                /**< tokens must carry the hash of this policy (a token with another hash is always refused), default false */
    CORE::CString::StringVector ciphers;        /**< the ciphers acceptable for packet protection, most wanted first */
    CORE::CString::StringSet allowedChannelTypes;   /**< channel types links may use, empty = all */
    bool relayOnly;                             /**< members' own addresses are never revealed: only relay endpoints are passed on */
    CORE::CString::StringSet allowedAuthenticators; /**< authenticator types links may use, empty = all */
    bool statelessCookies;                      /**< a UDP listener answers an unknown address with a cookie before it keeps any state, default true */
    CDRNPolicyLimits limits;
    CDRNAnomalyThresholds anomalies;

    CDRNSecurityPolicy( void );
    CDRNSecurityPolicy( const CDRNSecurityPolicy& src );
    virtual ~CDRNSecurityPolicy();
    CDRNSecurityPolicy& operator=( const CDRNSecurityPolicy& src );

    bool operator==( const CDRNSecurityPolicy& other ) const;
    bool operator!=( const CDRNSecurityPolicy& other ) const;

    /**
     *  Whether this policy is weaker than a minimum: it does not require what the minimum requires (encryption,
     *  signed tokens, token policy hashes, relay only, stateless cookies), allows a cipher, channel type or
     *  authenticator the minimum does not (an empty set allows any), or has a limit above the minimum's (0 is no
     *  limit; a limit of 0 in the minimum is none). The anomaly thresholds are reports, not protection: not compared.
     *  @param why receives what is weaker
     */
    bool IsWeakerThan( const CDRNSecurityPolicy& minimum, CORE::CString& why ) const;

    /** True if links may use the channel type */
    bool IsChannelTypeAllowed( const CORE::CString& channelType ) const;

    /** True if links may use the authenticator type */
    bool IsAuthenticatorAllowed( const CORE::CString& authenticatorType ) const;

    /** True if the endpoint may be used and passed on under this policy (its channel type, relay only) */
    bool IsEndpointAllowed( const CDRNEndpoint& endpoint ) const;

    /**
     *  Whether this node can live up to the policy. False, with the reason, when the policy requires something DRN
     *  does not have, for example encryption while there is no packet protection.
     */
    bool CanBeHonoured( CORE::CString& whyNot ) const;

    /** Whether DRN can protect packets with one of the ciphers: key agreement and a cipher codec are there */
    bool IsPacketProtectionAvailable( void ) const;

    /**
     *  The canonical encoding the policy hash is taken over: every field in a fixed order, names lowercased, the
     *  sets sorted. Two policies with the same meaning have the same encoding.
     */
    void GetCanonicalEncoding( CORE::CDynamicBuffer& encoding ) const;

    /** SHA-256 over the canonical encoding, hash must hold PolicyHashSize bytes */
    void GetPolicyHash( UInt8* hash ) const;

    /** The policy hash as 64 lowercase hex characters */
    CORE::CString GetPolicyHashString( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNSECURITYPOLICY_H ? */
