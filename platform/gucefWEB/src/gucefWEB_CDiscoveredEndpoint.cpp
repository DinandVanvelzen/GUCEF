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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefWEB_CDiscoveredEndpoint.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDiscoveredEndpoint::CDiscoveredEndpoint( void )
    : method()
    , host()
    , pathTemplate()
    , kind( 0 )
    , firstSequence( 0 )
    , requestCount( 0 )
    , statusCounts()
    , resourceTypes()
    , mimeTypes()
    , queryParams()
    , queryEnumValues()
    , queryEnumOverflow()
    , requestHeaderNames()
    , requestContentTypes()
    , hasRequestBodyShape( false )
    , requestBodyShape()
    , totalBytes( 0 )
    , hasShape( false )
    , shapeTried( false )
    , shape()
    , redirectTargets()
    , downloadRedirectChains()
    , downloadCount( 0 )
    , downloadDispositions()
    , downloadContentTypes()
    , downloadFileNames()
    , neverRecorded( false )
    , exampleUrls()
    , postDataExamples()
    , bodyFiles()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CDiscoveredEndpoint::GetKey( void ) const
{GUCEF_TRACE;

    return method + " " + host + pathTemplate;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
