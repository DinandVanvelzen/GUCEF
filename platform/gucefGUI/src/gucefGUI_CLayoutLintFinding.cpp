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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefGUI_CLayoutLintFinding.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLayoutLintFinding::CLayoutLintFinding( void )
    : rule( LAYOUTLINTRULE_OVERLAP )
    , severity( LAYOUTLINTSEVERITY_INFO )
    , widgetPath()
    , widgetType()
    , otherPath()
    , rect()
    , otherRect()
    , text()
    , font()
    , detail()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CString
CLayoutLintFinding::Describe( void ) const
{GUCEF_TRACE;

    CString line = LayoutLintRuleToId( rule ) + " " + LayoutLintRuleToString( rule ) + " (" + LayoutLintSeverityToString( severity ) + "): " + widgetPath;
    if ( !text.IsNULLOrEmpty() )
    {
        // long runs are shortened in the report only, the finding keeps the whole text
        CString shown = text;
        if ( shown.Length() > 60 )
            shown = shown.SubstrFromRange( 0, 57 ) + "...";
        line += ": '" + shown + "'";
    }
    if ( !detail.IsNULLOrEmpty() )
        line += " " + detail;
    if ( !otherPath.IsNULLOrEmpty() )
        line += " [" + otherPath + "]";
    return line;
}

/*-------------------------------------------------------------------------*/

bool
CLayoutLintFinding::operator<( const CLayoutLintFinding& other ) const
{GUCEF_TRACE;

    if ( severity != other.severity )
        return severity > other.severity;
    if ( widgetPath != other.widgetPath )
        return widgetPath < other.widgetPath;
    if ( rule != other.rule )
        return rule < other.rule;
    if ( rect.top != other.rect.top )
        return rect.top < other.rect.top;
    return rect.left < other.rect.left;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
