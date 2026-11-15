/*
 *  AssetConverterLib: a material of the intermediate model in glTF terms
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

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#include "AssetConverterLib_CAssetMaterial.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

void
SetFloats( CORE::CDataNode& node, const char* name, const Float32* values, UInt32 count )
{
    CORE::CDataNode::TVariantVector list;
    for ( UInt32 i=0; i<count; ++i )
        list.push_back( CORE::CVariant( values[ i ] ) );
    node.SetValuesOfChildByName( name, list );
}

void
GetFloats( const CORE::CDataNode& node, const char* name, Float32* values, UInt32 count )
{
    CORE::CDataNode::TVariantVector list;
    node.GetValuesOfChildByName( name, list );
    for ( UInt32 i=0; i<count && i<list.size(); ++i )
        values[ i ] = list[ i ].AsFloat32( values[ i ] );
}

void
SetReference( CORE::CDataNode& node, const char* name, const CAssetTextureReference& reference )
{
    if ( !reference.IsUsed() )
        return;
    CORE::CDataNode* child = node.AddChild( name, GUCEF_DATATYPE_OBJECT );
    reference.Serialize( *child );
}

void
GetReference( const CORE::CDataNode& node, const char* name, CAssetTextureReference& reference )
{
    const CORE::CDataNode* child = node.FindChild( name );
    if ( GUCEF_NULL != child )
        reference.Deserialize( *child );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetMaterial::CAssetMaterial( void )
    : name()
    , baseColorTexture()
    , metallicFactor( 1.0f )
    , roughnessFactor( 1.0f )
    , metallicRoughnessTexture()
    , normalTexture()
    , occlusionTexture()
    , emissiveTexture()
    , emissiveStrength( 1.0f )
    , alphaMode( ALPHAMODE_OPAQUE )
    , alphaCutoff( 0.5f )
    , doubleSided( false )
    , unlit( false )
    , extensions( "extensions", GUCEF_DATATYPE_OBJECT )
    , source()
{GUCEF_TRACE;

    for ( UInt32 i=0; i<4; ++i )
        baseColorFactor[ i ] = 1.0f;
    for ( UInt32 i=0; i<3; ++i )
        emissiveFactor[ i ] = 0.0f;
}

/*-------------------------------------------------------------------------*/

bool
CAssetMaterial::HasSameAppearance( const CAssetMaterial& other ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<4; ++i )
        if ( baseColorFactor[ i ] != other.baseColorFactor[ i ] )
            return false;
    for ( UInt32 i=0; i<3; ++i )
        if ( emissiveFactor[ i ] != other.emissiveFactor[ i ] )
            return false;
    if ( metallicFactor != other.metallicFactor || roughnessFactor != other.roughnessFactor || emissiveStrength != other.emissiveStrength ||
         alphaMode != other.alphaMode || doubleSided != other.doubleSided || unlit != other.unlit )
        return false;
    if ( ALPHAMODE_MASK == alphaMode && alphaCutoff != other.alphaCutoff )
        return false;
    if ( baseColorTexture != other.baseColorTexture || metallicRoughnessTexture != other.metallicRoughnessTexture ||
         normalTexture != other.normalTexture || occlusionTexture != other.occlusionTexture || emissiveTexture != other.emissiveTexture )
        return false;
    return CCanonicalJson::ToJson( extensions ) == CCanonicalJson::ToJson( other.extensions );
}

/*-------------------------------------------------------------------------*/

void
CAssetMaterial::GetTextureReferences( GUCEF::vector< const CAssetTextureReference* >& references ) const
{GUCEF_TRACE;

    references.clear();
    references.push_back( &baseColorTexture );
    references.push_back( &metallicRoughnessTexture );
    references.push_back( &normalTexture );
    references.push_back( &occlusionTexture );
    references.push_back( &emissiveTexture );
}

/*-------------------------------------------------------------------------*/

void
CAssetMaterial::GetTextureReferences( GUCEF::vector< CAssetTextureReference* >& references )
{GUCEF_TRACE;

    references.clear();
    references.push_back( &baseColorTexture );
    references.push_back( &metallicRoughnessTexture );
    references.push_back( &normalTexture );
    references.push_back( &occlusionTexture );
    references.push_back( &emissiveTexture );
}

/*-------------------------------------------------------------------------*/

bool
CAssetMaterial::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "name", name );
    SetFloats( node, "baseColorFactor", baseColorFactor, 4 );
    SetReference( node, "baseColorTexture", baseColorTexture );
    node.SetAttribute( "metallicFactor", metallicFactor );
    node.SetAttribute( "roughnessFactor", roughnessFactor );
    SetReference( node, "metallicRoughnessTexture", metallicRoughnessTexture );
    SetReference( node, "normalTexture", normalTexture );
    SetReference( node, "occlusionTexture", occlusionTexture );
    SetFloats( node, "emissiveFactor", emissiveFactor, 3 );
    SetReference( node, "emissiveTexture", emissiveTexture );
    node.SetAttribute( "emissiveStrength", emissiveStrength );
    node.SetAttribute( "alphaMode", ToString( alphaMode ) );
    node.SetAttribute( "alphaCutoff", alphaCutoff );
    node.SetAttribute( "doubleSided", doubleSided );
    node.SetAttribute( "unlit", unlit );
    if ( 0 != extensions.GetAttCount() || extensions.HasChildren() )
    {
        CORE::CDataNode* ext = node.AddChild( extensions );
        ext->SetName( "extensions" );
    }
    if ( !source.IsEmpty() )
    {
        CORE::CDataNode* src = node.AddChild( "source", GUCEF_DATATYPE_OBJECT );
        source.Serialize( *src );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetMaterial::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CAssetMaterial material;
    material.name = node.GetAttributeValueOrChildValueByName( "name" ).AsString();
    GetFloats( node, "baseColorFactor", material.baseColorFactor, 4 );
    GetReference( node, "baseColorTexture", material.baseColorTexture );
    material.metallicFactor = node.GetAttributeValueOrChildValueByName( "metallicFactor" ).AsFloat32( 1.0f );
    material.roughnessFactor = node.GetAttributeValueOrChildValueByName( "roughnessFactor" ).AsFloat32( 1.0f );
    GetReference( node, "metallicRoughnessTexture", material.metallicRoughnessTexture );
    GetReference( node, "normalTexture", material.normalTexture );
    GetReference( node, "occlusionTexture", material.occlusionTexture );
    GetFloats( node, "emissiveFactor", material.emissiveFactor, 3 );
    GetReference( node, "emissiveTexture", material.emissiveTexture );
    material.emissiveStrength = node.GetAttributeValueOrChildValueByName( "emissiveStrength" ).AsFloat32( 1.0f );
    FromString( node.GetAttributeValueOrChildValueByName( "alphaMode" ).AsString(), material.alphaMode );
    material.alphaCutoff = node.GetAttributeValueOrChildValueByName( "alphaCutoff" ).AsFloat32( 0.5f );
    material.doubleSided = node.GetAttributeValueOrChildValueByName( "doubleSided" ).AsBool( false );
    material.unlit = node.GetAttributeValueOrChildValueByName( "unlit" ).AsBool( false );
    const CORE::CDataNode* ext = node.FindChild( "extensions" );
    if ( GUCEF_NULL != ext )
        material.extensions = *ext;
    const CORE::CDataNode* src = node.FindChild( "source" );
    if ( GUCEF_NULL != src )
        material.source.Deserialize( *src );
    *this = material;
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
