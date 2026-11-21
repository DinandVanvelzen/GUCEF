/*
 *  AssetWorkbenchLib: the help of the workbench: the documents and the briefs of the help catalogue that the help window and the tooltips read
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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHHELP_H
#define ASSETWORKBENCHLIB_CWORKBENCHHELP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CHELPCATALOGUE_H
#include "gucefGUI_CHelpCatalogue.h"
#define GUCEF_GUI_CHELPCATALOGUE_H
#endif /* GUCEF_GUI_CHELPCATALOGUE_H ? */

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
 *  The help of the workbench as a catalogue of gucefGUI (a help source that the tooltip service and the help window read): the
 *  documents Overview, Opening assets, Views and layouts, Importing and conversion, Settings, Developer console and Keyboard shortcuts, in
 *  the markup of the help classes, kept in this module so that the help is there in every build and in the tests. The document of the
 *  shortcuts lists the keys that are in effect, which the application tells with SetShortcutRows().
 *
 *  Areas of the window point to a document with their property HelpTopic, so that F1 opens the help of the place the person is at:
 *  FindTopicOfWidget() follows the focus up to the nearest widget that has one.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchHelp
{
    public:

    static const CORE::CString TopicOverview;       /**< "workbench.overview" */
    static const CORE::CString TopicOpening;        /**< "workbench.opening" */
    static const CORE::CString TopicViews;          /**< "workbench.views" */
    static const CORE::CString TopicImporting;      /**< "workbench.importing" */
    static const CORE::CString TopicSettings;       /**< "workbench.settings" */
    static const CORE::CString TopicConsole;        /**< "workbench.console" */
    static const CORE::CString TopicShortcuts;      /**< "workbench.shortcuts" */

    CWorkbenchHelp( void );

    ~CWorkbenchHelp();

    /** The catalogue as the help source for the tooltip service and the help window */
    const GUI::THelpSourcePtr& GetSource( void ) const;

    GUI::CHelpCatalogue& GetCatalogue( void );

    /** The rows of the table of the shortcuts document: the text of the action and the text of its key chord */
    void SetShortcutRows( const CORE::CString::StringVector& actions, const CORE::CString::StringVector& chords );

    /**
     *  The document of the place of the widget: its property HelpTopic or that of its nearest ancestor that has one, the overview for a
     *  widget that has none and for none at all
     */
    CORE::CString FindTopicOfWidget( const GUI::CWidget* widget ) const;

    /** Checks the catalogue as a whole (no broken link, every document has a brief); true when there is no problem */
    bool Validate( CORE::CString::StringVector& problems ) const;

    private:

    CWorkbenchHelp( const CWorkbenchHelp& src );                /**< not implemented */
    CWorkbenchHelp& operator=( const CWorkbenchHelp& src );     /**< not implemented */

    private:

    GUI::THelpSourcePtr m_source;
    GUI::CHelpCatalogue* m_catalogue;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHHELP_H ? */
