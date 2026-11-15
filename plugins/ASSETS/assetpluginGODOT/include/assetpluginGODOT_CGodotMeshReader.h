/*
 *  assetpluginGODOT: the meshes of Godot resources
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTMESHREADER_H
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTMESHREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETMESH_H
#include "AssetConverterLib_CAssetMesh.h"
#define ASSETCONVERTERLIB_CASSETMESH_H
#endif /* ASSETCONVERTERLIB_CASSETMESH_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_MACROS_H
#include "assetpluginGODOT_macros.h"
#define GUCEF_ASSETPLUGIN_GODOT_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GODOT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads the surfaces of an ArrayMesh resource (the property "_surfaces", an array of dictionaries) into the mesh of the
 *  model. A surface holds:
 *
 *  - "format": the flags of the array formats of the rendering server (vertex 1, normal 2, tangent 4, colour 8, UV 16,
 *    UV2 32, custom 0 to 3 at 64 to 512, bones 1024, weights 2048, index 4096; 2D vertices, 8 bone weights and
 *    compressed attributes as the flags 1 << 25, 1 << 27 and 1 << 29);
 *  - "primitive" (triangles 3, triangle strip 4; points and lines are skipped), "vertex_count", "index_count";
 *  - "vertex_data": the positions of every vertex (three or, for 2D vertices, two floats), then the normals and
 *    tangents of every vertex in an octahedral packing of two 16 bit values each;
 *  - "attribute_data": per vertex the colour (four bytes), the UV and UV2 (two floats each) and the custom values;
 *  - "skin_data": per vertex the bone indices and the weights (16 bit values; not carried: a skeleton is not part of a
 *    text scene), "index_data": 16 bit indices, 32 bit above 65535 vertices;
 *  - "material": the reference of the material, handed back for the caller to resolve in its document.
 *
 *  Godot winds front faces clockwise: the triangles are turned over to glTF's counter clockwise winding. A surface of
 *  a layout this reader does not know (compressed attributes, a size of the data that does not fit the format) is left
 *  out with a report entry and keeps its place as an empty submesh, so the surface numbers of material overrides
 *  stay right.
 */
class GUCEF_ASSETPLUGIN_GODOT_EXPORT_CPP CGodotMeshReader
{
    public:

    typedef GUCEF::vector< const CORE::CDataNode* > TNodeVector;

    /**
     *  Reads the surfaces into the submeshes of 'mesh' (materialIndex -1); 'surfaceMaterials' gets per surface the
     *  value of its "material" member (NULL for none). False when the array holds no surface at all.
     */
    static bool ReadSurfaces( const CORE::CDataNode& surfaces, ASSETCONVERTER::CAssetMesh& mesh, TNodeVector& surfaceMaterials, ASSETCONVERTER::CConversionReport& report, const CORE::CString& subject );

    /** The unit vector of an octahedral packing, the two values in [ 0, 1 ] */
    static void DecodeOctahedron( Float32 x, Float32 y, Float32 out[ 3 ] );

    /** The tangent of an octahedral packing with the sign of the binormal in 'sign' */
    static void DecodeTangent( Float32 x, Float32 y, Float32 out[ 3 ], Float32& sign );

    private:

    CGodotMeshReader( void );
    ~CGodotMeshReader();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GODOT */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTMESHREADER_H ? */
