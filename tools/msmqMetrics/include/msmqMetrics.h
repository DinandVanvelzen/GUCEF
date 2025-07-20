/*
 *  MsmqMetrics: Service for obtaining MSMQ stats
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <queue>

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CIASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

class MsmqMetrics;

class RestApiProcessMetricsInfoResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiProcessMetricsInfoResource( MsmqMetrics* app );

    virtual ~RestApiProcessMetricsInfoResource();

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CORE::CString& params         ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    MsmqMetrics* m_app;
};

/*-------------------------------------------------------------------------*/

class RestApiProcessMetricsConfigResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiProcessMetricsConfigResource( MsmqMetrics* app, bool appConfig );

    virtual ~RestApiProcessMetricsConfigResource();

    virtual bool Serialize( const CORE::CString& resourcePath   , 
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CORE::CString& params         ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    MsmqMetrics* m_app;
    bool m_appConfig;
};

/*-------------------------------------------------------------------------*/

class MsmqMetrics : public CORE::CObservingNotifier
{
    public:

    MsmqMetrics( void );
    virtual ~MsmqMetrics();

    bool Start( void );

    bool SetStandbyMode( bool newMode );

    bool IsGlobalStandbyEnabled( void ) const;

    bool LoadConfig( const CORE::CValueList& appConfig   ,
                     const CORE::CDataNode& globalConfig );

    const CORE::CValueList& GetAppConfig( void ) const;

    const CORE::CDataNode& GetGlobalConfig( void ) const;

    private:

    void RegisterEventHandlers( void );

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void
    OnQueueActivityCheckTimerCycle( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData );

    static bool GetQueueSecurityDescriptor( const CORE::CString& formatName          ,
                                            SECURITY_INFORMATION infoTypeToGet       ,
                                            CORE::CDynamicBuffer& securityDescriptor );

    static bool GetQueueSecurityDescriptor( const std::wstring& formatName           ,
                                            SECURITY_INFORMATION infoTypeToGet       ,
                                            CORE::CDynamicBuffer& securityDescriptor );

    static bool GetQueueSecurityDescriptorDACL( const CORE::CString& formatName          ,
                                                CORE::CDynamicBuffer& securityDescriptor );
    
    static bool GetQueueOwner( const std::wstring& formatName ,
                               std::wstring& domainName       ,
                               std::wstring& accountName      ,
                               std::wstring& accountSid       ,
                               bool& isOwnerDefaulted         );
    
    static bool GetQueueOwner( const CORE::CString& formatName ,
                               CORE::CString& domainName       ,
                               CORE::CString& accountName      ,
                               CORE::CString& accountSid       ,
                               bool& isOwnerDefaulted          );
    
    typedef std::map< ::PSID, ::ACCESS_MASK >   TPSIDToAccessMaskMap;    
    static bool GetMsmqPermissionList( const CORE::CDynamicBuffer& securityDescriptor ,
                                       TPSIDToAccessMaskMap& accessList               );

    typedef std::map< CORE::CString, ::ACCESS_MASK >   TSIDStrToAccessMaskMap;    
    static bool GetMsmqPermissionList( const CORE::CString& formatName    ,
                                       TSIDStrToAccessMaskMap& accessList );

    static CORE::CString CovertPSIDToString( ::PSID psid );

    static std::wstring CovertPSIDToWString( ::PSID psid );

    static CORE::CString GetMsmqPermissionsAsString( ::ACCESS_MASK amMask );

    static bool GetAccountInfoForSid( const std::wstring& accountSid ,
                                      std::wstring& domainName       ,
                                      std::wstring& accountName      );
    
    static bool GetAccountInfoForSid( const CORE::CString& accountSid ,
                                      CORE::CString& domainName       ,
                                      CORE::CString& accountName      );

    static bool GetAccountInfoForSid( ::PSID accountSid         ,
                                      std::wstring& domainName  ,
                                      std::wstring& accountName );
    
    class MsmqQueueProperties
    {
        public:
        
        CORE::CString queueLabel;
        UInt64 quota;
        CORE::CString pathName;
        CORE::CString pathNameDNS;
        CORE::CString typeId;
        CORE::CString ownerDomainName;
        CORE::CString ownerAccountName;
        CORE::CString ownerSID;
        bool ownerIsDefaulted;
        ::ACCESS_MASK ownerAccessMask;        
        TSIDStrToAccessMaskMap queuePermissions;
        CORE::CString hostname;

        CORE::CString ToString( void ) const;

        MsmqQueueProperties( void );
    };

    class MsmqQueue
    {
        public:
        
        CORE::CString queueName;
        std::wstring msmqQueueFormatName;
        bool queueNameIsMsmqFormatName;
        CORE::CString metricFriendlyQueueName;
        MsmqQueueProperties queueProperties;
        bool isActive;
        bool isOutgoingQueue;
        bool collectMetrics;

        MsmqQueue( const CORE::CString& qName                           , 
                   bool queueNamesAreMsmqFormatNames                    ,
                   const CORE::CString& hostname = CORE::CString::Empty );

        MsmqQueue( const MsmqQueue& src );
    };
    typedef std::vector< MsmqQueue >    MsmqQueueVector;
    
    CORE::Int64 GetQueueMetric( MsmqQueue& q                           , 
                                const CORE::CString& metricDescription ,
                                CORE::UInt32 propId                    ,
                                CORE::UInt32 propType                  ) const;

    CORE::UInt64 GetMsmqComputerMetric( const CORE::CString& metricDescription ,
                                        CORE::UInt32 propId                    ,
                                        CORE::UInt32 propType                  ) const;
    
    CORE::Int64 GetCurrentNrOfMessagesInQueue( MsmqQueue& q ) const;

    CORE::Int64 GetCurrentByteCountOfMesssagesInQueue( MsmqQueue& q ) const;

    CORE::Int64 GetCurrentNrOfMessagesInQueueJournal( MsmqQueue& q ) const;

    CORE::Int64 GetCurrentByteCountOfMesssagesInQueueJournal( MsmqQueue& q ) const;

    CORE::UInt64 GetComputerGlobalTotalBytesOfAllMessagesOfAllQueues( void ) const;

    static const std::wstring& GetMsmqQueueFormatName( MsmqQueue& q );

    static bool GetMsmqQueueLabel( const std::wstring& queueFormatName  ,
                                   CORE::CString& queueLabel            );

    static bool GetMsmqQueuePathName( const std::wstring& queueFormatName  ,
                                      CORE::CString& queuePathName         );

    static bool GetMsmqQueuePathName( const CORE::CString& hostname        ,
                                      const std::wstring& queueFormatName  ,                                      
                                      CORE::CString& queuePathName         );

    static bool GetMsmqQueuePathNameDNS( const std::wstring& queueFormatName  ,
                                         CORE::CString& queuePathNameDNS      );

    static bool GetMsmqQueueQuota( const std::wstring& queueFormatName ,
                                   UInt64& queueQuota                  );

    static bool GetMsmqQueueType( const std::wstring& queueFormatName ,
                                  CLSID& queueTypeId                  );

    static bool GetMsmqQueueType( const std::wstring& queueFormatName ,
                                  CORE::CString& queueTypeId          );

    static bool GetMsmqQueueProperties( const std::wstring& queueFormatName  ,
                                        MsmqQueueProperties& queueProperties );

    static bool MsmqQueueGUIDToMsmqQueueFormatName( const CORE::CString& queueGuid ,
                                                    std::wstring& queueFormatName  );
    
    static bool MsmqQueueGUIDToMsmqQueueFormatName( const GUID& queueGuid          ,
                                                    std::wstring& queueFormatName  );

    static bool MsmqQueueGUIDToMsmqQueueFormatName( const GUID& queueGuid           ,
                                                     CORE::CString& queueFormatName );

    static bool MsmqPathNameToMsmqQueueFormatName( const std::wstring& pathName   ,
                                                    std::wstring& queueFormatName  );

    static bool MsmqPathNameToMsmqQueueFormatName( const CORE::CString& pathName   ,
                                                   CORE::CString& queueFormatName  );

    static bool MsmqQueueGUIDToMsmqQueueFormatName( const CORE::CString& queueGuid ,
                                                    CORE::CString& queueFormatName );
    
    static bool StringToMsmqGUID( const CORE::CAsciiString& guidString, GUID& guid );
    
    static bool MsmqGUIDToString( const GUID& guid, CORE::CAsciiString& guidString );

    static CORE::CString GenerateMetricsFriendlyQueueName( const CORE::CString& queueName );

    static bool InitQueueInfo( MsmqQueue& q );

    typedef std::vector< std::wstring > TWStringVector;

    static bool GetPrivateQueues( const std::wstring& computerName ,
                                  TWStringVector& queuePathNames   );

    static bool GetPrivateQueues( const CORE::CString& computerName           ,
                                  CORE::CString::StringVector& queuePathNames );

    static bool GetLocalPrivateQueues( CORE::CString::StringVector& queuePathNames );

    static bool GetPublicQueues( CORE::CString::StringVector& queueIDs );

    static bool GetMsmqActiveQueues( const CORE::CString& hostname          ,
                                     CORE::CString::StringSet& activeQueues );

    static bool UpdateMsmqActiveQueueStatus( const CORE::CString::StringSet& hostnames ,
                                             MsmqQueueVector& queues                   ,
                                             bool addNewlyDiscoveredQueues             );

    static bool FindPrivateQueues( const CORE::CString::StringSet& hostNames          ,
                                   const CORE::CString::StringSet& globPatternFilters ,
                                   CORE::CString::StringSet& foundQueues              );

    static bool FindAllQueues( const CORE::CString::StringSet& globPatternFilters ,
                               CORE::CString::StringSet& foundQueues              );
        
    void SetMetricCollectionBasedOnFilter( MsmqQueueVector& queues                      , 
                                           CORE::CString::StringSet& globPatternFilters );
    
    private:

    typedef CORE::CTEventHandlerFunctor< MsmqMetrics > TEventCallback;

    WEB::CHTTPServer m_httpServer;
    WEB::CDefaultHTTPServerRouter m_httpRouter;
    WEB::CTaskManagerServerResource m_taskManagementRsc;
    CORE::CValueList m_appConfig;
    CORE::CDataNode m_globalConfig;
    CORE::CTimer m_metricsTimer;
    CORE::CTimer m_queueActivityCheckTimer;
    bool m_enableRestApi;
    MsmqQueueVector m_queues;
    bool m_queueNamesAreMsmqFormatNames;
    bool m_dontSendMetricsForInactiveQueues;
    CORE::CString m_metricsPrefix;
    CORE::CString::StringSet m_hostnames;
    bool m_discoverQueues;
    bool m_gatherMsgsInQueueMetric;
    bool m_gatherMsgsTotalByteCountMetric;
    bool m_gatherMsgsInQueueJournalMetric;
    bool m_gatherMsgsInJournalTotalByteCountMetric;
    bool m_gatherTotalMsgBytesMetric;
    bool m_gatherMetricsFromOutgoingQueues;
};

/*-------------------------------------------------------------------------*/
