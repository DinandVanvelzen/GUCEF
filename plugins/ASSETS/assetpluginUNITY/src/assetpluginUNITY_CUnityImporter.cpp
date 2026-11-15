/*
 *  assetpluginUNITY: the importer of Unity projects and packages
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CGLOBPATTERN_H
#include "gucefCORE_CGlobPattern.h"
#define GUCEF_CORE_CGLOBPATTERN_H
#endif /* GUCEF_CORE_CGLOBPATTERN_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#include "AssetConverterLib_CAssetPackContext.h"
#define ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#endif /* ASSETCONVERTERLIB_CASSETPACKCONTEXT_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYPROJECT_H
#include "assetpluginUNITY_CUnityProject.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYPROJECT_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYPROJECT_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYMODEL_H
#include "assetpluginUNITY_CUnityModel.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYMODEL_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYMODEL_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYPREFABREADER_H
#include "assetpluginUNITY_CUnityPrefabReader.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYPREFABREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYPREFABREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYANIMATIONCLIPREADER_H
#include "assetpluginUNITY_CUnityAnimationClipReader.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYANIMATIONCLIPREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYANIMATIONCLIPREADER_H ? */

#include "assetpluginUNITY_CUnityImporter.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const MaterialNameIndex = "unity.materialName";

/** A boolean of the pack rules "unity" (for CanImport(), which has no effective settings) */
bool
GetPackRule( const ASSETCONVERTER::CAssetPackContext& context, const char* name, bool defaultValue )
{
    const CORE::CDataNode* rules = context.GetManifest().rules.FindChild( CUnityProject::SettingsName );
    if ( GUCEF_NULL == rules )
        return defaultValue;
    const CORE::CVariant& value = rules->GetAttributeValueOrChildValueByName( name );
    return value.IsNULLOrEmpty() ? defaultValue : value.AsBool( defaultValue, true );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnityImporter::CUnityImporter( void )
    : ASSETCONVERTER::CIAssetImporter()
    , m_lock()
    , m_overlays()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnityImporter::~CUnityImporter()
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    VFS::CVfsGlobal* vfs = VFS::CVfsGlobal::Instance();
    for ( TMountMap::const_iterator p = m_overlays.begin(); p != m_overlays.end(); ++p )
        for ( size_t m=0; m<(*p).second.size(); ++m )
            if ( GUCEF_NULL != vfs )
                vfs->GetVfs().UnmountArchiveByName( (*p).second[ m ] );
    m_overlays.clear();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnityImporter::GetName( void ) const
{GUCEF_TRACE;

    return "unity";
}

/*-------------------------------------------------------------------------*/

UInt32
CUnityImporter::GetVersion( void ) const
{GUCEF_TRACE;

    return 2;
}

/*-------------------------------------------------------------------------*/

void
CUnityImporter::GetDefaultSettings( CORE::CDataNode& settings ) const
{GUCEF_TRACE;

    CORE::CDataNode* own = settings.FindOrAddChild( CUnityProject::SettingsName, GUCEF_DATATYPE_OBJECT );
    own->SetAttribute( "importScenes", true );
    own->SetAttribute( "importModels", true );
    own->SetAttribute( "skipInactive", false );
}

/*-------------------------------------------------------------------------*/

bool
CUnityImporter::CanImport( const CORE::CString& vfsPath, ASSETCONVERTER::CAssetPackContext& context ) const
{GUCEF_TRACE;

    const CORE::CString extension = ASSETCONVERTER::CVfsPaths::ExtensionOf( vfsPath );
    if ( "prefab" == extension || "anim" == extension )
        return true;
    if ( "unity" == extension )
        return GetPackRule( context, "importScenes", true );
    if ( CUnityModel::IsModelExtension( extension ) )
        return GetPackRule( context, "importModels", true ) && VFS::CVfsGlobal::Instance()->GetVfs().FileExists( vfsPath + ".meta" );
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnityImporter::GetProjectRoot( const CORE::CString& vfsFolder )
{GUCEF_TRACE;

    const CORE::CString folder = ASSETCONVERTER::CVfsPaths::Normalize( vfsFolder );
    if ( "Assets" == folder || folder.StartsWith( "Assets/" ) )
        return CORE::CString::Empty;
    const Int32 inside = folder.HasSubstr( "/Assets/", false );
    if ( inside >= 0 )
        return folder.SubstrFromRange( 0, (UInt32) inside );
    if ( folder.EndsWith( "/Assets" ) )
        return folder.SubstrFromRange( 0, folder.Length() - 7 );
    return folder;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnityImporter::ReadGuid( const CORE::CString& metaText )
{GUCEF_TRACE;

    const Int32 at = metaText.HasSubstr( "guid:", true );
    if ( at < 0 )
        return CORE::CString::Empty;
    CORE::CString rest = metaText.SubstrFromRange( (UInt32) at + 5, metaText.Length() );
    const Int32 end = rest.HasChar( '\n', true );
    if ( end >= 0 )
        rest = rest.SubstrFromRange( 0, (UInt32) end );
    return rest.Trim( true ).Trim( false ).Lowercase();
}

/*-------------------------------------------------------------------------*/

UInt32
CUnityImporter::IndexFolder( ASSETCONVERTER::CAssetPackContext& context, const CORE::CString& vfsFolder )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::CVFS::TStringVector files;
    vfs.GetFileList( files, vfsFolder, true, true, CORE::CString::Empty, GUCEF_UINT32MAX );
    UInt32 indexed = 0;
    for ( size_t f=0; f<files.size(); ++f )
    {
        const CORE::CString path = ASSETCONVERTER::CVfsPaths::Normalize( files[ f ] );
        if ( !path.EndsWith( ".meta" ) )
            continue;
        CORE::CString text;
        if ( !CUnityProject::LoadText( path, text ) )
            continue;
        const CORE::CString guid = ReadGuid( text );
        if ( guid.IsNULLOrEmpty() )
            continue;
        const CORE::CString asset = path.SubstrFromRange( 0, path.Length() - 5 );
        context.SetIndexEntry( CUnityProject::GuidIndexName, guid, asset );
        ++indexed;
        if ( asset.EndsWith( ".mat" ) )
        {
            CORE::CString known;
            const CORE::CString key = ASSETCONVERTER::CVfsPaths::StemOf( asset ).Lowercase();
            if ( !context.TryResolve( MaterialNameIndex, key, known ) )
                context.SetIndexEntry( MaterialNameIndex, key, asset );
        }
    }
    return indexed;
}

/*-------------------------------------------------------------------------*/

void
CUnityImporter::UnmountOverlays( const CORE::CString& packId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TMountMap::iterator i = m_overlays.find( packId );
    if ( i == m_overlays.end() )
        return;
    for ( size_t m=0; m<(*i).second.size(); ++m )
        VFS::CVfsGlobal::Instance()->GetVfs().UnmountArchiveByName( (*i).second[ m ] );
    m_overlays.erase( i );
}

/*-------------------------------------------------------------------------*/

bool
CUnityImporter::IndexPack( ASSETCONVERTER::CAssetPackContext& context )
{GUCEF_TRACE;

    const CORE::CString& packId = context.GetManifest().id;
    UnmountOverlays( packId );

    const CORE::CString root = GetProjectRoot( context.GetSourceRoot() );
    const UInt32 indexed = IndexFolder( context, root );

    // packages inside the pack that the rules lay over it
    bool ok = true;
    CORE::CString::StringVector globs;
    const CORE::CDataNode* rules = context.GetManifest().rules.FindChild( CUnityProject::SettingsName );
    if ( GUCEF_NULL != rules )
    {
        // a list of globs, or one glob
        const CORE::CDataNode* list = rules->FindChild( "overlayPackages" );
        if ( GUCEF_NULL != list )
        {
            CORE::CDataNode::const_iterator i = list->ConstBegin();
            while ( i != list->ConstEnd() )
            {
                const CORE::CString glob = (*i)->GetValue().AsString();
                if ( !glob.IsNULLOrEmpty() )
                    globs.push_back( glob );
                ++i;
            }
        }
        const CORE::CString single = rules->GetAttributeValue( "overlayPackages" ).AsString();
        if ( !single.IsNULLOrEmpty() )
            globs.push_back( single );
    }
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    UInt32 overlays = 0;
    const CORE::CString::StringVector& files = context.GetFiles();
    for ( size_t f=0; f<files.size() && !globs.empty(); ++f )
    {
        if ( "unitypackage" != ASSETCONVERTER::CVfsPaths::ExtensionOf( files[ f ] ) )
            continue;
        bool match = false;
        for ( size_t g=0; g<globs.size() && !match; ++g )
            match = CORE::CGlobPattern::IsMatch( globs[ g ], files[ f ] );
        if ( !match )
            continue;
        const CORE::CString mountName = "assetpluginUNITY/" + packId + "/overlay" + CORE::UInt32ToString( overlays );
        const CORE::CString mountPoint = "assetpacks/" + packId + ".overlay" + CORE::UInt32ToString( overlays );
        // a package is mounted by its path; one inside the pack is first copied below the intermediate folder, as the
        // reader of packages needs a source it can open a second time, which entries of archives are not
        const CORE::CString copyPath = ASSETCONVERTER::CVfsPaths::Join( context.GetManifest().GetIntermediatePath(), ".overlays/" + packId + "." + CORE::UInt32ToString( overlays ) + ".unitypackage" );
        CORE::CDynamicBuffer bytes;
        bool mounted = vfs.LoadFile( bytes, context.ToVfsPath( files[ f ] ), "rb" ) && vfs.StoreAsFile( copyPath, bytes, 0, true );
        mounted = mounted && vfs.MountArchive( mountName, copyPath, "unitypackage", mountPoint, false, false );
        if ( !mounted )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "UnityImporter: The package " << files[ f ] << " of pack " << packId << " cannot be mounted as an overlay" );
            ok = false;
            continue;
        }
        {
            MT::CScopeMutex lock( m_lock );
            m_overlays[ packId ].push_back( mountName );
        }
        const UInt32 replaced = IndexFolder( context, mountPoint );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "UnityImporter: Pack " << packId << ": the package " << files[ f ] << " lies over the pack with " << replaced << " assets" );
        ++overlays;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "UnityImporter: Pack " << packId << ": " << indexed << " GUIDs indexed below " << ( root.IsNULLOrEmpty() ? CORE::CString( "/" ) : root ) );
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CUnityImporter::ImportModel( CUnityProject& project, CUnityMaterialReader& materials, const CORE::CString& vfsPath )
{GUCEF_TRACE;

    CUnityModel* model = project.GetModel( vfsPath );
    if ( GUCEF_NULL == model )
        return false;
    const CORE::CString subject = project.Describe( vfsPath );
    CORE::CDataNode clipIds;
    model->ApplyClips( project, clipIds );

    // the model as Unity has it, under the provenance and settings of the asset
    ASSETCONVERTER::CAssetScene& out = project.GetScene();
    {
        ASSETCONVERTER::CAssetScene kept = out;
        out = model->scene;
        out.name = kept.name;
        out.provenance = kept.provenance;
        out.settings = kept.settings;
        out.dependencies = kept.dependencies;
        out.extras = kept.extras;
        CORE::CDataNode* fbx = model->scene.extras.FindChild( "fbx" );
        if ( GUCEF_NULL != fbx )
            out.extras.AddChild( *fbx );
    }
    CORE::CDataNode* facts = out.extras.FindOrAddChild( CUnityProject::SettingsName, GUCEF_DATATYPE_OBJECT );
    facts->SetAttribute( "scale", (Float32) model->scale );
    facts->SetAttribute( "fileUnitMetres", (Float32) model->fileUnitMetres );
    facts->SetAttribute( "fileCoordinateSystem", model->fileSystem.ToString() );
    if ( clipIds.HasChildren() )
    {
        CORE::CDataNode* clips = facts->AddChild( clipIds );
        clips->SetName( "clips" );
    }
    project.Info( subject, "imported as Unity imports it: scale " + CORE::DoubleToString( model->scale ) + " (globalScale times the file unit where the .meta applies it; inferred from real packs)" );

    // the materials as Unity assigns them to the model's objects; the texture paths of the model file are never followed
    const UInt32 sourceMaterials = (UInt32) out.materials.size();
    if ( model->hasMeta && !model->meta.importMaterials )
        project.Info( subject, "the .meta imports no materials (importMaterials 0): Unity's default material is used; prefabs give the materials" );
    for ( UInt32 m=0; m<sourceMaterials; ++m )
        out.ReplaceMaterial( m, materials.GetModelMaterial( *model, m ) );
    const Int32 standard = materials.GetDefaultMaterial();
    for ( size_t mesh=0; mesh<out.meshes.size(); ++mesh )
        for ( size_t s=0; s<out.meshes[ mesh ].subMeshes.size(); ++s )
            if ( out.meshes[ mesh ].subMeshes[ s ].materialIndex < 0 )
                out.meshes[ mesh ].subMeshes[ s ].materialIndex = standard;
    out.RemoveUnusedMaterials();
    out.RemoveUnusedTextures();

    // a generated mesh collider per mesh node
    if ( model->hasMeta && model->meta.addColliders )
    {
        for ( size_t n=0; n<out.nodes.size(); ++n )
        {
            if ( out.nodes[ n ].mesh < 0 )
                continue;
            ASSETCONVERTER::CAssetCollider collider;
            collider.name = out.nodes[ n ].name + "_MeshCollider";
            collider.shape = ASSETCONVERTER::COLLIDERSHAPE_TRIMESH;
            collider.meshIndex = out.nodes[ n ].mesh;
            out.nodes[ n ].colliders.push_back( collider );
        }
        project.Info( subject, "the .meta generates mesh colliders (addColliders): a trimesh collider per mesh" );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnityImporter::Import( const CORE::CString& vfsPath, ASSETCONVERTER::CAssetPackContext& context, ASSETCONVERTER::CAssetScene& out, ASSETCONVERTER::CConversionReport& report )
{GUCEF_TRACE;

    const CORE::CString extension = ASSETCONVERTER::CVfsPaths::ExtensionOf( vfsPath );
    out.provenance.importerName = GetName();
    out.provenance.importerVersion = GetVersion();
    out.provenance.sourceEngine = "unity";
    out.provenance.sourceFormat = extension;
    out.coordinateSystem = MATH::CCoordinateSystem::Unity();
    out.normalMapConvention = ASSETCONVERTER::NORMALMAPCONVENTION_OPENGL;

    CUnityProject project( context, out, report, vfsPath );
    const CUnityMeta* meta = project.GetMeta( vfsPath );
    CORE::CDataNode* facts = out.extras.FindOrAddChild( CUnityProject::SettingsName, GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL != meta )
    {
        out.provenance.sourceId = meta->guid;
        facts->SetAttribute( "guid", meta->guid );
        facts->SetAttribute( "importer", meta->importerType );
    }
    else
    {
        project.Info( project.Describe( vfsPath ), "the asset has no .meta file: it has no GUID" );
    }

    CUnityMaterialReader materials( project );
    bool ok = false;
    if ( "prefab" == extension || "unity" == extension )
    {
        CUnityPrefabReader prefabs( project, materials );
        ok = prefabs.ReadAsset( vfsPath );
    }
    else
    if ( "anim" == extension )
    {
        CUnityAnimationClipReader clips( project );
        ok = clips.ReadAsset( vfsPath );
    }
    else
    if ( CUnityModel::IsModelExtension( extension ) )
    {
        ok = ImportModel( project, materials, vfsPath );
        facts = out.extras.FindOrAddChild( CUnityProject::SettingsName, GUCEF_DATATYPE_OBJECT );
        if ( GUCEF_NULL != meta )
        {
            facts->SetAttribute( "guid", meta->guid );
            facts->SetAttribute( "importer", meta->importerType );
        }
    }
    out.coordinateSystem = MATH::CCoordinateSystem::Unity();
    if ( ok )
        report.AddInfo( CUnityProject::SourceName, project.Describe( vfsPath ), "read " + CORE::UInt32ToString( (UInt32) out.nodes.size() ) + " nodes, " +
                        CORE::UInt32ToString( (UInt32) out.meshes.size() ) + " meshes, " + CORE::UInt32ToString( (UInt32) out.materials.size() ) + " materials, " +
                        CORE::UInt32ToString( (UInt32) out.textures.size() ) + " textures, " + CORE::UInt32ToString( (UInt32) out.animations.size() ) + " animations" );
    return ok;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
