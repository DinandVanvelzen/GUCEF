/*
 *  AssetConverterLib: one resource record of the pack index
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#include "AssetConverterLib_CPackIndexSerial.h"
#define ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXSERIAL_H ? */

#ifndef ASSETCONVERTERLIB_CPACKGROUPVOCABULARY_H
#include "AssetConverterLib_CPackGroupVocabulary.h"
#define ASSETCONVERTERLIB_CPACKGROUPVOCABULARY_H
#endif /* ASSETCONVERTERLIB_CPACKGROUPVOCABULARY_H ? */

#ifndef ASSETCONVERTERLIB_CCONTENTHASH_H
#include "AssetConverterLib_CContentHash.h"
#define ASSETCONVERTERLIB_CCONTENTHASH_H
#endif /* ASSETCONVERTERLIB_CCONTENTHASH_H ? */

#include "AssetConverterLib_CPackIndexResource.h"

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

CPackIndexResource::CPackIndexResource( void )
    : id()
    , path()
    , name()
    , title()
    , kind()
    , storeKind()
    , group()
    , subGroup()
    , groupRule()
    , format()
    , size( 0 )
    , digest()
    , metadataFiles()
    , stats()
    , hasBounds( false )
    , refs()
    , unresolvedRefs()
    , presentedUnder()
    , outputs()
    , thumbnails()
    , presentation()
    , tags()
    , held()
    , licenceRef()
    , status()
{GUCEF_TRACE;

    for ( UInt32 i=0; i<3; ++i )
    {
        boundsMin[ i ] = 0.0f;
        boundsMax[ i ] = 0.0f;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackIndexResource::MakeId( const CORE::CString& idBase, const CORE::CString& path )
{GUCEF_TRACE;

    const CORE::CString hash = CContentHash::HashText( idBase + "\n" + path );
    return "as" + hash.SubstrFromRange( 0, 20 );
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexResource::operator<( const CPackIndexResource& other ) const
{GUCEF_TRACE;

    const UInt32 order = CPackGroupVocabulary::GetGroupOrder( group );
    const UInt32 otherOrder = CPackGroupVocabulary::GetGroupOrder( other.group );
    if ( order != otherOrder )
        return order < otherOrder;
    if ( group != other.group )
        return group < other.group;
    if ( subGroup != other.subGroup )
        return subGroup < other.subGroup;
    return path < other.path;
}

/*-------------------------------------------------------------------------*/

void
CPackIndexResource::SetStat( const CORE::CString& statName, UInt64 value )
{GUCEF_TRACE;

    stats[ statName ] = CORE::CVariant( value );
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexResource::GetStat( const CORE::CString& statName, UInt64& value ) const
{GUCEF_TRACE;

    TValueMap::const_iterator i = stats.find( statName );
    if ( i == stats.end() )
        return false;
    value = (*i).second.AsUInt64( 0, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexResource::HasStats( void ) const
{GUCEF_TRACE;

    return !stats.empty();
}

/*-------------------------------------------------------------------------*/

namespace {

void
WriteValueMap( CORE::CDataNode& node, const char* name, const CPackIndexResource::TValueMap& values )
{GUCEF_TRACE;

    if ( values.empty() )
        return;
    CORE::CDataNode* child = node.AddChild( name, GUCEF_DATATYPE_OBJECT );
    CPackIndexResource::TValueMap::const_iterator i = values.begin();
    while ( i != values.end() )
    {
        child->SetAttribute( (*i).first, (*i).second );
        ++i;
    }
}

void
ReadValueMap( const CORE::CDataNode& node, const char* name, CPackIndexResource::TValueMap& values )
{GUCEF_TRACE;

    values.clear();
    const CORE::CDataNode* child = node.FindChild( name );
    if ( GUCEF_NULL == child )
        return;
    CORE::CDataNode::TAttributeMap::const_iterator i = child->AttributeBegin();
    while ( i != child->AttributeEnd() )
    {
        values[ (*i).first ] = (*i).second;
        ++i;
    }
}

}

/*-------------------------------------------------------------------------*/

bool
CPackIndexResource::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CPackIndexSerial::SetTextAlways( node, "id", id );
    CPackIndexSerial::SetTextAlways( node, "path", path );
    CPackIndexSerial::SetTextAlways( node, "name", name );
    CPackIndexSerial::SetText( node, "title", title );
    CPackIndexSerial::SetTextAlways( node, "kind", kind );
    CPackIndexSerial::SetText( node, "storeKind", storeKind );
    CPackIndexSerial::SetTextAlways( node, "group", group );
    CPackIndexSerial::SetText( node, "subGroup", subGroup );
    CPackIndexSerial::SetText( node, "groupRule", groupRule );
    CPackIndexSerial::SetText( node, "format", format );
    node.SetAttribute( "size", size );
    CPackIndexSerial::SetText( node, "digest", digest );
    CPackIndexSerial::SetList( node, "metadataFiles", metadataFiles );
    WriteValueMap( node, "stats", stats );
    if ( hasBounds )
    {
        CORE::CDataNode* bounds = node.AddChild( "bounds", GUCEF_DATATYPE_OBJECT );
        CORE::CDataNode::TVariantVector values;
        for ( UInt32 i=0; i<3; ++i )
            values.push_back( CORE::CVariant( boundsMin[ i ] ) );
        bounds->SetValuesOfChildByName( "min", values );
        values.clear();
        for ( UInt32 i=0; i<3; ++i )
            values.push_back( CORE::CVariant( boundsMax[ i ] ) );
        bounds->SetValuesOfChildByName( "max", values );
    }
    if ( !refs.empty() )
    {
        TReferenceVector sorted = refs;
        std::sort( sorted.begin(), sorted.end() );
        CORE::CDataNode* list = node.AddChild( "refs", GUCEF_DATATYPE_ARRAY );
        for ( size_t i=0; i<sorted.size(); ++i )
            sorted[ i ].Serialize( *list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    }
    CPackIndexSerial::SetList( node, "unresolvedRefs", unresolvedRefs );
    CPackIndexSerial::SetText( node, "presentedUnder", presentedUnder );
    if ( !outputs.IsEmpty() )
        outputs.Serialize( *node.AddChild( "outputs", GUCEF_DATATYPE_OBJECT ) );
    if ( !thumbnails.empty() )
    {
        TThumbnailVector sorted = thumbnails;
        std::sort( sorted.begin(), sorted.end() );
        CORE::CDataNode* list = node.AddChild( "thumbnails", GUCEF_DATATYPE_ARRAY );
        for ( size_t i=0; i<sorted.size(); ++i )
            sorted[ i ].Serialize( *list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    }
    WriteValueMap( node, "presentation", presentation );
    CPackIndexSerial::SetSortedList( node, "tags", tags );
    CPackIndexSerial::SetSortedList( node, "held", held );
    CPackIndexSerial::SetText( node, "licenceRef", licenceRef );
    status.Serialize( *node.AddChild( "status", GUCEF_DATATYPE_OBJECT ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexResource::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    *this = CPackIndexResource();
    id = CPackIndexSerial::GetText( node, "id" );
    path = CPackIndexSerial::GetText( node, "path" );
    name = CPackIndexSerial::GetText( node, "name" );
    title = CPackIndexSerial::GetText( node, "title" );
    kind = CPackIndexSerial::GetText( node, "kind" );
    storeKind = CPackIndexSerial::GetText( node, "storeKind" );
    group = CPackIndexSerial::GetText( node, "group" );
    subGroup = CPackIndexSerial::GetText( node, "subGroup" );
    groupRule = CPackIndexSerial::GetText( node, "groupRule" );
    format = CPackIndexSerial::GetText( node, "format" );
    size = CPackIndexSerial::GetCount( node, "size" );
    digest = CPackIndexSerial::GetText( node, "digest" );
    CPackIndexSerial::GetList( node, "metadataFiles", metadataFiles );
    ReadValueMap( node, "stats", stats );
    const CORE::CDataNode* bounds = node.FindChild( "bounds" );
    if ( GUCEF_NULL != bounds )
    {
        CORE::CDataNode::TVariantVector minValues;
        CORE::CDataNode::TVariantVector maxValues;
        bounds->GetValuesOfChildByName( "min", minValues );
        bounds->GetValuesOfChildByName( "max", maxValues );
        if ( 3 == minValues.size() && 3 == maxValues.size() )
        {
            hasBounds = true;
            for ( UInt32 i=0; i<3; ++i )
            {
                boundsMin[ i ] = minValues[ i ].AsFloat32( 0.0f, true );
                boundsMax[ i ] = maxValues[ i ].AsFloat32( 0.0f, true );
            }
        }
    }
    const CORE::CDataNode* list = node.FindChild( "refs" );
    if ( GUCEF_NULL != list )
    {
        CORE::CDataNode::const_iterator i = list->ConstBegin();
        while ( i != list->ConstEnd() )
        {
            CPackIndexReference ref;
            if ( ref.Deserialize( *(*i) ) )
                refs.push_back( ref );
            ++i;
        }
    }
    CPackIndexSerial::GetList( node, "unresolvedRefs", unresolvedRefs );
    presentedUnder = CPackIndexSerial::GetText( node, "presentedUnder" );
    const CORE::CDataNode* outputNode = node.FindChild( "outputs" );
    if ( GUCEF_NULL != outputNode )
        outputs.Deserialize( *outputNode );
    list = node.FindChild( "thumbnails" );
    if ( GUCEF_NULL != list )
    {
        CORE::CDataNode::const_iterator i = list->ConstBegin();
        while ( i != list->ConstEnd() )
        {
            CPackIndexThumbnail thumbnail;
            if ( thumbnail.Deserialize( *(*i) ) )
                thumbnails.push_back( thumbnail );
            ++i;
        }
    }
    ReadValueMap( node, "presentation", presentation );
    CPackIndexSerial::GetList( node, "tags", tags );
    CPackIndexSerial::GetList( node, "held", held );
    licenceRef = CPackIndexSerial::GetText( node, "licenceRef" );
    const CORE::CDataNode* statusNode = node.FindChild( "status" );
    if ( GUCEF_NULL != statusNode )
        status.Deserialize( *statusNode );
    return !id.IsNULLOrEmpty() && !path.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
