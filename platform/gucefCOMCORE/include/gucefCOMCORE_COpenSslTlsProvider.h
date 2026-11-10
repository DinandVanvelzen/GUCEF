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

#ifndef GUCEF_COMCORE_COPENSSLTLSPROVIDER_H
#define GUCEF_COMCORE_COPENSSLTLSPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_ITLSPROVIDER_H
#include "gucefCOMCORE_ITlsProvider.h"
#define GUCEF_COMCORE_ITLSPROVIDER_H
#endif /* GUCEF_COMCORE_ITLSPROVIDER_H ? */

#ifndef GUCEF_COMCORE_COPENSSLTLSPROVIDERSESSION_H
#include "gucefCOMCORE_COpenSslTlsProviderSession.h"
#define GUCEF_COMCORE_COPENSSLTLSPROVIDERSESSION_H
#endif /* GUCEF_COMCORE_COPENSSLTLSPROVIDERSESSION_H ? */

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
 *  TLS through the OpenSSL of the operating system (3.x, or 1.1.1), loaded when TLS is first asked for.
 *
 *  Nothing is linked: the libraries (libssl.so.3 with libcrypto.so.3, then libssl.so.1.1 with libcrypto.so.1.1,
 *  then libssl.so with libcrypto.so) are opened with LoadModuleDynamicly() the first time IsAvailable() is
 *  called and the entry points are looked up by name, see gucefCOMCORE_COpenSslTlsProviderAbi.h. A system without
 *  OpenSSL runs the application normally: IsAvailable() says false with the reason the loader gave, for example
 *  "libssl.so.3: cannot open shared object file: No such file or directory", and it is up to the application
 *  whether that is fatal, or whether another provider is registered, for example by a plugin.
 *
 *  Only available on Linux. The libraries stay loaded for the life of the process: a library that registered
 *  exit handlers is not safely unloaded.
 *
 *  Policy: TLS 1.2 or newer, security level 2, no compression, no renegotiation, certificate verification against
 *  SSL_CERT_FILE / SSL_CERT_DIR or the system bundle (and the CA of the settings), names matched against the
 *  subject alternative names only, ALPN, SNI and in-memory session resumption.
 */
class GUCEF_COMCORE_EXPORT_CPP COpenSslTlsProvider : public ITlsProvider
{
    public:

    static const CORE::CString ProviderName;

    /** A pair of libraries that belong together */
    struct SLibraryCandidate
    {
        CORE::CString sslName;
        CORE::CString cryptoName;
    };
    typedef struct SLibraryCandidate TLibraryCandidate;
    typedef GUCEF::vector< SLibraryCandidate > TLibraryCandidateVector;

    /** Looks for the libraries in the default order */
    COpenSslTlsProvider( void );

    /** Looks for the libraries in the given order, mostly for tests that make the libraries absent */
    explicit COpenSslTlsProvider( const TLibraryCandidateVector& candidates );

    virtual ~COpenSslTlsProvider() GUCEF_VIRTUAL_OVERRIDE;

    /** libssl.so.3 + libcrypto.so.3, libssl.so.1.1 + libcrypto.so.1.1, libssl.so + libcrypto.so */
    static void GetDefaultLibraryCandidates( TLibraryCandidateVector& candidates );

    /**
     *  Checks that a table of entry points is usable: a version of OpenSSL we support (1.1.1 or newer). Separate so
     *  that a test can try it with a table of fake functions.
     */
    static bool CheckApi( const SOpenSslApi& api, CORE::CString& whyNot );

    virtual CORE::CString GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsAvailable( CORE::CString& whyNot ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetVersionInfo( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TTlsSessionPtr CreateClientSession( const CTlsSettings& settings    ,
                                                const CORE::CString& serverName ,
                                                CTlsError& error                ) GUCEF_VIRTUAL_OVERRIDE;

    /** Test hook: the unix time the certificates are checked at, 0 for the clock. Never part of the settings of an application */
    void SetVerificationTimeForTesting( Int64 unixTime );

    /** For tests and diagnostics: the number of contexts (one per distinct settings) and of sessions waiting for resumption */
    UInt32 GetContextCount( void ) const;
    UInt32 GetResumableSessionCount( void ) const;

    private:

    COpenSslTlsProvider( const COpenSslTlsProvider& src );
    COpenSslTlsProvider& operator=( const COpenSslTlsProvider& src );

    /** Probes once, @return whether the libraries are usable */
    bool Probe( void ) const;

    bool TryLoad( const SLibraryCandidate& candidate, CORE::CString& whyNot ) const;

    private:

    TLibraryCandidateVector m_candidates;
    mutable TOpenSslStatePtr m_shared;
    mutable bool m_probed;
    mutable bool m_available;
    mutable CORE::CString m_whyNot;
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

#endif /* GUCEF_COMCORE_COPENSSLTLSPROVIDER_H ? */
