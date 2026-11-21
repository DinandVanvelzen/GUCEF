/*
 *  AssetWorkbenchLib: the tooltips of the workbench: one table of texts by the name of the control, which people can review as a whole
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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHTOOLTIPS_H
#define ASSETWORKBENCHLIB_CWORKBENCHTOOLTIPS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CITOOLTIPTEXTPROVIDER_H
#include "gucefGUI_CITooltipTextProvider.h"
#define GUCEF_GUI_CITOOLTIPTEXTPROVIDER_H
#endif /* GUCEF_GUI_CITOOLTIPTEXTPROVIDER_H ? */

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The tooltips of every control of the workbench, in one table (the file of this class) of texts by the name of the control, so that the
 *  texts can be reviewed together and say the same things in the same way. The tooltip service of the GUI context asks it for the text of a
 *  widget that has no tooltip of its own (CITooltipTextProvider), which also covers the widgets that forms make later, such as a dialog
 *  that opens for the first time. A tooltip that a widget has itself goes first.
 *
 *  The key of an entry is the name of the widget ("Back"), or the name of its parent and the name ("Pages/Close") when the same name has
 *  other texts in other places; the tab of a tab control has the key "<name of the control>/<title of the tab>".
 *
 *  A control is a widget that a person can use: a button, check box, combo box, edit box, list, tab control, slider, spinner, grid, tree, foldable
 *  panel, menu bar or secret field. FindMissing() lists the controls of a widget tree that no text reaches, for the tests.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchTooltips : public GUI::CITooltipTextProvider
{
    public:

    CWorkbenchTooltips( void );

    virtual ~CWorkbenchTooltips() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetTooltipFor( const GUI::CWidget& widget, Int32 localX, Int32 localY ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The text of the key, empty for none */
    CORE::CString FindText( const CORE::CString& key ) const;

    /** The number of entries of the table */
    UInt32 GetEntryCount( void ) const;

    /** Whether the widget is a control of the kinds above */
    static bool IsControl( const GUI::CWidget& widget );

    /**
     *  The paths (CWidget::GetNamePath()) of the controls in the tree of the widget, the widget included, that have no text: neither a tooltip
     *  of their own nor one from the table. Controls that are hidden are included; the number found
     */
    UInt32 FindMissing( const GUI::CWidget& root, CORE::CString::StringVector& missing ) const;

    private:

    CWorkbenchTooltips( const CWorkbenchTooltips& src );                /**< not implemented */
    CWorkbenchTooltips& operator=( const CWorkbenchTooltips& src );     /**< not implemented */

    CORE::CString FindTextOfWidget( const GUI::CWidget& widget, Int32 localX, Int32 localY ) const;

    private:

    typedef GUCEF::map< CORE::CString, CORE::CString > TTextMap;

    TTextMap m_texts;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHTOOLTIPS_H ? */
