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

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCADMISSION_H
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCADMISSION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPROFILE_H
#include "gucefASSETSTORE_CAssetLibrarySyncProfile.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPROFILE_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPROFILE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The admission rules of the library sync (library sync design, section 5.4), evaluated before every download a sync
 *  queues: the window of items in flight per store (halved on a rate limit answer, grown again after 20 successes), the
 *  ingest backlog (pending ingests, staged bytes), the caps per hour and day from a ledger of what was queued, and the free
 *  space of the volume of the storage location: an item of estimated size s is admitted only when
 *  free - inFlight x factor - s x factor >= reserve, with reserve the larger of reservePercent of the volume and
 *  reserveBytes. The free space comes from GetFileSystemStorageVolumeInformationByDirPath() of gucefCORE; a volume that
 *  cannot be read is "unknown" and the disk rule is skipped. A refused item waits: the reason says why. Every limit of 0
 *  is no maximum. Not thread safe: one job uses it on its thread.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetLibrarySyncAdmission
{
    public:

    /** What a sync has in flight when it asks to queue one more */
    struct GUCEF_ASSETSTORE_PUBLIC_CPP SLoad
    {
        UInt32 inFlightItems;           /**< queued, downloading or downloaded and not ingested */
        UInt32 pendingIngests;          /**< downloaded and not ingested */
        UInt64 inFlightBytes;           /**< estimate of every download in flight */
        UInt64 stagedBytes;             /**< estimate of the downloads that are done and not ingested */

        SLoad( void );
    };

    /** An entry of the ledger of the caps */
    struct GUCEF_ASSETSTORE_PUBLIC_CPP SLedgerEntry
    {
        Int64 queuedMs;
        UInt64 bytes;
    };
    typedef GUCEF::vector< SLedgerEntry > TLedgerVector;

    CAssetLibrarySyncAdmission( void );
    ~CAssetLibrarySyncAdmission();

    /** Takes the limits of the profile; 'downloadsPerStore' is what the download manager runs per store (the default window is max( 4, twice that )) */
    void Configure( const CAssetLibrarySyncProfile& profile, UInt32 downloadsPerStore );

    /** The folder whose volume holds the downloads; empty when not known (the disk rule is skipped) */
    void SetVolumePath( const CORE::CString& path );
    const CORE::CString& GetVolumePath( void ) const;

    /**
     *  The free and total bytes of the volume of the path, from gucefCORE (GetFileSystemStorageVolumeInformationByDirPath(), which
     *  answers for network mounts such as CIFS as well); false when the volume cannot be read or says nothing (no free bytes, no
     *  size): the caller then treats the space as unknown, which admits with a warning
     */
    static bool ReadVolume( const CORE::CString& path, UInt64& freeBytes, UInt64& totalBytes );

    /** A reader of volumes in place of the one of gucefCORE (tests: a full, small or unreadable volume); GUCEF_NULL restores gucefCORE */
    typedef bool ( *TVolumeReader )( const CORE::CString& path, UInt64& freeBytes, UInt64& totalBytes, void* context );
    static void SetVolumeReader( TVolumeReader reader, void* context );

    /**
     *  A manual download (the items a person selected) rather than a sync of the library: no reserve is kept and the factor of
     *  the converted copy is not applied, so only an item that does not fit on the volume at all is refused, and it is refused
     *  (Admit() says 'refused') instead of waiting
     */
    void SetManual( bool manual );

    /** The reserve of a volume of the size: the larger of the percentage and the bytes of the profile */
    UInt64 GetReserveBytes( UInt64 totalBytes ) const;

    /** The disk rule itself */
    static bool FitsOnDisk( UInt64 freeBytes, UInt64 inFlightBytes, UInt64 itemBytes, Float64 factor, UInt64 reserveBytes );

    /** The number of items in flight at a time now: the configured window, smaller after rate limit answers */
    UInt32 GetWindow( void ) const;
    UInt32 GetConfiguredWindow( void ) const;

    /** A rate limit answer of the store halves the window (down to 1) */
    void NoteRateLimited( void );

    /** A download that went well; after 20 in a row the window grows by one up to the configured size */
    void NoteSuccess( void );

    /** A download was queued: it counts for the caps */
    void NoteQueued( Int64 nowMs, UInt64 bytes );

    /** The ledger of the caps (kept by the service over restarts in a later step) */
    const TLedgerVector& GetLedger( void ) const;
    void SetLedger( const TLedgerVector& ledger );

    /**
     *  Whether one more item of the estimated size may be queued now; when not, the reason in words and whether it is the
     *  disk (a waiting state of the job) rather than the window (a slot frees when a download ends)
     */
    bool Admit( UInt64 itemBytes, const SLoad& load, Int64 nowMs, CORE::CString& reason, bool& waiting );

    /** As above; 'refused' says that the item does not fit at all and is not to be waited for (manual downloads) */
    bool Admit( UInt64 itemBytes, const SLoad& load, Int64 nowMs, CORE::CString& reason, bool& waiting, bool& refused );

    private:

    void TrimLedger( Int64 nowMs );

    private:

    CAssetLibrarySyncProfile m_profile;
    UInt32 m_configuredWindow;
    UInt32 m_window;
    UInt32 m_successes;
    CORE::CString m_volumePath;
    bool m_volumeWarned;
    bool m_manual;
    TLedgerVector m_ledger;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCADMISSION_H ? */
