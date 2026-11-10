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

#ifndef GUCEF_COMCORE_ITLSPROVIDER_H
#define GUCEF_COMCORE_ITLSPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_ITLSSESSION_H
#include "gucefCOMCORE_ITlsSession.h"
#define GUCEF_COMCORE_ITLSSESSION_H
#endif /* GUCEF_COMCORE_ITLSSESSION_H ? */

#ifndef GUCEF_COMCORE_CTLSSETTINGS_H
#include "gucefCOMCORE_CTlsSettings.h"
#define GUCEF_COMCORE_CTLSSETTINGS_H
#endif /* GUCEF_COMCORE_CTLSSETTINGS_H ? */

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
 *  One implementation of TLS: the operating system (SChannel, SecureTransport), a library of the system that is
 *  loaded when asked for (OpenSSL on Linux) or a plugin. Providers are registered with the CTlsProviderRegistry.
 *
 *  Providers are used from any thread: what they share between connections (trust anchors, session caches) is
 *  guarded by the provider.
 */
class GUCEF_COMCORE_EXPORT_CPP ITlsProvider
{
    public:

    virtual ~ITlsProvider();

    /** "openssl-dynamic", "schannel", "plugin:mbedtls" */
    virtual CORE::CString GetProviderName( void ) const = 0;

    /**
     *  Whether this provider can be used on this system. Probes when first asked (loading a library, asking the
     *  OS) and remembers the answer. When false whyNot says why in a way that helps the user, for example
     *  "libssl.so.3: cannot open shared object file: No such file or directory".
     */
    virtual bool IsAvailable( CORE::CString& whyNot ) const = 0;

    /** ETlsCapability bits */
    virtual UInt32 GetCapabilities( void ) const = 0;

    /** For logs: "OpenSSL 3.5.5 27 Jan 2026" */
    virtual CORE::CString GetVersionInfo( void ) const = 0;

    /**
     *  A session for a client connection. The settings are copied, the server name is the one SNI and the
     *  verification of the name use (the host of the connection unless the settings give another).
     *  @return NULL with the error filled in when the session cannot be made: the provider is unavailable, or the
     *          settings cannot be used. Verification is never skipped because of a settings problem
     */
    virtual TTlsSessionPtr CreateClientSession( const CTlsSettings& settings   ,
                                                const CORE::CString& serverName ,
                                                CTlsError& error                ) = 0;

    protected:

    ITlsProvider( void );

    private:

    ITlsProvider( const ITlsProvider& src );
    ITlsProvider& operator=( const ITlsProvider& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< ITlsProvider, MT::CMutex > TITlsProviderPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_ITLSPROVIDER_H ? */
