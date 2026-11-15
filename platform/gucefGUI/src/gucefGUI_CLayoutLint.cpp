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

#include <math.h>
#include <algorithm>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CWIDGETPAINTER_H
#include "gucefGUI_CWidgetPainter.h"
#define GUCEF_GUI_CWIDGETPAINTER_H
#endif /* GUCEF_GUI_CWIDGETPAINTER_H ? */

#ifndef GUCEF_GUI_CLISTBOX_H
#include "gucefGUI_CListBox.h"
#define GUCEF_GUI_CLISTBOX_H
#endif /* GUCEF_GUI_CLISTBOX_H ? */

#ifndef GUCEF_GUI_CTREEVIEW_H
#include "gucefGUI_CTreeview.h"
#define GUCEF_GUI_CTREEVIEW_H
#endif /* GUCEF_GUI_CTREEVIEW_H ? */

#ifndef GUCEF_GUI_CTEXTBOX_H
#include "gucefGUI_CTextbox.h"
#define GUCEF_GUI_CTEXTBOX_H
#endif /* GUCEF_GUI_CTEXTBOX_H ? */

#ifndef GUCEF_GUI_CGRIDVIEW_H
#include "gucefGUI_CGridView.h"
#define GUCEF_GUI_CGRIDVIEW_H
#endif /* GUCEF_GUI_CGRIDVIEW_H ? */

#ifndef GUCEF_GUI_CTABCONTROL_H
#include "gucefGUI_CTabControl.h"
#define GUCEF_GUI_CTABCONTROL_H
#endif /* GUCEF_GUI_CTABCONTROL_H ? */

#ifndef GUCEF_GUI_CSCROLLPANEL_H
#include "gucefGUI_CScrollPanel.h"
#define GUCEF_GUI_CSCROLLPANEL_H
#endif /* GUCEF_GUI_CSCROLLPANEL_H ? */

#ifndef GUCEF_GUI_CDRAWSURFACE_H
#include "gucefGUI_CDrawSurface.h"
#define GUCEF_GUI_CDRAWSURFACE_H
#endif /* GUCEF_GUI_CDRAWSURFACE_H ? */

#include "gucefGUI_CLayoutLint.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef CDrawSurface::TCommandVector TCommandVector;
typedef CDrawSurface::TCommand TCommand;

/** A rectangle larger than any picture: no clip */
const CRect Unbounded( -(1 << 28), -(1 << 28), 1 << 29, 1 << 29 );

bool
IsInside( const CRect& inner, const CRect& outer, Int32 tolerance )
{
    return inner.left >= outer.left - tolerance && inner.top >= outer.top - tolerance &&
           inner.Right() <= outer.Right() + tolerance && inner.Bottom() <= outer.Bottom() + tolerance;
}

CString
RectText( const CRect& rect )
{
    return CORE::Int32ToString( rect.width ) + "x" + CORE::Int32ToString( rect.height ) + " at " +
           CORE::Int32ToString( rect.left ) + "," + CORE::Int32ToString( rect.top );
}

/** How far a rectangle passes another on each side, as text ("12 px right, 3 px bottom"); empty when it does not */
CString
OverflowText( const CRect& inner, const CRect& outer )
{
    CString text;
    if ( inner.left < outer.left )
        text += CORE::Int32ToString( outer.left - inner.left ) + " px left";
    if ( inner.Right() > outer.Right() )
        text += ( text.IsNULLOrEmpty() ? "" : ", " ) + CORE::Int32ToString( inner.Right() - outer.Right() ) + " px right";
    if ( inner.top < outer.top )
        text += ( text.IsNULLOrEmpty() ? "" : ", " ) + CORE::Int32ToString( outer.top - inner.top ) + " px top";
    if ( inner.Bottom() > outer.Bottom() )
        text += ( text.IsNULLOrEmpty() ? "" : ", " ) + CORE::Int32ToString( inner.Bottom() - outer.Bottom() ) + " px bottom";
    return text;
}

/** The text a widget shows of its own: Text, else Caption (group box), else Title (window) */
const CString&
OwnText( const CWidget& widget )
{
    const CString& text = widget.GetPropertyRef( EWidgetProperty_Text );
    if ( !text.IsNULLOrEmpty() )
        return text;
    const CString& caption = widget.GetPropertyRef( EWidgetProperty_Caption );
    if ( !caption.IsNULLOrEmpty() )
        return caption;
    return widget.GetPropertyRef( EWidgetProperty_Title );
}

bool
IsRowsWidget( const CWidget& widget )
{
    return widget.IsRegisteredAs( "Listbox" ) || widget.IsRegisteredAs( "Treeview" ) || widget.IsRegisteredAs( "GridView" ) ||
           widget.IsRegisteredAs( "Textbox" ) || widget.IsRegisteredAs( "PopupMenu" );
}

/** A widget others are drawn on: a picture, a video, a drawing or a 3D view */
bool
IsCanvas( const CWidget& widget )
{
    return widget.IsRegisteredAs( "DrawSurface" ) || widget.IsRegisteredAs( "ImageFrame" );
}

bool
IsSingleLineText( const CWidget& widget )
{
    if ( widget.IsRegisteredAs( "Editbox" ) || widget.IsRegisteredAs( "Combobox" ) || widget.IsRegisteredAs( "Spinner" ) )
        return true;
    if ( widget.IsRegisteredAs( "Label" ) )
        return !widget.GetBoolProperty( EWidgetProperty_WordWrap ) && !widget.GetPropertyRef( EWidgetProperty_Text ).IsNULLOrEmpty();
    if ( widget.IsRegisteredAs( "Button" ) )
        return !widget.GetPropertyRef( EWidgetProperty_Text ).IsNULLOrEmpty() && widget.GetPropertyRef( EWidgetProperty_NormalImage ).IsNULLOrEmpty();
    return false;
}

/** The rules the LintIgnore property of the widget names, as bits */
UInt32
IgnoreMaskOf( const CWidget& widget )
{
    const CString& value = widget.GetPropertyRef( EWidgetProperty_LintIgnore );
    if ( value.IsNULLOrEmpty() )
        return 0;
    UInt32 mask = 0;
    CString list = value.ReplaceChar( '\n', ',' ).ReplaceChar( ' ', ',' ).ReplaceChar( ';', ',' );
    CString::StringVector names = list.ParseElements( ',', false );
    for ( UInt32 i=0; i<(UInt32) names.size(); ++i )
    {
        ELayoutLintRule rule = LAYOUTLINTRULE_COUNT;
        if ( StringToLayoutLintRule( names[ i ], rule ) )
            mask |= ( 1u << (UInt32) rule );
    }
    return mask;
}

Float32
Linear( UInt8 channel )
{
    const Float32 c = (Float32) channel / 255.0f;
    return c <= 0.03928f ? c / 12.92f : (Float32) pow( ( c + 0.055f ) / 1.055f, 2.4f );
}

Float32
Luminance( const CColour& colour )
{
    return 0.2126f * Linear( colour.r ) + 0.7152f * Linear( colour.g ) + 0.0722f * Linear( colour.b );
}

/** 'top' with its alpha over the opaque 'below' */
CColour
Blend( const CColour& top, const CColour& below )
{
    const UInt32 a = top.a;
    return CColour( (UInt8) ( ( top.r * a + below.r * ( 255 - a ) ) / 255 ), (UInt8) ( ( top.g * a + below.g * ( 255 - a ) ) / 255 ),
                    (UInt8) ( ( top.b * a + below.b * ( 255 - a ) ) / 255 ), 255 );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What one run of the lint keeps; reused by the next run so a matrix of runs allocates little */
class CLayoutLintState
{
    public:

    struct SNode
    {
        const CWidget* widget;
        Int32 parent;
        CRect rect;
        CRect ownClip;              /**< where its own drawing shows */
        CRect childClip;            /**< where the drawing of its children shows */
        CRect ancestorClip;         /**< the clipping ancestors without the picture: Unbounded when none */
        Int32 nearestClipper;       /**< the nearest ancestor that clips its children, -1 when none */
        bool control;               /**< a control: focusable or activatable, enabled or not */
        bool interactive;           /**< a control that is enabled */
        bool content;               /**< something the user reads or uses */
        bool visibleSomething;      /**< content, a fill, a border or visible children */
        bool zeroSize;
        bool seen;                  /**< some of it is on the screen: not cut away by a clipping ancestor (a scroll panel scrolled away from it) */
        bool inScrollPanel;         /**< below a scroll panel: what its viewport cuts vertically is scrolled to, not lost */
        bool placementReported;     /**< an L02, L03 or L14 finding for it or an ancestor */
        bool screenReported;        /**< an L03 finding for it or an ancestor */
        UInt32 ignoreMask;          /**< the rules LintIgnore suppresses here (its own and its ancestors') */
        GUCEF::vector< Int32 > children;
    };

    struct SFill
    {
        CRect rect;
        CColour colour;
        bool unknown;               /**< a picture: the colour under text on it is not known */
    };

    GUCEF::vector< SNode > nodes;
    GUCEF::vector< SFill > fills;
    TCommandVector commands;
    GUCEF::vector< CRect > clips;
    GUCEF::set< CString > estimatedFonts;
    CGUIContext* context;
    const CLayoutLintOptions* options;
    TLayoutLintFindingVector* findings;
    CRect screen;

    CLayoutLintState( void )
        : nodes(), fills(), commands(), clips(), estimatedFonts(), context( GUCEF_NULL ), options( GUCEF_NULL ), findings( GUCEF_NULL ), screen()
    {
    }

    void Run( void );

    private:

    void AddNode( const CWidget& widget, Int32 parent, const CRect& clip );
    void PaintNode( Int32 index );
    void CheckText( Int32 index, const TCommand& command, Int32 offsetX, Int32 offsetY, const CRect& clip );
    void CheckPlacement( Int32 index );
    void CheckSiblings( Int32 index );
    void CheckWidget( Int32 index );
    void CheckTargets( void );
    bool BackgroundAt( Int32 x, Int32 y, CColour& colour ) const;
    bool Report( Int32 index, ELayoutLintRule rule, ELayoutLintSeverity severity, const CRect& rect, const CRect& otherRect,
                 const CString& text, const CString& detail, Int32 other = -1 );
};

/*-------------------------------------------------------------------------*/

bool
CLayoutLintState::Report( Int32 index, ELayoutLintRule rule, ELayoutLintSeverity severity, const CRect& rect, const CRect& otherRect,
                          const CString& text, const CString& detail, Int32 other )
{
    if ( !options->IsRuleEnabled( rule ) )
        return false;
    const SNode& node = nodes[ index ];
    if ( 0 != ( node.ignoreMask & ( 1u << (UInt32) rule ) ) )
        return false;
    CLayoutLintFinding finding;
    finding.widgetPath = node.widget->GetNamePath();
    if ( options->IsSuppressed( rule, finding.widgetPath ) )
        return false;
    finding.rule = rule;
    finding.severity = severity;
    finding.widgetType = node.widget->GetType();
    if ( other >= 0 )
        finding.otherPath = nodes[ other ].widget->GetNamePath();
    finding.rect = rect;
    finding.otherRect = otherRect;
    finding.text = text;
    finding.font = node.widget->GetPropertyRef( EWidgetProperty_Font );
    finding.detail = detail;
    findings->push_back( finding );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintState::AddNode( const CWidget& widget, Int32 parent, const CRect& clip )
{
    // what the user does not see is not checked: hidden, in a hidden or collapsed ancestor (the caller stops there), invisible
    if ( !widget.IsVisible() || widget.GetFloatProperty( EWidgetProperty_Opacity ) <= 0.0f )
        return;

    const Int32 index = (Int32) nodes.size();
    nodes.push_back( SNode() );
    SNode& node = nodes.back();
    node.widget = &widget;
    node.parent = parent;
    node.rect = widget.GetRect();
    node.ownClip = clip.Intersection( node.rect );
    const bool clips = widget.GetBoolProperty( EWidgetProperty_Clip );
    node.childClip = clips ? clip.Intersection( node.rect ) : clip;
    node.ancestorClip = parent >= 0 ? nodes[ parent ].ancestorClip : Unbounded;
    node.nearestClipper = -1;
    if ( parent >= 0 )
    {
        const SNode& parentNode = nodes[ parent ];
        if ( parentNode.widget->GetBoolProperty( EWidgetProperty_Clip ) )
        {
            node.ancestorClip = parentNode.ancestorClip.Intersection( parentNode.rect );
            node.nearestClipper = parent;
        }
        else
            node.nearestClipper = parentNode.nearestClipper;
    }
    node.control = widget.GetBoolProperty( EWidgetProperty_Focusable ) || widget.IsActivatable();
    node.interactive = node.control && widget.IsEffectivelyEnabled();
    node.content = node.control || !OwnText( widget ).IsNULLOrEmpty() || IsRowsWidget( widget ) || IsCanvas( widget ) ||
                   widget.IsRegisteredAs( "Combobox" ) || widget.IsRegisteredAs( "TabControl" ) || widget.IsRegisteredAs( "ProgressBar" ) ||
                   widget.IsRegisteredAs( "Slider" ) || widget.IsRegisteredAs( "MenuBar" );
    node.visibleSomething = node.content;
    if ( !node.visibleSomething )
    {
        CColour colour;
        node.visibleSomething = ( widget.TryGetColourProperty( EWidgetProperty_BackgroundColour, colour ) && colour.a > 0 ) ||
                                ( widget.TryGetColourProperty( EWidgetProperty_BorderColour, colour ) && colour.a > 0 ) ||
                                !widget.GetPropertyRef( EWidgetProperty_BackgroundImage ).IsNULLOrEmpty();
    }
    node.zeroSize = false;
    node.seen = !node.ownClip.IsEmpty();
    node.inScrollPanel = parent >= 0 && ( nodes[ parent ].inScrollPanel || GUCEF_NULL != CWidget::ToScrollPanel( nodes[ parent ].widget ) );
    node.placementReported = parent >= 0 && nodes[ parent ].placementReported;
    node.screenReported = parent >= 0 && nodes[ parent ].screenReported;
    node.ignoreMask = IgnoreMaskOf( widget ) | ( parent >= 0 ? nodes[ parent ].ignoreMask : 0 );
    if ( parent >= 0 )
        nodes[ parent ].children.push_back( index );

    // its own drawing first, then the checks of its place (they need the parent), then the children on top of it
    PaintNode( index );
    CheckPlacement( index );
    if ( widget.AreChildrenShown() )
    {
        const CRect childClip = nodes[ index ].childClip;
        const CWidget::TWidgetVector& children = widget.GetChildren();
        for ( UInt32 i=0; i<(UInt32) children.size(); ++i )
            AddNode( *children[ i ], index, childClip );
    }
    if ( !nodes[ index ].children.empty() )
    {
        for ( UInt32 i=0; i<(UInt32) nodes[ index ].children.size(); ++i )
        {
            if ( nodes[ nodes[ index ].children[ i ] ].visibleSomething )
            {
                nodes[ index ].visibleSomething = true;
                break;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintState::PaintNode( Int32 index )
{
    const CWidget& widget = *nodes[ index ].widget;
    const CRect rect = nodes[ index ].rect;
    const CRect ownClip = nodes[ index ].ownClip;
    commands.clear();
    CWidgetPainter::Paint( widget, *context, commands );

    clips.clear();
    clips.push_back( ownClip );
    for ( UInt32 i=0; i<(UInt32) commands.size(); ++i )
    {
        const TCommand& command = commands[ i ];
        switch ( command.type )
        {
            case CDrawSurface::COMMAND_PUSHCLIP:
            {
                const CRect pushed( command.rect.left + rect.left, command.rect.top + rect.top, command.rect.width, command.rect.height );
                clips.push_back( clips.back().Intersection( pushed ) );
                break;
            }
            case CDrawSurface::COMMAND_POPCLIP:
            {
                if ( clips.size() > 1 )
                    clips.pop_back();
                break;
            }
            case CDrawSurface::COMMAND_FILLRECT:
            case CDrawSurface::COMMAND_IMAGE:
            {
                SFill fill;
                fill.rect = clips.back().Intersection( CRect( command.rect.left + rect.left, command.rect.top + rect.top, command.rect.width, command.rect.height ) );
                fill.colour = command.colour;
                fill.unknown = CDrawSurface::COMMAND_IMAGE == command.type;
                if ( !fill.rect.IsEmpty() && ( fill.unknown || fill.colour.a > 0 ) )
                    fills.push_back( fill );
                break;
            }
            case CDrawSurface::COMMAND_TEXT:
            {
                CheckText( index, command, rect.left, rect.top, clips.back() );
                break;
            }
            default:
                break;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CLayoutLintState::BackgroundAt( Int32 x, Int32 y, CColour& colour ) const
{
    // the fills drawn so far that cover the point, from the top down to the first opaque one, blended from the bottom up;
    // a picture among them makes the colour unknown
    Int32 i = (Int32) fills.size() - 1;
    GUCEF::vector< CColour > layers;
    for ( ; i>=0; --i )
    {
        const SFill& fill = fills[ i ];
        if ( !fill.rect.Contains( x, y ) )
            continue;
        if ( fill.unknown )
            return false;
        layers.push_back( fill.colour );
        if ( 255 == fill.colour.a )
            break;
    }
    colour = CColour( 0, 0, 0, 255 );     // what is under everything: the frame starts black
    for ( Int32 l=(Int32) layers.size() - 1; l>=0; --l )
        colour = Blend( layers[ l ], colour );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintState::CheckText( Int32 index, const TCommand& command, Int32 offsetX, Int32 offsetY, const CRect& clip )
{
    if ( command.text.IsNULLOrEmpty() || 0 == command.colour.a )
        return;
    const SNode& node = nodes[ index ];
    const CWidget& widget = *node.widget;
    const Int32 tolerance = options->tolerance;

    // the run: its width in its font, one line high
    Int32 width = 0, height = 0;
    const bool measured = context->MeasureText( command.font, command.text, 0, width, height );
    const Int32 lineHeight = context->GetLineHeight( command.font );
    if ( !measured )
    {
        // estimated: about half the line height per character
        width = (Int32) command.text.Length() * lineHeight / 2;
        if ( estimatedFonts.find( command.font ) == estimatedFonts.end() )
        {
            estimatedFonts.insert( command.font );
            Report( index, LAYOUTLINTRULE_ESTIMATEDTEXT, LAYOUTLINTSEVERITY_WARNING, node.rect, CRect(), command.text,
                    "the font \"" + command.font + "\" cannot be measured: the text findings are estimates" );
        }
    }
    const CRect run( command.x0 + offsetX, command.y0 + offsetY, width, lineHeight );
    const CRect area = command.textArea.IsEmpty() ? CRect() : CRect( command.textArea.left + offsetX, command.textArea.top + offsetY, command.textArea.width, command.textArea.height );
    const UInt32 flags = command.textFlags;
    const bool elided = 0 != ( flags & CDrawSurface::TEXTFLAG_ELIDED ) || CLayoutLint::EndsWithEllipsis( command.text );
    const bool numeric = 0 != ( flags & CDrawSurface::TEXTFLAG_NUMERIC ) || CLayoutLint::LooksNumeric( command.text );
    const bool scrollX = 0 != ( flags & CDrawSurface::TEXTFLAG_SCROLLABLE_X );

    // in a scroll panel a widget that holds its text is scrolled to: a vertical cut by the viewport is not silent
    const bool scrolledTo = node.inScrollPanel && run.top >= node.rect.top - tolerance && run.Bottom() <= node.rect.Bottom() + tolerance;
    const bool scrollY = 0 != ( flags & CDrawSurface::TEXTFLAG_SCROLLABLE_Y ) || scrolledTo;

    const bool cutX = run.left < clip.left - tolerance || run.Right() > clip.Right() + tolerance;
    // vertically a line may lose the leading above its ascenders (an eighth of the line, see CGUIContext::GetMinimumLineHeight())
    const Int32 toleranceY = tolerance + lineHeight / 16;
    const bool cutY = run.top < clip.top - toleranceY || run.Bottom() > clip.Bottom() + toleranceY;
    const bool invisible = clip.Intersection( run ).IsEmpty();

    // L12: a number is never shortened or cut ('#' marks of a number that does not fit are its sanctioned form)
    if ( numeric && ( elided || ( cutX && !scrollX && !invisible ) ) )
    {
        Report( index, LAYOUTLINTRULE_NUMBERELIDED, LAYOUTLINTSEVERITY_ERROR, run, clip, command.text,
                elided ? CString( "is shortened with \"...\"" ) : "needs " + CORE::Int32ToString( run.width ) + " px width, has " + CORE::Int32ToString( clip.width ) );
    }
    else
    if ( cutX && !elided && !scrollX && !invisible )
    {
        Report( index, LAYOUTLINTRULE_TEXTCLIPPED, LAYOUTLINTSEVERITY_ERROR, run, clip, command.text,
                "needs " + CORE::Int32ToString( run.width ) + " px width, has " + CORE::Int32ToString( clip.Intersection( run ).width ) +
                " (cut " + OverflowText( CRect( run.left, clip.top, run.width, clip.height ), clip ) + ")" );
    }

    // vertically: a cut outside a scrolling view, or a line taller than the row it is in
    if ( cutY && !scrollY )
    {
        Report( index, LAYOUTLINTRULE_TEXTCLIPPED, LAYOUTLINTSEVERITY_ERROR, run, clip, command.text,
                invisible ? CString( "is not visible at all: it lies outside its clip" ) :
                "needs " + CORE::Int32ToString( run.height ) + " px height, has " + CORE::Int32ToString( clip.Intersection( run ).height ) );
    }
    else
    if ( scrollY && !area.IsEmpty() && run.height > area.height + 2 * toleranceY )
    {
        Report( index, LAYOUTLINTRULE_TEXTCLIPPED, LAYOUTLINTSEVERITY_ERROR, run, area, command.text,
                "needs " + CORE::Int32ToString( run.height ) + " px height, its row has " + CORE::Int32ToString( area.height ) );
    }
    else
    if ( !scrollY && invisible && !cutY && !node.inScrollPanel )
    {
        Report( index, LAYOUTLINTRULE_TEXTCLIPPED, LAYOUTLINTSEVERITY_ERROR, run, clip, command.text, "is not visible at all: it lies outside its clip" );
    }

    // L05: shortened text says what it is in its tooltip
    if ( elided && !invisible && options->IsRuleEnabled( LAYOUTLINTRULE_TEXTELIDEDNOTOOLTIP ) )
    {
        const CRect seen = clip.Intersection( run );
        const Int32 x = seen.left + seen.width / 2;
        const Int32 y = seen.top + seen.height / 2;
        // the tooltip of what the pointer is on there: an input transparent label shows the tooltip of what is below it
        const CWidget* target = context->HitTest( x, y );
        if ( GUCEF_NULL == target )
            target = &widget;
        const CString tooltip = context->GetTooltipService().GetTooltipText( *target, x - target->GetRect().left, y - target->GetRect().top );
        if ( tooltip.IsNULLOrEmpty() )
            Report( index, LAYOUTLINTRULE_TEXTELIDEDNOTOOLTIP, LAYOUTLINTSEVERITY_WARNING, run, clip, command.text, "is shortened and has no tooltip there" );
    }

    // L15: the contrast with what is drawn under the middle of the run
    if ( options->IsRuleEnabled( LAYOUTLINTRULE_LOWCONTRAST ) && !invisible )
    {
        CColour background;
        const CRect seen = clip.Intersection( run );
        if ( BackgroundAt( seen.left + seen.width / 2, seen.top + seen.height / 2, background ) )
        {
            const CColour text = command.colour.a < 255 ? Blend( command.colour, background ) : command.colour;
            const Float32 ratio = CLayoutLint::GetContrastRatio( text, background );
            const Float32 needed = lineHeight >= options->largeTextLineHeight ? options->minContrastLargeText : options->minContrast;
            if ( ratio < needed )
            {
                char ratioText[ 32 ];
                snprintf( ratioText, sizeof( ratioText ), "%.2f", (double) ratio );
                Report( index, LAYOUTLINTRULE_LOWCONTRAST, widget.IsEffectivelyEnabled() ? LAYOUTLINTSEVERITY_WARNING : LAYOUTLINTSEVERITY_INFO, run, CRect(),
                        command.text, CString( "has a contrast of " ) + ratioText + ":1 with " + background.ToString() + ", needs " +
                        ( lineHeight >= options->largeTextLineHeight ? "3" : "4.5" ) + ":1" );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintState::CheckPlacement( Int32 index )
{
    SNode& node = nodes[ index ];
    const CWidget& widget = *node.widget;
    const CRect& rect = node.rect;

    // L08: content that has no room at all
    if ( node.content && ( rect.width < 2 || rect.height < 2 ) )
    {
        node.zeroSize = true;
        Report( index, LAYOUTLINTRULE_ZEROSIZE, LAYOUTLINTSEVERITY_ERROR, rect, CRect(), OwnText( widget ), "is " + RectText( rect ) );
        return;
    }
    // nothing to see of its own (a spacer, a transparent panel): its place does not matter, its children are judged themselves
    if ( !node.visibleSomething )
        return;

    // L02: inside the parent (a scroll panel scrolls vertically: only across)
    if ( node.parent >= 0 && !node.placementReported )
    {
        const SNode& parentNode = nodes[ node.parent ];
        CRect room = parentNode.rect;
        bool scrollsVertically = false;
        const CScrollPanel* scrollPanel = CWidget::ToScrollPanel( parentNode.widget );
        if ( GUCEF_NULL != scrollPanel )
        {
            const CRect view = scrollPanel->GetViewportRect();
            room = CRect( parentNode.rect.left + view.left, parentNode.rect.top + view.top, view.width, view.height );
            scrollsVertically = true;
        }
        const CRect checked = scrollsVertically ? CRect( rect.left, room.top, rect.width, room.height ) : rect;
        if ( !rect.IsEmpty() && !IsInside( checked, room, 0 ) )
        {
            const bool clipped = parentNode.widget->GetBoolProperty( EWidgetProperty_Clip );
            if ( Report( index, LAYOUTLINTRULE_OUTSIDEPARENT, clipped ? LAYOUTLINTSEVERITY_ERROR : LAYOUTLINTSEVERITY_WARNING, rect, room, OwnText( widget ),
                         "passes its parent by " + OverflowText( checked, room ) + ( clipped ? ", where the parent cuts it off" : ", drawn over what is next to it" ), node.parent ) )
                node.placementReported = true;
        }
    }

    // L14: beyond a clipping ancestor further up that does not scroll that way
    if ( !node.placementReported && node.content && node.nearestClipper >= 0 && node.nearestClipper != node.parent )
    {
        const SNode& clipper = nodes[ node.nearestClipper ];
        const bool scrollsVertically = GUCEF_NULL != CWidget::ToScrollPanel( clipper.widget );
        const CRect checked = scrollsVertically ? CRect( rect.left, clipper.rect.top, rect.width, clipper.rect.height ) : rect;
        if ( !rect.IsEmpty() && !IsInside( checked, clipper.rect, 0 ) )
        {
            if ( Report( index, LAYOUTLINTRULE_CONTENTUNREACHABLE, LAYOUTLINTSEVERITY_ERROR, rect, clipper.rect, OwnText( widget ),
                         "lies " + OverflowText( checked, clipper.rect ) + " beyond an ancestor that cuts it off and does not scroll there", node.nearestClipper ) )
                node.placementReported = true;
        }
    }

    // L03: beyond the picture, unless a clipping ancestor cuts it off first (that ancestor is reported) or it is a window the user moves
    if ( !node.screenReported && !rect.IsEmpty() && !IsInside( rect, screen, 0 ) )
    {
        const bool movableWindow = node.parent < 0 && widget.IsRegisteredAs( "Window" ) && widget.GetBoolProperty( EWidgetProperty_Movable );
        const CRect seen = rect.Intersection( node.ancestorClip );
        if ( !movableWindow && !seen.IsEmpty() && !IsInside( seen, screen, 0 ) )
        {
            if ( Report( index, LAYOUTLINTRULE_OUTSIDESCREEN, LAYOUTLINTSEVERITY_ERROR, rect, screen, OwnText( widget ), "passes the picture by " + OverflowText( seen, screen ) ) )
            {
                node.screenReported = true;
                node.placementReported = true;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintState::CheckSiblings( Int32 index )
{
    const SNode& parentNode = nodes[ index ];
    const UInt32 count = (UInt32) parentNode.children.size();
    if ( count < 2 )
        return;
    const Int32 gapLimit = options->relatedSpacing / 2;
    for ( UInt32 i=0; i<count; ++i )
    {
        const Int32 lower = parentNode.children[ i ];
        const SNode& a = nodes[ lower ];
        if ( a.zeroSize || a.rect.IsEmpty() || !a.seen )
            continue;
        for ( UInt32 j=i+1; j<count; ++j )
        {
            const Int32 upper = parentNode.children[ j ];
            const SNode& b = nodes[ upper ];
            if ( b.zeroSize || b.rect.IsEmpty() || !b.seen )
                continue;

            const CRect overlap = a.rect.Intersection( b.rect );
            if ( !overlap.IsEmpty() )
            {
                // L01: the lower one is drawn over. Not counted: a decoration or a canvas below (an overlay on a picture or a 3D view),
                // an input transparent widget without text, and an input transparent text over an empty view (its empty-state text)
                if ( !a.content || !b.content )
                    continue;
                if ( IsCanvas( *a.widget ) )
                    continue;
                const bool aTransparent = a.widget->GetBoolProperty( EWidgetProperty_InputTransparent );
                const bool bTransparent = b.widget->GetBoolProperty( EWidgetProperty_InputTransparent );
                if ( ( aTransparent && OwnText( *a.widget ).IsNULLOrEmpty() ) || ( bTransparent && OwnText( *b.widget ).IsNULLOrEmpty() ) )
                    continue;
                if ( bTransparent && a.children.empty() && ( IsRowsWidget( *a.widget ) || GUCEF_NULL != CWidget::ToScrollPanel( a.widget ) ) )
                {
                    const CListbox* list = CWidget::As< CListbox >( a.widget );
                    const CGridView* grid = CWidget::As< CGridView >( a.widget );
                    const bool empty = ( GUCEF_NULL != list && list->GetItems().empty() ) || ( GUCEF_NULL != grid && 0 == grid->GetRowCount() ) ||
                                       GUCEF_NULL != CWidget::ToScrollPanel( a.widget );
                    if ( empty )
                        continue;
                }
                Report( upper, LAYOUTLINTRULE_OVERLAP, LAYOUTLINTSEVERITY_ERROR, b.rect, a.rect, OwnText( *b.widget ),
                        "overlaps a sibling by " + CORE::Int32ToString( overlap.width ) + "x" + CORE::Int32ToString( overlap.height ) + " px", lower );
                continue;
            }

            // L10: controls side by side or above each other with a gap below half the related spacing
            if ( !a.interactive || !b.interactive )
                continue;
            Int32 gap = -1;
            const bool sameRow = a.rect.top < b.rect.Bottom() && b.rect.top < a.rect.Bottom();
            const bool sameColumn = a.rect.left < b.rect.Right() && b.rect.left < a.rect.Right();
            if ( sameRow )
                gap = b.rect.left >= a.rect.Right() ? b.rect.left - a.rect.Right() : a.rect.left - b.rect.Right();
            else
            if ( sameColumn )
                gap = b.rect.top >= a.rect.Bottom() ? b.rect.top - a.rect.Bottom() : a.rect.top - b.rect.Bottom();
            if ( gap < 0 || gap >= gapLimit )
                continue;
            if ( !sameRow && 0 == gap )
                continue;       // rows stacked edge to edge are a list of targets, as the rows of a list box
            if ( 0 == gap && a.widget->GetPropertyRef( EWidgetProperty_StyleClass ).HasSubstr( "Segmented" ) >= 0 &&
                 b.widget->GetPropertyRef( EWidgetProperty_StyleClass ).HasSubstr( "Segmented" ) >= 0 )
                continue;
            Report( upper, LAYOUTLINTRULE_TIGHTSPACING, LAYOUTLINTSEVERITY_WARNING, b.rect, a.rect, OwnText( *b.widget ),
                    "is " + CORE::Int32ToString( gap ) + " px from a control beside it, needs " + CORE::Int32ToString( gapLimit ), lower );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintState::CheckWidget( Int32 index )
{
    const SNode& node = nodes[ index ];
    const CWidget& widget = *node.widget;
    const CRect& rect = node.rect;
    if ( node.zeroSize || !node.seen )
        return;

    // L06: a single line of text lower than its line
    if ( IsSingleLineText( widget ) )
    {
        const Int32 lineHeight = context->GetMinimumLineHeight( widget.GetPropertyRef( EWidgetProperty_Font ) );
        if ( rect.height + options->tolerance < lineHeight )
            Report( index, LAYOUTLINTRULE_BELOWMINIMUM, node.interactive ? LAYOUTLINTSEVERITY_ERROR : LAYOUTLINTSEVERITY_WARNING, rect, CRect(), OwnText( widget ),
                    "is " + CORE::Int32ToString( rect.height ) + " px high, its line of text needs " + CORE::Int32ToString( lineHeight ) );
    }

    // L09 and L16: the rows a view shows
    Int32 rowCount = -1, rowsShown = 0, rowHeight = 0;
    if ( const CListbox* list = CWidget::As< CListbox >( &widget ) )
    {
        rowCount = (Int32) list->GetItems().size();
        rowHeight = list->GetRowHeight();
        rowsShown = rowHeight > 0 ? rect.height / rowHeight : 0;
    }
    else
    if ( const CTreeview* tree = CWidget::As< CTreeview >( &widget ) )
    {
        rowCount = (Int32) tree->GetVisibleRows().size();
        rowHeight = tree->GetRowHeight();
        rowsShown = rowHeight > 0 ? rect.height / rowHeight : 0;
    }
    else
    if ( const CGridView* grid = CWidget::As< CGridView >( &widget ) )
    {
        rowCount = (Int32) grid->GetRowCount();
        rowHeight = grid->GetRowHeight();
        rowsShown = grid->GetVisibleRowCount();
    }
    else
    if ( const CTextbox* textbox = CWidget::As< CTextbox >( &widget ) )
    {
        const Int32 lines = (Int32) textbox->GetDisplayLines().size();
        rowHeight = widget.GetIntProperty( EWidgetProperty_RowHeight ) > 0 ? widget.GetIntProperty( EWidgetProperty_RowHeight ) : context->GetLineHeight( widget.GetPropertyRef( EWidgetProperty_Font ) );
        rowsShown = rowHeight > 0 ? rect.height / rowHeight : 0;
        const Int32 needed = lines < options->minRows ? lines : options->minRows;
        if ( lines > rowsShown && rowsShown < needed )
            Report( index, LAYOUTLINTRULE_VIEWPORTTOOSMALL, LAYOUTLINTSEVERITY_ERROR, rect, CRect(), CString::Empty,
                    "shows " + CORE::Int32ToString( rowsShown ) + " of " + CORE::Int32ToString( lines ) + " lines, needs at least " + CORE::Int32ToString( needed ) );
    }
    if ( rowCount >= 0 )
    {
        const Int32 needed = rowCount < options->minRows ? rowCount : options->minRows;
        if ( rowCount > rowsShown && rowsShown < needed )
            Report( index, LAYOUTLINTRULE_VIEWPORTTOOSMALL, LAYOUTLINTSEVERITY_ERROR, rect, CRect(), CString::Empty,
                    "shows " + CORE::Int32ToString( rowsShown ) + " of " + CORE::Int32ToString( rowCount ) + " rows (" + CORE::Int32ToString( rowHeight ) +
                    " px each in " + CORE::Int32ToString( rect.height ) + " px), needs at least " + CORE::Int32ToString( needed ) );
        if ( 0 == rowCount )
            Report( index, LAYOUTLINTRULE_EMPTYVIEWPORT, LAYOUTLINTSEVERITY_INFO, rect, CRect(), CString::Empty, "has no rows and no text that says why" );
    }
    if ( const CScrollPanel* scrollPanel = CWidget::ToScrollPanel( &widget ) )
    {
        // a scroll panel that scrolls should show a few lines of what it holds
        const Int32 lineHeight = context->GetLineHeight( CString::Empty );
        const Int32 viewport = scrollPanel->GetViewportHeight();
        if ( scrollPanel->IsScrollable() && viewport < options->minRows * lineHeight )
            Report( index, LAYOUTLINTRULE_VIEWPORTTOOSMALL, LAYOUTLINTSEVERITY_ERROR, rect, CRect(), CString::Empty,
                    "scrolls in " + CORE::Int32ToString( viewport ) + " px, less than " + CORE::Int32ToString( options->minRows ) + " lines of " + CORE::Int32ToString( lineHeight ) );
    }

    // L13: every tab has a place in the strip or the overflow button reaches it
    if ( const CTabControl* tabs = CWidget::As< CTabControl >( &widget ) )
    {
        const CRect local( 0, 0, rect.width, rect.height );
        for ( UInt32 t=0; t<tabs->GetTabCount(); ++t )
        {
            const CRect tab = tabs->GetTabRect( t );
            if ( ( tab.IsEmpty() && !tabs->HasOverflow() ) || ( !tab.IsEmpty() && !IsInside( tab, local, 0 ) ) )
                Report( index, LAYOUTLINTRULE_TABUNREACHABLE, LAYOUTLINTSEVERITY_ERROR, CRect( tab.left + rect.left, tab.top + rect.top, tab.width, tab.height ), rect,
                        tabs->GetTabTitle( t ), "has no room in the tab strip and no overflow button reaches it" );
        }
    }

    // L17: an anchored panel full of controls
    const bool flow = widget.IsRegisteredAs( "HBox" ) || widget.IsRegisteredAs( "VBox" ) || widget.IsRegisteredAs( "Grid" ) || widget.IsRegisteredAs( "Margin" );
    if ( !flow && widget.IsRegisteredAs( "Panel" ) )
    {
        Int32 interactive = 0;
        for ( UInt32 c=0; c<(UInt32) node.children.size(); ++c )
            if ( nodes[ node.children[ c ] ].control )
                ++interactive;
        if ( interactive > options->maxAnchoredInteractive )
            Report( index, LAYOUTLINTRULE_ABSOLUTEINFLOW, LAYOUTLINTSEVERITY_INFO, rect, CRect(), CString::Empty,
                    "places " + CORE::Int32ToString( interactive ) + " controls by pixel offsets: a candidate for boxes" );
    }

    CheckSiblings( index );
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintState::CheckTargets( void )
{
    // L07: targets below the minimum, unless the circle of the minimum around each meets no other target (WCAG 2.5.8 spacing)
    const Int32 minimum = options->minHitTarget;
    const Int32 radius = minimum / 2;
    GUCEF::vector< Int32 > targets;
    for ( UInt32 i=0; i<(UInt32) nodes.size(); ++i )
        if ( nodes[ i ].interactive && !nodes[ i ].zeroSize && !nodes[ i ].rect.IsEmpty() && nodes[ i ].seen )
            targets.push_back( (Int32) i );
    for ( UInt32 t=0; t<(UInt32) targets.size(); ++t )
    {
        const SNode& node = nodes[ targets[ t ] ];
        const CRect& rect = node.rect;
        if ( rect.width >= minimum && rect.height >= minimum )
            continue;
        const Int32 cx = rect.left + rect.width / 2;
        const Int32 cy = rect.top + rect.height / 2;
        bool spaced = true;
        for ( UInt32 o=0; o<(UInt32) targets.size() && spaced; ++o )
        {
            if ( o == t )
                continue;
            const SNode& other = nodes[ targets[ o ] ];
            const CRect& r = other.rect;

            // a target that holds this one (a list behind its row buttons) does not count against it, nor one it holds
            if ( IsInside( rect, r, 0 ) || IsInside( r, rect, 0 ) )
                continue;
            const Int32 nx = cx < r.left ? r.left : ( cx > r.Right() ? r.Right() : cx );
            const Int32 ny = cy < r.top ? r.top : ( cy > r.Bottom() ? r.Bottom() : cy );
            if ( ( nx - cx ) * ( nx - cx ) + ( ny - cy ) * ( ny - cy ) < radius * radius )
                spaced = false;
            if ( spaced && ( r.width < minimum || r.height < minimum ) )
            {
                // two undersized targets: their circles must not meet
                const Int32 ox = r.left + r.width / 2;
                const Int32 oy = r.top + r.height / 2;
                if ( ( ox - cx ) * ( ox - cx ) + ( oy - cy ) * ( oy - cy ) < minimum * minimum )
                    spaced = false;
            }
        }
        if ( !spaced )
            Report( targets[ t ], LAYOUTLINTRULE_SMALLTARGET, LAYOUTLINTSEVERITY_ERROR, rect, CRect(), OwnText( *node.widget ),
                    "is " + CORE::Int32ToString( rect.width ) + "x" + CORE::Int32ToString( rect.height ) + " px, a target needs " +
                    CORE::Int32ToString( minimum ) + "x" + CORE::Int32ToString( minimum ) + " or room around it" );
    }
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintState::Run( void )
{
    nodes.clear();
    fills.clear();
    estimatedFonts.clear();
    screen = CRect( 0, 0, (Int32) context->GetLogicalWidth(), (Int32) context->GetLogicalHeight() );

    if ( GUCEF_NULL != options->root )
    {
        if ( options->root->IsEffectivelyVisible() )
            AddNode( *options->root, -1, screen );
    }
    else
    {
        const CGUIContext::TWidgetVector& roots = context->GetRootWidgets();
        for ( UInt32 i=0; i<(UInt32) roots.size(); ++i )
            AddNode( *roots[ i ], -1, screen );
    }
    for ( UInt32 i=0; i<(UInt32) nodes.size(); ++i )
        CheckWidget( (Int32) i );
    CheckTargets();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLayoutLint::CLayoutLint( void )
    : m_state( GUCEF_NEW CLayoutLintState() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLayoutLint::~CLayoutLint()
{GUCEF_TRACE;

    GUCEF_DELETE m_state;
    m_state = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CLayoutLint::Run( CGUIContext& context, const CLayoutLintOptions& options, TLayoutLintFindingVector& findings )
{GUCEF_TRACE;

    findings.clear();
    if ( options.layoutFirst )
        context.FlushChanges();
    m_state->context = &context;
    m_state->options = &options;
    m_state->findings = &findings;
    m_state->Run();
    m_state->findings = GUCEF_NULL;
    m_state->commands.clear();
    std::sort( findings.begin(), findings.end() );
    return CountAtLeast( findings, options.strict ? LAYOUTLINTSEVERITY_WARNING : options.failSeverity );
}

/*-------------------------------------------------------------------------*/

CString
CLayoutLint::Report( const TLayoutLintFindingVector& findings, UInt32 maxFindings )
{GUCEF_TRACE;

    UInt32 counts[ LAYOUTLINTSEVERITY_COUNT ] = { 0, 0, 0 };
    for ( UInt32 i=0; i<(UInt32) findings.size(); ++i )
        ++counts[ findings[ i ].severity ];
    CString report = CORE::UInt32ToString( counts[ LAYOUTLINTSEVERITY_ERROR ] ) + " errors, " + CORE::UInt32ToString( counts[ LAYOUTLINTSEVERITY_WARNING ] ) +
                     " warnings, " + CORE::UInt32ToString( counts[ LAYOUTLINTSEVERITY_INFO ] ) + " info";
    for ( UInt32 i=0; i<(UInt32) findings.size(); ++i )
    {
        if ( 0 != maxFindings && i >= maxFindings )
        {
            report += "\n  ... " + CORE::UInt32ToString( (UInt32) findings.size() - maxFindings ) + " more";
            break;
        }
        report += "\n  " + findings[ i ].Describe();
    }
    return report;
}

/*-------------------------------------------------------------------------*/

UInt32
CLayoutLint::CountAtLeast( const TLayoutLintFindingVector& findings, ELayoutLintSeverity severity )
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( UInt32 i=0; i<(UInt32) findings.size(); ++i )
        if ( findings[ i ].severity >= severity )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

void
CLayoutLint::CountByRule( const TLayoutLintFindingVector& findings, ELayoutLintSeverity minSeverity, UInt32* counts )
{GUCEF_TRACE;

    if ( GUCEF_NULL == counts )
        return;
    for ( UInt32 i=0; i<(UInt32) findings.size(); ++i )
        if ( findings[ i ].severity >= minSeverity && (UInt32) findings[ i ].rule < (UInt32) LAYOUTLINTRULE_COUNT )
            ++counts[ findings[ i ].rule ];
}

/*-------------------------------------------------------------------------*/

Float32
CLayoutLint::GetContrastRatio( const CColour& a, const CColour& b )
{GUCEF_TRACE;

    const Float32 la = Luminance( a );
    const Float32 lb = Luminance( b );
    const Float32 lighter = la > lb ? la : lb;
    const Float32 darker = la > lb ? lb : la;
    return ( lighter + 0.05f ) / ( darker + 0.05f );
}

/*-------------------------------------------------------------------------*/

bool
CLayoutLint::EndsWithEllipsis( const CString& text )
{GUCEF_TRACE;

    const UInt32 length = text.Length();
    return length >= 3 && '.' == text[ length - 1 ] && '.' == text[ length - 2 ] && '.' == text[ length - 3 ];
}

/*-------------------------------------------------------------------------*/

bool
CLayoutLint::LooksNumeric( const CString& text )
{GUCEF_TRACE;

    // digits, with what numbers are written with: signs, separators, a currency sign, a percent and a short unit ("27 B", "4.0 KiB")
    CString value = EndsWithEllipsis( text ) ? text.SubstrFromRange( 0, text.Length() - 3 ) : text;
    value = value.Trim( true ).Trim( false );
    if ( value.IsNULLOrEmpty() )
        return false;
    UInt32 digits = 0, letters = 0;
    for ( UInt32 i=0; i<value.Length(); ++i )
    {
        const char c = value[ i ];
        if ( c >= '0' && c <= '9' )
            ++digits;
        else
        if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) )
            ++letters;
        else
        if ( !( '.' == c || ',' == c || '-' == c || '+' == c || ' ' == c || '%' == c || '$' == c || ':' == c || '/' == c ) )
            return false;
    }
    if ( 0 == digits || !( value[ 0 ] >= '0' && value[ 0 ] <= '9' ) && '-' != value[ 0 ] && '+' != value[ 0 ] && '$' != value[ 0 ] )
        return false;
    return letters <= 3;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
