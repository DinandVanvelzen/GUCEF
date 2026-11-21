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

#include <algorithm>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXREADER_H
#include "AssetConverterLib_CPackIndexReader.h"
#define ASSETCONVERTERLIB_CPACKINDEXREADER_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXREADER_H ? */

#include "AssetWorkbenchLib_CLocalPackIndexSource.h"

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

CLocalPackIndexSource::SPack::SPack( void )
    : folder()
    , hasRecords( false )
    , recordsHash()
    , records()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLocalPackIndexSource::CLocalPackIndexSource( void )
    : IPackIndexSource()
    , m_packs()
    , m_documentReads( 0 )
    , m_lastMessage()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLocalPackIndexSource::~CLocalPackIndexSource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CLocalPackIndexSource::SetPackFolder( const CORE::CString& packId, const CORE::CString& indexFolder )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    SPack& pack = m_packs[ packId ];
    pack = SPack();
    pack.folder = indexFolder;
}

/*-------------------------------------------------------------------------*/

void
CLocalPackIndexSource::RemovePack( const CORE::CString& packId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_packs.erase( packId );
}

/*-------------------------------------------------------------------------*/

void
CLocalPackIndexSource::Invalidate( const CORE::CString& packId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TPackMap::iterator i = m_packs.find( packId );
    if ( i != m_packs.end() )
    {
        (*i).second.hasRecords = false;
        (*i).second.records.clear();
        (*i).second.recordsHash.Clear();
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CLocalPackIndexSource::GetDocumentReadCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_documentReads;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLocalPackIndexSource::GetName( void ) const
{GUCEF_TRACE;

    return "documents on the disk";
}

/*-------------------------------------------------------------------------*/

bool
CLocalPackIndexSource::IsCheap( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLocalPackIndexSource::GetLastMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lastMessage;
}

/*-------------------------------------------------------------------------*/

IPackIndexSource::EResult
CLocalPackIndexSource::Fail( EResult result, const CORE::CString& message )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_lastMessage = message;
    return result;
}

/*-------------------------------------------------------------------------*/

IPackIndexSource::EResult
CLocalPackIndexSource::ReadManifestOf( const CORE::CString& packId, ASSETCONVERTER::CPackIndexDocument& manifest, CORE::CString& folder )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        TPackMap::const_iterator i = m_packs.find( packId );
        if ( i == m_packs.end() )
            return Fail( RESULT_NOT_FOUND, "the source does not know the pack " + packId );
        folder = (*i).second.folder;
    }
    ASSETCONVERTER::CPackIndexReader reader;
    const ASSETCONVERTER::CPackIndexReader::EResult result = reader.ReadManifest( folder, manifest );
    if ( ASSETCONVERTER::CPackIndexReader::RESULT_OK == result )
        return RESULT_OK;
    return Fail( ASSETCONVERTER::CPackIndexReader::RESULT_MISSING == result ? RESULT_NOT_FOUND : RESULT_FAILED, reader.GetMessage() );
}

/*-------------------------------------------------------------------------*/

IPackIndexSource::EResult
CLocalPackIndexSource::GetManifest( const CORE::CString& packId, ASSETCONVERTER::CPackIndexDocument& manifest, bool& stale )
{GUCEF_TRACE;

    stale = false;
    CORE::CString folder;
    const EResult result = ReadManifestOf( packId, manifest, folder );
    if ( RESULT_OK == result )
        manifest.resources.clear();
    return result;
}

/*-------------------------------------------------------------------------*/

IPackIndexSource::EResult
CLocalPackIndexSource::GetGroups( const CORE::CString& packId, ASSETCONVERTER::CPackIndexGroup::TGroupVector& groups )
{GUCEF_TRACE;

    groups.clear();
    ASSETCONVERTER::CPackIndexDocument manifest;
    CORE::CString folder;
    const EResult result = ReadManifestOf( packId, manifest, folder );
    if ( RESULT_OK == result )
        groups = manifest.groups;
    return result;
}

/*-------------------------------------------------------------------------*/

IPackIndexSource::EResult
CLocalPackIndexSource::GetResources( const CORE::CString& packId, const CORE::CString& groupId, const CORE::CString& subGroup,
                                     const CORE::CString& pageToken, UInt32 maxEntries,
                                     ASSETCONVERTER::CPackIndexResource::TResourceVector& page, CORE::CString& nextPageToken, UInt64& total )
{GUCEF_TRACE;

    page.clear();
    nextPageToken.Clear();
    total = 0;
    ASSETCONVERTER::CPackIndexDocument manifest;
    CORE::CString folder;
    EResult result = ReadManifestOf( packId, manifest, folder );
    if ( RESULT_OK != result )
        return result;

    // the records of the document: read once, again when the manifest says another document
    {
        MT::CScopeMutex lock( m_lock );
        SPack& pack = m_packs[ packId ];
        if ( !pack.hasRecords || pack.recordsHash != manifest.documentHash )
        {
            ASSETCONVERTER::CPackIndexDocument full;
            ASSETCONVERTER::CPackIndexReader reader;
            if ( ASSETCONVERTER::CPackIndexReader::RESULT_OK != reader.Read( folder, full ) )
            {
                m_lastMessage = reader.GetMessage();
                return RESULT_FAILED;
            }
            pack.records = full.resources;
            std::sort( pack.records.begin(), pack.records.end() );
            pack.hasRecords = true;
            pack.recordsHash = manifest.documentHash;
            ++m_documentReads;
        }
        const UInt32 first = pageToken.IsNULLOrEmpty() ? 0 : CORE::StringToUInt32( pageToken, 0 );
        const UInt32 count = 0 == maxEntries ? DefaultPageSize : maxEntries;
        UInt32 seen = 0;
        UInt32 taken = 0;
        bool more = false;
        for ( size_t r=0; r<pack.records.size(); ++r )
        {
            const ASSETCONVERTER::CPackIndexResource& record = pack.records[ r ];
            if ( !groupId.IsNULLOrEmpty() && record.group != groupId )
                continue;
            if ( !groupId.IsNULLOrEmpty() && !subGroup.IsNULLOrEmpty() && record.subGroup != subGroup )
                continue;
            if ( seen >= first )
            {
                if ( taken < count )
                {
                    page.push_back( record );
                    ++taken;
                }
                else
                    more = true;
            }
            ++seen;
        }
        total = seen;
        if ( more )
            nextPageToken = CORE::UInt32ToString( first + count );
    }
    return RESULT_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
