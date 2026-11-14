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

#include <Objbase.h>
#include <Mq.h>
#include <sddl.h>

#undef SetPort

#include <string>

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#include "MsmqMetrics.h"

#ifndef GUCEF_CORE_METRICSMACROS_H
#include "gucefCORE_MetricsMacros.h"
#define GUCEF_CORE_METRICSMACROS_H
#endif /* GUCEF_CORE_METRICSMACROS_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H
#include "gucefWEB_CDummyHTTPServerResource.h"
#define GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

std::wstring 
RetrieveWin32APIErrorMessage( DWORD dwErr )
{
    // from: https://docs.microsoft.com/en-us/windows/win32/seccrypto/retrieving-error-messages

    WCHAR   wszMsgBuff[512];  // Buffer for text.

    DWORD   dwChars;  // Number of chars returned.

    // Try to get the message from the system errors.
    dwChars = ::FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM |
                                FORMAT_MESSAGE_IGNORE_INSERTS,
                                GUCEF_NULL,
                                dwErr,
                                0,
                                wszMsgBuff,
                                512,
                                GUCEF_NULL );

    if (0 == dwChars)
    {
        // The error code did not exist in the system errors.
        // Try Ntdsbmsg.dll for the error code.

        HINSTANCE hInstNtdsbmsg = GUCEF_NULL;

        // Load the library.
        hInstNtdsbmsg = ::LoadLibraryA( "Ntdsbmsg.dll" );
        if ( GUCEF_NULL == hInstNtdsbmsg )
        {
            // cannot load Ntdsbmsg.dll for refinement
            return std::wstring();
        }

        // Try getting message text from ntdsbmsg.
        dwChars = FormatMessageW( FORMAT_MESSAGE_FROM_HMODULE |
                                 FORMAT_MESSAGE_IGNORE_INSERTS,
                                 hInstNtdsbmsg,
                                 dwErr,
                                 0,
                                 wszMsgBuff,
                                 512,
                                 GUCEF_NULL );

        // Free the library.
        ::FreeLibrary( hInstNtdsbmsg );

    }

    return wszMsgBuff;
}

/*-------------------------------------------------------------------------*/

RestApiProcessMetricsInfoResource::RestApiProcessMetricsInfoResource( MsmqMetrics* app )
    : WEB::CCodecBasedHTTPServerResource()
    , m_app( app )
{GUCEF_TRACE;

    m_allowSerialize = true;
}

/*-------------------------------------------------------------------------*/

RestApiProcessMetricsInfoResource::~RestApiProcessMetricsInfoResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
RestApiProcessMetricsInfoResource::Serialize( const CORE::CString& resourcePath   ,
                                              CORE::CDataNode& output             ,
                                              const CORE::CString& representation ,
                                              const CORE::CString& params         )
{GUCEF_TRACE;

    static const CORE::CDateTime compileDt = CORE::CDateTime::CompileDateTime( __DATE__, __TIME__ );

    output.SetName( "info" );
    output.SetAttribute( "application", "MsmqMetrics" );
    output.SetAttribute( "buildDateTime", compileDt.ToIso8601DateTimeString( true, true ) );
    #ifdef GUCEF_DEBUG_MODE
    output.SetAttribute( "isReleaseBuild", CORE::CVariant( false ) );
    #else
    output.SetAttribute( "isReleaseBuild", CORE::CVariant( true ) );
    #endif
    return true;
}

/*-------------------------------------------------------------------------*/

RestApiProcessMetricsConfigResource::RestApiProcessMetricsConfigResource( MsmqMetrics* app, bool appConfig )
    : WEB::CCodecBasedHTTPServerResource()
    , m_app( app )
    , m_appConfig( appConfig )
{GUCEF_TRACE;

    m_allowSerialize = true;
}

/*-------------------------------------------------------------------------*/

RestApiProcessMetricsConfigResource::~RestApiProcessMetricsConfigResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
RestApiProcessMetricsConfigResource::Serialize( const CORE::CString& resourcePath   ,
                                                CORE::CDataNode& output             ,
                                                const CORE::CString& representation ,
                                                const CORE::CString& params         )
{GUCEF_TRACE;

    if ( m_appConfig )
    {
        const CORE::CValueList& loadedConfig = m_app->GetAppConfig();
        return loadedConfig.SaveConfig( output );
    }

    const CORE::CDataNode& globalConfig = m_app->GetGlobalConfig();
    output.Copy( globalConfig );
    return true;
}


/*-------------------------------------------------------------------------*/

MsmqMetrics::MsmqMetrics( void )
    : CORE::CObservingNotifier()
    , m_httpServer()
    , m_httpRouter()
    , m_taskManagementRsc()
    , m_appConfig()
    , m_globalConfig()
    , m_metricsTimer()
    , m_queueActivityCheckTimer()
    , m_enableRestApi( true )
    , m_queues()
    , m_queueNamesAreMsmqFormatNames( false )
    , m_dontSendMetricsForInactiveQueues( true )
    , m_metricsPrefix()
    , m_discoverQueues( false )
    , m_gatherMsgsInQueueMetric( true )
    , m_gatherMsgsTotalByteCountMetric( true )
    , m_gatherMsgsInQueueJournalMetric( true )
    , m_gatherMsgsInJournalTotalByteCountMetric( true )
    , m_gatherTotalMsgBytesMetric( false )
    , m_gatherMetricsFromOutgoingQueues( true )
{GUCEF_TRACE;

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

MsmqMetrics::~MsmqMetrics()
{GUCEF_TRACE;

    SetStandbyMode( true );
}

/*-------------------------------------------------------------------------*/

MsmqMetrics::MsmqQueue::MsmqQueue( const CORE::CString& qName        ,
                                   bool queueNamesAreMsmqFormatNames ,
                                   const CORE::CString& hostname     )
    : queueName( qName )
    , msmqQueueFormatName()
    , queueNameIsMsmqFormatName( queueNamesAreMsmqFormatNames )
    , metricFriendlyQueueName()
    , queueProperties()
    , isActive( true )
    , isOutgoingQueue( false )
    , collectMetrics( true )
{GUCEF_TRACE;

    queueProperties.hostname = hostname;
}

/*-------------------------------------------------------------------------*/

MsmqMetrics::MsmqQueue::MsmqQueue( const MsmqQueue& src )
    : queueName( src.queueName )
    , msmqQueueFormatName( src.msmqQueueFormatName )
    , queueNameIsMsmqFormatName( src.queueNameIsMsmqFormatName )
    , metricFriendlyQueueName( src.metricFriendlyQueueName )
    , queueProperties( src.queueProperties )
    , isActive( src.isActive )
    , isOutgoingQueue( src.isOutgoingQueue )
    , collectMetrics( src.collectMetrics )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

MsmqMetrics::MsmqQueueProperties::MsmqQueueProperties( void )
    : queueLabel()
    , quota( 0 )
    , pathName()
    , pathNameDNS()
    , ownerDomainName()
    , ownerAccountName()
    , ownerSID()
    , ownerIsDefaulted( false )
    , ownerAccessMask( 0 )
    , queuePermissions()
    , hostname()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString 
MsmqMetrics::MsmqQueueProperties::ToString( void ) const
{GUCEF_TRACE;

    CORE::CString ownerAccessPermsStr = ", ownerPermissions=[ " + MsmqMetrics::GetMsmqPermissionsAsString( ownerAccessMask ) + " ]";

    CORE::CString otherAccessPermsStr = ", otherPermissions=[ ";
    TSIDStrToAccessMaskMap::const_iterator i = queuePermissions.begin();
    while ( i != queuePermissions.end() )
    {
        CORE::CString domainName;
        CORE::CString accountName;
        MsmqMetrics::GetAccountInfoForSid( (*i).first, domainName, accountName );

        otherAccessPermsStr += " domainName=" + domainName + ", accountName=" + accountName + " permissions=[ " + MsmqMetrics::GetMsmqPermissionsAsString( (*i).second ) + " ]";

        ++i;
    }
    otherAccessPermsStr += " ]";
    
    CORE::CString propStr = "queueLabel=" + queueLabel + ", hostname=" + hostname + ", queueTypeId=" + typeId + ", quota=" + CORE::ToString( quota ) + ", pathName=" + pathName + ", pathNameDNS=" + pathNameDNS +
           ", ownerIsDefaulted=" + CORE::ToString( ownerIsDefaulted ) + ", ownerDomainName=" + ownerDomainName + ", ownerAccountName=" + ownerAccountName + ", ownerSID=" + ownerSID + ", ownerAccessMask=" + CORE::ToString( (UInt64) ownerAccessMask ) +
           ownerAccessPermsStr + otherAccessPermsStr;
    
    return propStr;
}

/*-------------------------------------------------------------------------*/

void
MsmqMetrics::RegisterEventHandlers( void )
{GUCEF_TRACE;

    TEventCallback callback( this, &MsmqMetrics::OnMetricsTimerCycle );
    SubscribeTo( &m_metricsTimer                ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback                       );

    TEventCallback callback2( this, &MsmqMetrics::OnQueueActivityCheckTimerCycle );
    SubscribeTo( &m_queueActivityCheckTimer     ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback2                      );
    
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::StringToMsmqGUID( const CORE::CAsciiString& guidString ,
                               GUID& guid                           )
{GUCEF_TRACE;

    if ( !guidString.IsNULLOrEmpty() && guidString.IsFormattingValid() )
    {
        int scanResult = sscanf( guidString.C_String(), "{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
                &guid.Data1, &guid.Data2, &guid.Data3, 
                &guid.Data4[0], &guid.Data4[1], &guid.Data4[2], &guid.Data4[3] ,
                &guid.Data4[4], &guid.Data4[5], &guid.Data4[6], &guid.Data4[7] );

        return 11 == scanResult;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::MsmqGUIDToString( const GUID& guid               ,
                               CORE::CAsciiString& guidString )
{GUCEF_TRACE;

    guidString.Reserve( 128 );
    int printResult = sprintf( guidString.C_String(), "{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
                guid.Data1, guid.Data2, guid.Data3, 
                guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3] ,
                guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7] );
    guidString.DetermineLength();
                
    return printResult > 0;
}

/*-------------------------------------------------------------------------*/

bool 
MsmqMetrics::GetMsmqQueueQuota( const std::wstring& queueFormatName ,
                                UInt64& queueQuota                  )
{GUCEF_TRACE;

    // Validate the input string.  
    if ( queueFormatName.empty() ) 
        return false;  

    // Define the maximum number of queue properties and a property counter.  
    const int NUMBEROFPROPERTIES = 1;  
    DWORD cPropId=0;  
  
    // Define a queue property structure.  
    MQQUEUEPROPS   QueueProps;  
    QUEUEPROPID    aQueuePropId[ NUMBEROFPROPERTIES ];  
    MQPROPVARIANT  aQueuePropVar[ NUMBEROFPROPERTIES ];  
    HRESULT        aQueuePropStatus[ NUMBEROFPROPERTIES ];  
    HRESULT queryResultCode = MQ_OK;  
  
    // Specify the PROPID_Q_LABEL property.  
    aQueuePropId[ cPropId ] = PROPID_Q_QUOTA;       // Property ID  
    aQueuePropVar[ cPropId ].vt = VT_NULL;          // Type indicator  
    cPropId++;  
  
    // Initialize the MQQUEUEPROPS structure.  
    QueueProps.cProp = cPropId;  
    QueueProps.aPropID = aQueuePropId;  
    QueueProps.aPropVar = aQueuePropVar;  
    QueueProps.aStatus = aQueuePropStatus;  
 
    // Get the queue properties.  
    queryResultCode = ::MQGetQueueProperties( queueFormatName.c_str(), &QueueProps );  
    if ( FAILED( queryResultCode ) )  
    {  
        CORE::UInt32 errorCode =  HRESULT_CODE( queryResultCode );
        std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqQueueLabel: Failed to obtain queue quota for queue format name \"" + 
            CORE::ToString( queueFormatName ) + "\". HRESULT=" + CORE::ToString( queryResultCode ) + " Code Segment= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) ) ; 
        return false; 
    } 
    
    queueQuota = (UInt64) aQueuePropVar[ 0 ].ulVal;  
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
MsmqMetrics::GetMsmqQueueType( const std::wstring& queueFormatName ,
                               CLSID& queueTypeId                  )
{GUCEF_TRACE;

    // Validate the input string.  
    if ( queueFormatName.empty() ) 
        return false;  

    // Define the maximum number of queue properties and a property counter.  
    const int NUMBEROFPROPERTIES = 1;  
    DWORD cPropId=0;  
  
    // Define a queue property structure.  
    MQQUEUEPROPS   QueueProps;  
    QUEUEPROPID    aQueuePropId[ NUMBEROFPROPERTIES ];  
    MQPROPVARIANT  aQueuePropVar[ NUMBEROFPROPERTIES ];  
    HRESULT        aQueuePropStatus[ NUMBEROFPROPERTIES ];  
    HRESULT queryResultCode = MQ_OK;  
  
    // Specify the PROPID_Q_LABEL property.  
    aQueuePropId[ cPropId ] = PROPID_Q_TYPE;       // Property ID  
    aQueuePropVar[ cPropId ].vt = VT_CLSID;        // Type indicator  
    aQueuePropVar[cPropId].puuid = &queueTypeId;  
    cPropId++;  
  
    // Initialize the MQQUEUEPROPS structure.  
    QueueProps.cProp = cPropId;  
    QueueProps.aPropID = aQueuePropId;  
    QueueProps.aPropVar = aQueuePropVar;  
    QueueProps.aStatus = aQueuePropStatus;  
 
    // Get the queue properties.  
    queryResultCode = ::MQGetQueueProperties( queueFormatName.c_str(), &QueueProps );  
    if ( FAILED( queryResultCode ) )  
    {  
        CORE::UInt32 errorCode =  HRESULT_CODE( queryResultCode );
        std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqQueueType: Failed to obtain queue type id for queue format name \"" + 
            CORE::ToString( queueFormatName ) + "\". HRESULT= 0x" + CORE::Base16Encode( &queryResultCode, sizeof(queryResultCode) ) + " Code Segment= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) ) ; 
        return false; 
    } 
     
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
MsmqMetrics::GetMsmqQueueType( const std::wstring& queueFormatName ,
                               CORE::CString& queueTypeId          )
{GUCEF_TRACE;

    CLSID queueTypeIdStruct;
    memset( &queueTypeIdStruct, 0, sizeof queueTypeIdStruct );
    if ( GetMsmqQueueType( queueFormatName   ,
                           queueTypeIdStruct ) )
    {
        CORE::CAsciiString queueTypeIdStr;
        if ( MsmqGUIDToString( queueTypeIdStruct, queueTypeIdStr ) )
        {
            queueTypeId = CORE::ToString( queueTypeIdStr );
            return true;
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
MsmqMetrics::GetMsmqQueueLabel( const std::wstring& queueFormatName  ,
                                CORE::CString& queueLabel            )
{GUCEF_TRACE;

    // Validate the input string.  
    if ( queueFormatName.empty() ) 
        return false;  

    // Define the maximum number of queue properties and a property counter.  
    const int NUMBEROFPROPERTIES = 1;  
    DWORD cPropId=0;  
  
    // Define a queue property structure.  
    MQQUEUEPROPS   QueueProps;  
    QUEUEPROPID    aQueuePropId[ NUMBEROFPROPERTIES ];  
    MQPROPVARIANT  aQueuePropVar[ NUMBEROFPROPERTIES ];  
    HRESULT        aQueuePropStatus[ NUMBEROFPROPERTIES ];  
    HRESULT queryResultCode = MQ_OK;  
  
    // Specify the PROPID_Q_QUOTA property.  
    aQueuePropId[ cPropId ] = PROPID_Q_LABEL;         // Property ID  
    aQueuePropVar[ cPropId ].vt = VT_NULL;            // Type indicator  
    cPropId++;  
  
    // Initialize the MQQUEUEPROPS structure.  
    QueueProps.cProp = cPropId;  
    QueueProps.aPropID = aQueuePropId;  
    QueueProps.aPropVar = aQueuePropVar;  
    QueueProps.aStatus = aQueuePropStatus;  
 
    // Get the queue properties.  
    queryResultCode = ::MQGetQueueProperties( queueFormatName.c_str(), &QueueProps );  
    if ( FAILED( queryResultCode ) )  
    {  
        CORE::UInt32 errorCode =  HRESULT_CODE( queryResultCode );
        std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqQueueLabel: Failed to obtain queue label for queue format name \"" + 
            CORE::ToString( queueFormatName ) + "\". HRESULT=" + CORE::ToString( queryResultCode ) + " Code Segment= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) ) ; 
        return false; 
    }  
  
    if ( GUCEF_NULL != aQueuePropVar[ 0 ].pwszVal )
    {
        std::wstring queueLabelWs = aQueuePropVar[ 0 ].pwszVal;
        queueLabel = CORE::ToString( queueLabelWs );
        ::MQFreeMemory( aQueuePropVar[ 0 ].pwszVal );
        aQueuePropVar[ 0 ].pwszVal = GUCEF_NULL;
        return true;
    }
    else
    {
        queueLabel.Clear();
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
MsmqMetrics::GetMsmqQueuePathName( const CORE::CString& hostname       ,
                                   const std::wstring& queueFormatName ,
                                   CORE::CString& queuePathName        )
{GUCEF_TRACE;

    // Validate the input string.  
    if ( queueFormatName.empty() ) 
        return false;  

    // Define the maximum number of queue properties and a property counter.  
    const int NUMBEROFPROPERTIES = 1;  
    DWORD cPropId=0;  
    HRESULT aQueuePropStatus[ NUMBEROFPROPERTIES ];  
    HRESULT queryResultCode = MQ_OK;  

    if ( hostname.IsNULLOrEmpty() )
    {  
        // Define a queue property structure.  
        ::MQQUEUEPROPS   QueueProps;  
        ::QUEUEPROPID    aQueuePropId[ NUMBEROFPROPERTIES ];  
        ::MQPROPVARIANT  aQueuePropVar[ NUMBEROFPROPERTIES ];  

        // Specify the PROPID_Q_PATHNAME property.  
        aQueuePropId[ cPropId ] = PROPID_Q_PATHNAME;   // Property ID  
        aQueuePropVar[ cPropId ].vt = VT_NULL;         // Type indicator  
        cPropId++;  
  
        // Initialize the MQQUEUEPROPS structure.  
        QueueProps.cProp = cPropId;  
        QueueProps.aPropID = aQueuePropId;  
        QueueProps.aPropVar = aQueuePropVar;  
        QueueProps.aStatus = aQueuePropStatus;  
 
        // Get the queue properties.  
        queryResultCode = ::MQGetQueueProperties( queueFormatName.c_str(), &QueueProps );  
        if ( FAILED( queryResultCode ) )  
        {  
            CORE::UInt32 errorCode =  HRESULT_CODE( queryResultCode );
            std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqQueuePathName: Failed to obtain queue path name for queue format name \"" + 
                CORE::ToString( queueFormatName ) + "\". HRESULT=" + CORE::ToString( queryResultCode ) + " Code Segment= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) ) ; 
            return false; 
        }  
  
        if ( GUCEF_NULL != aQueuePropVar[ 0 ].pwszVal )
        {
            std::wstring queuePathNameWs = aQueuePropVar[ 0 ].pwszVal;
            queuePathName = CORE::ToString( queuePathNameWs );
            ::MQFreeMemory( aQueuePropVar[ 0 ].pwszVal );
            aQueuePropVar[ 0 ].pwszVal = GUCEF_NULL;
            return true;
        }
        else
        {
            queuePathName.Clear();
            return true;
        }
    }
    else
    {
        // Only the MQMgmtGetInfo interface supports obtaining a pathname for a specific host
        // Note thought that MQMgmtGetInfo only works for Active queues!
        // Inactive queues will just throw an error

        // Define an MQMGMTROPS structure.  
        ::MQMGMTPROPS mgmtprops;  
        ::MGMTPROPID aMgmtPropId[ NUMBEROFPROPERTIES ];  
        ::MQPROPVARIANT aMgmtPropVar[ NUMBEROFPROPERTIES ];  

        aMgmtPropId[ cPropId ] = PROPID_MGMT_QUEUE_PATHNAME;  // Property identifier  
        aMgmtPropVar[ cPropId ].vt = VT_NULL;                 // Type indicator  
        ++cPropId;

        // Initialize the MQMGMTPROPS structure.  
        mgmtprops.cProp = cPropId;          // number of management properties  
        mgmtprops.aPropID = aMgmtPropId;    // management property IDs  
        mgmtprops.aPropVar = aMgmtPropVar;  // management property values  
        mgmtprops.aStatus  = GUCEF_NULL;          // no storage for error codes   

        // Queue format names are not unique if you take outgoing queues into account
        // for an outgoing queue you also need the hostname as such we specify the hostname if we have it
        LPCWSTR computerNamePtr = GUCEF_NULL;
        std::wstring computerName;
        if ( !hostname.IsNULLOrEmpty() )
        {
            if ( CORE::Utf8toUtf16( CORE::ToUtf8String( hostname ), computerName ) )
                computerNamePtr = computerName.c_str();
        }        
    
        // Now that we formulated the request
        // actually ask for the info
        std::wstring queueInfoFormatName = L"QUEUE=" + queueFormatName;
        queryResultCode = ::MQMgmtGetInfo( computerNamePtr, queueInfoFormatName.c_str(), &mgmtprops );
        if ( MQ_OK == queryResultCode )
        {
            switch ( aMgmtPropVar[ 0 ].vt )
            {
                case VT_LPWSTR: 
                {
                    if ( GUCEF_NULL != aMgmtPropVar[ 0 ].pwszVal )
                    {
                        std::wstring queuePathNameWs = aMgmtPropVar[ 0 ].pwszVal;
                        queuePathName = CORE::ToString( queuePathNameWs );
                        ::MQFreeMemory( aMgmtPropVar[ 0 ].pwszVal );
                        aMgmtPropVar[ 0 ].pwszVal = GUCEF_NULL;
                        return true;
                    }
                    else
                    {
                        queuePathName.Clear();
                        return true;
                    }
                }
                default:
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqQueuePathName: MQMgmtGetInfo result vt is unsupported. Change the code! Type=" + CORE::ToString( aMgmtPropVar[ 0 ].vt ) );
                    return false;
                }
            }        
        }
        else
        {
            CORE::UInt32 errorCode =  HRESULT_CODE( queryResultCode );
            std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqQueuePathName: MQMgmtGetInfo Failed to obtain queue path name for queue format name \"" + 
                CORE::ToString( queueFormatName ) + "\". HRESULT=" + CORE::ToString( queryResultCode ) + " Code Segment= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) ) ; 
            return false;     
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
MsmqMetrics::GetMsmqQueuePathName( const std::wstring& queueFormatName  ,
                                   CORE::CString& queuePathName         )
{GUCEF_TRACE;

    return GetMsmqQueuePathName( CORE::CString::Empty, queueFormatName, queuePathName );
}

/*-------------------------------------------------------------------------*/

bool 
MsmqMetrics::GetMsmqQueuePathNameDNS( const std::wstring& queueFormatName  ,
                                      CORE::CString& queuePathNameDNS      )
{GUCEF_TRACE;

    // Validate the input string.  
    if ( queueFormatName.empty() ) 
        return false;  

    // Define the maximum number of queue properties and a property counter.  
    const int NUMBEROFPROPERTIES = 1;  
    DWORD cPropId=0;  
  
    // Define a queue property structure.  
    MQQUEUEPROPS   QueueProps;  
    QUEUEPROPID    aQueuePropId[ NUMBEROFPROPERTIES ];  
    MQPROPVARIANT  aQueuePropVar[ NUMBEROFPROPERTIES ];  
    HRESULT        aQueuePropStatus[ NUMBEROFPROPERTIES ];  
    HRESULT queryResultCode = MQ_OK;  
  
    // Specify the PROPID_Q_PATHNAME property.  
    aQueuePropId[ cPropId ] = PROPID_Q_PATHNAME_DNS;       // Property ID  
    aQueuePropVar[ cPropId ].vt = VT_NULL;             // Type indicator  
    cPropId++;   
  
    // Initialize the MQQUEUEPROPS structure.  
    QueueProps.cProp = cPropId;  
    QueueProps.aPropID = aQueuePropId;  
    QueueProps.aPropVar = aQueuePropVar;  
    QueueProps.aStatus = aQueuePropStatus;  
 
    // Get the queue properties.  
    queryResultCode = ::MQGetQueueProperties( queueFormatName.c_str(), &QueueProps );  
    if ( FAILED( queryResultCode ) )  
    {  
        CORE::UInt32 errorCode =  HRESULT_CODE( queryResultCode );
        std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqQueuePathNameDNS: Failed to obtain queue path name DNS for queue format name \"" + 
            CORE::ToString( queueFormatName ) + "\". HRESULT=" + CORE::ToString( queryResultCode ) + " Code Segment= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) ) ; 
        return false; 
    }  
  
    if ( GUCEF_NULL != aQueuePropVar[ 0 ].pwszVal )
    {
        std::wstring queuePathNameDNSWs = aQueuePropVar[ 0 ].pwszVal;
        queuePathNameDNS = CORE::ToString( queuePathNameDNSWs );
        ::MQFreeMemory( aQueuePropVar[ 0 ].pwszVal );
        aQueuePropVar[ 0 ].pwszVal = GUCEF_NULL;
        return true;
    }
    else
    {
        queuePathNameDNS.Clear();
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
MsmqMetrics::GetMsmqQueueProperties( const std::wstring& queueFormatName  ,
                                     MsmqQueueProperties& queueProperties )
{GUCEF_TRACE;

    // Validate the input string.  
    if ( queueFormatName.empty() ) 
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqQueueProperties: No queue format name was provided" ); 
        return false; 
    }

    // We query for each of the properties independently since we have no clue what type of queue we are 
    // actually dealing with. As such we best effort try to obtain properties, getting whatever we can get
    bool totalSuccess = true;
    totalSuccess = GetMsmqQueueLabel( queueFormatName, queueProperties.queueLabel ) && totalSuccess;
    totalSuccess = GetMsmqQueueQuota( queueFormatName, queueProperties.quota ) && totalSuccess;
    totalSuccess = GetMsmqQueuePathName( queueProperties.hostname, queueFormatName, queueProperties.pathName ) && totalSuccess;
    totalSuccess = GetMsmqQueuePathNameDNS( queueFormatName, queueProperties.pathNameDNS ) && totalSuccess;
    totalSuccess = GetMsmqQueueType( queueFormatName, queueProperties.typeId ) && totalSuccess;
    totalSuccess = GetQueueOwner( CORE::ToString( queueFormatName ), queueProperties.ownerDomainName, queueProperties.ownerAccountName, queueProperties.ownerSID, queueProperties.ownerIsDefaulted ) && totalSuccess;
    totalSuccess = GetMsmqPermissionList( CORE::ToString( queueFormatName ), queueProperties.queuePermissions ) && totalSuccess;

    if ( !queueProperties.ownerSID.IsNULLOrEmpty() )
    {
        TSIDStrToAccessMaskMap::iterator i = queueProperties.queuePermissions.find( queueProperties.ownerSID );
        if ( i != queueProperties.queuePermissions.end() )
        {
            queueProperties.ownerAccessMask = (*i).second;
            queueProperties.queuePermissions.erase( i ); // no need to store this redundantly since we call out the owner explicitly
        }
    }    
    
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::MsmqPathNameToMsmqQueueFormatName( const std::wstring& pathName   ,
                                                std::wstring& queueFormatName  )
{GUCEF_TRACE;

    DWORD maxFormatNameSize = 512;
    WCHAR queueFormatNameTmp[ 512 ]; 
    HRESULT formatConversionResult = ::MQPathNameToFormatName( pathName.c_str()    ,    
                                                               queueFormatNameTmp  ,    
                                                               &maxFormatNameSize  );
    if ( !FAILED( formatConversionResult ) )
    {
        queueFormatName = queueFormatNameTmp;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:MsmqPathNameToMsmqQueueFormatName: Converted pathName \"" + 
            CORE::ToString( pathName ) + "\" to \"" + CORE::ToString( queueFormatName ) + "\"" ) ; 
        return true;
    }
    else
    {
        CORE::UInt32 errorCode =  HRESULT_CODE( formatConversionResult );
        std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:MsmqPathNameToMsmqQueueFormatName: Failed to convert path \"" + 
            CORE::ToString( pathName ) + "\" to MSMQ format name. HRESULT=" + CORE::ToString( formatConversionResult ) + " Code Segment= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) ) ; 
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::MsmqPathNameToMsmqQueueFormatName( const CORE::CString& pathName   ,
                                                CORE::CString& queueFormatName  )
{GUCEF_TRACE;

    std::wstring wPathName;
    if ( CORE::Utf8toUtf16( CORE::ToUtf8String( pathName ), wPathName ) )
    {
        std::wstring wQueueFormatName;
        if ( MsmqPathNameToMsmqQueueFormatName( wPathName, wQueueFormatName ) )
        {
            queueFormatName = CORE::ToString( wQueueFormatName );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::MsmqQueueGUIDToMsmqQueueFormatName( const GUID& queueGuid          ,
                                                 CORE::CString& queueFormatName )
{GUCEF_TRACE;

    std::wstring wQueueFormatName;
    if ( MsmqQueueGUIDToMsmqQueueFormatName( queueGuid, wQueueFormatName ) )
    {
        queueFormatName = CORE::ToString( wQueueFormatName );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::MsmqQueueGUIDToMsmqQueueFormatName( const CORE::CString& queueGuid ,
                                                 CORE::CString& queueFormatName )
{GUCEF_TRACE;

    std::wstring wQueueFormatName;
    if ( MsmqQueueGUIDToMsmqQueueFormatName( queueGuid, wQueueFormatName ) )
    {
        queueFormatName = CORE::ToString( wQueueFormatName );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::MsmqQueueGUIDToMsmqQueueFormatName( const GUID& queueGuid          ,
                                                 std::wstring& queueFormatName  )
{GUCEF_TRACE;

    DWORD maxFormatNameSize = 512;
    WCHAR queueFormatNameTmp[ 512 ]; 
    HRESULT formatConversionResult = ::MQInstanceToFormatName( const_cast< GUID* >( &queueGuid ) ,    
                                                               queueFormatNameTmp                ,    
                                                               &maxFormatNameSize                );
    if ( !FAILED( formatConversionResult ) )
    {
        queueFormatName = queueFormatNameTmp;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:MsmqQueueGUIDToMsmqQueueFormatName: Converted GUID structure to \"" + 
            CORE::ToString( queueFormatName ) + "\"" ) ; 
        return true;
    }
    else
    {
        CORE::UInt32 errorCode =  HRESULT_CODE( formatConversionResult );
        std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:MsmqQueueGUIDToMsmqQueueFormatName: Failed to convert GUID to MSMQ format name. HRESULT=" + 
            CORE::ToString( formatConversionResult ) + " Code Segment= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) ) ; 
        return false;
    }
    
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::MsmqQueueGUIDToMsmqQueueFormatName( const CORE::CString& queueGuid ,
                                                 std::wstring& queueFormatName  )
{GUCEF_TRACE;

    CORE::CAsciiString asciiGuidStr = queueGuid.ForceToAscii( 0 );
    if ( asciiGuidStr.DetermineLength() != queueGuid.Length() )
    {
        CORE::CAsciiString asciiGuidStrLogExample = queueGuid.ForceToAscii( '*' );
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:MsmqQueueGUIDToMsmqQueueFormatName: Code point count difference while converting queueGuid \"" + 
            queueGuid + "\" to ASCII representation. Replacing non-ASCII with '*' yields: " + asciiGuidStrLogExample ) ; 
    }

    GUID queueGuidStruct;
    if ( StringToMsmqGUID( asciiGuidStr, queueGuidStruct ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:MsmqQueueGUIDToMsmqQueueFormatName: Converted queueGuid string \"" + 
            queueGuid + "\" to a GUID structure successfully" ); 

        return MsmqQueueGUIDToMsmqQueueFormatName( queueGuidStruct, queueFormatName );
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:MsmqQueueGUIDToMsmqQueueFormatName: Failed to convert queueGuid string \"" + 
        queueGuid + "\" to a GUID structure" ); 
    return false;
}

/*-------------------------------------------------------------------------*/

const std::wstring&
MsmqMetrics::GetMsmqQueueFormatName( MsmqQueue& q )
{
    if ( !q.msmqQueueFormatName.empty() )
        return q.msmqQueueFormatName;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqQueueFormatName: MSMQ queue 'format name' is not yet set, attempting to obtain it. Queue=" + q.queueName ) ; 

    std::wstring wQueueName;
    CORE::Utf8toUtf16( q.queueName, wQueueName );

    // Should we try and take the string as given and not do anything fancy?
    if ( !q.queueNameIsMsmqFormatName )  
    {
        bool isSuspectedQueueGuid = ( 0 == q.queueName.HasChar( '{', true ) && 0 < q.queueName.HasChar( '}', true ) );
        if ( isSuspectedQueueGuid )
        {
            std::wstring wQueueFormatName;
            if ( MsmqQueueGUIDToMsmqQueueFormatName( q.queueName, wQueueFormatName ) )
            {
                q.msmqQueueFormatName = wQueueFormatName;
            }
        }
        else
        {
            std::wstring wQueueFormatName;
            if ( MsmqPathNameToMsmqQueueFormatName( wQueueName, wQueueFormatName ) )
            {
                q.msmqQueueFormatName = wQueueFormatName;
            }
        }
    }
    else
    {
        q.msmqQueueFormatName = wQueueName;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqQueueFormatName: Since the topic name is flagged as a format name we will use it as-is. format name aka Queue=" + q.queueName );
    }

    return q.msmqQueueFormatName; 
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::GetMsmqActiveQueues( const CORE::CString& hostname          ,
                                  CORE::CString::StringSet& activeQueues )
{GUCEF_TRACE;

    std::wstring wHostname;
    if ( !CORE::Utf8toUtf16( CORE::ToUtf8String( hostname ), wHostname ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqActiveQueues: Failed to convert hostname to UTF16" );
        return false;
    }

    // Define the required constants and variables.  
    const int NUMBEROFPROPERTIES = 1;  
    DWORD cPropId = 0;  

    // Define an MQMGMTROPS structure.  
    ::MQMGMTPROPS mgmtprops;  
    ::MGMTPROPID aMgmtPropId[ NUMBEROFPROPERTIES ];  
    ::MQPROPVARIANT aMgmtPropVar[ NUMBEROFPROPERTIES ];  

    aMgmtPropId[ cPropId ] = PROPID_MGMT_MSMQ_ACTIVEQUEUES;  // Property identifier  
    aMgmtPropVar[ cPropId ].vt = VT_NULL;                    // Type indicator  
    ++cPropId;

    // Initialize the MQMGMTPROPS structure.  
    mgmtprops.cProp = cPropId;   // number of management properties  
    mgmtprops.aPropID = aMgmtPropId;// management property IDs  
    mgmtprops.aPropVar = aMgmtPropVar;// management property values  
    mgmtprops.aStatus  = GUCEF_NULL;// no storage for error codes   

    // Now that we formulated the request
    // actually ask for the info
    HRESULT queueInfoFetchResult = ::MQMgmtGetInfo( wHostname.c_str(), L"MACHINE", &mgmtprops );
    if ( MQ_OK == queueInfoFetchResult )
    {
        UInt64 nrOfActiveQueues = (UInt64) aMgmtPropVar[ 0 ].calpwstr.cElems;
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqActiveQueues: MQMgmtGetInfo returned " + CORE::ToString( nrOfActiveQueues ) + " number of active queues for hostname " + hostname );
        
        for ( UInt64 i=0; i<nrOfActiveQueues; ++i )
        {
            std::wstring wActiveQueueFormatName = aMgmtPropVar[ 0 ].calpwstr.pElems[ i ];
            ::MQFreeMemory( aMgmtPropVar[ 0 ].calpwstr.pElems[ i ] );

            /*  Per MSMQ docs:
             *      The outgoing queues included in the list of active queues on a specific computer returned in PROPID_MGMT_MSMQ_ACTIVEQUEUES 
             *      can be identified by obtaining the value of PROPID_MGMT_QUEUE_LOCATION for each queue in the list. Because the format name of 
             *      an outgoing queue is a format name of the corresponding destination queue, the value of PROPID_MGMT_QUEUE_LOCATION returned 
             *      for an active outgoing queue residing on the computer specified in the call to MQMgmtGetInfo will refer to the destination queue 
             *      residing on a different computer and will be MGMT_QUEUE_REMOTE_LOCATION.
             *
             *  This means we that the 'activeQueues' are only meaningfull for outgoing queues when provided the context of the hostname 
             *  since without it you would only know about the destination queue itself
             */ 
            CORE::CString activeQueueFormatName = CORE::ToString( wActiveQueueFormatName );
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqActiveQueues: host " + hostname + " has active queue " + activeQueueFormatName );
            activeQueues.insert( activeQueueFormatName );
        }
        ::MQFreeMemory( aMgmtPropVar[ 0 ].calpwstr.pElems ); 

        return true;
    }

    CORE::UInt32 errorCode =  HRESULT_CODE( queueInfoFetchResult );
    std::wstring errMsg = RetrieveWin32APIErrorMessage( HRESULT_CODE( queueInfoFetchResult ) );
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqActiveQueues: Failed to obtain list of active queues for hostname " + hostname + 
            ". HRESULT= 0x" + CORE::Base16Encode( &queueInfoFetchResult, sizeof(queueInfoFetchResult) ) + ". errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
MsmqMetrics::UpdateMsmqActiveQueueStatus( const CORE::CString::StringSet& hostnames ,
                                          MsmqQueueVector& queues                   ,
                                          bool addNewlyDiscoveredQueues             )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:UpdateMsmqActiveQueueStatus: Will attempt to update/refresh the isActive flag for " + 
            CORE::ToString( queues.size() ) + " queues using a list of " + CORE::ToString( hostnames.size() ) + " hostnames" );

    bool totalSuccess = true;
    
    CORE::CString::StringSet::const_iterator i = hostnames.begin();
    while ( i != hostnames.end() )
    {
        const CORE::CString& currentHostname = (*i);

        CORE::CString::StringSet activeQueuesOnHost;
        if ( GetMsmqActiveQueues( currentHostname    ,
                                  activeQueuesOnHost ) )
        {
            MsmqQueueVector::iterator m  = queues.begin();
            while ( m != queues.end() )
            {
                MsmqQueue& q = (*m);
                if ( !q.queueProperties.hostname.IsNULLOrEmpty() &&
                      q.queueProperties.hostname == currentHostname )
                {
                    // Active queues are listed as format names so we check against the Q format name
                    CORE::CString::StringSet::iterator a = activeQueuesOnHost.find( CORE::ToString( q.msmqQueueFormatName ) );
                    bool queueIsActive = false;
                    if ( a != activeQueuesOnHost.end() )
                    {
                        queueIsActive = true;
                        activeQueuesOnHost.erase( a ); // erase so we can check for a remnant below   
                    }
                    
                    if ( q.isActive != queueIsActive )
                    {
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:UpdateMsmqActiveQueueStatus: Determined that Queue " + 
                                CORE::ToString( q.msmqQueueFormatName ) + " now has an isActive status of \'" + CORE::ToString( queueIsActive ) + "\' on host " + currentHostname );
                    }                
                    else
                    {
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:UpdateMsmqActiveQueueStatus: Determined that Queue " + 
                                CORE::ToString( q.msmqQueueFormatName ) + " still has an isActive status of \'" + CORE::ToString( queueIsActive ) + "\' on host " + currentHostname );
                    }
                    q.isActive = queueIsActive;
                }                
                ++m;
            }

            // If we have any active queues left these could be outgoing queues which AD does not provide
            if ( !activeQueuesOnHost.empty() )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:UpdateMsmqActiveQueueStatus: Found " + CORE::ToString( activeQueuesOnHost.size() ) + 
                        " active queues on the host which are not in our queue list" );

                if ( addNewlyDiscoveredQueues )
                {
                    CORE::CString::StringSet::iterator a = activeQueuesOnHost.begin();
                    while ( a != activeQueuesOnHost.end() )
                    {
                        MsmqQueue q( (*a), true, currentHostname );
                        InitQueueInfo( q );
                        q.isActive = true;

                        queues.push_back( q );
                        
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:UpdateMsmqActiveQueueStatus: Auto added newly discovered active queue " + q.queueName + 
                                " on host " + q.queueProperties.hostname );

                        ++a;
                    }
                }
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:UpdateMsmqActiveQueueStatus: Failed to obtain list of active queues for hostname " + 
                    currentHostname + " as such we will not be able to update/refresh the isActive flag for queues on that host" );
            totalSuccess = false;
        }
        ++i;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

CORE::Int64
MsmqMetrics::GetQueueMetric( MsmqQueue& q                           , 
                             const CORE::CString& metricDescription ,
                             UInt32 propId                          ,
                             UInt32 propType                        ) const
{GUCEF_TRACE;

    // MQMgmtGetInfo only supports a format name
    // As such depending on how the setting is presented we need to translate it
    const std::wstring& queueFormatName = GetMsmqQueueFormatName( q );
    if ( queueFormatName.empty() )
        return -1;

    // Define the required constants and variables.  
    const int NUMBEROFPROPERTIES = 1;  
    DWORD cPropId = 0;  

    // Define an MQMGMTROPS structure.  
    ::MQMGMTPROPS mgmtprops;  
    ::MGMTPROPID aMgmtPropId[ NUMBEROFPROPERTIES ];  
    ::MQPROPVARIANT aMgmtPropVar[ NUMBEROFPROPERTIES ];  

    aMgmtPropId[ cPropId ] = (PROPID) propId;              // Property identifier  
    aMgmtPropVar[ cPropId ].vt = (VARTYPE) propType;       // Type indicator  
    ++cPropId;

    // Initialize the MQMGMTPROPS structure.  
    mgmtprops.cProp = cPropId;   // number of management properties  
    mgmtprops.aPropID = aMgmtPropId;// management property IDs  
    mgmtprops.aPropVar = aMgmtPropVar;// management property values  
    mgmtprops.aStatus  = GUCEF_NULL;// no storage for error codes   

    // Queue format names are not unique if you take outgoing queues into account
    // for an outgoing queue you also need the hostname as such we specify the hostname if we have it
    LPCWSTR computerNamePtr = GUCEF_NULL;
    std::wstring computerName;
    if ( !q.queueProperties.hostname.IsNULLOrEmpty() )
    {
        if ( CORE::Utf8toUtf16( CORE::ToUtf8String( q.queueProperties.hostname ), computerName ) )
            computerNamePtr = computerName.c_str();
    }        
    
    // Now that we formulated the request
    // actually ask for the info
    std::wstring queueInfoFormatName = L"QUEUE=" + queueFormatName;
    HRESULT queueInfoFetchResult = ::MQMgmtGetInfo( computerNamePtr, queueInfoFormatName.c_str(), &mgmtprops );
    if ( MQ_OK == queueInfoFetchResult )
    {
        switch ( propType )
        {
            case VT_UI4: return (CORE::Int64) aMgmtPropVar[ 0 ].ulVal;
            case VT_UI8: return (CORE::Int64) aMgmtPropVar[ 0 ].uhVal.QuadPart;

            default:
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetQueueMetric: PropType is unsupported. Change the code! Type=" + CORE::ToString( propType ) );
                return 0;
            }
        }
        
    }

    CORE::UInt32 errorCode =  HRESULT_CODE( queueInfoFetchResult );
    std::wstring errMsg = RetrieveWin32APIErrorMessage( HRESULT_CODE( queueInfoFetchResult ) );
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetQueueMetric: Failed to obtain " + metricDescription + ". Queue Name: " + q.queueName + 
            " HRESULT= 0x" + CORE::Base16Encode( &queueInfoFetchResult, sizeof(queueInfoFetchResult) ) + ". errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );
    return -1;
}

/*-------------------------------------------------------------------------*/

CORE::UInt64
MsmqMetrics::GetMsmqComputerMetric( const CORE::CString& metricDescription ,
                                    UInt32 propId                          ,
                                    UInt32 propType                        ) const
{GUCEF_TRACE;

    // Define the required constants and variables.  
    const int NUMBEROFPROPERTIES = 1;  
    DWORD cPropId = 0;  

    // Define an MQMGMTROPS structure.  
    ::MQMGMTPROPS mgmtprops;  
    ::MGMTPROPID aMgmtPropId[ NUMBEROFPROPERTIES ];  
    ::MQPROPVARIANT aMgmtPropVar[ NUMBEROFPROPERTIES ];  

    aMgmtPropId[ cPropId ] = (PROPID) propId;              // Property identifier  
    aMgmtPropVar[ cPropId ].vt = (VARTYPE) propType;       // Type indicator  
    ++cPropId;

    // Initialize the MQMGMTPROPS structure.  
    mgmtprops.cProp = cPropId;   // number of management properties  
    mgmtprops.aPropID = aMgmtPropId;// management property IDs  
    mgmtprops.aPropVar = aMgmtPropVar;// management property values  
    mgmtprops.aStatus  = GUCEF_NULL;// no storage for error codes   

    // Now that we formulated the request
    // actually ask for the info
    HRESULT computerInfoFetchResult = ::MQMgmtGetInfo( GUCEF_NULL, L"MACHINE", &mgmtprops );
    if ( MQ_OK == computerInfoFetchResult )
    {
        switch ( propType )
        {
            case VT_UI4: return (CORE::Int64) aMgmtPropVar[ 0 ].ulVal;
            case VT_UI8: return (CORE::Int64) aMgmtPropVar[ 0 ].uhVal.QuadPart;

            default:
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqComputerMetric: PropType is unsupported. Change the code! Type=" + CORE::ToString( propType ) );
                return 0;
            }
        }
        
    }

    CORE::UInt32 errorCode =  HRESULT_CODE( computerInfoFetchResult );
    std::wstring errMsg = RetrieveWin32APIErrorMessage( HRESULT_CODE( computerInfoFetchResult ) );
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqComputerMetric: Failed to obtain " + metricDescription + 
            " HRESULT= 0x" + CORE::Base16Encode( &computerInfoFetchResult, sizeof(computerInfoFetchResult) ) + ". errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );
    return 0;
}

/*-------------------------------------------------------------------------*/

CORE::Int64
MsmqMetrics::GetCurrentNrOfMessagesInQueue( MsmqQueue& q ) const
{GUCEF_TRACE;

    return GetQueueMetric( q, "count of queued messages", PROPID_MGMT_QUEUE_MESSAGE_COUNT, VT_UI4 );
}

/*-------------------------------------------------------------------------*/

CORE::Int64
MsmqMetrics::GetCurrentByteCountOfMesssagesInQueue( MsmqQueue& q ) const
{GUCEF_TRACE;

    // For MSMQ 3.0 and above:
    #if ( _WIN32_WINNT >= 0x0501 )
    return GetQueueMetric( q, "byte count of messages", PROPID_MGMT_QUEUE_BYTES_IN_QUEUE, VT_UI4 );
    #else
    return -1;
    #endif
}

/*-------------------------------------------------------------------------*/

CORE::Int64
MsmqMetrics::GetCurrentNrOfMessagesInQueueJournal( MsmqQueue& q ) const
{GUCEF_TRACE;

    // For MSMQ 3.0 and above:
    #if ( _WIN32_WINNT >= 0x0501 )
    return GetQueueMetric( q, "count of messages in journal", PROPID_MGMT_QUEUE_JOURNAL_MESSAGE_COUNT, VT_UI4 );
    #else
    return -1;
    #endif
}

/*-------------------------------------------------------------------------*/

CORE::Int64
MsmqMetrics::GetCurrentByteCountOfMesssagesInQueueJournal( MsmqQueue& q ) const
{GUCEF_TRACE;

    // For MSMQ 3.0 and above:
    #if ( _WIN32_WINNT >= 0x0501 )
    return GetQueueMetric( q, "byte count of messages in journal", PROPID_MGMT_QUEUE_BYTES_IN_JOURNAL, VT_UI4 );
    #else
    return -1;
    #endif
}

/*-------------------------------------------------------------------------*/

CORE::UInt64
MsmqMetrics::GetComputerGlobalTotalBytesOfAllMessagesOfAllQueues( void ) const
{GUCEF_TRACE;

    // For MSMQ 3.0 and above:
    #if ( _WIN32_WINNT >= 0x0501 )
    return (CORE::UInt64) GetMsmqComputerMetric( "global byte count of messages", PROPID_MGMT_MSMQ_BYTES_IN_ALL_QUEUES, VT_UI8 );
    #else
    return 0;
    #endif
}

/*-------------------------------------------------------------------------*/

CORE::CString
MsmqMetrics::GenerateMetricsFriendlyQueueName( const CORE::CString& queueName )
{GUCEF_TRACE;

    // Let's avoid non-ASCII stumbling blocks and force the down to ASCII
    CORE::CAsciiString asciiMetricsFriendlyQueueName = queueName.ForceToAscii( '_' );
    
    // Replace special chars
    static const char disallowedChars[] = { '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '=', '+', ',', '<', '>', '/', '?', '`', '~', '\\', '|', '{', '}', '[', ']', ';', ':', '\'', '\"' };
    asciiMetricsFriendlyQueueName = asciiMetricsFriendlyQueueName.ReplaceChars( disallowedChars, sizeof(disallowedChars)/sizeof(char), '_' );

    // Back to the platform wide string convention format
    CORE::CString metricsFriendlyQueueName = CORE::ToString( asciiMetricsFriendlyQueueName );
    return metricsFriendlyQueueName;
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::InitQueueInfo( MsmqQueue& q )
{GUCEF_TRACE;

    q.msmqQueueFormatName = GetMsmqQueueFormatName( q );
    q.metricFriendlyQueueName = GenerateMetricsFriendlyQueueName( q.queueName );    
    if ( !q.msmqQueueFormatName.empty() )
    {
        GetMsmqQueueProperties( q.msmqQueueFormatName, q.queueProperties );

        if ( !q.queueProperties.pathName.IsNULLOrEmpty() )
        {
            q.metricFriendlyQueueName = GenerateMetricsFriendlyQueueName( q.queueProperties.pathName.ReplaceChar( '\\', '.' ) );
            GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "MsmqMetrics: Switched metrics friendly queue name for \"" + q.queueName + "\" to \"" + q.queueProperties.pathName + "\" as \"" + q.metricFriendlyQueueName + "\" for human readability and tracing" );
        }
    }

    // Handle the hostname resolution
    // note that for outgoing queues this sadly resolves to the hostname of the destination machine
    // as such we need to retain the hostname that was already set on the queue if we have it
    if ( !q.queueProperties.pathName.IsNULLOrEmpty() )
    {
        CORE::CString hostnamePortionLc = q.queueProperties.pathName.SubstrToChar( '\\', true, true );
        if ( !hostnamePortionLc.IsNULLOrEmpty() )
        {
            // Did we have a predefined (pre-init) hostname?
            if ( !q.queueProperties.hostname.IsNULLOrEmpty() )
            {
                CORE::CString hostnamePortion = hostnamePortionLc.Uppercase();
                if ( q.queueProperties.hostname != hostnamePortion )  // we always keep hostnames everywhere as uppercase
                {                       
                    CORE::CString overrideHostname = q.queueProperties.hostname.Lowercase();

                    CORE::CString oldPathName = q.queueProperties.pathName;
                    CORE::CString oldPathNameDNS = q.queueProperties.pathNameDNS;
                    CORE::CString oldMetricFriendlyQueueName = q.metricFriendlyQueueName;

                    q.queueProperties.pathName = oldPathName.ReplaceSubstr( hostnamePortionLc, overrideHostname );
                    q.queueProperties.pathNameDNS = oldPathNameDNS.ReplaceSubstr( hostnamePortionLc, overrideHostname );
                    q.metricFriendlyQueueName = GenerateMetricsFriendlyQueueName( q.queueProperties.pathName.ReplaceChar( '\\', '.' ) );
                    q.isOutgoingQueue = true;

                    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "MsmqMetrics: queue \"" + q.queueName + "\" has a pathname of \"" + oldPathName + 
                            "\" pointing thus at host \"" + hostnamePortion + "\" however the queue was already defined to exist on host " + q.queueProperties.hostname +
                            " thus this is actually referencing a remote queue. Adjusted accordingly. PathName Old=" + oldPathName + " New=" + q.queueProperties.pathName +
                            " PathNameDNS Old=" + oldPathNameDNS + " New=" + q.queueProperties.pathNameDNS + 
                            " MetricFriendlyQueueName Old=" + oldMetricFriendlyQueueName + " New=" + q.metricFriendlyQueueName );                    
                }
            }
            else
            {
                // Since no hostname was set yet we just set the property and are done
                q.queueProperties.hostname = hostnamePortionLc.Uppercase();
            }
        }
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:InitQueueInfo: Obtained properties of MSMQ queue with name \"" + q.queueName + 
        "\" as follows: " + q.queueProperties.ToString() );

    return true; // best effort is fine
}

/*-------------------------------------------------------------------------*/

bool 
MsmqMetrics::GetPrivateQueues( const std::wstring& computerName ,
                               TWStringVector& queuePathNames   )
{GUCEF_TRACE;

    // For MSMQ 3.0 and above:
    #if ( _WIN32_WINNT >= 0x0501 )
  
    // Define the required constants and variables.  
    const int NUMBEROFPROPERTIES = 1;                  // Number of properties  
    DWORD cPropId = 0;                                 // Property counter  
    DWORD cQ = 0;                                      // Queue counter  
    HRESULT hr = MQ_OK;                                // Return code  
  
    // Define an MQMGMTROPS structure.  
    MQMGMTPROPS mgmtprops;  
    MGMTPROPID aMgmtPropId[ NUMBEROFPROPERTIES ];  
    MQPROPVARIANT aMgmtPropVar[ NUMBEROFPROPERTIES ];  
  
    // Specify PROPID_MGMT_MSMQ_PRIVATEQ as a property to be retrieved.  
    aMgmtPropId[cPropId] = PROPID_MGMT_MSMQ_PRIVATEQ;  // Property ID  
    aMgmtPropVar[cPropId].vt = VT_NULL;                // Type indicator  
    cPropId++;  
  
    // Initialize the MQMGMTPROPS structure.  
    mgmtprops.cProp = cPropId;                         // Number of management properties  
    mgmtprops.aPropID = aMgmtPropId;                   // IDs of the management properties  
    mgmtprops.aPropVar = aMgmtPropVar;                 // Values of management properties  
    mgmtprops.aStatus  = GUCEF_NULL;                         // No storage for error codes  
  
    // Call MQMgmtGetInfo to retrieve the information.  
    hr = ::MQMgmtGetInfo( computerName.c_str() ,     // Name of the computer  
                          MO_MACHINE_TOKEN     ,     // Object name  
                          &mgmtprops           );    // Management properties structure  
  
    if ( FAILED( hr ) )  
    {  
        CORE::UInt32 errorCode =  HRESULT_CODE( hr );
        std::wstring errMsg = RetrieveWin32APIErrorMessage( HRESULT_CODE( hr ) );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetPrivateQueues: Failed. HRESULT= 0x" + CORE::Base16Encode( &hr, sizeof(hr) ) + " errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );
        return false;  
    }  
  
    // Collect the information
    if ( aMgmtPropVar[ 0 ].calpwstr.cElems > 0 )  
    {  
        for ( cQ=0; cQ < aMgmtPropVar[ 0 ].calpwstr.cElems; cQ++)  
        {  
            queuePathNames.push_back( aMgmtPropVar[ 0 ].calpwstr.pElems[ cQ ] );
        }  
    }  
  
    // Free the memory allocated to store the path names.  
    for ( cQ=0; cQ < aMgmtPropVar[ 0 ].calpwstr.cElems; cQ++ )  
    {  
        ::MQFreeMemory( aMgmtPropVar[ 0 ].calpwstr.pElems[ cQ ] );  
    }  
    ::MQFreeMemory( aMgmtPropVar[ 0 ].calpwstr.pElems );  
  
    return true;  

    #else

    // Not supported in older MSMQ
    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool 
MsmqMetrics::GetPrivateQueues( const CORE::CString& computerName           ,
                               CORE::CString::StringVector& queuePathNames )
{GUCEF_TRACE;

    CORE::CUtf8String utf8ComputerName = CORE::ToUtf8String( computerName );
    std::wstring utf16ComputerName;
    if ( CORE::Utf8toUtf16( utf8ComputerName, utf16ComputerName ) )
    {
        TWStringVector wQueuePathNames;
        if ( GetPrivateQueues( utf16ComputerName ,
                               wQueuePathNames   ) )
        {
            TWStringVector::iterator i = wQueuePathNames.begin();
            while ( i != wQueuePathNames.end() )
            {
                std::string utf8QueuePathName;
                if ( CORE::Utf16toUtf8( (*i), utf8QueuePathName ) )
                {
                    queuePathNames.push_back( CORE::ToString( utf8QueuePathName ) );
                }
                ++i;
            }
            return true;
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
MsmqMetrics::GetLocalPrivateQueues( CORE::CString::StringVector& queuePathNames )
{GUCEF_TRACE;

    // Something of note:
    //      'The ComputerName and Host Name is limited to 15 characters by MSMQ. 
    //       If the Host Name contains more than 15 characters, MSMQ will truncate the name. 
    //       In this case, you must use the truncated Host Name.'    
    CORE::CString hostName = CORE::GetHostname().Uppercase();
    if ( hostName.Length() > 15 )
        hostName = hostName.CutChars( hostName.Length() - 15, false );

    return GetPrivateQueues( hostName, queuePathNames );
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::GetPublicQueues( CORE::CString::StringVector& queueIDs )
{GUCEF_TRACE;
  
    try
    {
        const int MAX_PROPERTIES = 13;   // 13 possible queue properties  
        bool totalSuccess = true;
    
        // Set a MQCOLUMNSET structure to specify the properties to be returned:           
        //      PROPID_Q_INSTANCE.  
        MQCOLUMNSET column;  
        PROPID aPropId[ 1 ];     // Nr of properties to retrieve  
        DWORD dwColumnCount = 0;  
  
        aPropId[dwColumnCount] = PROPID_Q_INSTANCE;  
        dwColumnCount++;  
  
        column.cCol = dwColumnCount;  
        column.aCol = aPropId;  
  
        // Call MQLocateBegin to start a Active Directory query.  
        // This won't work if you are not in an Active Directory domain
        HANDLE hEnum = GUCEF_NULL;  
        HRESULT hr = ::MQLocateBegin( GUCEF_NULL    ,   // Start search at the top  
                                      GUCEF_NULL    ,   // Search criteria  (NULL = return everything, no filters)
                                      &column ,   // Properties to return  
                                      GUCEF_NULL    ,   // No sort order  
                                      &hEnum  );  // Enumeration handle  
  
        if ( FAILED( hr ) )  
        {  
            if ( MQ_ERROR_UNSUPPORTED_OPERATION != hr )
            {
                CORE::UInt32 errorCode =  HRESULT_CODE( hr );
                std::wstring errMsg = RetrieveWin32APIErrorMessage( HRESULT_CODE( hr ) );
                totalSuccess = false;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetPublicQueues: MQLocateBegin Failed. HRESULT= 0x" + CORE::Base16Encode( &hr, sizeof(hr) ) + " errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );
                return false; 
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetPublicQueues: MQLocateBegin is not a supported operation. Likely this machine is in a workgroup not an AD domain which is a requirement for using public queues" );
            }
        
            // This operation is not supported for Message Queuing installed in workgroup mode.
            // As such the list we obtained, nothing, actually is the complete list of available public queues since there are none
            return true;
        }  
  
        // Call MQLocateNext in a loop to examine the  
        // query results.  
        MQPROPVARIANT aPropVar[ MAX_PROPERTIES ];  
        DWORD cProps, i;  
  
        do  
        {  
            cProps = MAX_PROPERTIES;  
            hr = ::MQLocateNext( hEnum    ,   // Handle returned by MQLocateBegin  
                                 &cProps  ,   // Size of aPropVar array  
                                 aPropVar );  // An array of MQPROPVARIANT for results  
                        
  
            if ( FAILED( hr ) )  
            {  
                CORE::UInt32 errorCode =  HRESULT_CODE( hr );
                std::wstring errMsg = RetrieveWin32APIErrorMessage( HRESULT_CODE( hr ) );
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetPublicQueues: MQLocateNext Failed. HRESULT= 0x" + CORE::Base16Encode( &hr, sizeof(hr) ) + " errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );
                totalSuccess = false;
                break;  
            }  
  
            for ( i=0; i < cProps; i += dwColumnCount )    
            {  
                if ( VT_CLSID == aPropVar[ i ].vt )
                {
                    CORE::CAsciiString queueGuid;
                    if ( MsmqGUIDToString( *aPropVar[ i ].puuid, queueGuid ) )
                    {
                        queueIDs.push_back( queueGuid );
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetPublicQueues: Found public queue with ID " + queueGuid );  
                    }
                }
            }  
  
        } 
        while ( cProps > 0 );  
  
        // Call MQLocateEnd to end query.  
        hr = ::MQLocateEnd( hEnum );   // Handle returned by MQLocateBegin.  
        if ( FAILED( hr ) )  
        {  
            CORE::UInt32 errorCode =  HRESULT_CODE( hr );
            std::wstring errMsg = RetrieveWin32APIErrorMessage( HRESULT_CODE( hr ) );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetPublicQueues: MQLocateEnd Failed. HRESULT= 0x" + CORE::Base16Encode( &hr, sizeof(hr) ) + " errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );
            totalSuccess = false;
        } 

        return totalSuccess;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "MsmqMetrics:GetPublicQueues: STL exception: " + CORE::ToString( e.what() ) );  
        return false;
    }
    catch ( ... )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "MsmqMetrics:GetPublicQueues: Unknown exception" );  
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::GetQueueSecurityDescriptor( const CORE::CString& formatName          ,
                                         SECURITY_INFORMATION infoTypeToGet       ,
                                         CORE::CDynamicBuffer& securityDescriptor )
{GUCEF_TRACE;

    std::wstring wFormatName;
    if ( CORE::Utf8toUtf16( CORE::ToUtf8String( formatName ), wFormatName ) )
    {
        return GetQueueSecurityDescriptor( wFormatName, infoTypeToGet, securityDescriptor );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::GetQueueSecurityDescriptor( const std::wstring& formatName           ,
                                         SECURITY_INFORMATION infoTypeToGet       ,
                                         CORE::CDynamicBuffer& securityDescriptor )
{GUCEF_TRACE;

    // We dont know the storage size needed so we make a test call first to get the required storage size
    securityDescriptor.SetBufferSize( 1 );
    securityDescriptor.SetBytes( 0 );
    DWORD dwBufferLengthNeeded = 1;
    HRESULT hr = ::MQGetQueueSecurity( formatName.c_str()                                       ,   
                                       DACL_SECURITY_INFORMATION                                ,  // Retrieving only the DACL  
                                       (PSECURITY_DESCRIPTOR) securityDescriptor.GetBufferPtr() ,  
                                       (DWORD) securityDescriptor.GetBufferSize()               ,              
                                       &dwBufferLengthNeeded                                    );  
    if ( SUCCEEDED( hr ) )  
    {  
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetQueueSecurityDescriptor: Successfully obtained ( " + CORE::ToString( (CORE::UInt32) dwBufferLengthNeeded ) + 
                " byte ) security descriptor in buffer for queue: " + CORE::ToString( formatName ) );
        return true;
    }  
    if ( hr == MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL )  
    {    
        // Allocate the memory needed for the security descriptor buffer.  
        if ( securityDescriptor.SetBufferSize( (CORE::UInt32) dwBufferLengthNeeded ) )
        {
            // Retrieve the DACL from the queue's security descriptor.  
            securityDescriptor.SetBytes( 0 );
            hr = ::MQGetQueueSecurity( formatName.c_str()                                       ,   
                                       DACL_SECURITY_INFORMATION                                ,  // Retrieving only the DACL  
                                       (PSECURITY_DESCRIPTOR) securityDescriptor.GetBufferPtr() ,  
                                       (DWORD) securityDescriptor.GetBufferSize()               ,              
                                       &dwBufferLengthNeeded                                    );
        
            if ( SUCCEEDED( hr ) )  
            {  
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetQueueSecurityDescriptor: Successfully obtained ( " + CORE::ToString( (CORE::UInt32) dwBufferLengthNeeded ) + 
                        " byte ) security descriptor in buffer for queue: " + CORE::ToString( formatName ) );
                return true;
            }        
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetQueueSecurityDescriptor: Memory ( " + CORE::ToString( (CORE::UInt32) dwBufferLengthNeeded ) + 
                    " bytes ) could not be allocated for the security descriptor buffer for queue: " + CORE::ToString( formatName ) );
            return false;
        }
    }

    CORE::UInt32 errorCode =  HRESULT_CODE( hr );
    std::wstring errMsg = RetrieveWin32APIErrorMessage( HRESULT_CODE( hr ) );
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetQueueSecurityDescriptor: Failed. HRESULT= 0x" + CORE::Base16Encode( &hr, sizeof(hr) ) + " errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );  
    return false;
}  

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::GetQueueSecurityDescriptorDACL( const CORE::CString& formatName          ,
                                             CORE::CDynamicBuffer& securityDescriptor )
{GUCEF_TRACE;

    return GetQueueSecurityDescriptor( formatName, DACL_SECURITY_INFORMATION, securityDescriptor );
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::GetAccountInfoForSid( const CORE::CString& accountSid ,
                                   CORE::CString& domainName       ,
                                   CORE::CString& accountName      )
{GUCEF_TRACE; 

    CORE::CUtf8String utf8AccountSid = CORE::ToUtf8String( accountSid );
    std::wstring utf16AccountSid;
    if ( CORE::Utf8toUtf16( utf8AccountSid, utf16AccountSid ) )
    {
        std::wstring wDomainName;
        std::wstring wAccountName;
        if ( GetAccountInfoForSid( utf16AccountSid ,
                                   wDomainName     ,
                                   wAccountName    ) )
        {
            domainName = CORE::ToString( wDomainName );
            accountName = CORE::ToString( wAccountName );
            return true;
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::GetAccountInfoForSid( const std::wstring& accountSid ,
                                   std::wstring& domainName       ,
                                   std::wstring& accountName      )
{GUCEF_TRACE;        

    if ( accountSid.empty() )
        return false;
    
    ::PSID pSID = GUCEF_NULL;
    if ( ::ConvertStringSidToSidW( accountSid.c_str(), &pSID ) == TRUE )
    {
        bool result = GetAccountInfoForSid( pSID, domainName, accountName );
        ::LocalFree( pSID );

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetAccountInfoForSid: For SID \"" + CORE::ToString( accountSid ) + 
                "\" Found domain=\"" + CORE::ToString( domainName ) + "\" Found account=\"" + CORE::ToString( accountName ) + "\"" );  
        return result;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::GetAccountInfoForSid( ::PSID accountSid         ,
                                   std::wstring& domainName  ,
                                   std::wstring& accountName )
{GUCEF_TRACE;
        
    // Set a size that may be sufficient for the account name and domain name buffers.  
    const DWORD INITIAL_SIZE = 256;  
    DWORD cchAccName = INITIAL_SIZE;  
    DWORD cchDomainName = INITIAL_SIZE;  

    CORE::CDynamicBuffer accountNameBuffer;
    CORE::CDynamicBuffer domainNameBuffer;

    // We dont know the storage size needed for the account and domain names so we start at a decent size and grow as needed
    accountNameBuffer.SetBufferSize( INITIAL_SIZE );
    accountNameBuffer.SetBytes( 0 );
    domainNameBuffer.SetBufferSize( INITIAL_SIZE );
    domainNameBuffer.SetBytes( 0 );
        
    ::SID_NAME_USE eSidType;
    DWORD dwBufferLengthNeeded = 1;
    if ( ::LookupAccountSidW( GUCEF_NULL                                      ,                                         
                              accountSid                                ,  
                              (LPWSTR) accountNameBuffer.GetBufferPtr() ,  
                              &cchAccName                               ,  
                              (LPWSTR) domainNameBuffer.GetBufferPtr()  , 
                              &cchDomainName                            ,  
                              &eSidType                                 ) == FALSE ) 
    {
        bool issueWasResizeNeeded = false;
        if ( cchAccName < INITIAL_SIZE )
        {
            if ( !accountNameBuffer.SetBufferSize( (CORE::UInt32) cchAccName ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetQueueOwner: LookupAccountSidW needs " 
                        + CORE::ToString( (CORE::UInt32) cchAccName ) + " bytes for account name which could not be allocated" );
                return false;
            }
            accountNameBuffer.SetBytes( 0 );
            issueWasResizeNeeded = true;
        }
        if ( cchDomainName < INITIAL_SIZE )
        {
            if ( !domainNameBuffer.SetBufferSize( (CORE::UInt32) cchDomainName ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetQueueOwner: LookupAccountSidW needs " 
                        + CORE::ToString( (CORE::UInt32) cchDomainName ) + " bytes for domain name which could not be allocated" );
                return false;
            }
            domainNameBuffer.SetBytes( 0 );
            issueWasResizeNeeded = true;
        }

        if ( !issueWasResizeNeeded )
        {
            CORE::UInt32 errorCode = ::GetLastError(); 
            std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetQueueOwner: LookupAccountSidW Failed. errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );              
            return false;
        }

        if ( ::LookupAccountSidW( GUCEF_NULL                                      ,                                         
                                  accountSid                                ,  
                                  (LPWSTR) accountNameBuffer.GetBufferPtr() ,  
                                  &cchAccName                               ,  
                                  (LPWSTR) domainNameBuffer.GetBufferPtr()  , 
                                  &cchDomainName                            ,  
                                  &eSidType                                 ) == FALSE ) 
        {
            // Failed again
            CORE::UInt32 errorCode = ::GetLastError(); 
            std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetQueueOwner: LookupAccountSidW Failed again. errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );              
            return false;
        }
    }
    accountNameBuffer.SetDataSize( (CORE::UInt32) cchAccName );
    domainNameBuffer.SetDataSize( (CORE::UInt32) cchDomainName  );

    if ( domainNameBuffer.GetDataSize() > 0 )
        domainName.assign( domainNameBuffer.AsConstTypePtr< wchar_t >(), cchDomainName );
    if ( accountNameBuffer.GetDataSize() > 0 )
        accountName.assign( accountNameBuffer.AsConstTypePtr< wchar_t >(), cchAccName );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::GetQueueOwner( const std::wstring& formatName ,
                            std::wstring& domainName       ,
                            std::wstring& accountName      ,
                            std::wstring& accountSid       ,
                            bool& isOwnerDefaulted         )
{GUCEF_TRACE;                          

    domainName.clear();
    accountName.clear();
    accountSid.clear();
    isOwnerDefaulted = true;

    CORE::CDynamicBuffer securityDescriptor;
    if ( GetQueueSecurityDescriptor( formatName, OWNER_SECURITY_INFORMATION, securityDescriptor ) )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetQueueOwner: Obtained OWNER_SECURITY_INFORMATION" );

        // Retrieve the owner's SID from the security descriptor buffer.
        PSID pSID = GUCEF_NULL;
        BOOL fOwnerDefaulted = FALSE;  
        if ( ::GetSecurityDescriptorOwner( (PSECURITY_DESCRIPTOR) securityDescriptor.GetBufferPtr() ,  
                                           &pSID                                                    ,  
                                           &fOwnerDefaulted                                         ) == FALSE )
        {  
            CORE::UInt32 errorCode =  ::GetLastError(); 
            std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetQueueOwner: GetSecurityDescriptorOwner Failed. errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );              
            return false;
        } 
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetQueueOwner: Obtained owner info reference into the security descriptor" );
   
        if ( pSID == GUCEF_NULL )  
        {  
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetQueueOwner: GetSecurityDescriptorOwner states that no owner information was found in the security descriptor" ); 
            return false;  
        }  

        bool totalSuccess = true;
        accountSid = CovertPSIDToWString( pSID );
        totalSuccess = !accountSid.empty() && totalSuccess;
        totalSuccess = GetAccountInfoForSid( pSID, domainName, accountName ) && totalSuccess;

        if ( fOwnerDefaulted == FALSE )  
        {  
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetQueueOwner: GetSecurityDescriptorOwner states that no owner exists" );  
        }
        isOwnerDefaulted = fOwnerDefaulted == FALSE;

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetQueueOwner: Found that accountName=\"" + CORE::ToString( accountName ) + 
                "\" and domainName=\"" + CORE::ToString( accountName ) + "\" and sid=\"" + CORE::ToString( accountSid ) + "\"" );              
        
        return totalSuccess;
    }
    
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetQueueOwner: GetSecurityDescriptor Failed" );
    return false;
} 

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::GetQueueOwner( const CORE::CString& formatName ,
                            CORE::CString& domainName       ,
                            CORE::CString& accountName      ,
                            CORE::CString& sid              ,
                            bool& isOwnerDefaulted          )
{GUCEF_TRACE;

    std::wstring wFormatName;
    if ( CORE::Utf8toUtf16( CORE::ToUtf8String( formatName ), wFormatName ) )
    {
        std::wstring wDomainName;
        std::wstring wAccountName;
        std::wstring wSid;
        if ( GetQueueOwner( wFormatName, wDomainName, wAccountName, wSid, isOwnerDefaulted ) )
        {
            domainName = CORE::ToString( wDomainName );
            accountName = CORE::ToString( wAccountName );
            sid = CORE::ToString( wSid ); 
            return true;
        }
    }
    return false;    
}

/*-------------------------------------------------------------------------*/

CORE::CString
MsmqMetrics::GetMsmqPermissionsAsString( ::ACCESS_MASK amMask )
{GUCEF_TRACE;
  
    CORE::CString result;

    if ((amMask & MQSEC_QUEUE_GENERIC_ALL) == MQSEC_QUEUE_GENERIC_ALL)  
    {  
        result += "Full Control,";  
    }    
    if ((amMask & MQSEC_DELETE_QUEUE) == MQSEC_DELETE_QUEUE)  
    {  
        result += "Delete Queue,"; 
    }    
    if ((amMask & MQSEC_RECEIVE_MESSAGE) == MQSEC_RECEIVE_MESSAGE)  
    {  
        result += "Receive Message,";
    }    
    if ((amMask & MQSEC_DELETE_MESSAGE) == MQSEC_DELETE_MESSAGE)  
    {  
        result += "Delete Message,";
    }    
    if ((amMask & MQSEC_PEEK_MESSAGE) == MQSEC_PEEK_MESSAGE)  
    {  
        result += "Peek Message,";
    }    
    if ((amMask & MQSEC_RECEIVE_JOURNAL_MESSAGE) == MQSEC_RECEIVE_JOURNAL_MESSAGE)  
    {  
        result += "Receive Journal Message,";
    }    
    if ((amMask & MQSEC_DELETE_JOURNAL_MESSAGE) == MQSEC_DELETE_JOURNAL_MESSAGE)  
    {  
        result += "Delete Journal Message,";
    }    
    if ((amMask & MQSEC_GET_QUEUE_PROPERTIES) == MQSEC_GET_QUEUE_PROPERTIES)  
    {  
        result += "Get Queue Properties,";
    }    
    if ((amMask & MQSEC_SET_QUEUE_PROPERTIES) == MQSEC_SET_QUEUE_PROPERTIES)  
    {  
        result += "Set Queue Properties,";
    }    
    if ((amMask & MQSEC_GET_QUEUE_PERMISSIONS) == MQSEC_GET_QUEUE_PERMISSIONS)  
    {  
        result += "Get Queue Permissions,";
    }    
    if ((amMask & MQSEC_CHANGE_QUEUE_PERMISSIONS) == MQSEC_CHANGE_QUEUE_PERMISSIONS)  
    {  
        result += "Set Queue Permissions,";
    }   
    if ((amMask & MQSEC_TAKE_QUEUE_OWNERSHIP) == MQSEC_TAKE_QUEUE_OWNERSHIP)  
    {  
        result += "Take Queue Ownership,";
    }    
    if ((amMask & MQSEC_WRITE_MESSAGE) == MQSEC_WRITE_MESSAGE)  
    {  
        result += "Write Message,";
    }  
  
    return result;  
}  

/*-------------------------------------------------------------------------*/

std::wstring
MsmqMetrics::CovertPSIDToWString( ::PSID psid )
{GUCEF_TRACE;

    LPWSTR sidStr = GUCEF_NULL;
    if ( ::ConvertSidToStringSidW( psid, &sidStr ) == TRUE )
    {
        std::wstring result = sidStr; 
        ::LocalFree( sidStr );
        return result;      
    }

    CORE::UInt32 errorCode =  ::GetLastError(); 
    std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:CovertPSIDToWString: ConvertSidToStringSidW Failed. errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );              
    return std::wstring();
}

/*-------------------------------------------------------------------------*/

CORE::CString 
MsmqMetrics::CovertPSIDToString( ::PSID psid )
{GUCEF_TRACE;

    LPWSTR sidStr = GUCEF_NULL;
    if ( ::ConvertSidToStringSidW( psid, &sidStr ) == TRUE )
    {
        CORE::CString result = CORE::ToString( CORE::CUtf8String( sidStr ) );      
        ::LocalFree( sidStr );
        return result;
    }

    CORE::UInt32 errorCode =  ::GetLastError(); 
    std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:CovertPSIDToString: ConvertSidToStringSidW Failed. errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );              
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/
    
bool 
MsmqMetrics::GetMsmqPermissionList( const CORE::CString& formatName    ,
                                    TSIDStrToAccessMaskMap& accessList )
{GUCEF_TRACE;

    CORE::CDynamicBuffer securityDescriptor;
    if ( GetQueueSecurityDescriptorDACL( formatName,
                                         securityDescriptor ) )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqPermissionList: Obtained DACL security descriptor for Queue " + formatName );
        
        TPSIDToAccessMaskMap pSidAccessList;
        if ( GetMsmqPermissionList( securityDescriptor ,
                                    pSidAccessList     ) ) 
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqPermissionList: Obtained Permissions list for Queue " + formatName );
            
            // Now convert the struct pointers inside the security descriptor to 
            // stuff that is easier to store, sort and use, a string and a dword
            TPSIDToAccessMaskMap::iterator i = pSidAccessList.begin();
            while ( i != pSidAccessList.end() )
            {
                accessList[ CovertPSIDToString( (*i).first ) ] = (*i).second;
                ++i;
            }
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/
    
bool 
MsmqMetrics::GetMsmqPermissionList( const CORE::CDynamicBuffer& securityDescriptor  ,
                                    TPSIDToAccessMaskMap& accessList                )
{GUCEF_TRACE;        
                
    // Retrieve a pointer to the DACL in the security descriptor.  
    PACL pDacl = GUCEF_NULL; 
    BOOL fDaclPresent = FALSE;  
    BOOL fDaclDefaulted = TRUE;  
    if ( ::GetSecurityDescriptorDacl( (PSECURITY_DESCRIPTOR) securityDescriptor.GetConstBufferPtr() ,  
                                      &fDaclPresent                                                 ,  
                                      &pDacl                                                        ,  
                                      &fDaclDefaulted                                               ) == FALSE )  
    {  
        CORE::UInt32 errorCode =  ::GetLastError(); 
        std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqPermissionList: GetQueueSecurityDescriptorDACL Failed. errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );              
        return false;
    }          
  
    // Check whether no DACL or a NULL DACL was retrieved from the security descriptor buffer.  
    if ( fDaclPresent == FALSE  )  
    {  
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqPermissionList: No DACL was found (all access is denied)" );  
        return true;  
    }
    if ( pDacl == GUCEF_NULL ) 
    {  
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqPermissionList: a NULL DACL (unrestricted access) was found" );  
        return true;  
    }

    // Retrieve the ACL_SIZE_INFORMATION structure to find the number of ACEs in the DACL.  
    ::ACL_SIZE_INFORMATION aclsizeinfo;  
    memset( &aclsizeinfo, 0, sizeof( aclsizeinfo ) );
    if ( ::GetAclInformation( pDacl               ,  
                              &aclsizeinfo        ,  
                              sizeof(aclsizeinfo) ,  
                              AclSizeInformation  ) == FALSE)  
    {   
        CORE::UInt32 errorCode =  ::GetLastError(); 
        std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqPermissionList: GetAclInformation Failed. errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );              
        return false;
    }
        
    // Loop through the ACEs and display the information.  
    ::ACCESS_ALLOWED_ACE * pAce = GUCEF_NULL;  
    for ( DWORD cAce = 0; cAce < aclsizeinfo.AceCount; cAce++ )  
    {    
        // Get ACE info  
        if ( ::GetAce( pDacl           ,  
                       cAce            ,  
                       (LPVOID*) &pAce ) == FALSE )  
        {  
            CORE::UInt32 errorCode =  ::GetLastError(); 
            std::wstring errMsg = RetrieveWin32APIErrorMessage( errorCode );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqPermissionList: GetAce Failed. errorCode= " + CORE::ToString( errorCode ) + ". Error msg: " + CORE::ToString( errMsg ) );              
            continue;
        }  

        // The ace struct can hold other info not just access lists
        // as such we need to check the struct type
        switch( pAce->Header.AceType )  
        {  
            case ACCESS_ALLOWED_ACE_TYPE:
            {
                // Permissions granted
                accessList[ (PSID) &pAce->SidStart ] = pAce->Mask;
                break;
            }

            case ACCESS_DENIED_ACE_TYPE:
            {
                // Permissions explicitly denied
                //     ??? accessList[ (PSID) &pAce->SidStart ] = pAce->Mask;
                // @TODO?
                std::wstring domainName;
                std::wstring accountName;
                GetAccountInfoForSid( (PSID) &pAce->SidStart, domainName, accountName );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqPermissionList: Access explicitly denied for domain=" + 
                                CORE::ToString( domainName ) + " account=" + CORE::ToString( accountName ) + " mask=" + CORE::ToString( (UInt64) pAce->Mask ) );
                break;
            }
  
            default:  
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:GetMsmqPermissionList: GetAce Unknown ACE type " + CORE::ToString( pAce->Header.AceType ) );
                break;
            }
        }  
    }
    
    return true;
}   

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::FindPrivateQueues( const CORE::CString::StringSet& hostNames          ,
                                const CORE::CString::StringSet& globPatternFilters ,
                                CORE::CString::StringSet& foundQueues              )
{GUCEF_TRACE;
    
    bool totalSuccess = true;
    
    CORE::CString::StringSet::iterator m = hostNames.begin();
    while ( m != hostNames.end() )
    {
        CORE::CString::StringVector privateQueuePathNames;
        if ( GetPrivateQueues( (*m), privateQueuePathNames ) )
        {
            CORE::CString::StringVector::iterator i = privateQueuePathNames.begin();
            while ( i != privateQueuePathNames.end() )
            {
                CORE::CString queueFormatName;
                if ( MsmqPathNameToMsmqQueueFormatName( (*i), queueFormatName ) )
                {
                    if ( globPatternFilters.empty() || queueFormatName.WildcardEquals( globPatternFilters, '*', true, true ) )
                    {
                        foundQueues.insert( queueFormatName );
                        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:FindAllQueues: Found private queue on host \"" + (*m) + "\" that passes the filter criterea : " + queueFormatName );
                    }
                }
                else
                    totalSuccess = false;

                ++i;
            }
        }
        else
            totalSuccess = false;
        ++m;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool 
MsmqMetrics::FindAllQueues( const CORE::CString::StringSet& globPatternFilters ,
                            CORE::CString::StringSet& foundQueues              )
{GUCEF_TRACE;

    // For discovery in the MSMQ realm we always use the format name in order to be able to collapse sources into a singular uniform list
    bool totalSuccess = true;

    // Let's first check the private local queues as that is most likely to succeed
    CORE::CString::StringVector localPrivateQueuePathNames;
    if ( GetLocalPrivateQueues( localPrivateQueuePathNames ) )
    {
        CORE::CString::StringVector::iterator i = localPrivateQueuePathNames.begin();
        while ( i != localPrivateQueuePathNames.end() )
        {
            CORE::CString queueFormatName;
            if ( MsmqPathNameToMsmqQueueFormatName( (*i), queueFormatName ) )
            {
                if ( globPatternFilters.empty() || queueFormatName.WildcardEquals( globPatternFilters, '*', true, true ) )
                {
                    foundQueues.insert( queueFormatName );
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:FindAllQueues: Found local private queue that passes the filter criterea : " + queueFormatName );
                }
            }
            else
                totalSuccess = false;

            ++i;
        }
    }
    else
        totalSuccess = false;

    // Next try our luck at any public queues we have access to
    CORE::CString::StringVector publicQueueIDs;
    if ( GetPublicQueues( publicQueueIDs ) )
    {
        CORE::CString::StringVector::iterator i = publicQueueIDs.begin();
        while ( i != publicQueueIDs.end() )
        {
            CORE::CString queueFormatName;
            if ( MsmqQueueGUIDToMsmqQueueFormatName( (*i), queueFormatName ) )
            {
                if ( globPatternFilters.empty() || queueFormatName.WildcardEquals( globPatternFilters, '*', true, true ) )
                {
                    foundQueues.insert( queueFormatName );
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics:FindAllQueues: Found public queue that passes the filter criterea : " + queueFormatName );
                }
            }
            else
                totalSuccess = false;

            ++i;
        }        
    }
    else
        totalSuccess = false;

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

void
MsmqMetrics::OnQueueActivityCheckTimerCycle( CORE::CNotifier* notifier    ,
                                             const CORE::CEvent& eventId  ,
                                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    UpdateMsmqActiveQueueStatus( m_hostnames, m_queues, m_discoverQueues );
}

/*-------------------------------------------------------------------------*/

void
MsmqMetrics::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // Now publish all the metrics to the metrics system
    MsmqQueueVector::iterator i = m_queues.begin();
    while ( i != m_queues.end() )
    {
        MsmqQueue& q = (*i);
        if ( q.collectMetrics )
        {
            CORE::CString metricsPrefix = m_metricsPrefix + q.metricFriendlyQueueName;

            GUCEF_METRIC_GAUGE( metricsPrefix + ".msmqQueueIsActive", q.isActive ? 1 : 0, 1.0f );
        
            if ( !q.isOutgoingQueue || m_gatherMetricsFromOutgoingQueues )
            {
                // You can only obtain metrics for 'active' queues in MSMQ
                if ( q.isActive )
                {
                    if ( m_gatherMsgsInQueueMetric )
                    {
                        // in order to get this metric you need elevated access to the queue
                        CORE::Int64 msmqMsgsInQueue = GetCurrentNrOfMessagesInQueue( q );
                        if ( msmqMsgsInQueue >= 0 )
                            GUCEF_METRIC_GAUGE( metricsPrefix + ".msmqMsgsInQueue", msmqMsgsInQueue, 1.0f );
                    }

                    if ( m_gatherMsgsTotalByteCountMetric )
                    {
                        // in order to get this metric you need elevated access to the queue
                        // You also need MSMQ version 3.0 or later
                        CORE::Int64 msmqMsgsTotalByteCount = GetCurrentByteCountOfMesssagesInQueue( q );
                        if ( msmqMsgsTotalByteCount >= 0 )
                            GUCEF_METRIC_GAUGE( metricsPrefix + ".msmqMsgsTotalByteCount", msmqMsgsTotalByteCount, 1.0f );
                    }

                    if ( m_gatherMsgsInQueueJournalMetric )
                    {
                        // in order to get this metric you need elevated access to the queue
                        // You also need MSMQ version 3.0 or later
                        CORE::Int64 msmqMsgsInQueueJournal = GetCurrentNrOfMessagesInQueueJournal( q );
                        if ( msmqMsgsInQueueJournal >= 0 )
                            GUCEF_METRIC_GAUGE( metricsPrefix + ".msmqMsgsInQueueJournal", msmqMsgsInQueueJournal, 1.0f );
                    }

                    if ( m_gatherMsgsInJournalTotalByteCountMetric )
                    {
                        // in order to get this metric you need elevated access to the queue
                        // You also need MSMQ version 3.0 or later
                        CORE::Int64 msmqMsgsInJournalTotalByteCount = GetCurrentByteCountOfMesssagesInQueueJournal( q );
                        if ( msmqMsgsInJournalTotalByteCount >= 0 )
                            GUCEF_METRIC_GAUGE( metricsPrefix + ".msmqMsgsInJournalTotalByteCount", msmqMsgsInJournalTotalByteCount, 1.0f );
                    }
                }
                else
                {
                    if ( !m_dontSendMetricsForInactiveQueues )
                    {
                        // Because we still want to send something we will send 0 for the stats as our sign of life
                        if ( m_gatherMsgsInQueueMetric )
                            GUCEF_METRIC_GAUGE( metricsPrefix + ".msmqMsgsInQueue", 0, 1.0f );

                        if ( m_gatherMsgsTotalByteCountMetric )
                            GUCEF_METRIC_GAUGE( metricsPrefix + ".msmqMsgsTotalByteCount", 0, 1.0f );

                        if ( m_gatherMsgsInQueueJournalMetric )
                            GUCEF_METRIC_GAUGE( metricsPrefix + ".msmqMsgsInQueueJournal", 0, 1.0f );

                        if ( m_gatherMsgsInJournalTotalByteCountMetric )
                            GUCEF_METRIC_GAUGE( metricsPrefix + ".msmqMsgsInJournalTotalByteCount", 0, 1.0f );
                    }
                }
            }
        }
        ++i;
    }

    if ( m_gatherTotalMsgBytesMetric )
    {
        // in order to get this metric you need elevated access to the queue
        // You also need MSMQ version 3.0 or later
        CORE::UInt64 msmqTotalMsgBytes = GetComputerGlobalTotalBytesOfAllMessagesOfAllQueues();
        GUCEF_METRIC_GAUGE( m_metricsPrefix + "msmqTotalMsgBytes", msmqTotalMsgBytes, 1.0f );
    }
}

/*-------------------------------------------------------------------------*/

void
MsmqMetrics::SetMetricCollectionBasedOnFilter( MsmqQueueVector& queues                      , 
                                               CORE::CString::StringSet& globPatternFilters )
{GUCEF_TRACE;
    
    if ( globPatternFilters.empty() )
        return;
    
    UInt32 metricEnabledQueues = 0;
    UInt32 metricDisabledQueues = 0;
    MsmqQueueVector::iterator i = queues.begin();
    while ( i != queues.end() )
    {
        MsmqQueue& queue = (*i);

        CORE::CString nameToTest;
        if ( !queue.queueProperties.pathName.IsNULLOrEmpty() )
            nameToTest = queue.queueProperties.pathName;    
        else
        if ( !queue.queueProperties.pathNameDNS.IsNULLOrEmpty() )
            nameToTest = queue.queueProperties.pathNameDNS;    
        else
        if ( !queue.queueProperties.queueLabel.IsNULLOrEmpty() )
            nameToTest = queue.queueProperties.queueLabel;    
        else
        if ( !queue.queueName.IsNULLOrEmpty() )
            nameToTest = queue.queueName;
            
        bool matchesFilter = nameToTest.WildcardEquals( globPatternFilters, '*', true, true );
        if ( matchesFilter )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics: Queue with available name \"" + nameToTest + "\" matches filter pattern(s), will enable metric collection" );
            queue.collectMetrics = true;
            ++metricEnabledQueues;
        }
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics: Queue with available name \"" + nameToTest + "\" does not match any filter pattern(s), will disable metric collection" );
            queue.collectMetrics = false;
            ++metricDisabledQueues;
        }
        
        ++i;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics: After applying filter(s) to known queues we have " + 
            CORE::ToString( metricEnabledQueues ) + " queues with enabled metrics and " + CORE::ToString( metricDisabledQueues ) + " metric disabled queues" );
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::Start( void )
{GUCEF_TRACE;

    if ( m_enableRestApi )
    {
        if ( m_httpServer.Listen() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "MsmqMetrics: Opened REST API on port " + CORE::UInt16ToString( m_httpServer.GetPort() ) );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "MsmqMetrics: Failed to open REST API on port " + CORE::UInt16ToString( m_httpServer.GetPort() ) );
        }
    }

    if ( SetStandbyMode( false ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "MsmqMetrics: Startup completed successfully" );
        return true;
    }

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "MsmqMetrics: Startup failed" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::SetStandbyMode( bool newModeIsStandby )
{GUCEF_TRACE;

    m_metricsTimer.SetEnabled( !newModeIsStandby );
    m_queueActivityCheckTimer.SetEnabled( !newModeIsStandby );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::IsGlobalStandbyEnabled( void ) const
{GUCEF_TRACE;

    return m_metricsTimer.GetEnabled();
}

/*-------------------------------------------------------------------------*/

bool
MsmqMetrics::LoadConfig( const CORE::CValueList& appConfig   ,
                         const CORE::CDataNode& globalConfig )
{GUCEF_TRACE;

    m_enableRestApi = CORE::StringToBool( appConfig.GetValueAlways( "enableRestApi" ), m_enableRestApi );
    m_metricsPrefix = appConfig.GetValueAlways( "metricsPrefix", m_metricsPrefix );
    m_queueNamesAreMsmqFormatNames = CORE::StringToBool( appConfig.GetValueAlways( "queueNamesAreMsmqFormatNames" ), m_queueNamesAreMsmqFormatNames );
    m_discoverQueues = CORE::StringToBool( appConfig.GetValueAlways( "discoverQueues" ), m_discoverQueues );
    m_gatherMsgsInQueueMetric = CORE::StringToBool( appConfig.GetValueAlways( "gatherMsgsInQueueMetric" ), m_gatherMsgsInQueueMetric );
    m_gatherMsgsTotalByteCountMetric = CORE::StringToBool( appConfig.GetValueAlways( "gatherMsgsTotalByteCountMetric" ), m_gatherMsgsTotalByteCountMetric );
    m_gatherMsgsInQueueJournalMetric = CORE::StringToBool( appConfig.GetValueAlways( "gatherMsgsInQueueJournalMetric" ), m_gatherMsgsInQueueJournalMetric );
    m_gatherMsgsInJournalTotalByteCountMetric = CORE::StringToBool( appConfig.GetValueAlways( "gatherMsgsInJournalTotalByteCountMetric" ), m_gatherMsgsInJournalTotalByteCountMetric );
    m_gatherTotalMsgBytesMetric = CORE::StringToBool( appConfig.GetValueAlways( "gatherTotalMsgBytesMetric" ), m_gatherTotalMsgBytesMetric );
    m_gatherMetricsFromOutgoingQueues = CORE::StringToBool( appConfig.GetValueAlways( "gatherMetricsFromOutgoingQueues" ), m_gatherMetricsFromOutgoingQueues );

    CORE::CString::StringSet discoverQueueFilters = appConfig.GetValueAlways( "discoverQueueFilters" ).AsString().ParseUniqueElements( ';', false );
    CORE::CString::StringSet queueMetricCollectionFilters = appConfig.GetValueAlways( "queueMetricCollectionFilters" ).AsString().ParseUniqueElements( ';', false );
    m_dontSendMetricsForInactiveQueues = CORE::StringToBool( appConfig.GetValueAlways( "dontSendMetricsForInactiveQueues" ), m_dontSendMetricsForInactiveQueues );
    CORE::UInt32 metricsIntervalInMs = CORE::StringToUInt32( appConfig.GetValueAlways( "metricsIntervalInMs" ), 1000 );
    CORE::UInt32 queueActivityCheckIntervalInMs = CORE::StringToUInt32( appConfig.GetValueAlways( "queueActivityCheckIntervalInMs" ), 60000 );

    // Set the explicitly configured queues
    CORE::CString::StringVector queuesToWatch = appConfig.GetValueAlways( "queuesToWatch" ).AsString().ParseElements( ';', false );
    CORE::CString::StringVector::iterator i = queuesToWatch.begin();
    while ( i != queuesToWatch.end() )
    {
        m_queues.push_back( MsmqQueue( (*i), m_queueNamesAreMsmqFormatNames ) );
        ++i;                                                          
    }

    // See if we want to auto find more queues
    if ( m_discoverQueues )
    {
        CORE::CString::StringSet discoverdQueues;
        FindAllQueues( discoverQueueFilters, discoverdQueues );
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "MsmqMetrics: Discovered " + CORE::ToString( discoverdQueues.size() ) + " queues" );

        CORE::CString::StringSet::iterator n = discoverdQueues.begin();
        while ( n != discoverdQueues.end() )
        {
            // discovered queues are always 'format names'
            m_queues.push_back( MsmqQueue( (*n), true ) );
            ++n;                                                          
        }
    }

    // Perform an initial init of the queue information
    MsmqQueueVector::iterator n = m_queues.begin();
    while ( n != m_queues.end() )
    {
        MsmqQueue& q = (*n);
        InitQueueInfo( q );
        ++n;
    }    
    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "MsmqMetrics: Initialized info for " + CORE::ToString( m_queues.size() ) + " queues" );
    
    if ( m_discoverQueues )
    {
        // After init we can use the broader queue info to get a list of hostnames
        // Regardless of what info was used to identify the queue

        MsmqQueueVector::iterator n = m_queues.begin();
        while ( n != m_queues.end() )
        {
            MsmqQueue& q = (*n);
            
            // The init above would have tried to fill in the hostname
            if ( !q.queueProperties.hostname.IsNULLOrEmpty() )
                m_hostnames.insert( q.queueProperties.hostname.Uppercase() );
            ++n;
        }

        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "MsmqMetrics: We have " + CORE::ToString( m_hostnames.size() ) + " hostnames related to MSMQ queues" );

        // looking for additional private queues elsewhere, we already gathered the localhost ones
        CORE::CString nameOfThisHost = CORE::GetHostname().Uppercase();
        m_hostnames.erase( nameOfThisHost );        
        CORE::CString::StringSet discoverdQueues;
        if ( FindPrivateQueues( m_hostnames, discoverQueueFilters, discoverdQueues ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics: Found " + CORE::ToString( discoverdQueues.size() ) + " additional private queues" );
        }
        m_hostnames.insert( nameOfThisHost );

        // Add the additional private queues on other hosts
        CORE::CString::StringSet::iterator w = discoverdQueues.begin();
        while ( w != discoverdQueues.end() )
        {
            // discovered queues are always 'format names'
            MsmqQueue q( (*w), true ); 
            InitQueueInfo( q );
            m_queues.push_back( q );
            ++w;                                                          
        }
    }

    SetMetricCollectionBasedOnFilter( m_queues, queueMetricCollectionFilters );

    // Perform the initial update on queue active status
    UpdateMsmqActiveQueueStatus( m_hostnames, m_queues, m_discoverQueues );
    m_queueActivityCheckTimer.SetInterval( queueActivityCheckIntervalInMs );
                                                                                                                   
    m_metricsTimer.SetInterval( metricsIntervalInMs );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MsmqMetrics: There are now " + CORE::ToString( m_queues.size() ) + " queues defined to be monitored" );

    if ( m_enableRestApi )
    {
        m_httpServer.SetPort( CORE::StringToUInt16( CORE::ResolveVars( appConfig.GetValueAlways( "restApiPort" ) ), 10000 ) );

        m_httpRouter.SetResourceMapping( "/info", RestApiProcessMetricsInfoResource::THTTPServerResourcePtr( new RestApiProcessMetricsInfoResource( this ) )  );
        m_httpRouter.SetResourceMapping( "/config/appargs", RestApiProcessMetricsInfoResource::THTTPServerResourcePtr( new RestApiProcessMetricsConfigResource( this, true ) )  );
        m_httpRouter.SetResourceMapping( "/config", RestApiProcessMetricsInfoResource::THTTPServerResourcePtr( new RestApiProcessMetricsConfigResource( this, false ) )  );
        m_httpRouter.SetResourceMapping(  appConfig.GetValueAlways( "restBasicHealthUri", "/health/basic" ), RestApiProcessMetricsInfoResource::THTTPServerResourcePtr( new WEB::CDummyHTTPServerResource() )  );

        if ( !m_taskManagementRsc.ConnectHttpRouting( m_httpRouter ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "MsmqMetrics: Failed to set up task management API" );
        }

        m_httpServer.GetRouterController()->AddRouterMapping( &m_httpRouter, "", "" );
    }

    m_appConfig = appConfig;
    m_globalConfig.Copy( globalConfig );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CValueList&
MsmqMetrics::GetAppConfig( void ) const
{
    return m_appConfig;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
MsmqMetrics::GetGlobalConfig( void ) const
{
    return m_globalConfig;
}

/*-------------------------------------------------------------------------*/
