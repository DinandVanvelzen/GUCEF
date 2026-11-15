/*
 *  AssetConverterLib: the manifest of a pack: what to convert, how, and where to
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

#ifndef ASSETCONVERTERLIB_CASSETPACKMANIFEST_H
#define ASSETCONVERTERLIB_CASSETPACKMANIFEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPRESET_H
#include "AssetConverterLib_CAssetPreset.h"
#define ASSETCONVERTERLIB_CASSETPRESET_H
#endif /* ASSETCONVERTERLIB_CASSETPRESET_H ? */

#ifndef ASSETCONVERTERLIB_CPLUGINSTACKENTRY_H
#include "AssetConverterLib_CPluginStackEntry.h"
#define ASSETCONVERTERLIB_CPLUGINSTACKENTRY_H
#endif /* ASSETCONVERTERLIB_CPLUGINSTACKENTRY_H ? */

#ifndef ASSETCONVERTERLIB_CCOOKPROFILE_H
#include "AssetConverterLib_CCookProfile.h"
#define ASSETCONVERTERLIB_CCOOKPROFILE_H
#endif /* ASSETCONVERTERLIB_CCOOKPROFILE_H ? */


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
 *  A pack manifest, <pack>.assetpack.json, kept with the game or in a private folder, never in GUCEF:
 *
 *  - "id", "title", "store", "licenceRecord" (the identifier of the record <licenceFolder>/<id>.licence.json);
 *    "licenceFolder" (default: the folder of the manifest);
 *  - "source": a VFS folder or an archive the pipeline mounts; "importer": a name or "auto" (the first importer, by
 *    name, that can import a file); "include" and "exclude": globs on the paths in the pack (default include "**");
 *    "assets": globs of the files that are converted as assets; the files that pass "include" and "exclude" but not
 *    these stay visible to the importers that look things up (textures, nested scenes, models) and are not converted
 *    themselves (default: every file an importer can import);
 *  - "rules": the pack-wide settings of every asset (shader property maps, material overrides, "coordinateSystem" or
 *    "unitsPerMetre" overrides, "lod" { "policy": "source", "generate" or "none", ... }, "colliders" { "policy" },
 *    "animationClips"), and "nameHints" ("unreal", "godot", "both", "none" or "auto");
 *  - "presets": [ { "match", "importer", "settings" } ], top to bottom; "overrides": the folder of the per-asset
 *    overrides <overrides>/<asset path>.import.json;
 *  - "postProcessors": { "<source format>": [ entries ], "*": [ entries ] }; "validators": [ entries ] (without the
 *    member the built-in MissingTextures, TextureSize and BoneLimit run with their defaults);
 *  - "exporter" (default "gltf"), "intermediate" (default "intermediate"), "output" (default "out"),
 *    "cookTargets" [ names ], "cookProfiles" [ profiles ] (default one "desktop" profile).
 *
 *  Paths in a manifest are VFS paths; a path that does not start with '/' is relative to the folder of the manifest.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetPackManifest
{
    public:

    typedef GUCEF::map< CORE::CString, CPluginStackEntry::TEntryVector > TStackMap;
    typedef GUCEF::vector< CCookProfile > TProfileVector;

    static const CORE::CString AutoImporter;        /**< "auto" */
    static const CORE::CString AnySourceFormat;     /**< "*" */

    CORE::CString id;
    CORE::CString title;
    CORE::CString store;
    CORE::CString licenceRecord;
    CORE::CString licenceFolder;
    CORE::CString source;
    CORE::CString importer;
    CORE::CString::StringVector include;
    CORE::CString::StringVector exclude;
    CORE::CString::StringVector assets;
    CORE::CDataNode rules;
    CORE::CString nameHints;
    CAssetPreset::TPresetVector presets;
    CORE::CString overrides;
    TStackMap postProcessors;
    bool hasValidators;
    CPluginStackEntry::TEntryVector validators;
    CORE::CString exporter;
    CORE::CString intermediate;
    CORE::CString output;
    CORE::CString::StringVector cookTargets;
    TProfileVector cookProfiles;

    /** The folder of the manifest, against which relative paths resolve (set by Load()) */
    CORE::CString manifestFolder;

    CAssetPackManifest( void );

    /** Reads <pack>.assetpack.json from the VFS; the folder of the file becomes the manifest folder */
    bool Load( const CORE::CString& vfsPath );

    bool Serialize( CORE::CDataNode& node ) const;
    bool Deserialize( const CORE::CDataNode& node );

    /** A path of the manifest as a VFS path: relative to the manifest folder unless it starts with '/' */
    CORE::CString ResolvePath( const CORE::CString& path ) const;

    CORE::CString GetSourcePath( void ) const;
    CORE::CString GetIntermediatePath( void ) const;
    CORE::CString GetOutputPath( void ) const;
    CORE::CString GetOverridesPath( void ) const;

    /** <licenceFolder>/<licenceRecord>.licence.json; empty without a record */
    CORE::CString GetLicenceRecordPath( void ) const;

    /** Whether the path in the pack passes the include and exclude globs */
    bool IsIncluded( const CORE::CString& assetPath ) const;

    /** Whether the file is converted as an asset when an importer can import it: it passes the globs of "assets", when there are any */
    bool IsAssetSelected( const CORE::CString& assetPath ) const;

    /** The stack the manifest names for the source format (the entry of the format, else that of "*"); false when it names none */
    bool GetPostProcessorStack( const CORE::CString& sourceFormat, CPluginStackEntry::TEntryVector& stack ) const;

    /** The validators: those of the manifest, or the default list */
    void GetValidators( CPluginStackEntry::TEntryVector& list ) const;

    /** The pack-wide settings layer of every asset: the rules, plus "nameHints" */
    void GetPackSettings( CORE::CDataNode& settings ) const;

    /** The settings of the manifest that change the processing of every asset (stacks, validators, exporter, cooking), for the cache key */
    void GetProcessingSettings( CORE::CDataNode& settings ) const;

    /** The LOD and collider policies of the rules ("lod"/"policy", "colliders"/"policy"), default "source" */
    CORE::CString GetLodPolicy( void ) const;
    CORE::CString GetColliderPolicy( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETPACKMANIFEST_H ? */
