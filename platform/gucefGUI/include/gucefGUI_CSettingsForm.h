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

#ifndef GUCEF_GUI_CSETTINGSFORM_H
#define GUCEF_GUI_CSETTINGSFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CSETTINGSPAGEFORM_H
#include "gucefGUI_CSettingsPageForm.h"
#define GUCEF_GUI_CSETTINGSPAGEFORM_H
#endif /* GUCEF_GUI_CSETTINGSPAGEFORM_H ? */

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
 *  The settings screen of a game or tool: a window with a tab per page of settings and the buttons OK,
 *  Apply and Cancel. Apply applies the pages with changes, OK does that and closes, Cancel (or Escape)
 *  puts the settings back in the pages and closes.
 *
 *  Pages are CSettingsPageForm objects. The page factory knows them by name: gucefGUI registers "Video"
 *  (CVideoSettingsForm) and "General" (CGeneralSettingsForm), an application registers its own and picks
 *  the pages with AddPage(). AddPage() with an object adds a page the application made itself.
 *
 *  The layout is built in code and made for BuildDarkTheme(); it fills most of the context, which is
 *  meant to be a layer of its own at the resolution of the window.
 */
class GUCEF_GUI_PUBLIC_CPP CSettingsForm : public CFormEx
{
    public:

    static const CORE::CEvent SettingsAppliedEvent; /**< Apply or OK applied the pages with changes */
    static const CORE::CEvent ClosedEvent;          /**< OK or Cancel closed the form */

    static void RegisterEvents( void );

    typedef GUCEF::vector< CSettingsPageForm* > TPageVector;

    public:

    CSettingsForm( void );

    CSettingsForm( CIGUIContext* context );

    virtual ~CSettingsForm();

    /** The page types by name, for AddPage() */
    static CSettingsPageFactory& GetPageFactory( void );

    /** Registers the pages gucefGUI comes with: "Video" and "General"; done by CGuiGlobal at start */
    static void RegisterStandardPages( void );

    static void UnregisterStandardPages( void );

    /** Adds a page of a type the page factory knows; NULL when it does not know it */
    CSettingsPageForm* AddPage( const CString& pageTypeName );

    /** Adds a page the application made; it stays the application's and must not go before the form */
    bool AddPage( CSettingsPageForm* page );

    /** The pages in the order of their tabs */
    const TPageVector& GetPages( void ) const;

    CSettingsPageForm* GetPage( const CString& title ) const;

    /** Makes the window, a tab per page that was added, and the buttons */
    bool Build( void );

    /** Puts the settings in every page and shows the form, modal */
    bool Open( void );

    /** Open() with the page of that title in front (the page that was in front when there is none such) */
    bool Open( const CString& pageTitle );

    /** Brings the page of that title to the front; false when there is no such page or the form is not built */
    bool SelectPage( const CString& title );

    /** The title of the page that is in front, empty when the form is not built */
    CString GetSelectedPageTitle( void ) const;

    /** Applies the pages with changes; false when one of them failed */
    bool Apply( void );

    /** Apply() and close */
    bool Ok( void );

    /** Puts the settings back in the pages and closes */
    void Cancel( void );

    bool HasChanges( void ) const;

    void SetTitle( const CString& title );

    const CString& GetTitle( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CSettingsForm > TEventCallback;

    /** The user changed a value on a page */
    void OnPageChanged( CORE::CNotifier* notifier    ,
                        const CORE::CEvent& eventId  ,
                        CORE::CICloneable* eventData );

    void OnOkActivated( CORE::CNotifier* notifier    ,
                        const CORE::CEvent& eventId  ,
                        CORE::CICloneable* eventData );

    void OnApplyActivated( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData );

    void OnCancelActivated( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData );

    /** Escape cancels */
    void OnNavigation( CORE::CNotifier* notifier    ,
                       const CORE::CEvent& eventId  ,
                       CORE::CICloneable* eventData );

    void BuildLayout( CORE::CDataNode& layout );

    void UpdateButtons( void );

    void Close( void );

    CSettingsForm( const CSettingsForm& src );
    CSettingsForm& operator=( const CSettingsForm& src );

    private:

    typedef GUCEF::vector< TSettingsPageFormPtr > TOwnedPageVector;

    CString m_title;
    TPageVector m_pages;
    TOwnedPageVector m_ownedPages;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CSETTINGSFORM_H ? */
