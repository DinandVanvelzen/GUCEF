/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#include "gucefASSETSTORE_CAssetStoreServiceConfigSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetStoreServiceConfigSettings::GroupReprocessing = "Reprocessing";
const CORE::CString CAssetStoreServiceConfigSettings::GroupDownloads = "Downloads";
const CORE::CString CAssetStoreServiceConfigSettings::GroupService = "Service";
const CORE::CString CAssetStoreServiceConfigSettings::ReprocessPrefix = "reprocess.";
const CORE::CString CAssetStoreServiceConfigSettings::DownloadsPrefix = "downloads.";

/*-------------------------------------------------------------------------*/

namespace {

typedef CAssetSettingDescriptor D;

/** The members of the downloads that are registered, with a pointer to the member */
struct SDownloadKey
{
    const char* name;
    UInt32 CAssetStoreDownloadSettings::* member32;
    UInt64 CAssetStoreDownloadSettings::* member64;
};

const SDownloadKey DownloadKeys[] =
{
    { "maxConcurrentJobs", &CAssetStoreDownloadSettings::maxConcurrentJobs, GUCEF_NULL },
    { "maxConcurrentJobsPerProvider", &CAssetStoreDownloadSettings::maxConcurrentJobsPerProvider, GUCEF_NULL },
    { "maxAttempts", &CAssetStoreDownloadSettings::maxAttempts, GUCEF_NULL },
    { "retryBaseDelayInMs", &CAssetStoreDownloadSettings::retryBaseDelayInMs, GUCEF_NULL },
    { "retryMaxDelayInMs", &CAssetStoreDownloadSettings::retryMaxDelayInMs, GUCEF_NULL },
    { "progressIntervalInMs", &CAssetStoreDownloadSettings::progressIntervalInMs, GUCEF_NULL },
    { "connectTimeoutInMs", &CAssetStoreDownloadSettings::connectTimeoutInMs, GUCEF_NULL },
    { "maxBytesPerSecond", GUCEF_NULL, &CAssetStoreDownloadSettings::maxBytesPerSecond }
};
const UInt32 DownloadKeyCount = (UInt32) ( sizeof( DownloadKeys ) / sizeof( DownloadKeys[ 0 ] ) );

/** The members of the service itself that are registered */
struct SServiceKey
{
    const char* name;
    UInt32 CAssetStoreServiceConfig::* member32;
    bool CAssetStoreServiceConfig::* memberBool;
};

const SServiceKey ServiceKeys[] =
{
    { "downloadStagingKeepHours", &CAssetStoreServiceConfig::downloadStagingKeepHours, GUCEF_NULL },
    { "signInPromptSeconds", &CAssetStoreServiceConfig::signInPromptSeconds, GUCEF_NULL },
    { "resumeIngestsOnStart", GUCEF_NULL, &CAssetStoreServiceConfig::resumeIngestsOnStart },
    { "ingestResumeAttempts", &CAssetStoreServiceConfig::ingestResumeAttempts, GUCEF_NULL }
};
const UInt32 ServiceKeyCount = (UInt32) ( sizeof( ServiceKeys ) / sizeof( ServiceKeys[ 0 ] ) );

bool
Add( CAssetSettingsRegistry& registry, const CAssetSettingDescriptor& descriptor, CORE::CString& reason )
{GUCEF_TRACE;

    return registry.Register( descriptor, reason );
}

}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreServiceConfigSettings::RegisterDescriptors( CAssetSettingsRegistry& registry, CORE::CString& reason )
{GUCEF_TRACE;

    const CAssetReprocessSettings reprocess;
    const CAssetStoreDownloadSettings downloads;
    const CAssetStoreServiceConfig service;
    const CORE::CString& R = GroupReprocessing;
    const CORE::CString& L = GroupDownloads;
    const CORE::CString& S = GroupService;
    const CORE::CString rp = ReprocessPrefix;
    const CORE::CString dl = DownloadsPrefix;

    // the reprocessing of packs: the ranges of CAssetReprocessSettings::ApplyChanges(), applied by CLocalAssetStoreService::ApplyReprocessSettings()
    bool ok = Add( registry, D::MakeBool( rp + "enabled", R, reprocess.enabled, "The reprocessing operations are offered" ), reason );
    ok = ok && Add( registry, D::MakeUInt( rp + "maxConcurrentJobs", R, 0, 256, reprocess.maxConcurrentJobs, "jobs",
                                           "Reprocessing jobs that run at a time; the others wait their turn. 0: automatic, a quarter of the logical cores, 1 to 8" ), reason );
    ok = ok && Add( registry, D::MakeUInt( rp + "threadsPerJob", R, 0, 1024, reprocess.threadsPerJob, "threads",
                                           "Converter threads of one running job. 0: automatic, the cores that are not reserved divided by the jobs at a time" ), reason );
    ok = ok && Add( registry, D::MakeUInt( rp + "reservedCores", R, 0, 1024, reprocess.reservedCores, "cores",
                                           "Logical cores the automatic threads per job leave to the service itself" ), reason );
    ok = ok && Add( registry, D::MakeUInt( rp + "maxQueuedJobs", R, 0, 100000, reprocess.maxQueuedJobs, "jobs",
                                           "Reprocessing jobs that wait or run at a time; one more is refused as busy" ), reason );
    ok = ok && Add( registry, D::MakeUInt( rp + "maxJobsPerPackPerHour", R, 0, 1000000, reprocess.maxJobsPerPackPerHour, "jobs",
                                           "Reprocessing jobs of one pack in an hour for a machine caller; 0: no maximum" ), reason );
    ok = ok && Add( registry, D::MakeUInt( rp + "maxMachineJobsPerHour", R, 0, 1000000, reprocess.maxMachineJobsPerHour, "jobs",
                                           "Reprocessing jobs of machine callers together in an hour; 0: no maximum" ), reason );
    ok = ok && Add( registry, D::MakeUInt( rp + "maxMachineRedownloadsPerDay", R, 0, 1000000, reprocess.maxMachineRedownloadsPerDay, "downloads",
                                           "Downloads again of originals by machine callers together in a day; 0: no maximum" ), reason );
    ok = ok && Add( registry, D::MakeBool( rp + "limitOwner", R, reprocess.limitOwner,
                                           "The per pack and hourly limits apply to the devices with the role owner too" ), reason );
    ok = ok && Add( registry, D::MakeUInt( rp + "downloadTimeoutMinutes", R, 0, 525600, reprocess.downloadTimeoutMinutes, "minutes",
                                           "A download again that takes longer is cancelled" ), reason );
    ok = ok && Add( registry, D::MakeUInt( rp + "memoryBudgetMB", R, 0, 1073741824, reprocess.memoryBudgetMB, "MB",
                                           "The memory the running jobs may be estimated to need together. 0: automatic, 70 percent of the memory limit of the container or of the machine" ), reason );
    ok = ok && Add( registry, D::MakeUInt( rp + "memoryPerJobMB", R, 0, 1073741824, reprocess.memoryPerJobMB, "MB",
                                           "The least memory a job with a conversion step is estimated to need" ), reason );
    ok = ok && Add( registry, D::MakeUInt( rp + "memoryLargestAssetFactor", R, 0, 64, reprocess.memoryLargestAssetFactor, "x",
                                           "A job with a conversion step is estimated at this many times the largest asset of its pack when that is more; 0: memoryPerJobMB alone" ), reason );
    ok = ok && Add( registry, D::MakeUInt( rp + "maxConcurrentWritesPerLocation", R, 0, 256, reprocess.maxConcurrentWritesPerLocation, "jobs",
                                           "Running jobs that write to the same storage location at a time; 0: no limit (a network share may want 1 or 2)" ), reason );

    // the limits of the download manager, applied by CAssetStoreDownloadManager::SetSettings()
    ok = ok && Add( registry, D::MakeUInt( dl + "maxConcurrentJobs", L, 0, 256, downloads.maxConcurrentJobs, "downloads",
                                           "Downloads that run at once over all stores; 0: no limit" ), reason );
    ok = ok && Add( registry, D::MakeUInt( dl + "maxConcurrentJobsPerProvider", L, 0, 256, downloads.maxConcurrentJobsPerProvider, "downloads",
                                           "Downloads that run at once per store; 0: no limit" ), reason );
    ok = ok && Add( registry, D::MakeUInt( dl + "maxAttempts", L, 1, 100, downloads.maxAttempts, "attempts",
                                           "Attempts of a transfer that fails with a network error, a server error or a request to slow down" ), reason );
    ok = ok && Add( registry, D::MakeUInt( dl + "retryBaseDelayInMs", L, 0, 3600000, downloads.retryBaseDelayInMs, "ms",
                                           "The first wait after a failed attempt, doubled per attempt" ), reason );
    ok = ok && Add( registry, D::MakeUInt( dl + "retryMaxDelayInMs", L, 0, 86400000, downloads.retryMaxDelayInMs, "ms",
                                           "The longest wait between attempts, also the cap of a wait the store asks for" ), reason );
    ok = ok && Add( registry, D::MakeUInt( dl + "progressIntervalInMs", L, 0, 60000, downloads.progressIntervalInMs, "ms",
                                           "At most one progress event per download in this time" ), reason );
    ok = ok && Add( registry, D::MakeUInt( dl + "connectTimeoutInMs", L, 0, 600000, downloads.connectTimeoutInMs, "ms",
                                           "The time a connection may take; 0: no limit" ), reason );
    ok = ok && Add( registry, D::MakeUInt( dl + "maxBytesPerSecond", L, 0, (UInt64) 1000000 * 1000000, downloads.maxBytesPerSecond, "bytes/s",
                                           "The speed of one download; 0: no limit. Applies to the next transfer" ), reason );

    // the service itself
    ok = ok && Add( registry, D::MakeUInt( "downloadStagingKeepHours", S, 0, 8760, service.downloadStagingKeepHours, "hours",
                                           "A failed download keeps its staging folder this long for a retry; the next staging sweep applies it" ), reason );
    ok = ok && Add( registry, D::MakeUInt( "signInPromptSeconds", S, 30, 86400, service.signInPromptSeconds, "s",
                                           "How long a step of a sign in to a store waits for the person; applies to the next step" ).OwnerOnlyRead(), reason );
    ok = ok && Add( registry, D::MakeBool( "resumeIngestsOnStart", S, service.resumeIngestsOnStart,
                                           "At a start, the ingests that were left staged are resumed" ).AppliedAtRestart(), reason );
    ok = ok && Add( registry, D::MakeUInt( "ingestResumeAttempts", S, 0, 100, service.ingestResumeAttempts, "starts",
                                           "How many starts resume the same staged ingest" ).AppliedAtRestart(), reason );
    return ok;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreServiceConfigSettings::ReadValues( const CAssetSettingsRegistry& registry, const CAssetStoreServiceConfig& config, CORE::CString::StringMap& values )
{GUCEF_TRACE;

    values.clear();
    CORE::CDataNode reprocess( "reprocess", GUCEF_DATATYPE_OBJECT );
    CORE::CDataNodeSerializableSettings serializerSettings;
    config.reprocess.Serialize( reprocess, serializerSettings );
    const CAssetSettingsRegistry::TDescriptorVector& descriptors = registry.GetDescriptors();
    for ( size_t d=0; d<descriptors.size(); ++d )
    {
        const CAssetSettingDescriptor& descriptor = descriptors[ d ];
        CORE::CString text;
        bool found = false;
        if ( 0 == descriptor.key.HasSubstr( ReprocessPrefix ) )
        {
            const CORE::CVariant value = reprocess.GetAttributeValueOrChildValueByName( descriptor.key.CutChars( ReprocessPrefix.Length(), true ) );
            found = value.IsInitialized();
            text = value.AsString( CORE::CString::Empty, true );
        }
        else
        if ( 0 == descriptor.key.HasSubstr( DownloadsPrefix ) )
        {
            const CORE::CString name = descriptor.key.CutChars( DownloadsPrefix.Length(), true );
            for ( UInt32 k=0; k<DownloadKeyCount && !found; ++k )
            {
                if ( name != DownloadKeys[ k ].name )
                    continue;
                found = true;
                text = GUCEF_NULL != DownloadKeys[ k ].member32 ? CORE::ToString( config.downloads.*DownloadKeys[ k ].member32 )
                                                               : CORE::ToString( config.downloads.*DownloadKeys[ k ].member64 );
            }
        }
        else
        {
            for ( UInt32 k=0; k<ServiceKeyCount && !found; ++k )
            {
                if ( descriptor.key != ServiceKeys[ k ].name )
                    continue;
                found = true;
                text = GUCEF_NULL != ServiceKeys[ k ].member32 ? CORE::ToString( config.*ServiceKeys[ k ].member32 )
                                                              : CORE::CString( config.*ServiceKeys[ k ].memberBool ? "true" : "false" );
            }
        }
        if ( !found )
            continue;
        // the canonical form of the registry, so values compare as text
        CORE::CString normalized;
        CORE::CString problem;
        values[ descriptor.key ] = descriptor.Normalize( text, normalized, problem ) ? normalized : text;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreServiceConfigSettings::WriteValue( CAssetStoreServiceConfig& config, const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    if ( 0 == key.HasSubstr( ReprocessPrefix ) )
    {
        CORE::CDataNode change( "reprocess", GUCEF_DATATYPE_OBJECT );
        change.SetAttribute( key.CutChars( ReprocessPrefix.Length(), true ), value );
        CORE::CString::StringMap errors;
        return config.reprocess.ApplyChanges( change, errors );
    }
    if ( 0 == key.HasSubstr( DownloadsPrefix ) )
    {
        const CORE::CString name = key.CutChars( DownloadsPrefix.Length(), true );
        for ( UInt32 k=0; k<DownloadKeyCount; ++k )
        {
            if ( name != DownloadKeys[ k ].name )
                continue;
            if ( GUCEF_NULL != DownloadKeys[ k ].member32 )
                config.downloads.*DownloadKeys[ k ].member32 = CORE::StringToUInt32( value, config.downloads.*DownloadKeys[ k ].member32 );
            else
                config.downloads.*DownloadKeys[ k ].member64 = CORE::StringToUInt64( value, config.downloads.*DownloadKeys[ k ].member64 );
            return true;
        }
        return false;
    }
    for ( UInt32 k=0; k<ServiceKeyCount; ++k )
    {
        if ( key != ServiceKeys[ k ].name )
            continue;
        if ( GUCEF_NULL != ServiceKeys[ k ].member32 )
            config.*ServiceKeys[ k ].member32 = CORE::StringToUInt32( value, config.*ServiceKeys[ k ].member32 );
        else
            config.*ServiceKeys[ k ].memberBool = "true" == value;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
