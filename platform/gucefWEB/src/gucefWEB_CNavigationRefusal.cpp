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

#include "gucefWEB_CNavigationRefusal.h"

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

const CORE::CString CNavigationRefusal::ReasonNotAllowed = "not-allowed";
const CORE::CString CNavigationRefusal::ReasonDenied = "denied";
const CORE::CString CNavigationRefusal::ReasonNeverRecord = "never-record";
const CORE::CString CNavigationRefusal::ReasonScheme = "scheme";
const CORE::CString CNavigationRefusal::ReasonMalformed = "malformed";
const CORE::CString CNavigationRefusal::SourceNavigate = "navigate";
const CORE::CString CNavigationRefusal::SourcePage = "page";
const CORE::CString CNavigationRefusal::SourceRedirect = "redirect";
const CORE::CString CNavigationRefusal::SourceCommitted = "committed";

/*-------------------------------------------------------------------------*/

CNavigationRefusal::CNavigationRefusal( void )
    : urlTemplate()
    , host()
    , frameId()
    , isMainFrame( true )
    , reason()
    , source()
    , blocked( true )
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
