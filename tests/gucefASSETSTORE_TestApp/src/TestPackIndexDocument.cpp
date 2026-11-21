/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#include "gucefASSETSTORE_CAssetStoreJson.h"
#define GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREJSON_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKINDEXLOADER_H
#include "gucefASSETSTORE_CAssetPackIndexLoader.h"
#define GUCEF_ASSETSTORE_CASSETPACKINDEXLOADER_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKINDEXLOADER_H ? */
#ifndef TESTASSETSTORESUPPORT_H
#include "TestAssetStoreSupport.h"
#define TESTASSETSTORESUPPORT_H
#endif /* TESTASSETSTORESUPPORT_H ? */

#include "TestPackIndexDocument.h"

using namespace GUCEF;
using namespace GUCEF::CORE;

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CString
Sha256OfText( const CString& text )
{
    CDynamicBuffer buffer;
    buffer.CopyFrom( text, 0, false );
    return Sha256Of( buffer );
}

CString
LastPart( const CString& path )
{
    return path.SubstrToChar( '/', false );
}

CString
ExtensionOf( const CString& name )
{
    const Int32 dot = name.HasChar( '.', false );
    return dot < 0 ? CString::Empty : name.CutChars( (UInt32) dot + 1, true ).Lowercase();
}

struct SOrder
{
    const CTestPackIndexDocument* document;

    bool operator()( const CTestPackIndexResource& a, const CTestPackIndexResource& b ) const
    {
        const CString ga = CTestPackIndexDocument::GroupOfKind( a.kind );
        const CString gb = CTestPackIndexDocument::GroupOfKind( b.kind );
        const UInt32 oa = GUCEF::ASSETSTORE::CAssetPackIndexLoader::GetGroupOrder( ga );
        const UInt32 ob = GUCEF::ASSETSTORE::CAssetPackIndexLoader::GetGroupOrder( gb );
        if ( oa != ob )
            return oa < ob;
        if ( a.subGroup != b.subGroup )
            return a.subGroup < b.subGroup;
        return a.path < b.path;
    }
};

void
WriteResource( CDataNode& node, const CTestPackIndexDocument& document, const CTestPackIndexResource& resource )
{
    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "id", document.MakeDocumentId( resource.path ) );
    node.SetAttribute( "path", resource.path );
    node.SetAttribute( "name", LastPart( resource.path ) );
    node.SetAttribute( "kind", resource.kind );
    node.SetAttribute( "group", CTestPackIndexDocument::GroupOfKind( resource.kind ) );
    if ( !resource.subGroup.IsNULLOrEmpty() )
        node.SetAttribute( "subGroup", resource.subGroup );
    node.SetAttribute( "format", ExtensionOf( LastPart( resource.path ) ) );
    node.SetAttribute( "size", resource.size );
    if ( !resource.statNames.empty() )
    {
        CDataNode* stats = node.AddChild( "stats", GUCEF_DATATYPE_OBJECT );
        for ( size_t s=0; s<resource.statNames.size(); ++s )
            stats->SetAttribute( resource.statNames[ s ], resource.statValues[ s ] );
    }
    if ( !resource.refPaths.empty() )
    {
        CDataNode* refs = node.AddChild( "refs", GUCEF_DATATYPE_ARRAY );
        for ( size_t r=0; r<resource.refPaths.size(); ++r )
        {
            CDataNode* ref = refs->AddChild( CString::Empty, GUCEF_DATATYPE_OBJECT );
            ref->SetAttribute( "kind", resource.refKinds[ r ] );
            const bool byPath = r < resource.refByPath.size() && resource.refByPath[ r ];
            const bool wrongId = byPath && resource.refPaths[ r ].StartsWith( "\n" );
            const CString target = wrongId ? resource.refPaths[ r ].CutChars( 1, true ) : resource.refPaths[ r ];
            ref->SetAttribute( "to", wrongId ? CString( "asnosuchid" ) : document.MakeDocumentId( target ) );
            if ( byPath )
                ref->SetAttribute( "path", target );
        }
    }
    if ( resource.hasThumbnail )
    {
        CDataNode* thumbnails = node.AddChild( "thumbnails", GUCEF_DATATYPE_ARRAY );
        CDataNode* thumbnail = thumbnails->AddChild( CString::Empty, GUCEF_DATATYPE_OBJECT );
        thumbnail->SetAttribute( "size", (UInt32) 128 );
    }
    CDataNode* status = node.AddChild( "status", GUCEF_DATATYPE_OBJECT );
    const CString state = resource.state.IsNULLOrEmpty() ? CString( "indexed" ) : resource.state;
    status->SetAttribute( "state", state );
    if ( "indexed" != state )
    {
        CDataNode* steps = status->AddChild( "steps", GUCEF_DATATYPE_OBJECT );
        steps->AddChild( "analyse", GUCEF_DATATYPE_OBJECT )->SetAttribute( "s", "fresh" );
        if ( "converted" == state )
            steps->AddChild( "convert", GUCEF_DATATYPE_OBJECT )->SetAttribute( "s", "fresh" );
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTestPackIndexResource::CTestPackIndexResource( void )
    : path(), kind(), subGroup(), size( 0 ), state(), statNames(), statValues(), refPaths(), refKinds(), refByPath(), hasThumbnail( false )
{
}

/*-------------------------------------------------------------------------*/

CTestPackIndexResource::CTestPackIndexResource( const CString& pathInPack, const CString& resourceKind, const CString& subGroupPath, UInt64 sizeBytes )
    : path( pathInPack ), kind( resourceKind ), subGroup( subGroupPath ), size( sizeBytes ), state(), statNames(), statValues(), refPaths(), refKinds(), refByPath(), hasThumbnail( false )
{
}

/*-------------------------------------------------------------------------*/

CTestPackIndexResource&
CTestPackIndexResource::Stat( const CString& name, UInt64 value )
{
    statNames.push_back( name );
    statValues.push_back( value );
    return *this;
}

/*-------------------------------------------------------------------------*/

CTestPackIndexResource&
CTestPackIndexResource::Ref( const CString& refKind, const CString& toPath )
{
    refKinds.push_back( refKind );
    refPaths.push_back( toPath );
    refByPath.push_back( false );
    return *this;
}

/*-------------------------------------------------------------------------*/

CTestPackIndexResource&
CTestPackIndexResource::RefPath( const CString& refKind, const CString& toPath, bool wrongId )
{
    refKinds.push_back( refKind );
    refPaths.push_back( wrongId ? "\n" + toPath : toPath );
    refByPath.push_back( true );
    return *this;
}

/*-------------------------------------------------------------------------*/

CTestPackIndexDocument::CTestPackIndexDocument( void )
    : packName( "testpack" ), packKey( "0123456789abcdef" ), indexKey( "fedcba9876543210" ), state( "complete" ), version( 1 ), format( "gucef.packIndex" )
    , fileCount( 0 ), totalBytes( 0 ), noise( 0 ), shardSize( 0 ), derived( false ), omitDocumentHash( false ), restrictions(), resources()
{
}

/*-------------------------------------------------------------------------*/

CString
CTestPackIndexDocument::MakeDocumentId( const CString& path ) const
{
    return "as" + Sha256OfText( packName + "\n" + path ).SubstrToIndex( 20, true );
}

/*-------------------------------------------------------------------------*/

CString
CTestPackIndexDocument::GroupOfKind( const CString& kind )
{
    if ( "scene" == kind )      return "scenes";
    if ( "skeleton" == kind )   return "characters";
    if ( "model" == kind )      return "static-models";
    if ( "animation" == kind )  return "animations";
    if ( "particle" == kind )   return "visual-effects";
    if ( "material" == kind )   return "materials";
    if ( "texture" == kind || "image" == kind ) return "textures";
    if ( "audio" == kind )      return "audio";
    if ( "prefab" == kind )     return "prefabs";
    if ( "shader" == kind )     return "shaders";
    if ( "font" == kind )       return "fonts";
    return "other";
}

/*-------------------------------------------------------------------------*/

CString
CTestPackIndexDocument::NameOfGroup( const CString& group )
{
    if ( "static-models" == group )     return "Static models";
    if ( "textures" == group )          return "Textures and images";
    if ( "visual-effects" == group )    return "Visual effects";
    if ( group.IsNULLOrEmpty() )        return group;
    return group.SubstrToIndex( 1, true ).Uppercase() + group.CutChars( 1, true );
}

/*-------------------------------------------------------------------------*/

CString
CTestPackIndexDocument::WriteTo( const CString& indexFolder ) const
{
    GUCEF::vector< CTestPackIndexResource > sorted = resources;
    SOrder order;
    order.document = this;
    std::sort( sorted.begin(), sorted.end(), order );

    CDataNode manifest( "pack-index", GUCEF_DATATYPE_OBJECT );
    manifest.SetAttribute( "format", format );
    manifest.SetAttribute( "version", version );
    CString::StringVector levels;
    levels.push_back( "structure" );
    levels.push_back( "analysis" );
    manifest.SetValuesOfChildByName( "levels", levels );
    manifest.SetAttribute( "state", state );
    CDataNode* identity = manifest.AddChild( "identity", GUCEF_DATATYPE_OBJECT );
    identity->SetAttribute( "packKey", packKey );
    identity->SetAttribute( "name", packName );
    if ( fileCount > 0 )
    {
        CDataNode* fingerprint = identity->AddChild( "fingerprint", GUCEF_DATATYPE_OBJECT );
        fingerprint->SetAttribute( "fileCount", fileCount );
        fingerprint->SetAttribute( "totalBytes", totalBytes );
        fingerprint->SetAttribute( "treeHash", "00" );
    }
    CDataNode* producer = manifest.AddChild( "producer", GUCEF_DATATYPE_OBJECT );
    producer->SetAttribute( "tool", "TestPackIndexDocument" );
    producer->SetAttribute( "toolVersion", "1" );
    producer->SetAttribute( "indexKey", indexKey );
    if ( derived )
        producer->SetAttribute( "derivedFromCatalogue", true );
    CDataNode* licence = manifest.AddChild( "licence", GUCEF_DATATYPE_OBJECT );
    if ( !restrictions.empty() )
        licence->SetValuesOfChildByName( "restrictions", restrictions );

    // the counts and the group summaries
    GUCEF::map< CString, UInt32 > groupCount;
    GUCEF::map< CString, UInt64 > groupBytes;
    GUCEF::map< CString, GUCEF::map< CString, UInt32 > > subCount;
    UInt64 bytes = 0;
    for ( size_t r=0; r<sorted.size(); ++r )
    {
        const CString group = GroupOfKind( sorted[ r ].kind );
        ++groupCount[ group ];
        groupBytes[ group ] += sorted[ r ].size;
        bytes += sorted[ r ].size;
        if ( !sorted[ r ].subGroup.IsNULLOrEmpty() )
            ++subCount[ group ][ sorted[ r ].subGroup ];
    }
    CDataNode* counts = manifest.AddChild( "counts", GUCEF_DATATYPE_OBJECT );
    counts->SetAttribute( "resources", (UInt32) sorted.size() );
    counts->SetAttribute( "bytes", bytes );
    counts->SetAttribute( "noise", noise );
    CDataNode* groups = manifest.AddChild( "groups", GUCEF_DATATYPE_ARRAY );
    for ( GUCEF::map< CString, UInt32 >::const_iterator g=groupCount.begin(); g!=groupCount.end(); ++g )
    {
        CDataNode* group = groups->AddChild( CString::Empty, GUCEF_DATATYPE_OBJECT );
        group->SetAttribute( "id", (*g).first );
        group->SetAttribute( "name", NameOfGroup( (*g).first ) );
        group->SetAttribute( "count", (*g).second );
        group->SetAttribute( "bytes", groupBytes[ (*g).first ] );
        CDataNode* subs = group->AddChild( "subGroups", GUCEF_DATATYPE_ARRAY );
        GUCEF::map< CString, UInt32 >& found = subCount[ (*g).first ];
        for ( GUCEF::map< CString, UInt32 >::const_iterator s=found.begin(); s!=found.end(); ++s )
        {
            CDataNode* sub = subs->AddChild( CString::Empty, GUCEF_DATATYPE_OBJECT );
            sub->SetAttribute( "id", (*s).first );
            sub->SetAttribute( "name", LastPart( (*s).first ) );
            sub->SetAttribute( "count", (*s).second );
        }
    }

    // the records: inline, or in shards
    CString hashText = packKey + "\n" + indexKey + "\n" + state + "\n";
    if ( 0 == shardSize || sorted.size() <= shardSize )
    {
        CDataNode* list = manifest.AddChild( "resources", GUCEF_DATATYPE_ARRAY );
        for ( size_t r=0; r<sorted.size(); ++r )
        {
            WriteResource( *list->AddChild( CString::Empty, GUCEF_DATATYPE_OBJECT ), *this, sorted[ r ] );
            hashText += sorted[ r ].path + ":" + CString( UInt64ToString( sorted[ r ].size ) ) + ":" + sorted[ r ].state + ":" + sorted[ r ].subGroup + "\n";
        }
    }
    else
    {
        const CString shardFolder = indexFolder + "/pack-index.d";
        CDataNode* shards = manifest.AddChild( "shards", GUCEF_DATATYPE_ARRAY );
        UInt32 number = 0;
        for ( size_t first=0; first<sorted.size(); first+=shardSize )
        {
            const size_t last = first + shardSize < sorted.size() ? first + shardSize : sorted.size();
            CDataNode shard( "shard", GUCEF_DATATYPE_OBJECT );
            CDataNode* list = shard.AddChild( "resources", GUCEF_DATATYPE_ARRAY );
            for ( size_t r=first; r<last; ++r )
            {
                WriteResource( *list->AddChild( CString::Empty, GUCEF_DATATYPE_OBJECT ), *this, sorted[ r ] );
                hashText += sorted[ r ].path + ":" + CString( UInt64ToString( sorted[ r ].size ) ) + ":" + sorted[ r ].state + ":" + sorted[ r ].subGroup + "\n";
            }
            CString text;
            if ( !GUCEF::ASSETSTORE::CAssetStoreJson::ToText( shard, text ) )
                return CString::Empty;
            const CString file = "shard-" + CString( UInt32ToString( ++number ) ) + ".json";
            if ( !WriteVfsFile( shardFolder + "/" + file, text ) )
                return CString::Empty;
            CDataNode* info = shards->AddChild( CString::Empty, GUCEF_DATATYPE_OBJECT );
            info->SetAttribute( "file", file );
            info->SetAttribute( "count", (UInt32) ( last - first ) );
            info->SetAttribute( "firstPath", sorted[ first ].path );
            info->SetAttribute( "lastPath", sorted[ last - 1 ].path );
            info->SetAttribute( "sha256", Sha256OfText( text ) );
        }
    }
    if ( !omitDocumentHash )
        manifest.SetAttribute( "documentHash", Sha256OfText( hashText ) );
    CString manifestText;
    if ( !GUCEF::ASSETSTORE::CAssetStoreJson::ToText( manifest, manifestText ) )
        return CString::Empty;
    const CString manifestPath = indexFolder + "/pack-index.json";
    return WriteVfsFile( manifestPath, manifestText ) ? manifestPath : CString::Empty;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/
