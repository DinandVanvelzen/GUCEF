/*
 *  ProcessMetrics: Service for obtaining stats on a proc by name
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

#include <queue>

#ifndef GUCEF_CORE_CITASKCONSUMER_H
#include "gucefCORE_CITaskConsumer.h"
#define GUCEF_CORE_CITASKCONSUMER_H
#endif /* GUCEF_CORE_CITASKCONSUMER_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_COMCORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENT_H
#include "gucefPUBSUB_CPubSubClient.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENT_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENT_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#include "gucefPUBSUB_CPubSubClientFactory.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H ? */

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

#ifndef GUCEF_WEB_CTASKMANAGERSERVERRESOURCE_H
#include "gucefWEB_CTaskManagerServerResource.h"
#define GUCEF_WEB_CTASKMANAGERSERVERRESOURCE_H
#endif /* GUCEF_WEB_CTASKMANAGERSERVERRESOURCE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class ProcessMetrics;

class RestApiProcessMetricsInfoResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiProcessMetricsInfoResource( ProcessMetrics* app );

    virtual ~RestApiProcessMetricsInfoResource();

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CORE::CString& params         ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    ProcessMetrics* m_app;
};

/*-------------------------------------------------------------------------*/

class RestApiProcessMetricsConfigResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiProcessMetricsConfigResource( ProcessMetrics* app, bool appConfig );

    virtual ~RestApiProcessMetricsConfigResource();

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CORE::CString& params         ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    ProcessMetrics* m_app;
    bool m_appConfig;
};

/*-------------------------------------------------------------------------*/

class MetricThreshold : public CORE::CIConfigurable
{
    public:

    CORE::CVariant minThreshold;
    CORE::CVariant maxThreshold;
    bool applyMinThreshold;
    bool applyMaxThreshold;
    CORE::CString thresholdDescription;
    CORE::CString metricName;
    CORE::CString::StringSet procFilter;

    MetricThreshold( void );

    bool IsValid( void );

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const;

    virtual bool LoadConfig( const CORE::CDataNode& cfg );

    virtual const CORE::CString& GetClassTypeName( void ) const;

};

/*-------------------------------------------------------------------------*/

class ProcessMetrics : public CORE::CObservingNotifier
{
    public:

    ProcessMetrics( void );
    virtual ~ProcessMetrics();

    bool Start( void );

    bool SetStandbyMode( bool newMode );

    bool IsGlobalStandbyEnabled( void ) const;

    bool LoadConfig( const CORE::CValueList& appConfig   ,
                     const CORE::CDataNode& globalConfig );

    const CORE::CValueList& GetAppConfig( void ) const;

    const CORE::CDataNode& GetGlobalConfig( void ) const;

    private:

    void RegisterEventHandlers( void );

    void RefreshPIDs( const CORE::CString::StringSet& exeNames );

    void RefreshPIDs( void );

    bool LaunchProcs( const CORE::CString::StringSet& procsToLaunch );

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void
    OnProcScanTimerCycle( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData );

    void ValidateMetricThresholds( const CORE::CVariant& metricValue ,
                                   const CORE::CString& metricName   ,
                                   const CORE::CString& procName     );

    void PublishMetricThresholdExceeded( const CORE::CVariant& metricValue ,
                                         const CORE::CString& metricName   ,
                                         const CORE::CString& procName     ,
                                         const MetricThreshold& threshold  );

    bool SetupPubSubClient( const CORE::CDataNode& cfg );

    static CORE::CString GenerateMetricsFriendlyString( const CORE::CString& str );

    private:

    class CProcInfo : public CORE::CObservingNotifier
    {
        public:

        static const CORE::CEvent PidChangedEvent;

        CORE::TProcessId pid;
        CORE::TProcCpuDataPoint* previousProcCpuDataDataPoint;
        CORE::TProcessMemoryUsageInfo prevMemUseInfo;
        CORE::CProcessInformation processInformation;
        UInt64 lastUptimeInMs;
        CORE::CString exeName;
        bool pidWasPredefined;
        bool gatherMetrics;
        bool startIfNotRunning;
        bool restartIfStopsRunning;

        CProcInfo( void );

        ~CProcInfo();

        CProcInfo& operator=( const CProcInfo& src );

        void Clear( void );

        bool RefreshPID( void );

        bool RefreshPID( CORE::TProcessId* allProcIds, CORE::UInt32 procIdCount );

        bool RefreshPID( CORE::TProcessId newProcId );

        bool IsProcessStillActive( void );

        void Unlink( void );
    };

    typedef CORE::CTEventHandlerFunctor< ProcessMetrics > TEventCallback;
    typedef std::map< CORE::CString, CProcInfo > TProcessIdMap;
    typedef CORE::CString::StringSet TStringSet;
    typedef CORE::CString::StringVector TStringVector;
    typedef std::map< CORE::CString, MetricThreshold >  TMetricsThresholdMap;
    typedef std::map< CORE::CString, TMetricsThresholdMap >  TMetricsThresholdMapMap;
    typedef std::pair< CORE::CStorageDeviceInfoOSDataPtr, CORE::CStorageDeviceInformationPtr >  TStorageDeviceInfoPair;
    typedef std::map< CORE::CString, TStorageDeviceInfoPair > TStorageDeviceInfoMap;
    typedef std::pair< CORE::CStorageVolumeInfoOSDataPtr, CORE::CStorageVolumeInformationPtr >  TStorageVolumeInfoPair;
    typedef std::map< CORE::CString, TStorageVolumeInfoPair > TStorageVolumeInfoMap;

    bool GetStorageDeviceInformationObjs( const CORE::CString& deviceId                       ,
                                          CORE::CStorageDeviceInfoOSDataPtr& deviceInfoOSData ,
                                          CORE::CStorageDeviceInformationPtr& deviceInfo      );

    bool GetStorageVolumeInformationObjs( const CORE::CString& volumeId                       ,
                                          CORE::CStorageVolumeInfoOSDataPtr& volumeInfoOSData ,
                                          CORE::CStorageVolumeInformationPtr& deviceInfo      );

    void DispatchStorageVolumeMetrics( CORE::CStorageVolumeInfoOSDataPtr volumeInfoOSData ,
                                       CORE::CStorageVolumeInformationPtr deviceInfo      );

    WEB::CHTTPServer m_httpServer;
    WEB::CDefaultHTTPServerRouter m_httpRouter;
    WEB::CTaskManagerServerResource m_taskManagementRsc;
    CORE::CValueList m_appConfig;
    CORE::CDataNode m_globalConfig;
    CORE::CTimer m_metricsTimer;
    CORE::CTimer m_procIndexTimer;
    PUBSUB::CPubSubClientFactory::TProductPtr m_pubSubClient;
    PUBSUB::CPubSubClientTopicBasicPtr m_thresholdNotificationPublishTopic;
    PUBSUB::CPubSubClientFeatures m_pubSubFeatures;
    CORE::CString m_thresholdNotificationPrimaryPayloadCodecType;
    bool m_gatherMemStats;
    bool m_enableRestApi;
    bool m_enableEventMsgPublishing;
    TProcessIdMap m_exeProcsToWatch;
    CORE::UInt32 m_exeMatchPidMatchThreshold;
    TMetricsThresholdMap m_metricsThresholds;
    TMetricsThresholdMapMap m_procMetricsThresholds;
    CORE::TCpuDataPoint* m_globalCpuDataPoint;
    CORE::CString::StringSet m_storageVolumeIds;
    CORE::CString::StringMap m_storageVolumeIdsToPaths;
    CORE::CString::StringSet m_storageDeviceIds;
    TStorageDeviceInfoMap m_storageDeviceInfoMap;
    TStorageVolumeInfoMap m_storageVolumeInfoMap;

    bool m_gatherProcPageFaultCountInBytes;
    bool m_gatherProcPageFileUsageInBytes;
    bool m_gatherProcPeakPageFileUsageInBytes;
    bool m_gatherProcPeakWorkingSetSizeInBytes;
    bool m_gatherProcWorkingSetSizeInBytes;

    bool m_gatherGlobMemStats;
    bool m_gatherGlobalAvailablePageFileSizeInBytes;
    bool m_gatherGlobalPageFileUsageInBytes;
    bool m_gatherGlobalAvailableVirtualMemoryInBytes;
    bool m_gatherGlobalAvailExtendedVirtualMemoryInBytes;
    bool m_gatherGlobalMemoryLoadPercentage;
    bool m_gatherGlobalTotalPageFileSizeInBytes;
    bool m_gatherGlobalTotalPhysicalMemoryInBytes;
    bool m_gatherGlobalTotalVirtualMemoryInBytes;

    bool m_gatherProcCpuStats;
    bool m_gatherProcCpuUptime;
    bool m_gatherProcCpuOverallPercentage;
    bool m_gatherProcCpuThreadCount;

    bool m_gatherGlobalCpuStats;
    bool m_gatherGlobalCpuCurrentFrequencyInMhz;
    bool m_gatherGlobalCpuSpecMaxFrequencyInMhz;
    bool m_gatherGlobalCpuMaxFrequencyInMhz;
    bool m_gatherGlobalCpuOverallPercentage;
    bool m_gatherGlobalCpuOverallPercentagePerCore;

    bool m_gatherGlobalNetworkStats;
    bool m_gatherGlobalNetworkStatInboundOctets;
    bool m_gatherGlobalNetworkStatInboundUnicastOctets;
    bool m_gatherGlobalNetworkStatInboundUnicastPackets;
    bool m_gatherGlobalNetworkStatInboundNonUnicastPackets;
    bool m_gatherGlobalNetworkStatInboundErroredPackets;
    bool m_gatherGlobalNetworkStatInboundDiscardedPackets;
    bool m_gatherGlobalNetworkStatOutboundOctets;
    bool m_gatherGlobalNetworkStatOutboundUnicastOctets;
    bool m_gatherGlobalNetworkStatOutboundUnicastPackets;
    bool m_gatherGlobalNetworkStatOutboundNonUnicastPackets;
    bool m_gatherGlobalNetworkStatOutboundErroredPackets;
    bool m_gatherGlobalNetworkStatOutboundDiscardedPackets;
    bool m_gatherGlobalNetworkStatInboundMulticastOctets;
    bool m_gatherGlobalNetworkStatOutboundMulticastOctets;
    bool m_gatherGlobalNetworkStatInboundBroadcastOctets;
    bool m_gatherGlobalNetworkStatOutboundBroadcastOctets;
    bool m_gatherGlobalNetworkStatTransmitLinkSpeedBitsPerSec;
    bool m_gatherGlobalNetworkStatReceiveLinkSpeedBitsPerSec;
    bool m_useNetworkInterfaceDescriptionAsMetricName;

    bool m_gatherGlobalStorageVolumeStats;
    bool m_gatherGlobalStorageVolumeStatsPerVolume;
    bool m_gatherGlobalStorageVolumeBytesAvailableToCaller;
    bool m_gatherGlobalStorageVolumeBytesAvailable;
    bool m_gatherGlobalStorageVolumeBytes;
    bool m_gatherGlobalStorageVolumeAvailableToCallerPercentage;
    bool m_gatherGlobalStorageVolumeAvailablePercentage;
    bool m_gatherGlobalStorageVolumeBytesWritten;
    bool m_gatherGlobalStorageVolumeBytesWrittenPerSec;
    bool m_gatherGlobalStorageVolumeAvgWriteTimePerOperationInMs;
    bool m_gatherGlobalStorageVolumeBytesRead;
    bool m_gatherGlobalStorageVolumeBytesReadPerSec;
    bool m_gatherGlobalStorageVolumeAvgReadTimePerOperationInMs;
    bool m_gatherGlobalStorageVolumeRequestsQueued;
    bool m_gatherGlobalStorageVolumeRequestsSplit;
    bool m_gatherGlobalStorageVolumeRequestsSplitPerSec;
    bool m_gatherGlobalStorageVolumeReadTimeInMs;
    bool m_gatherGlobalStorageVolumeWriteTimeInMs;
    bool m_gatherGlobalStorageVolumeIdleTimeInMs;
    bool m_convertStorageVolumeIdsToPathsAsMetricName;

    bool m_gatherGlobalStorageDeviceStats;
    bool m_gatherGlobalStorageDeviceBytesWritten;
    bool m_gatherGlobalStorageDeviceBytesWrittenPerSec;
    bool m_gatherGlobalStorageDeviceAvgWriteTimePerOperationInMs;
    bool m_gatherGlobalStorageDeviceBytesRead;
    bool m_gatherGlobalStorageDeviceBytesReadPerSec;
    bool m_gatherGlobalStorageDeviceAvgReadTimePerOperationInMs;
    bool m_gatherGlobalStorageDeviceRequestsQueued;
    bool m_gatherGlobalStorageDeviceRequestsSplit;
    bool m_gatherGlobalStorageDeviceRequestsSplitPerSec;
    bool m_gatherGlobalStorageDeviceReadTimeInMs;
    bool m_gatherGlobalStorageDeviceWriteTimeInMs;
    bool m_gatherGlobalStorageDeviceIdleTimeInMs;
};

/*-------------------------------------------------------------------------*/
