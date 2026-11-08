/*
 *  httpclientpluginCURL: GUCEF plugin providing a HTTP client backend using libcurl
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "httpclientpluginCURL_CCurlHttpClientContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEBPLUGIN {
namespace HTTPCLIENTCURL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCurlHttpClientContext::CCurlHttpClientContext( void )
    : m_share( curl_share_init() )
    , m_shareLocks()
    , m_driverLock()
    , m_drivers()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_share )
    {
        // TLS sessions, DNS results and the public suffix list are shared, across threads under the locks below.
        // Cookies are not: the WEB module handles those. Connections are not either: curl does not support a connection
        // cache shared by transfers on several threads (CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT). A transfer looking for
        // a connection to reuse reads the state of every connection to the same host in the shared cache, also of those
        // that a transfer on another thread is setting up or using without the lock, which made concurrent synchronous
        // requests race. The connections stay in the multi handle of each driver, which one thread runs at a time
        curl_share_setopt( m_share, CURLSHOPT_LOCKFUNC, &CCurlHttpClientContext::ShareLock );
        curl_share_setopt( m_share, CURLSHOPT_UNLOCKFUNC, &CCurlHttpClientContext::ShareUnlock );
        curl_share_setopt( m_share, CURLSHOPT_USERDATA, this );
        curl_share_setopt( m_share, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION );
        curl_share_setopt( m_share, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS );
        curl_share_setopt( m_share, CURLSHOPT_SHARE, CURL_LOCK_DATA_PSL );
    }
}

/*-------------------------------------------------------------------------*/

CCurlHttpClientContext::~CCurlHttpClientContext()
{GUCEF_TRACE;

    // All requests are gone since they hold the context, only idle drivers remain
    TDriverMap::iterator i = m_drivers.begin();
    while ( i != m_drivers.end() )
    {
        GUCEF_DELETE (*i).second;
        ++i;
    }
    m_drivers.clear();

    if ( GUCEF_NULL != m_share )
        curl_share_cleanup( m_share );
    m_share = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CURLSH*
CCurlHttpClientContext::GetShareHandle( void ) const
{GUCEF_TRACE;

    return m_share;
}

/*-------------------------------------------------------------------------*/

CCurlMultiDriver*
CCurlHttpClientContext::GetOrCreateDriver( CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    if ( pulseGenerator.IsNULL() )
        return GUCEF_NULL;

    MT::CScopeMutex lock( m_driverLock );

    CORE::CPulseGenerator* key = pulseGenerator.GetPointerAlways();
    TDriverMap::iterator i = m_drivers.find( key );
    if ( i != m_drivers.end() )
        return (*i).second;

    CCurlMultiDriver* driver = GUCEF_NEW CCurlMultiDriver( pulseGenerator );
    m_drivers[ key ] = driver;
    return driver;
}

/*-------------------------------------------------------------------------*/

void
CCurlHttpClientContext::ReleaseDriverIfIdle( CCurlMultiDriver* driver )
{GUCEF_TRACE;

    if ( GUCEF_NULL == driver )
        return;

    MT::CScopeMutex lock( m_driverLock );

    if ( 0 == driver->GetActiveRequestCount() && !driver->IsInPulse() )
    {
        m_drivers.erase( driver->GetPulseGeneratorPointer() );
        GUCEF_DELETE driver;
    }
}

/*-------------------------------------------------------------------------*/

void
CCurlHttpClientContext::ShareLock( CURL* handle, curl_lock_data data, curl_lock_access access, void* userPtr )
{
    CCurlHttpClientContext* context = static_cast< CCurlHttpClientContext* >( userPtr );
    if ( GUCEF_NULL != context && data < CURL_LOCK_DATA_LAST )
        context->m_shareLocks[ data ].Lock();
}

/*-------------------------------------------------------------------------*/

void
CCurlHttpClientContext::ShareUnlock( CURL* handle, curl_lock_data data, void* userPtr )
{
    CCurlHttpClientContext* context = static_cast< CCurlHttpClientContext* >( userPtr );
    if ( GUCEF_NULL != context && data < CURL_LOCK_DATA_LAST )
        context->m_shareLocks[ data ].Unlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace HTTPCLIENTCURL */
}; /* namespace WEBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
