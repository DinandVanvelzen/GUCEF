/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CICHATSERVICE_H
#define GUCEF_MULTIPLAY_CICHATSERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_MULTIPLAY_CCHATMESSAGE_H
#include "gucefMULTIPLAY_CChatMessage.h"
#define GUCEF_MULTIPLAY_CCHATMESSAGE_H
#endif /* GUCEF_MULTIPLAY_CCHATMESSAGE_H ? */

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#include "gucefMULTIPLAY_CPlayerIdentity.h"
#define GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A chat channel the local user is in */
class GUCEF_MULTIPLAY_EXPORT_CPP CIChatChannel
{
    public:

    CIChatChannel( void );

    virtual ~CIChatChannel();

    /** The name as the application knows it, not as the backend spells it */
    virtual const CORE::CString& GetName( void ) const = 0;

    virtual const CORE::CString& GetTopic( void ) const = 0;

    virtual const CORE::CString::StringSet& GetMembers( void ) const = 0;
};

/*-------------------------------------------------------------------------*/

/** A plain chat channel, for backends that keep nothing more about a channel */
class GUCEF_MULTIPLAY_EXPORT_CPP CChatChannel : public CIChatChannel
{
    public:

    CORE::CString name;
    CORE::CString topic;
    CORE::CString::StringSet members;

    CChatChannel( void );

    explicit CChatChannel( const CORE::CString& name );

    virtual ~CChatChannel();

    virtual const CORE::CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetTopic( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString::StringSet& GetMembers( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** The data of ChatPresenceEvent */
class GUCEF_MULTIPLAY_EXPORT_CPP CChatPresenceEventData : public CORE::CICloneable
{
    public:

    enum EPresence
    {
        PRESENCE_JOINED = 0 ,
        PRESENCE_LEFT   = 1 ,
        PRESENCE_AWAY   = 2 ,
        PRESENCE_BACK   = 3
    };
    typedef enum EPresence TPresence;

    CORE::CString channel;      /**< empty when the change is not about one channel, a user that quits leaves them all */
    CORE::CString user;
    TPresence presence;
    CORE::CString reason;

    CChatPresenceEventData( void );

    CChatPresenceEventData( const CORE::CString& channel, const CORE::CString& user, TPresence presence, const CORE::CString& reason );

    virtual ~CChatPresenceEventData();

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** The data of ChatTopicEvent */
class GUCEF_MULTIPLAY_EXPORT_CPP CChatTopicEventData : public CORE::CICloneable
{
    public:

    CORE::CString channel;
    CORE::CString topic;

    CChatTopicEventData( void );

    CChatTopicEventData( const CORE::CString& channel, const CORE::CString& topic );

    virtual ~CChatTopicEventData();

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  A chat backend: channels and direct messages between users.
 *  Channel names are the names the application uses, a backend maps them to its own.
 *  Connect() completes its future when the backend accepted the user.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CIChatService : public CORE::CObservingNotifier ,
                                                 public CORE::CIConfigurable
{
    public:

    static const CORE::CEvent ChatMessageEvent;     /**< CChatMessage */
    static const CORE::CEvent ChatPresenceEvent;    /**< CChatPresenceEventData */
    static const CORE::CEvent ChatTopicEvent;       /**< CChatTopicEventData */
    static const CORE::CEvent ChatErrorEvent;       /**< TChatErrorEventData: a description of the problem */

    typedef CORE::TCloneableString  TChatErrorEventData;

    static void RegisterEvents( void );

    CIChatService( void );

    virtual ~CIChatService();

    virtual bool Connect( const CPlayerIdentity& identity, CORE::CFutureResult& future ) = 0;

    virtual void Disconnect( void ) = 0;

    virtual bool IsConnected( void ) const = 0;

    virtual bool JoinChannel( const CORE::CString& channel ) = 0;

    virtual bool LeaveChannel( const CORE::CString& channel ) = 0;

    /** Adds the names of the channels the local user is in */
    virtual void GetChannels( CORE::CString::StringVector& channels ) const = 0;

    /** The channel the local user is in, GUCEF_NULL when it is not */
    virtual const CIChatChannel* GetChannel( const CORE::CString& channel ) const = 0;

    virtual bool SendToChannel( const CORE::CString& channel, const CORE::CString& text ) = 0;

    virtual bool SendDirect( const CORE::CString& user, const CORE::CString& text ) = 0;

    /** The name others see the local user by */
    virtual const CORE::CString& GetLocalUserName( void ) const = 0;

    /** The name the service is registered with in the factory, for example "irc" */
    virtual const CORE::CString& GetServiceName( void ) const = 0;

    private:

    CIChatService( const CIChatService& src );
    CIChatService& operator=( const CIChatService& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTAbstractFactory< CORE::CString, CIChatService, MT::CMutex >    CChatServiceFactory;
typedef CChatServiceFactory::TProductPtr                                        CIChatServicePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CICHATSERVICE_H ? */
