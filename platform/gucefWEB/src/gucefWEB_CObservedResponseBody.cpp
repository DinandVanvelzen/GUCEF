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

#include "gucefWEB_CObservedResponseBody.h"

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

CObservedResponseBody::CObservedResponseBody( const CORE::CDynamicBuffer& responseBody )
    : method()
    , host()
    , pathTemplate()
    , status( 0 )
    , mimeType()
    , requestSeq( 0 )
    , tapId( 0 )
    , requestHeaders()
    , queryValues()
    , body( responseBody )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CObservedResponseBody::CObservedResponseBody( const CObservedResponseBody& src )
    : method( src.method )
    , host( src.host )
    , pathTemplate( src.pathTemplate )
    , status( src.status )
    , mimeType( src.mimeType )
    , requestSeq( src.requestSeq )
    , tapId( src.tapId )
    , requestHeaders( src.requestHeaders )
    , queryValues( src.queryValues )
    , body( src.body )
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
