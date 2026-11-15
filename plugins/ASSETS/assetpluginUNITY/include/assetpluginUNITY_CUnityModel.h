/*
 *  assetpluginUNITY: a model file of the project as Unity imports it
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYMODEL_H
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYMODEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYMETA_H
#include "assetpluginUNITY_CUnityMeta.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYMETA_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYMETA_H ? */

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

class CUnityProject;

/**
 *  A model file of the project (FBX, glTF, ...) read by the importer of its format through the plugin registry and
 *  brought into Unity's space the way Unity's model import does it:
 *
 *  - the scale is that of the .meta file (CUnityMeta::GetEffectiveScale(), inferred from real packs);
 *  - mesh data is kept in the frame of its node, mirrored on X (right- to left-handed) and scaled, which is the
 *    Mesh asset Unity makes; the prefabs of the project place those meshes with their own transforms;
 *  - the node transforms are mirrored the same way, and the top-level nodes get the rotation of the axis conversion
 *    of the file (Unity's import adds it there), so that the hierarchy of the model shows it as Unity does;
 *  - skins and animation takes follow the nodes;
 *  - node names are made unique as Unity does: a later node with the name of an earlier one gets " 1", " 2", ...
 *    appended (the prefabs and ID tables use those names).
 *
 *  The objects of the model are found by the IDs of the .meta file (FindMeshNode(), FindNode()): the ID table names
 *  them, and the rank among equal names picks one of several with the same name.
 */
class GUCEF_ASSETPLUGIN_UNITY_EXPORT_CPP CUnityModel
{
    public:

    ASSETCONVERTER::CAssetScene scene;          /**< in Unity's coordinates (MATH::CCoordinateSystem::Unity()) */
    CUnityMeta meta;
    bool hasMeta;
    CORE::CString vfsPath;
    CORE::CString importerName;
    MATH::CCoordinateSystem fileSystem;         /**< the axes and unit of the file, as its importer stated them */
    Float64 fileUnitMetres;                     /**< the length of a unit of the file in metres */
    Float64 scale;                              /**< metres per file unit after the import settings */

    CUnityModel( void );

    /** The importer of the converter that reads a model file with the extension ("fbx", "gltf"); empty when none */
    static CORE::CString GetImporterNameForExtension( const CORE::CString& extension );

    /** Whether a file with the extension is a model file the project can read */
    static bool IsModelExtension( const CORE::CString& extension );

    /** Reads the file through its importer and converts it; the findings go to the report of the project */
    bool Load( CUnityProject& project, const CORE::CString& modelVfsPath );

    /**
     *  The node that holds the mesh with the class-43 ID of the .meta file; -1 when it is not found. An ID the table
     *  does not have (the later scheme computes IDs from names and keeps no table) is resolved, with 'guessed' set,
     *  to the only mesh of a model with one mesh, else to the mesh node named 'nameHint' (the name of the object that
     *  uses it, which Unity gives the name of the node).
     */
    Int32 FindMeshNode( Int64 fileId, const CORE::CString& nameHint = CORE::CString::Empty, bool* guessed = GUCEF_NULL ) const;

    /**
     *  The node of a GameObject or component ID of the .meta file (classes 1, 4, 23, 33, 137) or of the root objects
     *  of the later scheme (CUnityBuiltins::IsModelRootObjectId()); -1 when it is not found
     */
    Int32 FindNode( Int64 fileId ) const;

    /** The node that is the root object of the model in Unity: the only top-level node, -1 when there are several */
    Int32 GetRootNode( void ) const;

    /**
     *  Splits the takes into the clips of the .meta file (clipAnimations: the frames of a take, its loop flag), or
     *  names the takes after the class-74 entries of the ID table when the file defines no clips. Takes without
     *  importAnimation are removed. The clips record their ID of the .meta file in 'clipIds' (by clip name).
     */
    void ApplyClips( CUnityProject& project, CORE::CDataNode& clipIds );

    /** Copies the frames from 'from' to 'to' (in seconds of the take) of a sampled clip, starting at 0 */
    static void CutAnimation( const ASSETCONVERTER::CAssetAnimation& take, Float32 from, Float32 to, ASSETCONVERTER::CAssetAnimation& clip );

    private:

    Int32 FindNamedNode( const CORE::CString& name, UInt32 rank, bool withMesh ) const;

    /** Makes the node names unique the way Unity does; the names of the file are kept in 'm_fileNames' */
    void MakeNamesUnique( void );

    CORE::CString::StringVector m_fileNames;
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

#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYMODEL_H ? */
