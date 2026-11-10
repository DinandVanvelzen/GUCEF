/*
 *  gucefAUTHGUI_TestApp: Application for testing gucefAUTHGUI
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_AUTHGUI_TESTAPP_TESTLAYOUTUTILS_H
#define GUCEF_AUTHGUI_TESTAPP_TESTLAYOUTUTILS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefAUTHGUI_TestCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace {

typedef GUCEF::vector< GUI::CWidget* > TWidgetVector;

struct SScreenSize
{
    GUCEF::UInt32 width;
    GUCEF::UInt32 height;
};

/** The sizes the forms are made for: the smallest, common ones, the window of the developer, a large one, ultra wide (21:9, 32:9) and a portrait one */
const SScreenSize ScreenSizes[] = { { 800, 600 }, { 1024, 768 }, { 1242, 952 }, { 1920, 1080 }, { 3440, 1440 }, { 3840, 1080 }, { 5120, 1440 }, { 600, 900 } };
const UInt32 ScreenSizeCount = 8;

/** The most a scroll bar may be from what it scrolls: in a wide window it sits at the content, not at the far edge */
const Int32 ScrollBarGapLimit = 24;

inline CORE::CString
SizeName( UInt32 width, UInt32 height )
{
    return CORE::UInt32ToString( width ) + "x" + CORE::UInt32ToString( height );
}

inline void
Collect( GUI::CWidget* widget, TWidgetVector& all )
{
    all.push_back( widget );
    const GUI::CWidget::TWidgetVector& children = widget->GetChildren();
    for ( UInt32 i=0; i<(UInt32)children.size(); ++i )
        Collect( children[ i ], all );
}

inline bool
IsContainerType( const CORE::CString& type )
{
    return type == "Panel" || type == "Form" || type == "VBox" || type == "HBox" || type == "Grid" || type == "Margin" ||
           type == "GroupBox" || type == "Window" || type == "ScrollPanel" || type == "TabControl" || type == "TabContentPane";
}

inline bool
IsDescendantOrSelf( const GUI::CWidget* widget, const GUI::CWidget* ancestor )
{
    for ( const GUI::CWidget* w=widget; GUCEF_NULL != w; w=w->GetParentWidget() )
        if ( w == ancestor )
            return true;
    return false;
}

inline bool
Inside( const GUI::CRect& inner, const GUI::CRect& outer )
{
    return inner.left >= outer.left && inner.top >= outer.top && inner.Right() <= outer.Right() && inner.Bottom() <= outer.Bottom();
}

inline bool
Intersects( const GUI::CRect& a, const GUI::CRect& b )
{
    return !a.IsEmpty() && !b.IsEmpty() && !a.Intersection( b ).IsEmpty();
}

/** The part of a widget that is seen: cut off at every ancestor that clips (a scroll panel, a window) */
inline GUI::CRect
VisibleRect( const GUI::CWidget* widget )
{
    GUI::CRect rect = widget->GetRect();
    for ( const GUI::CWidget* p=widget->GetParentWidget(); GUCEF_NULL != p; p=p->GetParentWidget() )
        if ( p->GetBoolProperty( "Clip" ) )
            rect = rect.Intersection( p->GetRect() );
    return rect;
}

/**
 *  The first thing that is wrong with the layout of a form, empty when it is fine:
 *  - a widget outside the rectangle of its parent (the width of a scroll panel less its scroll bar for what is in it; what
 *    is scrolled out vertically is cut off and is fine)
 *  - two controls that are seen and overlap
 *  - something that is seen overlapping the strip along the bottom edge, which has the buttons
 */
inline CORE::CString
FindLayoutProblem( GUI::CWidget* root )
{
    TWidgetVector all;
    Collect( root, all );

    for ( UInt32 i=1; i<(UInt32)all.size(); ++i )
    {
        GUI::CWidget* widget = all[ i ];
        GUI::CWidget* parent = widget->GetParentWidget();
        if ( !widget->IsVisible() || widget->GetRect().IsEmpty() )
            continue;
        GUI::CRect parentRect = parent->GetRect();
        const GUI::CScrollPanel* scrollParent = GUI::CWidget::ToScrollPanel( parent );
        const bool scrolls = GUCEF_NULL != scrollParent;
        if ( scrolls )
        {
            // the part of the column left of the scroll bar
            const GUI::CRect viewport = scrollParent->GetViewportRect();
            parentRect = GUI::CRect( parentRect.left + viewport.left, parentRect.top, viewport.width, parentRect.height );
        }
        GUI::CRect rect = widget->GetRect();
        if ( scrolls )
        {
            if ( rect.left < parentRect.left || rect.Right() > parentRect.Right() )
                return "\"" + widget->GetName() + "\" is wider than the scroll panel \"" + parent->GetName() + "\"";
        }
        else
        if ( !Inside( rect, parentRect ) )
            return "\"" + widget->GetName() + "\" (" + widget->GetType() + ") is outside \"" + parent->GetName() + "\"";
    }

    // the scroll bar is next to what it scrolls, also in a wide window, and inside the panel
    for ( UInt32 i=0; i<(UInt32)all.size(); ++i )
    {
        const GUI::CScrollPanel* scrollPanel = GUI::CWidget::ToScrollPanel( all[ i ] );
        if ( GUCEF_NULL == scrollPanel || !scrollPanel->IsVisible() || scrollPanel->GetRect().IsEmpty() )
            continue;
        const GUI::CRect panelRect = scrollPanel->GetRect();
        const GUI::CRect track = scrollPanel->GetTrackRect();
        const GUI::CRect viewport = scrollPanel->GetViewportRect();
        if ( track.IsEmpty() || track.left < 0 || track.Right() > panelRect.width || viewport.Right() != track.left )
            return "the scroll bar of \"" + scrollPanel->GetName() + "\" is not at the right end of its column";
        Int32 contentRight = -1;
        const GUI::CWidget::TWidgetVector& children = scrollPanel->GetChildren();
        for ( UInt32 c=0; c<(UInt32)children.size(); ++c )
            if ( children[ c ]->IsVisible() && !children[ c ]->GetRect().IsEmpty() && children[ c ]->GetRect().Right() > contentRight )
                contentRight = children[ c ]->GetRect().Right();
        if ( contentRight >= 0 )
        {
            const Int32 gap = panelRect.left + track.left - contentRight;
            if ( gap < 0 || gap > ScrollBarGapLimit )
                return "the scroll bar of \"" + scrollPanel->GetName() + "\" is " + CORE::Int32ToString( gap ) + " pixels from what it scrolls";
        }
    }

    // the controls that are seen do not overlap
    TWidgetVector controls;
    for ( UInt32 i=0; i<(UInt32)all.size(); ++i )
        if ( !IsContainerType( all[ i ]->GetType() ) && all[ i ]->IsVisible() && !VisibleRect( all[ i ] ).IsEmpty() )
            controls.push_back( all[ i ] );
    for ( UInt32 i=0; i<(UInt32)controls.size(); ++i )
        for ( UInt32 j=i+1; j<(UInt32)controls.size(); ++j )
            if ( Intersects( VisibleRect( controls[ i ] ), VisibleRect( controls[ j ] ) ) )
                return "\"" + controls[ i ]->GetName() + "\" overlaps \"" + controls[ j ]->GetName() + "\"";

    // the strip with the buttons
    GUI::CWidget* strip = GUCEF_NULL;
    for ( UInt32 i=0; i<(UInt32)all.size(); ++i )
        if ( all[ i ]->GetName() == "ButtonRow" )
            strip = all[ i ];
    if ( GUCEF_NULL != strip )
    {
        for ( UInt32 i=0; i<(UInt32)all.size(); ++i )
        {
            if ( all[ i ] == root || IsDescendantOrSelf( all[ i ], strip ) || !all[ i ]->IsVisible() )
                continue;
            if ( Intersects( VisibleRect( all[ i ] ), strip->GetRect() ) )
                return "\"" + all[ i ]->GetName() + "\" overlaps the strip of the buttons";
        }
    }
    return CORE::CString::Empty;
}

/**
 *  A form in a window of a limited width has its scroll bars inside that window, not further than a margin from its right edge: in a wide
 *  screen they are with the content in the window and not at the edge of the screen. Empty when it is so
 */
inline CORE::CString
FindBarEdgeProblem( GUI::CWidget* window )
{
    TWidgetVector all;
    Collect( window, all );
    const GUI::CRect windowRect = window->GetRect();
    const GUI::CScrollPanel* outermost = GUCEF_NULL;
    Int32 outermostRight = 0;
    for ( UInt32 i=0; i<(UInt32)all.size(); ++i )
    {
        const GUI::CScrollPanel* scrollPanel = GUI::CWidget::ToScrollPanel( all[ i ] );
        if ( GUCEF_NULL == scrollPanel || !scrollPanel->IsVisible() || scrollPanel->GetRect().IsEmpty() )
            continue;
        const Int32 trackRight = scrollPanel->GetRect().left + scrollPanel->GetTrackRect().Right();
        if ( trackRight > windowRect.Right() )
            return "the scroll bar of \"" + scrollPanel->GetName() + "\" is beyond the right edge of the window";
        if ( GUCEF_NULL == outermost || trackRight > outermostRight )
        {
            outermost = scrollPanel;
            outermostRight = trackRight;
        }
    }
    // the one furthest right: next to the edge of the window; the others are in columns and are checked against what they scroll
    if ( GUCEF_NULL != outermost && windowRect.Right() - outermostRight > 48 )
        return "the scroll bar of \"" + outermost->GetName() + "\" is " + CORE::Int32ToString( windowRect.Right() - outermostRight ) + " pixels from the right edge of the window";
    return CORE::CString::Empty;
}

/** Every pixel outside the rectangle is black: nothing is drawn beyond the window */
inline bool
OnlyBlackOutside( const IMAGE::TPixelMapPtr& frame, const GUI::CRect& rect )
{
    if ( frame.IsNULL() )
        return false;
    const UInt8* data = static_cast< const UInt8* >( frame->GetDataPtr() );
    const Int32 width = (Int32) frame->GetWidthInPixels();
    const Int32 height = (Int32) frame->GetHeightInPixels();
    for ( Int32 y=0; y<height; ++y )
    {
        for ( Int32 x=0; x<width; ++x )
        {
            if ( x >= rect.left && x < rect.Right() && y >= rect.top && y < rect.Bottom() )
                continue;
            const UInt8* pixel = data + ( (size_t) y * width + x ) * 4;
            if ( 0 != pixel[ 0 ] || 0 != pixel[ 1 ] || 0 != pixel[ 2 ] )
                return false;
        }
    }
    return true;
}

/** How many pixels inside the rectangle are not black: the window has been drawn */
inline UInt32
CountLit( const IMAGE::TPixelMapPtr& frame, const GUI::CRect& rect )
{
    UInt32 lit = 0;
    if ( frame.IsNULL() )
        return 0;
    const UInt8* data = static_cast< const UInt8* >( frame->GetDataPtr() );
    const Int32 width = (Int32) frame->GetWidthInPixels();
    for ( Int32 y=rect.top; y<rect.Bottom(); y+=3 )
        for ( Int32 x=rect.left; x<rect.Right(); x+=3 )
            if ( 0 != data[ ( (size_t) y * width + x ) * 4 + 1 ] )
                ++lit;
    return lit;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHGUI_TESTAPP_TESTLAYOUTUTILS_H ? */
