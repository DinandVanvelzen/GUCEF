/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CSERVICEANNOUNCEMENT_H
#define GUCEF_COMCORE_CSERVICEANNOUNCEMENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYENTRY_H
#include "gucefCOMCORE_CDiscoveryEntry.h"
#define GUCEF_COMCORE_CDISCOVERYENTRY_H
#endif /* GUCEF_COMCORE_CDISCOVERYENTRY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a service tells the local network about itself so a client can offer it as something to connect to.
 *
 *  An announcement is a convenience that produces candidates: it is heard from an address anyone on the network can
 *  forge, so it never carries a secret and it never makes a peer trusted. A client still has to authenticate the
 *  service it connects to (for example a pinned key after pairing); an announcement only says where to try.
 *  Keep it free of what eases an attack: no build or product version strings, only the protocol version.
 *
 *  - serviceType: the kind of service as "_<name>._tcp" or "_<name>._udp", by convention "_gucef-<product>._tcp"
 *    (IsValidServiceType()).
 *  - instanceId: stable per service instance (survives restarts), the identity of the announcement with the type.
 *  - displayName: what a person sees in a list.
 *  - endpoints: "scheme://host:port" each, at most MaxEndpoints. A host of 0.0.0.0 asks the receiver to put in the
 *    address the announcement came from (MakeEndpoint() with an empty host does that). The first endpoint is the one
 *    protocols with a single port (mDNS SRV) carry.
 *  - protocolVersion: "major.minor" of the protocol a client speaks to the service, attribute "pv".
 *  - keyFingerprint: optional fingerprint of the public key of the service, attribute "kfp". Public information that lets
 *    a client that pinned a key recognise its service and lets a person compare it while pairing; it proves nothing.
 *  - properties: further small public values; keys that look like they hold a secret are refused (IsForbiddenPropertyKey()).
 *  - ttlInSecs: how long a browser keeps it without hearing it again; announced again every TTL/3.
 *
 *  Config (LoadConfig/SaveConfig): the same names, endpoints and properties as children.
 */
class GUCEF_COMCORE_EXPORT_CPP CServiceAnnouncement : public CORE::CIConfigurable
{
    public:

    static const char* ProtocolVersionKey;      /**< "pv" */
    static const char* KeyFingerprintKey;       /**< "kfp" */
    static const UInt32 MaxEndpoints;           /**< 8 */
    static const UInt32 MaxEncodedSizeInBytes;  /**< 1400: what fits a single datagram of the UDP discovery clients */
    static const UInt32 DefaultTtlInSecs;       /**< 30 */
    static const UInt32 MinTtlInSecs;           /**< 3 */

    CORE::CString serviceType;
    CORE::CString instanceId;
    CORE::CString displayName;
    CORE::CString::StringVector endpoints;
    CORE::CString protocolVersion;
    CORE::CString keyFingerprint;
    CORE::CValueList properties;
    UInt32 ttlInSecs;

    CServiceAnnouncement( void );
    CServiceAnnouncement( const CServiceAnnouncement& src );
    virtual ~CServiceAnnouncement();
    CServiceAnnouncement& operator=( const CServiceAnnouncement& src );

    /** Writes the announcement into a discovery entry, replacing its content */
    void ToEntry( CDiscoveryEntry& entry ) const;

    /** Reads an announcement from a discovery entry; false when the entry has no service type or no instance id */
    bool FromEntry( const CIDiscoveryEntry& entry );

    /**
     *  Whether the announcement can be sent: a valid service type, an instance id, at most MaxEndpoints endpoints, a TTL
     *  of at least MinTtlInSecs, no reserved or forbidden property key and an encoded size within MaxEncodedSizeInBytes
     */
    bool IsValid( CORE::CString& reason ) const;

    /** Everything except the TTL is the same */
    bool HasSameContent( const CServiceAnnouncement& other ) const;

    /** The size of the beacon of the UDP discovery clients for this announcement, in bytes */
    UInt32 GetEncodedSizeInBytes( void ) const;

    /** The major part of the protocol version ("1" of "1.4"), empty when there is none */
    CORE::CString GetProtocolMajor( void ) const;

    /** The port of the first endpoint with the given scheme (any scheme when empty), 0 when there is none */
    UInt16 GetPort( const CORE::CString& scheme = CORE::CString::Empty ) const;

    /** The identity across clients and updates: the service type plus the instance id */
    CORE::CString GetKey( void ) const;

    /** "_<label>._tcp" or "_<label>._udp" with a label of 1 to 63 letters, digits and dashes, not starting or ending with a dash */
    static bool IsValidServiceType( const CORE::CString& serviceType );

    /** The attribute keys the announcement itself uses: "pv" and "kfp" */
    static bool IsReservedPropertyKey( const CORE::CString& key );

    /** Keys that suggest a secret ("secret", "password", "passwd", "token", "credential", "privatekey", "apikey", in any case) */
    static bool IsForbiddenPropertyKey( const CORE::CString& key );

    /** "scheme://host:port"; an empty host gives 0.0.0.0, which the receiver replaces by the address the announcement came from */
    static CORE::CString MakeEndpoint( const CORE::CString& scheme, const CORE::CString& host, UInt16 port );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CSERVICEANNOUNCEMENT_H ? */
