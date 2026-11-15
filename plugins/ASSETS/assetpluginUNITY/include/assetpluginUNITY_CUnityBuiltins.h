/*
 *  assetpluginUNITY: the built-in resources of the Unity editor the importer knows
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYBUILTINS_H
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYBUILTINS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETMESH_H
#include "AssetConverterLib_CAssetMesh.h"
#define ASSETCONVERTERLIB_CASSETMESH_H
#endif /* ASSETCONVERTERLIB_CASSETMESH_H ? */

#ifndef ASSETCONVERTERLIB_CASSETMATERIAL_H
#include "AssetConverterLib_CAssetMaterial.h"
#define ASSETCONVERTERLIB_CASSETMATERIAL_H
#endif /* ASSETCONVERTERLIB_CASSETMATERIAL_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_MACROS_H
#include "assetpluginUNITY_macros.h"
#define GUCEF_ASSETPLUGIN_UNITY_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNITY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the importer knows of the built-in resources of the Unity editor and of the well-known IDs of Unity files.
 *  The packages name built-in shaders only by number ({fileID: <id>, guid: 0000000000000000f000000000000000, type: 0});
 *  the table holds the ids whose meaning is known (46 Standard, 45 Standard specular setup, 7 legacy diffuse) and the
 *  ranges of the particle shaders and of the sprite, UI and sky shaders, whose names are not stated anywhere. Other
 *  ids are unknown built-ins.
 *
 *  The built-in primitive meshes (guid 0000000000000000e000000000000000) are made here as Unity has them, in Unity's
 *  coordinates: the cube, the sphere, the plane and the quad.
 */
class GUCEF_ASSETPLUGIN_UNITY_EXPORT_CPP CUnityBuiltins
{
    public:

    /** What a material's m_Shader refers to */
    enum EShaderKind
    {
        SHADERKIND_STANDARD = 0         ,   /**< built-in 46 */
        SHADERKIND_STANDARDSPECULAR     ,   /**< built-in 45 */
        SHADERKIND_LEGACYDIFFUSE        ,   /**< built-in 7 */
        SHADERKIND_URPLIT               ,   /**< the URP Lit shader, by its GUID */
        SHADERKIND_HDRPLIT              ,   /**< inferred from the property set of an unresolved shader */
        SHADERKIND_SHADERGRAPH          ,   /**< a shader graph of the package */
        SHADERKIND_CUSTOM               ,   /**< a .shader of the package */
        SHADERKIND_BUILTINPARTICLE      ,   /**< a built-in particle or effect shader */
        SHADERKIND_BUILTINSPRITE        ,   /**< a built-in sprite, UI or sky shader */
        SHADERKIND_BUILTINUNKNOWN       ,   /**< another built-in id */
        SHADERKIND_MISSING              ,   /**< a shader outside the package, or none */
        SHADERKIND_COUNT
    };
    typedef enum EShaderKind TShaderKind;

    static const CORE::CString UrpLitShaderGuid;            /**< "933532a4fcc9baf4fa0491de14d08ed7" */

    static const Int64 MaterialMainObjectId = 2100000;
    static const Int64 TextureMainObjectId = 2800000;
    static const Int64 ShaderMainObjectId = 4800000;
    static const Int64 MonoScriptMainObjectId = 11500000;
    static const Int64 PrefabMainObjectId = 100100000;
    static const Int64 AvatarObjectId = 9000000;
    static const Int64 DefaultMaterialId = 10303;
    static Int64 GetShaderGraphMainObjectId( void );    /**< -6465566751694194690 */

    /**
     *  Whether the ID is one of the objects of the root of a model in the later ID scheme, in which the IDs of the
     *  objects of a model file are computed from their names, so that the root ("//RootNode") of every model has the
     *  same IDs: GameObject 919132149155446097, Transform -8679921383154817045, MeshRenderer -7511558181221131132
     *  (observed in packages; the IDs of other objects of such models are not known without their table)
     */
    static bool IsModelRootObjectId( Int64 fileId );

    /** The names of the kinds: "standard", "standardSpecular", "legacyDiffuse", "urpLit", ... */
    static const char* GetShaderKindName( TShaderKind kind );

    /** The kind and a name of a built-in shader id (type 0, guid 0000000000000000f000000000000000) */
    static TShaderKind ClassifyBuiltinShader( Int64 fileId, CORE::CString& name );

    /** The name of a built-in primitive mesh id: Cube 10202, Cylinder 10206, Sphere 10207, Capsule 10208, Plane 10209, Quad 10210 */
    static bool GetBuiltinMeshName( Int64 fileId, CORE::CString& name );

    /** Makes the built-in mesh in Unity's coordinates; false for the ids it does not make (cylinder, capsule) */
    static bool MakeBuiltinMesh( Int64 fileId, ASSETCONVERTER::CAssetMesh& mesh );

    /** The default material of the editor (built-in 10303): the Standard shader in white, smoothness 0.5 */
    static void MakeDefaultMaterial( ASSETCONVERTER::CAssetMaterial& material );

    private:

    CUnityBuiltins( void );
    ~CUnityBuiltins();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYBUILTINS_H ? */
