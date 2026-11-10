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

#ifndef HTTPCLIENTPLUGIN_CURL_CCURLMULTIDRIVER_H
#define HTTPCLIENTPLUGIN_CURL_CCURLMULTIDRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <curl/curl.h>

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef HTTPCLIENTPLUGIN_CURL_MACROS_H
#include "httpclientpluginCURL_macros.h"
#define HTTPCLIENTPLUGIN_CURL_MACROS_H
#endif /* HTTPCLIENTPLUGIN_CURL_MACROS_H ? */


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

class CCurlHttpClientRequest;

/*-------------------------------------------------------------------------*/

/**
 *  Drives the curl transfers (a curl multi handle) which belong to a single pulse generator.
 *  Transfers progress on the pulses of that pulse generator, hence on the thread which owns it.
 *  While there are transfers it asks for periodic pulses, when there are none it stops asking.
 *  Not thread-safe: only use it from the thread of its pulse generator.
 */
class HTTPCLIENTPLUGIN_CURL_PRIVATE_CPP CCurlMultiDriver : public CORE::CObserver
{
    public:

    explicit CCurlMultiDriver( CORE::PulseGeneratorPtr pulseGenerator );

    virtual ~CCurlMultiDriver() GUCEF_VIRTUAL_OVERRIDE;

    /** adds the easy handle of the request to the transfers carried out by this driver */
    bool AddRequest( CCurlHttpClientRequest* request );

    /** removes the easy handle of the request. Safe to call when it is not added */
    void RemoveRequest( CCurlHttpClientRequest* request );

    UInt32 GetActiveRequestCount( void ) const;

    bool IsInPulse( void ) const;

    CORE::CPulseGenerator* GetPulseGeneratorPointer( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CCurlMultiDriver( void );                                       /**< not implemented */
    CCurlMultiDriver( const CCurlMultiDriver& src );                /**< not implemented */
    CCurlMultiDriver& operator=( const CCurlMultiDriver& src );     /**< not implemented */

    void OnPulse( CORE::CNotifier* notifier           ,
                  const CORE::CEvent& eventId         ,
                  CORE::CICloneable* eventData        );

    private:

    CORE::PulseGeneratorPtr m_pulseGenerator;
    CURLM* m_multi;
    UInt32 m_activeRequests;
    bool m_inPulse;
    bool m_pulsesRequested;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace HTTPCLIENTCURL */
}; /* namespace WEBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* HTTPCLIENTPLUGIN_CURL_CCURLMULTIDRIVER_H ? */
