/*
 *  assetpluginGODOT: the primitive meshes of Godot
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTPRIMITIVEMESHES_H
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTPRIMITIVEMESHES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETMESH_H
#include "AssetConverterLib_CAssetMesh.h"
#define ASSETCONVERTERLIB_CASSETMESH_H
#endif /* ASSETCONVERTERLIB_CASSETMESH_H ? */

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
 *  Builds the meshes of the primitive mesh resources of Godot from their properties: BoxMesh (size), PlaneMesh and
 *  QuadMesh (size, subdivide_width, subdivide_depth, orientation, center_offset), SphereMesh (radius, height,
 *  radial_segments, rings, is_hemisphere), CylinderMesh (top_radius, bottom_radius, height, radial_segments, rings,
 *  cap_top, cap_bottom) and CapsuleMesh (radius, height, radial_segments, rings), with the defaults of Godot. The
 *  meshes have positions, normals and UV coordinates, wound counter clockwise as glTF has it; the UV layout is the one
 *  of this builder, not the one of Godot. The material of the resource is the caller's to read.
 */
class GUCEF_ASSETPLUGIN_GODOT_EXPORT_CPP CGodotPrimitiveMeshes
{
    public:

    /** Whether the resource type is one of the primitive meshes above */
    static bool IsPrimitiveType( const CORE::CString& type );

    /** Builds the mesh (one submesh) of the resource section; false for a type that is not supported */
    static bool Build( const CORE::CString& type, const CORE::CDataNode& section, ASSETCONVERTER::CAssetMesh& mesh );

    private:

    CGodotPrimitiveMeshes( void );
    ~CGodotPrimitiveMeshes();
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

#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTPRIMITIVEMESHES_H ? */
