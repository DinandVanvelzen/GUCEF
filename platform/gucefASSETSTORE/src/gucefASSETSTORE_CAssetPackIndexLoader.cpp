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

#include <algorithm>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_COMCORE_CDATABASETRANSACTION_H
#include "gucefCOMCORE_CDatabaseTransaction.h"
#define GUCEF_COMCORE_CDATABASETRANSACTION_H
#endif /* GUCEF_COMCORE_CDATABASETRANSACTION_H ? */
#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */
#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#include "gucefASSETSTORE_CAssetStoreJson.h"
#define GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREJSON_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETDIGEST_H
#include "gucefASSETSTORE_CAssetDigest.h"
#define GUCEF_ASSETSTORE_CASSETDIGEST_H
#endif /* GUCEF_ASSETSTORE_CASSETDIGEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETKEYWORD_H
#include "gucefASSETSTORE_CAssetKeyword.h"
#define GUCEF_ASSETSTORE_CASSETKEYWORD_H
#endif /* GUCEF_ASSETSTORE_CASSETKEYWORD_H ? */

#include "gucefASSETSTORE_CAssetPackIndexLoader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetPackIndexLoader::FormatId = "gucef.packIndex";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const GroupOrderTable[] =
{
    "scenes", "characters", "static-models", "animations", "visual-effects", "materials", "textures", "audio", "prefabs", "shaders", "fonts", "other"
};
const UInt32 GroupOrderCount = sizeof( GroupOrderTable ) / sizeof( GroupOrderTable[ 0 ] );
const UInt32 ShardResourcesHint = 2000;

Int64
NowMs( void )
{
    return (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

CORE::CString
TextOf( const CORE::CDataNode& node, const char* name )
{
    return node.GetAttributeValueOrChildValueByName( name ).AsString();
}

UInt64
CountOf( const CORE::CDataNode& node, const char* name )
{
    return node.GetAttributeValueOrChildValueByName( name ).AsUInt64( 0, true );
}

/** The state of a step of a record: "fresh", "stale", "missing", "failed", "refused", "skipped" or empty */
CORE::CString
StepOf( const CORE::CDataNode& statusNode, const char* step )
{
    const CORE::CDataNode* steps = statusNode.FindChild( "steps" );
    if ( GUCEF_NULL == steps )
        return CORE::CString::Empty;
    const CORE::CDataNode* stepNode = steps->FindChild( step );
    return GUCEF_NULL == stepNode ? CORE::CString::Empty : TextOf( *stepNode, "s" );
}

CORE::CString
LastSegment( const CORE::CString& path )
{
    return path.SubstrToChar( '/', false );
}

CORE::CVariant::VariantVector
ToVariants( const CORE::CString::StringSet& values )
{
    CORE::CVariant::VariantVector variants;
    for ( CORE::CString::StringSet::const_iterator i=values.begin(); i!=values.end(); ++i )
        variants.push_back( CORE::CVariant( (*i) ) );
    return variants;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/** One record of the document with what the loader needs of it beyond the row */
struct CAssetPackIndexLoader::SDocument
{
    struct SRecord
    {
        CAssetPackResource row;
        CORE::CString documentId;
        GUCEF::vector< std::pair< CORE::CString, CORE::CString > > refs;    /**< kind and the document id of the target */
        CORE::CString::StringVector refPaths;                               /**< per reference the path of the target in the pack, empty when the document has none */
        GUCEF::map< CORE::CString, CORE::CVariant > stats;
        CORE::CString::StringVector tags;
    };

    CORE::CDataNode manifest;
    CORE::CString documentHash;
    CORE::CString indexKey;
    CORE::CString packKey;
    CORE::CString levels;
    CORE::CString producer;
    UInt32 version;
    UInt32 shardCount;
    UInt64 noiseCount;
    Int64 producedMs;
    bool hasFingerprint;
    UInt64 fingerprintFiles;
    UInt64 fingerprintBytes;
    GUCEF::map< CORE::CString, CORE::CString > groupNames;      /**< group id, or group id and a line feed and the sub-group path, to the name */
    GUCEF::vector< SRecord > records;

    SDocument( void )
        : manifest( "manifest", GUCEF_DATATYPE_OBJECT )
        , documentHash(), indexKey(), packKey(), levels(), producer()
        , version( 0 ), shardCount( 0 ), noiseCount( 0 ), producedMs( 0 )
        , hasFingerprint( false ), fingerprintFiles( 0 ), fingerprintBytes( 0 )
        , groupNames(), records()
    {}
};

/*-------------------------------------------------------------------------*/

CAssetPackIndexLoader::CAssetPackIndexLoader( CAssetCatalogue& catalogue, const CAssetKeywordNormalizer* normalizer )
    : m_catalogue( catalogue )
    , m_normalizer( normalizer )
    , m_lastError()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetPackIndexLoader::MakeResourceId( const CORE::CString& packId, const CORE::CString& path )
{GUCEF_TRACE;

    CAssetDigest idDigest( CAssetDigest::Sha256 );
    const CORE::CString key = packId + "\n" + path;
    idDigest.Update( key.C_String(), key.Length() );
    return "as" + idDigest.FinalHex().SubstrToIndex( 20, true );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetPackIndexLoader::GetGroupOrder( const CORE::CString& groupId )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<GroupOrderCount; ++i )
        if ( groupId == GroupOrderTable[ i ] )
            return i;
    return GroupOrderCount;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackIndexLoader::IsAvailable( void ) const
{GUCEF_TRACE;

    return m_catalogue.IsOpen() && m_catalogue.GetSchemaVersion() >= 4;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetPackIndexLoader::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    return m_lastError;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackIndexLoader::Fail( TAssetStoreStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    m_lastError = message;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AssetPackIndexLoader: " << message );
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackIndexLoader::ReadDocument( const CORE::CString& manifestVfsPath, SDocument& document, CORE::CString& message ) const
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CDynamicBuffer bytes;
    if ( !vfs.FileExists( manifestVfsPath ) || !vfs.LoadFile( bytes, manifestVfsPath, "rb" ) )
    {
        message = "the pack index " + manifestVfsPath + " cannot be read";
        return ASSETSTORE_STATUS_NOT_FOUND;
    }
    const CORE::CString text( static_cast< const char* >( bytes.GetConstBufferPtr() ), bytes.GetDataSize() );
    CORE::CDataNode root;
    if ( !CAssetStoreJson::FromText( text, root ) )
    {
        message = "the pack index " + manifestVfsPath + " is not JSON";
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    }
    if ( FormatId != TextOf( root, "format" ) )
    {
        message = "the file " + manifestVfsPath + " is not a pack index (its format is \"" + TextOf( root, "format" ) + "\")";
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    }
    document.version = (UInt32) CountOf( root, "version" );
    if ( document.version > MaxSchemaVersion )
    {
        message = "the pack index " + manifestVfsPath + " has version " + CORE::ToString( document.version ) + ", this build reads up to version " + CORE::ToString( MaxSchemaVersion );
        return ASSETSTORE_STATUS_UNSUPPORTED;
    }
    const CORE::CDataNode* identity = root.FindChild( "identity" );
    const CORE::CDataNode* producer = root.FindChild( "producer" );
    if ( GUCEF_NULL == identity || TextOf( *identity, "packKey" ).IsNULLOrEmpty() )
    {
        message = "the pack index " + manifestVfsPath + " has no identity";
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    }
    if ( "failed" == TextOf( root, "state" ) )
    {
        message = "the pack index " + manifestVfsPath + " is the document of a build that failed";
        return ASSETSTORE_STATUS_FAILED;
    }
    document.packKey = TextOf( *identity, "packKey" );
    const CORE::CDataNode* fingerprint = identity->FindChild( "fingerprint" );
    if ( GUCEF_NULL != fingerprint )
    {
        document.fingerprintFiles = CountOf( *fingerprint, "fileCount" );
        document.fingerprintBytes = CountOf( *fingerprint, "totalBytes" );
        document.hasFingerprint = document.fingerprintFiles > 0;
    }
    if ( GUCEF_NULL != producer )
    {
        document.indexKey = TextOf( *producer, "indexKey" );
        document.producer = TextOf( *producer, "tool" );
        const CORE::CString toolVersion = TextOf( *producer, "toolVersion" );
        if ( !toolVersion.IsNULLOrEmpty() )
            document.producer += "/" + toolVersion;
        if ( producer->GetAttributeValueOrChildValueByName( "derivedFromCatalogue" ).AsBool( false, true ) )
            document.producer += " derived";
    }
    document.documentHash = TextOf( root, "documentHash" );
    if ( document.documentHash.IsNULLOrEmpty() )
    {
        // a document without its hash is identified by the bytes of its manifest
        CAssetDigest digest( CAssetDigest::Sha256 );
        digest.Update( bytes.GetConstBufferPtr(), bytes.GetDataSize() );
        document.documentHash = digest.FinalHex();
    }
    if ( document.indexKey.IsNULLOrEmpty() )
        document.indexKey = document.documentHash;
    {
        CORE::CString::StringVector levels;
        root.GetValuesOfChildByName( "levels", levels );
        document.levels = CAssetCatalogue::JoinLines( levels ).ReplaceChar( '\n', ',' );
    }
    const CORE::CDataNode* produced = root.FindChild( "produced" );
    document.producedMs = GUCEF_NULL != produced ? (Int64) CountOf( *produced, "producedMs" ) : 0;
    const CORE::CDataNode* counts = root.FindChild( "counts" );
    document.noiseCount = GUCEF_NULL != counts ? CountOf( *counts, "noise" ) : 0;

    // the group names of the manifest
    const CORE::CDataNode* groups = root.FindChild( "groups" );
    if ( GUCEF_NULL != groups )
    {
        CORE::CDataNode::const_iterator g = groups->ConstBegin();
        for ( ; g != groups->ConstEnd(); ++g )
        {
            const CORE::CString groupId = TextOf( *(*g), "id" );
            document.groupNames[ groupId ] = TextOf( *(*g), "name" );
            const CORE::CDataNode* subs = (*g)->FindChild( "subGroups" );
            if ( GUCEF_NULL == subs )
                continue;
            CORE::CDataNode::const_iterator s = subs->ConstBegin();
            for ( ; s != subs->ConstEnd(); ++s )
                document.groupNames[ groupId + "\n" + TextOf( *(*s), "id" ) ] = TextOf( *(*s), "name" );
        }
    }

    // the records: inline, or in the shards beside the manifest, each checked against its hash
    GUCEF::vector< const CORE::CDataNode* > lists;
    GUCEF::vector< CORE::CDataNode* > shardTrees;
    const CORE::CDataNode* inlineList = root.FindChild( "resources" );
    if ( GUCEF_NULL != inlineList )
        lists.push_back( inlineList );
    const CORE::CDataNode* shards = root.FindChild( "shards" );
    TAssetStoreStatus status = ASSETSTORE_STATUS_OK;
    if ( GUCEF_NULL != shards )
    {
        document.shardCount = (UInt32) shards->GetNrOfDirectChildNodes();
        const Int32 lastSlash = manifestVfsPath.HasChar( '/', false );
        const CORE::CString folder = lastSlash < 0 ? CORE::CString::Empty : manifestVfsPath.SubstrToIndex( (UInt32) lastSlash, true );
        CORE::CDataNode::const_iterator s = shards->ConstBegin();
        for ( ; s != shards->ConstEnd() && ASSETSTORE_STATUS_OK == status; ++s )
        {
            const CORE::CString file = TextOf( *(*s), "file" );
            const CORE::CString expected = TextOf( *(*s), "sha256" );
            if ( file.IsNULLOrEmpty() || file.HasChar( '/' ) >= 0 || file.HasSubstr( ".." ) >= 0 )
            {
                message = "the pack index " + manifestVfsPath + " names a shard with an unsafe file name";
                return ASSETSTORE_STATUS_INVALID_ARGUMENT;
            }
            if ( file.Length() > 3 && ".gz" == file.CutChars( file.Length() - 3, true ) )
            {
                message = "the shard " + file + " of the pack index " + manifestVfsPath + " is compressed: this build does not read compressed shards";
                return ASSETSTORE_STATUS_UNSUPPORTED;
            }
            const CORE::CString shardPath = folder + "/pack-index.d/" + file;
            CORE::CDynamicBuffer shardBytes;
            if ( !vfs.FileExists( shardPath ) || !vfs.LoadFile( shardBytes, shardPath, "rb" ) )
            {
                message = "the shard " + shardPath + " of the pack index is not there";
                return ASSETSTORE_STATUS_NOT_FOUND;
            }
            CAssetDigest shardDigest( CAssetDigest::Sha256 );
            shardDigest.Update( shardBytes.GetConstBufferPtr(), shardBytes.GetDataSize() );
            if ( !expected.IsNULLOrEmpty() && !CAssetDigest::AreEqual( shardDigest.FinalHex(), expected ) )
            {
                message = "the shard " + shardPath + " does not match its sha256";
                return ASSETSTORE_STATUS_DIGEST_MISMATCH;
            }
            CORE::CDataNode* tree = GUCEF_NEW CORE::CDataNode();
            shardTrees.push_back( tree );
            const CORE::CString shardText( static_cast< const char* >( shardBytes.GetConstBufferPtr() ), shardBytes.GetDataSize() );
            if ( !CAssetStoreJson::FromText( shardText, *tree ) )
            {
                message = "the shard " + shardPath + " cannot be parsed";
                status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
                break;
            }
            const CORE::CDataNode* shardList = tree->FindChild( "resources" );
            if ( GUCEF_NULL == shardList )
            {
                message = "the shard " + shardPath + " has no resources";
                status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
                break;
            }
            lists.push_back( shardList );
        }
    }
    UInt64 total = 0;
    for ( size_t l=0; l<lists.size(); ++l )
        total += lists[ l ]->GetNrOfDirectChildNodes();
    if ( ASSETSTORE_STATUS_OK == status && total > MaxResources )
    {
        message = "the pack index " + manifestVfsPath + " has " + CORE::UInt64ToString( total ) + " resources, the most this build takes is " + CORE::ToString( MaxResources );
        status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
    }
    if ( ASSETSTORE_STATUS_OK == status )
        document.records.reserve( (size_t) total );
    for ( size_t l=0; ASSETSTORE_STATUS_OK == status && l<lists.size(); ++l )
    {
        CORE::CDataNode::const_iterator r = lists[ l ]->ConstBegin();
        for ( ; r != lists[ l ]->ConstEnd(); ++r )
        {
            const CORE::CDataNode& node = *(*r);
            SDocument::SRecord record;
            record.documentId = TextOf( node, "id" );
            record.row.path = TextOf( node, "path" );
            if ( record.row.path.IsNULLOrEmpty() )
                continue;
            record.row.name = TextOf( node, "name" );
            record.row.title = TextOf( node, "title" );
            record.row.kind = TextOf( node, "kind" );
            record.row.storeKind = TextOf( node, "storeKind" );
            record.row.groupId = TextOf( node, "group" );
            record.row.subGroup = TextOf( node, "subGroup" );
            record.row.groupRule = TextOf( node, "groupRule" );
            record.row.format = TextOf( node, "format" );
            record.row.sizeBytes = CountOf( node, "size" );
            record.row.digest = TextOf( node, "digest" );
            record.row.presentedUnder = TextOf( node, "presentedUnder" );
            const CORE::CDataNode* statusNode = node.FindChild( "status" );
            if ( GUCEF_NULL != statusNode )
            {
                record.row.status = TextOf( *statusNode, "state" );
                const CORE::CString analyse = StepOf( *statusNode, "analyse" );
                const CORE::CString convert = StepOf( *statusNode, "convert" );
                record.row.analysed = "fresh" == analyse || "analysed" == record.row.status || "converted" == record.row.status;
                record.row.converted = "fresh" == convert || "converted" == record.row.status;
            }
            const CORE::CDataNode* thumbnails = node.FindChild( "thumbnails" );
            record.row.hasThumbnail = GUCEF_NULL != thumbnails && thumbnails->GetNrOfDirectChildNodes() > 0;
            const CORE::CDataNode* stats = node.FindChild( "stats" );
            if ( GUCEF_NULL != stats )
            {
                CORE::CDataNode::TAttributeMap::const_iterator a = stats->AttributeBegin();
                for ( ; a != stats->AttributeEnd(); ++a )
                    record.stats[ (*a).first ] = (*a).second;
            }
            const CORE::CDataNode* refs = node.FindChild( "refs" );
            if ( GUCEF_NULL != refs )
            {
                CORE::CDataNode::const_iterator f = refs->ConstBegin();
                for ( ; f != refs->ConstEnd(); ++f )
                {
                    const CORE::CString kind = TextOf( *(*f), "kind" );
                    const CORE::CString to = TextOf( *(*f), "to" );
                    const CORE::CString path = TextOf( *(*f), "path" );
                    if ( !kind.IsNULLOrEmpty() && ( !to.IsNULLOrEmpty() || !path.IsNULLOrEmpty() ) )
                    {
                        record.refs.push_back( std::make_pair( kind, to ) );
                        record.refPaths.push_back( path );
                    }
                }
            }
            node.GetValuesOfChildByName( "tags", record.tags );
            document.records.push_back( record );
        }
    }
    for ( size_t t=0; t<shardTrees.size(); ++t )
        GUCEF_DELETE shardTrees[ t ];
    if ( ASSETSTORE_STATUS_OK != status )
        return status;

    // the manifest without the records
    document.manifest.Copy( root );
    document.manifest.SetName( "manifest" );
    document.manifest.DelChild( "resources" );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackIndexLoader::WriteRows( const CORE::CString& packId, const CORE::CString& manifestVfsPath, const SDocument& document, const CORE::CString& state,
                                  const CORE::CString& reason, Int64 loadedMs, CORE::CString& message )
{GUCEF_TRACE;

    const CORE::CVariant packIdValue( packId );
    GUCEF::map< CORE::CString, CORE::CString > storeIds;           // the id of the document to the id of the store
    GUCEF::map< CORE::CString, CAssetCatalogue::TRow > groupRows;  // group and a line feed and the sub-group, to the row
    for ( size_t r=0; r<document.records.size(); ++r )
        if ( !document.records[ r ].documentId.IsNULLOrEmpty() )
            storeIds[ document.records[ r ].documentId ] = MakeResourceId( packId, document.records[ r ].row.path );

    // which of the resources are assets of the pack
    CORE::CString::StringSet assetIds;
    {
        CORE::CDataTableQuery query;
        query.Select( "asset_id" ).WhereEquals( "pack_id", packIdValue );
        CAssetCatalogue::TRowVector rows;
        TAssetStoreStatus status = m_catalogue.Select( "assets", query, rows );
        if ( ASSETSTORE_STATUS_OK != status )
        {
            message = "the assets of the pack cannot be read: " + m_catalogue.GetLastErrorMessage();
            return status;
        }
        for ( size_t r=0; r<rows.size(); ++r )
            assetIds.insert( CAssetCatalogue::GetText( rows[ r ], "asset_id" ) );
    }

    CAssetCatalogue::TRowVector resourceRows;
    CAssetCatalogue::TRowVector relationRows;
    CAssetCatalogue::TRowVector keywordRows;
    GUCEF::map< CORE::CString, CAssetCatalogue::TRowVector > propRows;
    CORE::CString::StringSet needProps;
    bool hasStats = false;
    resourceRows.reserve( document.records.size() );
    GUCEF::map< CORE::CString, CORE::CString::StringSet > seenGroups;
    for ( size_t r=0; r<document.records.size(); ++r )
    {
        const SDocument::SRecord& record = document.records[ r ];
        const CAssetPackResource& resource = record.row;
        const CORE::CString storeId = MakeResourceId( packId, resource.path );
        const bool isAsset = assetIds.end() != assetIds.find( storeId );
        CAssetCatalogue::TRow row;
        row[ "resource_id" ] = CORE::CVariant( storeId );
        row[ "pack_id" ] = packIdValue;
        row[ "asset_id" ] = CORE::CVariant( isAsset ? storeId : CORE::CString::Empty );
        row[ "path" ] = CORE::CVariant( resource.path );
        row[ "name" ] = CORE::CVariant( resource.name );
        row[ "title" ] = CORE::CVariant( resource.title );
        row[ "kind" ] = CORE::CVariant( resource.kind );
        row[ "store_kind" ] = CORE::CVariant( resource.storeKind );
        row[ "group_id" ] = CORE::CVariant( resource.groupId );
        row[ "sub_group" ] = CORE::CVariant( resource.subGroup );
        row[ "group_rule" ] = CORE::CVariant( resource.groupRule );
        row[ "format" ] = CORE::CVariant( resource.format );
        row[ "size_bytes" ] = CORE::CVariant( (Int64) resource.sizeBytes );
        row[ "digest" ] = CORE::CVariant( resource.digest );
        row[ "status" ] = CORE::CVariant( resource.status );
        row[ "analysed" ] = CORE::CVariant( resource.analysed );
        row[ "converted" ] = CORE::CVariant( resource.converted );
        row[ "has_thumbnail" ] = CORE::CVariant( resource.hasThumbnail );
        row[ "presented_under" ] = CORE::CVariant( resource.presentedUnder );
        resourceRows.push_back( row );

        // the group rows count the records as they are, so they hold also where the manifest truncated its sub-group lists
        for ( UInt32 level=0; level<2; ++level )
        {
            if ( 1 == level && resource.subGroup.IsNULLOrEmpty() )
                break;
            const CORE::CString key = resource.groupId + "\n" + ( 0 == level ? CORE::CString::Empty : resource.subGroup );
            CAssetCatalogue::TRow& groupRow = groupRows[ key ];
            if ( groupRow.empty() )
            {
                groupRow[ "pack_id" ] = packIdValue;
                groupRow[ "group_id" ] = CORE::CVariant( resource.groupId );
                groupRow[ "sub_group" ] = CORE::CVariant( 0 == level ? CORE::CString::Empty : resource.subGroup );
                GUCEF::map< CORE::CString, CORE::CString >::const_iterator n = document.groupNames.find( key );
                CORE::CString name = n != document.groupNames.end() ? (*n).second : CORE::CString::Empty;
                if ( name.IsNULLOrEmpty() )
                    name = 0 == level ? resource.groupId : LastSegment( resource.subGroup );
                groupRow[ "name" ] = CORE::CVariant( name );
                groupRow[ "resource_count" ] = CORE::CVariant( (Int64) 0 );
                groupRow[ "size_bytes" ] = CORE::CVariant( (Int64) 0 );
            }
            groupRow[ "resource_count" ] = CORE::CVariant( CAssetCatalogue::GetInt( groupRow, "resource_count" ) + 1 );
            groupRow[ "size_bytes" ] = CORE::CVariant( CAssetCatalogue::GetInt( groupRow, "size_bytes" ) + (Int64) resource.sizeBytes );
        }

        // the references: between the resources, by the ids of the store
        // a reference with the path of its target gets the id of the store from the path (pack index design 2.12): the target need not be a
        // resource of the document (a .meta file is an asset of the catalogue all the same); one without a path is mapped through the document ids
        CORE::CString::StringSet writtenRefs;
        for ( size_t f=0; f<record.refs.size(); ++f )
        {
            CORE::CString toId;
            const CORE::CString& refPath = f < record.refPaths.size() ? record.refPaths[ f ] : CORE::CString::Empty;
            if ( !refPath.IsNULLOrEmpty() )
            {
                toId = MakeResourceId( packId, refPath );
            }
            else
            {
                GUCEF::map< CORE::CString, CORE::CString >::const_iterator to = storeIds.find( record.refs[ f ].second );
                if ( to == storeIds.end() )
                    continue;
                toId = (*to).second;
            }
            if ( toId == storeId || !writtenRefs.insert( record.refs[ f ].first + "\n" + toId ).second )
                continue;
            CAssetCatalogue::TRow relation;
            relation[ "from_asset" ] = CORE::CVariant( storeId );
            relation[ "kind" ] = CORE::CVariant( record.refs[ f ].first );
            relation[ "to_asset" ] = CORE::CVariant( toId );
            relation[ "pack_id" ] = packIdValue;
            relationRows.push_back( relation );
        }

        // the names of the resources that are no assets are searchable too
        if ( !isAsset && GUCEF_NULL != m_normalizer )
        {
            TAssetKeywordVector keywords;
            m_normalizer->Normalize( resource.name, "name", 3, keywords );
            if ( !resource.title.IsNULLOrEmpty() )
                m_normalizer->Normalize( resource.title, "title", 3, keywords );
            if ( !resource.subGroup.IsNULLOrEmpty() )
                m_normalizer->Normalize( resource.subGroup, "folder", 1, keywords );
            for ( size_t t=0; t<record.tags.size(); ++t )
                m_normalizer->Normalize( record.tags[ t ], "tag", 1, keywords );
            for ( size_t k=0; k<keywords.size(); ++k )
            {
                CAssetCatalogue::TRow keyword;
                keyword[ "keyword" ] = CORE::CVariant( keywords[ k ].keyword );
                keyword[ "object_id" ] = CORE::CVariant( storeId );
                keyword[ "object_type" ] = CORE::CVariant( CORE::CString( "resource" ) );
                keyword[ "pack_id" ] = packIdValue;
                keyword[ "field" ] = CORE::CVariant( keywords[ k ].field );
                keyword[ "weight" ] = CORE::CVariant( keywords[ k ].weight );
                keyword[ "original" ] = CORE::CVariant( keywords[ k ].original );
                keywordRows.push_back( keyword );
            }
        }
        if ( isAsset && !record.stats.empty() )
        {
            hasStats = true;
            needProps.insert( storeId );
        }
    }

    // the statistics of assets that have none: the typed property tables, for the families the document knows
    if ( hasStats )
    {
        static const char* const families[] = { "asset_model_props", "asset_texture_props", "asset_animation_props", "asset_audio_props" };
        GUCEF::map< CORE::CString, CORE::CString::StringSet > have;
        for ( UInt32 f=0; f<4; ++f )
        {
            CORE::CDataTableQuery query;
            query.Select( "asset_id" ).WhereEquals( "pack_id", packIdValue );
            CAssetCatalogue::TRowVector rows;
            if ( ASSETSTORE_STATUS_OK != m_catalogue.Select( families[ f ], query, rows ) )
                continue;
            for ( size_t r=0; r<rows.size(); ++r )
                have[ families[ f ] ].insert( CAssetCatalogue::GetText( rows[ r ], "asset_id" ) );
        }
        for ( size_t r=0; r<document.records.size(); ++r )
        {
            const SDocument::SRecord& record = document.records[ r ];
            const CORE::CString storeId = MakeResourceId( packId, record.row.path );
            if ( needProps.end() == needProps.find( storeId ) )
                continue;
            typedef GUCEF::map< CORE::CString, CORE::CVariant > TStats;
            const TStats& stats = record.stats;
            const CORE::CString& kind = record.row.kind;
            if ( "model" == kind && have[ "asset_model_props" ].end() == have[ "asset_model_props" ].find( storeId ) )
            {
                CAssetCatalogue::TRow props;
                props[ "asset_id" ] = CORE::CVariant( storeId );
                props[ "pack_id" ] = packIdValue;
                static const char* const names[][2] = { { "triangles", "triangles" }, { "vertices", "vertices" }, { "subMeshes", "sub_meshes" }, { "bones", "bones" }, { "animations", "animations" }, { "lods", "lods" } };
                for ( UInt32 n=0; n<6; ++n )
                {
                    TStats::const_iterator s = stats.find( names[ n ][ 0 ] );
                    if ( s != stats.end() )
                        props[ names[ n ][ 1 ] ] = CORE::CVariant( (Int64) (*s).second.AsUInt64( 0, true ) );
                }
                TStats::const_iterator bones = stats.find( "bones" );
                props[ "rigged" ] = CORE::CVariant( bones != stats.end() && (*bones).second.AsUInt64( 0, true ) > 0 );
                propRows[ "asset_model_props" ].push_back( props );
            }
            else
            if ( ( "texture" == kind || "image" == kind ) && have[ "asset_texture_props" ].end() == have[ "asset_texture_props" ].find( storeId ) )
            {
                CAssetCatalogue::TRow props;
                props[ "asset_id" ] = CORE::CVariant( storeId );
                props[ "pack_id" ] = packIdValue;
                const UInt64 width = stats.find( "width" ) != stats.end() ? stats.find( "width" )->second.AsUInt64( 0, true ) : 0;
                const UInt64 height = stats.find( "height" ) != stats.end() ? stats.find( "height" )->second.AsUInt64( 0, true ) : 0;
                props[ "width" ] = CORE::CVariant( (Int64) width );
                props[ "height" ] = CORE::CVariant( (Int64) height );
                props[ "resolution" ] = CORE::CVariant( (Int64) ( width > height ? width : height ) );
                if ( stats.find( "channels" ) != stats.end() )
                    props[ "channels" ] = CORE::CVariant( (Int64) stats.find( "channels" )->second.AsUInt64( 0, true ) );
                if ( stats.find( "bitDepth" ) != stats.end() )
                    props[ "bit_depth" ] = CORE::CVariant( (Int64) stats.find( "bitDepth" )->second.AsUInt64( 0, true ) );
                propRows[ "asset_texture_props" ].push_back( props );
            }
            else
            if ( "animation" == kind && have[ "asset_animation_props" ].end() == have[ "asset_animation_props" ].find( storeId ) )
            {
                CAssetCatalogue::TRow props;
                props[ "asset_id" ] = CORE::CVariant( storeId );
                props[ "pack_id" ] = packIdValue;
                if ( stats.find( "lengthS" ) != stats.end() )
                    props[ "length_s" ] = CORE::CVariant( (Float64) stats.find( "lengthS" )->second.AsFloat64( 0.0, true ) );
                if ( stats.find( "frameRate" ) != stats.end() )
                    props[ "frame_rate" ] = CORE::CVariant( (Float64) stats.find( "frameRate" )->second.AsFloat64( 0.0, true ) );
                propRows[ "asset_animation_props" ].push_back( props );
            }
            else
            if ( "audio" == kind && have[ "asset_audio_props" ].end() == have[ "asset_audio_props" ].find( storeId ) )
            {
                CAssetCatalogue::TRow props;
                props[ "asset_id" ] = CORE::CVariant( storeId );
                props[ "pack_id" ] = packIdValue;
                if ( stats.find( "lengthS" ) != stats.end() )
                    props[ "length_s" ] = CORE::CVariant( (Float64) stats.find( "lengthS" )->second.AsFloat64( 0.0, true ) );
                if ( stats.find( "channels" ) != stats.end() )
                    props[ "channels" ] = CORE::CVariant( (Int64) stats.find( "channels" )->second.AsUInt64( 0, true ) );
                if ( stats.find( "sampleRate" ) != stats.end() )
                    props[ "sample_rate" ] = CORE::CVariant( (Int64) stats.find( "sampleRate" )->second.AsUInt64( 0, true ) );
                propRows[ "asset_audio_props" ].push_back( props );
            }
        }
    }

    CAssetCatalogue::TRowVector groupRowVector;
    for ( GUCEF::map< CORE::CString, CAssetCatalogue::TRow >::const_iterator g=groupRows.begin(); g!=groupRows.end(); ++g )
        groupRowVector.push_back( (*g).second );

    CAssetCatalogue::TRowVector indexRows( 1 );
    CAssetCatalogue::TRow& indexRow = indexRows[ 0 ];
    indexRow[ "pack_id" ] = packIdValue;
    indexRow[ "index_key" ] = CORE::CVariant( document.indexKey );
    indexRow[ "pack_key" ] = CORE::CVariant( document.packKey );
    indexRow[ "schema_version" ] = CORE::CVariant( (Int64) document.version );
    indexRow[ "document_path" ] = CORE::CVariant( manifestVfsPath );
    indexRow[ "document_hash" ] = CORE::CVariant( document.documentHash );
    indexRow[ "levels" ] = CORE::CVariant( document.levels );
    indexRow[ "state" ] = CORE::CVariant( state );
    indexRow[ "state_reason" ] = CORE::CVariant( reason );
    indexRow[ "resource_count" ] = CORE::CVariant( (Int64) document.records.size() );
    indexRow[ "noise_count" ] = CORE::CVariant( (Int64) document.noiseCount );
    indexRow[ "shard_count" ] = CORE::CVariant( (Int64) document.shardCount );
    indexRow[ "producer" ] = CORE::CVariant( document.producer );
    indexRow[ "produced_ms" ] = CORE::CVariant( document.producedMs );
    indexRow[ "loaded_ms" ] = CORE::CVariant( loadedMs );

    // the rows of what the document replaces, then the new ones, the row of the index last
    TAssetStoreStatus status = m_catalogue.DeleteWhereEquals( "pack_resources", "pack_id", packIdValue );
    if ( ASSETSTORE_STATUS_OK == status )
        status = m_catalogue.DeleteWhereEquals( "pack_groups", "pack_id", packIdValue );
    if ( ASSETSTORE_STATUS_OK == status )
    {
        CORE::CDataTableQuery keywordQuery;
        keywordQuery.WhereEquals( "pack_id", packIdValue ).WhereEquals( "object_type", CORE::CVariant( CORE::CString( "resource" ) ) );
        status = m_catalogue.Delete( "keywords", keywordQuery );
    }
    if ( ASSETSTORE_STATUS_OK == status && !relationRows.empty() )
        status = m_catalogue.DeleteWhereEquals( "relations", "pack_id", packIdValue );
    if ( ASSETSTORE_STATUS_OK == status )
        status = m_catalogue.Write( "pack_groups", groupRowVector );
    if ( ASSETSTORE_STATUS_OK == status )
        status = m_catalogue.Write( "pack_resources", resourceRows );
    if ( ASSETSTORE_STATUS_OK == status )
        status = m_catalogue.Write( "relations", relationRows );
    if ( ASSETSTORE_STATUS_OK == status )
        status = m_catalogue.Write( "keywords", keywordRows );
    for ( GUCEF::map< CORE::CString, CAssetCatalogue::TRowVector >::const_iterator p=propRows.begin(); ASSETSTORE_STATUS_OK == status && p!=propRows.end(); ++p )
        status = m_catalogue.Write( (*p).first, (*p).second );
    if ( ASSETSTORE_STATUS_OK == status )
        status = m_catalogue.Write( "pack_indexes", indexRows );
    if ( ASSETSTORE_STATUS_OK != status )
        message = "writing the rows of the pack index failed: " + m_catalogue.GetLastErrorMessage();
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackIndexLoader::Replace( const CORE::CString& packId, const CORE::CString& manifestVfsPath, const SDocument& document, const CORE::CString& state,
                                const CORE::CString& reason, Int64 loadedMs, CORE::CString& message )
{GUCEF_TRACE;

    if ( m_catalogue.HasTransactions() )
    {
        COMCORE::CIDatabaseClientPtr client = m_catalogue.GetClient();
        COMCORE::CDatabaseTransaction transaction( client );
        if ( !transaction.IsActive() )
        {
            message = "the transaction of the load of the pack index cannot begin";
            return ASSETSTORE_STATUS_DATABASE_ERROR;
        }
        const TAssetStoreStatus status = WriteRows( packId, manifestVfsPath, document, state, reason, loadedMs, message );
        if ( ASSETSTORE_STATUS_OK != status )
            return status;
        if ( CORE::DATATABLE_STATUS_OK != transaction.Commit() )
        {
            message = "the transaction of the load of the pack index was not committed: " + client->GetLastErrorMessage();
            return ASSETSTORE_STATUS_DATABASE_ERROR;
        }
        return ASSETSTORE_STATUS_OK;
    }

    // without transactions: the row of the index says "loading" until the others are there; a load that is interrupted is repaired from the document
    CAssetCatalogue::TRowVector marker( 1 );
    marker[ 0 ][ "pack_id" ] = CORE::CVariant( packId );
    marker[ 0 ][ "index_key" ] = CORE::CVariant( document.indexKey );
    marker[ 0 ][ "pack_key" ] = CORE::CVariant( document.packKey );
    marker[ 0 ][ "document_path" ] = CORE::CVariant( manifestVfsPath );
    marker[ 0 ][ "document_hash" ] = CORE::CVariant( document.documentHash );
    marker[ 0 ][ "state" ] = CORE::CVariant( CAssetPackIndexInfo::StateLoading );
    TAssetStoreStatus status = m_catalogue.Write( "pack_indexes", marker );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        message = "writing the row of the pack index failed: " + m_catalogue.GetLastErrorMessage();
        return status;
    }
    return WriteRows( packId, manifestVfsPath, document, state, reason, loadedMs, message );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackIndexLoader::Load( const CORE::CString& packId, const CORE::CString& manifestVfsPath, bool force, CAssetPackIndexOutcome& outcome )
{GUCEF_TRACE;

    outcome.packId = packId;
    outcome.documentPath = manifestVfsPath;
    if ( !IsAvailable() )
    {
        outcome.message = "the catalogue has no pack index tables (schema version 4)";
        return outcome.status = ASSETSTORE_STATUS_UNSUPPORTED;
    }
    if ( packId.IsNULLOrEmpty() )
    {
        outcome.message = "no pack";
        return outcome.status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
    }
    CAssetCatalogue::TRow packRow;
    {
        CORE::CDataTableQuery query;
        query.WhereEquals( "pack_id", CORE::CVariant( packId ) );
        CAssetCatalogue::TRowVector rows;
        const TAssetStoreStatus status = m_catalogue.Select( "packs", query, rows );
        if ( ASSETSTORE_STATUS_OK != status || rows.empty() )
        {
            outcome.message = "there is no pack " + packId + " in the catalogue";
            return outcome.status = ASSETSTORE_STATUS_NOT_FOUND;
        }
        packRow = rows[ 0 ];
    }

    SDocument document;
    CORE::CString message;
    TAssetStoreStatus status = ReadDocument( manifestVfsPath, document, message );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        outcome.message = message;
        return outcome.status = status;
    }
    outcome.indexKey = document.indexKey;
    outcome.documentHash = document.documentHash;
    outcome.resourceCount = (UInt32) document.records.size();

    // the stored copy is this document: nothing to do
    if ( !force )
    {
        CAssetPackIndexInfo stored;
        if ( ASSETSTORE_STATUS_OK == GetInfo( packId, false, stored ) && stored.documentHash == document.documentHash &&
             ( CAssetPackIndexInfo::StateLoaded == stored.state || CAssetPackIndexInfo::StateStale == stored.state ) &&
             stored.documentPath == manifestVfsPath )
        {
            outcome.loaded = false;
            outcome.message = "the catalogue holds this pack index";
            return outcome.status = ASSETSTORE_STATUS_OK;
        }
    }

    // a document that lists fewer files or bytes than the pack has is of another pack content: loaded all the same, marked. The store leaves files
    // out of its counts (the descriptor of the pack, hidden files) that the converter lists, so a document may list more, never fewer
    CORE::CString state = CAssetPackIndexInfo::StateLoaded;
    CORE::CString reason;
    if ( document.hasFingerprint )
    {
        const UInt64 packFiles = (UInt64) CAssetCatalogue::GetInt( packRow, "file_count" );
        const UInt64 packBytes = (UInt64) CAssetCatalogue::GetInt( packRow, "size_bytes" );
        if ( document.fingerprintFiles < packFiles || document.fingerprintBytes < packBytes )
        {
            state = CAssetPackIndexInfo::StateStale;
            reason = "the document lists " + CORE::UInt64ToString( document.fingerprintFiles ) + " files and " + CORE::UInt64ToString( document.fingerprintBytes ) + " bytes, the pack has " +
                     CORE::UInt64ToString( packFiles ) + " files and " + CORE::UInt64ToString( packBytes ) + " bytes";
        }
    }
    status = Replace( packId, manifestVfsPath, document, state, reason, NowMs(), message );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        outcome.message = message;
        Fail( status, message );
        return outcome.status = status;
    }
    outcome.loaded = true;
    outcome.message = "loaded " + CORE::ToString( (UInt32) document.records.size() ) + " resource(s)";
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetPackIndexLoader: Loaded the pack index of pack " << packId << ": " << document.records.size() << " resources, state " << state );
    return outcome.status = ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackIndexLoader::Remove( const CORE::CString& packId )
{GUCEF_TRACE;

    if ( !IsAvailable() )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    CORE::CString::StringVector tables;
    CAssetCatalogue::GetPackIndexTables( tables );
    TAssetStoreStatus result = ASSETSTORE_STATUS_OK;
    for ( size_t t=0; t<tables.size(); ++t )
    {
        const TAssetStoreStatus status = m_catalogue.DeleteWhereEquals( tables[ t ], "pack_id", CORE::CVariant( packId ) );
        if ( ASSETSTORE_STATUS_OK != status )
            result = status;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackIndexLoader::MarkStale( const CORE::CString& packId, const CORE::CString& reason )
{GUCEF_TRACE;

    if ( !IsAvailable() )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    CORE::CDataTableQuery query;
    query.WhereEquals( "pack_id", CORE::CVariant( packId ) );
    CAssetCatalogue::TRowVector rows;
    TAssetStoreStatus status = m_catalogue.Select( "pack_indexes", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    if ( rows.empty() )
        return ASSETSTORE_STATUS_NOT_FOUND;
    rows[ 0 ][ "state" ] = CORE::CVariant( CAssetPackIndexInfo::StateStale );
    rows[ 0 ][ "state_reason" ] = CORE::CVariant( reason );
    return m_catalogue.Write( "pack_indexes", rows );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackIndexLoader::GetGroups( const CORE::CString& packId, TAssetPackGroupVector& groups ) const
{GUCEF_TRACE;

    groups.clear();
    if ( !IsAvailable() )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    CORE::CDataTableQuery query;
    query.WhereEquals( "pack_id", CORE::CVariant( packId ) );
    CAssetCatalogue::TRowVector rows;
    const TAssetStoreStatus status = m_catalogue.Select( "pack_groups", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    for ( size_t r=0; r<rows.size(); ++r )
    {
        CAssetPackGroup group;
        group.groupId = CAssetCatalogue::GetText( rows[ r ], "group_id" );
        group.subGroup = CAssetCatalogue::GetText( rows[ r ], "sub_group" );
        group.name = CAssetCatalogue::GetText( rows[ r ], "name" );
        group.resourceCount = (UInt64) CAssetCatalogue::GetInt( rows[ r ], "resource_count" );
        group.sizeBytes = (UInt64) CAssetCatalogue::GetInt( rows[ r ], "size_bytes" );
        groups.push_back( group );
    }
    // the fixed order of the groups, the function group before its sub-groups, the sub-groups by name
    struct SOrder
    {
        static bool Less( const CAssetPackGroup& a, const CAssetPackGroup& b )
        {
            const UInt32 oa = GetGroupOrder( a.groupId );
            const UInt32 ob = GetGroupOrder( b.groupId );
            if ( oa != ob )
                return oa < ob;
            if ( a.groupId != b.groupId )
                return a.groupId < b.groupId;
            return a.subGroup < b.subGroup;
        }
    };
    std::sort( groups.begin(), groups.end(), SOrder::Less );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

namespace {

void
ResourceFromRow( const CAssetCatalogue::TRow& row, CAssetPackResource& resource )
{
    resource.resourceId = CAssetCatalogue::GetText( row, "resource_id" );
    resource.packId = CAssetCatalogue::GetText( row, "pack_id" );
    resource.assetId = CAssetCatalogue::GetText( row, "asset_id" );
    resource.path = CAssetCatalogue::GetText( row, "path" );
    resource.name = CAssetCatalogue::GetText( row, "name" );
    resource.title = CAssetCatalogue::GetText( row, "title" );
    resource.kind = CAssetCatalogue::GetText( row, "kind" );
    resource.storeKind = CAssetCatalogue::GetText( row, "store_kind" );
    resource.groupId = CAssetCatalogue::GetText( row, "group_id" );
    resource.subGroup = CAssetCatalogue::GetText( row, "sub_group" );
    resource.groupRule = CAssetCatalogue::GetText( row, "group_rule" );
    resource.format = CAssetCatalogue::GetText( row, "format" );
    resource.sizeBytes = (UInt64) CAssetCatalogue::GetInt( row, "size_bytes" );
    resource.digest = CAssetCatalogue::GetText( row, "digest" );
    resource.status = CAssetCatalogue::GetText( row, "status" );
    resource.analysed = CAssetCatalogue::GetBool( row, "analysed" );
    resource.converted = CAssetCatalogue::GetBool( row, "converted" );
    resource.hasThumbnail = CAssetCatalogue::GetBool( row, "has_thumbnail" );
    resource.presentedUnder = CAssetCatalogue::GetText( row, "presented_under" );
}

bool
ResourceLess( const CAssetPackResource& a, const CAssetPackResource& b )
{
    const UInt32 oa = CAssetPackIndexLoader::GetGroupOrder( a.groupId );
    const UInt32 ob = CAssetPackIndexLoader::GetGroupOrder( b.groupId );
    if ( oa != ob )
        return oa < ob;
    if ( a.groupId != b.groupId )
        return a.groupId < b.groupId;
    if ( a.subGroup != b.subGroup )
        return a.subGroup < b.subGroup;
    return a.path < b.path;
}

}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackIndexLoader::GetResources( const CORE::CString& packId, const CORE::CString& groupId, const CORE::CString& subGroup, TAssetPackResourceVector& resources ) const
{GUCEF_TRACE;

    resources.clear();
    if ( !IsAvailable() )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    CORE::CDataTableQuery query;
    query.WhereEquals( "pack_id", CORE::CVariant( packId ) );
    if ( !groupId.IsNULLOrEmpty() )
    {
        query.WhereEquals( "group_id", CORE::CVariant( groupId ) );
        if ( !subGroup.IsNULLOrEmpty() )
            query.WhereEquals( "sub_group", CORE::CVariant( subGroup ) );
    }
    CAssetCatalogue::TRowVector rows;
    const TAssetStoreStatus status = m_catalogue.Select( "pack_resources", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    resources.resize( rows.size() );
    for ( size_t r=0; r<rows.size(); ++r )
        ResourceFromRow( rows[ r ], resources[ r ] );
    std::sort( resources.begin(), resources.end(), ResourceLess );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackIndexLoader::GetResource( const CORE::CString& resourceId, CAssetPackResource& resource ) const
{GUCEF_TRACE;

    resource.Clear();
    if ( !IsAvailable() )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    CORE::CDataTableQuery query;
    query.WhereEquals( "resource_id", CORE::CVariant( resourceId ) );
    CAssetCatalogue::TRowVector rows;
    const TAssetStoreStatus status = m_catalogue.Select( "pack_resources", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    if ( rows.empty() )
        return ASSETSTORE_STATUS_NOT_FOUND;
    ResourceFromRow( rows[ 0 ], resource );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackIndexLoader::GetInfo( const CORE::CString& packId, bool withManifest, CAssetPackIndexInfo& info ) const
{GUCEF_TRACE;

    info.Clear();
    if ( !IsAvailable() )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    CORE::CDataTableQuery query;
    query.WhereEquals( "pack_id", CORE::CVariant( packId ) );
    CAssetCatalogue::TRowVector rows;
    TAssetStoreStatus status = m_catalogue.Select( "pack_indexes", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    if ( rows.empty() )
        return ASSETSTORE_STATUS_NOT_FOUND;
    const CAssetCatalogue::TRow& row = rows[ 0 ];
    info.packId = packId;
    info.indexKey = CAssetCatalogue::GetText( row, "index_key" );
    info.packKey = CAssetCatalogue::GetText( row, "pack_key" );
    info.schemaVersion = (UInt32) CAssetCatalogue::GetInt( row, "schema_version" );
    info.documentPath = CAssetCatalogue::GetText( row, "document_path" );
    info.documentHash = CAssetCatalogue::GetText( row, "document_hash" );
    info.levels = CAssetCatalogue::GetText( row, "levels" ).ParseElements( ',', false );
    info.state = CAssetCatalogue::GetText( row, "state" );
    info.stateReason = CAssetCatalogue::GetText( row, "state_reason" );
    info.stale = CAssetPackIndexInfo::StateStale == info.state;
    info.resourceCount = (UInt64) CAssetCatalogue::GetInt( row, "resource_count" );
    info.noiseCount = (UInt64) CAssetCatalogue::GetInt( row, "noise_count" );
    info.shardCount = (UInt32) CAssetCatalogue::GetInt( row, "shard_count" );
    info.producer = CAssetCatalogue::GetText( row, "producer" );
    info.producedMs = CAssetCatalogue::GetInt( row, "produced_ms" );
    info.loadedMs = CAssetCatalogue::GetInt( row, "loaded_ms" );
    status = GetGroups( packId, info.groups );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    if ( withManifest && !info.documentPath.IsNULLOrEmpty() )
        ReadManifest( info.documentPath, info );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackIndexLoader::ReadManifest( const CORE::CString& manifestVfsPath, CAssetPackIndexInfo& info ) const
{GUCEF_TRACE;

    SDocument document;
    CORE::CString message;
    const TAssetStoreStatus status = ReadDocument( manifestVfsPath, document, message );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    info.manifest.Copy( document.manifest );
    info.manifest.SetName( "manifest" );
    info.hasManifest = true;
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackIndexLoader::GetPacksInState( const CORE::CString& state, CORE::CString::StringVector& packIds ) const
{GUCEF_TRACE;

    packIds.clear();
    if ( !IsAvailable() )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    CORE::CDataTableQuery query;
    query.Select( "pack_id" ).WhereEquals( "state", CORE::CVariant( state ) );
    CAssetCatalogue::TRowVector rows;
    const TAssetStoreStatus status = m_catalogue.Select( "pack_indexes", query, rows );
    for ( size_t r=0; r<rows.size(); ++r )
        packIds.push_back( CAssetCatalogue::GetText( rows[ r ], "pack_id" ) );
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetPackIndexLoader::GetPacksWithoutIndex( CORE::CString::StringVector& packIds ) const
{GUCEF_TRACE;

    packIds.clear();
    if ( !IsAvailable() )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    CAssetCatalogue::TRowVector packs;
    CORE::CDataTableQuery packQuery;
    packQuery.Select( "pack_id" ).Select( "state" );
    TAssetStoreStatus status = m_catalogue.Select( "packs", packQuery, packs );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    CAssetCatalogue::TRowVector indexed;
    CORE::CDataTableQuery indexQuery;
    indexQuery.Select( "pack_id" );
    status = m_catalogue.Select( "pack_indexes", indexQuery, indexed );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    CORE::CString::StringSet have;
    for ( size_t r=0; r<indexed.size(); ++r )
        have.insert( CAssetCatalogue::GetText( indexed[ r ], "pack_id" ) );
    for ( size_t r=0; r<packs.size(); ++r )
    {
        const CORE::CString packId = CAssetCatalogue::GetText( packs[ r ], "pack_id" );
        if ( have.end() == have.find( packId ) )
            packIds.push_back( packId );
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
