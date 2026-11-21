/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCENAME_H
#include "gucefDRN_CDRNResourceName.h"
#define GUCEF_DRN_CDRNRESOURCENAME_H
#endif /* GUCEF_DRN_CDRNRESOURCENAME_H ? */

#include "gucefDRN_CDRNFileSystemResourceProvider.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <limits.h>
  #include <stdlib.h>
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

static UInt64
ModificationTimeInMs( const struct stat& info )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    return (UInt64) info.st_mtim.tv_sec * 1000 + (UInt64) info.st_mtim.tv_nsec / 1000000;
    #else
    return (UInt64) info.st_mtime * 1000;
    #endif
}

/*-------------------------------------------------------------------------*/

static CORE::CString
RealPathOf( const CORE::CString& path )
{GUCEF_TRACE;

    char resolved[ PATH_MAX+1 ];
    if ( GUCEF_NULL == ::realpath( path.C_String(), resolved ) )
        return CORE::CString::Empty;
    return CORE::CString( resolved );
}

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNFileSystemResourceProvider::CDRNFileSystemResourceProvider( const CORE::CString& rootDirectory, bool followSymlinks )
    : CIDRNResourceProvider()
    , m_root( rootDirectory )
    , m_followSymlinks( followSymlinks )
{GUCEF_TRACE;

    // no separator at the end, so that a name is always joined with exactly one
    while ( m_root.Length() > 1 && ( m_root.EndsWith( "/" ) || m_root.EndsWith( "\\" ) ) )
        m_root = m_root.CutChars( 1, false, 0 );
}

/*-------------------------------------------------------------------------*/

CDRNFileSystemResourceProvider::~CDRNFileSystemResourceProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNFileSystemResourceProvider::GetRootDirectory( void ) const
{GUCEF_TRACE;

    return m_root;
}

/*-------------------------------------------------------------------------*/

bool
CDRNFileSystemResourceProvider::GetFollowSymlinks( void ) const
{GUCEF_TRACE;

    return m_followSymlinks;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNFileSystemResourceProvider::GetDescription( void ) const
{GUCEF_TRACE;

    return "filesystem:" + m_root;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNFileSystemResourceProvider::GetPathOf( const CORE::CString& name ) const
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return m_root + "\\" + name.ReplaceChar( '/', '\\' );
    #else
    return m_root + "/" + name;
    #endif
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNFileSystemResourceProvider::Inspect( const CORE::CString& name, CORE::CString& path, UInt64& size, UInt64& modifiedUtcMs ) const
{GUCEF_TRACE;

    size = 0;
    modifiedUtcMs = 0;
    path.Clear();
    if ( !CDRNResourceName::IsValidResourceName( name ) )
        return DRN_RESSTATUS_INVALID_NAME;     // checked by the facility already, and here again: this is what keeps the root
    path = GetPathOf( name );
    CORE::CString::StringVector segments = name.ParseElements( '/', false );

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    // every step below the root: a link is refused unless links are followed
    CORE::CString walked = m_root;
    for ( UInt32 i=0; i<segments.size(); ++i )
    {
        walked += "/" + segments[ i ];
        struct stat linkInfo;
        if ( 0 != ::lstat( walked.C_String(), &linkInfo ) )
            return DRN_RESSTATUS_NOT_FOUND;
        if ( S_ISLNK( linkInfo.st_mode ) && !m_followSymlinks )
            return DRN_RESSTATUS_NOT_FOUND;
    }

    struct stat info;
    if ( 0 != ::stat( path.C_String(), &info ) || !S_ISREG( info.st_mode ) )
        return DRN_RESSTATUS_NOT_FOUND;

    if ( m_followSymlinks )
    {
        CORE::CString realRoot = RealPathOf( m_root );
        CORE::CString realPath = RealPathOf( path );
        if ( realRoot.IsNULLOrEmpty() || realPath.IsNULLOrEmpty() )
            return DRN_RESSTATUS_NOT_FOUND;
        if ( !realRoot.EndsWith( "/" ) )
            realRoot += "/";
        if ( !realPath.StartsWith( realRoot ) )
            return DRN_RESSTATUS_NOT_FOUND;
    }

    size = (UInt64) info.st_size;
    modifiedUtcMs = ModificationTimeInMs( info );
    return DRN_RESSTATUS_OK;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // links and junctions are reparse points: never passed, whatever m_followSymlinks says
    CORE::CString walked = m_root;
    WIN32_FILE_ATTRIBUTE_DATA data;
    for ( UInt32 i=0; i<segments.size(); ++i )
    {
        walked += "\\" + segments[ i ];
        std::wstring widePath = CORE::ToWString( walked );
        if ( 0 == ::GetFileAttributesExW( widePath.c_str(), GetFileExInfoStandard, &data ) )
            return DRN_RESSTATUS_NOT_FOUND;
        if ( 0 != ( data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) )
            return DRN_RESSTATUS_NOT_FOUND;
    }
    if ( segments.empty() || 0 != ( data.dwFileAttributes & ( FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE ) ) )
        return DRN_RESSTATUS_NOT_FOUND;

    size = ( (UInt64) data.nFileSizeHigh << 32 ) | (UInt64) data.nFileSizeLow;
    UInt64 fileTime = ( (UInt64) data.ftLastWriteTime.dwHighDateTime << 32 ) | (UInt64) data.ftLastWriteTime.dwLowDateTime;
    static const UInt64 UnixEpochAsFileTime = 116444736000000000ULL;
    modifiedUtcMs = fileTime > UnixEpochAsFileTime ? ( fileTime - UnixEpochAsFileTime ) / 10000 : 0;
    return DRN_RESSTATUS_OK;

    #else

    (void) segments;
    return DRN_RESSTATUS_NOT_FOUND;

    #endif
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNFileSystemResourceProvider::Resolve( const CORE::CString& name, CDRNResourceInfo& info )
{GUCEF_TRACE;

    info.Clear();
    CORE::CString path;
    UInt64 size = 0;
    UInt64 modifiedUtcMs = 0;
    TDRNResourceStatus status = Inspect( name, path, size, modifiedUtcMs );
    if ( DRN_RESSTATUS_OK != status )
        return status;
    info.name = name;
    info.size = size;
    info.modifiedUtcMs = modifiedUtcMs;
    info.revision = CDRNResourceInfo::MakeRevision( size, modifiedUtcMs );
    return DRN_RESSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNFileSystemResourceProvider::Read( const CORE::CString& name            ,
                                      const CDRNResourceInfo& resolved     ,
                                      UInt64 offset                        ,
                                      UInt32 size                          ,
                                      void* destination                    ,
                                      UInt32& bytesRead                    )
{GUCEF_TRACE;

    bytesRead = 0;
    CORE::CString path;
    UInt64 currentSize = 0;
    UInt64 modifiedUtcMs = 0;
    TDRNResourceStatus status = Inspect( name, path, currentSize, modifiedUtcMs );
    if ( DRN_RESSTATUS_NOT_FOUND == status )
        return DRN_RESSTATUS_CHANGED;
    if ( DRN_RESSTATUS_OK != status )
        return status;
    if ( currentSize != resolved.size || CDRNResourceInfo::MakeRevision( currentSize, modifiedUtcMs ) != resolved.revision )
        return DRN_RESSTATUS_CHANGED;
    if ( offset > currentSize )
        return DRN_RESSTATUS_OUT_OF_RANGE;
    if ( 0 == size || offset == currentSize )
        return DRN_RESSTATUS_OK;

    CORE::CFileAccess file;
    if ( !file.Open( path, "rb" ) )
        return DRN_RESSTATUS_IO_ERROR;
    if ( 0 != file.Setpos( offset ) )
        return DRN_RESSTATUS_IO_ERROR;
    UInt64 left = currentSize - offset;
    UInt32 toRead = left < size ? (UInt32) left : size;
    bytesRead = file.Read( destination, 1, toRead );
    file.Close();
    return bytesRead == toRead ? DRN_RESSTATUS_OK : DRN_RESSTATUS_IO_ERROR;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
