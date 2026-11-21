/*
 *  AssetConverterLib: writes a pack index document in the canonical form, inline or sharded
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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXLOCATOR_H
#include "AssetConverterLib_CPackIndexLocator.h"
#define ASSETCONVERTERLIB_CPACKINDEXLOCATOR_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXLOCATOR_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#include "AssetConverterLib_CPackIndexSerial.h"
#define ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXSERIAL_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#ifndef ASSETCONVERTERLIB_CCONTENTHASH_H
#include "AssetConverterLib_CContentHash.h"
#define ASSETCONVERTERLIB_CCONTENTHASH_H
#endif /* ASSETCONVERTERLIB_CCONTENTHASH_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetConverterLib_CPackIndexWriter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPackIndexWriter::CPackIndexWriter( void )
    : m_shardSize( CPackIndexDocument::MaxInlineResources )
    , m_shardsWritten( 0 )
    , m_shardsReused( 0 )
    , m_error()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CPackIndexWriter::SetShardSize( UInt32 recordsPerShard )
{GUCEF_TRACE;

    m_shardSize = recordsPerShard > 0 ? recordsPerShard : CPackIndexDocument::MaxInlineResources;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& CPackIndexWriter::GetError( void ) const { return m_error; }
UInt32 CPackIndexWriter::GetShardsWritten( void ) const { return m_shardsWritten; }
UInt32 CPackIndexWriter::GetShardsReused( void ) const { return m_shardsReused; }

/*-------------------------------------------------------------------------*/

bool
CPackIndexWriter::StoreNode( const CORE::CDataNode& node, const CORE::CString& vfsPath )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CDynamicBuffer buffer( 4096, true );
    CCanonicalJson::ToJson( node, buffer );
    const CORE::CString folder = CORE::StripFilename( vfsPath );
    if ( !folder.IsNULLOrEmpty() )
        vfs.CreateDir( folder );

    // a temporary file and a rename: a reader never sees a half written file
    const CORE::CString temporary = vfsPath + ".tmp";
    if ( vfs.StoreAsFile( temporary, buffer, 0, true ) )
    {
        if ( vfs.MoveFileAtomically( temporary, vfsPath, true ) )
            return true;
        vfs.DeleteFile( temporary, true );
    }
    return vfs.StoreAsFile( vfsPath, buffer, 0, true );
}

/*-------------------------------------------------------------------------*/

void
CPackIndexWriter::RemoveObsoleteShards( const CORE::CString& indexFolder, const CPackIndexShardInfo::TShardVector& keep )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::CVFS::TStringVector listed;
    const CORE::CString shardFolder = CPackIndexLocator::GetShardFolder( indexFolder );
    vfs.GetFileList( listed, shardFolder, false, false, CORE::CString::Empty, 100000 );
    for ( size_t i=0; i<listed.size(); ++i )
    {
        const CORE::CString name = CVfsPaths::NameOf( listed[ i ] );
        if ( !name.WildcardEquals( "shard-*.json", '*', true ) )
            continue;
        bool kept = false;
        for ( size_t k=0; k<keep.size() && !kept; ++k )
            kept = keep[ k ].file == name;
        if ( !kept )
            vfs.DeleteFile( CPackIndexLocator::GetShardPath( indexFolder, name ), true );
    }
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexWriter::Write( CPackIndexDocument& document, const CORE::CString& indexFolder )
{GUCEF_TRACE;

    m_error.Clear();
    m_shardsWritten = 0;
    m_shardsReused = 0;
    if ( document.resources.size() > CPackIndexDocument::MaxResources )
    {
        m_error = "the pack holds " + CORE::UInt32ToString( (UInt32) document.resources.size() ) + " resources: more than the limit of " + CORE::UInt32ToString( CPackIndexDocument::MaxResources );
        return false;
    }
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    document.Normalise();
    document.shards.clear();

    // the shards: only for a large pack
    if ( document.resources.size() > CPackIndexDocument::MaxInlineResources )
    {
        size_t start = 0;
        UInt32 shardNumber = 0;
        while ( start < document.resources.size() )
        {
            // up to the shard size, and not over the border of a group
            const CORE::CString& group = document.resources[ start ].group;
            size_t end = start;
            while ( end < document.resources.size() && end - start < m_shardSize && document.resources[ end ].group == group )
                ++end;

            CORE::CDataNode shard;
            shard.SetNodeType( GUCEF_DATATYPE_OBJECT );
            shard.SetAttribute( "format", CORE::CString( "gucef.packIndex.shard" ) );
            shard.SetAttribute( "version", CPackIndexDocument::SchemaVersion );
            CORE::CDataNode* list = shard.AddChild( "resources", GUCEF_DATATYPE_ARRAY );
            for ( size_t r=start; r<end; ++r )
                document.resources[ r ].Serialize( *list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );

            CPackIndexShardInfo info;
            info.sha256 = CCanonicalJson::Hash( shard );
            ++shardNumber;
            CORE::CString number = CORE::UInt32ToString( shardNumber );
            while ( number.Length() < 4 )
                number = "0" + number;
            info.file = "shard-" + number + "-" + info.sha256.SubstrFromRange( 0, 12 ) + ".json";
            info.count = (UInt32) ( end - start );
            info.firstPath = document.resources[ start ].path;
            info.lastPath = document.resources[ end-1 ].path;
            info.group = group;
            const CORE::CString shardPath = CPackIndexLocator::GetShardPath( indexFolder, info.file );
            if ( vfs.FileExists( shardPath ) )
            {
                ++m_shardsReused;
            }
            else
            {
                if ( !StoreNode( shard, shardPath ) )
                {
                    m_error = "the shard " + shardPath + " cannot be written";
                    return false;
                }
                ++m_shardsWritten;
            }
            document.shards.push_back( info );
            start = end;
        }
    }

    document.version = CPackIndexDocument::SchemaVersion;
    document.documentHash = document.ComputeDocumentHash();
    CORE::CDataNode manifest;
    document.SerializeManifest( manifest, document.shards.empty(), true );
    if ( !StoreNode( manifest, CPackIndexLocator::GetManifestPath( indexFolder ) ) )
    {
        m_error = "the manifest " + CPackIndexLocator::GetManifestPath( indexFolder ) + " cannot be written";
        return false;
    }
    RemoveObsoleteShards( indexFolder, document.shards );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
