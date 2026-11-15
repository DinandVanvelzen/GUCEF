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

#include <math.h>
#include <string.h>

#include "cgltf.h"

#include "TestUnitySupport.h"

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

static const char* const TestPluginGroup = "assetpluginUNITY_TestApp";
static const char* const WorkMountPoint = "work";
static const char* const WorkArchiveName = "assetpluginUNITY_TestApp/work";
static CORE::CString g_workRoot;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
LoadTestPlugin( const char* moduleName )
{
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    const CORE::CString path = CORE::CString( "$MODULEDIR$/" ) + moduleName;
    return pluginControl.AddPluginFromDir( path + "_d", TestPluginGroup, true ) || pluginControl.AddPluginFromDir( path, TestPluginGroup, true );
}

/*-------------------------------------------------------------------------*/

bool
SetUpUnityTests( void )
{GUCEF_TRACE;

    VFS::CVfsGlobal::Instance();
    IMAGE::CImageGlobal::Instance();
    CAssetConverterGlobal::Instance();

    // JSON for the files of the converter, zlib for PNG, FreeImage to read pictures, libarchive for the archive writer,
    // the package reader, the model importers and the Unity importer
    bool loaded = LoadTestPlugin( "dstorepluginJSONPARSER" );
    loaded = LoadTestPlugin( "codecspluginZLIB" ) && loaded;
    loaded = LoadTestPlugin( "imgpluginFreeImage" ) && loaded;
    loaded = LoadTestPlugin( "vfspluginLIBARCHIVE" ) && loaded;
    loaded = LoadTestPlugin( "vfspluginUNITYPACKAGE" ) && loaded;
    loaded = LoadTestPlugin( "assetpluginGLTF" ) && loaded;
    loaded = LoadTestPlugin( "assetpluginFBX" ) && loaded;
    loaded = LoadTestPlugin( "assetpluginUNITY" ) && loaded;
    if ( !loaded )
        return false;

    g_workRoot = CORE::CombinePath( CORE::TempDir(), "assetpluginUNITY_TestApp" );
    if ( CORE::DirExists( g_workRoot ) )
        CORE::DeleteDir( g_workRoot, true );
    CORE::CreateDirs( g_workRoot );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.MountArchive( WorkArchiveName, g_workRoot, VFS::CVFS::FileSystemArchiveTypeName, WorkMountPoint, true, false, g_workRoot ) )
        return false;

    TAssetImporterPtr importer;
    if ( !CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "unity", importer ) )
        return false;
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr json;
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( json );
}

/*-------------------------------------------------------------------------*/

void
TearDownUnityTests( void )
{GUCEF_TRACE;

    VFS::CVfsGlobal::Instance()->GetVfs().UnmountArchiveByName( WorkArchiveName );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( TestPluginGroup );
}

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
WriteWorkFile( const CORE::CString& relativePath, const void* bytes, UInt32 size )
{GUCEF_TRACE;

    const CORE::CString path = WorkDiskPath( relativePath );
    CORE::CreateDirs( CORE::StripFilename( path ) );
    CORE::CFileAccess file( path, "wb" );
    return file.IsValid() && ( 0 == size || file.Write( bytes, 1, size ) == size );
}

/*-------------------------------------------------------------------------*/

bool
WriteWorkText( const CORE::CString& relativePath, const CORE::CString& text )
{GUCEF_TRACE;

    return WriteWorkFile( relativePath, text.C_String(), text.ByteSize() > 0 ? text.ByteSize() - 1 : 0 );
}

/*-------------------------------------------------------------------------*/

bool
ReadDiskFile( const CORE::CString& diskPath, CORE::CDynamicBuffer& bytes )
{GUCEF_TRACE;

    return bytes.LoadContentFromFile( diskPath );
}

/*-------------------------------------------------------------------------*/

bool
MakeTestPng( UInt32 width, UInt32 height, UInt8 r, UInt8 g, UInt8 b, UInt8 a, CORE::CDynamicBuffer& bytes )
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
    static UInt32 counter = 0;
    const CORE::CString name = "png/picture" + CORE::UInt32ToString( counter++ ) + ".png";
    CORE::CreateDirs( WorkDiskPath( "png" ) );
    if ( !CTextureBaker::SavePng( "work/" + name, *picture ) )
        return false;
    return ReadDiskFile( WorkDiskPath( name ), bytes );
}

/*-------------------------------------------------------------------------*/

bool
WriteLicence( const CORE::CString& relativeFolder, const CORE::CString& id )
{GUCEF_TRACE;

    CLicenceRecord record;
    record.id = id;
    record.store = "Test store";
    record.product = "Test pack";
    record.licenceName = "Test licence";
    record.seats = 1;
    CORE::CDataNode node( "licence", GUCEF_DATATYPE_OBJECT );
    record.Serialize( node );
    return WriteWorkText( relativeFolder + "/" + id + ".licence.json", CCanonicalJson::ToJson( node ) );
}

/*-------------------------------------------------------------------------*/

bool
ValidateWithCgltf( const CORE::CString& diskPath, CORE::CString& problem )
{GUCEF_TRACE;

    cgltf_options options;
    memset( &options, 0, sizeof( options ) );
    cgltf_data* data = GUCEF_NULL;
    cgltf_result result = cgltf_parse_file( &options, diskPath.C_String(), &data );
    if ( cgltf_result_success != result )
    {
        problem = "cgltf_parse_file failed with " + CORE::Int32ToString( (Int32) result );
        return false;
    }
    result = cgltf_load_buffers( &options, data, diskPath.C_String() );
    if ( cgltf_result_success == result )
        result = cgltf_validate( data );
    cgltf_free( data );
    if ( cgltf_result_success != result )
    {
        problem = "cgltf buffers or validation failed with " + CORE::Int32ToString( (Int32) result );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
Near( Float32 a, Float32 b, Float32 tolerance )
{
    return fabs( a - b ) <= tolerance;
}

/*-------------------------------------------------------------------------*/

Int32
FindNode( const CAssetScene& scene, const CORE::CString& name )
{GUCEF_TRACE;

    return scene.FindNode( name );
}

/*-------------------------------------------------------------------------*/

void
GetMeshBounds( const CAssetMesh& mesh, Float32 min[ 3 ], Float32 max[ 3 ] )
{GUCEF_TRACE;

    for ( UInt32 a=0; a<3; ++a )
    {
        min[ a ] = 1e30f;
        max[ a ] = -1e30f;
    }
    for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
    {
        const CAssetSubMesh::TFloatVector& positions = mesh.subMeshes[ s ].positions;
        for ( size_t p=0; p+2<positions.size(); p+=3 )
        {
            for ( UInt32 a=0; a<3; ++a )
            {
                if ( positions[ p+a ] < min[ a ] ) min[ a ] = positions[ p+a ];
                if ( positions[ p+a ] > max[ a ] ) max[ a ] = positions[ p+a ];
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
GetSceneBounds( const CAssetScene& scene, Float32 min[ 3 ], Float32 max[ 3 ] )
{GUCEF_TRACE;

    bool any = false;
    for ( UInt32 a=0; a<3; ++a )
    {
        min[ a ] = 1e30f;
        max[ a ] = -1e30f;
    }
    for ( size_t n=0; n<scene.nodes.size(); ++n )
    {
        if ( scene.nodes[ n ].mesh < 0 )
            continue;
        const MATH::TFloat32Matrix4D world = scene.GetWorldMatrix( (UInt32) n );
        const CAssetMesh& mesh = scene.meshes[ scene.nodes[ n ].mesh ];
        for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
        {
            const CAssetSubMesh::TFloatVector& positions = mesh.subMeshes[ s ].positions;
            for ( size_t p=0; p+2<positions.size(); p+=3 )
            {
                const MATH::CTPoint3D< Float32 > point = world.TransformPoint( MATH::CTPoint3D< Float32 >( positions[ p ], positions[ p+1 ], positions[ p+2 ] ) );
                const Float32 values[ 3 ] = { point.x, point.y, point.z };
                for ( UInt32 a=0; a<3; ++a )
                {
                    if ( values[ a ] < min[ a ] ) min[ a ] = values[ a ];
                    if ( values[ a ] > max[ a ] ) max[ a ] = values[ a ];
                }
                any = true;
            }
        }
    }
    return any;
}

/*-------------------------------------------------------------------------*/

UInt32
CountItems( const CConversionReport& report, TReportItemKind kind, const CORE::CString& text )
{GUCEF_TRACE;

    UInt32 count = 0;
    const CConversionReport::TEntryVector& entries = report.GetEntries();
    for ( size_t e=0; e<entries.size(); ++e )
        for ( size_t i=0; i<entries[ e ].items.size(); ++i )
            if ( entries[ e ].items[ i ].kind == kind && ( text.IsNULLOrEmpty() || entries[ e ].items[ i ].message.HasSubstr( text ) >= 0 || entries[ e ].items[ i ].subject.HasSubstr( text ) >= 0 ) )
                ++count;
    return count;
}

/*-------------------------------------------------------------------------*/
