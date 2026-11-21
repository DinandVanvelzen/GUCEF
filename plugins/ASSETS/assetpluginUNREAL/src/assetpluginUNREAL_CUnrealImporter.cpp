/*
 *  assetpluginUNREAL: The importer unreal: recognises Unreal packages and reports what it can read
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

#ifndef ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#include "AssetConverterLib_CAssetPackContext.h"
#define ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#endif /* ASSETCONVERTERLIB_CASSETPACKCONTEXT_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include <algorithm>
#include <ctype.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESOURCE_H
#include "assetpluginUNREAL_CUnrealPackageSource.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESOURCE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESOURCE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHREADER_H
#include "assetpluginUNREAL_CUnrealSkeletalMeshReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHCONVERTER_H
#include "assetpluginUNREAL_CUnrealSkeletalMeshConverter.h"
#include "assetpluginUNREAL_CUnrealTextureConverter.h"
#include "assetpluginUNREAL_CUnrealStaticMeshReader.h"
#include "assetpluginUNREAL_CUnrealStaticMeshConverter.h"
#include "assetpluginUNREAL_COodleToolProvider.h"
#include "assetpluginUNREAL_CUnrealCapabilities.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHCONVERTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHCONVERTER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H
#include "assetpluginUNREAL_CUnrealCompressedBufferHeader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETONREADER_H
#include "assetpluginUNREAL_CUnrealSkeletonReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETONREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETONREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H
#include "assetpluginUNREAL_CUnrealObjectPathResolver.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALIMPORTER_H
#include "assetpluginUNREAL_CUnrealMaterialImporter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALIMPORTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALIMPORTER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCHARACTERASSEMBLER_H
#include "assetpluginUNREAL_CUnrealCharacterAssembler.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCHARACTERASSEMBLER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCHARACTERASSEMBLER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H
#include "assetpluginUNREAL_CUnrealMetaHumanCharacter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANTEXTURES_H
#include "assetpluginUNREAL_CUnrealMetaHumanTextures.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANTEXTURES_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANTEXTURES_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALWARDROBEITEM_H
#include "assetpluginUNREAL_CUnrealWardrobeItem.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALWARDROBEITEM_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALWARDROBEITEM_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALHIDDENFACEMAP_H
#include "assetpluginUNREAL_CUnrealHiddenFaceMap.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALHIDDENFACEMAP_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALHIDDENFACEMAP_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPRESENTATIONHINTS_H
#include "assetpluginUNREAL_CUnrealPresentationHints.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPRESENTATIONHINTS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPRESENTATIONHINTS_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H
#include "assetpluginUNREAL_CUnrealImporter.h"
#include "assetpluginUNREAL_CUnrealPackRecogniser.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H ? */

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* const CUnrealImporter::PackageIndexName = "unreal.package";

/*-------------------------------------------------------------------------*/

CUnrealImporter::CUnrealImporter( void )
    : ASSETCONVERTER::CIAssetImporter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnrealImporter::~CUnrealImporter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealImporter::GetName( void ) const
{GUCEF_TRACE;

    return "unreal";
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealImporter::GetVersion( void ) const
{GUCEF_TRACE;

    // 2: masks of the vertices, the face material of the UV tile of the head, garments moved into the bind pose of the body
    // 3: texture sources stored as TSCF_UEDELTA are decoded (garment textures, hidden face maps)
    // 4: the hidden face map of each wardrobe item hides the body faces under its outfit (hidden face sets, faces kept)
    // 5: records the normalized relations of the object paths it resolves, engine and plugin content kept unresolved
    // 6: static meshes of UE 4.25 to 4.27, texture sources compressed with zlib in chunks, the editor bulk data of UE 5.0 to 5.2,
    //    skeletal meshes with imported mesh infos; packages that hold nothing to convert are skipped with the reason
    return 6;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealImporter::GetCapabilities( void )
{GUCEF_TRACE;

    return CORE::CString( "Reads the package structure of editor packages of engine releases 4.14 to 5.8 (summary, tables, soft object paths, payload table) and their tagged properties in both tag layouts. Recognises .uasset, .umap and .mhpkg. Skeletal meshes with their skeletons and materials are converted, static meshes of 4.25 to 5.8 (their mesh descriptions) with their materials, textures with the source forms PNG, JPEG, raw and the delta form (Oodle payloads when the oodle tool is found, see CUnrealCapabilities; zlib payloads of UE 4 through the zlib codec); a .mhpkg archive becomes one character (the body and its garments on one skeleton, the stored PNG textures of the character asset, presentation hints for a portrait view); the raw meshes of releases before 4.25, animation, levels and other classes are not yet supported: packages that hold only those are skipped with the reason" ) + ". " + CUnrealCapabilities::GetOodleStatus();}

/*-------------------------------------------------------------------------*/

bool
CUnrealImporter::CanImport( const CORE::CString& vfsPath, ASSETCONVERTER::CAssetPackContext& /* context */ ) const
{GUCEF_TRACE;

    const CORE::CString extension = ASSETCONVERTER::CVfsPaths::ExtensionOf( vfsPath ).Lowercase();
    if ( "mhpkg" == extension )
        return true;
    if ( "uasset" != extension && "umap" != extension )
        return false;

    // the tag at the start of the file
    CUnrealPackageSource source;
    CUnrealArchiveReader reader;
    if ( !source.OpenVfs( vfsPath ) || source.GetSize() < 4 || !source.OpenRange( 0, 4, reader, 4096 ) )
        return false;
    return UE_PACKAGE_FILE_TAG == reader.ReadUInt32() && !reader.HasError();
}

void
CUnrealImporter::GetFileKinds( ASSETCONVERTER::TAssetFileKindVector& kinds ) const
{GUCEF_TRACE;

    CUnrealPackRecogniser::GetFileKinds( kinds );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealImporter::RecognisePack( const ASSETCONVERTER::CAssetPackListing& listing, ASSETCONVERTER::CAssetPackProfile& profile ) const
{GUCEF_TRACE;

    return CUnrealPackRecogniser::Recognise( listing, profile );
}

/*-------------------------------------------------------------------------*/

/** The payloads of the trailer of a package that are compressed with Oodle (method 3 of the compressed buffer header) */
static UInt32
CountOodlePayloads( CUnrealPackage& package )
{GUCEF_TRACE;

    UInt32 count = 0;
    if ( !package.HasTrailer() )
        return 0;
    const CUnrealPackageTrailer::TEntryVector& entries = package.GetTrailer().GetEntries();
    for ( size_t e=0; e<entries.size(); ++e )
    {
        CUnrealArchiveReader payload;
        CUnrealCompressedBufferHeader header;
        const UInt64 peek = entries[ e ].compressedSize < 4096 ? entries[ e ].compressedSize : 4096;
        if ( package.GetSource().OpenRange( package.GetTrailer().GetPayloadOffset( entries[ e ] ), peek, payload, 4096 ) && header.Read( payload, entries[ e ].compressedSize ) && 3 == header.method )
            ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

/** A skeletal mesh of an archive of packages that waits for the assembly of the character */
struct SPendingMesh
{
    CUnrealSkeletalMesh mesh;
    CORE::CString packageFile;      /**< the VFS path of the package */
    CORE::CString packageName;      /**< the package name, for example /Game/EditableCharacters/Name/Name/Meshes/SKM_Name_CombinedBody */
};

/** What one import of a package or an archive of packages collects */
struct SImportState
{
    ASSETCONVERTER::CAssetScene* scene;
    ASSETCONVERTER::CConversionReport* report;
    const CUnrealObjectPathResolver* resolver;
    CORE::CString::StringSet reportedClasses;
    UInt32 meshCount;
    bool sceneFactsSet;

    // the import of an archive: the skeletal meshes are collected and assembled into one character
    bool assemble;
    GUCEF::vector< SPendingMesh* > pending;
    CUnrealMetaHumanCharacter* character;
    bool haveCharacter;
    GUCEF::vector< Int32 > textureIndices;
    CORE::CString intermediateFolder;
    CORE::CString assetName;
    UInt32 socketCount;             /**< sockets and virtual bones that the Skeleton assets of the archive declare; counted, not read */
    UInt32 virtualBoneCount;
    CIUnrealOodleProvider* provider;    /**< the oodle provider of this load session, started when the first payload needs it */
    CORE::CString textureFolder;        /**< the VFS folder that receives the image files of the textures, empty without an intermediate folder */
    UInt32 textureCount;
    GUCEF::vector< CUnrealWardrobeItem > wardrobeItems;     /**< the wardrobe items of the archive, for the hidden face maps of their outfits */

    SImportState( void ) : scene( GUCEF_NULL ), report( GUCEF_NULL ), resolver( GUCEF_NULL ), meshCount( 0 ), sceneFactsSet( false ), assemble( false ), character( GUCEF_NULL ), haveCharacter( false ), socketCount( 0 ), virtualBoneCount( 0 ), provider( GUCEF_NULL ), textureCount( 0 ), wardrobeItems() {}
    ~SImportState() { for ( size_t i=0; i<pending.size(); ++i ) delete pending[ i ]; delete character; }
};

/*-------------------------------------------------------------------------*/

/** Reads the packages of one file: skeletal meshes are converted, the other asset kinds are reported */
static void
ImportPackageFile( const CORE::CString& vfsPath, SImportState& state )
{GUCEF_TRACE;

    ASSETCONVERTER::CConversionReport& report = *state.report;
    ASSETCONVERTER::CAssetScene& scene = *state.scene;
    CUnrealPackage package;
    if ( !package.Open( vfsPath ) )
    {
        report.AddError( "unreal", vfsPath, package.GetError() );
        return;
    }
    const CUnrealPackageSummary& summary = package.GetSummary();
    report.AddInfo( "unreal", vfsPath, "Package " + summary.packageName + ", saved by " + summary.savedByEngineVersion.ToString() + ", UE4 file version " +
        CORE::Int32ToString( summary.versions.GetUe4Version() ) + ", UE5 file version " + CORE::Int32ToString( summary.versions.GetUe5Version() ) + ", " +
        CORE::UInt32ToString( package.GetNames().GetCount() ) + " names, " + CORE::UInt64ToString( package.GetImports().size() ) + " imports, " + CORE::UInt64ToString( package.GetExports().size() ) + " exports" );
    for ( size_t i=0; i<package.GetProblems().size(); ++i )
        report.AddWarning( "unreal", vfsPath, package.GetProblems()[ i ] );

    if ( !state.sceneFactsSet )
    {
        state.sceneFactsSet = true;
        scene.coordinateSystem = CUnrealSkeletalMeshConverter::GetCoordinateSystem();
        scene.normalMapConvention = ASSETCONVERTER::NORMALMAPCONVENTION_DIRECTX;
        scene.provenance.sourceFormat = "unreal";
        scene.provenance.sourceEngine = summary.savedByEngineVersion.ToString();
        scene.provenance.sourceFormatVersion = "UE4 " + CORE::Int32ToString( summary.versions.GetUe4Version() ) + ", UE5 " + CORE::Int32ToString( summary.versions.GetUe5Version() );
    }

    // every class of a top level export is an asset kind
    const CUnrealPackage::TExportVector& exports = package.GetExports();
    UInt32 topLevelAssets = 0;
    for ( size_t i=0; i<exports.size(); ++i )
        if ( exports[ i ].outerIndex.IsNull() && "MetaData" != package.GetClassName( exports[ i ] ) )
            ++topLevelAssets;
    if ( 0 == topLevelAssets && !state.assemble )
    {
        // the actors of a level that the editor keeps in packages of their own: their outer is the level in another package
        CORE::CString::StringSet classes;
        for ( size_t i=0; i<exports.size(); ++i )
            if ( "MetaData" != package.GetClassName( exports[ i ] ) )
                classes.insert( package.GetClassName( exports[ i ] ) );
        CORE::CString classList;
        for ( CORE::CString::StringSet::const_iterator c=classes.begin(); c!=classes.end() && classList.Length() < 200; ++c )
            classList += ( classList.IsNULLOrEmpty() ? "" : ", " ) + (*c);
        const CORE::CString subject = ( !summary.packageName.IsNULLOrEmpty() && '/' == summary.packageName[ 0 ] ) ? summary.packageName : ASSETCONVERTER::CVfsPaths::StemOf( vfsPath );
        if ( classList.IsNULLOrEmpty() )
            report.AddSkipped( "unreal", subject, "The package holds no asset: only the editor notes of a package (MetaData)" );
        else
            report.AddSkipped( "unreal", subject, "The package holds no asset of its own: its " + CORE::UInt64ToString( exports.size() ) + " exports (" + classList +
                               ") belong to an object of another package, as the actors of a level that the editor stores one package per actor; levels are not yet converted" );
        return;
    }
    for ( size_t i=0; i<exports.size(); ++i )
    {
        if ( !exports[ i ].outerIndex.IsNull() )
            continue;
        const CORE::CString className = package.GetClassName( exports[ i ] );
        if ( "SkeletalMesh" == className )
        {
            CUnrealSkeletalMeshReader reader;
            CUnrealSkeletalMesh mesh;
            if ( !reader.Read( package, (UInt32) i, mesh ) )
            {
                report.AddError( "unreal", exports[ i ].objectName, reader.GetError() );
                continue;
            }
            for ( size_t n=0; n<reader.GetNotes().size(); ++n )
                report.AddInfo( "unreal", exports[ i ].objectName, reader.GetNotes()[ n ] );
            if ( state.assemble )
            {
                SPendingMesh* item = new SPendingMesh;
                item->mesh = mesh;
                item->packageFile = vfsPath;
                item->packageName = summary.packageName;
                state.pending.push_back( item );
                continue;
            }
            GUCEF::vector< Int32 > slotMaterials( mesh.materialSlots.size(), -1 );
            CUnrealMaterialImporter::AddMaterialsOfMesh( package, mesh, *state.resolver, scene, report, slotMaterials );
            UInt32 rootNode = 0;
            if ( CUnrealSkeletalMeshConverter::AddToScene( mesh, slotMaterials, scene, report, rootNode ) )
                ++state.meshCount;
            continue;
        }
        if ( "MetaHumanCharacter" == className && state.assemble && !state.haveCharacter )
        {
            if ( GUCEF_NULL == state.character )
                state.character = new CUnrealMetaHumanCharacter;
            if ( !state.character->Read( package, (UInt32) i ) )
            {
                report.AddWarning( "unreal", exports[ i ].objectName, state.character->GetError() );
                continue;
            }
            state.haveCharacter = true;
            report.AddInfo( "unreal", exports[ i ].objectName, "MetaHuman character: " + CORE::UInt64ToString( state.character->textures.size() ) + " textures, " + CORE::UInt64ToString( state.character->records.size() ) + " payload records, " +
                CORE::UInt32ToString( state.character->payloadsAssigned ) + " assigned to textures, " + CORE::UInt64ToString( state.character->wardrobeItems.size() ) + " wardrobe items" );
            const CORE::CString characterName = state.character->nameOverride.IsNULLOrEmpty() ? state.character->name : state.character->nameOverride;
            CUnrealMetaHumanTextures::AddToScene( package, *state.character, state.intermediateFolder, state.assetName, characterName, scene, report, state.textureIndices, state.provider );
            continue;
        }
        if ( "MetaHumanWardrobeItem" == className && state.assemble )
        {
            CUnrealWardrobeItem item;
            if ( !item.Read( package, (UInt32) i ) )
            {
                report.AddWarning( "unreal", exports[ i ].objectName, item.GetError() );
                continue;
            }
            report.AddInfo( "unreal", item.name, "Wardrobe item: puts on " + ( item.principalAsset.IsNULLOrEmpty() ? CORE::CString( "nothing that can be read" ) : item.principalAsset ) +
                            ( item.bodyHiddenFaceMap.IsNULLOrEmpty() ? CORE::CString( ", no hidden face map of the body" ) : ", hidden face map of the body " + item.bodyHiddenFaceMap ) );
            state.wardrobeItems.push_back( item );
            continue;
        }
        if ( "Skeleton" == className )
        {
            CUnrealSkeletonReader skeletonReader;
            CUnrealReferenceSkeleton skeleton;
            if ( skeletonReader.Read( package, (UInt32) i, skeleton ) )
            {
                state.socketCount += skeletonReader.GetSocketCount();
                state.virtualBoneCount += skeletonReader.GetVirtualBoneCount();
                UInt32 facial = 0;
                for ( size_t b=0; b<skeleton.GetBones().size(); ++b )
                    if ( skeleton.GetBones()[ b ].name.HasSubstr( "FACIAL_" ) == 0 )
                        ++facial;
                report.AddInfo( "unreal", exports[ i ].objectName, "Skeleton with " + CORE::UInt64ToString( skeleton.GetBones().size() ) + " bones, " + CORE::UInt32ToString( facial ) + " of them facial joints, " + CORE::UInt32ToString( skeletonReader.GetSocketCount() ) +
                    " sockets and " + CORE::UInt32ToString( skeletonReader.GetVirtualBoneCount() ) + " virtual bones; the skinning of a mesh uses the reference skeleton of the mesh" );
            }
            else
                report.AddWarning( "unreal", exports[ i ].objectName, skeletonReader.GetError() );
            if ( !state.assemble )
                report.AddSkipped( "unreal", exports[ i ].objectName, "Class Skeleton: a skeleton is converted with the skeletal meshes that use it" );
            continue;
        }
        if ( "MetaData" == className )
            continue;       // the editor notes of the package, not an asset
        if ( "MaterialInstanceConstant" == className || "Material" == className )
        {
            // read with the meshes that use them; a package that holds only a material converts nothing of its own
            if ( !state.assemble && state.reportedClasses.insert( className ).second )
                report.AddSkipped( "unreal", exports[ i ].objectName, "Class " + className + ": a material is converted with the meshes that use it" );
            continue;
        }
        if ( "StaticMesh" == className && !state.assemble )
        {
            CUnrealStaticMeshReader reader;
            CUnrealStaticMesh mesh;
            if ( !reader.Read( package, (UInt32) i, state.provider, &report, mesh ) )
            {
                if ( reader.IsProviderMissing() || reader.IsWithoutGeometry() )
                    report.AddSkipped( "unreal", exports[ i ].objectName, "Class StaticMesh: " + reader.GetError() );
                else
                    report.AddError( "unreal", exports[ i ].objectName, reader.GetError() );
                continue;
            }
            for ( size_t n=0; n<reader.GetNotes().size(); ++n )
                report.AddInfo( "unreal", exports[ i ].objectName, reader.GetNotes()[ n ] );
            if ( 0 == state.meshCount )
                scene.coordinateSystem = CUnrealStaticMeshConverter::GetCoordinateSystem();
            GUCEF::vector< Int32 > slotMaterials;
            CUnrealMaterialImporter::AddMaterialsOfSlots( package, mesh.materialSlots, *state.resolver, scene, report, slotMaterials );
            UInt32 rootNode = 0;
            if ( CUnrealStaticMeshConverter::AddToScene( mesh, slotMaterials, scene, report, rootNode ) )
                ++state.meshCount;
            continue;
        }
        if ( "Texture2D" == className && !state.assemble )
        {
            // a package of a project is the texture itself (the textures of a character archive are read when the character asks for them)
            Int32 textureIndex = -1;
            if ( CUnrealTextureConverter::AddToScene( package, (Int32) i, state.provider, state.textureFolder, scene, report, textureIndex, true ) )
                ++state.textureCount;
            continue;
        }
        if ( !state.reportedClasses.insert( className ).second )
            continue;
        const UInt32 oodlePayloads = ( "Texture2D" == className || "StaticMesh" == className || "TextureCube" == className ) ? CountOodlePayloads( package ) : 0;
        if ( oodlePayloads > 0 && !state.provider->IsAvailable() )
            report.AddSkipped( "unreal", exports[ i ].objectName, "Class " + className + ": " + CUnrealCapabilities::GetNeedsOodleText() + ": its data is in " + CORE::UInt32ToString( oodlePayloads ) + " payloads of the package trailer that are compressed with Oodle" );
        else if ( oodlePayloads > 0 && state.assemble )
            report.AddSkipped( "unreal", exports[ i ].objectName, "Class " + className + ": not part of the assembly of the character: its textures and meshes (the conformal head and body, their textures) belong to the later steps of the MetaHuman import" );
        else
            report.AddSkipped( "unreal", exports[ i ].objectName, "Class " + className + ": the asset kind is not yet supported by the Unreal importer; the properties can be read" );
    }
}

/*-------------------------------------------------------------------------*/

/** How much a skeletal mesh looks like the body of a character by its name */
static UInt32
BodyScore( const CORE::CString& name )
{GUCEF_TRACE;

    if ( name.HasSubstr( "CombinedBody" ) >= 0 )
        return 3;
    if ( name.HasSubstr( "FullBody" ) >= 0 )
        return 2;
    if ( name.HasSubstr( "PreviewBody" ) >= 0 )
        return 1;
    return 0;
}

/*-------------------------------------------------------------------------*/

/**
 *  Whether the package belongs to the character: it is below the folder of the character asset and not in the folder of another
 *  character that the package carries along (the presets of its library are nested in a folder "EditableCharacters")
 */
static bool
IsContentOfCharacter( const CORE::CString& packageName, const CORE::CString& characterPackage )
{GUCEF_TRACE;

    const CORE::CString prefix = characterPackage + "/";
    if ( characterPackage.IsNULLOrEmpty() )
        return true;
    if ( 0 != packageName.HasSubstr( prefix ) )
        return false;
    return packageName.SubstrFromRange( prefix.Length(), packageName.Length() ).HasSubstr( "EditableCharacters/" ) < 0;
}

/*-------------------------------------------------------------------------*/

/** The part of a wardrobe item name that names its outfit: WI_Outfit gives Outfit */
static CORE::CString
OutfitStemOf( const CORE::CString& itemName )
{GUCEF_TRACE;

    return 0 == itemName.HasSubstr( "WI_" ) ? itemName.SubstrFromRange( 3, itemName.Length() ) : itemName;
}

/*-------------------------------------------------------------------------*/

/** Reads the hidden face map of a wardrobe item from its texture package (the archive is mounted); false with the reason in 'error' */
static bool
ReadHiddenFaceMap( const CUnrealWardrobeItem& item, SImportState& state, CUnrealHiddenFaceMap& map, CORE::CString& file, CORE::CString& error )
{GUCEF_TRACE;

    CORE::CString vfsPath;
    if ( !state.resolver->ResolveReference( item.bodyHiddenFaceMap, ASSETCONVERTER::CAssetReference::KindUsesTexture, vfsPath ) || !VFS::CVfsGlobal::Instance()->GetVfs().FileExists( vfsPath ) )
    {
        error = "the texture is not in the pack";
        return false;
    }
    CUnrealPackage package;
    if ( !package.Open( vfsPath ) )
    {
        error = "the texture package cannot be read: " + package.GetError();
        return false;
    }
    Int32 textureExport = -1;
    for ( size_t e=0; e<package.GetExports().size() && textureExport < 0; ++e )
        if ( package.GetExports()[ e ].outerIndex.IsNull() && "Texture2D" == package.GetClassName( package.GetExports()[ e ] ) )
            textureExport = (Int32) e;
    if ( textureExport < 0 )
    {
        error = "the package holds no Texture2D";
        return false;
    }
    CUnrealTextureConverter::SResult result;
    CUnrealHiddenFaceMap::TByteVector pixels;
    if ( !CUnrealTextureConverter::ConvertWithPixels( package, (UInt32) textureExport, state.provider, state.report, state.textureFolder, result, pixels ) )
    {
        error = result.error;
        return false;
    }
    if ( pixels.empty() )
    {
        error = "the source is a " + result.sourceFormat + " file; only raw sources are sampled";
        return false;
    }
    if ( !map.SetImage( pixels, result.width, result.height, result.channels ) )
    {
        error = "the pixels do not match the sizes of the texture";
        return false;
    }
    file = result.file;
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  The engine removes the body faces that the hidden face map of a wardrobe item marks when it assembles the character with that
 *  outfit. Here they become a hidden face set of the scene: the triangles stay in the body, in submeshes of their own that the set
 *  hides while the outfit is worn. The outfits of the assembly are worn: their sets are active.
 */
static void
ApplyHiddenFaceMaps( const CUnrealCharacterAssembler::SResult& assembly, const CORE::CString& characterPackage, SImportState& state )
{GUCEF_TRACE;

    ASSETCONVERTER::CConversionReport& report = *state.report;
    ASSETCONVERTER::CAssetScene& scene = *state.scene;
    if ( state.wardrobeItems.empty() || assembly.bodyHolderNode >= scene.nodes.size() )
        return;

    // the meshes of the body (its levels of detail); the materials of the head tile are left alone, and when the head tile has been
    // split off the UVs of the body tile are those of its textures (tile column 0), otherwise the body is UV tile 1002 (column 1)
    GUCEF::vector< UInt32 > bodyMeshes;
    for ( size_t n=0; n<scene.nodes.size(); ++n )
        if ( scene.nodes[ n ].mesh >= 0 && (size_t) scene.nodes[ n ].mesh < scene.meshes.size() && scene.IsInSubtree( (UInt32) n, assembly.bodyHolderNode ) &&
             std::find( bodyMeshes.begin(), bodyMeshes.end(), (UInt32) scene.nodes[ n ].mesh ) == bodyMeshes.end() )
            bodyMeshes.push_back( (UInt32) scene.nodes[ n ].mesh );
    CUnrealHiddenFaceMap::TInt32Set leaveAlone;
    for ( size_t m=0; m<bodyMeshes.size(); ++m )
        for ( size_t s=0; s<scene.meshes[ bodyMeshes[ m ] ].subMeshes.size(); ++s )
        {
            const Int32 material = scene.meshes[ bodyMeshes[ m ] ].subMeshes[ s ].materialIndex;
            if ( material >= 0 && (size_t) material < scene.materials.size() && scene.materials[ (size_t) material ].name.HasSubstr( "_UDIM1001" ) >= 0 )
                leaveAlone.insert( material );
        }
    const Int32 tile = leaveAlone.empty() ? 1 : 0;

    for ( size_t w=0; w<state.wardrobeItems.size(); ++w )
    {
        const CUnrealWardrobeItem& item = state.wardrobeItems[ w ];
        // the items of the character only, not those of the characters of its library that the archive carries along
        if ( item.bodyHiddenFaceMap.IsNULLOrEmpty() || !IsContentOfCharacter( item.packageName, characterPackage ) )
            continue;
        if ( scene.hiddenFaceSets.size() >= ASSETCONVERTER::CAssetHiddenFaceSet::MaxSets )
        {
            report.AddSkipped( "unreal", item.name, "The hidden face map is not applied: the scene has " + CORE::UInt32ToString( ASSETCONVERTER::CAssetHiddenFaceSet::MaxSets ) + " hidden face sets already" );
            continue;
        }
        CUnrealHiddenFaceMap map;
        if ( item.hasMaxCullValue )
            map.SetMaxCullValue( item.maxCullValue );
        if ( item.hasMinKeepValue )
            map.SetMinKeepValue( item.minKeepValue );
        CORE::CString file;
        CORE::CString error;
        if ( !ReadHiddenFaceMap( item, state, map, file, error ) )
        {
            report.AddWarning( "unreal", item.name, "The hidden face map " + item.bodyHiddenFaceMap + " cannot be used, the body shows through the outfit where it intersects: " + error );
            continue;
        }

        // the garments of the outfit: those named after it; when none is, the outfit is everything the assembly put on
        ASSETCONVERTER::CAssetHiddenFaceSet set;
        set.name = item.name;
        set.active = true;
        const CORE::CString stem = OutfitStemOf( item.name );
        for ( size_t g=0; g<assembly.garmentHolderNodes.size(); ++g )
            if ( assembly.garmentHolderNodes[ g ] < scene.nodes.size() && scene.nodes[ assembly.garmentHolderNodes[ g ] ].name.HasSubstr( stem ) >= 0 )
                set.garments.push_back( scene.nodes[ assembly.garmentHolderNodes[ g ] ].name );
        if ( set.garments.empty() )
            for ( size_t g=0; g<assembly.garmentHolderNodes.size(); ++g )
                if ( assembly.garmentHolderNodes[ g ] < scene.nodes.size() )
                    set.garments.push_back( scene.nodes[ assembly.garmentHolderNodes[ g ] ].name );

        const UInt32 bit = ASSETCONVERTER::CAssetHiddenFaceSet::BitOf( (UInt32) scene.hiddenFaceSets.size() );
        CORE::CString levels;
        UInt32 bandVertices = 0;
        for ( size_t m=0; m<bodyMeshes.size(); ++m )
        {
            CUnrealHiddenFaceMap::SResult result;
            map.HideFaces( scene.meshes[ bodyMeshes[ m ] ], bit, tile, leaveAlone, result );
            set.hiddenTriangles += result.hiddenTriangles;
            if ( 0 == m )
                bandVertices = result.bandVertices;
            levels += ( 0 == m ? "" : ", " ) + scene.meshes[ bodyMeshes[ m ] ].name + " " + CORE::UInt32ToString( result.hiddenTriangles ) + " of " + CORE::UInt32ToString( result.triangles );
        }
        Float32 hiddenShare = 0.0f, bandShare = 0.0f;
        map.GetCoverage( hiddenShare, bandShare );
        set.source = ASSETCONVERTER::CVfsPaths::NameOf( item.bodyHiddenFaceMap ) + ": triangles of UV tile 1002 of the body whose texels all lie below " + CORE::FloatToString( map.GetMaxCullValue() );
        scene.hiddenFaceSets.push_back( set );
        CORE::CString garmentList;
        for ( size_t g=0; g<set.garments.size(); ++g )
            garmentList += ( 0 == g ? "" : ", " ) + set.garments[ g ];
        report.AddInfo( "unreal", item.name, "Hidden face set of the outfit (" + garmentList + "), active: the hidden face map " + ASSETCONVERTER::CVfsPaths::NameOf( item.bodyHiddenFaceMap ) + " (" +
                        CORE::UInt32ToString( map.GetWidth() ) + " x " + CORE::UInt32ToString( map.GetHeight() ) + ", " + CORE::FloatToString( hiddenShare * 100.0f ) + " percent of the texels below " +
                        CORE::FloatToString( map.GetMaxCullValue() ) + ") hides " + levels + " triangles of the body levels in the tile of the map; the triangles stay in the body as submeshes of the set" );
        if ( bandVertices > 0 )
            report.AddApproximation( "unreal", item.name, CORE::UInt32ToString( bandVertices ) + " vertices of the first level of the body lie where the hidden face map is between " + CORE::FloatToString( map.GetMaxCullValue() ) + " and " +
                                     CORE::FloatToString( map.GetMinKeepValue() ) + " (" + CORE::FloatToString( bandShare * 100.0f ) + " percent of the texels): the engine moves them inwards along their normals, the converter keeps them in place" );
        if ( !item.hasMaxCullValue || !item.hasMinKeepValue )
            report.AddApproximation( "unreal", item.name, "the wardrobe item stores no cull and keep values for its hidden face map: the values " + CORE::FloatToString( map.GetMaxCullValue() ) + " and " + CORE::FloatToString( map.GetMinKeepValue() ) +
                                     " are assumed" );
    }
}

/*-------------------------------------------------------------------------*/

/** Makes one scene of the character from the collected skeletal meshes: the body, the garments on its skeleton, materials, textures and the presentation hints */
static void
AssembleCharacter( const CORE::CString& archiveName, SImportState& state )
{GUCEF_TRACE;

    ASSETCONVERTER::CConversionReport& report = *state.report;
    ASSETCONVERTER::CAssetScene& scene = *state.scene;
    if ( state.pending.empty() )
        return;
    const CUnrealMetaHumanCharacter* character = state.haveCharacter ? state.character : GUCEF_NULL;
    const CORE::CString characterName = GUCEF_NULL != character && !character->nameOverride.IsNULLOrEmpty() ? character->nameOverride : ( GUCEF_NULL != character ? character->name : archiveName );
    const CORE::CString characterPackage = GUCEF_NULL != character ? character->packageName : CORE::CString::Empty;

    // the body: the best name, then the most bones and vertices, among the content of the character
    Int32 body = -1;
    for ( size_t i=0; i<state.pending.size(); ++i )
    {
        const SPendingMesh& item = *state.pending[ i ];
        if ( !IsContentOfCharacter( item.packageName, characterPackage ) || item.mesh.lods.empty() || item.mesh.skeleton.GetBones().empty() )
            continue;
        if ( body < 0 )
        {
            body = (Int32) i;
            continue;
        }
        const SPendingMesh& best = *state.pending[ (size_t) body ];
        const UInt32 score = BodyScore( item.mesh.name );
        const UInt32 bestScore = BodyScore( best.mesh.name );
        if ( score > bestScore || ( score == bestScore && ( item.mesh.skeleton.GetBones().size() > best.mesh.skeleton.GetBones().size() ||
            ( item.mesh.skeleton.GetBones().size() == best.mesh.skeleton.GetBones().size() && 0 == bestScore && item.mesh.GetVertexCount( 0 ) > best.mesh.GetVertexCount( 0 ) ) ) ) )
            body = (Int32) i;
    }
    if ( body < 0 )
    {
        report.AddError( "unreal", characterName, "The archive has no skeletal mesh that can carry a character" );
        return;
    }

    // the garments: the other content of the character that fits the skeleton of the body; the rest is left out with the reason
    GUCEF::vector< size_t > parts;
    parts.push_back( (size_t) body );
    for ( size_t i=0; i<state.pending.size(); ++i )
    {
        if ( (Int32) i == body )
            continue;
        const SPendingMesh& item = *state.pending[ i ];
        if ( !IsContentOfCharacter( item.packageName, characterPackage ) )
            report.AddSkipped( "unreal", item.mesh.name, "Not part of the character " + characterName + ": the package " + item.packageName + " is content of another character or of the engine" );
        else if ( BodyScore( item.mesh.name ) > 0 )
            report.AddSkipped( "unreal", item.mesh.name, "Not part of the character " + characterName + ": another body mesh, " + state.pending[ (size_t) body ]->mesh.name + " is the body" );
        else if ( !CUnrealCharacterAssembler::IsCompatible( state.pending[ (size_t) body ]->mesh.skeleton, item.mesh.skeleton ) )
            report.AddSkipped( "unreal", item.mesh.name, "Not skinned onto the body: its skeleton of " + CORE::UInt64ToString( item.mesh.skeleton.GetBones().size() ) + " bones does not fit the skeleton of " + state.pending[ (size_t) body ]->mesh.name );
        else if ( item.mesh.lods.empty() )
            report.AddSkipped( "unreal", item.mesh.name, "The garment has no LOD models" );
        else
            parts.push_back( i );
    }

    // the materials of the parts, each package opened again for its material references
    GUCEF::vector< GUCEF::vector< Int32 > > slotMaterials( parts.size() );
    for ( size_t p=0; p<parts.size(); ++p )
    {
        const SPendingMesh& item = *state.pending[ parts[ p ] ];
        slotMaterials[ p ].assign( item.mesh.materialSlots.size(), -1 );
        CUnrealPackage package;
        if ( !package.Open( item.packageFile ) )
        {
            report.AddWarning( "unreal", item.mesh.name, "The package cannot be opened again for its materials: " + package.GetError() );
            continue;
        }
        CUnrealMaterialImporter::AddMaterialsOfMesh( package, item.mesh, *state.resolver, scene, report, slotMaterials[ p ] );
    }

    CUnrealCharacterAssembler::TPartVector garments;
    for ( size_t p=1; p<parts.size(); ++p )
        garments.push_back( CUnrealCharacterAssembler::SPart( &state.pending[ parts[ p ] ]->mesh, &slotMaterials[ p ] ) );
    CUnrealCharacterAssembler::SResult assembly;
    if ( !CUnrealCharacterAssembler::Assemble( characterName, CUnrealCharacterAssembler::SPart( &state.pending[ parts[ 0 ] ]->mesh, &slotMaterials[ 0 ] ), garments, scene, report, assembly ) )
        return;
    state.meshCount = (UInt32) parts.size();
    scene.name = characterName;

    // the textures of the character asset on the materials, and the extras for the viewer
    CORE::CString::StringVector unassigned;
    if ( GUCEF_NULL != character )
    {
        CUnrealMetaHumanTextures::AttachToMaterials( *character, state.textureIndices, scene, report, unassigned );
        CORE::CDataNode record( CUnrealPresentationHints::CharacterKey );
        CUnrealMetaHumanTextures::DescribeCharacter( *character, characterName, state.textureIndices, scene, record );
        if ( !unassigned.empty() )
        {
            CORE::CDataNode* list = record.AddChild( "unassignedTextures" );
            for ( size_t u=0; u<unassigned.size(); ++u )
                list->SetAttribute( CORE::UInt32ToString( (UInt32) u ), unassigned[ u ] );
        }
        record.SetAttribute( "skeletonSockets", state.socketCount );
        record.SetAttribute( "skeletonVirtualBones", state.virtualBoneCount );
        scene.extras.AddChild( record );
    }

    // the body faces under the outfits: after the textures, when the UVs of the body tile are final
    ApplyHiddenFaceMaps( assembly, characterPackage, state );
    CUnrealPresentationHints::SHints hints;
    if ( CUnrealPresentationHints::Build( scene, assembly, character, characterName, report, hints ) )
    {
        CORE::CDataNode presentation( CUnrealPresentationHints::ExtrasKey );
        CUnrealPresentationHints::ToDataNode( hints, character, characterName, presentation );
        scene.extras.AddChild( presentation );
        report.AddInfo( "unreal", characterName, "Presentation: portrait of the head" + CORE::CString( hints.headEstimated ? " (head bounds estimated)" : "" ) + ", the character is " + CORE::FloatToString( hints.boundsMax.v[ 1 ] - hints.boundsMin.v[ 1 ] ) + " m tall" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CUnrealImporter::Import( const CORE::CString& vfsPath, ASSETCONVERTER::CAssetPackContext& context, ASSETCONVERTER::CAssetScene& out, ASSETCONVERTER::CConversionReport& report )
{GUCEF_TRACE;

    out.provenance.importerName = GetName();
    out.provenance.importerVersion = GetVersion();
    out.name = CORE::ExtractFilename( vfsPath );

    CUnrealObjectPathResolver resolver;
    resolver.SetContext( &context );
    resolver.SetRecorder( &out );
    SImportState state;
    state.scene = &out;
    state.report = &report;
    state.resolver = &resolver;
    state.meshCount = 0;
    state.sceneFactsSet = false;

    // the oodle provider of this load: the tool is started when the first payload needs it and ends with the load
    COodleToolProvider oodle;
    oodle.SetCancellationSource( &report );
    state.provider = &oodle;
    CUnrealCapabilities::NoteOodleStatusOnce();
    state.assetName = ASSETCONVERTER::CVfsPaths::StemOf( vfsPath );
    if ( !context.GetManifest().GetIntermediatePath().IsNULLOrEmpty() )
        state.textureFolder = ASSETCONVERTER::CVfsPaths::Join( context.GetManifest().GetIntermediatePath(), "embedded/" + state.assetName );

    const CORE::CString extension = ASSETCONVERTER::CVfsPaths::ExtensionOf( vfsPath ).Lowercase();
    if ( "mhpkg" == extension )
    {
        // the archive is mounted like any other, its packages are then read one by one
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        CORE::CString mountName = "unreal_pack_";
        for ( UInt32 c=0; c<vfsPath.Length(); ++c )
            mountName += ( isalnum( (unsigned char) vfsPath[ c ] ) ? vfsPath[ c ] : '_' );
        CORE::CString archiveType;
        if ( !vfs.TryGetArchiveTypeForFile( vfsPath, archiveType ) || archiveType.IsNULLOrEmpty() || !vfs.MountArchive( mountName, vfsPath, archiveType, mountName, false, false ) )
        {
            report.AddError( "unreal", vfsPath, "The archive cannot be mounted: no archive plugin handles .mhpkg files" );
            return false;
        }
        resolver.AddPackagesOf( mountName );
        state.assemble = true;
        state.intermediateFolder = context.GetManifest().GetIntermediatePath();
        state.assetName = ASSETCONVERTER::CVfsPaths::StemOf( vfsPath );
        VFS::CVFS::TStringVector files;
        vfs.GetFileList( files, mountName, true, true, CORE::CString::EmptyStringSet, 1000000 );
        std::sort( files.begin(), files.end() );
        for ( size_t i=0; i<files.size(); ++i )
        {
            const CORE::CString fileExtension = ASSETCONVERTER::CVfsPaths::ExtensionOf( files[ i ] ).Lowercase();
            if ( "uasset" == fileExtension || "umap" == fileExtension )
                ImportPackageFile( files[ i ], state );
        }
        if ( state.pending.empty() )
            report.AddSkipped( "unreal", vfsPath, "The archive holds no skeletal mesh of the character: it carries the character definition only" +
                               CORE::CString( state.haveCharacter ? " (" + CORE::UInt64ToString( state.character->textures.size() ) + " textures of its own)" : "" ) +
                               ", whose body and head the MetaHuman tools generate; there is no geometry to convert" );
        AssembleCharacter( state.assetName, state );
        // the textures the materials name are packages of the archive: it stays mounted until they are read
        if ( !out.materials.empty() )
            CUnrealTextureConverter::AttachToMaterials( resolver, state.provider, state.textureFolder, out, report );
        vfs.UnmountArchiveByName( mountName );
    }
    else
    {
        ImportPackageFile( vfsPath, state );
        // the textures the materials name, now that all their materials exist
        if ( !out.materials.empty() )
            CUnrealTextureConverter::AttachToMaterials( resolver, state.provider, state.textureFolder, out, report );
    }
    oodle.Stop();
    if ( state.meshCount > 0 || state.textureCount > 0 )
        return true;

    // nothing converted: when every part of the package was left out for a reason that is in the report (kinds that are not
    // converted, sources the reader does not support) and nothing failed, the package is skipped, not failed
    ASSETCONVERTER::CConversionReportEntry* entry = report.GetCurrentEntry();
    if ( GUCEF_NULL != entry && !entry->HasErrors() && entry->GetItemCount( ASSETCONVERTER::REPORTITEM_SKIPPED ) > 0 )
        report.SetCurrentAssetSkipped();
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealImporter::IndexPack( ASSETCONVERTER::CAssetPackContext& context )
{GUCEF_TRACE;

    const CORE::CString::StringVector& files = context.GetFiles();
    UInt32 indexed = 0;
    for ( size_t i=0; i<files.size(); ++i )
    {
        const CORE::CString extension = ASSETCONVERTER::CVfsPaths::ExtensionOf( files[ i ] ).Lowercase();
        if ( "uasset" != extension && "umap" != extension )
            continue;
        CUnrealPackage package;
        if ( !package.Open( context.ToVfsPath( files[ i ] ) ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "assetpluginUNREAL: The package " << files[ i ] << " is not indexed: " << package.GetError() );
            continue;
        }
        const CORE::CString packageName = CUnrealObjectPathResolver::PackageNameOfFile( package.GetSummary().packageName, files[ i ] );
        if ( !packageName.IsNULLOrEmpty() )
        {
            context.SetIndexEntry( PackageIndexName, packageName, files[ i ] );
            ++indexed;
        }
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "assetpluginUNREAL: Indexed " << indexed << " packages" );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
