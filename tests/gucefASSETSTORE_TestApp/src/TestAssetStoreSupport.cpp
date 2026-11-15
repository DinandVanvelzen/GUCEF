/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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
#ifndef GUCEF_CORE_CSHA1_H
#include "gucefCORE_CSha1.h"
#define GUCEF_CORE_CSHA1_H
#endif /* GUCEF_CORE_CSHA1_H ? */
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
#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */
#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETDIGEST_H
#include "gucefASSETSTORE_CAssetDigest.h"
#define GUCEF_ASSETSTORE_CASSETDIGEST_H
#endif /* GUCEF_ASSETSTORE_CASSETDIGEST_H ? */
#ifndef DBPLUGIN_POSTGRESQL_H
#include "dbpluginPOSTGRESQL.h"
#define DBPLUGIN_POSTGRESQL_H
#endif /* DBPLUGIN_POSTGRESQL_H ? */
#include "TestAssetStoreSupport.h"

using namespace GUCEF;
using namespace GUCEF::CORE;

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

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
    if ( pluginControl.AddPluginFromDir( first, "AssetStoreTestPlugins", true ) )
        return true;
    return pluginControl.AddPluginFromDir( second, "AssetStoreTestPlugins", true );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      FUNCTIONS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
SetUpTestEnvironment( void )
{
    AddPlugin( "dstorepluginJSONPARSER" );
    AddPlugin( "codecspluginZLIB" );
    AddPlugin( "tablecodecpluginPARQUET" );
    AddPlugin( "dbpluginVFSTABLES" );
    // the sealing of the credential store and the web socket of the DevTools client
    AddPlugin( "codecspluginMONOCYPHER" );
    AddPlugin( "codecspluginSTBRUMMEHASH" );

    // the postgresql backend is linked: its plugin entry point registers it
    if ( !COMCORE::CComCoreGlobal::Instance()->GetDatabaseClientFactory().IsConstructible( "postgresql" ) )
        GUCEFPlugin_Load( 0, GUCEF_NULL );

    VFS::CVfsGlobal::Instance()->GetVfs().MountAllDelayMountedArchives();
}

/*-------------------------------------------------------------------------*/

void
TearDownTestEnvironment( void )
{
    // the plugins go while everything they use is still there, not in the handlers at exit
    CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "AssetStoreTestPlugins" );
}

/*-------------------------------------------------------------------------*/

CString
TestDirectory( const CString& name )
{
    return CombinePath( CombinePath( TempDir(), "gucefASSETSTORE_TestApp" ), name );
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
    vfs.UnmountArchiveByName( "AssetStoreTest_" + name );
    vfs.AddRoot( dir, "AssetStoreTest_" + name, true, false, mountPoint );
    return dir;
}

/*-------------------------------------------------------------------------*/

void
UnmountDirectory( const CString& name )
{
    VFS::CVfsGlobal::Instance()->GetVfs().UnmountArchiveByName( "AssetStoreTest_" + name );
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
PumpUntilJobSettled( ASSETSTORE::CAssetStoreDownloadManager& manager, const CString& jobId, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs; waited+=2 )
    {
        CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        if ( manager.WaitForJob( jobId, 0 ) )
        {
            PumpFor( 20 );
            return true;
        }
        MT::ThreadDelay( 2 );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
PumpUntilJobState( ASSETSTORE::CAssetStoreDownloadManager& manager, const CString& jobId, ASSETSTORE::TAssetDownloadJobState state, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs; waited+=2 )
    {
        CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        ASSETSTORE::CAssetStoreDownloadJob job;
        if ( manager.GetJob( jobId, job ) && job.state == state )
            return true;
        MT::ThreadDelay( 2 );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
PumpUntilJobBytes( ASSETSTORE::CAssetStoreDownloadManager& manager, const CString& jobId, UInt64 bytes, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs; waited+=2 )
    {
        CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        ASSETSTORE::CAssetStoreDownloadJob job;
        if ( manager.GetJob( jobId, job ) && job.GetBytesDone() >= bytes )
            return true;
        MT::ThreadDelay( 2 );
    }
    return false;
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

void
MakePattern( UInt32 size, UInt32 seed, CDynamicBuffer& content )
{
    content.Clear();
    content.SetBufferSize( size > 0 ? size : 1 );
    UInt32 state = seed * 2654435761u + 1;
    UInt8* bytes = static_cast< UInt8* >( content.GetBufferPtr() );
    for ( UInt32 i=0; i<size; ++i )
    {
        state = state * 1103515245u + 12345u;
        bytes[ i ] = (UInt8) ( state >> 16 );
    }
    content.SetDataSize( size );
}

/*-------------------------------------------------------------------------*/

bool
MakePng( UInt32 width, UInt32 height, bool normalMapColour, CDynamicBuffer& content )
{
    GUCEF::vector< UInt8 > pixels( width * height * 3 );
    for ( UInt32 p=0; p<width*height; ++p )
    {
        pixels[ p*3 ] = normalMapColour ? 128 : (UInt8) ( p & 0xFF );
        pixels[ p*3+1 ] = normalMapColour ? 128 : (UInt8) ( ( p >> 3 ) & 0xFF );
        pixels[ p*3+2 ] = normalMapColour ? 255 : 40;
    }
    IMAGE::TPixelMapPtr pixelMap( GUCEF_NEW IMAGE::CPixelMap( &pixels[ 0 ], width, height, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8 ) );
    IMAGE::CImage image( pixelMap );
    content.Clear();
    CDynamicBufferAccess access( content );
    return image.Save( access, "png" ) && content.GetDataSize() > 0;
}

/*-------------------------------------------------------------------------*/

void
MakeTga( UInt32 width, UInt32 height, CDynamicBuffer& content )
{
    content.Clear();
    UInt8 header[ 18 ] = { 0 };
    header[ 2 ] = 2;                                // uncompressed true colour
    header[ 12 ] = (UInt8) ( width & 0xFF );
    header[ 13 ] = (UInt8) ( width >> 8 );
    header[ 14 ] = (UInt8) ( height & 0xFF );
    header[ 15 ] = (UInt8) ( height >> 8 );
    header[ 16 ] = 32;
    header[ 17 ] = 8;
    content.Append( header, sizeof( header ) );
    GUCEF::vector< UInt8 > pixels( width * height * 4, 200 );
    content.Append( &pixels[ 0 ], (UInt32) pixels.size() );
}

/*-------------------------------------------------------------------------*/

CString
Sha256Of( const CDynamicBuffer& content )
{
    ASSETSTORE::CAssetDigest digest( ASSETSTORE::CAssetDigest::Sha256 );
    digest.Update( content.GetConstBufferPtr(), content.GetDataSize() );
    return digest.FinalHex();
}

/*-------------------------------------------------------------------------*/

CString
Sha1Of( const CDynamicBuffer& content )
{
    return CSha1::HashHex( content.GetConstBufferPtr(), content.GetDataSize() );
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
    return "gucef_assetstore_test_" + ToString( (UInt32) MT::GetProcessID() ) + "_" + ToString( (UInt32) ( MT::PrecisionTickCount() % 1000000 ) );
}

/*-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/
