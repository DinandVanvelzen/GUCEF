/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_ELAYOUTLINTSEVERITY_H
#define GUCEF_GUI_ELAYOUTLINTSEVERITY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_ETYPES_H
#include "gucefGUI_ETypes.h"
#define GUCEF_GUI_ETYPES_H
#endif /* GUCEF_GUI_ETYPES_H ? */

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  How bad a finding of the layout lint (CLayoutLint) is. The values are ordered: a higher value is worse.
 *
 *  Info: counted and listed, never a failure. Warning: reported, a failure only in strict mode or beyond a budget.
 *  Error: content or function is lost (clipped text, overlapping controls, a target too small to hit).
 */
enum ELayoutLintSeverity
{
    LAYOUTLINTSEVERITY_INFO     = 0 ,
    LAYOUTLINTSEVERITY_WARNING  = 1 ,
    LAYOUTLINTSEVERITY_ERROR    = 2 ,

    LAYOUTLINTSEVERITY_COUNT        /**< the number of severities, not a severity */
};
typedef enum ELayoutLintSeverity TLayoutLintSeverity;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** "Info", "Warning" or "Error"; an empty string for a value outside the enumeration */
GUCEF_GUI_PUBLIC_CPP const CString& LayoutLintSeverityToString( ELayoutLintSeverity severity );

/** The severity with the name, case insensitive; false (and 'severity' unchanged) for an unknown name */
GUCEF_GUI_PUBLIC_CPP bool StringToLayoutLintSeverity( const CString& name, ELayoutLintSeverity& severity );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_ELAYOUTLINTSEVERITY_H ? */
