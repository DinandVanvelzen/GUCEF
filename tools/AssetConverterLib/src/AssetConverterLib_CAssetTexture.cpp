/*
 *  AssetConverterLib: a texture of the intermediate model: its sources and the operations that make it
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

#include "AssetConverterLib_CAssetTexture.h"

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

CORE::CString WrapModeName( IMAGE::TWrapMode mode ) { return IMAGE::ToString( mode ); }

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetTexture::CAssetTexture( void )
    : name()
    , sources()
    , operations()
    , usage( TEXTUREUSAGE_COLOUR )
    , wrapS( IMAGE::WRAPMODE_REPEAT )
    , wrapT( IMAGE::WRAPMODE_REPEAT )
    , bakedPath()
    , width( 0 )
    , height( 0 )
    , missing( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

IMAGE::TColourSpace
CAssetTexture::GetColourSpace( void ) const
{GUCEF_TRACE;

    for ( size_t i=operations.size(); i>0; --i )
    {
        if ( TEXTUREOPERATION_COLOURSPACETAG == operations[ i-1 ].type )
            return operations[ i-1 ].colourSpace;
    }
    return ( TEXTUREUSAGE_COLOUR == usage ) ? IMAGE::COLOURSPACE_SRGB : IMAGE::COLOURSPACE_LINEAR;
}

/*-------------------------------------------------------------------------*/

bool
CAssetTexture::HasTexelOperations( void ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<operations.size(); ++i )
        if ( operations[ i ].ChangesTexels() )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetTexture::GetChainKeyText( void ) const
{GUCEF_TRACE;

    CORE::CString text;
    for ( size_t i=0; i<operations.size(); ++i )
    {
        if ( !operations[ i ].ChangesTexels() )
            continue;
        if ( !text.IsNULLOrEmpty() )
            text += ";";
        text += operations[ i ].ToKeyText();
    }
    return text;
}

/*-------------------------------------------------------------------------*/

void
CAssetTexture::AddGreenFlip( void )
{GUCEF_TRACE;

    operations.push_back( CTextureOperation::GreenFlip() );
}

/*-------------------------------------------------------------------------*/

bool
CAssetTexture::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "name", name );
    node.SetValuesOfChildByName( "sources", sources );
    CORE::CDataNode* ops = node.AddChild( "operations", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<operations.size(); ++i )
    {
        CORE::CDataNode* op = ops->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        operations[ i ].Serialize( *op );
    }
    node.SetAttribute( "usage", ToString( usage ) );
    node.SetAttribute( "wrapS", WrapModeName( wrapS ) );
    node.SetAttribute( "wrapT", WrapModeName( wrapT ) );
    node.SetAttribute( "colourSpace", IMAGE::ToString( GetColourSpace() ) );
    if ( !bakedPath.IsNULLOrEmpty() )
        node.SetAttribute( "baked", bakedPath );
    if ( missing )
        node.SetAttribute( "missing", true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetTexture::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CAssetTexture texture;
    texture.name = node.GetAttributeValueOrChildValueByName( "name" ).AsString();
    node.GetValuesOfChildByName( "sources", texture.sources );
    const CORE::CDataNode* ops = node.FindChild( "operations" );
    if ( GUCEF_NULL != ops )
    {
        CORE::CDataNode::const_iterator i = ops->ConstBegin();
        while ( i != ops->ConstEnd() )
        {
            CTextureOperation op;
            if ( !op.Deserialize( *(*i) ) )
                return false;
            texture.operations.push_back( op );
            ++i;
        }
    }
    FromString( node.GetAttributeValueOrChildValueByName( "usage" ).AsString(), texture.usage );
    IMAGE::FromString( node.GetAttributeValueOrChildValueByName( "wrapS" ).AsString(), texture.wrapS );
    IMAGE::FromString( node.GetAttributeValueOrChildValueByName( "wrapT" ).AsString(), texture.wrapT );
    texture.bakedPath = node.GetAttributeValueOrChildValueByName( "baked" ).AsString();
    texture.missing = node.GetAttributeValueOrChildValueByName( "missing" ).AsBool( false );
    *this = texture;
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
