/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#ifndef GUCEF_WEB_COBSERVEDWEBREQUEST_H
#define GUCEF_WEB_COBSERVEDWEBREQUEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_WEB_CWEBBODYSHAPE_H
#include "gucefWEB_CWebBodyShape.h"
#define GUCEF_WEB_CWEBBODYSHAPE_H
#endif /* GUCEF_WEB_CWEBBODYSHAPE_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One request a web page made, as the browser reported it over the DevTools protocol, see CChromeNetworkRecorder.
 *
 *  It holds what the recorder kept of the request, which is already less than what was sent: the values of secret
 *  headers are "<not recorded>" and of a never-record request only method, URL and status are kept. The URL and the
 *  request body are as they were sent, in memory only; CSiteDiscoveryJournal and CSiteEndpointCatalog redact them with
 *  CSiteDiscoveryRedactor before anything is written.
 */
class GUCEF_WEB_PUBLIC_CPP CObservedWebRequest
{
    public:

    typedef GUCEF::map< CORE::CString, CORE::CString > TStringMap;
    typedef GUCEF::vector< std::pair< CORE::CString, CORE::CString > > TStringPairVector;

    UInt32 sequence;                    /**< the number of the request in the journal */
    Float64 timestamp;                  /**< seconds, as reported by the browser */
    CORE::CString method;
    CORE::CString url;
    CORE::CString resourceType;         /**< Document, XHR, Fetch, Image, Script, ... */
    CORE::CString mimeType;             /**< of the response */
    Int32 status;                       /**< 0 when there was no response */
    UInt64 encodedDataLength;           /**< bytes received */
    bool failed;
    CORE::CString errorText;
    TStringMap requestHeaders;          /**< names in lower case, the values of secret headers are not recorded */
    CORE::CString requestContentType;   /**< the content type of the request body, without parameters */
    CORE::CString postData;             /**< the request body, in memory only */
    CWebBodyShape requestBodyShape;     /**< the fields of the request body without their values */
    bool hasResponseShape;
    CWebBodyShape responseShape;        /**< the fields of the response body, when it was looked at */
    CORE::CString bodyFile;             /**< the saved response, relative to the journal's folder; empty if not saved */
    UInt32 actionId;                    /**< the user action the request followed, 0 for none, see CUserActionTracker */
    bool isDownload;                    /**< the response is a file: Content-Disposition attachment or a download of the browser */
    CORE::CString contentDisposition;   /**< attachment or inline, without the file name */
    CORE::CString downloadFileName;     /**< the suggested file name with identifiers replaced, see CUrlPathTemplate::TemplateSegment() */
    CORE::CString::StringVector cookiesSent;    /**< the names (never the values) of the cookies the browser sent with the request */
    CORE::CString::StringVector cookiesSet;     /**< the names (never the values) of the cookies the response set */
    CORE::CString redirectTarget;       /**< a 3xx response: the host and path template (no query) it redirects to */
    CORE::CString::StringVector redirectedFromHosts;    /**< hosts of the earlier legs of the redirect chain this request is the last leg of, in order */
    TStringPairVector queryEnumValues;  /**< name and value of the enum-like query values, only with recordEnumQueryValues; see CSiteDiscoveryRedactor::ExtractEnumQueryValues() */
    bool neverRecorded;                 /**< a sign-in, security or credential request: only method, URL and status are kept */
    CORE::CString redactedClass;        /**< not empty: the stub a journal keeps of a line that would have held a secret of this class; only sequence, timestamp, method and host remain */

    CObservedWebRequest( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_COBSERVEDWEBREQUEST_H ? */
