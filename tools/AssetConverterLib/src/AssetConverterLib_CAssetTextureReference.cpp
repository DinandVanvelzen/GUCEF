/*
 *  AssetConverterLib: the use of a texture by a material slot
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

#include "AssetConverterLib_CAssetTextureReference.h"

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

CAssetTextureReference::CAssetTextureReference( void )
    : textureIndex( -1 )
    , texCoord( 0 )
    , scale( 1.0f )
    , hasTransform( false )
    , rotation( 0.0f )
{GUCEF_TRACE;

    offset[ 0 ] = offset[ 1 ] = 0.0f;
    tiling[ 0 ] = tiling[ 1 ] = 1.0f;
}

/*-------------------------------------------------------------------------*/

CAssetTextureReference::CAssetTextureReference( Int32 texture, UInt32 uvSet )
    : textureIndex( texture )
    , texCoord( uvSet )
    , scale( 1.0f )
    , hasTransform( false )
    , rotation( 0.0f )
{GUCEF_TRACE;

    offset[ 0 ] = offset[ 1 ] = 0.0f;
    tiling[ 0 ] = tiling[ 1 ] = 1.0f;
}

/*-------------------------------------------------------------------------*/

bool
CAssetTextureReference::IsUsed( void ) const
{GUCEF_TRACE;

    return textureIndex >= 0;
}

/*-------------------------------------------------------------------------*/

bool
CAssetTextureReference::operator==( const CAssetTextureReference& other ) const
{GUCEF_TRACE;

    return textureIndex == other.textureIndex && texCoord == other.texCoord && scale == other.scale && hasTransform == other.hasTransform &&
           offset[ 0 ] == other.offset[ 0 ] && offset[ 1 ] == other.offset[ 1 ] && tiling[ 0 ] == other.tiling[ 0 ] &&
           tiling[ 1 ] == other.tiling[ 1 ] && rotation == other.rotation;
}

/*-------------------------------------------------------------------------*/

bool
CAssetTextureReference::operator!=( const CAssetTextureReference& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CAssetTextureReference::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "texture", textureIndex );
    node.SetAttribute( "texCoord", texCoord );
    node.SetAttribute( "scale", scale );
    if ( hasTransform )
    {
        CORE::CDataNode* transform = node.AddChild( "transform", GUCEF_DATATYPE_OBJECT );
        CORE::CDataNode::TVariantVector values;
        values.push_back( CORE::CVariant( offset[ 0 ] ) );
        values.push_back( CORE::CVariant( offset[ 1 ] ) );
        transform->SetValuesOfChildByName( "offset", values );
        values.clear();
        values.push_back( CORE::CVariant( tiling[ 0 ] ) );
        values.push_back( CORE::CVariant( tiling[ 1 ] ) );
        transform->SetValuesOfChildByName( "scale", values );
        transform->SetAttribute( "rotation", rotation );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetTextureReference::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CAssetTextureReference reference;
    reference.textureIndex = node.GetAttributeValueOrChildValueByName( "texture" ).AsInt32( -1 );
    reference.texCoord = node.GetAttributeValueOrChildValueByName( "texCoord" ).AsUInt32( 0 );
    reference.scale = node.GetAttributeValueOrChildValueByName( "scale" ).AsFloat32( 1.0f );
    const CORE::CDataNode* transform = node.FindChild( "transform" );
    if ( GUCEF_NULL != transform )
    {
        reference.hasTransform = true;
        CORE::CDataNode::TVariantVector values;
        transform->GetValuesOfChildByName( "offset", values );
        for ( UInt32 i=0; i<2 && i<values.size(); ++i )
            reference.offset[ i ] = values[ i ].AsFloat32( 0.0f );
        values.clear();
        transform->GetValuesOfChildByName( "scale", values );
        for ( UInt32 i=0; i<2 && i<values.size(); ++i )
            reference.tiling[ i ] = values[ i ].AsFloat32( 1.0f );
        reference.rotation = transform->GetAttributeValueOrChildValueByName( "rotation" ).AsFloat32( 0.0f );
    }
    *this = reference;
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
