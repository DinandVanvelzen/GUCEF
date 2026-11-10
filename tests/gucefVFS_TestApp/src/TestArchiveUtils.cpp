/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#include "TestArchiveUtils.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace TestArchiveUtils {

/*-------------------------------------------------------------------------*/

bool
LargeFileTestsEnabled( void )
{GUCEF_TRACE;

    const char* value = CORE::GUCEFGetEnv( "GUCEF_TEST_LARGE_FILES" );
    return GUCEF_NULL != value && '\0' != value[ 0 ] && 0 != strcmp( value, "0" );
}

/*-------------------------------------------------------------------------*/

bool
LoadVfsPlugin( const CORE::CString& pluginName )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        return true;
    return pluginControl.AddPluginFromDir( secondPluginPath, "test", true );
}

/*-------------------------------------------------------------------------*/

CZeroSource::CZeroSource( UInt64 size, bool sizeKnown )
    : CORE::CIOAccess()
    , m_size( size )
    , m_pos( 0 )
    , m_sizeKnown( sizeKnown )
{GUCEF_TRACE;
}

void CZeroSource::Open( void ) { m_pos = 0; }
void CZeroSource::Close( void ) {}
bool CZeroSource::Opened( void ) const { return true; }

UInt32
CZeroSource::Read( void* dest, UInt32 esize, UInt32 elements )
{
    UInt64 wanted = (UInt64) esize * elements;
    UInt64 left = m_size - m_pos;
    UInt64 count = wanted < left ? wanted : left;
    memset( dest, 0, (size_t) count );
    m_pos += count;
    return esize > 0 ? (UInt32) ( count / esize ) : 0;
}

UInt64 CZeroSource::Tell( void ) const { return m_pos; }
Int32 CZeroSource::Seek( Int64 offset, Int32 origin ) { return -1; }
char CZeroSource::GetChar( void ) { char c = 0; Read( &c, 1, 1 ); return c; }
bool CZeroSource::Eof( void ) const { return m_pos >= m_size; }
bool CZeroSource::IsReadable( void ) const { return true; }
bool CZeroSource::IsWriteable( void ) const { return false; }
bool CZeroSource::IsValid( void ) { return true; }
void CZeroSource::Flush( void ) {}
UInt64 CZeroSource::GetSize( void ) const { return m_sizeKnown ? m_size : 0; }
MT::CICloneable* CZeroSource::Clone( void ) const { return GUCEF_NEW CZeroSource( m_size, m_sizeKnown ); }

/*-------------------------------------------------------------------------*/

Int32
RunExternalProgram( const CORE::CString& exe, const CORE::CProcess::TStringVector& args, UInt32 timeoutInMs )
{GUCEF_TRACE;

    CORE::CProcess process;
    if ( !process.Launch( exe, args ) )
        return -1;
    Int32 exitCode = -1;
    if ( !process.WaitForExit( timeoutInMs, exitCode ) )
    {
        process.Terminate( false );
        return -1;
    }
    return exitCode;
}

/*-------------------------------------------------------------------------*/

bool
IsToolAvailable( const CORE::CString& exe, const CORE::CString& versionArg )
{GUCEF_TRACE;

    CORE::CProcess::TStringVector args;
    args.push_back( versionArg );
    return 0 == RunExternalProgram( exe, args, 30000 );
}

/*-------------------------------------------------------------------------*/

bool
RunPythonCheck( const CORE::CString& scriptPath, const CORE::CProcess::TStringVector& args )
{GUCEF_TRACE;

    if ( !IsToolAvailable( "python3", "--version" ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Archive tests: python3 not available, skipping the check by " + scriptPath );
        return true;
    }
    CORE::CProcess::TStringVector allArgs;
    allArgs.push_back( "-I" );
    allArgs.push_back( scriptPath );
    allArgs.insert( allArgs.end(), args.begin(), args.end() );
    return 0 == RunExternalProgram( "python3", allArgs );
}

/*-------------------------------------------------------------------------*/

bool
ReadVfsFile( VFS::CVFS& vfs, const CORE::CString& path, CORE::CDynamicBuffer& content )
{GUCEF_TRACE;

    VFS::TBasicVfsResourcePtr file = vfs.GetFile( path, "rb", false );
    if ( file.IsNULL() )
        return false;
    CORE::IOAccessPtr access = file->GetAccess();
    if ( access.IsNULL() )
        return false;
    content.Clear();
    return content.Append( *access ) == access->GetSize();
}

/*-------------------------------------------------------------------------*/

}; /* namespace TestArchiveUtils */
