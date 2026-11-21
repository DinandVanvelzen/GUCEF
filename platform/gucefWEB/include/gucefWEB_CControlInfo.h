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

#ifndef GUCEF_WEB_CCONTROLINFO_H
#define GUCEF_WEB_CCONTROLINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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
 *  A control of a page as CDevToolsPage::ListControls() reports it: what kind of control it is and what it is called,
 *  never what it holds. No input values, no password fields, and no text of table cells: a control inside a table cell
 *  has inTableCell set and neither a label nor option labels.
 */
class GUCEF_WEB_PUBLIC_CPP CControlInfo
{
    public:

    static const CORE::CString RoleButton;          /**< "button" */
    static const CORE::CString RoleLink;            /**< "link" */
    static const CORE::CString RoleTab;             /**< "tab" */
    static const CORE::CString RoleSelect;          /**< "select": a select element */
    static const CORE::CString RoleCombobox;        /**< "combobox": a combobox or list box that is not a select element */
    static const CORE::CString RoleMenuItem;        /**< "menuitem", also the checkbox and radio forms of a menu item */
    static const CORE::CString RoleCheckbox;        /**< "checkbox", also a switch */
    static const CORE::CString RoleRadio;           /**< "radio" */

    CORE::CString role;                             /**< one of the roles above */
    CORE::CString label;                            /**< the accessible name, whitespace collapsed, at most 80 characters; empty in a table cell */
    bool visible;                                   /**< rendered, not hidden from assistive technology, with a box of non-zero size */
    bool enabled;                                   /**< not disabled */
    bool inTableCell;                               /**< inside a td, th or an element with a cell or header role */
    CORE::CString::StringVector optionLabels;       /**< the labels of the options of a select element, in order */
    Int64 backendNodeId;                            /**< the browser's handle of the element, valid until the page changes */

    CControlInfo( void );
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CControlInfo >   TControlInfoVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CCONTROLINFO_H ? */
