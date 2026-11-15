/*
 *  assetpluginUNITY: the .meta file of an asset
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYMETA_H
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYMETA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYREFERENCE_H
#include "assetpluginUNITY_CUnityReference.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYREFERENCE_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYREFERENCE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYCLIPANIMATION_H
#include "assetpluginUNITY_CUnityClipAnimation.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYCLIPANIMATION_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYCLIPANIMATION_H ? */

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
 *  The .meta file of an asset: its GUID, the importer and the settings the importer of the converter uses. The key
 *  names are those found in real files (Unity 5.x to 2017.x, and the newer object keys):
 *
 *  - the ID table of objects inside a model or texture: "fileIDToRecycleName" (a map from number to name; the number
 *    is classId * 100000 + 2 * index), or its later form "internalIDToNameTable" (items { first: { <classId>: <id> },
 *    second: <name> }). Names can repeat, so the key of a lookup is the number;
 *  - ModelImporter: meshes.globalScale, meshes.useFileScale (from version 18), meshes.useFileUnits (older),
 *    meshes.addColliders, materials.importMaterials/materialName/materialSearch, the newer
 *    materialImportMode/materialLocation and externalObjects (optional), animations.clipAnimations, importAnimation,
 *    animationType, bakeAxisConversion (optional, default false);
 *  - TextureImporter: textureType, mipmaps.sRGBTexture (version 4) or mipmaps.linearTexture (version 2),
 *    textureSettings.wrapMode/wrapU/wrapV/filterMode, maxTextureSize. The colour space of a texture is decided by the
 *    material slot that uses it, not by these flags; they are kept for the report.
 */
class GUCEF_ASSETPLUGIN_UNITY_EXPORT_CPP CUnityMeta
{
    public:

    struct SIdEntry
    {
        Int32 classId;
        CORE::CString name;
    };
    typedef GUCEF::map< Int64, SIdEntry > TIdTable;
    typedef GUCEF::vector< CUnityClipAnimation > TClipVector;
    typedef GUCEF::map< CORE::CString, CUnityReference > TReferenceMap;

    CORE::CString guid;
    CORE::CString importerType;         /**< "ModelImporter", "TextureImporter", "NativeFormatImporter", ... */
    Int32 importerVersion;              /**< serializedVersion of the importer */
    bool folderAsset;
    Int64 mainObjectFileId;             /**< NativeFormatImporter: mainObjectFileID, 0 when absent */

    TIdTable idTable;
    CORE::CString idTableKey;           /**< the key the table came from: "fileIDToRecycleName" or "internalIDToNameTable" */

    // ModelImporter
    Float64 globalScale;
    bool hasUseFileScale;               /**< meshes.useFileScale is present (version 18 and later) */
    bool useFileScale;
    bool useFileUnits;
    bool bakeAxisConversion;
    bool importMaterials;
    Int32 materialName;
    Int32 materialSearch;
    Int32 materialImportMode;           /**< -1 when absent */
    bool addColliders;
    bool importAnimation;
    Int32 animationType;                /**< 0 none, 1 legacy, 2 generic, 3 humanoid */
    TClipVector clips;
    TReferenceMap externalObjects;      /**< by the name of the object inside the model */

    // TextureImporter
    Int32 textureType;
    Int32 sRGBTexture;                  /**< -1 when absent */
    Int32 linearTexture;                /**< -1 when absent */
    Int32 wrapMode;
    Int32 wrapU;                        /**< -1 when absent */
    Int32 wrapV;                        /**< -1 when absent */
    Int32 filterMode;
    Int32 maxTextureSize;

    CORE::CDataNode root;               /**< the whole file */

    CUnityMeta( void );

    /** Reads the .meta file at the VFS path */
    bool Load( const CORE::CString& vfsPath, CORE::CString& problem );

    /** Reads the text of a .meta file */
    bool Parse( const CORE::CString& text, CORE::CString& problem );

    /** Reads the values of the mapping of a .meta file */
    bool Read( const CORE::CDataNode& mapping );

    bool IsModel( void ) const;

    bool IsTexture( void ) const;

    /** The entry of the ID table, false when the ID is not in it */
    bool TryGetIdEntry( Int64 fileId, SIdEntry& entry ) const;

    /**
     *  The rank of the ID among the entries of the same class with the same name, by ascending ID (0 for the first):
     *  the occurrence to take when several objects of a model have the name
     */
    UInt32 GetNameRank( Int64 fileId ) const;

    /** The IDs of the entries of a class, ascending */
    void GetIdsOfClass( Int32 classId, GUCEF::vector< Int64 >& ids ) const;

    /**
     *  The scale of a model: globalScale * ( useFileScale ? metres per file unit : 1 ), where files without the key
     *  useFileScale (before serializedVersion 18) do not apply the unit of the file. Inferred from the pairs of
     *  globalScale and UnitScaleFactor in real packs and checked against the child positions of their prefabs
     *  (serializedVersion 16 with globalScale 1 and 18 with globalScale 100 and useFileScale 1, both over centimetre
     *  files, give 1); 'fileUnitMetres' is the length of a unit of the model file in metres (0.01 for centimetres).
     */
    Float64 GetEffectiveScale( Float64 fileUnitMetres ) const;

    /** Whether the unit of the file applies: useFileScale where present; files without the key do not apply it */
    bool AppliesFileScale( void ) const;
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

#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYMETA_H ? */
