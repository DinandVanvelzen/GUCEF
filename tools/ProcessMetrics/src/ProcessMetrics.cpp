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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#include "ProcessMetrics.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    #include <winsock2.h>
#endif

#ifndef GUCEF_CORE_METRICSMACROS_H
#include "gucefCORE_MetricsMacros.h"
#define GUCEF_CORE_METRICSMACROS_H
#endif /* GUCEF_CORE_METRICSMACROS_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CCOM_H
#include "CCom.h"
#define GUCEF_COMCORE_CCOM_H
#endif /* GUCEF_COMCORE_CCOM_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#include "gucefPUBSUB_CPubSubGlobal.h"
#define GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#endif /* GUCEF_PUBSUB_CPUBSUBGLOBAL_H ? */

#ifndef GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H
#include "gucefWEB_CDummyHTTPServerResource.h"
#define GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

RestApiProcessMetricsInfoResource::RestApiProcessMetricsInfoResource( ProcessMetrics* app )
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
    output.SetAttribute( "application", "ProcessMetrics" );
    output.SetAttribute( "buildDateTime", compileDt.ToIso8601DateTimeString( true, true ) );
    #ifdef GUCEF_DEBUG_MODE
    output.SetAttribute( "isReleaseBuild", "false" );
    #else
    output.SetAttribute( "isReleaseBuild", "true" );
    #endif
    return true;
}

/*-------------------------------------------------------------------------*/

RestApiProcessMetricsConfigResource::RestApiProcessMetricsConfigResource( ProcessMetrics* app, bool appConfig )
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

MetricThreshold::MetricThreshold( void )
    : CORE::CIConfigurable()
    , minThreshold()
    , maxThreshold()
    , applyMinThreshold( false )
    , applyMaxThreshold( true )
    , thresholdDescription()
    , metricName()
    , procFilter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
MetricThreshold::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    cfg.SetAttribute( "minThreshold", minThreshold );
    cfg.SetAttribute( "maxThreshold", maxThreshold );
    cfg.SetAttribute( "applyMinThreshold", applyMinThreshold );
    cfg.SetAttribute( "applyMaxThreshold", applyMaxThreshold );
    cfg.SetAttribute( "thresholdDescription", thresholdDescription );
    cfg.SetAttribute( "metricName", metricName );

    CORE::CString procFilterStr;
    CORE::CString::StringSet::iterator i = procFilter.begin();
    while ( i != procFilter.end() )
    {
        if ( procFilterStr.IsNULLOrEmpty() )
        {
            procFilterStr = (*i);
        }
        else
        {
            procFilterStr += ';' + (*i);
        }
        ++i;
    }
    cfg.SetAttribute( "procFilter", procFilterStr );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
MetricThreshold::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    minThreshold = cfg.GetAttributeValueOrChildValueByName( "minThreshold", minThreshold );
    maxThreshold = cfg.GetAttributeValueOrChildValueByName( "maxThreshold", maxThreshold );
    applyMinThreshold = cfg.GetAttributeValueOrChildValueByName( "applyMinThreshold" ).AsBool( applyMinThreshold, true );
    applyMaxThreshold = cfg.GetAttributeValueOrChildValueByName( "applyMaxThreshold" ).AsBool( applyMaxThreshold, true );
    thresholdDescription = cfg.GetAttributeValueOrChildValueByName( "thresholdDescription" ).AsString( thresholdDescription, true );
    metricName = cfg.GetAttributeValueOrChildValueByName( "metricName" ).AsString( metricName, true );

    CORE::CString procFilterStr = cfg.GetAttributeValueOrChildValueByName( "procFilter" ).AsString( CORE::CString::Empty, true );
    procFilter = procFilterStr.ParseUniqueElements( ';', false );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
MetricThreshold::IsValid( void )
{GUCEF_TRACE;

    return ( applyMinThreshold || applyMaxThreshold ) && !metricName.IsNULLOrEmpty() && ( minThreshold.IsInitialized() || maxThreshold.IsInitialized() );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
MetricThreshold::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "MetricThreshold";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const CORE::CEvent ProcessMetrics::CProcInfo::PidChangedEvent = "GUCEF::CORE::ProcessMetrics::CProcInfo::PidChangedEvent";

/*-------------------------------------------------------------------------*/

ProcessMetrics::CProcInfo::CProcInfo( void )
    : pid( GUCEF_NULL )
    , previousProcCpuDataDataPoint( GUCEF_NULL )
    , processInformation()
    , lastUptimeInMs( 0 )
    , exeName()
    , gatherMetrics( false )
    , startIfNotRunning( false )
    , restartIfStopsRunning( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

ProcessMetrics::CProcInfo::~CProcInfo()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
ProcessMetrics::CProcInfo::Clear( void )
{GUCEF_TRACE;

    CORE::FreeProcCpuDataPoint( previousProcCpuDataDataPoint );
    previousProcCpuDataDataPoint = GUCEF_NULL;
    CORE::FreeProcessId( pid );
    pid = GUCEF_NULL;
    processInformation.Clear();
    lastUptimeInMs = 0;
    exeName.Clear();
    gatherMetrics = false;
    startIfNotRunning = false;
    restartIfStopsRunning = false;
}

/*-------------------------------------------------------------------------*/

ProcessMetrics::CProcInfo& 
ProcessMetrics::CProcInfo::operator=( const CProcInfo& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        Clear();
        
        pid = CORE::CopyProcessId( src.pid );
        previousProcCpuDataDataPoint = CORE::CopyProcCpuDataPoint( previousProcCpuDataDataPoint, pid );
        processInformation = src.processInformation;
        lastUptimeInMs = src.lastUptimeInMs;
        exeName = src.exeName;
        gatherMetrics = src.gatherMetrics;
        startIfNotRunning = src.startIfNotRunning;
        restartIfStopsRunning = src.restartIfStopsRunning;
    }
    return *this;
}    

/*-------------------------------------------------------------------------*/

bool 
ProcessMetrics::CProcInfo::IsProcessStillActive( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != pid )
    {
        OSWRAP_BOOLINT status = OSWRAP_TRUE;
        OSWRAP_BOOLINT checkSuccess = CORE::IsProcessStillActive( pid, &status );
        if ( OSWRAP_TRUE == checkSuccess )
        {
            if ( status == OSWRAP_TRUE )
            {
                // the proc is (still) alive per the O/S
                if ( GUCEF_NULL != previousProcCpuDataDataPoint )
                {
                    // If we are gathering CPU data points we will perform an additional sanity check
                    CORE::TProcessCpuUsageInfo cpuUseInfo;
                    if ( OSWRAP_TRUE == CORE::GetProcessCpuUsage( pid, previousProcCpuDataDataPoint, &cpuUseInfo ) )
                    {
                        if ( lastUptimeInMs > cpuUseInfo.uptimeInMs )
                        {
                            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Proc \"" + exeName + "\" : Uptime went down even though the OS claims the proc is still alive. lastUptime=" + 
                                CORE::ToString( lastUptimeInMs ) + " -> uptime=" + CORE::ToString( cpuUseInfo.uptimeInMs ) );
                        
                            lastUptimeInMs = cpuUseInfo.uptimeInMs;

                            // If the up time went down then the process cannot possibly have been the same one that persisted
                            // Since things don't add up we will report this proc as dead since the context is that in this app it will cause
                            // us to refresh all relevant information                        
                            return false;
                        }
                        lastUptimeInMs = cpuUseInfo.uptimeInMs;
                    }
                }
            
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Proc \"" + exeName + "\" is still alive" );
                return true;
            }
            else
            {
                // Check succeeded but the app was reported as no longer alive
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Proc \"" + exeName + "\" is NOT alive" );
                return false;
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Failed to check alive status of proc with name \"" + exeName + "\"" );
        }
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Cannot check if Proc \"" + exeName + "\" is still alive because we have no PID" );
    return false;
}

/*-------------------------------------------------------------------------*/

void
ProcessMetrics::CProcInfo::Unlink( void )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Unlinking proc entry for \"" + exeName + "\"" );

    CORE::FreeProcCpuDataPoint( previousProcCpuDataDataPoint );
    previousProcCpuDataDataPoint = GUCEF_NULL;
    CORE::FreeProcessId( pid );
    pid = GUCEF_NULL;
    lastUptimeInMs = 0;
}

/*-------------------------------------------------------------------------*/

bool 
ProcessMetrics::CProcInfo::RefreshPID( void )
{GUCEF_TRACE;

    bool success = false;
    CORE::UInt32 procIdCount = 0;
    CORE::TProcessId* procIds = GUCEF_NULL;
    CORE::UInt32 retVal = CORE::GetProcessList( &procIds, &procIdCount );
    if ( OSWRAP_TRUE == retVal )
    {
        success = RefreshPID( procIds, procIdCount );
    }
    CORE::FreeProcessList( procIds );
    return success;
}

/*-------------------------------------------------------------------------*/

bool 
ProcessMetrics::CProcInfo::RefreshPID( CORE::TProcessId* procIds, CORE::UInt32 procIdCount )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Refresing PID administration for " + exeName );

    bool success = false;
    for ( CORE::UInt32 i=0; i<procIdCount; ++i )
    {
        CORE::TProcessId* searchPid = CORE::GetProcessIdAtIndex( procIds, i );

        CORE::CString searchExeName;
        if ( GUCEF_NULL != searchPid && CORE::GetExeNameForProcessId( searchPid, searchExeName ) )
        {
            searchExeName = CORE::ExtractFilename( searchExeName );
            CORE::Int32 dotIndex = searchExeName.HasChar( '.', false );
            if ( dotIndex >= 0 )
                searchExeName = searchExeName.SubstrToIndex( (CORE::UInt32) dotIndex, true );

            if ( searchExeName == exeName )
            {
                return RefreshPID( searchPid );
            }
        }
    }
    
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: No running proc match found for \"" + exeName + "\"" );
    return success;
}

/*-------------------------------------------------------------------------*/

bool 
ProcessMetrics::CProcInfo::RefreshPID( CORE::TProcessId* newProcId )
{GUCEF_TRACE;

    if ( GUCEF_NULL != pid )
    {
        CORE::FreeProcessId( pid );
        pid = GUCEF_NULL;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Refreshing pre-existing PID for \"" + exeName + "\"" );
    }            
    else
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Applying PID for \"" + exeName + "\"" );
    }
                
    pid = CORE::CopyProcessId( newProcId );
    
    if ( CORE::CProcessInformation::TryGetProcessInformation( pid, processInformation ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Obtained process information for \"" + exeName + "\"" );
    }
    else
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Failed to obtain process information for \"" + exeName + "\"" );
    }

    if ( GUCEF_NULL != previousProcCpuDataDataPoint )
    {
        CORE::TProcCpuDataPoint* refreshedPidDatapoint = CORE::CopyProcCpuDataPoint( previousProcCpuDataDataPoint, pid );
        CORE::FreeProcCpuDataPoint( previousProcCpuDataDataPoint );
        previousProcCpuDataDataPoint = refreshedPidDatapoint;
    }
    else
    {
        previousProcCpuDataDataPoint = CORE::CreateProcCpuDataPoint( pid ); 
    }

    return true;
}

/*-------------------------------------------------------------------------*/

ProcessMetrics::ProcessMetrics( void )
    : CORE::CObservingNotifier()
    , m_httpServer()
    , m_httpRouter()
    , m_taskManagementRsc()
    , m_appConfig()
    , m_globalConfig()
    , m_metricsTimer()
    , m_procIndexTimer()
    , m_pubSubClient()
    , m_thresholdNotificationPublishTopic()
    , m_pubSubFeatures()
    , m_thresholdNotificationPrimaryPayloadCodecType()
    , m_gatherMemStats( true )
    , m_gatherCpuStats( true )
    , m_enableRestApi( true )
    , m_enableEventMsgPublishing( true )
    , m_exeProcsToWatch()
    , m_exeMatchPidMatchThreshold( 0 )
    , m_metricsThresholds()
    , m_procMetricsThresholds()
    , m_globalCpuDataPoint( GUCEF_NULL )
    , m_storageVolumeIds()
    , m_storageVolumeIdsToPaths()
    , m_gatherProcPageFaultCountInBytes( true )
    , m_gatherProcPageFileUsageInBytes( true )
    , m_gatherProcPeakPageFileUsageInBytes( true )
    , m_gatherProcPeakWorkingSetSizeInBytes( true )
    , m_gatherProcWorkingSetSizeInBytes( true )
    , m_gatherGlobMemStats( true )
    , m_gatherGlobalAvailablePageFileSizeInBytes( true )
    , m_gatherGlobalPageFileUsageInBytes( true )
    , m_gatherGlobalAvailableVirtualMemoryInBytes( true )
    , m_gatherGlobalAvailExtendedVirtualMemoryInBytes( true )
    , m_gatherGlobalMemoryLoadPercentage( true )
    , m_gatherGlobalTotalPageFileSizeInBytes( true )
    , m_gatherGlobalTotalPhysicalMemoryInBytes( false )
    , m_gatherGlobalTotalVirtualMemoryInBytes( true )
    , m_gatherProcCpuUptime( true )
    , m_gatherProcCpuOverallPercentage( true )
    , m_gatherGlobalCpuStats( true )
    , m_gatherGlobalCpuCurrentFrequencyInMhz( true )
    , m_gatherGlobalCpuSpecMaxFrequencyInMhz( false )
    , m_gatherGlobalCpuMaxFrequencyInMhz( false )
    , m_gatherGlobalCpuOverallPercentage( true )
    , m_gatherGlobalCpuOverallPercentagePerCore( true )
    , m_gatherGlobalNetworkStats( true )
    , m_gatherGlobalNetworkStatInboundOctets( true )
    , m_gatherGlobalNetworkStatInboundUnicastOctets( true )
    , m_gatherGlobalNetworkStatInboundUnicastPackets( true )
    , m_gatherGlobalNetworkStatInboundNonUnicastPackets( true )
    , m_gatherGlobalNetworkStatInboundErroredPackets( true )
    , m_gatherGlobalNetworkStatInboundDiscardedPackets( true )
    , m_gatherGlobalNetworkStatOutboundOctets( true )
    , m_gatherGlobalNetworkStatOutboundUnicastOctets( true )
    , m_gatherGlobalNetworkStatOutboundUnicastPackets( true )
    , m_gatherGlobalNetworkStatOutboundNonUnicastPackets( true )
    , m_gatherGlobalNetworkStatOutboundErroredPackets( true )
    , m_gatherGlobalNetworkStatOutboundDiscardedPackets( true )
    , m_gatherGlobalStorageStats( true )
    , m_gatherGlobalStorageVolumeBytesAvailableToCaller( true )
    , m_gatherGlobalStorageVolumeBytesAvailable( true )
    , m_gatherGlobalStorageVolumeBytes( false )
    , m_gatherGlobalStorageVolumeAvailableToCallerPercentage( true )
    , m_gatherGlobalStorageVolumeAvailablePercentage( true )
    , m_convertStorageVolumeIdsToPaths( true )
    , m_gatherGlobalNetworkStatInboundMulticastOctets( true )
    , m_gatherGlobalNetworkStatOutboundMulticastOctets( true )
    , m_gatherGlobalNetworkStatInboundBroadcastOctets( true )
    , m_gatherGlobalNetworkStatOutboundBroadcastOctets( true )
{GUCEF_TRACE;

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

ProcessMetrics::~ProcessMetrics()
{GUCEF_TRACE;

    SetStandbyMode( true );

    CORE::FreeCpuDataPoint( m_globalCpuDataPoint );
    m_globalCpuDataPoint = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
ProcessMetrics::RegisterEventHandlers( void )
{GUCEF_TRACE;

    TEventCallback callback( this, &ProcessMetrics::OnMetricsTimerCycle );
    SubscribeTo( &m_metricsTimer                ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback                       );
    
    TEventCallback callback2( this, &ProcessMetrics::OnProcScanTimerCycle );
    SubscribeTo( &m_procIndexTimer              ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback2                      );    
}

/*-------------------------------------------------------------------------*/

void
ProcessMetrics::RefreshPIDs( const CORE::CString::StringSet& refreshExeNames )
{GUCEF_TRACE;

    if ( refreshExeNames.empty() )
        return;
    
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Refresing PID administration for " + CORE::ToString( refreshExeNames.size() ) + " items" );

    CORE::UInt32 procIdCount = 0;
    CORE::TProcessId* procIds = GUCEF_NULL;
    CORE::UInt32 retVal = CORE::GetProcessList( &procIds, &procIdCount );
    if ( OSWRAP_TRUE == retVal )
    {
        for ( CORE::UInt32 i=0; i<procIdCount; ++i )
        {
            CORE::TProcessId* pid = CORE::GetProcessIdAtIndex( procIds, i );

            CORE::CString exeName;
            if ( CORE::GetExeNameForProcessId( pid, exeName ) )
            {
                exeName = CORE::ExtractFilename( exeName );
                CORE::Int32 dotIndex = exeName.HasChar( '.', false );
                if ( dotIndex >= 0 )
                    exeName = exeName.SubstrToIndex( (CORE::UInt32) dotIndex, true );

                TStringSet::iterator n = refreshExeNames.find( exeName );
                if ( n != refreshExeNames.end() )
                {
                    TProcessIdMap::iterator m = m_exeProcsToWatch.find( exeName );
                    if ( m != m_exeProcsToWatch.end() )
                    {
                        CProcInfo& procInfo = (*m).second;
                        if ( procInfo.RefreshPID( pid ) )
                        {
                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Refreshed PID for \"" + procInfo.exeName + "\"" );
                        }
                    }
                }
            }
        }
    }
    CORE::FreeProcessList( procIds );
}

/*-------------------------------------------------------------------------*/

bool
ProcessMetrics::LaunchProcs( const CORE::CString::StringSet& procsToLaunch )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Attempting to launch " + CORE::ToString( procsToLaunch.size() ) + " proc(s)" );
    
    bool totalSuccess = true;
    CORE::CString::StringSet::const_iterator i = procsToLaunch.begin();
    while ( i != procsToLaunch.end() )
    {
        const CORE::CString& exeToLaunch = (*i);

        TProcessIdMap::iterator m = m_exeProcsToWatch.find( exeToLaunch );
        if ( m != m_exeProcsToWatch.end() )
        {
            CProcInfo& procInfo = (*m).second;

            // Double check to be sure, just in case
            if ( !procInfo.IsProcessStillActive() )
            {
                const CORE::CString& exePath = procInfo.processInformation.GetImagePath();
                const CORE::CString& cmdLine = procInfo.processInformation.GetCommandLineArgs();
                if ( !exePath.IsNULLOrEmpty() )
                {
                    if ( CORE::Execute_Program( exePath.C_String(), cmdLine.C_String() ) > 0 )
                    {
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Launched process \"" + procInfo.exeName + "\" using image \"" + exePath + "\" and command line \"" + cmdLine + "\"" );
                    }
                    else
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Failed to launch process \"" + procInfo.exeName + "\" using image \"" + exePath + "\" and command line \"" + cmdLine + "\"" );
                        totalSuccess = false;
                    }
                }
                else
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Don't have an executable image path from where to launch process \"" + procInfo.exeName + "\" hence will rely on the proc perhaps being in a known system path" );

                    if ( CORE::Execute_Program( procInfo.exeName.C_String(), cmdLine.C_String() ) > 0 )
                    {
                        // right away refresh the PID with a scan to obtain better path info
                        if ( procInfo.RefreshPID() )
                        {
                            const CORE::CString& actualExePath = procInfo.processInformation.GetImagePath();
                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Launched process \"" + procInfo.exeName + "\" using image \"" + actualExePath + "\" and command line \"" + cmdLine + "\"" );
                        }
                        else
                        {
                            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Launched process \"" + procInfo.exeName + 
                                "\" using only its name and command line \"" + cmdLine + "\" but unable to get its meta-data" );
                        }
                    }
                    else
                    {
                        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Failed to launch process \"" + procInfo.exeName + "\" using only its name. Try adding the process to known standard system search paths" );
                        totalSuccess = false;
                    }
                }
            }
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Cannot find proc entry for proc to launch " + exeToLaunch + ", this should never happen" );
            totalSuccess = false;
        }
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
ProcessMetrics::SetupPubSubClient( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    const CORE::CDataNode* pubSubClientCfgNode = cfg.Find( "PubSubClientConfig" );
    if ( GUCEF_NULL != pubSubClientCfgNode )
    {
        PUBSUB::CPubSubClientConfig pubSubClientCfg;
        if ( pubSubClientCfg.LoadConfig( *pubSubClientCfgNode ) )
        {
            m_pubSubClient = PUBSUB::CPubSubGlobal::Instance()->GetPubSubClientFactory().Create( pubSubClientCfg.pubsubClientType, pubSubClientCfg );
            if ( !m_pubSubClient.IsNULL() )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics:SetupPubSubClient: Successfully instantiated pub-sub client of type: " + pubSubClientCfg.pubsubClientType );
                m_pubSubClient->GetSupportedFeatures( m_pubSubFeatures );

                // This app really only has topics for one purpose which is sending event messages
                // As such we just grab the first config'd topic as the topic to do that on vs having yet another config setting
                CORE::CString::StringSet topicNameList;
                m_pubSubClient->GetConfiguredTopicNameList( topicNameList );
                if ( !topicNameList.empty() )
                {
                    m_thresholdNotificationPublishTopic = m_pubSubClient->GetTopicAccess( *topicNameList.begin() );
                    if ( GUCEF_NULL == m_thresholdNotificationPublishTopic )
                    {
                        m_thresholdNotificationPublishTopic = m_pubSubClient->CreateTopicAccess( *topicNameList.begin() );
                    }
                    if ( GUCEF_NULL != m_thresholdNotificationPublishTopic )
                    {
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics:SetupPubSubClient: Successfully obtained access to topic with name: " + *topicNameList.begin() );
                        return true;
                    }
                    else
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics:SetupPubSubClient: Instantiated pub-sub client has topic configured, but unable to obtain access. topicName=" + *topicNameList.begin() );
                    }
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics:SetupPubSubClient: Instantiated pub-sub client has no topics configured" );
                }
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics:SetupPubSubClient: Failed to instantiate pub-sub client from config data. pubsubClientType=" + pubSubClientCfg.pubsubClientType );
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics:SetupPubSubClient: Failed to load config data from config node \"PubSubClientConfig\"" );
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics:SetupPubSubClient: No config node \"PubSubClientConfig\" could be located" );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
ProcessMetrics::PublishMetricThresholdExceeded( const CORE::CVariant& metricValue ,
                                                const CORE::CString& metricName   ,
                                                const CORE::CString& procName     ,
                                                const MetricThreshold& threshold  )
{GUCEF_TRACE;

    if ( m_pubSubClient.IsNULL() || GUCEF_NULL == m_thresholdNotificationPublishTopic )
        return;

    PUBSUB::CBasicPubSubMsg msg;

    msg.AddKeyValuePair( "ProcessMetrics.EventMsgType", "MetricThresholdExceeded" );
    msg.AddKeyValuePair( "ProcessMetrics.MetricName", metricName );
    msg.AddKeyValuePair( "ProcessMetrics.MetricValue", metricValue );
    if ( threshold.applyMinThreshold )
        msg.AddKeyValuePair( "ProcessMetrics.MetricMinThreshold", threshold.minThreshold );
    if ( threshold.applyMaxThreshold )
        msg.AddKeyValuePair( "ProcessMetrics.MetricMaxThreshold", threshold.maxThreshold );
    if ( !procName.IsNULLOrEmpty() )
        msg.AddKeyValuePair( "ProcessMetrics.ProcName", procName );

    // if we have support for key-value let's use that since it doesnt require a consumer to use special parsing
    if ( !m_pubSubFeatures.supportsKeyValueSetPerMsg || !m_pubSubFeatures.supportsAbsentPrimaryPayloadPerMsg )
    {
        // @TODO: It would be better if we could source this from the codec itself
        int payloadVarType = GUCEF_DATATYPE_BINARY_BLOB;
        if ( "json" == m_thresholdNotificationPrimaryPayloadCodecType ||
             "xml" == m_thresholdNotificationPrimaryPayloadCodecType   )
        {
            payloadVarType = GUCEF_DATATYPE_UTF8_STRING;
        }

        if ( !msg.MoveKeyValuePairsToEncodedPrimaryPayload( m_thresholdNotificationPrimaryPayloadCodecType, payloadVarType ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProcessMetrics:PublishMetricThresholdExceeded: Failed to encode primary payload on event message. Aborting" );
            return;
        }
    }

    CORE::UInt64 publishActionId = 0;
    if ( m_thresholdNotificationPublishTopic->Publish( publishActionId, msg, true ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "ProcessMetrics:PublishMetricThresholdExceeded: Published metric threshold exceeded event message" );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProcessMetrics:PublishMetricThresholdExceeded: Failed to publish event message" );
    }
}

/*-------------------------------------------------------------------------*/

void
ProcessMetrics::ValidateMetricThresholds( const CORE::CVariant& metricValue ,
                                          const CORE::CString& metricName   ,
                                          const CORE::CString& procName     )
{GUCEF_TRACE;

    if ( !m_enableEventMsgPublishing )
        return;

    TMetricsThresholdMap::iterator i = m_metricsThresholds.find( metricName );
    if ( i != m_metricsThresholds.end() )
    {
        MetricThreshold& thresholds = (*i).second;

        if ( thresholds.applyMinThreshold )
        {
            if ( metricValue <= thresholds.minThreshold )
            {
                PublishMetricThresholdExceeded( metricValue, metricName, procName, thresholds );
            }
        }
        if ( thresholds.applyMaxThreshold )
        {
            if ( metricValue >= thresholds.maxThreshold )
            {
                PublishMetricThresholdExceeded( metricValue, metricName, procName, thresholds );
            }
        }
    }

    TMetricsThresholdMapMap::iterator n = m_procMetricsThresholds.find( procName );
    if ( n != m_procMetricsThresholds.end() )
    {
        TMetricsThresholdMap& thresholdMapForProc = (*n).second;
        TMetricsThresholdMap::const_iterator m = thresholdMapForProc.find( metricName );
        if ( m != thresholdMapForProc.end() )
        {
            const MetricThreshold& thresholds = (*m).second;

            if ( thresholds.applyMinThreshold )
            {
                if ( metricValue <= thresholds.minThreshold )
                {
                    PublishMetricThresholdExceeded( metricValue, metricName, procName, thresholds );
                }
            }
            if ( thresholds.applyMaxThreshold )
            {
                if ( metricValue >= thresholds.maxThreshold )
                {
                    PublishMetricThresholdExceeded( metricValue, metricName, procName, thresholds );
                }
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
ProcessMetrics::OnProcScanTimerCycle( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // First get the count of how many procs we are locked on to out of the specified ones
    CORE::CString::StringSet nonLockedOnProcs;
    CORE::CString::StringSet procsToLaunch;
    UInt32 lockedOnProcs = 0;
    TProcessIdMap::iterator m = m_exeProcsToWatch.begin();
    while ( m != m_exeProcsToWatch.end() )
    {        
        // We don't just check that we found the proc, we also revalidate that its still alive
        CProcInfo& procInfo = (*m).second;
        if ( procInfo.IsProcessStillActive() )
        {
            ++lockedOnProcs;
        }
        else
        {
            nonLockedOnProcs.insert( procInfo.exeName );

            if ( procInfo.startIfNotRunning                                     && 
                 ( GUCEF_NULL == procInfo.pid && 0 == procInfo.lastUptimeInMs )  )
            {
                // this proc is not running and has not run before to the best of our knowledge
                procsToLaunch.insert( procInfo.exeName );
            }
            else
            if ( procInfo.restartIfStopsRunning                                  && 
                 ( GUCEF_NULL != procInfo.pid || 0 != procInfo.lastUptimeInMs )  )
            {
                // this proc was running and is requested to be auto-restarted
                procsToLaunch.insert( procInfo.exeName );
            }
        }
        ++m;
    }

    // Are we locked onto a sufficient nr of procs per the config?
    // negative number means always match everything
    if ( m_exeMatchPidMatchThreshold > 0 && lockedOnProcs >= (UInt32) m_exeMatchPidMatchThreshold )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Locked onto " + CORE::ToString( lockedOnProcs ) + 
            " procs out of a minimum match total of " + CORE::ToString( m_exeMatchPidMatchThreshold ) + ", skipping proc scan" );
        return;
    }

    LaunchProcs( procsToLaunch );
    RefreshPIDs( nonLockedOnProcs );
}

/*-------------------------------------------------------------------------*/

void
ProcessMetrics::RefreshPIDs( void )
{GUCEF_TRACE;

    // First get a flat list of the procs
    CORE::CString::StringSet nonLockedOnProcs;
    UInt32 lockedOnProcs = 0;
    TProcessIdMap::iterator m = m_exeProcsToWatch.begin();
    while ( m != m_exeProcsToWatch.end() )
    {        
        // We don't just check that we found the proc, we also revalidate that its still alive
        CProcInfo& procInfo = (*m).second;
        if ( procInfo.IsProcessStillActive() )
        {
            ++lockedOnProcs;
        }
        else
        {
            nonLockedOnProcs.insert( procInfo.exeName );
        }
        ++m;
    }

    RefreshPIDs( nonLockedOnProcs );
}

/*-------------------------------------------------------------------------*/

void
ProcessMetrics::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    TStringSet failedProcs;

    if ( m_gatherMemStats )
    {        
        TProcessIdMap::iterator m = m_exeProcsToWatch.begin();
        while ( m != m_exeProcsToWatch.end() )
        {
            CProcInfo& procInfo = (*m).second;
            if ( procInfo.gatherMetrics )
            {            
                 CORE::TProcessMemoryUsageInfo memUseInfo;
                if ( OSWRAP_TRUE == CORE::GetProcessMemoryUsage( procInfo.pid, &memUseInfo ) )
                {
                    const CORE::CString& procName = (*m).first;
                    CORE::CString metricPrefix = "ProcessMetrics." + procName + '.';

                    if ( m_gatherProcPageFaultCountInBytes )
                    {
                        static const CORE::CString metricName = "MemUse.PageFaultCountInBytes";
                        GUCEF_METRIC_TIMING( metricPrefix + metricName, memUseInfo.pageFaultCountInBytes, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( memUseInfo.pageFaultCountInBytes ), metricName, procName );
                    }
                    if ( m_gatherProcPageFileUsageInBytes )
                    {
                        static const CORE::CString metricName = "MemUse.PageFileUsageInBytes";
                        GUCEF_METRIC_TIMING( metricPrefix + metricName, memUseInfo.pageFileUsageInBytes, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( memUseInfo.pageFileUsageInBytes ), metricName, procName );
                    }
                    if ( m_gatherProcPeakPageFileUsageInBytes )
                    {
                        static const CORE::CString metricName = "MemUse.PeakPageFileUsageInBytes";
                        GUCEF_METRIC_TIMING( metricPrefix + metricName, memUseInfo.peakPageFileUsageInBytes, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( memUseInfo.peakPageFileUsageInBytes ), metricName, procName );
                    }
                    if ( m_gatherProcPeakWorkingSetSizeInBytes )
                    {
                        static const CORE::CString metricName = "MemUse.PeakWorkingSetSizeInBytes";
                        GUCEF_METRIC_TIMING( metricPrefix + metricName, memUseInfo.peakWorkingSetSizeInBytes, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( memUseInfo.peakWorkingSetSizeInBytes ), metricName, procName );
                    }
                    if ( m_gatherProcWorkingSetSizeInBytes )
                    {
                        static const CORE::CString metricName = "MemUse.WorkingSetSizeInBytes";
                        GUCEF_METRIC_TIMING( metricPrefix + metricName, memUseInfo.workingSetSizeInBytes, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( memUseInfo.workingSetSizeInBytes ), metricName, procName );
                    }
                }
                else
                {
                    failedProcs.insert( (*m).first );
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Failed to obtain memory stats for \"" + (*m).first + "\"" );
                }
            }
            ++m;
        }
    }

    if ( m_gatherGlobalCpuStats )
    {
        CORE::TCpuStats* cpuStats = GUCEF_NULL;
        if ( OSWRAP_TRUE == CORE::GetCpuStats( m_globalCpuDataPoint, &cpuStats ) && GUCEF_NULL != cpuStats )
        {
            CORE::CString lCpuMetricPrefix = "ProcessMetrics.Cpu.";

            if ( m_gatherGlobalCpuOverallPercentage )
            {
                static const CORE::CString metricName = "CpuUsePercentage";
                GUCEF_METRIC_GAUGE( lCpuMetricPrefix + metricName, cpuStats->cpuUsePercentage, 1.0f );
                ValidateMetricThresholds( CORE::CVariant( cpuStats->cpuUsePercentage ), metricName, CORE::CString::Empty );                
            }

            for ( CORE::UInt32 i=0; i<cpuStats->logicalCpuCount; ++i )
            {
                lCpuMetricPrefix = "ProcessMetrics.Cpu." + CORE::ToString( i ) +  ".";
                CORE::TLogicalCpuStats* lCpuStats = &cpuStats->logicalCpuStats[ i ];

                if ( m_gatherGlobalCpuCurrentFrequencyInMhz )
                {
                    // check if the O/S was able to provide this value.
                    // If not disable the stat. Unsupported stats return 0
                    if ( lCpuStats->cpuCurrentFrequencyInMhz > 0.5 )
                    {
                        static const CORE::CString metricName = "CpuCurrentFrequencyInMhz";
                        GUCEF_METRIC_GAUGE( lCpuMetricPrefix + metricName, lCpuStats->cpuCurrentFrequencyInMhz, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( lCpuStats->cpuCurrentFrequencyInMhz ), metricName, CORE::CString::Empty );
                    }
                    else
                    {
                        m_gatherGlobalCpuCurrentFrequencyInMhz = false;
                    }
                }
                if ( m_gatherGlobalCpuSpecMaxFrequencyInMhz )
                {
                    // check if the O/S was able to provide this value.
                    // If not disable the stat. Unsupported stats return 0
                    if ( lCpuStats->cpuSpecMaxFrequencyInMhz > 0.5 )
                    {
                        static const CORE::CString metricName = "CpuSpecMaxFrequencyInMhz";
                        GUCEF_METRIC_GAUGE( lCpuMetricPrefix + metricName, lCpuStats->cpuSpecMaxFrequencyInMhz, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( lCpuStats->cpuSpecMaxFrequencyInMhz ), metricName, CORE::CString::Empty );
                    }
                    else
                    {
                        m_gatherGlobalCpuSpecMaxFrequencyInMhz = false;
                    }
                }
                if ( m_gatherGlobalCpuMaxFrequencyInMhz )
                {
                    // check if the O/S was able to provide this value.
                    // If not disable the stat. Unsupported stats return 0
                    if ( lCpuStats->cpuCurrentFrequencyInMhz > 0.5 )
                    {
                        static const CORE::CString metricName = "CpuMaxFrequencyInMhz";
                        GUCEF_METRIC_GAUGE( lCpuMetricPrefix + metricName, lCpuStats->cpuMaxFrequencyInMhz, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( lCpuStats->cpuMaxFrequencyInMhz ), metricName, CORE::CString::Empty );
                    }
                    else
                    {
                        m_gatherGlobalCpuMaxFrequencyInMhz = false;
                    }
                }
                if ( m_gatherGlobalCpuOverallPercentagePerCore )
                {
                    static const CORE::CString metricName = "CpuUsePercentage";
                    GUCEF_METRIC_GAUGE( lCpuMetricPrefix + metricName, lCpuStats->cpuUsePercentage, 1.0f );
                    ValidateMetricThresholds( CORE::CVariant( lCpuStats->cpuUsePercentage ), metricName, CORE::CString::Empty );                
                }
            }
        }
    }

    if ( m_gatherCpuStats )
    {
        TProcessIdMap::iterator m = m_exeProcsToWatch.begin();
        while ( m != m_exeProcsToWatch.end() )
        {
            CProcInfo& procInfo = (*m).second;
            if ( procInfo.gatherMetrics )
            { 
                CORE::TProcessCpuUsageInfo cpuUseInfo;
                if ( OSWRAP_TRUE == CORE::GetProcessCpuUsage( procInfo.pid, procInfo.previousProcCpuDataDataPoint, &cpuUseInfo ) )
                {
                    // Any time we grab the uptime we should update the variable we retain for longer
                    // this is used as an extra sanity check on whether the process's lifecycle was interrupted
                    procInfo.lastUptimeInMs = cpuUseInfo.uptimeInMs;

                    const CORE::CString& procName = (*m).first;                
                    CORE::CString metricPrefix = "ProcessMetrics." + procName + '.';

                    if ( m_gatherProcCpuUptime )
                    {
                        static const CORE::CString metricName = "CpuUse.UptimeInMs";
                        GUCEF_METRIC_GAUGE( metricPrefix + metricName, cpuUseInfo.uptimeInMs, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( cpuUseInfo.uptimeInMs ), metricName, procName );
                    }
                    if ( m_gatherProcCpuOverallPercentage )
                    {
                        static const CORE::CString metricName = "CpuUse.TotalCpuUsePercentage";
                        GUCEF_METRIC_GAUGE( metricPrefix + metricName, cpuUseInfo.overallCpuConsumptionPercentage, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( cpuUseInfo.overallCpuConsumptionPercentage ), metricName, procName );
                    }
                }
                else
                {
                    failedProcs.insert( (*m).first );
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Failed to obtain cpu stats for \"" + (*m).first + "\"" );
                }
            }
            ++m;
        }
    }

    if ( m_gatherGlobMemStats )
    {
        CORE::TGlobalMemoryUsageInfo globMemInfo;
        if ( OSWRAP_TRUE == CORE::GetGlobalMemoryUsage( &globMemInfo ) )
        {
            if ( m_gatherGlobalAvailablePageFileSizeInBytes )
            {
                static const CORE::CString metricName = "ProcessMetrics.MemUse.AvailablePageFileSizeInBytes";
                GUCEF_METRIC_TIMING( metricName, globMemInfo.availablePageFileSizeInBytes, 1.0f );
                ValidateMetricThresholds( CORE::CVariant( globMemInfo.availablePageFileSizeInBytes ), metricName, CORE::CString::Empty );
            }
            if ( m_gatherGlobalPageFileUsageInBytes )
            {
                static const CORE::CString metricName = "ProcessMetrics.MemUse.AvailablePhysicalMemoryInBytes";
                GUCEF_METRIC_TIMING( metricName, globMemInfo.availablePhysicalMemoryInBytes, 1.0f );
                ValidateMetricThresholds( CORE::CVariant( globMemInfo.availablePhysicalMemoryInBytes ), metricName, CORE::CString::Empty );
            }
            if ( m_gatherGlobalAvailableVirtualMemoryInBytes )
            {
                static const CORE::CString metricName = "ProcessMetrics.MemUse.AvailableVirtualMemoryInBytes";
                GUCEF_METRIC_TIMING( metricName, globMemInfo.availableVirtualMemoryInBytes, 1.0f );
                ValidateMetricThresholds( CORE::CVariant( globMemInfo.availableVirtualMemoryInBytes ), metricName, CORE::CString::Empty );
            }
            if ( m_gatherGlobalAvailExtendedVirtualMemoryInBytes )
            {
                static const CORE::CString metricName = "ProcessMetrics.MemUse.AvailExtendedVirtualMemoryInBytes";
                GUCEF_METRIC_TIMING( metricName, globMemInfo.availExtendedVirtualMemoryInBytes, 1.0f );
                ValidateMetricThresholds( CORE::CVariant( globMemInfo.availExtendedVirtualMemoryInBytes ), metricName, CORE::CString::Empty );
            }
            if ( m_gatherGlobalMemoryLoadPercentage )
            {
                static const CORE::CString metricName = "ProcessMetrics.MemUse.MemoryLoadPercentage";
                GUCEF_METRIC_TIMING( metricName, (CORE::UInt64) globMemInfo.memoryLoadPercentage, 1.0f );
                ValidateMetricThresholds( CORE::CVariant( globMemInfo.memoryLoadPercentage ), metricName, CORE::CString::Empty );
            }
            if ( m_gatherGlobalTotalPageFileSizeInBytes )
            {
                static const CORE::CString metricName = "ProcessMetrics.MemUse.TotalPageFileSizeInBytes";
                GUCEF_METRIC_TIMING( metricName, globMemInfo.totalPageFileSizeInBytes, 1.0f );
                ValidateMetricThresholds( CORE::CVariant( globMemInfo.totalPageFileSizeInBytes ), metricName, CORE::CString::Empty );
            }
            if ( m_gatherGlobalTotalPhysicalMemoryInBytes )
            {
                static const CORE::CString metricName = "ProcessMetrics.MemUse.TotalPhysicalMemoryInBytes";
                GUCEF_METRIC_GAUGE( metricName, globMemInfo.totalPhysicalMemoryInBytes, 1.0f );
                ValidateMetricThresholds( CORE::CVariant( globMemInfo.totalPhysicalMemoryInBytes ), metricName, CORE::CString::Empty );
            }
            if ( m_gatherGlobalTotalVirtualMemoryInBytes )
            {
                static const CORE::CString metricName = "ProcessMetrics.MemUse.TotalVirtualMemoryInBytes";
                GUCEF_METRIC_TIMING( metricName, globMemInfo.totalVirtualMemoryInBytes, 1.0f );
                ValidateMetricThresholds( CORE::CVariant( globMemInfo.totalVirtualMemoryInBytes ), metricName, CORE::CString::Empty );
            }
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Failed to obtain global memory stats" );
        }
    }

    if ( m_gatherGlobalNetworkStats )
    {
        COMCORE::CCom& comms = COMCORE::CComCoreGlobal::Instance()->GetCom();
        
        COMCORE::CCom::TINetworkInterfacePtrVector nics;
        if ( comms.GetAllNetworkInterfaces( nics ) )
        {
            static const CORE::CString nicMetricNamePrefix = "ProcessMetrics.Network.";
            COMCORE::CCom::TINetworkInterfacePtrVector::iterator i = nics.begin();
            while ( i != nics.end() )
            {
                COMCORE::CINetworkInterfacePtr& nic = (*i);
                COMCORE::CNetworkInterfaceMetrics nicMetrics;
                if ( !nic.IsNULL() && nic->GetMetrics( nicMetrics ) )
                {
                    if ( m_gatherGlobalNetworkStatInboundOctets && nicMetrics.hasInboundOctets )
                    {
                        CORE::CString nicMetricName = nicMetricNamePrefix + nic->GetAdapterName() + ".InboundOctets";
                        GUCEF_METRIC_GAUGE( nicMetricName, nicMetrics.inboundOctets, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( nicMetrics.inboundOctets ), nicMetricName, CORE::CString::Empty );
                    }
                    if ( m_gatherGlobalNetworkStatInboundUnicastOctets && nicMetrics.hasInboundUnicastOctets )
                    {
                        CORE::CString nicMetricName = nicMetricNamePrefix + nic->GetAdapterName() + ".InboundUnicastOctets";
                        GUCEF_METRIC_GAUGE( nicMetricName, nicMetrics.inboundUnicastOctets, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( nicMetrics.inboundUnicastOctets ), nicMetricName, CORE::CString::Empty );
                    }
                    if ( m_gatherGlobalNetworkStatInboundUnicastPackets && nicMetrics.hasInboundUnicastPackets )
                    {
                        CORE::CString nicMetricName = nicMetricNamePrefix + nic->GetAdapterName() + ".InboundUnicastPackets";
                        GUCEF_METRIC_GAUGE( nicMetricName, nicMetrics.inboundUnicastPackets, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( nicMetrics.inboundUnicastPackets ), nicMetricName, CORE::CString::Empty );
                    }
                    if ( m_gatherGlobalNetworkStatInboundNonUnicastPackets && nicMetrics.hasInboundNonUnicastPackets )
                    {
                        CORE::CString nicMetricName = nicMetricNamePrefix + nic->GetAdapterName() + ".InboundNonUnicastPackets";
                        GUCEF_METRIC_GAUGE( nicMetricName, nicMetrics.inboundNonUnicastPackets, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( nicMetrics.inboundNonUnicastPackets ), nicMetricName, CORE::CString::Empty );
                    }
                    if ( m_gatherGlobalNetworkStatInboundErroredPackets && nicMetrics.hasInboundErroredPackets )
                    {
                        CORE::CString nicMetricName = nicMetricNamePrefix + nic->GetAdapterName() + ".InboundErroredPackets";
                        GUCEF_METRIC_GAUGE( nicMetricName, nicMetrics.inboundErroredPackets, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( nicMetrics.inboundErroredPackets ), nicMetricName, CORE::CString::Empty );
                    }
                    if ( m_gatherGlobalNetworkStatInboundDiscardedPackets && nicMetrics.hasInboundDiscardedPackets )
                    {
                        CORE::CString nicMetricName = nicMetricNamePrefix + nic->GetAdapterName() + ".InboundDiscardedPackets";
                        GUCEF_METRIC_GAUGE( nicMetricName, nicMetrics.inboundDiscardedPackets, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( nicMetrics.inboundDiscardedPackets ), nicMetricName, CORE::CString::Empty );
                    }
                    if ( m_gatherGlobalNetworkStatOutboundOctets && nicMetrics.hasOutboundOctets )
                    {
                        CORE::CString nicMetricName = nicMetricNamePrefix + nic->GetAdapterName() + ".OutboundOctets";
                        GUCEF_METRIC_GAUGE( nicMetricName, nicMetrics.outboundOctets, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( nicMetrics.outboundOctets ), nicMetricName, CORE::CString::Empty );
                    }
                    if ( m_gatherGlobalNetworkStatOutboundUnicastOctets && nicMetrics.hasOutboundUnicastOctets )
                    {
                        CORE::CString nicMetricName = nicMetricNamePrefix + nic->GetAdapterName() + ".OutboundUnicastOctets";
                        GUCEF_METRIC_GAUGE( nicMetricName, nicMetrics.outboundUnicastOctets, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( nicMetrics.outboundUnicastOctets ), nicMetricName, CORE::CString::Empty );
                    }
                    if ( m_gatherGlobalNetworkStatOutboundUnicastPackets && nicMetrics.hasOutboundUnicastPackets )
                    {
                        CORE::CString nicMetricName = nicMetricNamePrefix + nic->GetAdapterName() + ".OutboundUnicastPackets";
                        GUCEF_METRIC_GAUGE( nicMetricName, nicMetrics.outboundUnicastPackets, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( nicMetrics.outboundUnicastPackets ), nicMetricName, CORE::CString::Empty );
                    }
                    if ( m_gatherGlobalNetworkStatOutboundNonUnicastPackets && nicMetrics.hasOutboundNonUnicastPackets )
                    {
                        CORE::CString nicMetricName = nicMetricNamePrefix + nic->GetAdapterName() + ".OutboundNonUnicastPackets";
                        GUCEF_METRIC_GAUGE( nicMetricName, nicMetrics.outboundNonUnicastPackets, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( nicMetrics.outboundNonUnicastPackets ), nicMetricName, CORE::CString::Empty );
                    }
                    if ( m_gatherGlobalNetworkStatOutboundErroredPackets && nicMetrics.hasOutboundErroredPackets )
                    {
                        CORE::CString nicMetricName = nicMetricNamePrefix + nic->GetAdapterName() + ".OutboundErroredPackets";
                        GUCEF_METRIC_GAUGE( nicMetricName, nicMetrics.outboundErroredPackets, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( nicMetrics.outboundErroredPackets ), nicMetricName, CORE::CString::Empty );
                    }
                    if ( m_gatherGlobalNetworkStatOutboundDiscardedPackets && nicMetrics.hasOutboundDiscardedPackets )
                    {
                        CORE::CString nicMetricName = nicMetricNamePrefix + nic->GetAdapterName() + ".OutboundDiscardedPackets";
                        GUCEF_METRIC_GAUGE( nicMetricName, nicMetrics.outboundDiscardedPackets, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( nicMetrics.outboundDiscardedPackets ), nicMetricName, CORE::CString::Empty );
                    }
                    if ( m_gatherGlobalNetworkStatInboundMulticastOctets && nicMetrics.hasInboundMulticastOctets )
                    {
                        CORE::CString nicMetricName = nicMetricNamePrefix + nic->GetAdapterName() + ".InboundMulticastOctets";
                        GUCEF_METRIC_GAUGE( nicMetricName, nicMetrics.inboundMulticastOctets, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( nicMetrics.inboundMulticastOctets ), nicMetricName, CORE::CString::Empty );
                    }
                    if ( m_gatherGlobalNetworkStatOutboundMulticastOctets && nicMetrics.hasOutboundMulticastOctets )
                    {
                        CORE::CString nicMetricName = nicMetricNamePrefix + nic->GetAdapterName() + ".OutboundMulticastOctets";
                        GUCEF_METRIC_GAUGE( nicMetricName, nicMetrics.outboundMulticastOctets, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( nicMetrics.outboundMulticastOctets ), nicMetricName, CORE::CString::Empty );
                    }
                    if ( m_gatherGlobalNetworkStatInboundBroadcastOctets && nicMetrics.hasInboundBroadcastOctets )
                    {
                        CORE::CString nicMetricName = nicMetricNamePrefix + nic->GetAdapterName() + ".InboundBroadcastOctets";
                        GUCEF_METRIC_GAUGE( nicMetricName, nicMetrics.inboundBroadcastOctets, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( nicMetrics.inboundBroadcastOctets ), nicMetricName, CORE::CString::Empty );
                    }
                    if ( m_gatherGlobalNetworkStatOutboundBroadcastOctets && nicMetrics.hasOutboundBroadcastOctets )
                    {
                        CORE::CString nicMetricName = nicMetricNamePrefix + nic->GetAdapterName() + ".OutboundBroadcastOctets";
                        GUCEF_METRIC_GAUGE( nicMetricName, nicMetrics.outboundBroadcastOctets, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( nicMetrics.outboundBroadcastOctets ), nicMetricName, CORE::CString::Empty );
                    }
                    if ( m_gatherGlobalNetworkStatTransmitLinkSpeedBitsPerSec && nicMetrics.hasTransmitLinkSpeedBitsPerSec )
                    {
                        CORE::CString nicMetricName = nicMetricNamePrefix + nic->GetAdapterName() + ".TransmitLinkSpeedBitsPerSec";
                        GUCEF_METRIC_GAUGE( nicMetricName, nicMetrics.transmitLinkSpeedBitsPerSec, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( nicMetrics.transmitLinkSpeedBitsPerSec ), nicMetricName, CORE::CString::Empty );
                    }
                    if ( m_gatherGlobalNetworkStatReceiveLinkSpeedBitsPerSec && nicMetrics.hasReceiveLinkSpeedBitsPerSec )
                    {
                        CORE::CString nicMetricName = nicMetricNamePrefix + nic->GetAdapterName() + "ReceiveLinkSpeedBitsPerSec";
                        GUCEF_METRIC_GAUGE( nicMetricName, nicMetrics.receiveLinkSpeedBitsPerSec, 1.0f );
                        ValidateMetricThresholds( CORE::CVariant( nicMetrics.receiveLinkSpeedBitsPerSec ), nicMetricName, CORE::CString::Empty );
                    }                    
                }
                ++i;
            }
        }
    }

    if ( m_gatherGlobalStorageStats )
    {
        if ( m_storageVolumeIds.empty() )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Fetching storage volume list" );
            if ( CORE::GetAllFileSystemStorageVolumes( m_storageVolumeIds ) )
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Successfully obtained a list of " + CORE::ToString( m_storageVolumeIds.size() ) + " storage volumes" );    
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Error trying to obtain a list of storage volumes. Obtained " + CORE::ToString( m_storageVolumeIds.size() ) );
                if ( m_storageVolumeIds.empty() )
                {
                    m_gatherGlobalStorageStats = false;
                }
            }

            if ( m_convertStorageVolumeIdsToPaths )
            {
                CORE::CString::StringSet::iterator i = m_storageVolumeIds.begin();
                while ( i != m_storageVolumeIds.end() )
                {
                    const CORE::CString& storageVolumeId = (*i);
                    CORE::CString::StringSet storageVolumePathNames;

                    if ( CORE::GetAllFileSystemPathNamesForVolume( storageVolumeId, storageVolumePathNames ) )
                    {
                        CORE::CString::StringSet::iterator j = storageVolumePathNames.begin();
                        while ( j != storageVolumePathNames.end() )
                        {
                            const CORE::CString& pathName = (*j);
                            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Storage volume id: " + storageVolumeId + " has path name " + pathName );
                            ++j;
                        }
                        if ( !storageVolumePathNames.empty() )
                        {
                            const CORE::CString& pathName = *storageVolumePathNames.begin();
                            m_storageVolumeIdsToPaths[ storageVolumeId ] = pathName;
                        }
                        ++i;
                    }
                }
            }            
        }

        static const CORE::CString storageMetricNamePrefix = "ProcessMetrics.Storage.";
        
        CORE::CString::StringSet::iterator i = m_storageVolumeIds.begin();
        while ( i != m_storageVolumeIds.end() )
        {
            const CORE::CString& storageVolumeId = (*i);
            const CORE::CString& storagePath = m_storageVolumeIdsToPaths[ storageVolumeId ];

            CORE::TStorageVolumeInformation volumeInfo;
            if ( CORE::GetFileSystemStorageVolumeInformationByVolumeId( volumeInfo, storageVolumeId ) && 0 < volumeInfo.totalNumberOfBytes )
            {
                const CORE::CString* metricStrToUse = &storageVolumeId;
                if ( m_convertStorageVolumeIdsToPaths && !storagePath.IsNULLOrEmpty() )
                    metricStrToUse = &storagePath;

                CORE::CString metricsStorageVolumeId = GenerateMetricsFriendlyString( *metricStrToUse );
                
                if ( m_gatherGlobalStorageVolumeBytesAvailableToCaller )
                {
                    CORE::CString storageMetricName = storageMetricNamePrefix + metricsStorageVolumeId + ".FreeBytesAvailableToCaller";
                    GUCEF_METRIC_GAUGE( storageMetricName, volumeInfo.freeBytesAvailableToCaller, 1.0f );
                    ValidateMetricThresholds( CORE::CVariant( volumeInfo.freeBytesAvailableToCaller ), storageMetricName, CORE::CString::Empty );
                }
                if ( m_gatherGlobalStorageVolumeBytesAvailable )
                {
                    CORE::CString storageMetricName = storageMetricNamePrefix + metricsStorageVolumeId + ".TotalNumberOfFreeBytes";
                    GUCEF_METRIC_GAUGE( storageMetricName, volumeInfo.totalNumberOfFreeBytes, 1.0f );
                    ValidateMetricThresholds( CORE::CVariant( volumeInfo.totalNumberOfFreeBytes ), storageMetricName, CORE::CString::Empty );
                }
                if ( m_gatherGlobalStorageVolumeBytes )
                {
                    CORE::CString storageMetricName = storageMetricNamePrefix + metricsStorageVolumeId + ".TotalNumberOfBytes";
                    GUCEF_METRIC_GAUGE( storageMetricName, volumeInfo.totalNumberOfBytes, 1.0f );
                    ValidateMetricThresholds( CORE::CVariant( volumeInfo.totalNumberOfBytes ), storageMetricName, CORE::CString::Empty );
                }
                if ( m_gatherGlobalStorageVolumeAvailableToCallerPercentage )
                {
                    CORE::Float64 storageVolumeAvailableToCallerPercentage = volumeInfo.freeBytesAvailableToCaller / ( 0.01 * volumeInfo.totalNumberOfBytes );
                    CORE::CString storageMetricName = storageMetricNamePrefix + metricsStorageVolumeId + ".StorageVolumeAvailableToCallerPercentage";
                    GUCEF_METRIC_GAUGE( storageMetricName, storageVolumeAvailableToCallerPercentage, 1.0f );
                    ValidateMetricThresholds( CORE::CVariant( storageVolumeAvailableToCallerPercentage ), storageMetricName, CORE::CString::Empty );
                }
                if ( m_gatherGlobalStorageVolumeAvailablePercentage )
                {
                    CORE::Float64 storageVolumeAvailablePercentage = volumeInfo.totalNumberOfFreeBytes / ( 0.01 * volumeInfo.totalNumberOfBytes );
                    CORE::CString storageMetricName = storageMetricNamePrefix + metricsStorageVolumeId + ".StorageVolumeAvailablePercentage";
                    GUCEF_METRIC_GAUGE( storageMetricName, storageVolumeAvailablePercentage, 1.0f );
                    ValidateMetricThresholds( CORE::CVariant( storageVolumeAvailablePercentage ), storageMetricName, CORE::CString::Empty );
                }
            }
            ++i;
        }
    }

    RefreshPIDs( failedProcs );
}

/*-------------------------------------------------------------------------*/

CORE::CString
ProcessMetrics::GenerateMetricsFriendlyString( const CORE::CString& str )
{GUCEF_TRACE;

    // Let's avoid non-ASCII stumbling blocks and force the down to ASCII
    CORE::CAsciiString asciiMetricsFriendlyStr = str.ForceToAscii( '_' );

    // Replace special chars
    static const char disallowedChars[] = { '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '=', '+', ',', '.', '<', '>', '/', '?', '`', '~', '\\', '|', '{', '}', '[', ']', ';', ':', '\'', '\"' };
    asciiMetricsFriendlyStr = asciiMetricsFriendlyStr.ReplaceChars( disallowedChars, sizeof(disallowedChars)/sizeof(char), '_' );

    // Back to the platform wide string convention format
    CORE::CString metricsFriendlyStr = CORE::ToString( asciiMetricsFriendlyStr );
    return metricsFriendlyStr;
}

/*-------------------------------------------------------------------------*/

bool
ProcessMetrics::Start( void )
{GUCEF_TRACE;

    if ( m_enableRestApi )
    {
        if ( m_httpServer.Listen() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "ProcessMetrics: Opened REST API on port " + CORE::UInt16ToString( m_httpServer.GetPort() ) );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProcessMetrics: Failed to open REST API on port " + CORE::UInt16ToString( m_httpServer.GetPort() ) );
        }
    }

    if ( SetStandbyMode( false ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "ProcessMetrics: Startup completed successfully" );
        return true;
    }

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "ProcessMetrics: Startup failed" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
ProcessMetrics::SetStandbyMode( bool newModeIsStandby )
{GUCEF_TRACE;

    m_metricsTimer.SetEnabled( !newModeIsStandby );
    m_procIndexTimer.SetEnabled( !newModeIsStandby );

    if ( newModeIsStandby )
    {
        if ( !m_exeProcsToWatch.empty() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics:SetStandbyMode: Cleaning up process information" );
            TProcessIdMap::iterator m = m_exeProcsToWatch.begin();
            while ( m != m_exeProcsToWatch.end() )
            {
                CProcInfo& procInfo = (*m).second;                
                procInfo.Unlink();
                ++m;
            }
        }
    }
    else
    {
        RefreshPIDs();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
ProcessMetrics::IsGlobalStandbyEnabled( void ) const
{GUCEF_TRACE;

    return m_metricsTimer.GetEnabled();
}

/*-------------------------------------------------------------------------*/

bool
ProcessMetrics::LoadConfig( const CORE::CValueList& appConfig   ,
                            const CORE::CDataNode& globalConfig )
{GUCEF_TRACE;

    m_gatherCpuStats = CORE::StringToBool( appConfig.GetValueAlways( "gatherProcCPUStats" ), m_gatherCpuStats );
    m_enableRestApi = CORE::StringToBool( appConfig.GetValueAlways( "enableRestApi" ), m_enableRestApi );
    m_enableEventMsgPublishing = CORE::StringToBool( appConfig.GetValueAlways( "enableEventMsgPublishing" ), m_enableEventMsgPublishing );
    m_metricsTimer.SetInterval( CORE::StringToUInt32( appConfig.GetValueAlways( "metricsGatheringIntervalInMs" ), 1000 ) );
    m_exeMatchPidMatchThreshold = CORE::StringToUInt32( appConfig.GetValueAlways( "exeMatchPidMatchThreshold" ), m_exeMatchPidMatchThreshold );
    m_procIndexTimer.SetInterval( appConfig.GetValueAlways( "procScanIntervalInMs", 5000 ).AsUInt32( 5000, true ) );

    m_gatherMemStats = CORE::StringToBool( appConfig.GetValueAlways( "gatherProcMemStats" ), m_gatherMemStats );
    m_gatherProcPageFaultCountInBytes = CORE::StringToBool( appConfig.GetValueAlways( "gatherProcPageFaultCountInBytes" ), m_gatherProcPageFaultCountInBytes );
    m_gatherProcPageFileUsageInBytes = CORE::StringToBool( appConfig.GetValueAlways( "gatherProcPageFileUsageInBytes" ), m_gatherProcPageFileUsageInBytes );
    m_gatherProcPeakPageFileUsageInBytes = CORE::StringToBool( appConfig.GetValueAlways( "gatherProcPeakPageFileUsageInBytes" ), m_gatherProcPeakPageFileUsageInBytes );
    m_gatherProcPeakWorkingSetSizeInBytes = CORE::StringToBool( appConfig.GetValueAlways( "gatherProcPeakWorkingSetSizeInBytes" ), m_gatherProcPeakWorkingSetSizeInBytes );
    m_gatherProcWorkingSetSizeInBytes = CORE::StringToBool( appConfig.GetValueAlways( "gatherProcWorkingSetSizeInBytes" ), m_gatherProcWorkingSetSizeInBytes );

    m_gatherGlobMemStats = CORE::StringToBool( appConfig.GetValueAlways( "gatherGlobalMemStats" ), m_gatherGlobMemStats );
    m_gatherGlobalAvailablePageFileSizeInBytes = CORE::StringToBool( appConfig.GetValueAlways( "gatherGlobalAvailablePageFileSizeInBytes" ), m_gatherGlobalAvailablePageFileSizeInBytes );
    m_gatherGlobalPageFileUsageInBytes = CORE::StringToBool( appConfig.GetValueAlways( "gatherGlobalPageFileUsageInBytes" ), m_gatherGlobalPageFileUsageInBytes );
    m_gatherGlobalAvailableVirtualMemoryInBytes = CORE::StringToBool( appConfig.GetValueAlways( "gatherGlobalAvailableVirtualMemoryInBytes" ), m_gatherGlobalAvailableVirtualMemoryInBytes );
    m_gatherGlobalAvailExtendedVirtualMemoryInBytes = CORE::StringToBool( appConfig.GetValueAlways( "gatherGlobalAvailExtendedVirtualMemoryInBytes" ), m_gatherGlobalAvailExtendedVirtualMemoryInBytes );
    m_gatherGlobalMemoryLoadPercentage = CORE::StringToBool( appConfig.GetValueAlways( "gatherGlobalMemoryLoadPercentage" ), m_gatherGlobalMemoryLoadPercentage );
    m_gatherGlobalTotalPageFileSizeInBytes = CORE::StringToBool( appConfig.GetValueAlways( "gatherGlobalTotalPageFileSizeInBytes" ), m_gatherGlobalTotalPageFileSizeInBytes );
    m_gatherGlobalTotalPhysicalMemoryInBytes = CORE::StringToBool( appConfig.GetValueAlways( "gatherGlobalTotalPhysicalMemoryInBytes" ), m_gatherGlobalTotalPhysicalMemoryInBytes );
    m_gatherGlobalTotalVirtualMemoryInBytes = CORE::StringToBool( appConfig.GetValueAlways( "gatherGlobalTotalVirtualMemoryInBytes" ), m_gatherGlobalTotalVirtualMemoryInBytes );

    m_gatherProcCpuUptime = CORE::StringToBool( appConfig.GetValueAlways( "gatherProcCPUUptime" ), m_gatherProcCpuUptime );
    m_gatherProcCpuOverallPercentage = CORE::StringToBool( appConfig.GetValueAlways( "gatherProcCpuOverallPercentage" ), m_gatherProcCpuOverallPercentage );

    m_gatherGlobalCpuStats = appConfig.GetValueAlways( "gatherGlobalCpuStats", m_gatherGlobalCpuStats ).AsBool( m_gatherGlobalCpuStats, true );
    m_gatherGlobalCpuCurrentFrequencyInMhz = appConfig.GetValueAlways( "gatherGlobalCpuCurrentFrequencyInMhz", m_gatherGlobalCpuCurrentFrequencyInMhz ).AsBool( m_gatherGlobalCpuCurrentFrequencyInMhz, true );
    m_gatherGlobalCpuSpecMaxFrequencyInMhz = appConfig.GetValueAlways( "gatherGlobalCpuSpecMaxFrequencyInMhz", m_gatherGlobalCpuSpecMaxFrequencyInMhz ).AsBool( m_gatherGlobalCpuSpecMaxFrequencyInMhz, true );
    m_gatherGlobalCpuMaxFrequencyInMhz = appConfig.GetValueAlways( "gatherGlobalCpuMaxFrequencyInMhz", m_gatherGlobalCpuMaxFrequencyInMhz ).AsBool( m_gatherGlobalCpuMaxFrequencyInMhz, true );
    m_gatherGlobalCpuOverallPercentage = appConfig.GetValueAlways( "gatherGlobalCpuOverallPercentage", m_gatherGlobalCpuOverallPercentage ).AsBool( m_gatherGlobalCpuOverallPercentage, true );
    m_gatherGlobalCpuOverallPercentagePerCore = appConfig.GetValueAlways( "gatherGlobalCpuOverallPercentagePerCore", m_gatherGlobalCpuOverallPercentagePerCore ).AsBool( m_gatherGlobalCpuOverallPercentagePerCore, true );   

    m_gatherGlobalNetworkStats = appConfig.GetValueAlways( "gatherGlobalNetworkStats", m_gatherGlobalNetworkStats ).AsBool( m_gatherGlobalNetworkStats, true );
    m_gatherGlobalNetworkStatInboundOctets = appConfig.GetValueAlways( "gatherGlobalNetworkStatInboundOctets", m_gatherGlobalNetworkStatInboundOctets ).AsBool( m_gatherGlobalNetworkStatInboundOctets, true );
    m_gatherGlobalNetworkStatInboundUnicastOctets = appConfig.GetValueAlways( "gatherGlobalNetworkStatInboundUnicastOctets", m_gatherGlobalNetworkStatInboundUnicastOctets ).AsBool( m_gatherGlobalNetworkStatInboundUnicastOctets, true );
    m_gatherGlobalNetworkStatInboundUnicastPackets = appConfig.GetValueAlways( "gatherGlobalNetworkStatInboundUnicastPackets", m_gatherGlobalNetworkStatInboundUnicastPackets ).AsBool( m_gatherGlobalNetworkStatInboundUnicastPackets, true );
    m_gatherGlobalNetworkStatInboundErroredPackets = appConfig.GetValueAlways( "gatherGlobalNetworkStatInboundErroredPackets", m_gatherGlobalNetworkStatInboundErroredPackets ).AsBool( m_gatherGlobalNetworkStatInboundErroredPackets, true );
    m_gatherGlobalNetworkStatInboundDiscardedPackets = appConfig.GetValueAlways( "gatherGlobalNetworkStatInboundDiscardedPackets", m_gatherGlobalNetworkStatInboundDiscardedPackets ).AsBool( m_gatherGlobalNetworkStatInboundDiscardedPackets, true );
    m_gatherGlobalNetworkStatOutboundOctets = appConfig.GetValueAlways( "gatherGlobalNetworkStatOutboundOctets", m_gatherGlobalNetworkStatOutboundOctets ).AsBool( m_gatherGlobalNetworkStatOutboundOctets, true );
    m_gatherGlobalNetworkStatOutboundUnicastOctets = appConfig.GetValueAlways( "gatherGlobalNetworkStatOutboundUnicastOctets", m_gatherGlobalNetworkStatOutboundUnicastOctets ).AsBool( m_gatherGlobalNetworkStatOutboundUnicastOctets, true );
    m_gatherGlobalNetworkStatOutboundUnicastPackets = appConfig.GetValueAlways( "gatherGlobalNetworkStatOutboundUnicastPackets", m_gatherGlobalNetworkStatOutboundUnicastPackets ).AsBool( m_gatherGlobalNetworkStatOutboundUnicastPackets, true );
    m_gatherGlobalNetworkStatOutboundErroredPackets = appConfig.GetValueAlways( "gatherGlobalNetworkStatOutboundErroredPackets", m_gatherGlobalNetworkStatOutboundErroredPackets ).AsBool( m_gatherGlobalNetworkStatOutboundErroredPackets, true );
    m_gatherGlobalNetworkStatOutboundDiscardedPackets = appConfig.GetValueAlways( "gatherGlobalNetworkStatOutboundDiscardedPackets", m_gatherGlobalNetworkStatOutboundDiscardedPackets ).AsBool( m_gatherGlobalNetworkStatOutboundDiscardedPackets, true );    
    m_gatherGlobalNetworkStatInboundMulticastOctets = appConfig.GetValueAlways( "gatherGlobalNetworkStatInboundMulticastOctets", m_gatherGlobalNetworkStatInboundMulticastOctets ).AsBool( m_gatherGlobalNetworkStatInboundMulticastOctets, true );
    m_gatherGlobalNetworkStatOutboundMulticastOctets = appConfig.GetValueAlways( "gatherGlobalNetworkStatOutboundMulticastOctets", m_gatherGlobalNetworkStatOutboundMulticastOctets ).AsBool( m_gatherGlobalNetworkStatOutboundMulticastOctets, true );
    m_gatherGlobalNetworkStatInboundBroadcastOctets = appConfig.GetValueAlways( "gatherGlobalNetworkStatInboundBroadcastOctets", m_gatherGlobalNetworkStatInboundBroadcastOctets ).AsBool( m_gatherGlobalNetworkStatInboundBroadcastOctets, true );
    m_gatherGlobalNetworkStatOutboundBroadcastOctets = appConfig.GetValueAlways( "gatherGlobalNetworkStatOutboundBroadcastOctets", m_gatherGlobalNetworkStatOutboundBroadcastOctets ).AsBool( m_gatherGlobalNetworkStatOutboundBroadcastOctets, true );
    m_gatherGlobalNetworkStatOutboundNonUnicastPackets = appConfig.GetValueAlways( "gatherGlobalNetworkStatOutboundNonUnicastPackets", m_gatherGlobalNetworkStatOutboundNonUnicastPackets ).AsBool( m_gatherGlobalNetworkStatOutboundNonUnicastPackets, true );
    m_gatherGlobalNetworkStatInboundNonUnicastPackets = appConfig.GetValueAlways( "gatherGlobalNetworkStatInboundNonUnicastPackets", m_gatherGlobalNetworkStatInboundNonUnicastPackets ).AsBool( m_gatherGlobalNetworkStatInboundNonUnicastPackets, true );
    m_gatherGlobalNetworkStatTransmitLinkSpeedBitsPerSec = appConfig.GetValueAlways( "gatherGlobalNetworkStatTransmitLinkSpeedBitsPerSec", m_gatherGlobalNetworkStatTransmitLinkSpeedBitsPerSec ).AsBool( m_gatherGlobalNetworkStatTransmitLinkSpeedBitsPerSec, true );
    m_gatherGlobalNetworkStatReceiveLinkSpeedBitsPerSec = appConfig.GetValueAlways( "gatherGlobalNetworkStatReceiveLinkSpeedBitsPerSec", m_gatherGlobalNetworkStatReceiveLinkSpeedBitsPerSec ).AsBool( m_gatherGlobalNetworkStatReceiveLinkSpeedBitsPerSec, true );

    m_gatherGlobalStorageStats = appConfig.GetValueAlways( "gatherGlobalStorageStats", m_gatherGlobalStorageStats ).AsBool( m_gatherGlobalStorageStats, true );
    m_gatherGlobalStorageVolumeBytesAvailableToCaller = appConfig.GetValueAlways( "gatherGlobalStorageVolumeBytesAvailableToCaller", m_gatherGlobalStorageVolumeBytesAvailableToCaller ).AsBool( m_gatherGlobalStorageVolumeBytesAvailableToCaller, true );
    m_gatherGlobalStorageVolumeBytesAvailable= appConfig.GetValueAlways( "gatherGlobalStorageVolumeBytesAvailable", m_gatherGlobalStorageVolumeBytesAvailable ).AsBool( m_gatherGlobalStorageVolumeBytesAvailable, true );
    m_gatherGlobalStorageVolumeBytes = appConfig.GetValueAlways( "gatherGlobalStorageVolumeBytes", m_gatherGlobalStorageVolumeBytes ).AsBool( m_gatherGlobalStorageVolumeBytes, true );
    m_gatherGlobalStorageVolumeAvailableToCallerPercentage = appConfig.GetValueAlways( "gatherGlobalStorageVolumeAvailableToCallerPercentage", m_gatherGlobalStorageVolumeAvailableToCallerPercentage ).AsBool( m_gatherGlobalStorageVolumeAvailableToCallerPercentage, true );
    m_gatherGlobalStorageVolumeAvailablePercentage = appConfig.GetValueAlways( "gatherGlobalStorageVolumeAvailablePercentage", m_gatherGlobalStorageVolumeAvailablePercentage ).AsBool( m_gatherGlobalStorageVolumeAvailablePercentage, true );
    m_convertStorageVolumeIdsToPaths = appConfig.GetValueAlways( "convertStorageVolumeIdsToPaths", m_convertStorageVolumeIdsToPaths ).AsBool( m_convertStorageVolumeIdsToPaths, true );

    TStringSet exeProcsToWatch = appConfig.GetValueAlways( "exeProcsToWatch" ).AsString().ParseUniqueElements( ';', false );
    TStringSet::iterator i = exeProcsToWatch.begin();
    while ( i != exeProcsToWatch.end() )
    {
        CProcInfo& procInfo = m_exeProcsToWatch[ (*i) ];
        procInfo.exeName = (*i);
        procInfo.gatherMetrics = true;
        ++i;
    }
    TStringSet exeProcsToStart = appConfig.GetValueAlways( "exeProcsToStart" ).AsString().ParseUniqueElements( ';', false );
    i = exeProcsToStart.begin();
    while ( i != exeProcsToStart.end() )
    {
        CProcInfo& procInfo = m_exeProcsToWatch[ (*i) ];
        procInfo.exeName = (*i);
        procInfo.startIfNotRunning = true;
        ++i;
    }
    TStringSet exeProcsToRestart = appConfig.GetValueAlways( "exeProcsToRestart" ).AsString().ParseUniqueElements( ';', false );
    i = exeProcsToRestart.begin();
    while ( i != exeProcsToRestart.end() )
    {
        CProcInfo& procInfo = m_exeProcsToWatch[ (*i) ];
        procInfo.exeName = (*i);
        procInfo.restartIfStopsRunning = true;
        ++i;
    }

    if ( m_gatherGlobalCpuStats )
    {
        m_globalCpuDataPoint = CORE::CreateCpuDataPoint();
    }

    if ( m_enableEventMsgPublishing )
    {
        m_metricsThresholds.clear();
        m_procMetricsThresholds.clear();

        CORE::CDataNode::TConstDataNodeSet metricsThresholdsCfgs = globalConfig.FindChildrenOfType( "MetricThreshold", true );
        CORE::CDataNode::TConstDataNodeSet::const_iterator c = metricsThresholdsCfgs.begin();
        while ( c != metricsThresholdsCfgs.end() )
        {
            MetricThreshold threshold;
            if ( threshold.LoadConfig( *(*c) ) )
            {
                if ( threshold.IsValid() )
                {
                    if ( threshold.procFilter.empty() )
                    {
                        m_metricsThresholds[ threshold.metricName ] = threshold;
                    }
                    else
                    {
                        CORE::CString::StringSet::iterator n = threshold.procFilter.begin();
                        while ( n != threshold.procFilter.end() )
                        {
                            m_procMetricsThresholds[ (*n) ][ threshold.metricName ] = threshold;
                            ++n;
                        }
                    }
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Metric threshold has invalid entry" );
                    // continue best effort, at least we'd have metrics
                }
            }
            ++c;
        }

        m_thresholdNotificationPrimaryPayloadCodecType =  appConfig.GetValueAlways( "thresholdNotificationPrimaryPayloadCodecType", "json" );
        if ( !m_thresholdNotificationPrimaryPayloadCodecType.IsNULLOrEmpty() )
        {
            if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().IsRegistered( m_thresholdNotificationPrimaryPayloadCodecType ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Codec for encoding the threshold exceeded notifications is not registered. CodeType=" + m_thresholdNotificationPrimaryPayloadCodecType );
                // continue best effort, at least we'd have metrics
            }
        }

        if ( !SetupPubSubClient( globalConfig ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Failed set up Pub/Sub client. Event messaging will not be possible" );
            // continue best effort, at least we'd have metrics
        }
    }
    else
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProcessMetrics: Metric threshold event message publishing is not enabled per config and thus will not be available" );
    }

    if ( m_enableRestApi )
    {
        m_httpServer.SetPort( CORE::StringToUInt16( CORE::ResolveVars( appConfig.GetValueAlways( "restApiPort" ) ), 10000 ) );

        m_httpRouter.SetResourceMapping( "/info", RestApiProcessMetricsInfoResource::THTTPServerResourcePtr( new RestApiProcessMetricsInfoResource( this ) )  );
        m_httpRouter.SetResourceMapping( "/config/appargs", RestApiProcessMetricsInfoResource::THTTPServerResourcePtr( new RestApiProcessMetricsConfigResource( this, true ) )  );
        m_httpRouter.SetResourceMapping( "/config", RestApiProcessMetricsInfoResource::THTTPServerResourcePtr( new RestApiProcessMetricsConfigResource( this, false ) )  );
        m_httpRouter.SetResourceMapping(  appConfig.GetValueAlways( "restBasicHealthUri", "/health/basic" ), RestApiProcessMetricsInfoResource::THTTPServerResourcePtr( new WEB::CDummyHTTPServerResource() )  );

        if ( !m_taskManagementRsc.ConnectHttpRouting( m_httpRouter ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProcessMetrics: Failed to set up task management API" );
        }

        m_httpServer.GetRouterController()->AddRouterMapping( &m_httpRouter, "", "" );
    }

    m_appConfig = appConfig;
    m_globalConfig.Copy( globalConfig );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CValueList&
ProcessMetrics::GetAppConfig( void ) const
{
    return m_appConfig;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
ProcessMetrics::GetGlobalConfig( void ) const
{
    return m_globalConfig;
}

/*-------------------------------------------------------------------------*/
