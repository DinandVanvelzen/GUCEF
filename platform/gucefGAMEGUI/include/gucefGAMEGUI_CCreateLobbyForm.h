/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_GAMEGUI_CCREATELOBBYFORM_H
#define GUCEF_GAMEGUI_CCREATELOBBYFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H
#include "gucefMULTIPLAY_CLobbyCreateParams.h"
#define GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H
#endif /* GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H ? */

#ifndef GUCEF_GAMEGUI_CLOBBYSETTINGSPANELFORM_H
#include "gucefGAMEGUI_CLobbySettingsPanelForm.h"
#define GUCEF_GAMEGUI_CLOBBYSETTINGSPANELFORM_H
#endif /* GUCEF_GAMEGUI_CLOBBYSETTINGSPANELFORM_H ? */

#ifndef GUCEF_GAMEGUI_MACROS_H
#include "gucefGAMEGUI_macros.h"
#define GUCEF_GAMEGUI_MACROS_H
#endif /* GUCEF_GAMEGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GAMEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The data of CCreateLobbyForm::CreateRequestedEvent */
class GUCEF_GAMEGUI_EXPORT_CPP CCreateLobbyRequest : public CORE::CICloneable
{
    public:

    MULTIPLAY::CLobbyCreateParams params;       /**< what the lobby is made with */
    CORE::CString serviceName;                  /**< the lobby service to make it with, as the service factory knows it; empty when the form knows none */

    CCreateLobbyRequest( void );

    CCreateLobbyRequest( const CCreateLobbyRequest& src );

    virtual ~CCreateLobbyRequest();

    CCreateLobbyRequest& operator=( const CCreateLobbyRequest& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  The form to start a lobby: its name, a password, the most players, whether spectators may watch, whether it is on the
 *  LAN or on the internet, the lobby service to make it with, and the game settings of the schema (the same panel the
 *  lobby shows, CLobbySettingsPanelForm). Create checks the name and notifies CreateRequestedEvent with the
 *  CLobbyCreateParams (in a CCreateLobbyRequest); Cancel (or Escape) notifies CancelledEvent.
 *
 *  The game gives the form a template (SetTemplate()): the game id and version, the schema with its default values,
 *  the topology, the slots and the defaults of the fields. The params the form makes are the template with what the
 *  player filled in. The lobby services come from SetServiceNames(), or are those of the lobby service factory of
 *  gucefMULTIPLAY.
 *  The layout is built in code and made for BuildDarkTheme(). The window is nine tenths of the screen (at least 560 x 460, at most
 *  2000 x 1000) in the middle of it, which the player can move and fold up. On a window of 700 pixels or more it has two columns: the
 *  general fields (widgets Name, Password, MaxPlayers, Spectators, Where, Service) on the left and the game settings on the right, in
 *  the scroll panel SettingsHolder that fills the column. A window of 1500 or more gives the general fields a column of 480 pixels
 *  and the settings the rest, with the groups of the settings side by side (up to three columns). A narrower window has one column
 *  (the scroll panel Body) with the
 *  general fields above the game settings, scrolling as a whole. The error and the buttons are in a strip along the bottom edge,
 *  ButtonRow, that no other widget uses. The layout follows the size of the screen while the form is open; what the player filled in stays.
 */
class GUCEF_GAMEGUI_EXPORT_CPP CCreateLobbyForm : public GUI::CFormEx
{
    public:

    static const CORE::CEvent CreateRequestedEvent;     /**< CCreateLobbyRequest */
    static const CORE::CEvent CancelledEvent;           /**< no data */

    static void RegisterEvents( void );

    /** The most players a lobby can be made for */
    static const UInt32 MaxPlayersLimit = 64;

    CCreateLobbyForm( void );

    CCreateLobbyForm( GUI::CIGUIContext* context );

    virtual ~CCreateLobbyForm();

    /** Makes the window, hidden */
    bool Build( void );

    /** Shows the window, modal, with the focus in the name */
    bool Open( void );

    void Close( void );

    bool IsOpen( void ) const;

    /** What the lobby is made from and the defaults of the fields */
    void SetTemplate( const MULTIPLAY::CLobbyCreateParams& params );

    const MULTIPLAY::CLobbyCreateParams& GetTemplate( void ) const;

    /** The lobby services to choose from; none: those of the lobby service factory */
    void SetServiceNames( const CORE::CString::StringVector& serviceNames );

    const CORE::CString::StringVector& GetServiceNames( void ) const;

    /** The lobby service chosen */
    CORE::CString GetServiceName( void );

    bool SelectService( const CORE::CString& serviceName );

    /** The panel with the settings of the schema, NULL before the form is built */
    CLobbySettingsPanelForm* GetSettingsPanel( void ) const;

    /**
     *  The params as the form has them now.
     *
     *  @return false when the name is missing, which is shown as the error of the form
     */
    bool GetCreateParams( MULTIPLAY::CLobbyCreateParams& params );

    /** Checks the params and notifies CreateRequestedEvent; false when they are not good */
    bool Create( void );

    /** Closes and notifies CancelledEvent */
    void Cancel( void );

    CORE::CString GetErrorText( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPreLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CCreateLobbyForm > TEventCallback;

    void BuildLayout( CORE::CDataNode& layout );
    void AddGeneralFields( CORE::CDataNode& parent );
    void AddFooter( CORE::CDataNode& window );
    enum TTier { TIER_NARROW, TIER_NORMAL, TIER_WIDE };
    TTier GetTierForScreen( void );
    bool LoadForCurrentSize( void );
    void UpdateLayoutMode( void );
    void ShowTemplate( void );
    void ShowServiceNames( void );

    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCreateActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCancelActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFieldTextEntered( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNavigation( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CCreateLobbyForm( const CCreateLobbyForm& src );
    CCreateLobbyForm& operator=( const CCreateLobbyForm& src );

    private:

    MULTIPLAY::CLobbyCreateParams m_template;
    CORE::CString::StringVector m_serviceNames;
    CLobbySettingsPanelForm* m_settingsPanel;
    TTier m_tier;                   /**< the layout: one column that scrolls as a whole (narrow), two columns, or two with columns of groups (wide) */
    bool m_open;
    bool m_subscribedToFrames;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_CCREATELOBBYFORM_H ? */
