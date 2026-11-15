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

#ifndef GUCEF_GUI_CLAYOUTLINTOPTIONS_H
#define GUCEF_GUI_CLAYOUTLINTOPTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CGLOBPATTERN_H
#include "gucefCORE_CGlobPattern.h"
#define GUCEF_CORE_CGLOBPATTERN_H
#endif /* GUCEF_CORE_CGLOBPATTERN_H ? */

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

class CWidget;

/**
 *  What a run of the layout lint (CLayoutLint::Run()) checks and how strict it is: the rules that are on, the
 *  thresholds of the rules, the part of the widget tree to check and the findings to leave out.
 *
 *  A suppression names a rule and a glob pattern over the widget name path (CORE::CGlobPattern, "*" and "?"); it says
 *  that a finding is intended (an overlay that covers a picture on purpose). Every suppression should carry its
 *  reason where it is made. In the pattern "*" matches within one name of the path and "**" across names. A widget can
 *  also suppress rules for itself and what is in it with the LintIgnore property.
 */
class GUCEF_GUI_PUBLIC_CPP CLayoutLintOptions
{
    public:

    /** The values of the thresholds, in logical pixels at scale 1 */
    enum EDefaults
    {
        DefaultMinHitTarget = 24        ,   /**< WCAG 2.2, 2.5.8 (AA), pointer use */
        DefaultMinRows = 3              ,   /**< the rows a list, grid, tree or text box shows at least while it has more */
        DefaultRelatedSpacing = 8       ,   /**< interactive siblings closer than half of it are tight */
        DefaultMaxAnchoredInteractive = 6 , /**< an anchored panel with more interactive children is a candidate for containers */
        DefaultTolerance = 1                /**< pixels a text may pass its clip before it counts as clipped */
    };

    CLayoutLintOptions( void );

    /** Every rule on (the default) or off */
    void EnableAllRules( bool enabled );

    void EnableRule( ELayoutLintRule rule, bool enabled );

    bool IsRuleEnabled( ELayoutLintRule rule ) const;

    /** Leaves out the findings of a rule for the widgets whose name path matches the glob pattern; false for a pattern that is not valid */
    bool AddSuppression( ELayoutLintRule rule, const CString& pathPattern );

    void ClearSuppressions( void );

    /** Whether a finding of the rule for the widget with the name path is suppressed by an option (not by LintIgnore) */
    bool IsSuppressed( ELayoutLintRule rule, const CString& widgetPath ) const;

    /** The findings at this severity and worse are what Run() counts as failures; Error by default */
    ELayoutLintSeverity failSeverity;

    /** Strict: warnings fail as well (failSeverity is taken as Warning) */
    bool strict;

    /** The widget whose subtree is checked; NULL for every root of the context */
    const CWidget* root;

    /** Whether Run() lays out first (CGUIContext::FlushChanges()); off for a run from inside a frame that is being drawn */
    bool layoutFirst;

    Int32 minHitTarget;
    Int32 minRows;
    Int32 relatedSpacing;
    Int32 maxAnchoredInteractive;
    Int32 tolerance;

    /** The contrast below which text is reported (WCAG 1.4.3: 4.5, and 3 for large text) */
    Float32 minContrast;
    Float32 minContrastLargeText;

    /** The line height from which text counts as large for the contrast rule (24 px text has a line of about 32) */
    Int32 largeTextLineHeight;

    private:

    struct SSuppression
    {
        ELayoutLintRule rule;
        CORE::CGlobPattern pattern;
    };
    typedef GUCEF::vector< SSuppression > TSuppressionVector;

    bool m_enabled[ LAYOUTLINTRULE_COUNT ];
    TSuppressionVector m_suppressions;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CLAYOUTLINTOPTIONS_H ? */
