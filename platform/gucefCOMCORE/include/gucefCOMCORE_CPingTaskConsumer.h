/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CPINGTASKCONSUMER_H
#define GUCEF_COMCORE_CPINGTASKCONSUMER_H
 
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CIASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_COMCORE_EXPORT_CPP CPingTaskConsumer : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;
    static const CORE::CString ClassTypeName;
    
    static const CORE::CEvent PingStartedEvent;
    static const CORE::CEvent PingReponseEvent;
    static const CORE::CEvent PingTimeoutEvent;
    static const CORE::CEvent PingFailedEvent;
    static const CORE::CEvent PingStoppedEvent;
    
    static void RegisterEvents( void );
    
    class GUCEF_COMCORE_EXPORT_CPP CEchoReceivedEventData : public CORE::CICloneable
    {
        public: 
        
        CEchoReceivedEventData( const CHostAddress& host   ,
                                const UInt32 echoSize      ,
                                const UInt32 roundTripTime );

        CEchoReceivedEventData( const CEchoReceivedEventData& src );
        
        virtual ~CEchoReceivedEventData() GUCEF_VIRTUAL_OVERRIDE;
                
        virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
        
        const CHostAddress& GetHostAddress( void ) const;
        
        UInt32 GetEchoSize( void ) const;
        
        UInt32 GetRoundTripTime( void ) const;
        
        private:
        
        CHostAddress m_hostAddress;
        UInt32 m_echoSize;
        UInt32 m_roundTripTime;
    };    
    
    public:
    
    typedef GUCEF::vector< CString > TStringVector;
    
    class GUCEF_COMCORE_EXPORT_CPP CPingTaskData : public CORE::CICloneable
    {
        public:
        
        CPingTaskData( const TStringVector& remoteHosts    ,
                       const Int32 maxPings = 0            ,
                       const UInt32 bytesToSend = 32       ,
                       const UInt32 timeout = 1000         ,
                       const Int32 minimalPingDelta = 500  );
        
        CPingTaskData( const CPingTaskData& src );
        
        virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
        
        void SetRemoteHosts( const TStringVector& hostList );
        
        const TStringVector& GetRemoteHosts( void ) const;
        
        void SetMaxPings( const Int32 maxPings );
        
        Int32 GetMaxPings( void ) const;
        
        void SetBytesToSend( const UInt32 bytesToSend );
        
        UInt32 GetBytesToSend( void ) const;
        
        void SetTimeout( const UInt32 timeout );
        
        UInt32 GetTimeout( void ) const;
        
        void SetMinimalPingDelta( const UInt32 minimalPingDelta );
        
        UInt32 GetMinimalPingDelta( void ) const;
        
        private:
        
        TStringVector m_hostList;
        Int32 m_maxPings;
        UInt32 m_bytesToSend;
        UInt32 m_timeout;
        UInt32 m_minimalPingDelta;
    };
    
    CPingTaskConsumer( void );
    
    virtual ~CPingTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnding( CORE::CTaskPtr task ,
                               bool willBeForced   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task ,
                              bool wasForced      ) GUCEF_VIRTUAL_OVERRIDE;
    
    private:
    
    CPingTaskConsumer( const CPingTaskConsumer& src );
    CPingTaskConsumer& operator=( const CPingTaskConsumer& src );
    
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    static void IcmpCallback( void* vdata );
    
    #endif

    private:
    
    struct SPingEntry
    {
        // generic ping entry attributes
        UInt32 pingCount;
        UInt64 ticksAtLastPing;
        bool areWeWaitingForPingResult;
        
        // attributes used for ping callbacks
        CPingTaskConsumer* taskConsumer;
        const CHostAddress* host;
        void* replyBuffer;
        UInt32 echoSize;
    };
    typedef struct SPingEntry TPingEntry;
    typedef GUCEF::map< CHostAddress, TPingEntry > TPingCounters;

    void* m_platformData;
    bool m_notDone;
    CPingTaskData* m_taskData;
    TPingCounters m_pingCounters;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CPingTaskConsumer, MT::CMutex > CPingTaskConsumerPtr;
typedef CORE::CTFactory< CORE::CTaskConsumer, CPingTaskConsumer, MT::CMutex > TPingTaskConsumerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CPINGTASKCONSUMER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 29-12-2006 :
        - Dinand: Initial version

----------------------------------------------------------------------------*/
