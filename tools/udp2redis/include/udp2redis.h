/*
 *  Udp2Redis: service which pushes UDP packets into kafka topics
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <deque>

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CIASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H ? */

#ifndef GUCEF_WEB_CDEFAULTHTTPSERVERROUTER_H
#include "gucefWEB_CDefaultHTTPServerRouter.h"
#define GUCEF_WEB_CDEFAULTHTTPSERVERROUTER_H
#endif /* GUCEF_WEB_CDEFAULTHTTPSERVERROUTER_H ? */

#ifndef GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H
#include "gucefWEB_CCodecBasedHTTPServerResource.h"
#define GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H ? */

#include "hiredis.h"
#include "async.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

class Udp2RedisChannel : public CORE::CTaskConsumer
{
    public:

    typedef CORE::CTEventHandlerFunctor< Udp2RedisChannel > TEventCallback;

    Udp2RedisChannel();
    Udp2RedisChannel( const Udp2RedisChannel& src );
    virtual ~Udp2RedisChannel() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task ,
                              bool wasForced      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    class ChannelSettings
    {
        public:

        typedef std::vector< COMCORE::CHostAddress > HostAddressVector;

        ChannelSettings( void );
        ChannelSettings( const ChannelSettings& src );
        ChannelSettings& operator=( const ChannelSettings& src );

        COMCORE::CHostAddress redisAddress;
        CORE::CString channelStreamName;
        COMCORE::CHostAddress udpInterface;
        HostAddressVector udpMulticastToJoin;
    };

    bool LoadConfig( const ChannelSettings& channelSettings );

    class ChannelMetrics
    {
        public:

        ChannelMetrics( void );

        CORE::UInt32 udpBytesReceived;
        CORE::UInt32 udpMessagesReceived;
        CORE::UInt32 redisMessagesTransmitted;
        CORE::UInt32 redisTransmitOverflowQueueSize;
        CORE::UInt32 redisErrorReplies;
    };

    const ChannelMetrics& GetMetrics( void ) const;

    private:

    typedef COMCORE::CUDPSocket::TPacketEntryVector TPacketEntryVector;
    typedef COMCORE::CUDPSocket::TPacketEntry TPacketEntry;

    void
    OnUDPSocketError( CORE::CNotifier* notifier   ,
                      const CORE::CEvent& eventID ,
                      CORE::CICloneable* evenData );

    void
    OnUDPSocketClosed( CORE::CNotifier* notifier   ,
                       const CORE::CEvent& eventID ,
                       CORE::CICloneable* evenData );

    void
    OnUDPSocketOpened( CORE::CNotifier* notifier   ,
                       const CORE::CEvent& eventID ,
                       CORE::CICloneable* evenData );

    void
    OnUDPPacketsRecieved( CORE::CNotifier* notifier   ,
                          const CORE::CEvent& eventID ,
                          CORE::CICloneable* evenData );

    void
    OnRedisReconnectTimer( CORE::CNotifier* notifier   ,
                           const CORE::CEvent& eventID ,
                           CORE::CICloneable* evenData );

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    CORE::UInt32 GetRedisErrorRepliesCounter( bool resetCounter );

    private:

    void RegisterEventHandlers( void );

    int RedisSend( const TPacketEntryVector& udpPackets ,
                   CORE::UInt32 packetCount             );

    bool SendQueuedPackagesIfAny( void );

    bool RedisConnect( void );

    bool AddToOverflowQueue( const TPacketEntryVector& udpPackets, CORE::UInt32 packetCount );

    static void
    OnRedisASyncVoidReply( redisAsyncContext* context ,
                           void *reply                ,
                           void *privdata             );

    void
    OnRedisASyncReply( redisAsyncContext* context ,
                       redisReply* reply          );

    static void
    OnRedisASyncConnect( const struct redisAsyncContext* context ,
                         int status                              );

    static void
    OnRedisASyncDisconnect( const struct redisAsyncContext* context ,
                            int status                              );

    static void
    OnRedisAddReadEvent( void* privData );

    static void
    OnRedisDelReadEvent( void* privData );

    static void
    OnRedisAddWriteEvent( void* privData );

    static void
    OnRedisDelWriteEvent( void* privData );

    static void
    OnRedisCleanupEvent( void* privData );

    static void
    OnRedisScheduleTimerEvent( void* privData, struct timeval tv );

    private:

    typedef std::deque< CORE::CDynamicBuffer > TDynamicBufferQueue;
    typedef std::vector< redisAsyncContext* > redisAsyncContextVector;

    ChannelSettings m_channelSettings;
    CORE::CString m_redisStreamSendCmd;
    CORE::CTimer* m_redisReconnectTimer;
    redisAsyncContext* m_redisContext;
    GUCEF::COMCORE::CUDPSocket* m_udpSocket;
    TDynamicBufferQueue m_redisMsgQueueOverflowQueue;
    redisOptions m_redisOptions;
    bool m_redisReadFlag;
    bool m_redisWriteFlag;
    bool m_redisTimeoutFlag;
    CORE::CTimer* m_metricsTimer;
    ChannelMetrics m_metrics;
    CORE::UInt32 m_redisErrorReplies;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< Udp2RedisChannel, MT::CMutex > Udp2RedisChannelPtr;

/*-------------------------------------------------------------------------*/

class Udp2Redis;

class RestApiUdp2RedisInfoResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiUdp2RedisInfoResource( Udp2Redis* app );

    virtual ~RestApiUdp2RedisInfoResource();

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CORE::CString& params         ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    Udp2Redis* m_app;
};

/*-------------------------------------------------------------------------*/

class RestApiUdp2RedisConfigResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiUdp2RedisConfigResource( Udp2Redis* app, bool appConfig );

    virtual ~RestApiUdp2RedisConfigResource();

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CORE::CString& params         ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    Udp2Redis* m_app;
    bool m_appConfig;
};

/*-------------------------------------------------------------------------*/

class Udp2Redis : public CORE::CObserver
{
    public:

    Udp2Redis( void );
    virtual ~Udp2Redis();

    bool Start( void );

    bool LoadConfig( const CORE::CValueList& appConfig   ,
                     const CORE::CDataNode& globalConfig );

    const CORE::CValueList& GetAppConfig( void ) const;

    const CORE::CDataNode& GetGlobalConfig( void ) const;

    private:

    typedef CORE::CTEventHandlerFunctor< Udp2Redis > TEventCallback;

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    private:

    typedef std::map< CORE::Int32, Udp2RedisChannel::ChannelSettings > ChannelSettingsMap;
    typedef std::vector< Udp2RedisChannelPtr > Udp2RedisChannelVector;

    CORE::UInt16 m_udpStartPort;
    CORE::UInt16 m_channelCount;
    CORE::Int32 m_redisStreamStartChannelID;
    CORE::CString m_redisStreamName;
    CORE::CString m_redisHost;
    CORE::UInt16 m_redisPort;
    Udp2RedisChannelVector m_channels;
    ChannelSettingsMap m_channelSettings;
    WEB::CHTTPServer m_httpServer;
    WEB::CDefaultHTTPServerRouter m_httpRouter;
    CORE::CValueList m_appConfig;
    CORE::CDataNode m_globalConfig;

    CORE::CTimer m_metricsTimer;
    bool m_transmitMetrics;
};
