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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#include "gucefASSETSTORE_CAssetLibrarySyncAdmission.h"

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

namespace {

const Int64 HourMs = 60 * 60 * 1000;
const Int64 DayMs = 24 * HourMs;
const UInt32 SuccessesToGrow = 20;

CAssetLibrarySyncAdmission::TVolumeReader g_volumeReader = GUCEF_NULL;
void* g_volumeReaderContext = GUCEF_NULL;

CORE::CString
DescribeBytes( UInt64 bytes )
{
    const UInt64 mib = 1024 * 1024;
    if ( bytes >= 1024 * mib )
        return CORE::ToString( bytes / ( 1024 * mib ) ) + " GiB";
    if ( bytes >= mib )
        return CORE::ToString( bytes / mib ) + " MiB";
    return CORE::ToString( bytes ) + " bytes";
}

CORE::CString
DescribeTime( Int64 utcMs )
{
    CORE::CDateTime time;
    time.FromUnixEpochBasedTicksInMillisecs( (UInt64) utcMs );
    return time.ToIso8601DateTimeString( true, true );
}

}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncAdmission::SLoad::SLoad( void )
    : inFlightItems( 0 )
    , pendingIngests( 0 )
    , inFlightBytes( 0 )
    , stagedBytes( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncAdmission::CAssetLibrarySyncAdmission( void )
    : m_profile()
    , m_configuredWindow( 4 )
    , m_window( 4 )
    , m_successes( 0 )
    , m_volumePath()
    , m_volumeWarned( false )
    , m_manual( false )
    , m_ledger()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncAdmission::~CAssetLibrarySyncAdmission()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncAdmission::Configure( const CAssetLibrarySyncProfile& profile, UInt32 downloadsPerStore )
{GUCEF_TRACE;

    m_profile = profile;
    m_configuredWindow = profile.maxInFlightPerStore;
    if ( 0 == m_configuredWindow )
        m_configuredWindow = 2 * downloadsPerStore > 4 ? 2 * downloadsPerStore : 4;
    m_window = m_configuredWindow;
    m_successes = 0;
    m_volumeWarned = false;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncAdmission::SetVolumePath( const CORE::CString& path )
{GUCEF_TRACE;

    m_volumePath = path;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetLibrarySyncAdmission::GetVolumePath( void ) const
{GUCEF_TRACE;

    return m_volumePath;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncAdmission::ReadVolume( const CORE::CString& path, UInt64& freeBytes, UInt64& totalBytes )
{GUCEF_TRACE;

    freeBytes = 0;
    totalBytes = 0;
    if ( path.IsNULLOrEmpty() )
        return false;
    if ( GUCEF_NULL != g_volumeReader )
        return g_volumeReader( path, freeBytes, totalBytes, g_volumeReaderContext );
    CORE::CStorageVolumeInformation info;
    if ( !CORE::GetFileSystemStorageVolumeInformationByDirPath( info, path ) || !info.hasFreeBytesAvailableToCaller )
        return false;
    freeBytes = info.freeBytesAvailableToCaller;
    totalBytes = info.hasTotalNumberOfBytes ? info.totalNumberOfBytes : 0;
    // a file system that answers but knows nothing (no size and nothing free) is not read
    return 0 != totalBytes || 0 != freeBytes;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncAdmission::SetVolumeReader( TVolumeReader reader, void* context )
{GUCEF_TRACE;

    g_volumeReader = reader;
    g_volumeReaderContext = context;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncAdmission::SetManual( bool manual )
{GUCEF_TRACE;

    m_manual = manual;
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetLibrarySyncAdmission::GetReserveBytes( UInt64 totalBytes ) const
{GUCEF_TRACE;

    const UInt64 byPercent = ( totalBytes / 100 ) * m_profile.reservePercent;
    return byPercent > m_profile.reserveBytes ? byPercent : m_profile.reserveBytes;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncAdmission::FitsOnDisk( UInt64 freeBytes, UInt64 inFlightBytes, UInt64 itemBytes, Float64 factor, UInt64 reserveBytes )
{GUCEF_TRACE;

    const Float64 needed = ( (Float64) inFlightBytes + (Float64) itemBytes ) * factor + (Float64) reserveBytes;
    return (Float64) freeBytes >= needed;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetLibrarySyncAdmission::GetWindow( void ) const
{GUCEF_TRACE;

    return m_window;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetLibrarySyncAdmission::GetConfiguredWindow( void ) const
{GUCEF_TRACE;

    return m_configuredWindow;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncAdmission::NoteRateLimited( void )
{GUCEF_TRACE;

    m_window = m_window > 1 ? m_window / 2 : 1;
    m_successes = 0;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncAdmission::NoteSuccess( void )
{GUCEF_TRACE;

    if ( m_window >= m_configuredWindow )
        return;
    if ( ++m_successes >= SuccessesToGrow )
    {
        ++m_window;
        m_successes = 0;
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncAdmission::NoteQueued( Int64 nowMs, UInt64 bytes )
{GUCEF_TRACE;

    SLedgerEntry entry;
    entry.queuedMs = nowMs;
    entry.bytes = bytes;
    m_ledger.push_back( entry );
    TrimLedger( nowMs );
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncAdmission::TrimLedger( Int64 nowMs )
{GUCEF_TRACE;

    size_t keepFrom = 0;
    while ( keepFrom < m_ledger.size() && m_ledger[ keepFrom ].queuedMs <= nowMs - DayMs )
        ++keepFrom;
    if ( keepFrom > 0 )
        m_ledger.erase( m_ledger.begin(), m_ledger.begin() + keepFrom );
}

/*-------------------------------------------------------------------------*/

const CAssetLibrarySyncAdmission::TLedgerVector&
CAssetLibrarySyncAdmission::GetLedger( void ) const
{GUCEF_TRACE;

    return m_ledger;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncAdmission::SetLedger( const TLedgerVector& ledger )
{GUCEF_TRACE;

    m_ledger = ledger;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncAdmission::Admit( UInt64 itemBytes, const SLoad& load, Int64 nowMs, CORE::CString& reason, bool& waiting )
{GUCEF_TRACE;

    bool refused = false;
    return Admit( itemBytes, load, nowMs, reason, waiting, refused );
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncAdmission::Admit( UInt64 itemBytes, const SLoad& load, Int64 nowMs, CORE::CString& reason, bool& waiting, bool& refused )
{GUCEF_TRACE;

    reason.Clear();
    waiting = false;
    refused = false;

    // the window: a slot frees when an item in flight ends
    if ( load.inFlightItems >= m_window )
    {
        reason = "the window of " + CORE::ToString( m_window ) + " item(s) in flight is full";
        return false;
    }

    // the ingest backlog
    if ( m_profile.maxPendingIngests > 0 && load.pendingIngests >= m_profile.maxPendingIngests )
    {
        reason = "waiting for the ingest of " + CORE::ToString( load.pendingIngests ) + " download(s)";
        return false;
    }
    if ( m_profile.maxStagedBytes > 0 && load.stagedBytes > 0 && load.stagedBytes + itemBytes > m_profile.maxStagedBytes )
    {
        reason = "waiting for the ingest of " + DescribeBytes( load.stagedBytes ) + " that is staged";
        return false;
    }

    // the caps of the profile, over the ledger
    TrimLedger( nowMs );
    UInt32 inHour = 0;
    UInt64 bytesInDay = 0;
    Int64 oldestInHour = nowMs;
    for ( size_t e=0; e<m_ledger.size(); ++e )
    {
        bytesInDay += m_ledger[ e ].bytes;
        if ( m_ledger[ e ].queuedMs > nowMs - HourMs )
        {
            ++inHour;
            if ( m_ledger[ e ].queuedMs < oldestInHour )
                oldestInHour = m_ledger[ e ].queuedMs;
        }
    }
    if ( m_profile.maxItemsPerHour > 0 && inHour >= m_profile.maxItemsPerHour )
    {
        reason = "the cap of " + CORE::ToString( m_profile.maxItemsPerHour ) + " item(s) per hour is reached, resumes at " + DescribeTime( oldestInHour + HourMs );
        waiting = true;
        return false;
    }
    if ( m_profile.maxItemsPerDay > 0 && m_ledger.size() >= m_profile.maxItemsPerDay )
    {
        reason = "the daily cap of " + CORE::ToString( m_profile.maxItemsPerDay ) + " item(s) is reached, resumes at " + DescribeTime( m_ledger[ 0 ].queuedMs + DayMs );
        waiting = true;
        return false;
    }
    if ( m_profile.maxBytesPerDay > 0 && !m_ledger.empty() && bytesInDay + itemBytes > m_profile.maxBytesPerDay )
    {
        reason = "the daily cap of " + DescribeBytes( m_profile.maxBytesPerDay ) + " is reached, resumes at " + DescribeTime( m_ledger[ 0 ].queuedMs + DayMs );
        waiting = true;
        return false;
    }

    // the free space of the volume of the location
    UInt64 freeBytes = 0;
    UInt64 totalBytes = 0;
    if ( !ReadVolume( m_volumePath, freeBytes, totalBytes ) )
    {
        if ( !m_volumeWarned )
        {
            m_volumeWarned = true;
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncAdmission: The free space of the storage location cannot be read (" << m_volumePath << "), the disk rule is skipped" );
        }
        return true;
    }
    if ( m_manual )
    {
        // a manual download keeps no reserve: only an item that does not fit at all is refused, at once
        if ( !FitsOnDisk( freeBytes, load.inFlightBytes, itemBytes, 1.0, 0 ) )
        {
            reason = "not enough free space on the library volume: " + DescribeBytes( freeBytes ) + " free, " +
                     DescribeBytes( load.inFlightBytes + itemBytes ) + " needed";
            refused = true;
            return false;
        }
        return true;
    }
    const UInt64 reserve = GetReserveBytes( totalBytes );
    if ( !FitsOnDisk( freeBytes, load.inFlightBytes, itemBytes, m_profile.diskFactor, reserve ) )
    {
        reason = "waiting for disk space: " + DescribeBytes( freeBytes ) + " free, " + DescribeBytes( reserve ) + " kept in reserve, " +
                 DescribeBytes( (UInt64) ( ( (Float64) load.inFlightBytes + (Float64) itemBytes ) * m_profile.diskFactor ) ) + " needed";
        waiting = true;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
