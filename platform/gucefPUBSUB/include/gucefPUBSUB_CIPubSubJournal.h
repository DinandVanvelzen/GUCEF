/*
 *  gucefPUBSUB: GUCEF module providing pub-sub communication facilities
 *
 *  Copyright (C) 1998 - 2022.  Dinand Vanvelzen
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

#ifndef GUCEF_PUBSUB_CIPUBSUBJOURNAL_H
#define GUCEF_PUBSUB_CIPUBSUBJOURNAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_PUBSUB_MACROS_H
#include "gucefPUBSUB_macros.h"    
#define GUCEF_PUBSUB_MACROS_H
#endif /* GUCEF_PUBSUB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Class providing an interface to implement a pub-sub messaging journal to trace what happened
 *  This differs from a regular log in that the more specific nature allows performance optimizations to be applied
 *  and as such allows per message tracing without the overhead of a regular log
 */
class GUCEF_PUBSUB_EXPORT_CPP CIPubSubJournal
{
    public:

    enum EPubSubActionType : UInt8
    {
        PUBSUBACTION_UNDEFINED = 0,

        PUBSUBACTION_CLIENT_CREATED,
        PUBSUBACTION_CLIENT_DESTROYED,
        PUBSUBACTION_TOPIC_CREATED,
        PUBSUBACTION_TOPIC_DESTROYED,
        PUBSUBACTION_TOPIC_SUBSCRIBED,
        PUBSUBACTION_TOPIC_UNSUBSCRIBED,
        PUBSUBACTION_MESSAGE_RECEIVED,
        PUBSUBACTION_MESSAGE_SENT,
        PUBSUBACTION_MESSAGE_SEND_FAILURE,
        PUBSUBACTION_MESSAGE_ROUTED,
        PUBSUBACTION_MESSAGE_INFLIGHT_TIMEOUT,
        PUBSUBACTION_MESSAGE_RECEIVED_ACKNOWLEDGED,
        PUBSUBACTION_MESSAGE_SENT_ACKNOWLEDGED,
        PUBSUBACTION_MESSAGE_CACHED,
        PUBSUBACTION_MESSAGE_DELETED_FROM_CACHE,
        PUBSUBACTION_MESSAGE_CACHE_MISS
    };
    typedef enum EPubSubActionType TPubSubActionType;

    virtual ~CIPubSubJournal() {}

    virtual bool ReadJournalEntry( UInt64& eventTimestamp    ,
                                   TPubSubActionType& action ,
                                   UInt64& msgActionId       ) = 0;

    /**
     *
     */
    virtual bool AddJournalEntry( UInt64 eventTimestamp    ,
                                  TPubSubActionType action ,
                                  UInt64 msgActionId       ) = 0;


    bool AddClientCreatedJournalEntry( void )
    {GUCEF_TRACE;
    
        return AddJournalEntry( CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() ,
                                PUBSUBACTION_CLIENT_CREATED                                          , 
                                0                                                                    ); 
    }

    bool AddClientDestroyedJournalEntry( void )
    {GUCEF_TRACE;

        return AddJournalEntry( CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() ,
                                PUBSUBACTION_CLIENT_DESTROYED                                        ,
                                0                                                                    );
    }

    bool AddTopicCreatedJournalEntry( void )
    {GUCEF_TRACE;

        return AddJournalEntry( CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() ,
                                PUBSUBACTION_TOPIC_CREATED                                           ,
                                0                                                                    );
    }

    bool AddTopicDestroyedJournalEntry( void )
    {GUCEF_TRACE;

        return AddJournalEntry( CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() ,
                                PUBSUBACTION_TOPIC_DESTROYED                                         ,
                                0                                                                    );
    }

    bool AddTopicSubscribedJournalEntry( void )
    {GUCEF_TRACE;

        return AddJournalEntry( CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() ,
                                PUBSUBACTION_TOPIC_SUBSCRIBED                                        ,
                                0                                                                    );
    }

    bool AddTopicUnsubscribedJournalEntry( void )
    {GUCEF_TRACE;

        return AddJournalEntry( CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() ,
                                PUBSUBACTION_TOPIC_UNSUBSCRIBED                                      ,
                                0                                                                    );
    }

    bool AddMessageReceivedJournalEntry( UInt64 msgActionId )
    {GUCEF_TRACE;

        return AddJournalEntry( CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() ,
                                PUBSUBACTION_MESSAGE_RECEIVED                                        ,
                                msgActionId                                                          );
    }

    bool AddMessageSentJournalEntry( UInt64 msgActionId )
    {GUCEF_TRACE;

        return AddJournalEntry( CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() ,
                                PUBSUBACTION_MESSAGE_SENT                                            ,
                                msgActionId                                                          );
    }

    bool AddMessageSendFailureJournalEntry( UInt64 msgActionId )
    {GUCEF_TRACE;

        return AddJournalEntry( CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() ,
                                PUBSUBACTION_MESSAGE_SEND_FAILURE                                    ,
                                msgActionId                                                          );
    }

    bool AddMessageCachedJournalEntry( UInt64 msgActionId )
    {GUCEF_TRACE;

        return AddJournalEntry( CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() ,
                                PUBSUBACTION_MESSAGE_CACHED                                          ,
                                msgActionId                                                          );
    }

    bool AddMessageDeletedFromCacheJournalEntry( UInt64 msgActionId )
    {GUCEF_TRACE;

        return AddJournalEntry( CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() ,
                                PUBSUBACTION_MESSAGE_DELETED_FROM_CACHE                              ,
                                msgActionId                                                          );
    }

    bool AddMessageCacheMissJournalEntry( UInt64 msgActionId )
    {GUCEF_TRACE;

        return AddJournalEntry( CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() ,
                                PUBSUBACTION_MESSAGE_CACHE_MISS                                      ,
                                msgActionId                                                          );
    }

};

/*-------------------------------------------------------------------------*/

typedef CORE::CTBasicSharedPtr< CIPubSubJournal, MT::CMutex > CIPubSubJournalBasicPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PUBSUB_CIPUBSUBJOURNAL_H ? */
