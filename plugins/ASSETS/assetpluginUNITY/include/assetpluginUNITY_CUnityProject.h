/*
 *  assetpluginUNITY: the Unity project of one import
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYPROJECT_H
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYPROJECT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#include "AssetConverterLib_CAssetPackContext.h"
#define ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#endif /* ASSETCONVERTERLIB_CASSETPACKCONTEXT_H ? */

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLFILE_H
#include "assetpluginUNITY_CUnityYamlFile.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLFILE_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLFILE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYMETA_H
#include "assetpluginUNITY_CUnityMeta.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYMETA_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYMETA_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYSHADERINFO_H
#include "assetpluginUNITY_CUnityShaderInfo.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYSHADERINFO_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYSHADERINFO_H ? */

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

class CUnityModel;

/**
 *  The Unity project as one import sees it: GUIDs resolved to VFS paths through the index of the pack (built by
 *  CUnityImporter::IndexPack(), the index "unity.guid"), and the files of the project read once per import and kept:
 *  .meta files, YAML files, shaders and the models (through the importers of their format). Every file read is a
 *  dependency of the asset. It also holds the scene the import fills and the elements already made in it (materials,
 *  textures, meshes), so that what several objects share is made once, and it reports each finding once.
 *
 *  An instance lives for one Import() call on one thread.
 */
class GUCEF_ASSETPLUGIN_UNITY_EXPORT_CPP CUnityProject
{
    public:

    static const CORE::CString GuidIndexName;   /**< "unity.guid": GUID to the VFS path of the asset */
    static const CORE::CString SourceName;      /**< "unity": the source of the report items */
    static const CORE::CString SettingsName;    /**< "unity": the member of the effective settings */

    CUnityProject( ASSETCONVERTER::CAssetPackContext& context, ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CConversionReport& report, const CORE::CString& assetVfsPath );

    ~CUnityProject();

    ASSETCONVERTER::CAssetPackContext& GetContext( void );
    ASSETCONVERTER::CAssetScene& GetScene( void );
    ASSETCONVERTER::CConversionReport& GetReport( void );
    const CORE::CString& GetAssetPath( void ) const;

    /** The importer's member of the effective settings of the asset (an empty object when there is none) */
    const CORE::CDataNode& GetSettings( void ) const;

    /** A boolean of the settings */
    bool GetSettingBool( const CORE::CString& name, bool defaultValue ) const;

    /** The VFS path of the asset with the GUID; false when the pack does not have it */
    bool ResolveGuid( const CORE::CString& guid, CORE::CString& vfsPath ) const;

    /** The .meta file of the asset at the VFS path, NULL when there is none */
    const CUnityMeta* GetMeta( const CORE::CString& assetVfsPath );

    /** The YAML documents of the file, NULL when it cannot be read (reported) */
    const CUnityYamlFile* GetYaml( const CORE::CString& vfsPath );

    /** The shader of a .shader or .shadergraph file, NULL when it cannot be read */
    const CUnityShaderInfo* GetShader( const CORE::CString& vfsPath );

    /** The model file read through the importer of its format and brought into Unity's space; NULL when it cannot be */
    CUnityModel* GetModel( const CORE::CString& vfsPath );

    /** Reads a text file of the VFS */
    static bool LoadText( const CORE::CString& vfsPath, CORE::CString& text );

    /** The scene elements made for a key (a material by its reference, a texture by its chain, a mesh by its source and materials) */
    bool TryGetCached( const CORE::CString& key, Int32& index ) const;
    void SetCached( const CORE::CString& key, Int32 index );

    /** Adds a dependency of the asset */
    void AddDependency( const CORE::CString& vfsPath );

    /** Report items; the same subject and message are reported once per import */
    void Warn( const CORE::CString& subject, const CORE::CString& message );
    void Info( const CORE::CString& subject, const CORE::CString& message );
    void Approximate( const CORE::CString& subject, const CORE::CString& message );
    void Skip( const CORE::CString& subject, const CORE::CString& message );

    /** The text used in reports for a VFS path: the path in the pack where it is one */
    CORE::CString Describe( const CORE::CString& vfsPath ) const;

    private:

    bool Once( const CORE::CString& kind, const CORE::CString& subject, const CORE::CString& message );

    CUnityProject( const CUnityProject& src );              /**< not implemented */
    CUnityProject& operator=( const CUnityProject& src );   /**< not implemented */

    typedef GUCEF::map< CORE::CString, CUnityMeta* > TMetaMap;
    typedef GUCEF::map< CORE::CString, CUnityYamlFile* > TYamlMap;
    typedef GUCEF::map< CORE::CString, CUnityShaderInfo* > TShaderMap;
    typedef GUCEF::map< CORE::CString, CUnityModel* > TModelMap;
    typedef GUCEF::map< CORE::CString, Int32 > TIndexMap;

    ASSETCONVERTER::CAssetPackContext& m_context;
    ASSETCONVERTER::CAssetScene& m_scene;
    ASSETCONVERTER::CConversionReport& m_report;
    CORE::CString m_assetPath;
    CORE::CDataNode m_settings;
    TMetaMap m_metas;
    TYamlMap m_yamls;
    TShaderMap m_shaders;
    TModelMap m_models;
    TIndexMap m_cache;
    CORE::CString::StringSet m_reported;
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

#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYPROJECT_H ? */
