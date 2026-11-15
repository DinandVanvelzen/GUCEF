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

#ifndef GUCEF_GUI_CLAYOUTLINT_H
#define GUCEF_GUI_CLAYOUTLINT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CLAYOUTLINTFINDING_H
#include "gucefGUI_CLayoutLintFinding.h"
#define GUCEF_GUI_CLAYOUTLINTFINDING_H
#endif /* GUCEF_GUI_CLAYOUTLINTFINDING_H ? */

#ifndef GUCEF_GUI_CLAYOUTLINTOPTIONS_H
#include "gucefGUI_CLayoutLintOptions.h"
#define GUCEF_GUI_CLAYOUTLINTOPTIONS_H
#endif /* GUCEF_GUI_CLAYOUTLINTOPTIONS_H ? */

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

class CGUIContext;
class CLayoutLintState;

/**
 *  Checks a laid out widget tree for layout defects, without rendering: overlapping controls, widgets outside their
 *  parent or the picture, text that is cut without an ellipsis, targets too small to hit, viewports too small for
 *  their rows and the other rules of ELayoutLintRule.
 *
 *  Run() walks the effectively visible widgets of the context the way the renderers draw them (a widget's own drawing
 *  stays inside it, its children are cut at its edge when it has Clip), lets CWidgetPainter paint every widget into
 *  scratch command lists and reads the text commands with their clips and their text flags
 *  (CDrawSurface::TEXTFLAG_*). It changes nothing in the widgets. Text is measured with the fonts of the context, so a
 *  check with the fonts the application uses sees what the user sees; a font the context cannot measure is reported
 *  (L11) and its text is estimated.
 *
 *  The rules and what they check on the layout of today:
 *  L01 Overlap: two visible siblings with content intersect (a decoration, a canvas or an empty view below the other is
 *      not counted, nor an input transparent empty-state text over an empty view).
 *  L02 OutsideParent: a widget with something to see is not inside its parent (only the horizontal axis in a scroll
 *      panel); Error when the parent clips it, Warning when it is drawn over the neighbours.
 *  L03 OutsideScreen: a widget extends beyond the logical picture and no clipping ancestor cuts it; a movable window is exempt.
 *  L04 TextClipped: a text run passes its clip (the widget, Clip ancestors, the painter's clips) by more than the
 *      tolerance, horizontally without an ellipsis or a horizontal scroll, vertically outside a vertical scroll; a row of
 *      a scrolling view is also checked against its row.
 *  L05 TextElidedNoTooltip: an elided run (TEXTFLAG_ELIDED, or text that ends in "...") where the tooltip service gives no text
 *      for the widget the pointer is on at the middle of the run (CGUIContext::HitTest()).
 *  L06 BelowMinimum: a single line text widget lower than its line (the minimum of the size requests waits for them).
 *  L07 SmallTarget: an enabled interactive widget below the minimum hit target, unless the circle of that diameter around
 *      it meets no other target (the spacing exception of WCAG 2.5.8).
 *  L08 ZeroSize: a widget with content less than 2 pixels on an axis.
 *  L09 ViewportTooSmall: a list, tree, grid or text box that shows fewer than the minimum rows (or all of them, when fewer)
 *      while it has more; a scroll panel lower than the minimum rows of its text while its content does not fit.
 *  L10 TightSpacing: interactive siblings side by side closer than half the related spacing (touching ones only with
 *      the StyleClass "Segmented").
 *  L11 EstimatedText: a font the context cannot measure.
 *  L12 NumberElided: a numeric run (TEXTFLAG_NUMERIC, or digits before "...") that is elided or cut.
 *  L13 TabUnreachable: a tab without a rectangle in its strip and no overflow button.
 *  L14 ContentUnreachable: a widget with content beyond a clipping ancestor (other than its parent) that does not scroll
 *      on that axis.
 *  L15 LowContrast: text against the fills drawn under it below WCAG 1.4.3 (Info for disabled widgets; text over a picture
 *      is not judged).
 *  L16 EmptyViewport: a list, tree or grid without rows (there is no empty-state text yet).
 *  L17 AbsoluteInFlow: an anchored panel with more interactive children than the options allow.
 *
 *  A widget suppresses rules for itself and everything in it with the LintIgnore property (rule ids or names).
 */
class GUCEF_GUI_PUBLIC_CPP CLayoutLint
{
    public:

    CLayoutLint( void );

    ~CLayoutLint();

    /**
     *  Checks the context (laid out first when the options say so) and replaces 'findings' with what it found, sorted by
     *  severity, then path. Returns how many findings are failures: at the fail severity of the options or worse
     */
    UInt32 Run( CGUIContext& context, const CLayoutLintOptions& options, TLayoutLintFindingVector& findings );

    /** A text for test output and the log: a line of counts and a line per finding, at most 'maxFindings' (0: all) */
    static CString Report( const TLayoutLintFindingVector& findings, UInt32 maxFindings = 0 );

    /** How many findings are at the severity or worse */
    static UInt32 CountAtLeast( const TLayoutLintFindingVector& findings, ELayoutLintSeverity severity );

    /** Adds the number of findings per rule at the severity or worse to 'counts' (LAYOUTLINTRULE_COUNT entries) */
    static void CountByRule( const TLayoutLintFindingVector& findings, ELayoutLintSeverity minSeverity, UInt32* counts );

    /** The contrast ratio of two opaque colours as WCAG defines it: 1 to 21 */
    static Float32 GetContrastRatio( const CColour& a, const CColour& b );

    /** Whether text that was drawn shortened ends in the ellipsis of CWidgetPainter::FitText() */
    static bool EndsWithEllipsis( const CString& text );

    /** Whether the text is a number as it is shown (digits with signs, separators, a unit or a currency sign), "..." aside */
    static bool LooksNumeric( const CString& text );

    private:

    CLayoutLint( const CLayoutLint& src );
    CLayoutLint& operator=( const CLayoutLint& src );

    CLayoutLintState* m_state;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CLAYOUTLINT_H ? */
