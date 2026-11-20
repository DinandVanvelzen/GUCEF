/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_COMCORE_CSECURETRANSPORTTLSPROVIDER_H
#define GUCEF_COMCORE_CSECURETRANSPORTTLSPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_ITLSPROVIDER_H
#include "gucefCOMCORE_ITlsProvider.h"
#define GUCEF_COMCORE_ITLSPROVIDER_H
#endif /* GUCEF_COMCORE_ITLSPROVIDER_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS )

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

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
 *  TLS through the Secure Transport of macOS (Security.framework): there is nothing to install and nothing to
 *  load, the operating system provides it. The certificates of the peer are verified by the trust evaluation of
 *  the system (SecTrust) against the keychains of the system and the CA of the settings, the names are matched
 *  against the subject alternative names.
 *
 *  Secure Transport speaks TLS 1.2 at most. Settings that demand TLS 1.3 as a minimum are refused with
 *  TLSERR_SETTINGS and the capabilities do not include TLSCAP_TLS13, so that a provider that has TLS 1.3 (OpenSSL,
 *  a plugin) can be preferred by whoever needs it. It does resume sessions but cannot tell when it did, so
 *  TLSCAP_RESUMPTION is not announced.
 *
 *  The server side (TLSCAP_SERVER) presents the certificate and key of its settings. Secure Transport needs the
 *  key in a keychain for that: the provider makes a private keychain file in the temporary directory, which is
 *  removed again when the provider is destroyed.
 *
 *  Only available on macOS.
 */
class CSecureTransportTlsProvider : public ITlsProvider
{
    public:

    static const CORE::CString ProviderName;

    CSecureTransportTlsProvider( void );

    virtual ~CSecureTransportTlsProvider() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsAvailable( CORE::CString& whyNot ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetVersionInfo( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TTlsSessionPtr CreateClientSession( const CTlsSettings& settings    ,
                                                const CORE::CString& serverName ,
                                                CTlsError& error                ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TTlsSessionPtr CreateServerSession( const CTlsServerSettings& settings ,
                                                CTlsError& error                   ) GUCEF_VIRTUAL_OVERRIDE;

    /** Test hook: the unix time the certificates are checked at, 0 for the clock. Never part of the settings of an application */
    void SetVerificationTimeForTesting( Int64 unixTime );

    private:

    CSecureTransportTlsProvider( const CSecureTransportTlsProvider& src );
    CSecureTransportTlsProvider& operator=( const CSecureTransportTlsProvider& src );

    /** The identity (certificate + key) and chain of the server for the given files, kept while the files do not change. A retained CFArrayRef */
    void* AcquireServerIdentity( const CTlsServerSettings& settings, CTlsError& error );

    bool EnsureKeychain( CTlsError& error );

    private:

    struct SServerIdentity
    {
        CORE::CString key;          /**< files and their stamps */
        void* chain;                /**< CFArrayRef: the identity followed by the intermediate certificates, retained */
    };

    SServerIdentity m_serverIdentity;
    void* m_keychain;               /**< SecKeychainRef, the one that was made last */
    GUCEF::vector< void* > m_keychains;                 /**< all of them: a key of a server stays in the keychain it was imported in for as long as there are sessions */
    GUCEF::vector< CORE::CString > m_keychainPaths;
    Int64 m_verifyTimeOverride;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CSECURETRANSPORTTLSPROVIDER_H ? */
