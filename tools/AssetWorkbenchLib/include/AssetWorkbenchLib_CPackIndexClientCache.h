/*
 *  AssetWorkbenchLib: the library of the asset workbench
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


#ifndef ASSETWORKBENCHLIB_CPACKINDEXCLIENTCACHE_H
#define ASSETWORKBENCHLIB_CPACKINDEXCLIENTCACHE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#include "AssetConverterLib_CPackIndexDocument.h"
#define ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a client of the pack index operations of the asset store service keeps (section 6.4 of the pack index design):
 *
 *  - the manifest of a pack in memory, and on the disk when a folder is set, under the index key: a document is immutable per
 *    index key, so a cached manifest is never wrong, only out of date, and it is asked again (GetPackIndex()) when the service
 *    says the index of the pack changed (MarkForCheck()) and kept when the key is the same. The manifest on the disk is what a pack
 *    opened without the service is shown from;
 *  - the pages of the resources and the group summaries in memory, the last MaxPages of them, each with the catalogue revision it
 *    was read at; the pages of a pack go when the service says the pack or its index changed (DropPagesOfPack()).
 *
 *  Thread safe.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CPackIndexClientCache
{
    public:

    static const UInt32 MaxPages = 32;

    /** The manifest of a pack as cached */
    struct SManifest
    {
        CORE::CString indexKey;
        CORE::CString documentHash;
        bool stale;
        bool needsCheck;                                    /**< the service said the index changed: ask again before trusting it */
        ASSETCONVERTER::CPackIndexDocument document;

        SManifest( void );
    };

    /** A page of records or of group summaries as cached */
    struct SPage
    {
        CORE::CString key;
        CORE::CString packId;
        UInt64 revision;
        UInt64 total;
        CORE::CString nextPageToken;
        ASSETCONVERTER::CPackIndexResource::TResourceVector resources;
        ASSETCONVERTER::CPackIndexGroup::TGroupVector groups;
        UInt64 lastUsed;

        SPage( void );
    };

    explicit CPackIndexClientCache( const CORE::CString& serviceId );

    /** The VFS folder of the manifests on the disk; empty for none (the default). The folder is made when it is not there */
    void SetDiskFolder( const CORE::CString& vfsFolder );

    // the manifests

    bool FindManifest( const CORE::CString& packId, SManifest& manifest ) const;
    void PutManifest( const CORE::CString& packId, const SManifest& manifest );
    /** What the disk holds of the pack; false when there is nothing (or no folder) */
    bool LoadManifestFromDisk( const CORE::CString& packId, SManifest& manifest ) const;
    void MarkForCheck( const CORE::CString& packId );
    void MarkAllForCheck( void );
    void RemovePack( const CORE::CString& packId );

    // the pages

    bool FindPage( const CORE::CString& key, SPage& page );
    void PutPage( const SPage& page );
    void DropPagesOfPack( const CORE::CString& packId );
    void DropAllPages( void );
    UInt32 GetPageCount( void ) const;

    /** The key of a page of the resources of a pack */
    static CORE::CString MakeResourcesKey( const CORE::CString& packId, const CORE::CString& groupId, const CORE::CString& subGroup, const CORE::CString& pageToken, UInt32 maxEntries );

    /** The key of the group summaries of a pack */
    static CORE::CString MakeGroupsKey( const CORE::CString& packId );

    // what the tests count

    UInt32 GetMemoryManifestHitCount( void ) const;
    UInt32 GetDiskManifestHitCount( void ) const;
    UInt32 GetPageHitCount( void ) const;
    UInt32 GetEvictionCount( void ) const;

    private:

    CORE::CString GetDiskPath( const CORE::CString& packId ) const;

    private:

    typedef GUCEF::map< CORE::CString, SManifest > TManifestMap;
    typedef GUCEF::vector< SPage > TPageVector;

    CORE::CString m_serviceId;
    CORE::CString m_diskFolder;
    TManifestMap m_manifests;
    TPageVector m_pages;
    UInt64 m_clock;
    mutable UInt32 m_memoryHits;
    mutable UInt32 m_diskHits;
    UInt32 m_pageHits;
    UInt32 m_evictions;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CPACKINDEXCLIENTCACHE_H ? */
