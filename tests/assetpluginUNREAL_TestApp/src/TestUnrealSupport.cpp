/*
 *  assetpluginUNREAL_TestApp: Support of the tests of the Unreal package reader: plugins, work folder, golden files
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

#include <stdlib.h>
#include <string.h>

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "TestUnrealSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETPLUGIN::UNREAL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const TestPluginGroup = "assetpluginUNREAL_TestApp";
static const char* const WorkMountPoint = "work";
static const char* const WorkArchiveName = "assetpluginUNREAL_TestApp/work";
static CORE::CString g_workRoot;

/*-------------------------------------------------------------------------*/

static bool
LoadTestPlugin( const char* moduleName )
{
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    const CORE::CString path = CORE::CString( "$MODULEDIR$/" ) + moduleName;
    return pluginControl.AddPluginFromDir( path + "_d", TestPluginGroup, true ) || pluginControl.AddPluginFromDir( path, TestPluginGroup, true );
}

/*-------------------------------------------------------------------------*/

CORE::CString
HomeDir( void )
{GUCEF_TRACE;

    const char* home = getenv( "HOME" );
    return GUCEF_NULL != home ? CORE::CString( home ) : CORE::CString( "." );
}

/*-------------------------------------------------------------------------*/

CORE::CString
GoldenDir( void )
{GUCEF_TRACE;

    return CORE::CombinePath( CORE::CombinePath( CORE::CombinePath( HomeDir(), "AssetTests" ), "unreal" ), "golden" );
}

/*-------------------------------------------------------------------------*/

/** The locations of the local corpus, from the manifest in the scratch area of the user: lines of key=value */
CORE::CString
CorpusSetting( const CORE::CString& key )
{
    CORE::CDynamicBuffer manifest;
    if ( !manifest.LoadContentFromFile( CORE::CombinePath( CORE::CombinePath( CORE::CombinePath( HomeDir(), "AssetTests" ), "unreal" ), "corpus.txt" ) ) )
        return CORE::CString();
    const CORE::CString text( static_cast< const char* >( manifest.GetConstBufferPtr() ), manifest.GetDataSize() );
    const CORE::CString::StringVector lines = text.ParseElements( '\n', false );
    for ( size_t i=0; i<lines.size(); ++i )
    {
        const Int32 equals = lines[ i ].HasChar( '=' );
        if ( equals > 0 && lines[ i ].SubstrFromRange( 0, (UInt32) equals ) == key )
            return lines[ i ].SubstrFromRange( (UInt32) equals + 1, lines[ i ].Length() ).Trim( false ).Trim( true );
    }
    return CORE::CString();
}

/*-------------------------------------------------------------------------*/

bool
CorpusEnabled( void )
{GUCEF_TRACE;

    const char* value = getenv( "GUCEF_TEST_ASSET_CORPUS" );
    return GUCEF_NULL != value && 0 == strcmp( value, "1" );
}

/*-------------------------------------------------------------------------*/

bool
SetUpUnrealTests( void )
{GUCEF_TRACE;

    VFS::CVfsGlobal::Instance();
    ASSETCONVERTER::CAssetConverterGlobal::Instance();

    // zip archives for the archives of the corpus, then the plugin under test
    // libarchive reads the .mhpkg archives (64 bit zip); it is loaded first so that the zip plugin keeps the type "zip"
    bool loaded = LoadTestPlugin( "vfspluginLIBARCHIVE" );
    loaded = LoadTestPlugin( "vfspluginZIP" ) && loaded;
    loaded = LoadTestPlugin( "assetpluginUNREAL" ) && loaded;
    // the zlib codec inflates the chunked payloads of the UE 4 packages
    loaded = LoadTestPlugin( "codecspluginZLIB" ) && loaded;
    if ( !loaded )
        return false;

    // the work folder is in the scratch area of the user, not in /tmp: some tests write packages of tens of megabytes; the name
    // can be set (GUCEF_TEST_UNREAL_WORK) so that several runs at the same time do not share it
    g_workRoot = CORE::CombinePath( CORE::CombinePath( CORE::CombinePath( HomeDir(), "AssetTests" ), "unreal" ), CORE::CString( GUCEF_NULL != getenv( "GUCEF_TEST_UNREAL_WORK" ) ? getenv( "GUCEF_TEST_UNREAL_WORK" ) : "work" ) );
    if ( CORE::DirExists( g_workRoot ) )
        CORE::DeleteDir( g_workRoot, true );
    CORE::CreateDirs( g_workRoot );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.MountArchive( WorkArchiveName, g_workRoot, VFS::CVFS::FileSystemArchiveTypeName, WorkMountPoint, true, false, g_workRoot ) )
        return false;

    ASSETCONVERTER::TAssetImporterPtr importer;
    return ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "unreal", importer );
}

/*-------------------------------------------------------------------------*/

void
TearDownUnrealTests( void )
{GUCEF_TRACE;

    VFS::CVfsGlobal::Instance()->GetVfs().UnmountArchiveByName( WorkArchiveName );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( TestPluginGroup );
    if ( !g_workRoot.IsNULLOrEmpty() && CORE::DirExists( g_workRoot ) )
        CORE::DeleteDir( g_workRoot, true );
}

/*-------------------------------------------------------------------------*/

CORE::CString
WorkDiskPath( const CORE::CString& relativePath )
{GUCEF_TRACE;

    return CORE::CombinePath( g_workRoot, relativePath );
}

/*-------------------------------------------------------------------------*/

CORE::CString
WorkVfsPath( const CORE::CString& relativePath )
{GUCEF_TRACE;

    return CORE::CString( WorkMountPoint ) + "/" + relativePath;
}

/*-------------------------------------------------------------------------*/

bool
WriteWorkFile( const CORE::CString& relativePath, const GUCEF::vector< UInt8 >& bytes )
{GUCEF_TRACE;

    const CORE::CString path = WorkDiskPath( relativePath );
    CORE::CreateDirs( CORE::StripFilename( path ) );
    CORE::CFileAccess file( path, "wb" );
    if ( !file.IsValid() )
        return false;
    UInt64 offset = 0;
    while ( offset < bytes.size() )
    {
        const UInt32 chunk = bytes.size() - offset > 0x10000000 ? 0x10000000 : (UInt32) ( bytes.size() - offset );
        if ( file.Write( &bytes[ (size_t) offset ], 1, chunk ) != chunk )
            return false;
        offset += chunk;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
DeleteWorkFile( const CORE::CString& relativePath )
{GUCEF_TRACE;

    CORE::DeleteFile( WorkDiskPath( relativePath ) );
}

/*-------------------------------------------------------------------------*/

bool
CheckGolden( const CORE::CString& name, const CORE::CString& text, CORE::CString& problem )
{GUCEF_TRACE;

    problem.Clear();
    const CORE::CString path = CORE::CombinePath( GoldenDir(), name + ".txt" );
    if ( !CORE::FileExists( path ) )
    {
        CORE::CreateDirs( GoldenDir() );
        CORE::CFileAccess file( path, "wb" );
        if ( !file.IsValid() || text.ByteSize() == 0 )
        {
            problem = "The golden file " + path + " cannot be created";
            return false;
        }
        file.Write( text.C_String(), 1, text.ByteSize() - 1 );
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Golden file created: " << path );
        return true;
    }
    CORE::CDynamicBuffer golden;
    if ( !golden.LoadContentFromFile( path ) )
    {
        problem = "The golden file " + path + " cannot be read";
        return false;
    }
    const CORE::CString expected( static_cast< const char* >( golden.GetConstBufferPtr() ), golden.GetDataSize() );
    if ( expected == text )
        return true;

    // the first line that differs
    const CORE::CString::StringVector a = expected.ParseElements( '\n', true );
    const CORE::CString::StringVector b = text.ParseElements( '\n', true );
    for ( size_t i=0; i<a.size() || i<b.size(); ++i )
    {
        const CORE::CString left = i < a.size() ? a[ i ] : CORE::CString( "<end>" );
        const CORE::CString right = i < b.size() ? b[ i ] : CORE::CString( "<end>" );
        if ( left != right )
        {
            problem = "Golden " + name + " line " + CORE::UInt64ToString( i + 1 ) + ": expected \"" + left + "\" found \"" + right + "\"";
            break;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/

CNoOodleToolScope::CNoOodleToolScope( void )
    : m_had( false )
    , m_old()
{GUCEF_TRACE;

    const char* old = getenv( "GUCEF_OOZ_EXTRACT_PATH" );
    m_had = GUCEF_NULL != old;
    if ( m_had )
        m_old = old;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    _putenv_s( "GUCEF_OOZ_EXTRACT_PATH", "no-such-tool-for-the-tests" );
    #else
    setenv( "GUCEF_OOZ_EXTRACT_PATH", "/no/such/tool/for/the/tests/ooz_extract", 1 );
    #endif
}

/*-------------------------------------------------------------------------*/

CNoOodleToolScope::~CNoOodleToolScope()
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    _putenv_s( "GUCEF_OOZ_EXTRACT_PATH", m_had ? m_old.C_String() : "" );
    #else
    if ( m_had )
        setenv( "GUCEF_OOZ_EXTRACT_PATH", m_old.C_String(), 1 );
    else
        unsetenv( "GUCEF_OOZ_EXTRACT_PATH" );
    #endif
}
