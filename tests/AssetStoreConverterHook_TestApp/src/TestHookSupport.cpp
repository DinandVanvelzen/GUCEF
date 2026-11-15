/*
 *  AssetStoreConverterHook_TestApp: Test application for the converter hook of the primary asset store (AssetStoreConverterHook)
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

#include <stdlib.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */
#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */
#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */
#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */
#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */
#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */
#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */
#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */
#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H
#include "gucefIMAGE_CImageGlobal.h"
#define GUCEF_IMAGE_CIMAGEGLOBAL_H
#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */
#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */
#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */
#ifndef ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#include "AssetConverterLib_CAssetConverterGlobal.h"
#define ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H ? */
#ifndef DBPLUGIN_POSTGRESQL_H
#include "dbpluginPOSTGRESQL.h"
#define DBPLUGIN_POSTGRESQL_H
#endif /* DBPLUGIN_POSTGRESQL_H ? */

#include "TestHookSupport.h"

using namespace GUCEF;
using namespace GUCEF::CORE;

namespace HOOKTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const PluginGroup = "AssetStoreConverterHookTestPlugins";

bool
AddPlugin( const CString& name )
{
    #ifdef GUCEF_DEBUG_MODE
    const CString first = "$MODULEDIR$/" + name + "_d";
    const CString second = "$MODULEDIR$/" + name;
    #else
    const CString first = "$MODULEDIR$/" + name;
    const CString second = "$MODULEDIR$/" + name + "_d";
    #endif
    CPluginControl& pluginControl = CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( first, PluginGroup, true ) )
        return true;
    return pluginControl.AddPluginFromDir( second, PluginGroup, true );
}

CString
TestDirectory( const CString& name )
{
    return CombinePath( CombinePath( TempDir(), "AssetStoreConverterHook_TestApp" ), name );
}

CString
Base64( const GUCEF::vector< UInt8 >& bytes )
{
    static const char* const alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    CString text;
    for ( size_t i=0; i<bytes.size(); i+=3 )
    {
        const UInt32 a = bytes[ i ];
        const UInt32 b = i + 1 < bytes.size() ? bytes[ i + 1 ] : 0;
        const UInt32 c = i + 2 < bytes.size() ? bytes[ i + 2 ] : 0;
        const UInt32 triple = ( a << 16 ) | ( b << 8 ) | c;
        const char group[ 4 ] = { alphabet[ ( triple >> 18 ) & 63 ], alphabet[ ( triple >> 12 ) & 63 ], i + 1 < bytes.size() ? alphabet[ ( triple >> 6 ) & 63 ] : '=', i + 2 < bytes.size() ? alphabet[ triple & 63 ] : '=' };
        text.Append( group, 4 );
    }
    return text;
}

void
AddFloat( GUCEF::vector< UInt8 >& bytes, Float32 value )
{
    const UInt8* p = reinterpret_cast< const UInt8* >( &value );
    bytes.insert( bytes.end(), p, p + 4 );
}

void
AddUInt16( GUCEF::vector< UInt8 >& bytes, UInt16 value )
{
    bytes.push_back( (UInt8) ( value & 255 ) );
    bytes.push_back( (UInt8) ( value >> 8 ) );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      FUNCTIONS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
SetUpTestEnvironment( void )
{
    VFS::CVfsGlobal::Instance();
    IMAGE::CImageGlobal::Instance();
    ASSETCONVERTER::CAssetConverterGlobal::Instance();
    bool loaded = AddPlugin( "dstorepluginJSONPARSER" );
    loaded = AddPlugin( "codecspluginZLIB" ) && loaded;
    loaded = AddPlugin( "imgpluginFreeImage" ) && loaded;
    loaded = AddPlugin( "assetpluginGLTF" ) && loaded;
    AddPlugin( "tablecodecpluginPARQUET" );
    AddPlugin( "dbpluginVFSTABLES" );

    // the postgresql backend is linked: its plugin entry point registers it
    if ( !COMCORE::CComCoreGlobal::Instance()->GetDatabaseClientFactory().IsConstructible( "postgresql" ) )
        GUCEFPlugin_Load( 0, GUCEF_NULL );

    VFS::CVfsGlobal::Instance()->GetVfs().MountAllDelayMountedArchives();
    return loaded;
}

/*-------------------------------------------------------------------------*/

void
TearDownTestEnvironment( void )
{
    // the plugins go while everything they use is still there, not in the handlers at exit
    CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( PluginGroup );
}

/*-------------------------------------------------------------------------*/

CString
MountFreshDirectory( const CString& name, const CString& mountPoint )
{
    const CString dir = TestDirectory( name );
    if ( DirExists( dir ) )
        DeleteDir( dir, true );
    CreateDirs( dir );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    vfs.UnmountArchiveByName( "HookTest_" + name );
    vfs.AddRoot( dir, "HookTest_" + name, true, false, mountPoint );
    return dir;
}

/*-------------------------------------------------------------------------*/

void
UnmountDirectory( const CString& name )
{
    VFS::CVfsGlobal::Instance()->GetVfs().UnmountArchiveByName( "HookTest_" + name );
}

/*-------------------------------------------------------------------------*/

void
PumpFor( UInt32 durationInMs )
{
    for ( UInt32 waited=0; waited<durationInMs; waited+=2 )
    {
        CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 2 );
    }
}

/*-------------------------------------------------------------------------*/

bool
WriteVfsFile( const CString& path, const CDynamicBuffer& content )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const Int32 slash = path.HasChar( '/', false );
    if ( slash > 0 )
        vfs.CreateDir( path.SubstrToIndex( (UInt32) slash, true ) );
    return vfs.StoreAsFile( path, content, 0, true );
}

/*-------------------------------------------------------------------------*/

bool
WriteVfsFile( const CString& path, const CString& content )
{
    CDynamicBuffer buffer;
    buffer.CopyFrom( content, 0, false );
    return WriteVfsFile( path, buffer );
}

/*-------------------------------------------------------------------------*/

bool
ReadVfsFile( const CString& path, CDynamicBuffer& content )
{
    content.Clear();
    return VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( content, path, "rb" );
}

/*-------------------------------------------------------------------------*/

CString
ReadVfsText( const CString& path )
{
    CDynamicBuffer buffer;
    if ( !ReadVfsFile( path, buffer ) )
        return CString::Empty;
    const char terminator = '\0';
    buffer.Append( &terminator, 1, true );
    return CString( static_cast< const char* >( buffer.GetConstBufferPtr() ) );
}

/*-------------------------------------------------------------------------*/

bool
MakePng( UInt32 width, UInt32 height, CDynamicBuffer& content )
{
    GUCEF::vector< UInt8 > pixels( width * height * 3 );
    for ( UInt32 p=0; p<width*height; ++p )
    {
        pixels[ p*3 ] = (UInt8) ( p & 0xFF );
        pixels[ p*3+1 ] = (UInt8) ( ( p >> 3 ) & 0xFF );
        pixels[ p*3+2 ] = 40;
    }
    IMAGE::TPixelMapPtr pixelMap( GUCEF_NEW IMAGE::CPixelMap( &pixels[ 0 ], width, height, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8 ) );
    IMAGE::CImage image( pixelMap );
    content.Clear();
    CDynamicBufferAccess access( content );
    return image.Save( access, "png" ) && content.GetDataSize() > 0;
}

/*-------------------------------------------------------------------------*/

CString
MakeTriangleGltf( const CString& nodeName, const CString& pictureName, const CString& generator )
{
    GUCEF::vector< UInt8 > binary;
    const Float32 positions[ 9 ] = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
    for ( UInt32 i=0; i<9; ++i )
        AddFloat( binary, positions[ i ] );
    const Float32 normals[ 9 ] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };
    for ( UInt32 i=0; i<9; ++i )
        AddFloat( binary, normals[ i ] );
    const Float32 uvs[ 6 ] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };
    for ( UInt32 i=0; i<6; ++i )
        AddFloat( binary, uvs[ i ] );
    for ( UInt16 i=0; i<3; ++i )
        AddUInt16( binary, i );
    while ( 0 != binary.size() % 4 )
        binary.push_back( 0 );

    const bool textured = !pictureName.IsNULLOrEmpty();
    CString json = "{ \"asset\": { \"version\": \"2.0\"" + ( generator.IsNULLOrEmpty() ? CString::Empty : ", \"generator\": \"" + generator + "\"" ) + " }, \"scene\": 0, \"scenes\": [ { \"nodes\": [ 0 ] } ],"
                   " \"nodes\": [ { \"name\": \"" + nodeName + "\", \"mesh\": 0 } ],"
                   " \"meshes\": [ { \"primitives\": [ { \"attributes\": { \"POSITION\": 0, \"NORMAL\": 1, \"TEXCOORD_0\": 2 }, \"indices\": 3" + ( textured ? CString( ", \"material\": 0" ) : CString::Empty ) + " } ] } ],";
    if ( textured )
    {
        json += " \"materials\": [ { \"name\": \"Painted\", \"pbrMetallicRoughness\": { \"baseColorTexture\": { \"index\": 0 } } } ],"
                " \"textures\": [ { \"source\": 0 } ], \"images\": [ { \"uri\": \"" + pictureName + "\" } ],";
    }
    json += " \"buffers\": [ { \"byteLength\": " + CORE::ToString( (UInt32) binary.size() ) + ", \"uri\": \"data:application/octet-stream;base64," + Base64( binary ) + "\" } ],"
            " \"bufferViews\": [ { \"buffer\": 0, \"byteOffset\": 0, \"byteLength\": 36 }, { \"buffer\": 0, \"byteOffset\": 36, \"byteLength\": 36 },"
            " { \"buffer\": 0, \"byteOffset\": 72, \"byteLength\": 24 }, { \"buffer\": 0, \"byteOffset\": 96, \"byteLength\": 6 } ],"
            " \"accessors\": [ { \"bufferView\": 0, \"componentType\": 5126, \"count\": 3, \"type\": \"VEC3\", \"min\": [ 0, 0, 0 ], \"max\": [ 1, 1, 0 ] },"
            " { \"bufferView\": 1, \"componentType\": 5126, \"count\": 3, \"type\": \"VEC3\" },"
            " { \"bufferView\": 2, \"componentType\": 5126, \"count\": 3, \"type\": \"VEC2\" },"
            " { \"bufferView\": 3, \"componentType\": 5123, \"count\": 3, \"type\": \"SCALAR\" } ] }";
    return json;
}

/*-------------------------------------------------------------------------*/

bool
WriteGltfPack( const CString& folder, const CString& title, const CString& licenceJson, UInt32 modelCount )
{
    CString packJson = "{ \"title\": \"" + title + "\", \"author\": \"Forge Works\"";
    if ( !licenceJson.IsNULLOrEmpty() )
        packJson += ", \"licence\": " + licenceJson;
    packJson += " }";
    bool written = WriteVfsFile( folder + "/pack.json", packJson );
    CDynamicBuffer png;
    written = MakePng( 16, 16, png ) && WriteVfsFile( folder + "/Models/tex.png", png ) && written;
    for ( UInt32 m=1; m<=modelCount; ++m )
    {
        const CString name = "SM_Tri_0" + CORE::ToString( m );
        written = WriteVfsFile( folder + "/Models/" + name + ".gltf", MakeTriangleGltf( name, "tex.png", title ) ) && written;
    }
    return written;
}

/*-------------------------------------------------------------------------*/

bool
WaitForFuture( const CFutureResult& future, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs; waited+=2 )
    {
        if ( future.IsReady() || future.HasNoFuture() )
            return future.IsReady();
        CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 2 );
    }
    return future.IsReady();
}

/*-------------------------------------------------------------------------*/

bool
GetPostgreSqlConnectionString( CString& connectionString )
{
    const char* value = getenv( "GUCEF_TEST_POSTGRESQL" );
    if ( GUCEF_NULL == value || '\0' == value[ 0 ] )
        return false;
    connectionString = value;
    return true;
}

/*-------------------------------------------------------------------------*/

CString
MakeTestSchemaName( void )
{
    return "gucef_assetstore_hook_test_" + ToString( (UInt32) MT::GetProcessID() ) + "_" + ToString( (UInt32) ( MT::PrecisionTickCount() % 1000000 ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace HOOKTEST */

/*-------------------------------------------------------------------------*/
