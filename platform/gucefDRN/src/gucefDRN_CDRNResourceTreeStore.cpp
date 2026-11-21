/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CRESOURCEMETADATA_H
#include "gucefCORE_CResourceMetaData.h"
#define GUCEF_CORE_CRESOURCEMETADATA_H
#endif /* GUCEF_CORE_CRESOURCEMETADATA_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_DRN_CDRNWIREWRITER_H
#include "gucefDRN_CDRNWireWriter.h"
#define GUCEF_DRN_CDRNWIREWRITER_H
#endif /* GUCEF_DRN_CDRNWIREWRITER_H ? */

#ifndef GUCEF_DRN_CDRNWIREREADER_H
#include "gucefDRN_CDRNWireReader.h"
#define GUCEF_DRN_CDRNWIREREADER_H
#endif /* GUCEF_DRN_CDRNWIREREADER_H ? */

#include "gucefDRN_CDRNResourceTreeStore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char TreeMagic[ 8 ] = { 'G', 'U', 'D', 'R', 'N', 'R', 'T', '1' };
static const char TreeFileExtension[] = ".drntree";

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A tree file met while pruning */
class CTreeFileEntry
{
    public:

    CORE::CString path;
    UInt64 bytes;
    UInt64 writtenMs;

    CTreeFileEntry( void ) : path(), bytes( 0 ), writtenMs( 0 ) {}
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CDRNResourceTreeStore::GetTreeFilePath( const CORE::CString& directory, const CORE::CString& provider, const CDRNResourceInfo& info )
{GUCEF_TRACE;

    CORE::CString key = provider + "\n" + info.name + "\n" + info.revision;
    UInt8 digest[ CDRNResourceHasher::DigestSize ];
    CDRNResourceHasher hasher;
    hasher.Hash( key.C_String(), key.ByteSize() > 0 ? key.ByteSize() - 1 : 0, digest );
    return CORE::CombinePath( directory, CDRNCrypto::ToHex( digest, 16 ) + TreeFileExtension );
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceTreeStore::Save( const CORE::CString& directory, const CORE::CString& provider, const CDRNResourceInfo& info, const CDRNResourceHashTree& tree )
{GUCEF_TRACE;

    if ( directory.IsNULLOrEmpty() || !tree.IsComplete() || tree.GetResourceSize() != info.size )
        return false;
    if ( !CORE::DirExists( directory ) && !CORE::CreateDirs( directory ) )
        return false;

    CORE::CDynamicBuffer content;
    CDRNWireWriter writer( content );
    writer.WriteBytes( TreeMagic, sizeof( TreeMagic ) );
    writer.WriteUInt8( FormatVersion );
    writer.WriteString( provider );
    writer.WriteString( info.name );
    writer.WriteVarUInt64( info.size );
    writer.WriteVarUInt64( info.modifiedUtcMs );
    writer.WriteString( info.revision );
    writer.WriteVarUInt64( tree.GetLeafCount() );
    UInt8 leaf[ CDRNResourceHashTree::HashSize ];
    for ( UInt64 i=0; i<tree.GetLeafCount(); ++i )
    {
        if ( !tree.GetLeafHash( i, leaf ) )
            return false;
        writer.WriteBytes( leaf, sizeof( leaf ) );
    }
    UInt8 checksum[ CDRNResourceHasher::DigestSize ];
    CDRNResourceHasher hasher;
    hasher.Hash( content.GetConstBufferPtr(), writer.GetOffset(), checksum );
    writer.WriteBytes( checksum, sizeof( checksum ) );
    content.SetDataSize( writer.GetOffset() );
    return CORE::WriteFileAtomically( GetTreeFilePath( directory, provider, info ), content );
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceTreeStore::Load( const CORE::CString& directory, const CORE::CString& provider, const CDRNResourceInfo& info, CDRNResourceHashTree& tree )
{GUCEF_TRACE;

    if ( directory.IsNULLOrEmpty() )
        return false;
    CORE::CString path = GetTreeFilePath( directory, provider, info );
    if ( !CORE::FileExists( path ) )
        return false;
    CORE::CDynamicBuffer content;
    if ( !content.LoadContentFromFile( path ) || content.GetDataSize() < sizeof( TreeMagic ) + CDRNResourceHasher::DigestSize )
        return false;

    UInt32 contentSize = content.GetDataSize() - CDRNResourceHasher::DigestSize;
    UInt8 checksum[ CDRNResourceHasher::DigestSize ];
    CDRNResourceHasher hasher;
    hasher.Hash( content.GetConstBufferPtr(), contentSize, checksum );
    if ( !CORE::CSha256::ConstantTimeEquals( checksum, static_cast< const UInt8* >( content.GetConstBufferPtr() ) + contentSize, sizeof( checksum ) ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNResourceTreeStore: The tree \"" << path << "\" is damaged and is not used" );
        return false;
    }

    CDRNWireReader reader( content.GetConstBufferPtr(), contentSize );
    char magic[ sizeof( TreeMagic ) ];
    UInt8 version = 0;
    CORE::CString storedProvider;
    CORE::CString storedName;
    UInt64 size = 0;
    UInt64 modifiedUtcMs = 0;
    CORE::CString revision;
    UInt64 leafCount = 0;
    if ( !reader.ReadBytes( magic, sizeof( magic ) ) || 0 != memcmp( magic, TreeMagic, sizeof( magic ) ) ||
         !reader.ReadUInt8( version ) || FormatVersion != version ||
         !reader.ReadString( storedProvider ) || !reader.ReadString( storedName ) || !reader.ReadVarUInt64( size ) ||
         !reader.ReadVarUInt64( modifiedUtcMs ) || !reader.ReadString( revision ) || !reader.ReadVarUInt64( leafCount ) )
        return false;
    if ( storedProvider != provider || storedName != info.name || size != info.size || modifiedUtcMs != info.modifiedUtcMs ||
         revision != info.revision || leafCount != CDRNResourceInfo::ChunkCountFor( size ) ||
         (UInt64) reader.GetRemaining() != leafCount * CDRNResourceHashTree::HashSize )
        return false;

    if ( !tree.Reset( size ) )
        return false;
    for ( UInt64 i=0; i<leafCount; ++i )
    {
        tree.SetLeafHash( i, reader.GetCurrentPtr() );
        reader.SkipBytes( CDRNResourceHashTree::HashSize );
    }
    return tree.Finish( hasher );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceTreeStore::Prune( const CORE::CString& directory, UInt64 maxBytes, UInt32 maxAgeSeconds )
{GUCEF_TRACE;

    if ( directory.IsNULLOrEmpty() || !CORE::DirExists( directory ) )
        return;

    GUCEF::vector< CTreeFileEntry > entries;
    UInt64 total = 0;
    UInt64 nowMs = CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();

    CORE::CFileSystemIterator iterator;
    if ( iterator.FindFirst( directory ) )
    {
        do
        {
            CORE::CString name = iterator.GetResourceName();
            if ( !iterator.IsAFile() || !name.EndsWith( TreeFileExtension ) )
                continue;
            CTreeFileEntry entry;
            entry.path = CORE::CombinePath( directory, name );
            entry.bytes = CORE::FileSize( entry.path );
            entry.writtenMs = CORE::GetFileModificationTime( entry.path ).ToUnixEpochBasedTicksInMillisecs();
            if ( 0 != maxAgeSeconds && entry.writtenMs + (UInt64) maxAgeSeconds * 1000 < nowMs )
            {
                CORE::DeleteFile( entry.path );
                continue;
            }
            total += entry.bytes;
            entries.push_back( entry );
        }
        while ( iterator.FindNext() );
        iterator.FindClose();
    }

    while ( total > maxBytes && !entries.empty() )
    {
        UInt32 oldest = 0;
        for ( UInt32 i=1; i<entries.size(); ++i )
            if ( entries[ i ].writtenMs < entries[ oldest ].writtenMs )
                oldest = i;
        CORE::DeleteFile( entries[ oldest ].path );
        total -= entries[ oldest ].bytes;
        entries.erase( entries.begin() + oldest );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
