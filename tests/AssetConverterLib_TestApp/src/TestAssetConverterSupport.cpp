/*
 *  AssetConverterLib_TestApp: Test application for the asset converter library (AssetConverterLib)
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

#include <math.h>
#include <string.h>

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "TestAssetConverterSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETCONVERTER;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const TestPluginGroup = "AssetConverterLibTests";
const char* const WorkMountPoint = "work";
const char* const WorkArchiveName = "AssetConverterLibTests/work";

CORE::CString g_workRoot;
CStubImporter* g_stubImporter = GUCEF_NULL;
CStubCooker* g_stubCooker = GUCEF_NULL;
MT::CMutex g_stubLock;

/*-------------------------------------------------------------------------*/

bool
LoadTestPlugin( const char* moduleName )
{
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    const CORE::CString path = CORE::CString( "$MODULEDIR$/" ) + moduleName;
    return pluginControl.AddPluginFromDir( path + "_d", TestPluginGroup, true ) || pluginControl.AddPluginFromDir( path, TestPluginGroup, true );
}

/*-------------------------------------------------------------------------*/

bool
StoreBytes( const CORE::CString& vfsPath, UInt32 size, char fill )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    vfs.CreateDir( CVfsPaths::FolderOf( vfsPath ) );
    CORE::CDynamicBuffer buffer( size, true );
    for ( UInt32 i=0; i<size; ++i )
        buffer.Append( &fill, 1, true );
    return vfs.StoreAsFile( vfsPath, buffer, 0, true );
}

/*-------------------------------------------------------------------------*/

void
AddBox( CAssetScene& scene, UInt32 node, Float32 size, Int32 material )
{
    const Float32 h = size * 0.5f;
    CAssetMesh mesh;
    mesh.name = scene.nodes[ node ].name;
    CAssetSubMesh subMesh;
    for ( UInt32 i=0; i<8; ++i )
    {
        const Float32 x = ( i & 1 ) ? h : -h, y = ( i & 2 ) ? h : -h, z = ( i & 4 ) ? h : -h;
        subMesh.positions.push_back( x );
        subMesh.positions.push_back( y );
        subMesh.positions.push_back( z );
        const Float32 n = 1.0f / (Float32) sqrt( 3.0 );
        subMesh.normals.push_back( x > 0.0f ? n : -n );
        subMesh.normals.push_back( y > 0.0f ? n : -n );
        subMesh.normals.push_back( z > 0.0f ? n : -n );
        subMesh.uvSets[ 0 ].push_back( x > 0.0f ? 1.0f : 0.0f );
        subMesh.uvSets[ 0 ].push_back( y > 0.0f ? 1.0f : 0.0f );
    }
    const UInt32 faces[] = { 0,2,1, 1,2,3,  4,5,6, 5,7,6,  0,1,4, 1,5,4,  2,6,3, 3,6,7,  0,4,2, 2,4,6,  1,3,5, 3,7,5 };
    for ( UInt32 i=0; i<sizeof( faces ) / sizeof( faces[ 0 ] ); ++i )
        subMesh.indices.push_back( faces[ i ] );
    subMesh.materialIndex = material;
    mesh.subMeshes.push_back( subMesh );
    scene.meshes.push_back( mesh );
    scene.nodes[ node ].mesh = (Int32) scene.meshes.size() - 1;
}

/*-------------------------------------------------------------------------*/

Int32
AddTexture( CAssetScene& scene, GUCEF::map< CORE::CString, Int32 >& known, const CORE::CString& vfsPath, TTextureUsage usage )
{
    GUCEF::map< CORE::CString, Int32 >::const_iterator i = known.find( vfsPath );
    if ( i != known.end() )
        return (*i).second;
    CAssetTexture texture;
    texture.name = CVfsPaths::NameOf( vfsPath );
    texture.sources.push_back( vfsPath );
    texture.usage = usage;
    scene.textures.push_back( texture );
    known[ vfsPath ] = (Int32) scene.textures.size() - 1;
    // the normalized relation, as a real importer records it: the file, or the name when it is not there
    if ( VFS::CVfsGlobal::Instance()->GetVfs().FileExists( vfsPath ) )
        scene.AddReference( CAssetReference::KindUsesTexture, vfsPath, CVfsPaths::NameOf( vfsPath ) );
    else
        scene.AddUnresolvedReference( CAssetReference::KindUsesTexture, CVfsPaths::NameOf( vfsPath ) );
    return known[ vfsPath ];
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CStubImporter::CStubImporter( void )
    : CIAssetImporter()
    , version( 1 )
    , importCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString CStubImporter::GetName( void ) const { return "stub"; }
UInt32 CStubImporter::GetVersion( void ) const { return version; }

/*-------------------------------------------------------------------------*/

bool
CStubImporter::CanImport( const CORE::CString& vfsPath, CAssetPackContext& /* context */ ) const
{GUCEF_TRACE;

    return "stub" == CVfsPaths::ExtensionOf( vfsPath );
}

/*-------------------------------------------------------------------------*/

bool
CStubImporter::IndexPack( CAssetPackContext& context )
{GUCEF_TRACE;

    // every .stub file by its name, as an importer would index GUIDs
    const CORE::CString::StringVector& files = context.GetFiles();
    for ( size_t i=0; i<files.size(); ++i )
        if ( "stub" == CVfsPaths::ExtensionOf( files[ i ] ) )
            context.SetIndexEntry( "name", CVfsPaths::StemOf( files[ i ] ), files[ i ] );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CStubImporter::GetDefaultSettings( CORE::CDataNode& settings ) const
{GUCEF_TRACE;

    settings.SetAttribute( "stubDefault", "default" );
    settings.SetAttribute( "stubScale", (UInt32) 1 );
}

/*-------------------------------------------------------------------------*/

bool
CStubImporter::Import( const CORE::CString& vfsPath, CAssetPackContext& /* context */, CAssetScene& out, CConversionReport& report )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( g_stubLock );
        ++importCount;
    }
    CORE::CDataNode source;
    if ( !CCanonicalJson::Load( vfsPath, source ) )
    {
        report.AddError( "stub", vfsPath, "not readable as JSON" );
        return false;
    }
    if ( source.GetAttributeValueOrChildValueByName( "fail" ).AsBool( false ) )
    {
        report.AddError( "stub", vfsPath, "the source asks to fail" );
        return false;
    }
    const CORE::CString folder = CVfsPaths::FolderOf( vfsPath );
    out.provenance.sourceFormat = "stub";
    out.provenance.sourceFormatVersion = "1";
    out.provenance.sourceEngine = source.GetAttributeValueOrChildValueByName( "engine" ).AsString();
    const CORE::CString system = source.GetAttributeValueOrChildValueByName( "system" ).AsString();
    if ( !system.IsNULLOrEmpty() )
        out.coordinateSystem.FromString( system );
    FromString( source.GetAttributeValueOrChildValueByName( "normalMaps" ).AsString(), out.normalMapConvention );

    GUCEF::map< CORE::CString, Int32 > textures;
    const CORE::CDataNode* materials = source.FindChild( "materials" );
    if ( GUCEF_NULL != materials )
    {
        CORE::CDataNode::const_iterator i = materials->ConstBegin();
        while ( i != materials->ConstEnd() )
        {
            CAssetMaterial material;
            material.name = (*i)->GetAttributeValueOrChildValueByName( "name" ).AsString();
            const CORE::CString baseColor = (*i)->GetAttributeValueOrChildValueByName( "baseColor" ).AsString();
            if ( !baseColor.IsNULLOrEmpty() )
                material.baseColorTexture = CAssetTextureReference( AddTexture( out, textures, CVfsPaths::Join( folder, baseColor ), TEXTUREUSAGE_COLOUR ) );
            const CORE::CString normal = (*i)->GetAttributeValueOrChildValueByName( "normal" ).AsString();
            if ( !normal.IsNULLOrEmpty() )
                material.normalTexture = CAssetTextureReference( AddTexture( out, textures, CVfsPaths::Join( folder, normal ), TEXTUREUSAGE_NORMAL ) );
            material.source.shaderName = "Stub/Shader";
            material.source.properties.SetAttribute( "_Color", baseColor );
            out.materials.push_back( material );
            ++i;
        }
    }

    const CORE::CDataNode* nodes = source.FindChild( "nodes" );
    if ( GUCEF_NULL != nodes )
    {
        CORE::CDataNode::const_iterator i = nodes->ConstBegin();
        while ( i != nodes->ConstEnd() )
        {
            const UInt32 node = out.AddNode( (*i)->GetAttributeValueOrChildValueByName( "name" ).AsString(), (*i)->GetAttributeValueOrChildValueByName( "parent" ).AsInt32( -1 ) );
            CORE::CDataNode::TVariantVector position;
            (*i)->GetValuesOfChildByName( "position", position );
            if ( 3 == position.size() )
                out.nodes[ node ].transform.position = MATH::TFloat32Transform3D::TPositionVector( position[ 0 ].AsFloat32(), position[ 1 ].AsFloat32(), position[ 2 ].AsFloat32() );
            if ( (*i)->GetAttributeValueOrChildValueByName( "mesh" ).AsBool( false ) )
            {
                AddBox( out, node, (*i)->GetAttributeValueOrChildValueByName( "size" ).AsFloat32( 1.0f ), (*i)->GetAttributeValueOrChildValueByName( "material" ).AsInt32( -1 ) );
                if ( (*i)->GetAttributeValueOrChildValueByName( "skinned" ).AsBool( false ) )
                {
                    // four influences per vertex, on four joints
                    CAssetSubMesh& subMesh = out.meshes.back().subMeshes[ 0 ];
                    subMesh.influencesPerVertex = 4;
                    for ( UInt32 v=0; v<subMesh.GetVertexCount(); ++v )
                        for ( UInt32 j=0; j<4; ++j )
                        {
                            subMesh.joints.push_back( j );
                            subMesh.weights.push_back( 0.25f );
                        }
                }
            }
            ++i;
        }
    }

    CORE::CString::StringVector animations;
    source.GetValuesOfChildByName( "animations", animations );
    for ( size_t a=0; a<animations.size() && !out.nodes.empty(); ++a )
    {
        CAssetAnimation animation;
        animation.name = animations[ a ];
        CAssetAnimationChannel channel;
        channel.targetNode = 0;
        channel.times.push_back( 0.0f );
        channel.times.push_back( 1.0f );
        for ( UInt32 v=0; v<6; ++v )
            channel.values.push_back( (Float32) v );
        animation.channels.push_back( channel );
        out.animations.push_back( animation );
    }

    CORE::CString::StringVector dependencies;
    source.GetValuesOfChildByName( "dependencies", dependencies );
    for ( size_t d=0; d<dependencies.size(); ++d )
        out.AddDependency( CVfsPaths::Join( folder, dependencies[ d ] ) );

    // relations of other kinds: { "kind", "path" (relative to the folder of the source, absent for an unresolved one), "via", "optional" }
    const CORE::CDataNode* references = source.FindChild( "references" );
    if ( GUCEF_NULL != references )
    {
        for ( const CORE::CDataNode* reference = references->GetFirstChild(); GUCEF_NULL != reference; reference = reference->GetNext() )
        {
            const CORE::CString kind = reference->GetAttributeValueOrChildValueByName( "kind" ).AsString();
            const CORE::CString path = reference->GetAttributeValueOrChildValueByName( "path" ).AsString();
            const CORE::CString via = reference->GetAttributeValueOrChildValueByName( "via" ).AsString();
            const bool optional = reference->GetAttributeValueOrChildValueByName( "optional" ).AsBool( false );
            if ( path.IsNULLOrEmpty() )
                out.AddUnresolvedReference( kind, via, optional );
            else
                out.AddReference( kind, CVfsPaths::Join( folder, path ), via, optional );
        }
    }

    const CORE::CString approximation = source.GetAttributeValueOrChildValueByName( "approximation" ).AsString();
    if ( !approximation.IsNULLOrEmpty() )
        report.AddApproximation( "stub", out.name, approximation );

    // what reached the importer, for the tests
    CORE::CDataNode* settings = out.extras.AddChild( out.settings );
    settings->SetName( "settings" );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString CStubExporter::GetName( void ) const { return "gltf"; }
UInt32 CStubExporter::GetVersion( void ) const { return 1; }

/*-------------------------------------------------------------------------*/

bool
CStubExporter::Export( const CAssetScene& scene, const CORE::CString& vfsDir, CConversionReport& report )
{GUCEF_TRACE;

    CORE::CDataNode document( "gltf", GUCEF_DATATYPE_OBJECT );
    document.SetAttribute( "asset", scene.provenance.assetPath );
    CORE::CDataNode* nodes = document.AddChild( "nodes", GUCEF_DATATYPE_ARRAY );
    for ( size_t n=0; n<scene.nodes.size(); ++n )
    {
        CORE::CDataNode* node = nodes->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        node->SetAttribute( "name", scene.nodes[ n ].name );
        CORE::CDataNode::TVariantVector translation;
        translation.push_back( CORE::CVariant( scene.nodes[ n ].transform.position.x ) );
        translation.push_back( CORE::CVariant( scene.nodes[ n ].transform.position.y ) );
        translation.push_back( CORE::CVariant( scene.nodes[ n ].transform.position.z ) );
        node->SetValuesOfChildByName( "translation", translation );
    }
    CORE::CDataNode* meshes = document.AddChild( "meshes", GUCEF_DATATYPE_ARRAY );
    for ( size_t m=0; m<scene.meshes.size(); ++m )
    {
        CORE::CDataNode* mesh = meshes->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        mesh->SetAttribute( "name", scene.meshes[ m ].name );
        mesh->SetAttribute( "vertices", scene.meshes[ m ].GetVertexCount() );
        mesh->SetAttribute( "triangles", scene.meshes[ m ].GetTriangleCount() );
    }
    CORE::CString::StringVector images;
    for ( size_t t=0; t<scene.textures.size(); ++t )
        if ( !scene.textures[ t ].bakedPath.IsNULLOrEmpty() )
            images.push_back( scene.textures[ t ].bakedPath );
    document.SetValuesOfChildByName( "images", images );

    const CORE::CString path = CVfsPaths::Join( vfsDir, scene.name + ".gltf" );
    if ( !CCanonicalJson::Store( path, document ) )
        return false;
    report.AddOutputFile( path, images );
    return true;
}

/*-------------------------------------------------------------------------*/

CStubCooker::CStubCooker( void )
    : CIAssetCooker()
    , version( 1 )
    , maxBones( 0 )
    , cookCalls( 0 )
    , cookedAssets()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString CStubCooker::GetTargetName( void ) const { return "stubcook"; }
UInt32 CStubCooker::GetVersion( void ) const { return version; }
UInt32 CStubCooker::GetMaxBonesPerSubMesh( void ) const { return maxBones; }

/*-------------------------------------------------------------------------*/

bool
CStubCooker::Cook( const CORE::CString& /* intermediateDir */, const CORE::CString& outDir, const CCookSettings& settings, CConversionReport& report )
{GUCEF_TRACE;

    ++cookCalls;
    const CORE::CString base = CVfsPaths::Join( outDir, settings.packId );
    const CORE::CString shared = CVfsPaths::Join( base, "textures/shared.tex" );
    bool ok = StoreBytes( shared, SharedTextureSize, 's' );
    for ( size_t a=0; a<settings.assets.size(); ++a )
    {
        const CCookSettings::SAsset& asset = settings.assets[ a ];
        report.SelectAsset( asset.assetPath );
        const CORE::CString stem = CVfsPaths::StemOf( asset.assetPath );
        const CORE::CString mesh = CVfsPaths::Join( base, stem + ".mesh" );
        const CORE::CString material = CVfsPaths::Join( base, stem + ".material" );
        const CORE::CString own = CVfsPaths::Join( base, "textures/" + stem + ".tex" );
        ok = StoreBytes( mesh, MeshSize, 'm' ) && ok;
        ok = StoreBytes( material, MaterialSize, 'a' ) && ok;
        ok = StoreBytes( own, OwnTextureSize, 'o' ) && ok;
        CORE::CString::StringVector references;
        references.push_back( material );
        report.AddOutputFile( mesh, references, settings.profile.name );
        references.clear();
        references.push_back( own );
        references.push_back( shared );
        report.AddOutputFile( material, references, settings.profile.name );
        report.AddOutputFile( own, CORE::CString::StringVector(), settings.profile.name );
        report.AddOutputFile( shared, CORE::CString::StringVector(), settings.profile.name );
        cookedAssets.push_back( asset.assetPath );
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

COrderPostProcessor::COrderPostProcessor( const CORE::CString& name )
    : CIAssetPostProcessor()
    , m_name( name )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString COrderPostProcessor::GetName( void ) const { return m_name; }
UInt32 COrderPostProcessor::GetVersion( void ) const { return 1; }

/*-------------------------------------------------------------------------*/

bool
COrderPostProcessor::Process( CAssetScene& scene, const CORE::CDataNode& settings, CConversionReport& /* report */ )
{GUCEF_TRACE;

    const CORE::CString order = scene.extras.GetAttributeValueOrChildValueByName( "order" ).AsString();
    CORE::CString entry = m_name;
    const CORE::CString tag = settings.GetAttributeValueOrChildValueByName( "tag" ).AsString();
    if ( !tag.IsNULLOrEmpty() )
        entry += "(" + tag + ")";
    scene.extras.SetAttribute( "order", order.IsNULLOrEmpty() ? entry : order + "," + entry );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
SetUpConverterTests( void )
{GUCEF_TRACE;

    VFS::CVfsGlobal::Instance();
    IMAGE::CImageGlobal::Instance();
    CAssetConverterGlobal::Instance();

    // JSON for every file of the converter, zlib for PNG, FreeImage to read pictures, zip for packs that are archives
    LoadTestPlugin( "dstorepluginJSONPARSER" );
    LoadTestPlugin( "codecspluginZLIB" );
    LoadTestPlugin( "imgpluginFreeImage" );
    LoadTestPlugin( "vfspluginZIP" );

    // the work folder, writeable in the VFS
    g_workRoot = CORE::CombinePath( CORE::TempDir(), "AssetConverterLib_TestApp" );
    if ( CORE::DirExists( g_workRoot ) )
        CORE::DeleteDir( g_workRoot, true );
    CORE::CreateDirs( g_workRoot );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.MountArchive( WorkArchiveName, g_workRoot, VFS::CVFS::FileSystemArchiveTypeName, WorkMountPoint, true, false, g_workRoot ) )
        return false;

    // the plugins of the tests
    CAssetPluginRegistry& plugins = CAssetConverterGlobal::Instance()->GetPluginRegistry();
    g_stubImporter = GUCEF_NEW CStubImporter();
    g_stubCooker = GUCEF_NEW CStubCooker();
    plugins.RegisterImporter( TAssetImporterPtr( g_stubImporter ) );
    plugins.RegisterExporter( TAssetExporterPtr( GUCEF_NEW CStubExporter() ) );
    plugins.RegisterCooker( TAssetCookerPtr( g_stubCooker ) );
    plugins.RegisterPostProcessor( TAssetPostProcessorPtr( GUCEF_NEW COrderPostProcessor( "OrderA" ) ) );
    plugins.RegisterPostProcessor( TAssetPostProcessorPtr( GUCEF_NEW COrderPostProcessor( "OrderB" ) ) );

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr json;
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( json );
}

/*-------------------------------------------------------------------------*/

void
TearDownConverterTests( void )
{GUCEF_TRACE;

    CAssetPluginRegistry& plugins = CAssetConverterGlobal::Instance()->GetPluginRegistry();
    plugins.UnregisterImporter( "stub" );
    plugins.UnregisterExporter( "gltf" );
    plugins.UnregisterCooker( "stubcook" );
    plugins.UnregisterPostProcessor( "OrderA" );
    plugins.UnregisterPostProcessor( "OrderB" );
    g_stubImporter = GUCEF_NULL;
    g_stubCooker = GUCEF_NULL;
    VFS::CVfsGlobal::Instance()->GetVfs().UnmountArchiveByName( WorkArchiveName );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( TestPluginGroup );
}

/*-------------------------------------------------------------------------*/

CStubImporter& GetStubImporter( void ) { return *g_stubImporter; }
CStubCooker& GetStubCooker( void ) { return *g_stubCooker; }

/*-------------------------------------------------------------------------*/

CORE::CString
WorkDiskPath( const CORE::CString& relativePath )
{GUCEF_TRACE;

    return CORE::CombinePath( g_workRoot, relativePath );
}

/*-------------------------------------------------------------------------*/

CORE::CString
MakeWorkFolder( const CORE::CString& name )
{GUCEF_TRACE;

    const CORE::CString folder = WorkDiskPath( name );
    if ( CORE::DirExists( folder ) )
        CORE::DeleteDir( folder, true );
    CORE::CreateDirs( folder );
    return folder;
}

/*-------------------------------------------------------------------------*/

bool
WriteWorkFile( const CORE::CString& relativePath, const CORE::CString& text )
{GUCEF_TRACE;

    const CORE::CString path = WorkDiskPath( relativePath );
    CORE::CreateDirs( CORE::StripFilename( path ) );
    return CORE::WriteStringAsTextFile( path, text );
}

/*-------------------------------------------------------------------------*/

CORE::CString
ReadVfsText( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer;
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( buffer, vfsPath, "rb" ) )
        return CORE::CString::Empty;
    const char terminator = '\0';
    buffer.Append( &terminator, 1, true );
    return CORE::CString( static_cast< const char* >( buffer.GetConstBufferPtr() ) );
}

/*-------------------------------------------------------------------------*/

IMAGE::TPixelMapPtr
MakePicture( UInt32 width, UInt32 height, UInt8 r, UInt8 g, UInt8 b, UInt8 a )
{GUCEF_TRACE;

    IMAGE::TPixelMapPtr picture( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    UInt8* texels = static_cast< UInt8* >( picture->GetDataPtr() );
    for ( UInt32 i=0; i<width*height; ++i )
    {
        texels[ i*4 ] = r;
        texels[ i*4+1 ] = g;
        texels[ i*4+2 ] = b;
        texels[ i*4+3 ] = a;
    }
    return picture;
}

/*-------------------------------------------------------------------------*/

const UInt8*
Texels( const IMAGE::CPixelMap& picture )
{GUCEF_TRACE;

    return static_cast< const UInt8* >( picture.GetDataPtr() );
}

/*-------------------------------------------------------------------------*/

bool
WriteTestPng( const CORE::CString& vfsPath, UInt32 width, UInt32 height, UInt8 r, UInt8 g, UInt8 b, UInt8 a )
{GUCEF_TRACE;

    IMAGE::TPixelMapPtr picture = MakePicture( width, height, r, g, b, a );
    return CTextureBaker::SavePng( vfsPath, *picture );
}

/*-------------------------------------------------------------------------*/

bool
WriteLicence( const CORE::CString& relativeFolder, const CORE::CString& id, bool ueOnly, bool referenceOnly, bool restricted, bool reviewed )
{GUCEF_TRACE;

    CLicenceRecord record;
    record.id = id;
    record.store = "Test store";
    record.product = "Test pack";
    record.licenceName = "Test licence";
    record.seats = 1;
    record.ueOnly = ueOnly;
    record.referenceOnly = referenceOnly;
    record.restrictedOrNonStandard = restricted;
    record.reviewed = reviewed;
    CORE::CDataNode node( "licence", GUCEF_DATATYPE_OBJECT );
    record.Serialize( node );
    return WriteWorkFile( relativeFolder + "/" + id + ".licence.json", CCanonicalJson::ToJson( node ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
MakeStubSource( const CORE::CString& nodeName, const CORE::CString& baseColorTexture )
{GUCEF_TRACE;

    CORE::CString text = "{ \"nodes\": [ { \"name\": \"" + nodeName + "\", \"mesh\": true, \"size\": 2, \"material\": 0 } ], \"materials\": [ { \"name\": \"" + nodeName + "Material\"";
    if ( !baseColorTexture.IsNULLOrEmpty() )
        text += ", \"baseColor\": \"" + baseColorTexture + "\"";
    text += " } ] }";
    return text;
}

/*-------------------------------------------------------------------------*/

bool
Near( Float32 a, Float32 b, Float32 tolerance )
{
    return fabs( a - b ) <= tolerance;
}

/*-------------------------------------------------------------------------*/

CORE::CString
WriteStandardPack( const CORE::CString& name, const CORE::CString& extraMembers, bool withLicence )
{GUCEF_TRACE;

    MakeWorkFolder( name );
    const CORE::CString vfsFolder = "work/" + name;
    WriteWorkFile( name + "/src/props/rock.stub", "{ \"nodes\": [ { \"name\": \"Rock\", \"mesh\": true, \"size\": 2, \"material\": 0 } ], "
                   "\"materials\": [ { \"name\": \"RockMaterial\", \"baseColor\": \"rock.png\" } ], \"dependencies\": [ \"rock.notes.txt\" ] }" );
    WriteWorkFile( name + "/src/props/crate.stub", MakeStubSource( "Crate", "crate.png" ) );
    WriteWorkFile( name + "/src/props/rock.notes.txt", "notes" );
    WriteWorkFile( name + "/src/readme.txt", "not an asset" );
    WriteTestPng( vfsFolder + "/src/props/rock.png", 4, 4, 120, 110, 100, 255 );
    WriteTestPng( vfsFolder + "/src/props/crate.png", 4, 4, 90, 60, 30, 255 );
    if ( withLicence )
        WriteLicence( name + "/licences", "rec", false, false, false, false );
    CORE::CString manifest = "{ \"id\": \"" + name + "\", \"title\": \"Pack " + name + "\", \"store\": \"Test store\", \"licenceRecord\": \"rec\", "
                             "\"licenceFolder\": \"licences\", \"source\": \"src\", \"importer\": \"auto\", \"overrides\": \"overrides\", "
                             "\"cookTargets\": [ \"stubcook\" ], \"cookProfiles\": [ { \"name\": \"desktop\" } ]";
    if ( !extraMembers.IsNULLOrEmpty() )
        manifest += ", " + extraMembers;
    manifest += " }";
    WriteWorkFile( name + "/pack.assetpack.json", manifest );
    return vfsFolder + "/pack.assetpack.json";
}
