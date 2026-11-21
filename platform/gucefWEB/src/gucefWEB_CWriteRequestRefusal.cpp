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

#include "gucefWEB_CWriteRequestRefusal.h"

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

const CORE::CString CWriteRequestRefusal::ReasonMethodNotAllowed = "method-not-allowed";
const CORE::CString CWriteRequestRefusal::ReasonSchemeNotAllowed = "scheme-not-allowed";
const CORE::CString CWriteRequestRefusal::ReasonHostNotAllowed = "host-not-allowed";
const CORE::CString CWriteRequestRefusal::ReasonAmbiguousPath = "ambiguous-path";
const CORE::CString CWriteRequestRefusal::ReasonPathNotAllowed = "path-not-allowed";
const CORE::CString CWriteRequestRefusal::ReasonQueryNotAllowed = "query-not-allowed";
const CORE::CString CWriteRequestRefusal::ReasonMalformed = "malformed";

/*-------------------------------------------------------------------------*/

CWriteRequestRefusal::CWriteRequestRefusal( void )
    : method()
    , url()
    , host()
    , pathTemplate()
    , resourceType()
    , frameId()
    , isMainFrame( false )
    , redirected( false )
    , reason()
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
