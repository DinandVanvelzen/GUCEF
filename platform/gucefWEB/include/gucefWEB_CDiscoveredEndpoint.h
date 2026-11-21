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

#ifndef GUCEF_WEB_CDISCOVEREDENDPOINT_H
#define GUCEF_WEB_CDISCOVEREDENDPOINT_H

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
 *  What is known about one endpoint of a web site: the requests of the same method, host and path template, see
 *  CSiteEndpointCatalog. Without example values (CSiteDiscoverySettings::recordExampleValues) it holds structure
 *  only: names, types, counts. With them it also holds a few redacted examples, for private use.
 */
class GUCEF_WEB_PUBLIC_CPP CDiscoveredEndpoint
{
    public:

    typedef GUCEF::map< Int32, UInt32 >                                 TStatusCountMap;
    typedef GUCEF::map< CORE::CString, CORE::CString::StringVector >    TQueryParamMap;
    typedef GUCEF::map< CORE::CString, UInt32 >                         TValueCountMap;
    typedef GUCEF::map< CORE::CString, TValueCountMap >                 TQueryEnumMap;

    static const UInt32 MaxEnumValues = 12;         /**< distinct values kept per query parameter */

    CORE::CString method;
    CORE::CString host;
    CORE::CString pathTemplate;
    Int32 kind;                                     /**< CSiteEndpointCatalog::EKind of the first observation */
    UInt32 firstSequence;
    UInt32 requestCount;
    TStatusCountMap statusCounts;
    CORE::CString::StringSet resourceTypes;
    CORE::CString::StringSet mimeTypes;
    TQueryParamMap queryParams;                     /**< name to a few distinct example values; no values without example values */
    TQueryEnumMap queryEnumValues;                  /**< name to the enum-like values seen and how often, only with recordEnumQueryValues */
    CORE::CString::StringSet queryEnumOverflow;     /**< names that had more distinct enum-like values than MaxEnumValues */
    CORE::CString::StringSet requestHeaderNames;
    CORE::CString::StringSet requestContentTypes;
    bool hasRequestBodyShape;
    CWebBodyShape requestBodyShape;                 /**< of the first request that had a body */
    UInt64 totalBytes;
    bool hasShape;
    bool shapeTried;                                /**< a response was looked at, whether that gave a shape or not */
    CWebBodyShape shape;                            /**< of the responses: the union of those whose shape was recorded, see CWebBodyShape::Merge() */
    CORE::CString::StringSet redirectTargets;       /**< "302 -> host/path-template" of the responses that redirected */
    CORE::CString::StringSet downloadRedirectChains;/**< "host > host > host" of the downloads that went through redirects */
    UInt32 downloadCount;
    CORE::CString::StringSet downloadDispositions;
    CORE::CString::StringSet downloadContentTypes;
    CORE::CString::StringSet downloadFileNames;     /**< templates of the suggested file names */
    bool neverRecorded;                             /**< a sign-in, security or credential endpoint: nothing but that it was requested */
    CORE::CString::StringVector exampleUrls;        /**< redacted, only with example values */
    CORE::CString::StringVector postDataExamples;   /**< redacted, only with example values */
    CORE::CString::StringVector bodyFiles;          /**< saved responses, only with example values */

    CDiscoveredEndpoint( void );

    /** METHOD host/path-template */
    CORE::CString GetKey( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CDISCOVEREDENDPOINT_H ? */
