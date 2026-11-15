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

#ifndef ASSETCONVERTERLIB_CASSETTEXTURE_H
#define ASSETCONVERTERLIB_CASSETTEXTURE_H

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

#ifndef ASSETCONVERTERLIB_CTEXTUREOPERATION_H
#include "AssetConverterLib_CTextureOperation.h"
#define ASSETCONVERTERLIB_CTEXTUREOPERATION_H
#endif /* ASSETCONVERTERLIB_CTEXTUREOPERATION_H ? */


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
 *  A texture of an asset: the VFS paths of its sources and the operation chain that turns them into the texture the
 *  intermediate uses (channel extract, invert, repack, green flip, alpha premultiplication, colour space tag). The
 *  importers fill the sources and the operations; the baker (CTextureBaker) writes the result once per unique chain
 *  and fills the members of the baked result.
 *
 *  Without a PACK step the chain works on the picture of the first source. A value type.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetTexture
{
    public:

    typedef GUCEF::vector< CTextureOperation > TOperationVector;

    CORE::CString name;
    CORE::CString::StringVector sources;    /**< VFS paths; the first is the main source */
    TOperationVector operations;
    TTextureUsage usage;
    IMAGE::TWrapMode wrapS;
    IMAGE::TWrapMode wrapT;

    // the baked result, filled by the baker
    CORE::CString bakedPath;                /**< VFS path of the baked file */
    UInt32 width;
    UInt32 height;
    bool missing;                           /**< a source could not be read or the chain failed */

    CAssetTexture( void );

    /** The colour space of the result: the last tag of the chain, else linear for normal maps and masks, sRGB for colour */
    IMAGE::TColourSpace GetColourSpace( void ) const;

    /** Whether a step of the chain changes texels */
    bool HasTexelOperations( void ) const;

    /** The canonical text of the steps that change texels, in order */
    CORE::CString GetChainKeyText( void ) const;

    /** Adds a green flip at the end of the chain: the normalisation of DirectX normal maps */
    void AddGreenFlip( void );

    /** Name, sources, operations, usage, wrap modes and the baked file (without the sizes, which the summary has) */
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

#endif /* ASSETCONVERTERLIB_CASSETTEXTURE_H ? */
