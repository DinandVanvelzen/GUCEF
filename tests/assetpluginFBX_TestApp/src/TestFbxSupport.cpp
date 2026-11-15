/*
 *  assetpluginFBX_TestApp: Test application for the FBX plugin of the asset converter (assetpluginFBX)
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

#include "TestFbxSupport.h"

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

static const char* const TestPluginGroup = "assetpluginFBX_TestApp";
static const char* const WorkMountPoint = "work";
static const char* const WorkArchiveName = "assetpluginFBX_TestApp/work";
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
SetUpFbxTests( void )
{GUCEF_TRACE;

    VFS::CVfsGlobal::Instance();
    IMAGE::CImageGlobal::Instance();
    CAssetConverterGlobal::Instance();

    // JSON for every file of the converter, zlib for PNG, FreeImage to read pictures, the importer plugin and the glTF plugin whose exporter writes the intermediate
    bool loaded = LoadTestPlugin( "dstorepluginJSONPARSER" );
    loaded = LoadTestPlugin( "codecspluginZLIB" ) && loaded;
    loaded = LoadTestPlugin( "imgpluginFreeImage" ) && loaded;
    loaded = LoadTestPlugin( "assetpluginGLTF" ) && loaded;
    loaded = LoadTestPlugin( "assetpluginFBX" ) && loaded;
    if ( !loaded )
        return false;

    g_workRoot = CORE::CombinePath( CORE::TempDir(), "assetpluginFBX_TestApp" );
    if ( CORE::DirExists( g_workRoot ) )
        CORE::DeleteDir( g_workRoot, true );
    CORE::CreateDirs( g_workRoot );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.MountArchive( WorkArchiveName, g_workRoot, VFS::CVFS::FileSystemArchiveTypeName, WorkMountPoint, true, false, g_workRoot ) )
        return false;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr json;
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( json );
}

/*-------------------------------------------------------------------------*/

void
TearDownFbxTests( void )
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
    return file.IsValid() && file.Write( bytes, 1, size ) == size;
}

/*-------------------------------------------------------------------------*/

bool
WriteWorkText( const CORE::CString& relativePath, const CORE::CString& text )
{GUCEF_TRACE;

    const CORE::CString path = WorkDiskPath( relativePath );
    CORE::CreateDirs( CORE::StripFilename( path ) );
    return CORE::WriteStringAsTextFile( path, text );
}

/*-------------------------------------------------------------------------*/

bool
ReadVfsBytes( const CORE::CString& vfsPath, CORE::CDynamicBuffer& bytes )
{GUCEF_TRACE;

    return VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( bytes, vfsPath, "rb" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
ReadVfsText( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer;
    if ( !ReadVfsBytes( vfsPath, buffer ) )
        return CORE::CString::Empty;
    const char terminator = '\0';
    buffer.Append( &terminator, 1, true );
    return CORE::CString( static_cast< const char* >( buffer.GetConstBufferPtr() ) );
}

/*-------------------------------------------------------------------------*/

bool
SameBytes( const CORE::CString& vfsPathA, const CORE::CString& vfsPathB )
{GUCEF_TRACE;

    CORE::CDynamicBuffer a;
    CORE::CDynamicBuffer b;
    if ( !ReadVfsBytes( vfsPathA, a ) || !ReadVfsBytes( vfsPathB, b ) )
        return false;
    return a.GetDataSize() == b.GetDataSize() && 0 == memcmp( a.GetConstBufferPtr(), b.GetConstBufferPtr(), a.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
WriteTestPng( const CORE::CString& vfsPath, UInt32 width, UInt32 height, UInt8 r, UInt8 g, UInt8 b, UInt8 a )
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
    return CTextureBaker::SavePng( vfsPath, *picture );
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
OpenPackContext( CAssetPackContext& context, const CORE::CString& sourceFolder, const CORE::CString& intermediateFolder )
{GUCEF_TRACE;

    CAssetPackManifest manifest;
    manifest.id = "test";
    manifest.source = "/" + sourceFolder;
    manifest.intermediate = "/" + intermediateFolder;
    return context.Open( manifest );
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
