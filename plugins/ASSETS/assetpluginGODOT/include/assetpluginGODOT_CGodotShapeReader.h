/*
 *  assetpluginGODOT: the collision shapes of Godot
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTSHAPEREADER_H
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTSHAPEREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

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
 *  Turns the shape resources of Godot into the colliders of the model (the shapes of OMI_physics_shape):
 *  BoxShape3D (size) a box, SphereShape3D (radius) a sphere, CapsuleShape3D and CylinderShape3D (radius, height) a
 *  capsule and a cylinder along Y, ConvexPolygonShape3D (points) a convex shape with the hull of its points as mesh,
 *  and ConcavePolygonShape3D (data, three points per triangle) a trimesh. The scale of the shape node relative to its
 *  body is baked into the dimensions: a collider has no scale. HeightMapShape3D, WorldBoundaryShape3D,
 *  SeparationRayShape3D and the 2D shapes are reported and give no collider.
 */
class GUCEF_ASSETPLUGIN_GODOT_EXPORT_CPP CGodotShapeReader
{
    public:

    static bool IsShapeType( const CORE::CString& type );

    /**
     *  Fills the collider from the shape section. 'relative' is the transform of the shape node in the frame of its
     *  body: the collider gets its translation and rotation, and the dimensions its scale. Meshes of convex and
     *  trimesh shapes are added to the scene. False with a report entry for a shape that gives no collider.
     */
    static bool Read( const CORE::CString& type, const CORE::CDataNode& section, const MATH::TFloat32Transform3D& relative, bool trigger, const CORE::CString& name, ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CAssetCollider& collider, ASSETCONVERTER::CConversionReport& report );

    private:

    CGodotShapeReader( void );
    ~CGodotShapeReader();
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

#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTSHAPEREADER_H ? */
