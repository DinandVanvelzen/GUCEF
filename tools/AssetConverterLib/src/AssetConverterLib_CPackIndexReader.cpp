/*
 *  AssetConverterLib: reads a pack index document: inline or sharded, one reader for both
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

#include "AssetConverterLib_CPackIndexReader.h"

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

CPackIndexReader::CPackIndexReader( void )
    : m_message()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString& CPackIndexReader::GetMessage( void ) const { return m_message; }

/*-------------------------------------------------------------------------*/

CORE::CString
CPackIndexReader::ToString( EResult result )
{GUCEF_TRACE;

    switch ( result )
    {
        case RESULT_OK:                 return "ok";
        case RESULT_MISSING:            return "missing";
        case RESULT_CORRUPT:            return "corrupt";
        case RESULT_NOT_A_PACK_INDEX:   return "not a pack index";
        case RESULT_VERSION_TOO_NEW:    return "version too new";
    }
    return "unknown";
}

/*-------------------------------------------------------------------------*/

CPackIndexReader::EResult
CPackIndexReader::ReadManifest( const CORE::CString& indexFolder, CPackIndexDocument& document )
{GUCEF_TRACE;

    m_message.Clear();
    document.Clear();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString path = CPackIndexLocator::GetManifestPath( indexFolder );
    if ( !vfs.FileExists( path ) )
    {
        m_message = "there is no pack index at " + path;
        return RESULT_MISSING;
    }
    CORE::CDataNode node;
    if ( !CCanonicalJson::Load( path, node ) )
    {
        m_message = "the pack index " + path + " cannot be parsed";
        return RESULT_CORRUPT;
    }
    if ( CPackIndexDocument::FormatId != CPackIndexSerial::GetText( node, "format" ) )
    {
        m_message = "the file " + path + " is not a pack index (its format is " + CPackIndexSerial::GetText( node, "format" ) + ")";
        return RESULT_NOT_A_PACK_INDEX;
    }
    const UInt32 version = (UInt32) CPackIndexSerial::GetCount( node, "version" );
    if ( version > CPackIndexDocument::SchemaVersion )
    {
        m_message = "the pack index " + path + " has version " + CORE::UInt32ToString( version ) + ", this build reads up to version " +
                    CORE::UInt32ToString( CPackIndexDocument::SchemaVersion ) + ": use a newer build or build the index again";
        return RESULT_VERSION_TOO_NEW;
    }
    if ( !document.DeserializeManifest( node ) )
    {
        m_message = "the pack index " + path + " has no identity";
        return RESULT_CORRUPT;
    }

    // the hash is checked over the tree as read, so members this build does not know do not make a good document corrupt
    const CORE::CString stored = document.documentHash;
    node.DelAttribute( "documentHash" );
    node.DelChild( "produced" );
    if ( stored.IsNULLOrEmpty() || CCanonicalJson::Hash( node ) != stored )
    {
        m_message = "the pack index " + path + " does not match its documentHash";
        return RESULT_CORRUPT;
    }
    return RESULT_OK;
}

/*-------------------------------------------------------------------------*/

CPackIndexReader::EResult
CPackIndexReader::ReadShard( const CORE::CString& indexFolder, const CPackIndexShardInfo& shard, CPackIndexResource::TResourceVector& resources )
{GUCEF_TRACE;

    m_message.Clear();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString path = CPackIndexLocator::GetShardPath( indexFolder, shard.file );
    CORE::CDynamicBuffer bytes;
    if ( !vfs.FileExists( path ) || !vfs.LoadFile( bytes, path, "rb" ) )
    {
        m_message = "the shard " + path + " is not there";
        return RESULT_CORRUPT;
    }
    if ( CContentHash::HashBuffer( bytes.GetConstBufferPtr(), bytes.GetDataSize() ) != shard.sha256 )
    {
        m_message = "the shard " + path + " does not match its sha256";
        return RESULT_CORRUPT;
    }
    const CORE::CString text( static_cast< const char* >( bytes.GetConstBufferPtr() ), bytes.GetDataSize() );
    CORE::CDataNode node;
    if ( !CCanonicalJson::Parse( text, node ) )
    {
        m_message = "the shard " + path + " cannot be parsed";
        return RESULT_CORRUPT;
    }
    const CORE::CDataNode* list = node.FindChild( "resources" );
    if ( GUCEF_NULL == list )
    {
        m_message = "the shard " + path + " has no resources";
        return RESULT_CORRUPT;
    }
    resources.reserve( resources.size() + list->GetNrOfDirectChildNodes() );
    CORE::CDataNode::const_iterator i = list->ConstBegin();
    while ( i != list->ConstEnd() )
    {
        CPackIndexResource resource;
        if ( resource.Deserialize( *(*i) ) )
            resources.push_back( resource );
        ++i;
    }
    return RESULT_OK;
}

/*-------------------------------------------------------------------------*/

CPackIndexReader::EResult
CPackIndexReader::Read( const CORE::CString& indexFolder, CPackIndexDocument& document )
{GUCEF_TRACE;

    const EResult result = ReadManifest( indexFolder, document );
    if ( RESULT_OK != result )
        return result;
    for ( size_t i=0; i<document.shards.size(); ++i )
    {
        const EResult shardResult = ReadShard( indexFolder, document.shards[ i ], document.resources );
        if ( RESULT_OK != shardResult )
            return shardResult;
    }
    return RESULT_OK;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexReader::MarkBad( const CORE::CString& indexFolder )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString path = CPackIndexLocator::GetManifestPath( indexFolder );
    if ( !vfs.FileExists( path ) )
        return false;
    vfs.DeleteFile( path + ".bad", true );
    return vfs.MoveFile( path, path + ".bad", true );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
