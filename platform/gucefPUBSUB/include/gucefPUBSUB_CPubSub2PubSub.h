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

#ifndef GUCEF_PUBSUB_CPUBSUB2PUBSUB_H
#define GUCEF_PUBSUB_CPUBSUB2PUBSUB_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <deque>

#ifndef GUCEF_MT_CNOLOCK_H
#include "gucefMT_CNoLock.h"
#define GUCEF_MT_CNOLOCK_H
#endif /* GUCEF_MT_CNOLOCK_H ? */

#ifndef GUCEF_MT_CTMAILBOX_H
#include "gucefMT_CTMailBox.h"
#define GUCEF_MT_CTMAILBOX_H
#endif /* GUCEF_MT_CTMAILBOX_H ? */

#ifndef GUCEF_CORE_CTMAILBOXFORSHAREDCLONEABLES_H
#include "gucefCORE_CTMailboxForSharedCloneables.h"
#define GUCEF_CORE_CTMAILBOXFORSHAREDCLONEABLES_H
#endif /* GUCEF_CORE_CTMAILBOXFORSHAREDCLONEABLES_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CIASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#include "gucefCORE_CGloballyConfigurable.h"
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERSWAP_H
#include "gucefCORE_CDynamicBufferSwap.h"
#define GUCEF_CORE_CDYNAMICBUFFERSWAP_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERSWAP_H ? */

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

#ifndef GUCEF_WEB_CTREADABLEMAPINDEXHTTPSERVERRESOURCE_H
#include "gucefWEB_CTReadableMapIndexHttpServerResource.h"
#define GUCEF_WEB_CTREADABLEMAPINDEXHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CTREADABLEMAPINDEXHTTPSERVERRESOURCE_H ? */

#ifndef GUCEF_WEB_CTDATANODESERIALIZABLEMAPHTTPSERVERRESOURCE_H
#include "gucefWEB_CTDataNodeSerializableMapHttpServerResource.h"
#define GUCEF_WEB_CTDATANODESERIALIZABLEMAPHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CTDATANODESERIALIZABLEMAPHTTPSERVERRESOURCE_H ? */

#ifndef GUCEF_WEB_CTASKMANAGERSERVERRESOURCE_H
#include "gucefWEB_CTaskManagerServerResource.h"
#define GUCEF_WEB_CTASKMANAGERSERVERRESOURCE_H
#endif /* GUCEF_WEB_CTASKMANAGERSERVERRESOURCE_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENT_H
#include "gucefPUBSUB_CPubSubClient.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENT_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENT_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#include "gucefPUBSUB_CPubSubClientFactory.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H
#include "gucefPUBSUB_CPubSubMsgBinarySerializer.h"
#define GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H
#endif /* GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBMSGCONTAINERBINARYSERIALIZER_H
#include "gucefPUBSUB_CPubSubMsgContainerBinarySerializer.h"
#define GUCEF_PUBSUB_CPUBSUBMSGCONTAINERBINARYSERIALIZER_H
#endif /* GUCEF_PUBSUB_CPUBSUBMSGCONTAINERBINARYSERIALIZER_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBSIDECHANNELSETTINGS_H
#include "gucefPUBSUB_CPubSubSideChannelSettings.h"
#define GUCEF_PUBSUB_CPUBSUBSIDECHANNELSETTINGS_H
#endif /* GUCEF_PUBSUB_CPUBSUBSIDECHANNELSETTINGS_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H
#include "gucefPUBSUB_CPubSubClientSide.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTCHANNEL_H
#include "gucefPUBSUB_CPubSubClientChannel.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTCHANNEL_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTCHANNEL_H ? */

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

class PubSub2PubSub;

class GUCEF_PUBSUB_EXPORT_CPP RestApiPubSub2PubSubInfoResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiPubSub2PubSubInfoResource( PubSub2PubSub* app );

    virtual ~RestApiPubSub2PubSubInfoResource();

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CORE::CString& params         ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    PubSub2PubSub* m_app;
};

/*-------------------------------------------------------------------------*/

class GUCEF_PUBSUB_EXPORT_CPP RestApiPubSub2PubSubHealthResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiPubSub2PubSubHealthResource( PubSub2PubSub* app );

    virtual ~RestApiPubSub2PubSubHealthResource();

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CORE::CString& params         ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    PubSub2PubSub* m_app;
};

/*-------------------------------------------------------------------------*/

class GUCEF_PUBSUB_EXPORT_CPP RestApiPubSub2PubSubConfigResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiPubSub2PubSubConfigResource( PubSub2PubSub* app, bool appConfig );

    virtual ~RestApiPubSub2PubSubConfigResource();

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CORE::CString& params         ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDeserializeState Deserialize( const CORE::CString& resourcePath   ,
                                           const CORE::CDataNode& input        ,
                                           const CORE::CString& representation ,
                                           bool isDeltaUpdateOnly              ) GUCEF_VIRTUAL_OVERRIDE;

    TDeserializeState UpdateGlobalConfig( const CORE::CDataNode& cfg );
    
    private:

    PubSub2PubSub* m_app;
    bool m_appConfig;
};

/*-------------------------------------------------------------------------*/

class GUCEF_PUBSUB_EXPORT_CPP RestApiPubSubClientChannelConfigResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiPubSubClientChannelConfigResource( PubSub2PubSub* app );

    virtual ~RestApiPubSubClientChannelConfigResource();

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CORE::CString& params         ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    PubSub2PubSub* m_app;
};

/*-------------------------------------------------------------------------*/

class GUCEF_PUBSUB_EXPORT_CPP PubSub2PubSubConfig : public CORE::CIConfigurable
{
    public:

    class GUCEF_PUBSUB_EXPORT_CPP ExplicitChannelSideTopicOverlayConfig : public CORE::CIConfigurable
    {
        public:

        CORE::CString topicName;

        ExplicitChannelSideTopicOverlayConfig( void );
        ExplicitChannelSideTopicOverlayConfig( const ExplicitChannelSideTopicOverlayConfig& src );
        virtual ~ExplicitChannelSideTopicOverlayConfig();

        virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

        virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

        virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    };

    class GUCEF_PUBSUB_EXPORT_CPP ExplicitChannelSideOverlayConfig : public CORE::CIConfigurable
    {
        public:

        typedef std::vector< ExplicitChannelSideTopicOverlayConfig >   ExplicitChannelSideTopicOverlayConfigVector;
        typedef CPubSubClientConfig::THostAddressVector                THostAddressVector;

        CORE::CString sideId;
        THostAddressVector remoteAddresses;
        ExplicitChannelSideTopicOverlayConfigVector topics;

        ExplicitChannelSideOverlayConfig( void );
        ExplicitChannelSideOverlayConfig( const ExplicitChannelSideOverlayConfig& src );
        virtual ~ExplicitChannelSideOverlayConfig();

        virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

        virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

        virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    };

    class GUCEF_PUBSUB_EXPORT_CPP ExplicitChannelOverlayConfig : public CORE::CIConfigurable
    {
        public:

        typedef std::vector< ExplicitChannelSideOverlayConfig >   ExplicitChannelSideOverlayConfigVector;

        CORE::CString usingTemplate;
        CORE::Int32 channelId;
        CORE::CString channelName;
        ExplicitChannelSideOverlayConfigVector sides;

        ExplicitChannelOverlayConfig( void );
        ExplicitChannelOverlayConfig( const ExplicitChannelOverlayConfig& src );
        virtual ~ExplicitChannelOverlayConfig();

        virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

        virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

        virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    };

    class GUCEF_PUBSUB_EXPORT_CPP NumericalAutoChannelConfig : public CORE::CIConfigurable
    {
        public:

        typedef std::vector< ExplicitChannelSideOverlayConfig >   ExplicitChannelSideOverlayConfigVector;

        CORE::CString usingTemplate;
        UInt32 channelCount;
        Int32 firstChannelId;
        CORE::CString::StringSet channelIds;

        NumericalAutoChannelConfig( void );
        NumericalAutoChannelConfig( const NumericalAutoChannelConfig& src );
        virtual ~NumericalAutoChannelConfig();

        virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

        virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

        virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    };

    typedef std::map< CORE::CString, CPubSubChannelSettings >   StringToPubSubChannelConfigMap;
    typedef std::map< Int32, CPubSubChannelSettings >           Int32ToPubSubChannelConfigMap;
    typedef std::vector< ExplicitChannelOverlayConfig >         ExplicitChannelOverlayConfigVector;
    typedef std::set< CORE::Int32 >                             Int32Set;

    StringToPubSubChannelConfigMap channelConfigTemplates;
    ExplicitChannelOverlayConfigVector explicitOverlayChannels;
    NumericalAutoChannelConfig numericalAutoChannelConfig;
    Int32ToPubSubChannelConfigMap channelConfigs;

    bool globalStandbyEnabled;
    bool applyCpuThreadAffinityByDefault;
    bool enableRestApi;
    UInt16 restApiPort;
    CORE::CString restBasicHealthUri;
    bool transmitMetrics;

    bool NormalizeConfig( void );

    PubSub2PubSubConfig( void );
    PubSub2PubSubConfig( const PubSub2PubSubConfig& src );
    virtual ~PubSub2PubSubConfig();

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

class GUCEF_PUBSUB_EXPORT_CPP PubSub2PubSub : public CORE::CObserver             ,
                                              public CORE::CGloballyConfigurable
{
    public:

    PubSub2PubSub( void );
    virtual ~PubSub2PubSub();

    bool Start( void );

    bool SetStandbyMode( bool newMode );

    bool IsGlobalStandbyEnabled( void ) const;

    bool IsHealthy( void ) const;

    bool GetLatestIsHealthyState( void ) const;

    CORE::CDateTime GetLatestIsHealthyStateChangeDt( void ) const;

    const CORE::CDataNode& GetGlobalConfig( void ) const;

    static const CORE::CDataNode* GetAppConfig( const CORE::CDataNode& globalConfig );

    static CORE::CDataNode* GetAppConfig( CORE::CDataNode& globalConfig );

    const CORE::CDataNode* GetAppConfig( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const CORE::CDateTime& GetAppCompileDateTime( void );

    CPubSubClientChannelPtr GetChannelByChannelId( CORE::Int32 cid ) const;

    private:

    typedef CORE::CTEventHandlerFunctor< PubSub2PubSub > TEventCallback;

    void
    OnVfsInit( CORE::CNotifier* notifier    ,
               const CORE::CEvent& eventId  ,
               CORE::CICloneable* eventData );

    void
    OnAppShutdown( CORE::CNotifier* notifier    ,
                   const CORE::CEvent& eventId  ,
                   CORE::CICloneable* eventData );

    void
    OnChannelHealthStatusChanged( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData );

    void RegisterEventHandlers( void );

    void RegisterChannelEventHandlers( CPubSubClientChannelPtr channel );

    bool LoadConfigAfterVfsInit( const CORE::CDataNode& globalConfig );
    
    private:

    typedef std::map< CORE::Int32, CPubSubClientChannelPtr > PubSubClientChannelMap;
    typedef WEB::CTDataNodeSerializableMapHttpServerResource< CORE::Int32, CPubSubChannelSettings >        TWebChannelCfgMapIndexMap;

    bool m_isInStandby;
    bool m_desiredInStandby;
    PubSubClientChannelMap m_channels;
    WEB::CHTTPServer m_httpServer;
    WEB::CDefaultHTTPServerRouter m_httpRouter;
    WEB::CTaskManagerServerResource m_taskManagementRsc;
    mutable bool m_isHealthy;
    mutable CORE::CDateTime m_lastIsHealthyChange;
    PubSub2PubSubConfig m_config;
    CORE::CDataNode m_globalConfig;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PUBSUB_CPUBSUB2PUBSUB_H ? */

