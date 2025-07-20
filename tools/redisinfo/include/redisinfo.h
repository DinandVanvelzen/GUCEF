/*
 *  redisinfo: Service for obtaining redis info from a cluster
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <deque>

#ifndef GUCEF_MT_CTMAILBOX_H
#include "gucefMT_CTMailBox.h"
#define GUCEF_MT_CTMAILBOX_H
#endif /* GUCEF_MT_CTMAILBOX_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CIASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "CIConfigurable.h"
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

#ifndef GUCEF_WEB_CTCONFIGURABLEMAPHTTPSERVERRESOURCE_H
#include "gucefWEB_CTConfigurableMapHttpServerResource.h"
#define GUCEF_WEB_CTCONFIGURABLEMAPHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CTCONFIGURABLEMAPHTTPSERVERRESOURCE_H ? */

#ifndef GUCEF_WEB_CTASKMANAGERSERVERRESOURCE_H
#include "gucefWEB_CTaskManagerServerResource.h"
#define GUCEF_WEB_CTASKMANAGERSERVERRESOURCE_H
#endif /* GUCEF_WEB_CTASKMANAGERSERVERRESOURCE_H ? */

#ifndef GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H
#include "gucefWEB_CDummyHTTPServerResource.h"
#define GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H ? */

#ifndef GUCEF_WEB_CTREADABLEMAPINDEXHTTPSERVERRESOURCE_H
#include "gucefWEB_CTReadableMapIndexHttpServerResource.h"
#define GUCEF_WEB_CTREADABLEMAPINDEXHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CTREADABLEMAPINDEXHTTPSERVERRESOURCE_H ? */

#ifndef GUCEF_WEB_CTDATANODESERIALIZABLEMAPHTTPSERVERRESOURCE_H
#include "gucefWEB_CTDataNodeSerializableMapHttpServerResource.h"
#define GUCEF_WEB_CTDATANODESERIALIZABLEMAPHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CTDATANODESERIALIZABLEMAPHTTPSERVERRESOURCE_H ? */

#ifndef REDISINFO_CREDISCLUSTERKEYCACHE_H
#include "redisinfo_CRedisClusterKeyCache.h"
#define REDISINFO_CREDISCLUSTERKEYCACHE_H
#endif /* REDISINFO_CREDISCLUSTERKEYCACHE_H ? */

#include "hiredis.h"
#include "async.h"

#include "redis++.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace REDISINFO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class Settings : public CORE::CIConfigurable
{
    public:

    Settings( void );
    Settings( const Settings& src );
    Settings& operator=( const Settings& src );

    CORE::CString clusterName;
    COMCORE::CHostAddress redisAddress;
    bool collectMetrics;
    CORE::CString metricPrefix;
    bool gatherInfoReplication;
    bool gatherInfoPersistence;
    bool gatherInfoStats;
    bool gatherInfoCommandStats;
    bool gatherInfoMemory;
    bool gatherStreamInfo;
    CORE::CString::StringSet streamsToGatherInfoFrom;
    CORE::Int32 streamIndexingInterval;
    CORE::UInt32 redisScanCountSize;
    bool persistRedisKeyCacheSnapshot;
    CORE::CString redisKeyCacheSnapshotPath;
    CORE::CString redisKeyCacheSnapshotCodec;
    bool gatherInfoClients;
    bool gatherInfoCpu;
    bool gatherInfoKeyspace;
    bool gatherClusterInfo;
    bool gatherErrorReplyCount;

    bool redisConnectionOptionKeepAlive;
    UInt64 redisConnectionOptionConnectTimeoutInMs;
    UInt64 redisConnectionOptionSocketTimeoutInMs;

    bool retainRedisInfo;

    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& tree ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

typedef std::map< CORE::CString, Settings > TStringToSettingsMap;

/*-------------------------------------------------------------------------*/

class RedisNode : public CORE::CIDataNodeSerializable
{
    public:

    static const CORE::CString ClassTypeName;

    typedef std::vector< RedisNode > RedisNodeVector;

    COMCORE::CHostAddress host;
    CORE::CString nodeId;
    CORE::Int32 startSlot;
    CORE::Int32 endSlot;
    RedisNodeVector replicas;

    RedisNode* FindReplica( const CORE::CString& replicaNodeId ,
                            const CORE::CString& parentNodeId  ,
                            bool createIfNotFound = false      );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ,
                            CORE::CDataNode** targetNode                        ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     *
     *  @param domRootNode Node that acts as root of the DOM data tree from which to deserialize
     *  @return whether deserializing the object data from the given DOM was successfull.
     */
    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    RedisNode& operator=( const RedisNode& other );

    RedisNode( void );

    RedisNode( const RedisNode& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

typedef std::map< CORE::UInt32, RedisNode > RedisNodeMap;

/*-------------------------------------------------------------------------*/

class RedisNodeWithPipe : public RedisNode
{
    public:

    sw::redis::Pipeline* redisPipe;
    CORE::UInt32 redisErrorReplies;
    CORE::UInt32 redisTimeoutReplies;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    RedisNodeWithPipe& operator=( const RedisNode& other );

    RedisNodeWithPipe( void );

    CORE::UInt32 GetRedisErrorRepliesCounter( bool resetCounter );

    CORE::UInt32 GetRedisTimeoutRepliesCounter( bool resetCounter );
};

typedef std::map< CORE::UInt32, RedisNodeWithPipe > RedisNodeWithPipeMap;

/*-------------------------------------------------------------------------*/

class RedisInfoService : public CORE::CTaskConsumer
{
    public:

    typedef CORE::CTEventHandlerFunctor< RedisInfoService >     TEventCallback;
    typedef CORE::CString::StringSet                            TStringSet;
    typedef std::map< CORE::UInt32, TStringSet >                TUInt32ToStringSetMap;

    static const CORE::CString HashSlotFileCodec;

    RedisInfoService();
    virtual ~RedisInfoService();

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnding( CORE::CTaskPtr task ,
                               bool willBeForced   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task ,
                              bool wasForced      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    bool LoadConfig( const Settings& settings );

    const Settings& GetSettings( void ) const;

    bool ConnectHttpRouting( WEB::CIHTTPServerRouter& webRouter );
    bool DisconnectHttpRouting( WEB::CIHTTPServerRouter& webRouter );

    private:

    bool RedisConnect( bool reset );

    bool RedisDisconnect( void );

    void RedisReconnect( void );

    void RegisterEventHandlers( void );

    bool RefreshRedisNodePipes( void );

    bool GetRedisStreamInfo( struct redisReply* replyNode           ,
                             CORE::CValueList& info                 ,
                             const CORE::CString& optionalKeyPrefix ,
                             bool statLikeValuesOnly                ,
                             bool redisIdsAsFloat                   );

    bool SerializeRedisClusterNodeMap( const RedisNodeMap& nodeMap, CORE::CDataNode& doc ) const;

    bool DeserializeKeysForHashSlots( TUInt32ToStringSetMap& hashMap, const CORE::CDataNode& doc ) const;

    bool SaveDocTo( const CORE::CDataNode& doc     ,
                    const CORE::CString& codecName ,
                    const CORE::CString& vfsPath   ) const;

    bool LoadDocFrom( CORE::CDataNode& doc           ,
                      const CORE::CString& codecName ,
                      const CORE::CString& vfsPath   ) const;

    bool LoadDocFrom( CORE::CDataNode& doc         ,
                      const CORE::CString& vfsPath ) const;

    CORE::UInt32 CalculateRedisHashSlot( const CORE::CString& keyStr ) const;

    bool CalculateKeysForAllHashSlots( TUInt32ToStringSetMap& hashMap ,
                                       CORE::UInt32 minKeysPerSlot    ,
                                       CORE::UInt32 maxKeysPerSlot    ) const;

    bool SerializeKeysForHashSlots( const TUInt32ToStringSetMap& hashMap, CORE::CDataNode& doc ) const;

    bool GetRedisClusterNodeMap( RedisNodeMap& nodeMap );

    bool GetRedisClusterSlots( RedisNodeMap& nodeMap );

    bool LoadHashSlotMap( void );

    bool ProvideHashSlotMapDoc( void );

    bool ProvideRedisNodesDoc( void );

    bool GetRedisKeysForNode( RedisNodeWithPipe& node           ,
                              CORE::CString::StringVector& keys ,
                              const CORE::CString& keyType      );

    bool GetRedisStreamInfo( const CORE::CString& streamName        ,
                             CORE::CValueList& info                 ,
                             const CORE::CString& optionalKeyPrefix ,
                             bool statLikeValuesOnly                ,
                             bool redisIdsAsFloat                   );

    bool GetRedisStreamInfoForAllStreams( CORE::CValueList& info  ,
                                          bool statLikeValuesOnly ,
                                          bool redisIdsAsFloat    );

    bool GetRedisInfoReplication( CORE::CValueList& kv );

    bool GetRedisInfoPersistence( CORE::CValueList& kv );

    bool GetRedisInfoStats( CORE::CValueList& kv );

    bool GetRedisInfoCommandStats( CORE::CValueList& kv );

    bool GetRedisInfoMemory( CORE::CValueList& kv );

    bool GetRedisInfoClients( CORE::CValueList& kv );

    bool GetRedisInfoCpu( CORE::CValueList& kv );

    bool GetRedisInfoKeyspace( CORE::CValueList& kv );

    bool GetRedisClusterInfo( CORE::CValueList& kv );

    bool GetRedisInfo( const CORE::CString& cmd             ,
                       const CORE::CString& type            ,
                       CORE::CValueList& kv                 ,
                       RedisNodeWithPipe* node = GUCEF_NULL );

    bool GetRedisInfo( struct redisReply* replyNode   ,
                       const CORE::CString& type      ,
                       CORE::CValueList& kv           ,
                       const CORE::CString& keyPrefix );

    RedisNodeWithPipe* FindNodeInfo( const CORE::CString& hashable );

    RedisNodeWithPipe* FindNodeInfo( CORE::UInt32 hashSlot );

    void SendKeyValueStats( const CORE::CValueList& kv        ,
                            const CORE::CString& metricPrefix );

    void SendErrorReplyStats( const CORE::CString& metricPrefix );
    
    bool IsStreamIndexingNeeded( void ) const;

    CORE::UInt32 GetRedisClusterErrorRepliesCounter( bool resetCounter );

    CORE::UInt32 GetRedisClusterTimeoutRepliesCounter( bool resetCounter );

    CORE::UInt8 GetTypeOfRedisInfoValue( const CORE::CString& valueName ) const;

    void PopulateDefaultRedisInfoValueTypes( void );

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void
    OnRedisReconnectTimerCycle( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData );

    void
    OnVfsInitCompleted( CORE::CNotifier* notifier    ,
                        const CORE::CEvent& eventId  ,
                        CORE::CICloneable* eventData );

    void
    OnRedisKeyCacheUpdate( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData );
        
    private:

    RedisInfoService( const RedisInfoService& src ); // not implemented

    typedef std::vector< std::pair< sw::redis::StringView, sw::redis::StringView > >    TRedisArgs;
    typedef std::vector< GUCEF::WEB::CIHTTPServerRouter::THTTPServerResourcePtr >       THttpResourceVector;
    typedef std::map< CORE::CString, CORE::CValueList >                                 TStringToValueListMap;
    typedef std::map< CORE::CString, CORE::UInt8 >                                      TStringToUInt8Map;
    typedef GUCEF::WEB::CTDataNodeSerializableMapHttpServerResource< CORE::CString, CORE::CValueList >  TStringToValueListMapHttpResource;
    typedef GUCEF::WEB::CTDataNodeSerializableMapHttpServerResource< CORE::UInt32, RedisNodeWithPipe >  TUInt32ToRedisNodeMapHttpResource;    
    typedef GUCEF::WEB::CTDataNodeSerializableMapHttpServerResource< CORE::UInt32, TStringSet >         TUInt32ToStringSetMapHttpResource;    

    RedisClusterPtr m_redisContext;
    Settings m_settings;
    TRedisArgs m_redisPacketArgs;
    CORE::CTimer* m_metricsTimer;
    CORE::CTimer* m_redisReconnectTimer;
    CORE::CString::StringSet m_filteredStreamNames;
    RedisNodeWithPipeMap m_redisNodesMap;
    TUInt32ToStringSetMap m_hashSlotOriginStrMap;
    CORE::UInt32 m_redisClusterErrorReplies;
    CORE::UInt32 m_redisClusterTimeoutReplies;
    THttpResourceVector m_httpResources;
    
    CORE::CValueList m_cmdClusterInfo;
    CORE::CValueList m_cmdInfoCommandstats;
    CORE::CValueList m_cmdInfoMemory;
    CORE::CValueList m_cmdInfoReplication;
    CORE::CValueList m_cmdInfoPersistence;
    CORE::CValueList m_cmdInfoStats;
    CORE::CValueList m_cmdInfoClients;
    CORE::CValueList m_cmdInfoCpu;
    CORE::CValueList m_cmdInfoKeyspace;
    TStringToValueListMap m_cmdXinfoStreamMap;
    CORE::CValueList m_status;
    TStringToUInt8Map m_redisInfoValueTypes;

    bool m_vfsIsInitialized;
    MT::CReadWriteLock m_lock;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< RedisInfoService, MT::CMutex > RedisInfoServicePtr;

/*-------------------------------------------------------------------------*/

class RedisInfo;

class RestApiRedisInfoInfoResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiRedisInfoInfoResource( RedisInfo* app );

    virtual ~RestApiRedisInfoInfoResource() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CORE::CString& params         ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    RedisInfo* m_app;
};

/*-------------------------------------------------------------------------*/

class RestApiRedisInfoConfigResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiRedisInfoConfigResource( RedisInfo* app, bool appConfig );

    virtual ~RestApiRedisInfoConfigResource() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CORE::CString& params         ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDeserializeState Deserialize( const CORE::CString& resourcePath   ,
                                           const CORE::CDataNode& input        ,
                                           const CORE::CString& representation ,
                                           bool isDeltaUpdateOnly              ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    RedisInfo* m_app;
    bool m_appConfig;
};

/*-------------------------------------------------------------------------*/

class RedisInfo : public CORE::CObserver
{
    public:

    RedisInfo( void );
    virtual ~RedisInfo();

    bool Start( void );

    bool SetStandbyMode( bool newMode );

    bool IsGlobalStandbyEnabled( void ) const;

    bool LoadConfig( const CORE::CValueList& appConfig   ,
                     const CORE::CDataNode& globalConfig );

    const CORE::CValueList& GetAppConfig( void ) const;

    const CORE::CDataNode& GetGlobalConfig( void ) const;

    private:

    typedef CORE::CTEventHandlerFunctor< RedisInfo > TEventCallback;
    typedef std::map< CORE::CString, RedisInfoServicePtr > TStringToInfoServiceMap;
    typedef GUCEF::WEB::CTReadableMapIndexHttpServerResource< CORE::CString, RedisInfoServicePtr > TStringToInfoServiceMapWebResource;

    private:

    bool m_isInStandby;
    bool m_globalStandbyEnabled;
    CORE::CString m_redisHost;
    CORE::UInt16 m_redisPort;
    WEB::CHTTPServer m_httpServer;
    WEB::CDefaultHTTPServerRouter m_httpRouter;
    WEB::CTaskManagerServerResource m_taskManagementRsc;
    CORE::CValueList m_appConfig;
    CORE::CDataNode m_globalConfig;
    TStringToInfoServiceMap m_infoServices;
    TStringToSettingsMap m_settings;
    MT::CReadWriteLock m_appLock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace REDISINFO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

