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

#ifndef GUCEF_GUI_ELAYOUTLINTRULE_H
#define GUCEF_GUI_ELAYOUTLINTRULE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_ELAYOUTLINTSEVERITY_H
#include "gucefGUI_ELayoutLintSeverity.h"
#define GUCEF_GUI_ELAYOUTLINTSEVERITY_H
#endif /* GUCEF_GUI_ELAYOUTLINTSEVERITY_H ? */

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
 *  The rules of the layout lint (CLayoutLint). Each rule has a short id ("L01") and a name ("Overlap"); both are
 *  accepted wherever a rule is given as text (options, the LintIgnore property, budgets). The value of an id is
 *  its number less one, so the ids never change their meaning.
 */
enum ELayoutLintRule
{
    LAYOUTLINTRULE_OVERLAP              = 0  ,  /**< L01: two visible siblings with content intersect */
    LAYOUTLINTRULE_OUTSIDEPARENT        = 1  ,  /**< L02: a child is not inside its parent */
    LAYOUTLINTRULE_OUTSIDESCREEN        = 2  ,  /**< L03: a widget extends beyond the logical picture */
    LAYOUTLINTRULE_TEXTCLIPPED          = 3  ,  /**< L04: drawn text is cut by a clip without an ellipsis and without scrolling */
    LAYOUTLINTRULE_TEXTELIDEDNOTOOLTIP  = 4  ,  /**< L05: elided text whose widget gives no tooltip there */
    LAYOUTLINTRULE_BELOWMINIMUM         = 5  ,  /**< L06: a widget is smaller than its minimum (today: a text widget lower than one line) */
    LAYOUTLINTRULE_SMALLTARGET          = 6  ,  /**< L07: an interactive widget is below the minimum hit target */
    LAYOUTLINTRULE_ZEROSIZE             = 7  ,  /**< L08: a visible widget with content is less than 2 pixels on an axis */
    LAYOUTLINTRULE_VIEWPORTTOOSMALL     = 8  ,  /**< L09: a list, grid, tree or text box shows fewer rows than its minimum while it has more */
    LAYOUTLINTRULE_TIGHTSPACING         = 9  ,  /**< L10: two interactive siblings are closer than half the related spacing */
    LAYOUTLINTRULE_ESTIMATEDTEXT        = 10 ,  /**< L11: text in a font that cannot be measured: the text findings are approximate */
    LAYOUTLINTRULE_NUMBERELIDED         = 11 ,  /**< L12: a number is elided or clipped */
    LAYOUTLINTRULE_TABUNREACHABLE       = 12 ,  /**< L13: a tab has no room in its strip and nothing reaches it */
    LAYOUTLINTRULE_CONTENTUNREACHABLE   = 13 ,  /**< L14: content beyond a clipping ancestor that does not scroll that way */
    LAYOUTLINTRULE_LOWCONTRAST          = 14 ,  /**< L15: text against the fill under it below the contrast of WCAG 1.4.3 */
    LAYOUTLINTRULE_EMPTYVIEWPORT        = 15 ,  /**< L16: a list, grid or tree without items and without an empty-state text */
    LAYOUTLINTRULE_ABSOLUTEINFLOW       = 16 ,  /**< L17: an anchored panel with many interactive children: a candidate for containers */

    LAYOUTLINTRULE_COUNT                        /**< the number of rules, not a rule */
};
typedef enum ELayoutLintRule TLayoutLintRule;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The name of a rule ("Overlap"); empty for a value outside the enumeration */
GUCEF_GUI_PUBLIC_CPP const CString& LayoutLintRuleToString( ELayoutLintRule rule );

/** The short id of a rule ("L01"); empty for a value outside the enumeration */
GUCEF_GUI_PUBLIC_CPP const CString& LayoutLintRuleToId( ELayoutLintRule rule );

/** The rule with the name or the id, case insensitive; false (and 'rule' unchanged) when there is none */
GUCEF_GUI_PUBLIC_CPP bool StringToLayoutLintRule( const CString& nameOrId, ELayoutLintRule& rule );

/** The severity a rule reports at unless a case of the rule says otherwise (see the rules in CLayoutLint) */
GUCEF_GUI_PUBLIC_CPP ELayoutLintSeverity GetLayoutLintRuleDefaultSeverity( ELayoutLintRule rule );

/**
 *  Whether the rule is fully checked on the layout system of today. False for the rules that are approximations until
 *  the measure contract and the layout engine exist (L06 checks one line of text only, L13 the tab rectangles only)
 */
GUCEF_GUI_PUBLIC_CPP bool IsLayoutLintRuleComplete( ELayoutLintRule rule );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_ELAYOUTLINTRULE_H ? */
