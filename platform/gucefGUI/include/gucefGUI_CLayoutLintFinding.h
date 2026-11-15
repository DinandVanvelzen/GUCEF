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

#ifndef GUCEF_GUI_CLAYOUTLINTFINDING_H
#define GUCEF_GUI_CLAYOUTLINTFINDING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

#ifndef GUCEF_GUI_ELAYOUTLINTRULE_H
#include "gucefGUI_ELayoutLintRule.h"
#define GUCEF_GUI_ELAYOUTLINTRULE_H
#endif /* GUCEF_GUI_ELAYOUTLINTRULE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One finding of the layout lint (CLayoutLint): the rule, how bad it is, the widget (by its name path), the other
 *  widget when the rule compares two (an overlap, a parent), both rectangles in logical pixels of the context, the
 *  drawn text when the rule is about text, and a sentence that says what is wrong in numbers.
 */
class GUCEF_GUI_PUBLIC_CPP CLayoutLintFinding
{
    public:

    ELayoutLintRule rule;
    ELayoutLintSeverity severity;
    CString widgetPath;     /**< CWidget::GetNamePath() of the widget the finding is about */
    CString widgetType;     /**< the registered type of the widget */
    CString otherPath;      /**< the other widget: the sibling it overlaps, the parent it leaves; empty when none */
    CRect rect;             /**< the widget, or the text run for the text rules */
    CRect otherRect;        /**< the other widget, or the room the text has (its clip) */
    CString text;           /**< the drawn text for the text rules */
    CString font;           /**< the font of the widget, to write about it (the debug overlay) */
    CString detail;         /**< what is wrong, in numbers: "needs 22 px height, has 11 px" */

    CLayoutLintFinding( void );

    /** One line: "L04 TextClipped (Error): Form/Rows/Row0/Detail: 'Ingest: into ...' needs 22 px height, has 11 px" */
    CString Describe( void ) const;

    /** Orders by severity (worst first), then by widget path, then by rule */
    bool operator<( const CLayoutLintFinding& other ) const;
};

typedef GUCEF::vector< CLayoutLintFinding > TLayoutLintFindingVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CLAYOUTLINTFINDING_H ? */
