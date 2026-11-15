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

#ifndef ASSETCONVERTERLIB_CTEXTUREOPERATION_H
#define ASSETCONVERTERLIB_CTEXTUREOPERATION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_IMAGE_ETYPES_H
#include "gucefIMAGE_ETypes.h"
#define GUCEF_IMAGE_ETYPES_H
#endif /* GUCEF_IMAGE_ETYPES_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One step of the operation chain of a CAssetTexture. The steps run in order on the picture of the first source of
 *  the texture, or on the picture a TEXTUREOPERATION_PACK step builds from several sources, through the operations of
 *  gucefIMAGE (CChannelOperations). The members that a type does not use keep their defaults.
 *
 *  A value type. ToKeyText() is the canonical text of the step: the baker names a baked texture by the hash of the
 *  sources and of the texts of the steps that change texels, so the result of an operation chain is written once.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CTextureOperation
{
    public:

    TTextureOperationType type;
    UInt32 channel;                     /**< EXTRACTCHANNEL: the channel, in the logical order R, G, B, A */
    UInt32 channelMask;                 /**< INVERT, SCALEBIAS: bit i selects storage channel i */
    Float32 scale;                      /**< SCALEBIAS */
    Float32 bias;                       /**< SCALEBIAS */
    CORE::CString mapping;              /**< SWIZZLE: one, three or four of R, G, B, A, 0 and 1 */
    Int32 packSources[ 4 ];             /**< PACK: per channel of the result the index of a source of the texture, -1 for a constant */
    UInt32 packChannels[ 4 ];           /**< PACK: per channel of the result the channel of that source */
    IMAGE::TColourSpace colourSpace;    /**< COLOURSPACETAG: the tag; PREMULTIPLYALPHA, UNPREMULTIPLYALPHA: the space of the colour */

    CTextureOperation( void );

    static CTextureOperation ExtractChannel( UInt32 channel );
    static CTextureOperation Swizzle( const CORE::CString& mapping );
    static CTextureOperation Invert( UInt32 channelMask );
    static CTextureOperation ScaleBias( UInt32 channelMask, Float32 scale, Float32 bias );
    static CTextureOperation Pack( const Int32 sources[ 4 ], const UInt32 channels[ 4 ] );
    static CTextureOperation GreenFlip( void );
    static CTextureOperation PremultiplyAlpha( IMAGE::TColourSpace colourSpace );
    static CTextureOperation UnpremultiplyAlpha( IMAGE::TColourSpace colourSpace );
    static CTextureOperation ColourSpaceTag( IMAGE::TColourSpace colourSpace );

    /** False for a tag: the step names something but changes no texel */
    bool ChangesTexels( void ) const;

    /** The canonical text of the step, with only the members its type uses */
    CORE::CString ToKeyText( void ) const;

    bool operator==( const CTextureOperation& other ) const;
    bool operator!=( const CTextureOperation& other ) const;

    /** As attributes of the node: "type" and the members its type uses */
    bool Serialize( CORE::CDataNode& node ) const;
    bool Deserialize( const CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CTEXTUREOPERATION_H ? */
