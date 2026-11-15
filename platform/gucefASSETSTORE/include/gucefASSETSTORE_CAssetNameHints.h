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

#ifndef GUCEF_ASSETSTORE_CASSETNAMEHINTS_H
#define GUCEF_ASSETSTORE_CASSETNAMEHINTS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the names and extensions of files tell (sections 7.2 and 9.5): the kind of an asset, its format, the role of
 *  a texture, levels of detail and collision meshes. The naming conventions are the common ones of game art:
 *  SM_ (static mesh), SK_ and SKM_ (skeletal mesh), T_ (texture), M_ and MI_ (material), A_ and ANIM_ (animation),
 *  UCX_, UBX_, USP_ and UCP_ (collision), _LOD0 .. _LODn, -col; texture roles by suffix: _D, _BaseColor, _Albedo,
 *  _Diffuse, _N, _Normal, _R, _Roughness, _M, _Metallic, _AO, _E, _Emissive, _H, _Height, _ORM, _Mask, _Opacity.
 *  Kind ids are those of the vocabulary: static-model, skeletal-model, material, texture, decal, hdri, animation,
 *  music, sound-effect, ambient-sound, voice, vfx, sprite, icon, font, ui-kit, scene, prefab, code, template,
 *  document, other.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetNameHints
{
    public:

    /** The lower case extension of a file name without the dot, "" without one */
    static CORE::CString GetExtension( const CORE::CString& fileName );

    /** The file name of a path without its folders and its extension */
    static CORE::CString GetBaseName( const CORE::CString& path );

    /** The format id of an extension: "fbx", "gltf" (gltf, glb), "obj", "usd" (usd, usda, usdc, usdz), "blender", "png", ... */
    static CORE::CString FormatFromExtension( const CORE::CString& extension );

    /** The kind the extension alone suggests, "other" when it suggests none */
    static CORE::CString KindFromExtension( const CORE::CString& extension );

    /** The kind of a file from its path (folders and name) and extension */
    static CORE::CString GuessKind( const CORE::CString& path );

    /** Whether the kind is one of the model kinds */
    static bool IsModelKind( const CORE::CString& kind );

    /** Whether files of the extension are archives the VFS may be able to mount */
    static bool IsArchiveExtension( const CORE::CString& extension );

    /** The role of a texture from its name: "albedo", "normal", "roughness", "metallic", "ao", "emissive", "height", "orm", "mask", "opacity"; "" when the name does not tell */
    static CORE::CString GuessTextureRole( const CORE::CString& name );

    /** The level of detail of a name ending in _LODn, -1 without one */
    static Int32 GetLodIndex( const CORE::CString& name );

    /** Whether the name is that of a collision mesh */
    static bool IsCollisionName( const CORE::CString& name );

    private:

    CAssetNameHints( void );    /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETNAMEHINTS_H ? */
