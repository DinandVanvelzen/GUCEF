/*
 *  assetpluginUNITY_TestApp: Test application for the Unity plugin of the asset converter (assetpluginUNITY)
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

#include <string.h>

#include "TestUnityFbxWriter.h"
#include "TestUnityContent.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* const GuidHousePrefab = "f0000000000000000000000000000001";
const char* const GuidStreetPrefab = "f0000000000000000000000000000002";
const char* const GuidUrpMaterial = "c0000000000000000000000000000003";
const char* const GuidHouseModel = "b0000000000000000000000000000001";

namespace {

const char* const GuidAssets = "90000000000000000000000000000001";
const char* const GuidAlbedo = "a0000000000000000000000000000001";
const char* const GuidMetal = "a0000000000000000000000000000002";
const char* const GuidNormal = "a0000000000000000000000000000003";
const char* const GuidEmission = "a0000000000000000000000000000004";
const char* const GuidGrid = "a0000000000000000000000000000005";
const char* const GuidOverlay = "a0000000000000000000000000000006";
const char* const GuidHouseSmall = "b0000000000000000000000000000003";
const char* const GuidHouseNew = "b0000000000000000000000000000004";
const char* const GuidMover = "b0000000000000000000000000000002";
const char* const GuidStandard = "c0000000000000000000000000000001";
const char* const GuidNoKeyword = "c0000000000000000000000000000002";
const char* const GuidCustom = "c0000000000000000000000000000004";
const char* const GuidGraphMaterial = "c0000000000000000000000000000005";
const char* const GuidMissing = "c0000000000000000000000000000006";
const char* const GuidShader = "d0000000000000000000000000000001";
const char* const GuidGraph = "d0000000000000000000000000000002";
const char* const GuidConvex = "e1000000000000000000000000000001";
const char* const GuidScene = "f0000000000000000000000000000003";
const char* const GuidAnim = "f0000000000000000000000000000004";
const char* const GuidNewPrefab = "f0000000000000000000000000000005";
const char* const GuidCrateModel = "b0000000000000000000000000000005";
const char* const GuidPairModel = "b0000000000000000000000000000006";
const char* const GuidCratePrefab = "f0000000000000000000000000000006";
const char* const GuidPairPrefab = "f0000000000000000000000000000007";

typedef CORE::CString TText;

const TText YamlHeader = "%YAML 1.1\n%TAG !u! tag:unity3d.com,2011:\n";

/** A texture slot of a material: "    - <name>:\n        m_Texture: ..." */
TText
TexEnv( const char* name, const char* guid, const char* scale = "{x: 1, y: 1}", const char* offset = "{x: 0, y: 0}" )
{
    const TText texture = GUCEF_NULL == guid ? TText( "{fileID: 0}" ) : "{fileID: 2800000, guid: " + TText( guid ) + ", type: 3}";
    return "    - " + TText( name ) + ":\n        m_Texture: " + texture + "\n        m_Scale: " + TText( scale ) + "\n        m_Offset: " + TText( offset ) + "\n";
}

TText
Material( const TText& name, const TText& shader, const TText& keywords, const TText& texEnvs, const TText& floats, const TText& colours, bool newKeys )
{
    const TText prefabKeys = newKeys ? "  m_CorrespondingSourceObject: {fileID: 0}\n  m_PrefabInstance: {fileID: 0}\n  m_PrefabAsset: {fileID: 0}\n"
                                     : "  m_PrefabParentObject: {fileID: 0}\n  m_PrefabInternal: {fileID: 0}\n";
    return "--- !u!21 &2100000\nMaterial:\n  serializedVersion: 6\n  m_ObjectHideFlags: 0\n" + prefabKeys + "  m_Name: " + name + "\n  m_Shader: " + shader + "\n"
           "  m_ShaderKeywords: " + keywords + "\n  m_LightmapFlags: 4\n  m_EnableInstancingVariants: 0\n  m_DoubleSidedGI: 0\n  m_CustomRenderQueue: -1\n"
           "  stringTagMap: {}\n  disabledShaderPasses: []\n  m_SavedProperties:\n    serializedVersion: 3\n    m_TexEnvs:\n" + texEnvs + "    m_Floats:\n" + floats +
           "    m_Colors:\n" + colours;
}

TText
TextureMeta( const char* guid, bool version4, int textureType, int colourFlag, int wrapMode )
{
    TText block = "  serializedVersion: " + TText( version4 ? "4" : "2" ) + "\n  mipmaps:\n    mipMapMode: 0\n    enableMipMap: 1\n";
    block += version4 ? "    sRGBTexture: " + CORE::Int32ToString( colourFlag ) + "\n" : "    linearTexture: " + CORE::Int32ToString( colourFlag ) + "\n";
    block += "  bumpmap:\n    convertToNormalMap: 0\n    heightScale: .25\n  isReadable: 0\n  textureFormat: 1\n  maxTextureSize: 2048\n"
             "  textureSettings:\n    serializedVersion: 2\n    filterMode: -1\n    aniso: -1\n    mipBias: -1\n    wrapMode: " + CORE::Int32ToString( wrapMode ) + "\n"
             "  nPOTScale: 1\n  spriteMode: 0\n  spritePivot: {x: .5, y: .5}\n  alphaIsTransparency: 0\n  textureType: " + CORE::Int32ToString( textureType ) + "\n"
             "  userData: \n  assetBundleName: \n  assetBundleVariant: \n";
    return MakeMeta( guid, "TextureImporter:\n" + block );
}

TText
ModelMetaRecycle( const char* guid, int version, const char* globalScale, bool fileScaleKey, const TText& table, const TText& clips, int importAnimation )
{
    TText meshes = "  meshes:\n    lODScreenPercentages: []\n    globalScale: " + TText( globalScale ) + "\n    meshCompression: 0\n    addColliders: 0\n"
                   "    importBlendShapes: 1\n    swapUVChannels: 0\n    generateSecondaryUV: 0\n    useFileUnits: 1\n";
    if ( fileScaleKey )
        meshes += "    useFileScale: 1\n";
    meshes += "    optimizeMeshForGPU: 1\n    weldVertices: 1\n    secondaryUVAngleDistortion: 8\n";
    return MakeMeta( guid, "ModelImporter:\n  serializedVersion: " + CORE::Int32ToString( version ) + "\n" + table +
                     "  materials:\n    importMaterials: 0\n    materialName: 0\n    materialSearch: 1\n"
                     "  animations:\n    legacyGenerateAnimations: 4\n    bakeSimulation: 0\n    animationCompression: 1\n    animationRotationError: .5\n"
                     "    animationPositionError: .5\n    animationScaleError: .5\n    animationWrapMode: 0\n    extraExposedTransformPaths: []\n"
                     "    clipAnimations:" + clips + "\n    isReadable: 0\n" + meshes +
                     "  tangentSpace:\n    normalSmoothAngle: 60\n    normalImportMode: 0\n    tangentImportMode: 3\n"
                     "  importAnimation: " + CORE::Int32ToString( importAnimation ) + "\n  copyAvatar: 0\n"
                     "  humanDescription:\n    human: []\n    skeleton: []\n    armTwist: .5\n    rootMotionBoneName: \n"
                     "  lastHumanDescriptionAvatarSource: {instanceID: 0}\n  animationType: 2\n  additionalBone: 0\n  userData: \n" );
}

/** A GameObject of the older generation with its components (two lists: "- component:" and the 5.x "- <class>:" form) */
TText
OldGameObject( const char* id, const char* name, const TText& components, int active = 1, bool classKeys = false )
{
    TText list;
    CORE::CString::StringVector entries = TText( components ).ParseElements( ' ', false );
    for ( size_t e=0; e<entries.size(); ++e )
    {
        const TText entry = entries[ e ];
        const Int32 colon = entry.HasChar( ':', true );
        const TText classKey = entry.SubstrFromRange( 0, (UInt32) colon );
        const TText fileId = entry.SubstrFromRange( (UInt32) colon + 1, entry.Length() );
        list += classKeys ? "  - " + classKey + ": {fileID: " + fileId + "}\n" : "  - component: {fileID: " + fileId + "}\n";
    }
    return "--- !u!1 &" + TText( id ) + "\nGameObject:\n  m_ObjectHideFlags: 0\n  m_PrefabParentObject: {fileID: 0}\n  m_PrefabInternal: {fileID: 100100000}\n"
           "  serializedVersion: 5\n  m_Component:\n" + list + "  m_Layer: 0\n  m_Name: " + TText( name ) + "\n  m_TagString: Untagged\n  m_IsActive: " + CORE::Int32ToString( active ) + "\n";
}

TText
OldTransform( const char* id, const char* gameObject, const char* position, const char* father, const TText& children )
{
    TText list = children.IsNULLOrEmpty() ? TText( " []\n" ) : "\n" + children;
    return "--- !u!4 &" + TText( id ) + "\nTransform:\n  m_ObjectHideFlags: 1\n  m_PrefabParentObject: {fileID: 0}\n  m_PrefabInternal: {fileID: 100100000}\n"
           "  m_GameObject: {fileID: " + TText( gameObject ) + "}\n  m_LocalRotation: {x: 0, y: -0, z: -0, w: 1}\n  m_LocalPosition: " + TText( position ) + "\n"
           "  m_LocalScale: {x: 1, y: 1, z: 1}\n  m_Children:" + list + "  m_Father: {fileID: " + TText( father ) + "}\n  m_RootOrder: 0\n"
           "  m_LocalEulerAnglesHint: {x: 0, y: 0, z: 0}\n";
}

TText
Child( const char* id )
{
    return "  - {fileID: " + TText( id ) + "}\n";
}

TText
MeshFilter( const char* id, const char* gameObject, const TText& mesh )
{
    return "--- !u!33 &" + TText( id ) + "\nMeshFilter:\n  m_ObjectHideFlags: 1\n  m_PrefabParentObject: {fileID: 0}\n  m_PrefabInternal: {fileID: 100100000}\n"
           "  m_GameObject: {fileID: " + TText( gameObject ) + "}\n  m_Mesh: " + mesh + "\n";
}

TText
MeshRenderer( const char* id, const char* gameObject, const TText& materials )
{
    return "--- !u!23 &" + TText( id ) + "\nMeshRenderer:\n  m_ObjectHideFlags: 1\n  m_PrefabParentObject: {fileID: 0}\n  m_PrefabInternal: {fileID: 100100000}\n"
           "  m_GameObject: {fileID: " + TText( gameObject ) + "}\n  m_Enabled: 1\n  m_CastShadows: 1\n  m_ReceiveShadows: 1\n  m_Materials:\n" + materials +
           "  m_StaticBatchInfo:\n    firstSubMesh: 0\n    subMeshCount: 0\n  m_StaticBatchRoot: {fileID: 0}\n  m_SortingOrder: 0\n";
}

TText
MaterialRef( const char* guid )
{
    return "  - {fileID: 2100000, guid: " + TText( guid ) + ", type: 2}\n";
}

TText
ModelMesh( const char* fileId, const char* guid )
{
    return "{fileID: " + TText( fileId ) + ", guid: " + TText( guid ) + ", type: 3}";
}

/** Hexadecimal text of the little-endian bytes of floats and 16 bit indices */
TText
HexFloats( const float* values, UInt32 count )
{
    static const char digits[] = "0123456789abcdef";
    TText text;
    for ( UInt32 i=0; i<count; ++i )
    {
        UInt8 bytes[ 4 ];
        memcpy( bytes, &values[ i ], 4 );
        for ( UInt32 b=0; b<4; ++b )
        {
            const char pair[ 3 ] = { digits[ bytes[ b ] >> 4 ], digits[ bytes[ b ] & 15 ], '\0' };
            text += pair;
        }
    }
    return text;
}

TText
HexIndices( const UInt16* values, UInt32 count )
{
    static const char digits[] = "0123456789abcdef";
    TText text;
    for ( UInt32 i=0; i<count; ++i )
    {
        const UInt8 bytes[ 2 ] = { (UInt8) ( values[ i ] & 0xFF ), (UInt8) ( values[ i ] >> 8 ) };
        for ( UInt32 b=0; b<2; ++b )
        {
            const char pair[ 3 ] = { digits[ bytes[ b ] >> 4 ], digits[ bytes[ b ] & 15 ], '\0' };
            text += pair;
        }
    }
    return text;
}

bool
AddPng( CTestUnityProject& project, const char* pathname, const char* guid, UInt8 r, UInt8 g, UInt8 b, UInt8 a, const TText& meta )
{
    CORE::CDynamicBuffer png;
    if ( !MakeTestPng( 8, 8, r, g, b, a, png ) )
        return false;
    project.AddBytes( pathname, guid, png, meta );
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CTestUnityProject::AddText( const CORE::CString& pathname, const CORE::CString& guid, const CORE::CString& text, const CORE::CString& meta )
{
    SAsset asset;
    asset.pathname = pathname;
    asset.guid = guid;
    if ( text.ByteSize() > 1 )
        asset.content.CopyFrom( text.ByteSize() - 1, text.C_String() );
    asset.meta = meta;
    asset.folder = false;
    m_assets.push_back( asset );
}

/*-------------------------------------------------------------------------*/

void
CTestUnityProject::AddBytes( const CORE::CString& pathname, const CORE::CString& guid, const CORE::CDynamicBuffer& bytes, const CORE::CString& meta )
{
    SAsset asset;
    asset.pathname = pathname;
    asset.guid = guid;
    asset.content = bytes;
    asset.meta = meta;
    asset.folder = false;
    m_assets.push_back( asset );
}

/*-------------------------------------------------------------------------*/

void
CTestUnityProject::AddFolder( const CORE::CString& pathname, const CORE::CString& guid )
{
    SAsset asset;
    asset.pathname = pathname;
    asset.guid = guid;
    asset.meta = "fileFormatVersion: 2\nguid: " + guid + "\nfolderAsset: yes\nDefaultImporter:\n  userData: \n";
    asset.folder = true;
    m_assets.push_back( asset );
}

/*-------------------------------------------------------------------------*/

const CTestUnityProject::TAssetVector&
CTestUnityProject::GetAssets( void ) const
{
    return m_assets;
}

/*-------------------------------------------------------------------------*/

bool
CTestUnityProject::WritePackage( const CORE::CString& diskPath ) const
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CreateDirs( CORE::StripFilename( diskPath ) );
    CORE::CFileAccess output( diskPath, "wb" );
    if ( !output.IsValid() )
        return false;
    VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "tgz" );
    if ( writer.IsNULL() || !writer->Open( output ) )
        return false;
    bool ok = true;
    VFS::CArchiveWriterEntrySettings settings;
    settings.modificationTime = CORE::CDateTime( 2021, 6, 15, 12, 34, 56, 0, 0 );
    for ( size_t a=0; a<m_assets.size() && ok; ++a )
    {
        const SAsset& asset = m_assets[ a ];
        ok = writer->AddDirectory( asset.guid, settings.modificationTime );
        if ( !asset.folder )
        {
            CORE::CDynamicBufferAccess content( asset.content );
            ok = ok && writer->AddFile( asset.guid + "/asset", content, settings );
        }
        CORE::CDynamicBuffer meta;
        meta.CopyFrom( asset.meta.ByteSize() - 1, asset.meta.C_String() );
        CORE::CDynamicBufferAccess metaAccess( meta );
        ok = ok && writer->AddFile( asset.guid + "/asset.meta", metaAccess, settings );
        CORE::CDynamicBuffer pathname;
        pathname.CopyFrom( asset.pathname.ByteSize() - 1, asset.pathname.C_String() );
        CORE::CDynamicBufferAccess pathAccess( pathname );
        ok = ok && writer->AddFile( asset.guid + "/pathname", pathAccess, settings );
    }
    return writer->Close() && ok;
}

/*-------------------------------------------------------------------------*/

bool
CTestUnityProject::WriteFolder( const CORE::CString& diskFolder ) const
{GUCEF_TRACE;

    for ( size_t a=0; a<m_assets.size(); ++a )
    {
        const SAsset& asset = m_assets[ a ];
        const CORE::CString path = CORE::CombinePath( diskFolder, asset.pathname );
        CORE::CreateDirs( asset.folder ? path : CORE::StripFilename( path ) );
        if ( !asset.folder )
        {
            CORE::CFileAccess file( path, "wb" );
            if ( !file.IsValid() || ( asset.content.GetDataSize() > 0 && file.Write( asset.content.GetConstBufferPtr(), 1, asset.content.GetDataSize() ) != asset.content.GetDataSize() ) )
                return false;
        }
        if ( !CORE::WriteStringAsTextFile( path + ".meta", asset.meta ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
MakeMeta( const CORE::CString& guid, const CORE::CString& importerBlock )
{
    return "fileFormatVersion: 2\nguid: " + guid + "\ntimeCreated: 1580682637\nlicenseType: Store\n" + importerBlock;
}

/*-------------------------------------------------------------------------*/

bool
BuildTestProject( CTestUnityProject& project )
{GUCEF_TRACE;

    project.AddFolder( "Assets", GuidAssets );

    // textures: the metallic and emission maps carry the sRGB flag, as in real packs
    bool ok = AddPng( project, "Assets/Textures/albedo.png", GuidAlbedo, 200, 40, 40, 255, TextureMeta( GuidAlbedo, true, 0, 1, 1 ) );
    ok = ok && AddPng( project, "Assets/Textures/metal.png", GuidMetal, 255, 0, 0, 128, TextureMeta( GuidMetal, true, 0, 1, -1 ) );
    ok = ok && AddPng( project, "Assets/Textures/normal.png", GuidNormal, 128, 128, 255, 255, TextureMeta( GuidNormal, false, 1, 1, -1 ) );
    ok = ok && AddPng( project, "Assets/Textures/emission.png", GuidEmission, 255, 255, 0, 255, TextureMeta( GuidEmission, true, 0, 1, -1 ) );
    ok = ok && AddPng( project, "Assets/Textures/grid.png", GuidGrid, 40, 200, 40, 255, TextureMeta( GuidGrid, false, 0, 0, -1 ) );
    ok = ok && AddPng( project, "Assets/Textures/overlay.png", GuidOverlay, 10, 10, 10, 255, TextureMeta( GuidOverlay, false, 0, 0, -1 ) );
    if ( !ok )
        return false;

    // the house: centimetres, a 16 unit cube with a door and two windows of the same name
    const STestFbxNode houseNodes[] = { { "House", -1, { 0, 0, 0 }, 16 }, { "Door", 0, { 2, 0, 0 }, 2 }, { "Window", 0, { 5, 5, 0 }, 1 }, { "Window", 0, { 8, 5, 0 }, 3 } };
    const TText houseFbx = MakeNodesFbx( FbxYUpCentimetres(), houseNodes, 4 );
    const TText recycle = "  fileIDToRecycleName:\n    100000: //RootNode\n    100002: Door\n    100004: Window\n    100006: Window\n"
                          "    400000: //RootNode\n    400002: Door\n    400004: Window\n    400006: Window\n"
                          "    4300000: House\n    4300002: Door\n    4300004: Window\n    4300006: Window\n";
    project.AddText( "Assets/Models/house.fbx", GuidHouseModel, houseFbx, ModelMetaRecycle( GuidHouseModel, 18, "100", true, recycle, " []", 0 ) );
    project.AddText( "Assets/Models/house_small.fbx", GuidHouseSmall, houseFbx, ModelMetaRecycle( GuidHouseSmall, 18, "1", true, recycle, " []", 0 ) );
    const TText internalTable = "  internalIDToNameTable:\n  - first:\n      1: -3254102880193716800\n    second: //RootNode\n"
                                "  - first:\n      43: -2345678901234567890\n    second: House\n  - first:\n      43: 6178320345591209833\n    second: Door\n"
                                "  externalObjects: {}\n";
    project.AddText( "Assets/Models/house_old.fbx", "b0000000000000000000000000000007", houseFbx, ModelMetaRecycle( "b0000000000000000000000000000007", 16, "1", false, recycle, " []", 0 ) );
    project.AddText( "Assets/Models/house_new.fbx", GuidHouseNew, houseFbx, ModelMetaRecycle( GuidHouseNew, 21, ".00999999978", true, internalTable, " []", 0 ) );

    // the mover: one take of two seconds, split into two clips of one second
    const TText clips = "\n    - serializedVersion: 16\n      name: Open\n      takeName: Take 001\n      firstFrame: 0\n      lastFrame: 30\n      wrapMode: 0\n"
                        "      orientationOffsetY: 0\n      level: 0\n      cycleOffset: 0\n      loop: 0\n      hasAdditiveReferencePose: 0\n      loopTime: 1\n"
                        "      loopBlend: 0\n      mirror: 0\n      bodyMask: 01000000\n      curves: []\n      events: []\n      transformMask: []\n      maskType: 0\n"
                        "      maskSource: {instanceID: 0}\n"
                        "    - serializedVersion: 16\n      name: Close\n      takeName: Take 001\n      firstFrame: 30\n      lastFrame: 60\n      wrapMode: 0\n"
                        "      loopTime: 0\n      mirror: 0\n      curves: []\n      events:\n      - time: .5\n        functionName: OnClosed\n        data: \n"
                        "      transformMask: []\n      maskType: 0\n";
    const TText moverTable = "  fileIDToRecycleName:\n    100000: //RootNode\n    400000: //RootNode\n    4300000: Mover\n    7400000: Open\n    7400002: Close\n"
                             "    9500000: //RootNode\n";
    project.AddText( "Assets/Models/mover.fbx", GuidMover, MakeMoverFbx( FbxYUpCentimetres() ), ModelMetaRecycle( GuidMover, 19, "1", true, moverTable, clips, 1 ) );

    // the shaders: a ShaderLab shader of the package and a shader graph
    const TText shader = "Shader \"Test/Triplanar\"\n{\n\tProperties\n\t{\n\t\t_Texture(\"Texture\", 2D) = \"white\" {}\n\t\t_Overlay(\"Overlay\", 2D) = \"white\" {}\n"
                         "\t\t[HDR]_EmissionColor(\"Emission Color\", Color) = (0,0,0,0)\n\t\t_Emission(\"Emission\", 2D) = \"white\" {}\n"
                         "\t\t_Opacity(\"Opacity\", Color) = (1,1,1,1) // a colour, not a float\n\t\t_Tiling(\"Tiling\", Range( 0 , 10)) = 1\n"
                         "\t\t[HideInInspector] _texcoord( \"\", 2D ) = \"white\" {}\n\t\t[HideInInspector] __dirty( \"\", Int ) = 1\n\t}\n"
                         "\tSubShader\n\t{\n\t\tTags{ \"RenderType\" = \"Transparent\"  \"Queue\" = \"Transparent+0\" }\n\t\tCGPROGRAM\n"
                         "\t\t#pragma surface surf Standard alpha:fade keepalpha\n\t\tENDCG\n\t}\n}\n";
    project.AddText( "Assets/Shaders/Triplanar.shader", GuidShader, shader, MakeMeta( GuidShader, "ShaderImporter:\n  externalObjects: {}\n  defaultTextures: []\n  userData: \n" ) );
    const TText graph = "{\n    \"m_SGVersion\": 3,\n    \"m_Type\": \"UnityEditor.ShaderGraph.GraphData\",\n    \"m_Properties\": [\n        { \"m_Id\": \"p1\" },\n        { \"m_Id\": \"p2\" }\n    ]\n}\n\n"
                        "{\n    \"m_SGVersion\": 1,\n    \"m_Type\": \"UnityEditor.ShaderGraph.Internal.Texture2DShaderProperty\",\n    \"m_ObjectId\": \"p1\",\n"
                        "    \"m_Name\": \"Grid\",\n    \"m_DefaultReferenceName\": \"_Grid\",\n    \"m_OverrideReferenceName\": \"\"\n}\n\n"
                        "{\n    \"m_SGVersion\": 1,\n    \"m_Type\": \"UnityEditor.ShaderGraph.Internal.ColorShaderProperty\",\n    \"m_ObjectId\": \"p2\",\n"
                        "    \"m_Name\": \"Base Color\",\n    \"m_DefaultReferenceName\": \"Color_A1B2\",\n    \"m_OverrideReferenceName\": \"_BaseColor\"\n}\n";
    project.AddText( "Assets/Shaders/Grid.shadergraph", GuidGraph, graph,
                     MakeMeta( GuidGraph, "ScriptedImporter:\n  internalIDToNameTable: []\n  externalObjects: {}\n  serializedVersion: 2\n  userData: \n"
                                          "  script: {fileID: 11500000, guid: 625f186215c104763be7675aa2d941aa, type: 3}\n" ) );

    // the materials
    const TText nativeMeta = "NativeFormatImporter:\n  externalObjects: {}\n  mainObjectFileID: 2100000\n  userData: \n";
    project.AddText( "Assets/Materials/Standard.mat", GuidStandard,
                     YamlHeader + Material( "Standard_Painted", "{fileID: 46, guid: 0000000000000000f000000000000000, type: 0}", "_ALPHATEST_ON _EMISSION _METALLICGLOSSMAP\n    _NORMALMAP",
                                            TexEnv( "_BumpMap", GuidNormal ) + TexEnv( "_DetailAlbedoMap", GUCEF_NULL ) + TexEnv( "_EmissionMap", GuidEmission ) +
                                            TexEnv( "_MainTex", GuidAlbedo, "{x: 2, y: 2}", "{x: .5, y: 0}" ) + TexEnv( "_MetallicGlossMap", GuidMetal ) + TexEnv( "_OcclusionMap", GUCEF_NULL ),
                                            "    - _BumpScale: 1.5\n    - _Cutoff: .300000012\n    - _GlossMapScale: .5\n    - _Glossiness: .5\n    - _Metallic: -0\n    - _Mode: 1\n",
                                            "    - _Color: {r: 1, g: 1, b: 1, a: 1}\n    - _EmissionColor: {r: 2, g: 1, b: 0, a: 1}\n", false ) +
                     "--- !u!1002 &2100001\nEditorExtensionImpl:\n  serializedVersion: 6\n",
                     MakeMeta( GuidStandard, nativeMeta ) );
    project.AddText( "Assets/Materials/NoKeyword.mat", GuidNoKeyword,
                     YamlHeader + Material( "NoKeyword", "{fileID: 46, guid: 0000000000000000f000000000000000, type: 0}", "_ALPHAPREMULTIPLY_ON",
                                            TexEnv( "_MainTex", GuidAlbedo ), "    - _Glossiness: 0\n    - _Mode: 3\n",
                                            "    - _Color: {r: .5, g: .5, b: .5, a: 1}\n    - _EmissionColor: {r: 1, g: 1, b: 1, a: 1}\n", false ),
                     MakeMeta( GuidNoKeyword, nativeMeta ) );
    project.AddText( "Assets/Materials/Urp.mat", GuidUrpMaterial,
                     YamlHeader + "--- !u!114 &-4229432104447468213\nMonoBehaviour:\n  m_ObjectHideFlags: 11\n  m_CorrespondingSourceObject: {fileID: 0}\n"
                     "  m_PrefabInstance: {fileID: 0}\n  m_PrefabAsset: {fileID: 0}\n  m_GameObject: {fileID: 0}\n  m_Enabled: 1\n  m_EditorHideFlags: 0\n"
                     "  m_Script: {fileID: 11500000, guid: d0353a89b1f911e48b9e16bdc9f2e058, type: 3}\n  m_Name: \n  m_EditorClassIdentifier: \n  version: 1\n" +
                     Material( "Urp_Glass", "{fileID: 4800000, guid: 933532a4fcc9baf4fa0491de14d08ed7,\n    type: 3}", "_SURFACE_TYPE_TRANSPARENT",
                               TexEnv( "_BaseMap", GuidAlbedo ) + TexEnv( "_MainTex", GuidGrid ),
                               "    - _Cull: 0\n    - _Glossiness: 0\n    - _Metallic: 0\n    - _Smoothness: .8\n    - _Surface: 1\n    - _WorkflowMode: 1\n",
                               "    - _BaseColor: {r: .5, g: .5, b: .5, a: .5}\n    - _Color: {r: 1, g: 0, b: 0, a: 1}\n", true ),
                     MakeMeta( GuidUrpMaterial, nativeMeta ) );
    project.AddText( "Assets/Materials/Custom.mat", GuidCustom,
                     YamlHeader + Material( "Custom_Triplanar", "{fileID: 4800000, guid: " + TText( GuidShader ) + ", type: 3}", "",
                                            TexEnv( "_Emission", GuidEmission ) + TexEnv( "_MainTex", GuidAlbedo ) + TexEnv( "_Overlay", GuidOverlay ) + TexEnv( "_Texture", GuidGrid ),
                                            "    - _Tiling: 2\n    - __dirty: 0\n",
                                            "    - _EmissionColor: {r: 1, g: 1, b: 1, a: 1}\n    - _Opacity: {r: .5, g: .5, b: .5, a: 1}\n", false ),
                     MakeMeta( GuidCustom, nativeMeta ) );
    project.AddText( "Assets/Materials/Graph.mat", GuidGraphMaterial,
                     YamlHeader + Material( "Graph_Grid", "{fileID: -6465566751694194690, guid: " + TText( GuidGraph ) + ", type: 3}", "",
                                            TexEnv( "_Grid", GuidGrid ) + TexEnv( "_MainTex", GuidAlbedo ), "    - _QueueOffset: 0\n",
                                            "    - _BaseColor: {r: 1, g: 1, b: 1, a: 1}\n", true ),
                     MakeMeta( GuidGraphMaterial, nativeMeta ) );
    project.AddText( "Assets/Materials/Missing.mat", GuidMissing,
                     YamlHeader + Material( "Missing_Shader", "{fileID: 4800000, guid: 12345678901234567890123456789012, type: 3}", "",
                                            TexEnv( "_MainTex", GuidAlbedo ), "    - _Glossiness: .25\n", "    - _Color: {r: 1, g: 1, b: 1, a: 1}\n", false ),
                     MakeMeta( GuidMissing, nativeMeta ) );

    // a generated convex mesh: a MonoBehaviour and a Mesh with positions only (a tetrahedron)
    const float corners[ 12 ] = { 0, 0, 0,   1, 0, 0,   0, 1, 0,   0, 0, 1 };
    const UInt16 triangles[ 12 ] = { 0, 1, 2,   0, 3, 1,   0, 2, 3,   1, 3, 2 };
    TText channels;
    for ( UInt32 c=0; c<8; ++c )
        channels += "    - stream: 0\n      offset: 0\n      format: 0\n      dimension: " + TText( 0 == c ? "3" : "0" ) + "\n";
    const TText convex = YamlHeader + "--- !u!114 &11400000\nMonoBehaviour:\n  m_ObjectHideFlags: 0\n  m_PrefabParentObject: {fileID: 0}\n  m_PrefabInternal: {fileID: 0}\n"
                         "  m_GameObject: {fileID: 0}\n  m_Enabled: 1\n  m_Script: {fileID: 11500000, guid: 5b71ad40e238046238f9b0c6f33c3791, type: 3}\n"
                         "  m_Name: House_Convex\n  m_EditorClassIdentifier: \n  ConvexMeshes:\n  - {fileID: 43361272355599466}\n  HashOfSourceMeshes: 0cdda16eefd45096\n"
                         "--- !u!43 &43361272355599466\nMesh:\n  m_ObjectHideFlags: 0\n  m_PrefabParentObject: {fileID: 0}\n  m_PrefabInternal: {fileID: 0}\n"
                         "  m_Name: Generated convex submesh 1\n  serializedVersion: 8\n  m_SubMeshes:\n  - serializedVersion: 2\n    firstByte: 0\n    indexCount: 12\n"
                         "    topology: 0\n    firstVertex: 0\n    vertexCount: 4\n    localAABB:\n      m_Center: {x: .5, y: .5, z: .5}\n      m_Extent: {x: .5, y: .5, z: .5}\n"
                         "  m_Shapes:\n    vertices: []\n    shapes: []\n    channels: []\n    fullWeights: []\n  m_BindPose: []\n  m_BoneNameHashes: \n"
                         "  m_RootBoneNameHash: 0\n  m_MeshCompression: 0\n  m_IsReadable: 1\n  m_IndexBuffer: " + HexIndices( triangles, 12 ) + "\n  m_Skin: []\n"
                         "  m_VertexData:\n    m_CurrentChannels: 1\n    m_VertexCount: 4\n    m_Channels:\n" + channels + "    m_DataSize: 48\n"
                         "    _typelessdata: " + HexFloats( corners, 12 ) + "\n  m_CompressedMesh:\n    m_Vertices:\n      m_NumItems: 0\n      m_Range: 0\n      m_Start: 0\n"
                         "      m_Data: \n      m_BitSize: 0\n";
    project.AddText( "Assets/Collision/House_Convex.asset", GuidConvex, convex, MakeMeta( GuidConvex, "NativeFormatImporter:\n  mainObjectFileID: 11400000\n  userData: \n" ) );

    // the older prefab: the whole hierarchy, a Prefab object of its own
    TText house = YamlHeader;
    house += OldGameObject( "100000", "House", "4:400000 33:3300000 23:2300000 65:6500000", 1, true );
    house += OldGameObject( "100002", "Door", "4:400002 33:3300002 23:2300002 135:13500000" );
    house += OldGameObject( "100004", "WindowA", "4:400004 33:3300004 23:2300004" );
    house += OldGameObject( "100006", "WindowB", "4:400006 33:3300006 23:2300006 136:13600000" );
    house += OldGameObject( "100008", "Collision", "4:400008 64:64780012992622230" );
    house += OldGameObject( "100010", "Primitive", "4:400010 33:3300010 23:2300010", 0 );
    house += OldGameObject( "100012", "Lost", "4:400012 33:3300012 23:2300012" );
    house += OldGameObject( "100014", "Lods", "4:400014 205:20500000" );
    house += OldGameObject( "100016", "LOD0", "4:400016 33:3300016 23:2300016" );
    house += OldGameObject( "100018", "LOD1", "4:400018 33:3300018 23:2300018" );
    house += OldGameObject( "100020", "Lamp", "4:400020 108:10800000" );
    house += OldTransform( "400000", "100000", "{x: -0, y: 0, z: 0}", "0", Child( "400002" ) + Child( "400004" ) + Child( "400006" ) + Child( "400008" ) + Child( "400010" ) +
                           Child( "400012" ) + Child( "400014" ) + Child( "400020" ) );
    house += OldTransform( "400002", "100002", "{x: -2, y: 0, z: 0}", "400000", "" );
    house += OldTransform( "400004", "100004", "{x: -5, y: 5, z: 0}", "400000", "" );
    house += OldTransform( "400006", "100006", "{x: -8, y: 5, z: 0}", "400000", "" );
    house += OldTransform( "400008", "100008", "{x: 0, y: 0, z: 0}", "400000", "" );
    house += OldTransform( "400010", "100010", "{x: 0, y: 20, z: 0}", "400000", "" );
    house += OldTransform( "400012", "100012", "{x: 0, y: 0, z: 0}", "400000", "" );
    house += OldTransform( "400014", "100014", "{x: 30, y: 0, z: 0}", "400000", Child( "400016" ) + Child( "400018" ) );
    house += OldTransform( "400016", "100016", "{x: 0, y: 0, z: 0}", "400014", "" );
    house += OldTransform( "400018", "100018", "{x: 0, y: 0, z: 0}", "400014", "" );
    house += OldTransform( "400020", "100020", "{x: 0, y: 10, z: 0}", "400000", "" );
    house += MeshFilter( "3300000", "100000", ModelMesh( "4300000", GuidHouseModel ) );
    house += MeshFilter( "3300002", "100002", ModelMesh( "4300002", GuidHouseModel ) );
    house += MeshFilter( "3300004", "100004", ModelMesh( "4300004", GuidHouseModel ) );
    house += MeshFilter( "3300006", "100006", ModelMesh( "4300006", GuidHouseModel ) );
    house += MeshFilter( "3300010", "100010", "{fileID: 10202, guid: 0000000000000000e000000000000000, type: 0}" );
    house += MeshFilter( "3300012", "100012", "{fileID: 4300000, guid: 99999999999999999999999999999999, type: 3}" );
    house += MeshFilter( "3300016", "100016", ModelMesh( "4300000", GuidHouseModel ) );
    house += MeshFilter( "3300018", "100018", ModelMesh( "4300002", GuidHouseModel ) );
    house += MeshRenderer( "2300000", "100000", MaterialRef( GuidStandard ) );
    house += MeshRenderer( "2300002", "100002", MaterialRef( GuidCustom ) );
    house += MeshRenderer( "2300004", "100004", MaterialRef( GuidMissing ) );
    house += MeshRenderer( "2300006", "100006", MaterialRef( GuidGraphMaterial ) );
    house += MeshRenderer( "2300010", "100010", "  - {fileID: 10303, guid: 0000000000000000f000000000000000, type: 0}\n" );
    house += MeshRenderer( "2300012", "100012", MaterialRef( GuidStandard ) );
    house += MeshRenderer( "2300016", "100016", MaterialRef( GuidStandard ) );
    house += MeshRenderer( "2300018", "100018", MaterialRef( GuidNoKeyword ) );
    house += "--- !u!65 &6500000\nBoxCollider:\n  m_ObjectHideFlags: 1\n  m_PrefabParentObject: {fileID: 0}\n  m_PrefabInternal: {fileID: 100100000}\n"
             "  m_GameObject: {fileID: 100000}\n  m_Material: {fileID: 0}\n  m_IsTrigger: 0\n  m_Enabled: 1\n  serializedVersion: 2\n"
             "  m_Size: {x: 16, y: 16, z: 16}\n  m_Center: {x: -8, y: 8, z: 8}\n";
    house += "--- !u!135 &13500000\nSphereCollider:\n  m_ObjectHideFlags: 1\n  m_GameObject: {fileID: 100002}\n  m_Material: {fileID: 0}\n  m_IsTrigger: 1\n"
             "  m_Enabled: 1\n  serializedVersion: 2\n  m_Radius: .5\n  m_Center: {x: 0, y: 1, z: 0}\n";
    house += "--- !u!136 &13600000\nCapsuleCollider:\n  m_ObjectHideFlags: 1\n  m_GameObject: {fileID: 100006}\n  m_Material: {fileID: 0}\n  m_IsTrigger: 0\n"
             "  m_Enabled: 1\n  m_Radius: .25\n  m_Height: 2\n  m_Direction: 0\n  m_Center: {x: 0, y: 0, z: 0}\n";
    house += "--- !u!64 &64780012992622230\nMeshCollider:\n  m_ObjectHideFlags: 1\n  m_PrefabParentObject: {fileID: 0}\n  m_PrefabInternal: {fileID: 100100000}\n"
             "  m_GameObject: {fileID: 100008}\n  m_Material: {fileID: 0}\n  m_IsTrigger: 0\n  m_Enabled: 1\n  serializedVersion: 2\n  m_Convex: 1\n"
             "  m_InflateMesh: 0\n  m_SkinWidth: .00999999978\n  m_Mesh: {fileID: 43361272355599466, guid: " + TText( GuidConvex ) + ", type: 2}\n";
    house += "--- !u!205 &20500000\nLODGroup:\n  m_ObjectHideFlags: 1\n  m_GameObject: {fileID: 100014}\n  serializedVersion: 2\n  m_LocalReferencePoint: {x: 0, y: 0, z: 0}\n"
             "  m_Size: 16\n  m_FadeMode: 0\n  m_AnimateCrossFading: 0\n  m_LODs:\n  - screenRelativeHeight: .5\n    fadeTransitionWidth: 0\n    renderers:\n"
             "    - renderer: {fileID: 2300016}\n  - screenRelativeHeight: .100000001\n    fadeTransitionWidth: 0\n    renderers:\n    - renderer: {fileID: 2300018}\n"
             "  m_Enabled: 1\n";
    house += "--- !u!108 &10800000\nLight:\n  m_ObjectHideFlags: 1\n  m_GameObject: {fileID: 100020}\n  m_Enabled: 1\n  serializedVersion: 8\n  m_Type: 2\n"
             "  m_Color: {r: 1, g: .5, b: .25, a: 1}\n  m_Intensity: 2\n  m_Range: 12\n  m_SpotAngle: 30\n";
    house += "--- !u!1001 &100100000\nPrefab:\n  m_ObjectHideFlags: 1\n  serializedVersion: 2\n  m_Modification:\n    m_TransformParent: {fileID: 0}\n"
             "    m_Modifications: []\n    m_RemovedComponents: []\n  m_ParentPrefab: {fileID: 0}\n  m_RootGameObject: {fileID: 100000}\n  m_IsPrefabParent: 1\n";
    project.AddText( "Assets/Prefabs/House.prefab", GuidHousePrefab, house, MakeMeta( GuidHousePrefab, "NativeFormatImporter:\n  mainObjectFileID: 100100000\n  userData: \n" ) );

    // a prefab of the house model through the later ID table
    TText newHouse = YamlHeader;
    newHouse += OldGameObject( "100000", "NewHouse", "4:400000 33:3300000 23:2300000" );
    newHouse += OldTransform( "400000", "100000", "{x: 0, y: 0, z: 0}", "0", "" );
    newHouse += MeshFilter( "3300000", "100000", ModelMesh( "-2345678901234567890", GuidHouseNew ) );
    newHouse += MeshRenderer( "2300000", "100000", MaterialRef( GuidStandard ) );
    project.AddText( "Assets/Prefabs/NewHouse.prefab", GuidNewPrefab, newHouse, MakeMeta( GuidNewPrefab, "NativeFormatImporter:\n  userData: \n" ) );

    // the newer prefab: a prefab instance of the house with modifications, a removed component and an added child
    const TText target = ", guid: " + TText( GuidHousePrefab ) + ", type: 3}";
    TText street = YamlHeader;
    street += "--- !u!1 &1000000000000000001\nGameObject:\n  m_ObjectHideFlags: 0\n  m_CorrespondingSourceObject: {fileID: 0}\n  m_PrefabInstance: {fileID: 0}\n"
              "  m_PrefabAsset: {fileID: 0}\n  serializedVersion: 6\n  m_Component:\n  - component: {fileID: 4000000000000000001}\n  m_Layer: 0\n  m_Name: Street\n"
              "  m_IsActive: 1\n";
    street += "--- !u!4 &4000000000000000001\nTransform:\n  m_ObjectHideFlags: 0\n  m_CorrespondingSourceObject: {fileID: 0}\n  m_PrefabInstance: {fileID: 0}\n"
              "  m_PrefabAsset: {fileID: 0}\n  m_GameObject: {fileID: 1000000000000000001}\n  m_LocalRotation: {x: 0, y: 0, z: 0, w: 1}\n"
              "  m_LocalPosition: {x: 0, y: 0, z: 0}\n  m_LocalScale: {x: 1, y: 1, z: 1}\n  m_Children:\n  - {fileID: 5000000000000000001}\n"
              "  m_Father: {fileID: 0}\n  m_RootOrder: 0\n";
    street += "--- !u!1001 &-7000000000000000001\nPrefabInstance:\n  m_ObjectHideFlags: 0\n  serializedVersion: 2\n  m_Modification:\n"
              "    m_TransformParent: {fileID: 4000000000000000001}\n    m_Modifications:\n"
              "    - target: {fileID: 400000" + target + "\n      propertyPath: m_LocalPosition.x\n      value: 5\n      objectReference: {fileID: 0}\n"
              "    - target: {fileID: 100000" + target + "\n      propertyPath: m_Name\n      value: HouseA\n      objectReference: {fileID: 0}\n"
              "    - target: {fileID: 2300000" + target + "\n      propertyPath: m_Materials.Array.data[0]\n      value: \n"
              "      objectReference: {fileID: 2100000, guid: " + TText( GuidUrpMaterial ) + ", type: 2}\n"
              "    - target: {fileID: 999999" + target + "\n      propertyPath: m_Name\n      value: Ghost\n      objectReference: {fileID: 0}\n"
              "    m_RemovedComponents:\n    - {fileID: 6500000" + target + "\n"
              "  m_SourcePrefab: {fileID: 100100000" + target + "\n";
    street += "--- !u!4 &5000000000000000001 stripped\nTransform:\n  m_CorrespondingSourceObject: {fileID: 400000" + target + "\n"
              "  m_PrefabInstance: {fileID: -7000000000000000001}\n  m_PrefabAsset: {fileID: 0}\n";
    street += "--- !u!1 &1000000000000000002\nGameObject:\n  m_ObjectHideFlags: 0\n  serializedVersion: 6\n  m_Component:\n  - component: {fileID: 4000000000000000002}\n"
              "  m_Name: Sign\n  m_IsActive: 1\n";
    street += "--- !u!4 &4000000000000000002\nTransform:\n  m_GameObject: {fileID: 1000000000000000002}\n  m_LocalRotation: {x: 0, y: 0, z: 0, w: 1}\n"
              "  m_LocalPosition: {x: 0, y: 3, z: 0}\n  m_LocalScale: {x: 1, y: 1, z: 1}\n  m_Children: []\n  m_Father: {fileID: 5000000000000000001}\n";
    project.AddText( "Assets/Prefabs/Street.prefab", GuidStreetPrefab, street, MakeMeta( GuidStreetPrefab, "PrefabImporter:\n  externalObjects: {}\n  userData: \n" ) );

    // a scene of the older generation: settings, a prefab instance and a light
    const TText sceneTarget = ", guid: " + TText( GuidHousePrefab ) + ", type: 2}";
    TText scene = YamlHeader;
    scene += "--- !u!29 &1\nOcclusionCullingSettings:\n  m_ObjectHideFlags: 0\n  serializedVersion: 2\n";
    scene += "--- !u!104 &2\nRenderSettings:\n  m_ObjectHideFlags: 0\n  serializedVersion: 8\n  m_Fog: 0\n  m_FogColor: {r: .5, g: .5, b: .5, a: 1}\n";
    scene += "--- !u!1001 &1500000001\nPrefab:\n  m_ObjectHideFlags: 0\n  serializedVersion: 2\n  m_Modification:\n    m_TransformParent: {fileID: 0}\n"
             "    m_Modifications:\n"
             "    - target: {fileID: 400000" + sceneTarget + "\n      propertyPath: m_LocalPosition.z\n      value: 3\n      objectReference: {fileID: 0}\n"
             "    - target: {fileID: 400000" + sceneTarget + "\n      propertyPath: m_LocalRotation.y\n      value: .707106829\n      objectReference: {fileID: 0}\n"
             "    - target: {fileID: 400000" + sceneTarget + "\n      propertyPath: m_LocalRotation.w\n      value: .707106829\n      objectReference: {fileID: 0}\n"
             "    m_RemovedComponents: []\n  m_ParentPrefab: {fileID: 100100000" + sceneTarget + "\n  m_IsPrefabParent: 0\n";
    scene += "--- !u!1 &600000\nGameObject:\n  m_ObjectHideFlags: 0\n  m_PrefabParentObject: {fileID: 0}\n  m_PrefabInternal: {fileID: 0}\n  serializedVersion: 5\n"
             "  m_Component:\n  - component: {fileID: 400600}\n  - component: {fileID: 10800600}\n  m_Name: Sun\n  m_IsActive: 1\n";
    scene += "--- !u!4 &400600\nTransform:\n  m_GameObject: {fileID: 600000}\n  m_LocalRotation: {x: .408217937, y: -.234569728, z: .109381676, w: .875426054}\n"
             "  m_LocalPosition: {x: 0, y: 3, z: 0}\n  m_LocalScale: {x: 1, y: 1, z: 1}\n  m_Children: []\n  m_Father: {fileID: 0}\n";
    scene += "--- !u!108 &10800600\nLight:\n  m_GameObject: {fileID: 600000}\n  m_Enabled: 1\n  m_Type: 1\n  m_Color: {r: 1, g: .956862748, b: .839215696, a: 1}\n"
             "  m_Intensity: 1.20000005\n";
    project.AddText( "Assets/Scenes/Demo.unity", GuidScene, scene, MakeMeta( GuidScene, "DefaultImporter:\n  userData: \n" ) );

    // an animation clip with generic curves: an exact rotation, stepped positions, a scale component, a muscle curve
    const TText anim = YamlHeader + "--- !u!74 &7400000\nAnimationClip:\n  m_ObjectHideFlags: 0\n  m_Name: Spin\n  serializedVersion: 6\n  m_Legacy: 0\n"
                       "  m_Compressed: 0\n  m_UseHighQualityCurve: 1\n  m_RotationCurves:\n  - curve:\n      serializedVersion: 2\n      m_Curve:\n"
                       "      - serializedVersion: 3\n        time: 0\n        value: {x: 0, y: 0, z: 0, w: 1}\n        inSlope: {x: 0, y: 0, z: 0, w: 0}\n"
                       "        outSlope: {x: 0, y: .785398185, z: 0, w: 0}\n        tangentMode: 0\n        weightedMode: 0\n"
                       "        inWeight: {x: .333333343, y: .333333343, z: .333333343, w: .333333343}\n        outWeight: {x: .333333343, y: .333333343, z: .333333343, w: .333333343}\n"
                       "      - serializedVersion: 3\n        time: 1\n        value: {x: 0, y: .707106769, z: 0, w: .707106769}\n        inSlope: {x: 0, y: .5, z: 0, w: -.5}\n"
                       "        outSlope: {x: 0, y: 0, z: 0, w: 0}\n        tangentMode: 0\n        weightedMode: 0\n"
                       "      m_PreInfinity: 2\n      m_PostInfinity: 2\n      m_RotationOrder: 4\n    path: Arm\n"
                       "  m_CompressedRotationCurves: []\n  m_EulerCurves: []\n  m_PositionCurves:\n  - curve:\n      serializedVersion: 2\n      m_Curve:\n"
                       "      - serializedVersion: 3\n        time: 0\n        value: {x: 0, y: 0, z: 0}\n        inSlope: {x: Infinity, y: Infinity, z: Infinity}\n"
                       "        outSlope: {x: Infinity, y: Infinity, z: Infinity}\n        tangentMode: 103\n        weightedMode: 0\n"
                       "      - serializedVersion: 3\n        time: .5\n        value: {x: 1, y: 0, z: 0}\n        inSlope: {x: Infinity, y: Infinity, z: Infinity}\n"
                       "        outSlope: {x: Infinity, y: Infinity, z: Infinity}\n        tangentMode: 103\n        weightedMode: 0\n"
                       "      m_PreInfinity: 2\n      m_PostInfinity: 2\n      m_RotationOrder: 4\n    path: Arm/Hand\n  m_ScaleCurves: []\n"
                       "  m_FloatCurves:\n  - curve:\n      serializedVersion: 2\n      m_Curve:\n      - serializedVersion: 3\n        time: 0\n        value: 1\n"
                       "        inSlope: 0\n        outSlope: 0\n        tangentMode: 136\n        weightedMode: 0\n      - serializedVersion: 3\n        time: 1\n"
                       "        value: 2\n        inSlope: 0\n        outSlope: 0\n        tangentMode: 136\n        weightedMode: 0\n"
                       "      m_PreInfinity: 2\n      m_PostInfinity: 2\n    attribute: m_LocalScale.x\n    path: Arm\n    classID: 4\n    script: {fileID: 0}\n"
                       "  - curve:\n      serializedVersion: 2\n      m_Curve:\n      - serializedVersion: 3\n        time: 0\n        value: 0\n        inSlope: 0\n"
                       "        outSlope: 0\n        tangentMode: 0\n    attribute: RootT.x\n    path: \n    classID: 95\n    script: {fileID: 0}\n"
                       "  m_PPtrCurves: []\n  m_SampleRate: 30\n  m_WrapMode: 0\n  m_Bounds:\n    m_Center: {x: 0, y: 0, z: 0}\n    m_Extent: {x: 0, y: 0, z: 0}\n"
                       "  m_AnimationClipSettings:\n    serializedVersion: 2\n    m_StartTime: 0\n    m_StopTime: 1\n    m_LoopTime: 1\n    m_KeepOriginalOrientation: 0\n"
                       "  m_EditorCurves: []\n  m_EulerEditorCurves: []\n  m_HasGenericRootTransform: 0\n  m_Events: []\n";
    // models of the later ID scheme (an empty internalIDToNameTable: the IDs are computed from the names), an instance of
    // such a model with the IDs of its root objects, and prefabs that name meshes by computed IDs
    const TText laterMeta = "ModelImporter:\n  serializedVersion: 22200\n  internalIDToNameTable: []\n  externalObjects: {}\n  materials:\n    materialImportMode: 0\n"
                            "    materialName: 0\n    materialSearch: 1\n    materialLocation: 1\n  meshes:\n    lODScreenPercentages: []\n    globalScale: 1\n"
                            "    useFileScale: 1\n  importAnimation: 1\n  animationType: 2\n";
    const STestFbxNode crateNodes[] = { { "Crate", -1, { 0, 0, 0 }, 100 } };
    project.AddText( "Assets/Models/crate.fbx", GuidCrateModel, MakeNodesFbx( FbxYUpCentimetres(), crateNodes, 1 ), MakeMeta( GuidCrateModel, laterMeta ) );
    const STestFbxNode pairNodes[] = { { "Pair", -1, { 0, 0, 0 }, 0 }, { "Left", 0, { 0, 0, 0 }, 50 }, { "Right", 0, { 100, 0, 0 }, 200 } };
    project.AddText( "Assets/Models/pair.fbx", GuidPairModel, MakeNodesFbx( FbxYUpCentimetres(), pairNodes, 3 ), MakeMeta( GuidPairModel, laterMeta ) );
    const TText crateTarget = ", guid: " + TText( GuidCrateModel ) + ",\n        type: 3}";
    TText crate = YamlHeader;
    crate += "--- !u!1001 &7012961101115699340\nPrefabInstance:\n  m_ObjectHideFlags: 0\n  serializedVersion: 2\n  m_Modification:\n    serializedVersion: 3\n"
             "    m_TransformParent: {fileID: 0}\n    m_Modifications:\n"
             "    - target: {fileID: -8679921383154817045" + crateTarget + "\n      propertyPath: m_LocalPosition.x\n      value: 2\n      objectReference: {fileID: 0}\n"
             "    - target: {fileID: -8679921383154817045" + crateTarget + "\n      propertyPath: m_LocalEulerAnglesHint.x\n      value: 0\n      objectReference: {fileID: 0}\n"
             "    - target: {fileID: -7511558181221131132" + crateTarget + "\n      propertyPath: m_Materials.Array.data[0]\n      value: \n"
             "      objectReference: {fileID: 2100000, guid: " + TText( GuidStandard ) + ", type: 2}\n"
             "    - target: {fileID: 919132149155446097" + crateTarget + "\n      propertyPath: m_Name\n      value: CrateA\n      objectReference: {fileID: 0}\n"
             "    m_RemovedComponents: []\n    m_RemovedGameObjects: []\n    m_AddedGameObjects: []\n    m_AddedComponents: []\n"
             "  m_SourcePrefab: {fileID: 100100000, guid: " + TText( GuidCrateModel ) + ", type: 3}\n";
    project.AddText( "Assets/Prefabs/Crate.prefab", GuidCratePrefab, crate, MakeMeta( GuidCratePrefab, "PrefabImporter:\n  externalObjects: {}\n  userData: \n" ) );
    TText pair = YamlHeader;
    pair += "--- !u!1 &1000000000000000011\nGameObject:\n  m_ObjectHideFlags: 0\n  serializedVersion: 6\n  m_Component:\n  - component: {fileID: 4000000000000000011}\n"
            "  - component: {fileID: 3300000000000000011}\n  - component: {fileID: 2300000000000000011}\n  m_Name: Right\n  m_IsActive: 1\n";
    pair += "--- !u!4 &4000000000000000011\nTransform:\n  m_GameObject: {fileID: 1000000000000000011}\n  m_LocalRotation: {x: 0, y: 0, z: 0, w: 1}\n"
            "  m_LocalPosition: {x: 0, y: 0, z: 0}\n  m_LocalScale: {x: 1, y: 1, z: 1}\n  m_Children: []\n  m_Father: {fileID: 0}\n";
    pair += "--- !u!33 &3300000000000000011\nMeshFilter:\n  m_GameObject: {fileID: 1000000000000000011}\n  m_Mesh: {fileID: 5444477770160502568, guid: " + TText( GuidPairModel ) + ", type: 3}\n";
    pair += "--- !u!23 &2300000000000000011\nMeshRenderer:\n  m_GameObject: {fileID: 1000000000000000011}\n  m_Enabled: 1\n  m_Materials:\n" + MaterialRef( GuidStandard );
    project.AddText( "Assets/Prefabs/Pair.prefab", GuidPairPrefab, pair, MakeMeta( GuidPairPrefab, "PrefabImporter:\n  externalObjects: {}\n  userData: \n" ) );

    // a package inside the package with a variant of a material under the same GUID, as render-pipeline variants ship
    CTestUnityProject overlay;
    overlay.AddText( "Assets/Materials/Standard.mat", GuidStandard,
                     YamlHeader + Material( "Standard_Urp", "{fileID: 4800000, guid: 933532a4fcc9baf4fa0491de14d08ed7, type: 3}", "",
                                            TexEnv( "_BaseMap", GuidGrid ), "    - _Smoothness: .5\n    - _Surface: 0\n", "    - _BaseColor: {r: 1, g: 1, b: 1, a: 1}\n", true ),
                     MakeMeta( GuidStandard, nativeMeta ) );
    CORE::CDynamicBuffer overlayBytes;
    if ( !overlay.WritePackage( WorkDiskPath( "overlay.unitypackage" ) ) || !ReadDiskFile( WorkDiskPath( "overlay.unitypackage" ), overlayBytes ) )
        return false;
    project.AddBytes( "Assets/URP_ExtractMe.unitypackage", "90000000000000000000000000000002", overlayBytes, MakeMeta( "90000000000000000000000000000002", "DefaultImporter:\n  userData: \n" ) );

    project.AddText( "Assets/Animation/Spin.anim", GuidAnim, anim, MakeMeta( GuidAnim, "NativeFormatImporter:\n  mainObjectFileID: 7400000\n  userData: \n" ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
WriteTestProject( const CORE::CString& name )
{GUCEF_TRACE;

    CTestUnityProject project;
    if ( !BuildTestProject( project ) )
        return false;
    MakeWorkFolder( name );
    return project.WritePackage( WorkDiskPath( name + "/test.unitypackage" ) ) && project.WriteFolder( WorkDiskPath( name + "/project" ) );
}

/*-------------------------------------------------------------------------*/
