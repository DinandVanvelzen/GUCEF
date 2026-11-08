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

#ifndef GUCEF_COMCORE_CTLSSERVERSETTINGS_H
#define GUCEF_COMCORE_CTLSSERVERSETTINGS_H

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
 *  How the server side of TLS connections is set up: the certificate the server presents and its private key, both
 *  PEM files. The certificate file holds the certificate of the server first, followed by the intermediate
 *  certificates of its chain. The key file is not encrypted: it is protected by the permissions of the file.
 *
 *  TLS is used when both files are given (IsEnabled()). The files are read when a connection needs them and read
 *  again once either file changed, so a renewed certificate is used for new connections without a restart.
 *  Connections that are open keep the certificate they started with.
 *
 *  The policy is that of the client side: TLS 1.2 or newer, there is no setting below it. Certificates of clients
 *  are not asked for.
 *
 *  ALPN: with protocols given, a client that offers protocols of which none is in the list is refused (RFC 7301),
 *  the first protocol of the list that the client offers is selected. A client that offers none is accepted.
 *
 *  In config: certificateChainPath, privateKeyPath, minVersion ("1.2" or "1.3"), maxVersion, alpn (comma
 *  separated), handshakeTimeoutInMs.
 */
class GUCEF_COMCORE_EXPORT_CPP CTlsServerSettings : public CORE::CIConfigurable
{
    public:

    typedef CORE::CString::StringVector TStringVector;

    static const UInt32 DefaultHandshakeTimeoutInMs = 10000;

    CTlsServerSettings( void );

    CTlsServerSettings( const CTlsServerSettings& src );

    virtual ~CTlsServerSettings() GUCEF_VIRTUAL_OVERRIDE;

    CTlsServerSettings& operator=( const CTlsServerSettings& src );

    /** Whether TLS is to be used: a certificate and a key are given */
    bool IsEnabled( void ) const;

    /** PEM file with the certificate of the server, followed by the intermediate certificates of its chain */
    void SetCertificateChainPath( const CORE::CString& path );
    const CORE::CString& GetCertificateChainPath( void ) const;

    /** PEM file with the private key of the certificate, not encrypted */
    void SetPrivateKeyPath( const CORE::CString& path );
    const CORE::CString& GetPrivateKeyPath( void ) const;

    /** Lowest version to accept. The floor is TLS 1.2: a lower value is raised to it */
    void SetMinVersion( ETlsVersion version );
    ETlsVersion GetMinVersion( void ) const;

    /** Highest version to use, TLSVERSION_ANY for the newest the provider has. Not below the minimum */
    void SetMaxVersion( ETlsVersion version );
    ETlsVersion GetMaxVersion( void ) const;

    /** Application protocols the server speaks (ALPN) in order of preference, for HTTP "http/1.1". Empty: no ALPN */
    void SetAlpnProtocols( const TStringVector& protocols );
    void AddAlpnProtocol( const CORE::CString& protocol );
    const TStringVector& GetAlpnProtocols( void ) const;

    /** How long a client has to complete the handshake after it connected, the connection is closed after that */
    void SetHandshakeTimeoutInMs( UInt32 timeoutInMs );
    UInt32 GetHandshakeTimeoutInMs( void ) const;

    /** Identifies what the context of a provider depends on: the files, the versions and ALPN. Not the contents of the files */
    CORE::CString GetContextKey( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CORE::CString m_certificateChainPath;
    CORE::CString m_privateKeyPath;
    ETlsVersion m_minVersion;
    ETlsVersion m_maxVersion;
    TStringVector m_alpnProtocols;
    UInt32 m_handshakeTimeoutInMs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CTLSSERVERSETTINGS_H ? */
