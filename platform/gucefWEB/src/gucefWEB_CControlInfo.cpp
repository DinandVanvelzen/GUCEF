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

#include "gucefWEB_CControlInfo.h"

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

const CORE::CString CControlInfo::RoleButton = "button";
const CORE::CString CControlInfo::RoleLink = "link";
const CORE::CString CControlInfo::RoleTab = "tab";
const CORE::CString CControlInfo::RoleSelect = "select";
const CORE::CString CControlInfo::RoleCombobox = "combobox";
const CORE::CString CControlInfo::RoleMenuItem = "menuitem";
const CORE::CString CControlInfo::RoleCheckbox = "checkbox";
const CORE::CString CControlInfo::RoleRadio = "radio";

/*-------------------------------------------------------------------------*/

CControlInfo::CControlInfo( void )
    : role()
    , label()
    , visible( false )
    , enabled( false )
    , inTableCell( false )
    , optionLabels()
    , backendNodeId( 0 )
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
