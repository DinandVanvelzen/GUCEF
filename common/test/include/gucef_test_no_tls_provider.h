/*
 *  gucef common header: header-only removal of the TLS providers for the duration of a test
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

/*
 *  Test support: the system a test runs on has a TLS provider (the OpenSSL of the system on Linux). To test what the
 *  framework does on a system that has none, an object of this class takes the providers out of the registry of the
 *  process and puts them back when it goes out of scope:
 *
 *    GUCEF::TESTFW::CNoTlsProviderScope noTls;
 *    ASSERT_FALSE( GUCEF::WEB::CHTTPClient::IsSchemeSupported( "https" ) );     // no plugin loaded either
 */

#ifndef GUCEF_TEST_NO_TLS_PROVIDER_H
#define GUCEF_TEST_NO_TLS_PROVIDER_H

#ifdef __cplusplus

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

namespace GUCEF {
namespace TESTFW {

/*-------------------------------------------------------------------------*/

class CNoTlsProviderScope
{
    public:

    CNoTlsProviderScope( void )
        : m_providers()
    {
        COMCORE::CTlsProviderRegistry& registry = COMCORE::CComCoreGlobal::Instance()->GetTlsProviderRegistry();
        registry.GetProviderStatus( m_providers );
        registry.UnregisterAll();
    }

    ~CNoTlsProviderScope()
    {
        COMCORE::CTlsProviderRegistry& registry = COMCORE::CComCoreGlobal::Instance()->GetTlsProviderRegistry();
        for ( size_t i=0; i<m_providers.size(); ++i )
            registry.RegisterProvider( m_providers[ i ].provider, m_providers[ i ].priority );
    }

    private:

    CNoTlsProviderScope( const CNoTlsProviderScope& src );
    CNoTlsProviderScope& operator=( const CNoTlsProviderScope& src );

    COMCORE::CTlsProviderRegistry::TTlsProviderStatusVector m_providers;
};

/*-------------------------------------------------------------------------*/

} /* namespace TESTFW */
} /* namespace GUCEF */

#endif /* __cplusplus ? */

#endif /* GUCEF_TEST_NO_TLS_PROVIDER_H ? */
