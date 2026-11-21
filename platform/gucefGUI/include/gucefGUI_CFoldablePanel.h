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

#ifndef GUCEF_GUI_CFOLDABLEPANEL_H
#define GUCEF_GUI_CFOLDABLEPANEL_H

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
 *  A panel with a header strip (its title and a fold button) over its content, that folds into a thin strip along one edge of
 *  its parent that shows only the title, and unfolds again. Registered as "FoldablePanel".
 *
 *  Properties:
 *  - Title: the text of the header; TitleHeight: the thickness of the header and of the folded strip (24 by default);
 *    TitleBarColour and TitleColour: its colours.
 *  - Dock: the edge of the parent the panel sits at, Left, Right, Top or Bottom. It says where the folded strip is and which way
 *    the fold button points. A folded panel at Left or Right is a vertical strip (the title reads downwards, a letter to a line),
 *    one at Top or Bottom a horizontal strip.
 *  - Collapsed: whether the panel is folded. A folded panel does not show its children and they take no input.
 *
 *  The children are placed below the header. In a CSplitter a folded pane has the size of the strip and the other pane gets the
 *  rest; the size the pane had stays with the splitter, so unfolding brings it back. Outside a splitter a folded panel makes
 *  itself the strip at its Dock edge of the room the layout gave it.
 *
 *  Input: a click on the header, or on the strip while folded, folds and unfolds. Events: FoldedChangedEvent
 *  (CValueChangedEventData, property Collapsed, the values "true" and "false").
 */
class GUCEF_GUI_PUBLIC_CPP CFoldablePanel : public CPanel
{
    public:

    static const CORE::CEvent FoldedChangedEvent;

    static void RegisterEvents( void );

    CFoldablePanel( void );

    virtual ~CFoldablePanel();

    bool IsFolded( void ) const;

    bool SetFolded( bool folded );

    bool ToggleFolded( void );

    /** Whether the folded strip is a vertical one (Dock Left or Right) */
    bool IsStripVertical( void ) const;

    /** The thickness of the strip while folded: the width of a vertical one, the height of a horizontal one */
    Int32 GetFoldedExtent( void ) const;

    /** The header (unfolded) or the whole strip (folded), relative to the widget */
    CRect GetHeaderRect( void ) const;

    /** The fold button, relative to the widget */
    CRect GetFoldButtonRect( void ) const;

    /** Where the children are placed, in the coordinates of the context: below the header; empty while folded */
    CRect GetContentRect( void ) const;

    virtual bool AreChildrenShown( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "FoldablePanel"; }

    static CWidget* Create( void );

    protected:

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void AdjustPlacement( CRect& rect, const CRect& area, bool sizeFromArea ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CFoldablePanel( const CFoldablePanel& src );
    CFoldablePanel& operator=( const CFoldablePanel& src );

    void UpdateFoldedState( void );

    private:

    bool m_folded;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CFOLDABLEPANEL_H ? */
