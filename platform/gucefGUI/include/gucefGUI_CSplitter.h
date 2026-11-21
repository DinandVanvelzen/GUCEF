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

#ifndef GUCEF_GUI_CSPLITTER_H
#define GUCEF_GUI_CSPLITTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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

/**
 *  Two panes with a handle between them that the user drags to give one more room and the other less. The first two
 *  children of the splitter are the panes (the first and the second one); the layout of the context puts them on either side of
 *  the handle. Registered as "Splitter".
 *
 *  Properties:
 *  - Orientation: Horizontal puts the panes side by side (a vertical bar between them), Vertical puts them above each other.
 *  - SplitRatio: the part of the room (the length less the handle) that the first pane has, 0 to 1. It is what the user changes and
 *    what a layout state keeps: a window that grows gives both panes more room.
 *  - DefaultSplitRatio: what a double click on the handle goes back to.
 *  - FirstMinSize, SecondMinSize: the least pixels each pane keeps along the splitter, for the handle, the keys and the layout.
 *  - HandleSize: the thickness of the handle in pixels; HandleColour and HoverColour (the handle under the pointer or dragged).
 *
 *  A pane that is a CFoldablePanel and is folded has the size of its strip, the other pane gets the rest and the handle is not
 *  shown and cannot be used until the pane is unfolded: the ratio is kept, so unfolding gives the size from before.
 *
 *  Input: drag the handle, a double click on it resets the ratio, the arrow keys along the splitter move it by KeyStep pixels while
 *  the splitter has the focus. The pointer over the handle is a resize cursor.
 *
 *  Events (observe the splitter): SizeChangedEvent when the ratio changes, for the user and for code alike (CValueChangedEventData,
 *  property SplitRatio); CollapsedChangedEvent when a pane folds or unfolds (CValueChangedEventData, property Collapsed, 'source'
 *  is the pane, the values "true" and "false").
 */
class GUCEF_GUI_PUBLIC_CPP CSplitter : public CPanel
{
    public:

    static const CORE::CEvent SizeChangedEvent;
    static const CORE::CEvent CollapsedChangedEvent;

    static void RegisterEvents( void );

    /** Pixels the arrow keys move the handle by */
    enum
    {
        KEYSTEP = 16
    };

    CSplitter( void );

    CSplitter( const CString& widgetTypeName );

    virtual ~CSplitter();

    /** Whether the panes are side by side (Orientation Horizontal) */
    bool IsSideBySide( void ) const;

    Float32 GetSplitRatio( void ) const;

    /** Sets the ratio, limited to 0 to 1; the layout keeps the minimum sizes */
    bool SetSplitRatio( Float32 ratio );

    /** Goes back to DefaultSplitRatio */
    bool ResetSplitRatio( void );

    /** Gives the first pane this many pixels (as far as the minimum sizes allow) by setting the ratio that has that outcome */
    bool SetFirstPaneExtent( Int32 pixels );

    /** The pixels of the first pane, the handle and the second pane along the splitter as the layout gives them now */
    Int32 GetFirstPaneExtent( void ) const;
    Int32 GetHandleExtent( void ) const;
    Int32 GetSecondPaneExtent( void ) const;

    CWidget* GetFirstPane( void ) const;
    CWidget* GetSecondPane( void ) const;

    /** Whether the pane is a folded CFoldablePanel */
    bool IsFirstPaneFolded( void ) const;
    bool IsSecondPaneFolded( void ) const;

    /** Where the layout puts the panes, in the coordinates of the context; called by the context while it places the children */
    void GetPaneRects( CRect& first, CRect& second ) const;

    /** The handle relative to the widget; empty when a pane is folded */
    CRect GetHandleRect( void ) const;

    /** Whether the pointer is on the handle or the handle is being dragged: the handle is drawn lighter */
    bool IsHandleActive( void ) const;

    bool IsDragging( void ) const;

    /** A pane (a CFoldablePanel) folded or unfolded; the panel tells its splitter */
    void OnPaneFoldChanged( CWidget& pane );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "Splitter"; }

    static CWidget* Create( void );

    protected:

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CSplitter( const CSplitter& src );
    CSplitter& operator=( const CSplitter& src );

    /** The length along the splitter, and the extents of the first pane, the handle and the second pane that add up to it */
    Int32 GetTotalExtent( void ) const;
    void ComputeExtents( Int32& first, Int32& handle, Int32& second ) const;
    Int32 GetFoldedExtentOf( const CWidget* pane ) const;
    void SetHandleActive( bool active );
    void UpdateCursor( bool onHandle );
    void RestoreCursor( void );
    void EndDrag( void );

    private:

    bool m_dragging;
    bool m_handleActive;
    bool m_cursorApplied;
    UInt32 m_previousCursor;
    Int32 m_grabOffset;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CSPLITTER_H ? */
