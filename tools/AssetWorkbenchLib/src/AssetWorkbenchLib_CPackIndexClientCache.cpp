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


/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#include "AssetWorkbenchLib_CPackIndexClientCache.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CPackIndexClientCache::MaxPages;

/*-------------------------------------------------------------------------*/

CPackIndexClientCache::SManifest::SManifest( void )
    : indexKey()
    , documentHash()
    , stale( false )
    , needsCheck( false )
    , document()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPackIndexClientCache::SPage::SPage( void )
    : key()
    , packId()
    , revision( 0 )
    , total( 0 )
    , nextPageToken()
    , resources()
    , groups()
    , lastUsed( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPackIndexClientCache::CPackIndexClientCache( const CORE::CString& serviceId )
    : m_serviceId( serviceId )
    , m_diskFolder()
    , m_manifests()
    , m_pages()
    , m_clock( 0 )
    , m_memoryHits( 0 )
    , m_diskHits( 0 )
    , m_pageHits( 0 )
    , m_evictions( 0 )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CPackIndexClientCache::SetDiskFolder( const CORE::CString& vfsFolder )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_diskFolder = vfsFolder;
    if ( !m_diskFolder.IsNULLOrEmpty() )
    {
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        if ( !vfs.DirExists( m_diskFolder ) )
            vfs.CreateDir( m_diskFolder );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackIndexClientCache::GetDiskPath( const CORE::CString& packId ) const
{GUCEF_TRACE;

    return m_diskFolder + "/" + m_serviceId.ReplaceChar( ':', '_' ).ReplaceChar( '/', '_' ) + "." + packId.ReplaceChar( '/', '_' ).ReplaceChar( '\\', '_' ) + ".manifest.json";
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexClientCache::FindManifest( const CORE::CString& packId, SManifest& manifest ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TManifestMap::const_iterator i = m_manifests.find( packId );
    if ( i == m_manifests.end() )
        return false;
    manifest = (*i).second;
    ++m_memoryHits;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CPackIndexClientCache::PutManifest( const CORE::CString& packId, const SManifest& manifest )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_manifests[ packId ] = manifest;
    if ( m_diskFolder.IsNULLOrEmpty() )
        return;
    CORE::CDataNode node( "manifest", GUCEF_DATATYPE_OBJECT );
    if ( manifest.document.SerializeManifest( node, false, true ) )
        ASSETCONVERTER::CCanonicalJson::Store( GetDiskPath( packId ), node );
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexClientCache::LoadManifestFromDisk( const CORE::CString& packId, SManifest& manifest ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_diskFolder.IsNULLOrEmpty() )
        return false;
    const CORE::CString path = GetDiskPath( packId );
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().FileExists( path ) )
        return false;
    CORE::CDataNode node;
    if ( !ASSETCONVERTER::CCanonicalJson::Load( path, node ) )
        return false;
    SManifest loaded;
    if ( !loaded.document.DeserializeManifest( node ) )
        return false;
    loaded.indexKey = loaded.document.producer.indexKey;
    loaded.documentHash = loaded.document.documentHash;
    loaded.needsCheck = true;
    manifest = loaded;
    ++m_diskHits;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CPackIndexClientCache::MarkForCheck( const CORE::CString& packId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TManifestMap::iterator i = m_manifests.find( packId );
    if ( i != m_manifests.end() )
        (*i).second.needsCheck = true;
}

/*-------------------------------------------------------------------------*/

void
CPackIndexClientCache::MarkAllForCheck( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( TManifestMap::iterator i = m_manifests.begin(); i != m_manifests.end(); ++i )
        (*i).second.needsCheck = true;
}

/*-------------------------------------------------------------------------*/

void
CPackIndexClientCache::RemovePack( const CORE::CString& packId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_manifests.erase( packId );
    for ( size_t p=m_pages.size(); p>0; --p )
        if ( m_pages[ p-1 ].packId == packId )
            m_pages.erase( m_pages.begin() + ( p - 1 ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackIndexClientCache::MakeResourcesKey( const CORE::CString& packId, const CORE::CString& groupId, const CORE::CString& subGroup, const CORE::CString& pageToken, UInt32 maxEntries )
{GUCEF_TRACE;

    return "r\n" + packId + "\n" + groupId + "\n" + subGroup + "\n" + pageToken + "\n" + CORE::ToString( maxEntries );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackIndexClientCache::MakeGroupsKey( const CORE::CString& packId )
{GUCEF_TRACE;

    return "g\n" + packId;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexClientCache::FindPage( const CORE::CString& key, SPage& page )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( size_t p=0; p<m_pages.size(); ++p )
    {
        if ( m_pages[ p ].key != key )
            continue;
        m_pages[ p ].lastUsed = ++m_clock;
        page = m_pages[ p ];
        ++m_pageHits;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CPackIndexClientCache::PutPage( const SPage& page )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( size_t p=0; p<m_pages.size(); ++p )
    {
        if ( m_pages[ p ].key == page.key )
        {
            m_pages[ p ] = page;
            m_pages[ p ].lastUsed = ++m_clock;
            return;
        }
    }
    if ( m_pages.size() >= MaxPages )
    {
        // the page that was used the longest ago goes
        size_t oldest = 0;
        for ( size_t p=1; p<m_pages.size(); ++p )
            if ( m_pages[ p ].lastUsed < m_pages[ oldest ].lastUsed )
                oldest = p;
        m_pages.erase( m_pages.begin() + oldest );
        ++m_evictions;
    }
    m_pages.push_back( page );
    m_pages.back().lastUsed = ++m_clock;
}

/*-------------------------------------------------------------------------*/

void
CPackIndexClientCache::DropPagesOfPack( const CORE::CString& packId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( size_t p=m_pages.size(); p>0; --p )
        if ( m_pages[ p-1 ].packId == packId )
            m_pages.erase( m_pages.begin() + ( p - 1 ) );
}

/*-------------------------------------------------------------------------*/

void
CPackIndexClientCache::DropAllPages( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_pages.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CPackIndexClientCache::GetPageCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_pages.size();
}

/*-------------------------------------------------------------------------*/

UInt32 CPackIndexClientCache::GetMemoryManifestHitCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_memoryHits; }
UInt32 CPackIndexClientCache::GetDiskManifestHitCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_diskHits; }
UInt32 CPackIndexClientCache::GetPageHitCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_pageHits; }
UInt32 CPackIndexClientCache::GetEvictionCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_evictions; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
