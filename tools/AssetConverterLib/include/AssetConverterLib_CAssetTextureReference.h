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

#ifndef ASSETCONVERTERLIB_CASSETTEXTUREREFERENCE_H
#define ASSETCONVERTERLIB_CASSETTEXTUREREFERENCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */


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
 *  The use of a texture of the scene by a slot of a material, as a glTF textureInfo: the texture, the UV set, the
 *  scale of a normal texture or the strength of an occlusion texture, and the KHR_texture_transform values (the tiling
 *  and offset of the source). A value type; a texture index below 0 means the slot is not used.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetTextureReference
{
    public:

    Int32 textureIndex;
    UInt32 texCoord;
    Float32 scale;              /**< normal: scale; occlusion: strength; other slots: unused */
    bool hasTransform;          /**< whether the KHR_texture_transform values below apply */
    Float32 offset[ 2 ];
    Float32 tiling[ 2 ];
    Float32 rotation;

    CAssetTextureReference( void );

    explicit CAssetTextureReference( Int32 texture, UInt32 uvSet = 0 );

    bool IsUsed( void ) const;

    bool operator==( const CAssetTextureReference& other ) const;
    bool operator!=( const CAssetTextureReference& other ) const;

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

#endif /* ASSETCONVERTERLIB_CASSETTEXTUREREFERENCE_H ? */
