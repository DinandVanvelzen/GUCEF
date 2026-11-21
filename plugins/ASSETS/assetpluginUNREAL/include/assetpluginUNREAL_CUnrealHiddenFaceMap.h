/*
 *  assetpluginUNREAL: the hidden face map of an outfit: which faces of the body the outfit hides
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALHIDDENFACEMAP_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALHIDDENFACEMAP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETMESH_H
#include "AssetConverterLib_CAssetMesh.h"
#define ASSETCONVERTERLIB_CASSETMESH_H
#endif /* ASSETCONVERTERLIB_CASSETMESH_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#include "assetpluginUNREAL_macros.h"
#define GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The hidden face map of an outfit: a grey image in the UV layout of the body, black where the outfit covers the body and
 *  white where the body stays visible. The engine applies it when it assembles the character: a triangle of the body whose
 *  covered texels all lie below the cull value is removed; vertices in regions between the cull value and the keep value
 *  are moved inwards along their normals (shrunk), by up to the shrink distance.
 *
 *  This class decides per triangle in the same way. A triangle covers the texels whose centres lie inside it in texture
 *  space, and always the texels of its corners and of the middles of its edges (so that a triangle smaller than a texel
 *  covers at least one). The image is sampled with the UVs as the intermediate stores them: u to the right, v downwards
 *  from the top row, within the UV tile the map belongs to. The faces are not removed: HideFaces() moves the triangles
 *  that the map hides into submeshes of their own, marked with the bit of the hidden face set of the outfit.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealHiddenFaceMap
{
    public:

    static const Float32 DefaultMaxCullValue;       /**< 0.1: assumed, the owned outfits do not store the value */
    static const Float32 DefaultMinKeepValue;       /**< 0.9: assumed, as above */

    typedef GUCEF::vector< UInt8 > TByteVector;
    typedef GUCEF::set< Int32 > TInt32Set;

    /** What HideFaces() did */
    struct SResult
    {
        UInt32 triangles;           /**< triangles of the mesh that lie in the tile of the map */
        UInt32 hiddenTriangles;     /**< of those, hidden by the map */
        UInt32 otherTriangles;      /**< triangles outside the tile (or crossing tiles) or with materials left alone: never hidden */
        UInt32 newSubMeshes;        /**< submeshes made for the hidden triangles */
        UInt32 bandVertices;        /**< vertices of shown triangles whose map value lies between the cull and the keep value */

        SResult( void );
    };

    CUnrealHiddenFaceMap( void );

    /**
     *  Takes the first channel of the pixels (8 bit, 'channels' samples per pixel, rows from the top).
     *  @return false when the sizes do not match the pixels
     */
    bool SetImage( const TByteVector& pixels, UInt32 width, UInt32 height, UInt32 channels );

    UInt32 GetWidth( void ) const;
    UInt32 GetHeight( void ) const;

    void SetMaxCullValue( Float32 value );
    Float32 GetMaxCullValue( void ) const;

    void SetMinKeepValue( Float32 value );
    Float32 GetMinKeepValue( void ) const;

    /** The value (0 to 1) of the texel at u, v (0 to 1, clamped) */
    Float32 Sample( Float32 u, Float32 v ) const;

    /** Whether every texel the triangle covers lies below the cull value. The UVs are those of the map (0 to 1) */
    bool IsHidden( const Float32* uv0, const Float32* uv1, const Float32* uv2 ) const;

    /** The share of the texels of the map below the cull value, and between the cull and the keep value, for the report */
    void GetCoverage( Float32& hiddenShare, Float32& bandShare ) const;

    /**
     *  Moves the triangles that the map hides into submeshes of their own with 'setBit' added to their hiddenBy (a submesh whose
     *  triangles are all hidden only gets the bit). The mesh keeps every triangle.
     *  @param tile the UV tile column (0 for u in [0,1), 1 for [1,2), ...) of UV set 0 that the map covers; its UVs are taken minus the column
     *  @param leaveAlone submeshes with these materials are not hidden (the parts of the body in other tiles)
     */
    void HideFaces( ASSETCONVERTER::CAssetMesh& mesh, UInt32 setBit, Int32 tile, const TInt32Set& leaveAlone, SResult& result ) const;

    private:

    UInt8 Texel( Int32 x, Int32 y ) const;

    /** Whether a texel the triangle covers has a value of at least 'threshold' (0 to 255); the corners in texel units */
    bool CoversAtLeast( const Float32* a, const Float32* b, const Float32* c, UInt32 threshold ) const;

    TByteVector m_values;
    UInt32 m_width;
    UInt32 m_height;
    Float32 m_maxCullValue;
    Float32 m_minKeepValue;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALHIDDENFACEMAP_H ? */
