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

#ifndef GUCEF_GAMEGUI_CCONNECTINGFORM_H
#define GUCEF_GAMEGUI_CCONNECTINGFORM_H

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

#ifndef GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H
#include "gucefMULTIPLAY_CLaunchDescription.h"
#define GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H
#endif /* GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H ? */

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

/**
 *  The hand-over from the lobby to the game: a modal window that shows which endpoint of the launch description is
 *  tried, that the relay of the lobby server is the fallback when none of the endpoints can be reached, how far the
 *  transfer of the game state is, and a Cancel button.
 *
 *  The form does not connect by itself: the game connects with the launch description and tells the form what it
 *  does. Begin() takes the endpoints (those of the host first, then the relay) and shows the first one;
 *  TryNextEndpoint() moves on when the game could not reach the one shown; SetProgress() shows the transfer;
 *  ReportConnected() ends the hand-over, ReportFailed() shows why it did not work. Cancel (or Escape) notifies
 *  CancelRequestedEvent.
 *  The layout is built in code and made for BuildDarkTheme().
 */
class GUCEF_GAMEGUI_EXPORT_CPP CConnectingForm : public GUI::CFormEx
{
    public:

    static const CORE::CEvent CancelRequestedEvent;     /**< no data */
    static const CORE::CEvent ConnectedEvent;           /**< no data: ReportConnected() was called */
    static const CORE::CEvent FailedEvent;              /**< TFailedEventData: why it did not work */

    typedef CORE::TCloneableString TFailedEventData;

    static void RegisterEvents( void );

    CConnectingForm( void );

    CConnectingForm( GUI::CIGUIContext* context );

    virtual ~CConnectingForm();

    /** Makes the window, hidden */
    bool Build( void );

    /** Takes the endpoints of the launch description, shows the first one and shows the window, modal */
    bool Begin( const MULTIPLAY::CLaunchDescription& launch );

    /** Takes the endpoints directly: those to try in order, then the relay, which can be empty */
    bool Begin( const CORE::CString::StringVector& endpoints, const CORE::CString& relay );

    /**
     *  Moves on to the next endpoint, the relay after the last one.
     *
     *  @return false when there is nothing left to try, which is reported with FailedEvent
     */
    bool TryNextEndpoint( void );

    /** The endpoint that is tried now, empty when there is none */
    const CORE::CString& GetCurrentEndpoint( void ) const;

    /** Whether the relay is what is tried now */
    bool IsUsingRelay( void ) const;

    /** How many endpoints there are to try, the relay included */
    UInt32 GetEndpointCount( void ) const;

    /** Shows how far the transfer is, 0 .. 1, with a text such as "12 of 340 KB" */
    void SetProgress( Float32 fraction, const CORE::CString& caption );

    Float32 GetProgress( void ) const;

    /** Replaces the status line */
    void SetStatusText( const CORE::CString& text );

    /** The hand-over worked: the window closes and ConnectedEvent is notified */
    void ReportConnected( void );

    /** The hand-over did not work: the reason is shown and FailedEvent is notified; the window stays for the player to read it */
    void ReportFailed( const CORE::CString& reason );

    /** Closes the window and notifies CancelRequestedEvent */
    void Cancel( void );

    void Close( void );

    bool IsOpen( void ) const;

    bool HasFailed( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CConnectingForm > TEventCallback;

    void BuildLayout( CORE::CDataNode& layout );
    void ShowCurrentEndpoint( void );
    void SetLabelText( const char* widgetName, const CORE::CString& text );

    void OnCancelActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNavigation( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CConnectingForm( const CConnectingForm& src );
    CConnectingForm& operator=( const CConnectingForm& src );

    private:

    CORE::CString::StringVector m_endpoints;    /**< in the order they are tried, the relay last */
    bool m_hasRelay;
    UInt32 m_current;                           /**< into m_endpoints, m_endpoints.size() when there is none */
    Float32 m_progress;
    bool m_failed;
    bool m_open;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_CCONNECTINGFORM_H ? */
