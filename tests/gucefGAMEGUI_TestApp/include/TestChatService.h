/*
 *  gucefGAMEGUI_TestApp: Application for testing gucefGAMEGUI
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_GAMEGUI_TESTAPP_TESTCHATSERVICE_H
#define GUCEF_GAMEGUI_TESTAPP_TESTCHATSERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CICHATSERVICE_H
#include "gucefMULTIPLAY_CIChatService.h"
#define GUCEF_MULTIPLAY_CICHATSERVICE_H
#endif /* GUCEF_MULTIPLAY_CICHATSERVICE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An in-memory chat service for tests: every CTestChatService in the process is a user of one chat "server". Messages
 *  to a channel go to every user in it, the sender included (as services without echo-message report them), direct
 *  messages go to the user they are for and to the sender. Presence and topics are told to the users in the channel.
 */
class CTestChatService : public GUCEF::MULTIPLAY::CIChatService
{
    public:

    static const GUCEF::CORE::CString ServiceName;      /**< "test-chat" */

    CTestChatService( void );

    virtual ~CTestChatService();

    virtual bool Connect( const GUCEF::MULTIPLAY::CPlayerIdentity& identity, GUCEF::CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Disconnect( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool JoinChannel( const GUCEF::CORE::CString& channel ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LeaveChannel( const GUCEF::CORE::CString& channel ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetChannels( GUCEF::CORE::CString::StringVector& channels ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const GUCEF::MULTIPLAY::CIChatChannel* GetChannel( const GUCEF::CORE::CString& channel ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SendToChannel( const GUCEF::CORE::CString& channel, const GUCEF::CORE::CString& text ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SendDirect( const GUCEF::CORE::CString& user, const GUCEF::CORE::CString& text ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const GUCEF::CORE::CString& GetLocalUserName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const GUCEF::CORE::CString& GetServiceName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Sets the topic of a channel the user is in and tells everyone in it */
    bool SetTopic( const GUCEF::CORE::CString& channel, const GUCEF::CORE::CString& topic );

    /** Tells the observers of the service about a problem */
    void ReportError( const GUCEF::CORE::CString& description );

    virtual bool SaveConfig( GUCEF::CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const GUCEF::CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::map< GUCEF::CORE::CString, GUCEF::MULTIPLAY::CChatChannel > TChannelMap;

    void Deliver( const GUCEF::MULTIPLAY::CChatMessage& message );
    void NotePresence( const GUCEF::CORE::CString& channel, const GUCEF::CORE::CString& user, GUCEF::MULTIPLAY::CChatPresenceEventData::TPresence presence );
    bool IsInChannel( const GUCEF::CORE::CString& channel ) const;

    private:

    GUCEF::CORE::CString m_userName;
    bool m_connected;
    TChannelMap m_channels;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_TESTAPP_TESTCHATSERVICE_H ? */
