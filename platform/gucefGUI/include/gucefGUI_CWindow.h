/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#ifndef GUCEF_GUI_CWINDOW_H
#define GUCEF_GUI_CWINDOW_H

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
 *  A panel with a title bar. Properties: Title, TitleVisible, TitleHeight, TitleBarColour, TitleColour,
 *  Movable, Collapsible, Collapsed, MinSize, MaxSize. Runtime: FoldButtonHovered, FoldButtonPressed.
 *
 *  Dragging the title bar moves the window: the offset parts of its Position change, the relative parts stay, so it
 *  works with every layout. The title bar stays inside the parent (the screen for a root): a window that fits is
 *  kept completely inside horizontally, a wider one covers the parent; vertically the title bar stays inside. A
 *  window the user moved is kept in place when its parent changes size. The pointer stays captured while the button is
 *  down, also outside the title bar.
 *
 *  A fold button at the right end of the title bar and a double click on the title bar collapse the window to its
 *  title bar (Collapsed) and expand it again. A collapsed window shows no children, they take no input and no focus;
 *  its Size and Position are not changed, so expanding restores both (and a window that was moved while it was
 *  collapsed keeps the place it was dropped on).
 *
 *  MinSize and MaxSize (pixels, empty for none, per axis 0 for none) limit the size the layout gives the window,
 *  so a window of 90% of the screen can stop growing on a large one. A minimum larger than the parent is the size of the parent.
 *
 *  Windows that are notifications or overlays set Movable and Collapsible to false.
 */
class GUCEF_GUI_PUBLIC_CPP CWindow : public CPanel
{
    public:

    static const CORE::CEvent MovedEvent;               /**< the user moved the window: CValueChangedEventData, property "Position" with the old and new value */
    static const CORE::CEvent CollapsedChangedEvent;    /**< the window collapsed or expanded: CValueChangedEventData, property "Collapsed" */

    static void RegisterEvents( void );

    public:

    CWindow( void );

    virtual ~CWindow();

    virtual bool SetTitleBarText( const CString& title );

    virtual bool GetTitleBarText( CString& title ) const;

    virtual bool SetTitleBarVisible( const bool isVisible );

    virtual bool GetTitleBarVisible( bool& isVisible ) const;

    bool SetMovable( bool movable );

    /** Whether the user can move the window by its title bar */
    bool IsMovable( void ) const;

    bool SetCollapsible( bool collapsible );

    /** Whether the user can fold the window into its title bar (the fold button, a double click on the title bar) */
    bool IsCollapsible( void ) const;

    bool SetCollapsed( bool collapsed );

    /** Whether the window is folded into its title bar now: Collapsed, with a title bar to fold into and Collapsible */
    bool IsCollapsed( void ) const;

    /** Folds or unfolds the window */
    bool ToggleCollapsed( void );

    /** The height of the title bar, 0 without one */
    Int32 GetTitleBarHeight( void ) const;

    /** The title bar relative to the window, empty without one */
    CRect GetTitleBarRect( void ) const;

    /** The fold button relative to the window, empty when the window is not collapsible */
    CRect GetFoldButtonRect( void ) const;

    /** Whether the user holds the title bar now */
    bool IsDragging( void ) const;

    /** Whether the user moved the window (it is kept where it was put) */
    bool IsMovedByUser( void ) const;

    virtual bool AreChildrenShown( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CWindow* AsWindow( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CWindow* AsWindow( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static CWidget* Create( void );

    protected:

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void AdjustSize( Int32& width, Int32& height, const CRect& area ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void AdjustPlacement( CRect& rect, const CRect& area, bool sizeFromArea ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** The area the window may move in: the parent, the screen for a root */
    CRect GetMoveArea( void ) const;

    /** The window rectangle 'rect' moved the least that keeps the title bar inside 'area' */
    void ClampToArea( CRect& rect, const CRect& area ) const;

    void BeginDrag( const CPointerEventData& pointer );
    void DragTo( const CPointerEventData& pointer );
    void EndDrag( void );
    void UpdateCollapsedState( void );

    CWindow( const CWindow& src );
    CWindow& operator=( const CWindow& src );

    private:

    bool m_dragging;
    bool m_movedByUser;
    bool m_collapsed;               /**< the state that was last reported */
    Int32 m_dragStartX;
    Int32 m_dragStartY;
    CUVector m_dragStartPosition;
    CRect m_dragStartRect;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CWINDOW_H ? */
