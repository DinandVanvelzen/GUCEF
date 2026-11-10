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

#ifndef HTTPCLIENTPLUGIN_CURL_CCURLHTTPCLIENTCONTEXT_H
#define HTTPCLIENTPLUGIN_CURL_CCURLHTTPCLIENTCONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <curl/curl.h>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef HTTPCLIENTPLUGIN_CURL_CCURLMULTIDRIVER_H
#include "httpclientpluginCURL_CCurlMultiDriver.h"
#define HTTPCLIENTPLUGIN_CURL_CCURLMULTIDRIVER_H
#endif /* HTTPCLIENTPLUGIN_CURL_CCURLMULTIDRIVER_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  State shared by a curl HTTP client backend and all the requests it started:
 *  - a curl share handle so that TLS sessions and DNS results are reused across requests, also across threads
 *    (curl calls back for locking)
 *  - the multi drivers, one per pulse generator that requests run on. Connections are kept for reuse by the multi
 *    handle of a driver, for the requests of that driver: curl does not support sharing connections between threads
 *  Requests hold a shared pointer to it so they can outlive the backend.
 */
class HTTPCLIENTPLUGIN_CURL_PRIVATE_CPP CCurlHttpClientContext
{
    public:

    CCurlHttpClientContext( void );

    ~CCurlHttpClientContext();

    CURLSH* GetShareHandle( void ) const;

    /** the driver of the given pulse generator, created when it does not exist yet */
    CCurlMultiDriver* GetOrCreateDriver( CORE::PulseGeneratorPtr pulseGenerator );

    /**
     *  Deletes the driver if it has no requests and is not in the middle of handling a pulse.
     *  Otherwise it is kept for reuse and deleted later
     */
    void ReleaseDriverIfIdle( CCurlMultiDriver* driver );

    private:

    CCurlHttpClientContext( const CCurlHttpClientContext& src );                /**< not implemented */
    CCurlHttpClientContext& operator=( const CCurlHttpClientContext& src );     /**< not implemented */

    static void ShareLock( CURL* handle, curl_lock_data data, curl_lock_access access, void* userPtr );
    static void ShareUnlock( CURL* handle, curl_lock_data data, void* userPtr );

    private:

    typedef GUCEF::map< CORE::CPulseGenerator*, CCurlMultiDriver* > TDriverMap;

    CURLSH* m_share;
    MT::CMutex m_shareLocks[ CURL_LOCK_DATA_LAST ];
    MT::CMutex m_driverLock;
    TDriverMap m_drivers;
};

typedef CORE::CTSharedPtr< CCurlHttpClientContext, MT::CMutex > CCurlHttpClientContextPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace HTTPCLIENTCURL */
}; /* namespace WEBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* HTTPCLIENTPLUGIN_CURL_CCURLHTTPCLIENTCONTEXT_H ? */
