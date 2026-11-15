/*
 *  AssetConverterLib: one step of the operation chain of a texture
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

#include "AssetConverterLib_CTextureOperation.h"

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

CTextureOperation::CTextureOperation( void )
    : type( TEXTUREOPERATION_COLOURSPACETAG )
    , channel( 0 )
    , channelMask( 0 )
    , scale( 1.0f )
    , bias( 0.0f )
    , mapping()
    , colourSpace( IMAGE::COLOURSPACE_LINEAR )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<4; ++i )
    {
        packSources[ i ] = -1;
        packChannels[ i ] = 0;
    }
}

/*-------------------------------------------------------------------------*/

CTextureOperation
CTextureOperation::ExtractChannel( UInt32 channel )
{GUCEF_TRACE;

    CTextureOperation op;
    op.type = TEXTUREOPERATION_EXTRACTCHANNEL;
    op.channel = channel;
    return op;
}

/*-------------------------------------------------------------------------*/

CTextureOperation
CTextureOperation::Swizzle( const CORE::CString& mapping )
{GUCEF_TRACE;

    CTextureOperation op;
    op.type = TEXTUREOPERATION_SWIZZLE;
    op.mapping = mapping.Uppercase();
    return op;
}

/*-------------------------------------------------------------------------*/

CTextureOperation
CTextureOperation::Invert( UInt32 channelMask )
{GUCEF_TRACE;

    CTextureOperation op;
    op.type = TEXTUREOPERATION_INVERT;
    op.channelMask = channelMask;
    return op;
}

/*-------------------------------------------------------------------------*/

CTextureOperation
CTextureOperation::ScaleBias( UInt32 channelMask, Float32 scale, Float32 bias )
{GUCEF_TRACE;

    CTextureOperation op;
    op.type = TEXTUREOPERATION_SCALEBIAS;
    op.channelMask = channelMask;
    op.scale = scale;
    op.bias = bias;
    return op;
}

/*-------------------------------------------------------------------------*/

CTextureOperation
CTextureOperation::Pack( const Int32 sources[ 4 ], const UInt32 channels[ 4 ] )
{GUCEF_TRACE;

    CTextureOperation op;
    op.type = TEXTUREOPERATION_PACK;
    for ( UInt32 i=0; i<4; ++i )
    {
        op.packSources[ i ] = sources[ i ];
        op.packChannels[ i ] = channels[ i ];
    }
    return op;
}

/*-------------------------------------------------------------------------*/

CTextureOperation
CTextureOperation::GreenFlip( void )
{GUCEF_TRACE;

    CTextureOperation op;
    op.type = TEXTUREOPERATION_GREENFLIP;
    return op;
}

/*-------------------------------------------------------------------------*/

CTextureOperation
CTextureOperation::PremultiplyAlpha( IMAGE::TColourSpace colourSpace )
{GUCEF_TRACE;

    CTextureOperation op;
    op.type = TEXTUREOPERATION_PREMULTIPLYALPHA;
    op.colourSpace = colourSpace;
    return op;
}

/*-------------------------------------------------------------------------*/

CTextureOperation
CTextureOperation::UnpremultiplyAlpha( IMAGE::TColourSpace colourSpace )
{GUCEF_TRACE;

    CTextureOperation op;
    op.type = TEXTUREOPERATION_UNPREMULTIPLYALPHA;
    op.colourSpace = colourSpace;
    return op;
}

/*-------------------------------------------------------------------------*/

CTextureOperation
CTextureOperation::ColourSpaceTag( IMAGE::TColourSpace colourSpace )
{GUCEF_TRACE;

    CTextureOperation op;
    op.type = TEXTUREOPERATION_COLOURSPACETAG;
    op.colourSpace = colourSpace;
    return op;
}

/*-------------------------------------------------------------------------*/

bool
CTextureOperation::ChangesTexels( void ) const
{GUCEF_TRACE;

    return TEXTUREOPERATION_COLOURSPACETAG != type;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTextureOperation::ToKeyText( void ) const
{GUCEF_TRACE;

    CORE::CString text = ToString( type );
    switch ( type )
    {
        case TEXTUREOPERATION_EXTRACTCHANNEL:
            text += "(" + CORE::UInt32ToString( channel ) + ")";
            break;
        case TEXTUREOPERATION_SWIZZLE:
            text += "(" + mapping + ")";
            break;
        case TEXTUREOPERATION_INVERT:
            text += "(" + CORE::UInt32ToString( channelMask ) + ")";
            break;
        case TEXTUREOPERATION_SCALEBIAS:
            text += "(" + CORE::UInt32ToString( channelMask ) + "," + CCanonicalJson::FormatFloat32( scale ) + "," + CCanonicalJson::FormatFloat32( bias ) + ")";
            break;
        case TEXTUREOPERATION_PACK:
        {
            text += "(";
            for ( UInt32 i=0; i<4; ++i )
            {
                if ( i > 0 )
                    text += ",";
                text += CORE::Int32ToString( packSources[ i ] ) + ":" + CORE::UInt32ToString( packChannels[ i ] );
            }
            text += ")";
            break;
        }
        case TEXTUREOPERATION_PREMULTIPLYALPHA:
        case TEXTUREOPERATION_UNPREMULTIPLYALPHA:
        case TEXTUREOPERATION_COLOURSPACETAG:
            text += "(" + IMAGE::ToString( colourSpace ) + ")";
            break;
        default:
            break;
    }
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CTextureOperation::operator==( const CTextureOperation& other ) const
{GUCEF_TRACE;

    return ToKeyText() == other.ToKeyText();
}

/*-------------------------------------------------------------------------*/

bool
CTextureOperation::operator!=( const CTextureOperation& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CTextureOperation::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "type", ToString( type ) );
    switch ( type )
    {
        case TEXTUREOPERATION_EXTRACTCHANNEL:
            node.SetAttribute( "channel", channel );
            break;
        case TEXTUREOPERATION_SWIZZLE:
            node.SetAttribute( "mapping", mapping );
            break;
        case TEXTUREOPERATION_INVERT:
            node.SetAttribute( "channelMask", channelMask );
            break;
        case TEXTUREOPERATION_SCALEBIAS:
            node.SetAttribute( "channelMask", channelMask );
            node.SetAttribute( "scale", scale );
            node.SetAttribute( "bias", bias );
            break;
        case TEXTUREOPERATION_PACK:
        {
            CORE::CDataNode::TVariantVector sources;
            CORE::CDataNode::TVariantVector channels;
            for ( UInt32 i=0; i<4; ++i )
            {
                sources.push_back( CORE::CVariant( packSources[ i ] ) );
                channels.push_back( CORE::CVariant( packChannels[ i ] ) );
            }
            node.SetValuesOfChildByName( "sources", sources );
            node.SetValuesOfChildByName( "channels", channels );
            break;
        }
        case TEXTUREOPERATION_PREMULTIPLYALPHA:
        case TEXTUREOPERATION_UNPREMULTIPLYALPHA:
        case TEXTUREOPERATION_COLOURSPACETAG:
            node.SetAttribute( "colourSpace", IMAGE::ToString( colourSpace ) );
            break;
        default:
            break;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTextureOperation::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CTextureOperation op;
    if ( !FromString( node.GetAttributeValueOrChildValueByName( "type" ).AsString(), op.type ) )
        return false;
    op.channel = node.GetAttributeValueOrChildValueByName( "channel" ).AsUInt32( 0 );
    op.channelMask = node.GetAttributeValueOrChildValueByName( "channelMask" ).AsUInt32( 0 );
    op.scale = node.GetAttributeValueOrChildValueByName( "scale" ).AsFloat32( 1.0f );
    op.bias = node.GetAttributeValueOrChildValueByName( "bias" ).AsFloat32( 0.0f );
    op.mapping = node.GetAttributeValueOrChildValueByName( "mapping" ).AsString().Uppercase();
    IMAGE::FromString( node.GetAttributeValueOrChildValueByName( "colourSpace" ).AsString(), op.colourSpace );
    CORE::CDataNode::TVariantVector sources;
    CORE::CDataNode::TVariantVector channels;
    node.GetValuesOfChildByName( "sources", sources );
    node.GetValuesOfChildByName( "channels", channels );
    for ( UInt32 i=0; i<4; ++i )
    {
        if ( i < sources.size() )
            op.packSources[ i ] = sources[ i ].AsInt32( -1 );
        if ( i < channels.size() )
            op.packChannels[ i ] = channels[ i ].AsUInt32( 0 );
    }
    *this = op;
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
