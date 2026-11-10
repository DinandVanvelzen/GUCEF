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

#ifndef GUCEF_GUI_CSCROLLPANEL_H
#define GUCEF_GUI_CSCROLLPANEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

#ifndef GUCEF_GUI_CPANEL_H
#include "gucefGUI_CPanel.h"
#define GUCEF_GUI_CPANEL_H
#endif /* GUCEF_GUI_CPANEL_H ? */

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

/**
 *  A panel whose children can be taller than the panel: they are cut off at its edge and scrolled vertically with a
 *  scroll bar on the right. Registered as "ScrollPanel".
 *
 *  The children are placed like those of a plain panel, in the area left of the scroll bar (their relative sizes and
 *  positions refer to that area), then moved up by ScrollPosition pixels. The content is as high as the lowest edge of a
 *  visible child plus BottomPadding; the scroll position is kept between 0 and the content minus the panel's height, also
 *  when the panel or the children change size. Hit testing and drawing use the rectangles the children really have, so
 *  the clipping and the input follow the scrolling.
 *
 *  The children and the scroll bar sit in a column: the whole panel, or with MaxColumnWidth a column of at most that
 *  width in the middle of it, so the bar is next to the content of a wide panel and not at the far edge. The margins left
 *  and right of the column are part of the panel: the wheel and the keys scroll it there too.
 *
 *  Properties: ScrollPosition (pixels, as the ScrollPosition of a list box is rows), ScrollStep (pixels for a notch of
 *  the wheel), ScrollBarWidth, MaxColumnWidth, BottomPadding, TrackColour, TrackEdgeColour, ThumbColour, ThumbHoverColour,
 *  ThumbPressedColour. Runtime: ThumbHovered.
 *
 *  The wheel scrolls it (when the widget below the pointer has no use for it), the thumb can be dragged, a click on the
 *  track moves a page, PageUp, PageDown, Home and End scroll it, and a child that gets the keyboard focus (Tab, a click)
 *  is scrolled into view. The whole width of the bar takes the pointer, the thumb is never shorter than 32 pixels.
 */
class GUCEF_GUI_PUBLIC_CPP CScrollPanel : public CPanel
{
    public:

    CScrollPanel( void );

    virtual ~CScrollPanel();

    bool SetScrollPosition( Int32 pixels );

    Int32 GetScrollPosition( void ) const;

    /** The height of what the children take, as of the last layout */
    Int32 GetContentHeight( void ) const;

    /** The height of the part of the panel the content is seen in */
    Int32 GetViewportHeight( void ) const;

    /** The largest scroll position: the content less the viewport, 0 when everything fits */
    Int32 GetMaxScrollPosition( void ) const;

    /** Whether the content is higher than the viewport */
    bool IsScrollable( void ) const;

    /** The column the children and the scroll bar are in, relative to the panel: the whole panel or the middle part of MaxColumnWidth */
    CRect GetColumnRect( void ) const;

    /** Where the children are, relative to the panel: the column left of the scroll bar */
    CRect GetViewportRect( void ) const;

    /** The scroll bar relative to the panel: the right end of the column */
    CRect GetTrackRect( void ) const;

    /** The thumb relative to the panel, empty when the content fits */
    CRect GetThumbRect( void ) const;

    /** Scrolls the least that shows the widget (a descendant) completely, or its top when it is higher than the viewport */
    bool ScrollIntoView( const CWidget* widget );

    /** Whether the thumb is held */
    bool IsThumbDragged( void ) const;

    /**
     *  Scrolls for a key: PageUp and PageDown a page, Home and End to the start and the end, and with lines also the arrows
     *  a step. False when the key is not one of those or there is nothing to scroll
     */
    bool ScrollByKey( INPUT::KeyCode keyCode, bool lines );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CScrollPanel* AsScrollPanel( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CScrollPanel* AsScrollPanel( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static CWidget* Create( void );

    protected:

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnDescendantFocusReceived( CWidget* descendant ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CGUIContext;

    /** The layout measured the content */
    void SetExtent( Int32 contentHeight, Int32 viewportHeight );

    /** Scrolls to the position (kept within the range) as a user change; false when that is where it already was */
    bool ScrollByUser( Int32 position );
    void DragThumbTo( Int32 localY );

    CScrollPanel( const CScrollPanel& src );
    CScrollPanel& operator=( const CScrollPanel& src );

    private:

    Int32 m_contentHeight;
    Int32 m_viewportHeight;
    bool m_thumbDragged;
    Int32 m_thumbGrabOffset;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CSCROLLPANEL_H ? */
