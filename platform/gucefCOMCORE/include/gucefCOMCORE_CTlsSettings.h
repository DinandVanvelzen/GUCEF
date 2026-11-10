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

#ifndef GUCEF_COMCORE_CTLSSETTINGS_H
#define GUCEF_COMCORE_CTLSSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_COMCORE_TLSTYPES_H
#include "gucefCOMCORE_TlsTypes.h"
#define GUCEF_COMCORE_TLSTYPES_H
#endif /* GUCEF_COMCORE_TLSTYPES_H ? */

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
 *  How a TLS client connection is set up. The defaults are the safe ones: TLS 1.2 or newer, the certificate of
 *  the peer is verified against the trust store of the system and has to name the host (subject alternative
 *  names, never the common name).
 *
 *  There is no setting that allows a version below TLS 1.2. Verification can only be switched off with
 *  DisableVerificationInsecurely(), which every connection logs a warning for, and configuration cannot switch
 *  it off by itself: LoadConfig() ignores a verifyMode of "none".
 *
 *  In config (section TlsClient): minVersion ("1.2" or "1.3"), maxVersion, useSystemTrust, caBundlePath,
 *  caDir, alpn (comma separated), handshakeTimeoutInMs, allowResumption, sessionCacheSize.
 */
class GUCEF_COMCORE_EXPORT_CPP CTlsSettings : public CORE::CIConfigurable
{
    public:

    typedef CORE::CString::StringVector TStringVector;

    static const UInt32 DefaultHandshakeTimeoutInMs = 10000;
    static const UInt32 DefaultSessionCacheSize = 32;

    CTlsSettings( void );
    CTlsSettings( const CTlsSettings& src );
    virtual ~CTlsSettings() GUCEF_VIRTUAL_OVERRIDE;
    CTlsSettings& operator=( const CTlsSettings& src );

    /** Lowest version to accept. The floor is TLS 1.2: a lower value is raised to it */
    void SetMinVersion( ETlsVersion version );
    ETlsVersion GetMinVersion( void ) const;

    /** Highest version to use, TLSVERSION_ANY for the newest the provider has. Not below the minimum */
    void SetMaxVersion( ETlsVersion version );
    ETlsVersion GetMaxVersion( void ) const;

    /**
     *  Turns the verification of the certificate of the peer off: anyone can then impersonate the server.
     *  For tests and for endpoints on a closed network only. Every connection made with these settings
     *  logs a warning.
     */
    void DisableVerificationInsecurely( void );
    bool IsVerificationEnabled( void ) const;

    /** Whether the trust store of the operating system is used, true by default. With false only the CA given here is trusted */
    void SetUseSystemTrust( bool useSystemTrust );
    bool GetUseSystemTrust( void ) const;

    /** File with the PEM certificates of authorities to trust, added to the trust store of the system. A file that cannot be read fails the connection */
    void SetCaBundlePath( const CORE::CString& path );
    const CORE::CString& GetCaBundlePath( void ) const;

    /** Directory of hashed PEM certificates (c_rehash style) to trust, added like the bundle */
    void SetCaDir( const CORE::CString& path );
    const CORE::CString& GetCaDir( void ) const;

    /** PEM certificates held in memory, added like the bundle */
    void SetCaBundleData( const CORE::CString& pem );
    const CORE::CString& GetCaBundleData( void ) const;

    /**
     *  The name sent as SNI and the certificate is verified for. Empty: the host the connection is made to.
     *  An IP address is verified against the IP addresses of the certificate and no SNI is sent.
     */
    void SetServerName( const CORE::CString& serverName );
    const CORE::CString& GetServerName( void ) const;

    /** Application protocols to offer (ALPN) in order of preference, for HTTP "http/1.1" */
    void SetAlpnProtocols( const TStringVector& protocols );
    void AddAlpnProtocol( const CORE::CString& protocol );
    const TStringVector& GetAlpnProtocols( void ) const;

    void SetHandshakeTimeoutInMs( UInt32 timeoutInMs );
    UInt32 GetHandshakeTimeoutInMs( void ) const;

    /** Whether sessions are kept in memory to resume them on a later connection to the same host, true by default. Never written anywhere */
    void SetAllowResumption( bool allowResumption );
    bool GetAllowResumption( void ) const;

    /** How many sessions the cache of a context holds, 0 turns the cache off */
    void SetSessionCacheSize( UInt32 size );
    UInt32 GetSessionCacheSize( void ) const;

    /**
     *  Identifies what the context of a provider depends on: versions, trust, ALPN, verification and the
     *  session cache. Settings with the same key can share a context, the server name is not part of it.
     */
    CORE::CString GetContextKey( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    ETlsVersion m_minVersion;
    ETlsVersion m_maxVersion;
    bool m_verify;
    bool m_useSystemTrust;
    CORE::CString m_caBundlePath;
    CORE::CString m_caDir;
    CORE::CString m_caBundleData;
    CORE::CString m_serverName;
    TStringVector m_alpnProtocols;
    UInt32 m_handshakeTimeoutInMs;
    bool m_allowResumption;
    UInt32 m_sessionCacheSize;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CTLSSETTINGS_H ? */
