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

#ifndef GUCEF_GAMEGUI_CDIRECTCONNECTFORM_H
#define GUCEF_GAMEGUI_CDIRECTCONNECTFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

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

/** The data of CDirectConnectForm::ConnectRequestedEvent */
class GUCEF_GAMEGUI_EXPORT_CPP CDirectConnectRequest : public CORE::CICloneable
{
    public:

    CORE::CString address;      /**< as the player typed it, without white space around it */
    CORE::CString endpoint;     /**< the same as an endpoint URI such as udp://host:port, see CDirectConnectForm::ParseAddress() */
    CORE::CString password;

    CDirectConnectRequest( void );

    CDirectConnectRequest( const CORE::CString& address  ,
                           const CORE::CString& endpoint ,
                           const CORE::CString& password );

    virtual ~CDirectConnectRequest();

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  A small modal window to join a game by its address: the address (host:port or an endpoint URI such as
 *  tcp://host:port) and the password of the game. Connect checks the address; a good one is notified with
 *  ConnectRequestedEvent (CDirectConnectRequest), a bad one shows what is wrong and nothing is notified.
 *  Enter in either field connects, Escape cancels.
 *  The layout is built in code and made for BuildDarkTheme().
 */
class GUCEF_GAMEGUI_EXPORT_CPP CDirectConnectForm : public GUI::CFormEx
{
    public:

    static const CORE::CEvent ConnectRequestedEvent;    /**< CDirectConnectRequest */
    static const CORE::CEvent CancelledEvent;           /**< no data */

    static void RegisterEvents( void );

    CDirectConnectForm( void );

    CDirectConnectForm( GUI::CIGUIContext* context );

    virtual ~CDirectConnectForm();

    /** Makes the window, hidden */
    bool Build( void );

    /** Shows the window, modal, with the focus in the address */
    bool Open( void );

    /** Hides the window */
    void Close( void );

    bool IsOpen( void ) const;

    void SetAddress( const CORE::CString& address );
    CORE::CString GetAddress( void );

    void SetPassword( const CORE::CString& password );
    CORE::CString GetPassword( void );

    /** The scheme of an address given as host:port, "udp" by default */
    void SetDefaultScheme( const CORE::CString& scheme );
    const CORE::CString& GetDefaultScheme( void ) const;

    /**
     *  Checks the address and notifies ConnectRequestedEvent when it is good.
     *
     *  @return whether the request was made
     */
    bool Connect( void );

    /** Closes and notifies CancelledEvent */
    void Cancel( void );

    /** The text of the problem shown after a Connect() that did not go, empty when there is none */
    CORE::CString GetErrorText( void );

    /**
     *  Takes an address apart: host:port, or scheme://host:port with an optional path.
     *
     *  @param endpoint the address as an endpoint URI, with the default scheme when the address has none
     *  @return false when there is no host and port in the address
     */
    static bool ParseAddress( const CORE::CString& address       ,
                              const CORE::CString& defaultScheme ,
                              CORE::CString& endpoint            );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    void BuildLayout( CORE::CDataNode& layout );

    CDirectConnectForm( const CDirectConnectForm& src );
    CDirectConnectForm& operator=( const CDirectConnectForm& src );

    private:

    CORE::CString m_defaultScheme;
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

#endif /* GUCEF_GAMEGUI_CDIRECTCONNECTFORM_H ? */
