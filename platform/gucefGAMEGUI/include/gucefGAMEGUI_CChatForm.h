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

#ifndef GUCEF_GAMEGUI_CCHATFORM_H
#define GUCEF_GAMEGUI_CCHATFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_GUI_CPOPUPMENU_H
#include "gucefGUI_CPopupMenu.h"
#define GUCEF_GUI_CPOPUPMENU_H
#endif /* GUCEF_GUI_CPOPUPMENU_H ? */

#ifndef GUCEF_MULTIPLAY_CICHATSERVICE_H
#include "gucefMULTIPLAY_CIChatService.h"
#define GUCEF_MULTIPLAY_CICHATSERVICE_H
#endif /* GUCEF_MULTIPLAY_CICHATSERVICE_H ? */

#ifndef GUCEF_MULTIPLAY_CCHATMESSAGE_H
#include "gucefMULTIPLAY_CChatMessage.h"
#define GUCEF_MULTIPLAY_CCHATMESSAGE_H
#endif /* GUCEF_MULTIPLAY_CCHATMESSAGE_H ? */

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
 *  Chat, as an embeddable form: it fills a placeholder widget (as CPathFieldForm does) with a tab control that has a
 *  tab per channel and per direct conversation, the messages of the tab shown, the members of the channel in a list,
 *  the line the player types in and a button to send it. It is bound to a CIChatService.
 *
 *  Messages that arrive show in the tab of their channel, or of the other user for a direct message; a tab that is not
 *  shown counts what it has not shown yet ("lobby (3)"). A channel the service reports the local user in gets a
 *  tab, and loses it when the user leaves. Double clicking a member, or choosing "Direct message" in the menu of
 *  the right mouse button, opens a conversation with that member. The Close button leaves the channel of the tab,
 *  or drops the conversation.
 *
 *  With a single channel (SetSingleChannel(), as the chat in a lobby) there is that channel only: no tab control,
 *  no direct messages, no Close button.
 */
class GUCEF_GAMEGUI_EXPORT_CPP CChatForm : public GUI::CFormEx
{
    public:

    static const CORE::CEvent MessageSentEvent;     /**< MULTIPLAY::CChatMessage: the player sent a line (channel or direct) */

    static void RegisterEvents( void );

    /** How many lines a tab keeps */
    static const UInt32 MaxLinesPerTab = 500;

    CChatForm( void );

    virtual ~CChatForm();

    /** Makes the chat in 'placeholder', which it fills */
    bool Build( GUI::CWidget* placeholder );

    /** The chat service the form talks to, not owned; its channels get tabs */
    void SetService( MULTIPLAY::CIChatService* service );

    MULTIPLAY::CIChatService* GetService( void ) const;

    /** Only this channel; joined when the service is connected and the local user is not in it yet. Empty: all channels. */
    void SetSingleChannel( const CORE::CString& channel );

    const CORE::CString& GetSingleChannel( void ) const;

    /** Joins the channel with the service and shows a tab for it; false when the service refuses or only a single channel is shown */
    bool JoinChannel( const CORE::CString& channel );

    /** Opens the tab of a conversation with a user and shows it; false for the local user, nobody, or a single channel form */
    bool OpenDirectConversation( const CORE::CString& user );

    /** Leaves the channel of a tab or drops the conversation; false when the tab is not there or the form shows a single channel */
    bool CloseTab( UInt32 index );

    /** Sends a line to the channel or user of the shown tab; false when there is no tab, or the service refuses */
    bool SendText( const CORE::CString& text );

    UInt32 GetTabCount( void ) const;

    /** The channel or user the tab is for */
    CORE::CString GetTabName( UInt32 index ) const;

    bool IsTabDirect( UInt32 index ) const;

    /** What the tab control shows for the tab, with the count of what was not read yet */
    CORE::CString GetTabTitle( UInt32 index ) const;

    Int32 FindTab( const CORE::CString& name, bool direct ) const;

    Int32 GetActiveTabIndex( void ) const;

    bool SetActiveTab( UInt32 index );

    UInt32 GetUnreadCount( UInt32 index ) const;

    /** The lines of a tab, oldest first */
    const CORE::CString::StringVector& GetTabLines( UInt32 index ) const;

    /** The lines the message box shows */
    CORE::CString::StringVector GetShownLines( void );

    /** The names in the members list */
    CORE::CString::StringVector GetShownMembers( void );

    CORE::CString GetShownTopic( void );

    /** The line as the box shows it: "[14:05] name: text" */
    static CORE::CString FormatMessage( const MULTIPLAY::CChatMessage& message );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct STab
    {
        CORE::CString name;
        bool direct;
        CORE::CString::StringVector lines;
        UInt32 unread;
    };
    typedef struct STab TTab;
    typedef GUCEF::vector< TTab > TTabVector;

    void BuildLayout( CORE::CDataNode& layout );
    bool Rebuild( void );
    Int32 AddTab( const CORE::CString& name, bool direct );
    void RemoveTab( UInt32 index );
    void AppendLine( UInt32 tabIndex, const CORE::CString& line );
    void ShowTabs( void );
    void ShowActiveTab( void );
    void ShowMembers( void );
    void ScrollMessagesToEnd( void );
    void OnChatMessage( const MULTIPLAY::CChatMessage& message );
    void OnChatPresence( const MULTIPLAY::CChatPresenceEventData& presence );
    void OpenMemberMenu( Int32 x, Int32 y, Int32 row );
    void SendInput( void );

    CChatForm( const CChatForm& src );
    CChatForm& operator=( const CChatForm& src );

    private:

    GUI::CWidget* m_placeholder;
    MULTIPLAY::CIChatService* m_service;
    CORE::CString m_singleChannel;
    TTabVector m_tabs;
    Int32 m_active;
    GUI::CPopupMenu* m_memberMenu;
    CORE::CString m_menuUser;
    bool m_updating;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_CCHATFORM_H ? */
