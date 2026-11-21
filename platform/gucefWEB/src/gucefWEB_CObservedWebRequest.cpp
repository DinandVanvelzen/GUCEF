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

#include "gucefWEB_CObservedWebRequest.h"

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

CObservedWebRequest::CObservedWebRequest( void )
    : sequence( 0 )
    , timestamp( 0.0 )
    , method()
    , url()
    , resourceType()
    , mimeType()
    , status( 0 )
    , encodedDataLength( 0 )
    , failed( false )
    , errorText()
    , requestHeaders()
    , requestContentType()
    , postData()
    , requestBodyShape()
    , hasResponseShape( false )
    , responseShape()
    , bodyFile()
    , actionId( 0 )
    , isDownload( false )
    , contentDisposition()
    , downloadFileName()
    , cookiesSent()
    , cookiesSet()
    , redirectTarget()
    , redirectedFromHosts()
    , queryEnumValues()
    , neverRecorded( false )
    , redactedClass()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
