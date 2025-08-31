/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <malloc.h>             /* memory management */
#include <stdio.h>              /* standard I/O utils */
#include <string.h>             /* standard string utils */
#include <assert.h>
#include <limits.h>
#include <fstream>
#include <sstream>

#ifndef GUCEF_CORE_CONFIG_H
#include "gucefCORE_config.h"
#define GUCEF_CORE_CONFIG_H
#endif /* GUCEF_CORE_CONFIG_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVSTRUTILS_H
#include "dvstrutils.h"
#define GUCEF_CORE_DVSTRUTILS_H
#endif /* GUCEF_CORE_DVSTRUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

  #ifndef GUCEF_CORE_MSWINUTILS_H
  #include "gucefCORE_mswinutils.h"
  #define GUCEF_CORE_MSWINUTILS_H
  #endif /* GUCEF_CORE_MSWINUTILS_H ? */

  #ifndef GUCEF_CORE_CPPMSWINUTILS_H
  #include "gucefCORE_cppmswinutils.h"
  #define GUCEF_CORE_CPPMSWINUTILS_H
  #endif /* GUCEF_CORE_CPPMSWINUTILS_H ? */

  #include <windows.h>		/* WIN32 API */
  #include <winioctl.h>
  #include <Shlobj.h>
  #undef min
  #undef max
  /* #include <dir.h>: obsolete *//* needed for MAXFILE define */
  #include <io.h>                 /* Dir itteration: findfirst ect. */
  #include <direct.h>             /* dir tools */
  #include "Shlwapi.h"
  #define MAX_DIR_LENGTH MAX_PATH
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #include <dirent.h>             /* needed for dirent strcture */
  #include <unistd.h>             /* POSIX utilities */
  #include <limits.h>             /* Linux OS limits */
  #include <sys/stat.h>           /* needed for stat function */
  #include <sys/statvfs.h>
  #include <errno.h>
  #include <fcntl.h>
  #define MAX_DIR_LENGTH PATH_MAX
#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
  #include <dirent.h>  // Limited support in Emscripten
  #include <unistd.h>  // Limited support in Emscripten
  #include <limits.h>
  #include <sys/stat.h>
  #include <errno.h>
  #include <fcntl.h>
  #define MAX_DIR_LENGTH PATH_MAX
#else
  #error Unsupported OS
#endif

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
  #include <android/log.h>
#endif

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#include "dvcppfileutils.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Small helper class to hold the O/S specific data needed to repeatedly access the storage device
 *  Avoid exposing the O/S specific data objects to the public API
 */
class GUCEF_HIDDEN CStorageDeviceInfoOSDataImpl
{
    public:

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    CPdhAccess::CPdhDevicePerfStatAccess devicePerfStatAccess;
    
    #endif

    CStorageDeviceInfoOSDataImpl( void )
    {GUCEF_TRACE;
    }
    ~CStorageDeviceInfoOSDataImpl()
    {GUCEF_TRACE;
    }
};

/*--------------------------------------------------------------------------*/

/**
 *  Small helper class to hold the O/S specific data needed to repeatedly access the storage volume
 *  Avoid exposing the O/S specific data objects to the public API
 */
class GUCEF_HIDDEN CStorageVolumeInfoOSDataImpl
{
    public:

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    CPdhAccess::CPdhVolumePerfStatAccess volumePerfStatAccess;
    
    #endif

    CStorageVolumeInfoOSDataImpl( void )
    {GUCEF_TRACE;
    }
    ~CStorageVolumeInfoOSDataImpl()
    {GUCEF_TRACE;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

bool GUCEF_HIDDEN
TryToDetermineIfRunningInContainer( void )
{GUCEF_TRACE;

    // in a containerized environment we'd expect to see certain things
    // we leverage this to try and make the determination
    // do note that this code may need to evolve as the container landscape evolves
    // for now we aim for the code to support Docker and containerd

    CString cgroupInfo;
    if ( LoadTextFileAsString( "/proc/1/cgroup", cgroupInfo, true, "\n" ) )
    {
        bool hasContainerdKeyword =
                -1 < cgroupInfo.HasSubstr( "/pod", true, true ) ||
                -1 < cgroupInfo.HasSubstr( "/kube", true, true );

        if ( hasContainerdKeyword )
            return true;

        bool hasDockerKeyword =
            -1 < cgroupInfo.HasSubstr( "/docker", true, true );

        if ( hasDockerKeyword )
            return true;
    }

    return false;
}

/*--------------------------------------------------------------------------*/

bool GUCEF_HIDDEN
ParseFakeUUID( const CString& fakeUuid ,
               CString& deviceName     ,
               bool& hasVersionInfo    ,
               int& majorVersion       ,
               int& minorVersion       )
{GUCEF_TRACE;

    deviceName.Clear();
    hasVersionInfo = false;
    majorVersion = 0;
    minorVersion = 0;

    if ( fakeUuid.StartsWith( "fakeuuid:" ) )
    {
        CString remnant = fakeUuid.CutChars( 9, true, 0 );
        Int32 underscoreOffset = remnant.HasChar( '_', 0, true );
        if ( underscoreOffset < 0 )
        {
            deviceName = remnant;
            return true;
        }

        Int32 colonOffset = remnant.HasChar( ':', (UInt32) underscoreOffset, true );
        if ( colonOffset > 0 )
        {
            majorVersion = StringToInt32( remnant.SubstrFromRange( underscoreOffset+1, colonOffset ), 0 );
            minorVersion = StringToInt32( remnant.SubstrToIndex( colonOffset+1, false ), 0 );
            hasVersionInfo = true;
            return true;
        }
    }
    return false;
}

#endif

/*--------------------------------------------------------------------------*/

bool
TryResolveSpecialDir( TSpecialDirs dir, CString& resolvedPath )
{GUCEF_TRACE;

    resolvedPath.Clear();

    // Some we can resolve by redirecting to dedicated functions we have
    switch ( dir )
    {
        case SPECIALDIR_CURRENT_WORK_DIR:
            resolvedPath = CORE::CurrentWorkingDir();
            return true;
        case SPECIALDIR_UP_DIR:
            resolvedPath = "..";
            return true;
        case SPECIALDIR_MODULE_DIR:
            resolvedPath = CORE::ModuleDir();
            return true;
        case SPECIALDIR_TEMP_DIR:
            resolvedPath = CORE::TempDir();
            return true;
        default:
        {
            // handled below, no action here on purpose
        }
    }

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    KNOWNFOLDERID folderId;
    switch ( dir )
    {
        case SPECIALDIR_HOME_DIR:
            folderId = ::FOLDERID_Profile;
            break;
        case SPECIALDIR_LOCAL_APPDATA_DIR:
            folderId = ::FOLDERID_LocalAppData;
            break;
        case SPECIALDIR_APPDATA_DIR:
            folderId = FOLDERID_RoamingAppData;
            break;
        case SPECIALDIR_INTERNET_CACHE_DIR:
            folderId = FOLDERID_InternetCache;
            break;
        case SPECIALDIR_DESKTOP_DIR:
            folderId = FOLDERID_Desktop;
            break;
        case SPECIALDIR_MY_DOCUMENTS_DIR:
            folderId = FOLDERID_Documents;
            break;
        case SPECIALDIR_MY_DOWNLOADS_DIR:
            folderId = FOLDERID_Downloads;
            break;
        case SPECIALDIR_MY_MUSIC_DIR:
            folderId = FOLDERID_Music;
            break;
        case SPECIALDIR_MY_PICTURES_DIR:
            folderId = FOLDERID_Pictures;
            break;
        case SPECIALDIR_MY_VIDEO_DIR:
            folderId = FOLDERID_Videos;
            break;
        case SPECIALDIR_MY_GAMES_DIR:
            folderId = FOLDERID_Games;
            break;
        case SPECIALDIR_PROGRAMS_DIR:
            folderId = FOLDERID_Programs;
            break;
        case SPECIALDIR_X86_PROGRAMS_DIR:
            folderId = FOLDERID_ProgramFilesX86;
            break;
        case SPECIALDIR_X64_PROGRAMS_DIR:
            folderId = FOLDERID_ProgramFilesX64;
            break;
        case SPECIALDIR_RECENT_DIR:
            folderId = FOLDERID_Recent;
            break;
        case SPECIALDIR_SYSTEM_DIR:
            folderId = FOLDERID_System;
            break;
        case SPECIALDIR_USER_DIR:
            folderId = FOLDERID_UserProfiles;
            break;

        default:
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "TryResolveSpecialDir: Unsupported special directory" );
            return false;
    }

    WCHAR* path = GUCEF_NULL;
    if ( S_OK != ::SHGetKnownFolderPath( folderId, 0, NULL, &path ) )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "TryResolveSpecialDir: SHGetKnownFolderPath failed" );
        return false;
    }

    resolvedPath = ToString( path );
    ::CoTaskMemFree( path );
    return true;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    const char* path = GUCEF_NULL;
    switch ( dir )
    {
        case SPECIALDIR_HOME_DIR:
            path = getenv( "HOME" );
            break;
        case SPECIALDIR_TEMP_DIR:
            path = getenv( "TMPDIR" );
            if ( GUCEF_NULL == path )
                path = getenv( "TMP" );
            if ( GUCEF_NULL == path )
                path = getenv( "TEMP" );
            break;
        case SPECIALDIR_APPDATA_DIR:
            path = getenv( "XDG_DATA_HOME" );
            if ( GUCEF_NULL == path )
                path = getenv( "HOME" );
            break;
//        case SPECIALDIR_CONFIG_DIR:
//            path = getenv( "XDG_CONFIG_HOME" );
//            if ( GUCEF_NULL == path )
//                path = getenv( "HOME" );
//            break;
        case SPECIALDIR_INTERNET_CACHE_DIR:
            path = getenv( "XDG_CACHE_HOME" );
            if ( GUCEF_NULL == path )
                path = getenv( "HOME" );
            break;
        case SPECIALDIR_DESKTOP_DIR:
            path = getenv( "XDG_DESKTOP_DIR" );
            if ( GUCEF_NULL == path )
                path = getenv( "HOME" );
            break;
        case SPECIALDIR_MY_DOCUMENTS_DIR:
            path = getenv( "XDG_DOCUMENTS_DIR" );
            if ( GUCEF_NULL == path )
                path = getenv( "HOME" );
            break;
        case SPECIALDIR_MY_DOWNLOADS_DIR:
            path = getenv( "XDG_DOWNLOAD_DIR" );
            if ( GUCEF_NULL == path )
                path = getenv( "HOME" );
            break;
        case SPECIALDIR_MY_MUSIC_DIR:
            path = getenv( "XDG_MUSIC_DIR" );
            if ( GUCEF_NULL == path )
                path = getenv( "HOME" );
            break;
        case SPECIALDIR_MY_PICTURES_DIR:
            path = getenv( "XDG_PICTURES_DIR" );
            if ( GUCEF_NULL == path )
                path = getenv( "HOME" );
            break;
        default:
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "TryResolveSpecialDir: Unsupported special directory" );
            return false;
    }

    if ( GUCEF_NULL != path )
    {
        resolvedPath = path;
        return true;
    }
    return false;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    const char* path = GUCEF_NULL;
    switch ( dir )
    {
        case SPECIALDIR_HOME_DIR:
            path = "/home";  // Default virtual home directory in Emscripten
            break;
        case SPECIALDIR_TEMP_DIR:
            path = "/tmp";  // Default temporary directory in Emscripten
            break;
        case SPECIALDIR_APPDATA_DIR:
            path = "/appdata";  // Custom app data directory
            break;
        default:
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "TryResolveSpecialDir: Unsupported special directory for Emscripten" );
            return false;
    }

    if ( GUCEF_NULL != path )
    {
        resolvedPath = path;
        return true;
    }
    return false;

    #else

    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "TryResolveSpecialDir: Unsupported platform" );
    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

CDateTime
GetFileModificationTime( const CString& path )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    std::wstring wFilepath = ToWString( path );

    ::WIN32_FILE_ATTRIBUTE_DATA data;
    if ( 0 != ::GetFileAttributesExW( wFilepath.c_str(), GetFileExInfoStandard, &data ) )
    {
        return CDateTime( data.ftLastWriteTime );
    }
    return -1;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    struct stat buf;

    /* Get File Statistics for stat.c. */
    if( stat( path.C_String(), &buf ) == 0 )
    {
        /* get the date/time last modified */
        return CDateTime( buf.st_mtime, true );
    }
    return -1;

    #else

    /*
     *  Unsupported platform
     */
    return CDateTime();

    #endif
}


/*-------------------------------------------------------------------------*/

CDateTime
GetFileCreationTime( const CString& path )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    std::wstring wFilepath = ToWString( path );

    WIN32_FILE_ATTRIBUTE_DATA data;
    if ( 0 != GetFileAttributesExW( wFilepath.c_str(), GetFileExInfoStandard, &data ) )
    {
        return CDateTime( data.ftCreationTime );
    }
    return -1;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    struct stat buf;

    /* Get File Statistics for stat.c. */
    if( stat( path.C_String(), &buf ) == 0 )
    {
        /* get the date/time the file was created */
        return CDateTime( buf.st_ctime, true );
    }
    return -1;

    #else

    /*
     *  Unsupported platform
     */
    return CDateTime();

    #endif
}

/*-------------------------------------------------------------------------*/

bool
SetFileMetaData( const CString& filePath           ,
                 const CResourceMetaData& metaData )
{GUCEF_TRACE;

    if ( FileExists( filePath ) )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

        bool totalSuccess = true;
        std::wstring wFilepath = ToWString( filePath );

        // Get the current file attributes
        // we only want to change the flags we have and keep the rest as-is
        CResourceMetaData originalMetaData;
        if ( !GetFileMetaData( filePath, originalMetaData ) )
            return false;

        WIN32_FILE_ATTRIBUTE_DATA data;
        memset( &data, 0, sizeof data );

        // Convert boolean flags to file attribute flags
        if ( metaData.hasIsHidden ) { metaData.isHidden ? data.dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN : data.dwFileAttributes &= ~FILE_ATTRIBUTE_HIDDEN; }
        else { originalMetaData.isHidden ? data.dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN : data.dwFileAttributes &= ~FILE_ATTRIBUTE_HIDDEN; }

        if ( metaData.hasIsReadOnly ) { metaData.isReadOnly ? data.dwFileAttributes |= FILE_ATTRIBUTE_READONLY : data.dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY; }
        else { originalMetaData.isReadOnly ? data.dwFileAttributes |= FILE_ATTRIBUTE_READONLY : data.dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY; }

        if ( metaData.hasIsSystemResource ) { metaData.isSystemResource ? data.dwFileAttributes |= FILE_ATTRIBUTE_SYSTEM : data.dwFileAttributes &= ~FILE_ATTRIBUTE_SYSTEM; }
        else { originalMetaData.isSystemResource ? data.dwFileAttributes |= FILE_ATTRIBUTE_SYSTEM : data.dwFileAttributes &= ~FILE_ATTRIBUTE_SYSTEM; }

        if ( metaData.hasIsArchive ) { metaData.isArchive ? data.dwFileAttributes |= FILE_ATTRIBUTE_ARCHIVE : data.dwFileAttributes &= ~FILE_ATTRIBUTE_ARCHIVE; }
        else { originalMetaData.isArchive ? data.dwFileAttributes |= FILE_ATTRIBUTE_ARCHIVE : data.dwFileAttributes &= ~FILE_ATTRIBUTE_ARCHIVE; }

        if ( metaData.hasIsCompressed ) { metaData.isCompressed ? data.dwFileAttributes |= FILE_ATTRIBUTE_COMPRESSED : data.dwFileAttributes &= ~FILE_ATTRIBUTE_COMPRESSED; }
        else { originalMetaData.isCompressed ? data.dwFileAttributes |= FILE_ATTRIBUTE_COMPRESSED : data.dwFileAttributes &= ~FILE_ATTRIBUTE_COMPRESSED; }

        if ( metaData.hasIsEncrypted ) { metaData.isEncrypted ? data.dwFileAttributes |= FILE_ATTRIBUTE_ENCRYPTED : data.dwFileAttributes &= ~FILE_ATTRIBUTE_ENCRYPTED; }
        else { originalMetaData.isEncrypted ? data.dwFileAttributes |= FILE_ATTRIBUTE_ENCRYPTED : data.dwFileAttributes &= ~FILE_ATTRIBUTE_ENCRYPTED; }

        if ( metaData.hasIsTemporary ) { metaData.isTemporary ? data.dwFileAttributes |= FILE_ATTRIBUTE_TEMPORARY : data.dwFileAttributes &= ~FILE_ATTRIBUTE_TEMPORARY; }
        else { originalMetaData.isTemporary ? data.dwFileAttributes |= FILE_ATTRIBUTE_TEMPORARY : data.dwFileAttributes &= ~FILE_ATTRIBUTE_TEMPORARY; }

        //if ( metaData.hasIsOffline ) { metaData.isOffline ? data.dwFileAttributes |= FILE_ATTRIBUTE_OFFLINE : data.dwFileAttributes &= ~FILE_ATTRIBUTE_OFFLINE; }
        //else
             { originalMetaData.hasIsOffline ? data.dwFileAttributes |= FILE_ATTRIBUTE_OFFLINE : data.dwFileAttributes &= ~FILE_ATTRIBUTE_OFFLINE; }

        // Set the new file attributes
        BOOL result = ::SetFileAttributesW( wFilepath.c_str(), data.dwFileAttributes );
        if ( 0 == result )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "SetFileMetaData: SetFileAttributesW failed with error code: " + ToString( (UInt32) ::GetLastError() ) );
            totalSuccess = false;
        }

        // Convert date time to windows file time as needed
        metaData.hasCreationDateTime ? data.ftCreationTime = metaData.creationDateTime.ToWindowsFiletime() : data.ftCreationTime = originalMetaData.creationDateTime.ToWindowsFiletime();
        metaData.hasModifiedDateTime ? data.ftLastWriteTime = metaData.modifiedDateTime.ToWindowsFiletime() : data.ftLastWriteTime = originalMetaData.modifiedDateTime.ToWindowsFiletime();
        metaData.hasLastAccessedDateTime ? data.ftLastAccessTime = metaData.lastAccessedDateTime.ToWindowsFiletime() : data.ftLastAccessTime = originalMetaData.lastAccessedDateTime.ToWindowsFiletime();

        // Open the file for writing attributes without needing a handle for data read/write operations
        // we dont want to touch the file contents
        HANDLE hFile = ::CreateFileW( wFilepath.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        if ( INVALID_HANDLE_VALUE != hFile )
        {
            if ( 0 == ::SetFileTime( hFile, &data.ftCreationTime, &data.ftLastAccessTime, &data.ftLastWriteTime ) )
            {
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "SetFileMetaData: SetFileTime failed with error code: " + ToString( (UInt32) ::GetLastError() ) );
                totalSuccess = false;
            }
            ::CloseHandle( hFile );
            hFile = INVALID_HANDLE_VALUE;
        }
        else
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "SetFileMetaData: CreateFileW for FILE_WRITE_ATTRIBUTES failed with error code: " + ToString( (UInt32) ::GetLastError() ) );
            totalSuccess = false;
        }

        return totalSuccess;

        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

        /*
         *  Unsupported platform  - @TODO
         */
        return false;

        #else

        /*
         *  Unsupported platform
         */
        return false;

        #endif
    }

    // Cannot set meta-data on a non-existing file
    return false;
}

/*-------------------------------------------------------------------------*/

bool
GetFileMetaData( const CString& filePath     ,
                 CResourceMetaData& metaData )
{GUCEF_TRACE;

    metaData.Clear();

    if ( FileExists( filePath ) )
    {
        metaData.resourceExists = true;

        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

        std::wstring wFilepath = ToWString( filePath );

        WIN32_FILE_ATTRIBUTE_DATA data;
        if ( 0 != ::GetFileAttributesExW( wFilepath.c_str(), GetFileExInfoStandard, &data ) )
        {
            metaData.creationDateTime = CDateTime( data.ftCreationTime );
            metaData.hasCreationDateTime = true;
            metaData.modifiedDateTime = CDateTime( data.ftLastWriteTime );
            metaData.hasModifiedDateTime = true;
            metaData.lastAccessedDateTime = CDateTime( data.ftLastAccessTime );
            metaData.hasLastAccessedDateTime = true;

            ULARGE_INTEGER ul; // <- we use this struct for byte allignment
            ul.HighPart = data.nFileSizeHigh;
            ul.LowPart = data.nFileSizeLow;
            metaData.resourceSizeInBytes = (UInt64) ul.QuadPart;
            metaData.hasResourceSizeInBytes = true;

            data.dwFileAttributes & FILE_ATTRIBUTE_READONLY ? metaData.isReadOnly = true : metaData.isReadOnly = false;
            metaData.hasIsReadOnly = true;
            data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ? metaData.isHidden = true : metaData.isHidden = false;
            metaData.hasIsHidden = true;
            data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM || data.dwFileAttributes & FILE_ATTRIBUTE_DEVICE ? metaData.isSystemResource = true : metaData.isSystemResource = false;
            metaData.hasIsSystemResource = true;
            data.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE ? metaData.isArchive = true : metaData.isArchive = false;
            metaData.hasIsArchive = true;
            data.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED ? metaData.isCompressed = true : metaData.isCompressed = false;
            metaData.hasIsCompressed = true;
            data.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED ? metaData.isEncrypted = true : metaData.isEncrypted = false;
            metaData.hasIsEncrypted = true;
            data.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY ? metaData.isTemporary = true : metaData.isTemporary = false;
            metaData.hasIsTemporary = true;
            data.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE ? metaData.isOffline = true : metaData.isOffline = false;
            metaData.hasIsOffline = true;
        }
        else
        {
            // Failed to get file attributes.
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "GetFileMetaData: GetFileAttributesExW failed with error code: " + ToString( (UInt32) ::GetLastError() ) );
        }

        // true no matter what since access rights also factor into it
        // the meta data class has its own flags
        return true;

        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

        struct stat buf;

        // Get File Statistics for stat.c.
        if( stat( filePath.C_String(), &buf ) == 0 )
        {
            metaData.creationDateTime = CDateTime( buf.st_ctime, true );
            metaData.hasCreationDateTime = true;
            metaData.modifiedDateTime = CDateTime( buf.st_mtim, true );
            metaData.hasModifiedDateTime = true;
            metaData.lastAccessedDateTime = CDateTime( buf.st_atim, true );
            metaData.hasLastAccessedDateTime = true;
            metaData.resourceSizeInBytes = (UInt64) buf.st_size;
            metaData.hasResourceSizeInBytes = true;
        }

        // true no matter what since access rights also factor into it
        // the meta data class has its own flags
        return true;

        #else

        /*
         *  Unsupported platform
         */
        return false;

        #endif
    }
    else
    {
        metaData.resourceExists = false;
        return false;
    }
}

/*-------------------------------------------------------------------------*/

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

static int
recursive_mkdir(const char* dir, int accessPerms)
{
    GUCEF_TRACE;

    char tmp[PATH_MAX];
    char* p = NULL;
    size_t len;
    int retValue = 0;

    snprintf(tmp, sizeof(tmp), "%s", dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
    {
        tmp[len - 1] = 0;
    }
    for (p = tmp + 1; *p; ++p)
    {
        if (*p == '/')
        {
            *p = 0;
            retValue = mkdir(tmp, accessPerms);
            if (0 != retValue)
            {
                if (EEXIST != errno)
                {
                    return retValue;
                }
            }
            *p = '/';
        }
    }
    retValue = mkdir(tmp, accessPerms);
    if (0 != retValue)
    {
        if (EEXIST != errno)
        {
            return retValue;
        }
        else return 0;
    }
    return retValue;
}

#endif

/*-------------------------------------------------------------------------*/

bool
CreateDirs( const CString& path )
{GUCEF_TRACE;

    if ( path.IsNULLOrEmpty() )
        return false;

    CString actualPath = RelativePath( path );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // On Windows we have to take drive letter path syntax into account
    Int32 driveLetterSegIndex = actualPath.HasSubstr( ":\\", true );
    Int32 dirSepCharIndex = -1;
    if ( driveLetterSegIndex > 0 )
        dirSepCharIndex = actualPath.HasChar( GUCEF_DIRSEPCHAR, (UInt32) driveLetterSegIndex+3, true );
    else
        dirSepCharIndex = actualPath.HasChar( GUCEF_DIRSEPCHAR, true );

    while ( dirSepCharIndex >= 0 )
    {
        CString dirSegment = actualPath.SubstrToIndex( (UInt32) dirSepCharIndex, true );
        std::wstring wDirSegment = ToWString( dirSegment );
        if ( FALSE == ::CreateDirectoryW( wDirSegment.c_str(), NULL ) && ::GetLastError() != ERROR_ALREADY_EXISTS )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CreateDirs: Unable to create dir: \"" + dirSegment + "\" ErrorCode=" + ToString( (UInt32) ::GetLastError() ) );
            return false;
        }

        dirSepCharIndex = actualPath.HasChar( GUCEF_DIRSEPCHAR, dirSepCharIndex+1, true );
    }

    std::wstring wActualPath = ToWString( actualPath );
    if ( FALSE == ::CreateDirectoryW( wActualPath.c_str(), NULL ) && ::GetLastError() != ERROR_ALREADY_EXISTS )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CreateDirs: Unable to create dir: \"" + actualPath + "\" ErrorCode=" + ToString( (UInt32) ::GetLastError() ) );
        return false;
    }
    return true;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    int accessPerms = 0777;

    Int32 dirSepCharIndex = actualPath.HasChar( GUCEF_DIRSEPCHAR, true );
    while ( dirSepCharIndex >= 0 )
    {
        CString dirSegment = actualPath.SubstrToIndex( (UInt32) dirSepCharIndex, true );
        if ( 0 != ::mkdir( dirSegment.C_String(), accessPerms ) && EEXIST != errno )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CreateDirs: Unable to create dir: \"" + dirSegment + "\" ErrorCode=" + ToString( (UInt32) errno ) );
            return false;
        }

        dirSepCharIndex = actualPath.HasChar( GUCEF_DIRSEPCHAR, dirSepCharIndex, true );
    }

    if ( 0 != ::mkdir( actualPath.C_String(), accessPerms ) && EEXIST != errno )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CreateDirs: Unable to create dir: \"" + actualPath + "\" ErrorCode=" + ToString( (UInt32) errno ) );
        return false;
    }
    return true;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    int accessPerms = 0777;

    Int32 dirSepCharIndex = actualPath.HasChar( GUCEF_DIRSEPCHAR, true );
    while ( dirSepCharIndex >= 0 )
    {
        CString dirSegment = actualPath.SubstrToIndex( (UInt32) dirSepCharIndex, true );
        if ( 0 != ::mkdir( dirSegment.C_String(), accessPerms ) && EEXIST != errno )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CreateDirs: Unable to create dir: \"" + dirSegment + "\" ErrorCode=" + ToString( (UInt32) errno ) );
            return false;
        }

        dirSepCharIndex = actualPath.HasChar( GUCEF_DIRSEPCHAR, dirSepCharIndex, true );
    }

    if ( 0 != ::mkdir( actualPath.C_String(), accessPerms ) && EEXIST != errno )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CreateDirs: Unable to create dir: \"" + actualPath + "\" ErrorCode=" + ToString( (UInt32) errno ) );
        return false;
    }
    return true;

    #else

    /*
     *	Unsupported O/S build
     */
    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "CreateDirs: Platform has no supported implementation" );
    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
DeleteFile( const CString& path )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    CString actualPath = RelativePath( path );
    return 0 == ::unlink( actualPath.C_String() );

    #else

    CString actualPath = RelativePath( path );
    return 0 != Delete_File( actualPath.C_String() );

    #endif
}

/*-------------------------------------------------------------------------*/

bool
MoveFile( const CString& oldPath ,
          const CString& newPath ,
          const bool overwrite   )
{GUCEF_TRACE;

    CString actualOldPath = RelativePath( oldPath );
    CString actualNewPath = RelativePath( newPath );

    if ( !CreatePathDirectories( actualNewPath ) )
        return false;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    std::wstring wActualOldPath = ToWString( actualOldPath );
    std::wstring wActualNewPath = ToWString( actualNewPath );

    DWORD flags = MOVEFILE_WRITE_THROUGH | MOVEFILE_COPY_ALLOWED;
    if ( overwrite )
        flags = flags | MOVEFILE_REPLACE_EXISTING;

    BOOL result = ::MoveFileExW( wActualOldPath.c_str(), wActualNewPath.c_str(), flags );
    if ( result != TRUE )
    {
        GUCEF_WARNING_LOG( LOGLEVEL_BELOW_NORMAL, "MoveFile: MoveFileExW failed with error code: " +
            ToString( (UInt32) ::GetLastError() ) );
    }
    return result == TRUE;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

    struct stat originalPermissions;
    if ( 0 != ::stat( actualOldPath.C_String(), &originalPermissions ) )
        return 0;

    ::chmod( actualOldPath.C_String(), 0777 );

    if ( 0 != ::rename( actualOldPath.C_String(), actualNewPath.C_String() ) )
    {
        return 0;
    }

    ::chmod( actualNewPath.C_String(), originalPermissions.st_mode );
    return 1;

    #else

    // catch all expensive implementation
    if ( CopyFile( actualNewPath, actualOldPath, overwrite ) )
    {
    	return DeleteFile( actualOldPath );
    }
    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
FileExists( const CString& filename )
{GUCEF_TRACE;

    if ( !filename.IsNULLOrEmpty() )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

        std::wstring wFilename = ToWString( RelativePath( filename ) );

        WIN32_FIND_DATAW FileInfo;
        HANDLE hFind = GUCEF_NULL;
        hFind = FindFirstFileW( wFilename.c_str(), &FileInfo );
        if ( hFind != INVALID_HANDLE_VALUE )
        {
            FindClose( hFind );

            /* make sure we found a file not a directory */
            return !( FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY );
        }
        return 0;

        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

        struct stat buf;
        return stat( RelativePath( filename ).C_String(), &buf ) == 0;

        #else

        FILE* fptr = fopen( RelativePath( filename ).C_String(), "rb" );
        fclose( fptr );
        return GUCEF_NULL != fptr;

        #endif
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
DirExists( const CString& path )
{GUCEF_TRACE;

    if ( !path.IsNULLOrEmpty() )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

        std::wstring wPath = ToWString( path );

        WIN32_FIND_DATAW FileInfo;
        HANDLE hFind = GUCEF_NULL;
        hFind = FindFirstFileW( wPath.c_str(), &FileInfo );
        if ( hFind != INVALID_HANDLE_VALUE )
        {
            FindClose( hFind );

            /* make sure we found a directory not a file */
            return ( FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) > 0;
        }
        return false;

        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

        struct stat buf;
        if ( stat( path.C_String(), &buf ) == 0 )
            if ( ( buf.st_mode & S_IFDIR ) != 0 )
                return true;
        return false;

        #else

        FILE *fptr = fopen( filename.C_String(), "rb" );
        fclose( fptr );
        return fptr > 0;

        #endif
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
PathExists( const CString& path )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    std::wstring utf16Path = ToWString( path );
    if ( utf16Path.size() > 0 )
        return ::GetFileAttributesW( utf16Path.c_str() ) != INVALID_FILE_ATTRIBUTES;
    return false;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

    struct stat buf;
    return ( stat( path.C_String(), &buf ) == 0 );

    #else

    /*
     *  Unsupported platform
     */
    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
CreatePathDirectories( const CString& path )
{GUCEF_TRACE;

    Int32 delimIndex = path.HasChar( GUCEF_DIRSEPCHAR, false );
    if ( delimIndex < 0 )
        delimIndex = path.HasChar( GUCEF_DIRSEPCHAROPPOSITE, false );

    if ( delimIndex >= 0 )
    {
        CString dirs = path.SubstrToIndex( (UInt32) delimIndex, true );
        return CreateDirs( dirs );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

inline bool
IsSpecialLinuxFile( const struct stat& fileStat )
{GUCEF_TRACE;

    // File type masks:
    //  S_IFMT = bit mask for the file type bit fields
    //  S_IFLNK = symbolic link
    //  S_IFREG = regular file
    //  S_IFCHR = character device
    //  S_IFBLK = block device
    //  S_IFIFO = FIFO
    //  S_IFSOCK = socket

    //bool isRegularFile = S_ISREG( fileStat.st_mode );
    bool isDirectory = S_ISDIR( fileStat.st_mode );
    bool isBlockDevice = S_ISBLK( fileStat.st_mode );
    bool isSymLink = S_ISLNK( fileStat.st_mode );
    bool isCharDevice = S_ISCHR( fileStat.st_mode );
    bool isFifo = S_ISFIFO( fileStat.st_mode );
    bool isSocket = S_ISSOCK( fileStat.st_mode );
    //bool isSpecialNamed = S_ISNAM( fileStat.st_mode );
    return !isDirectory && ( isSymLink || isCharDevice || isFifo || isSocket );
}

/*-------------------------------------------------------------------------*/

inline bool
IsLinuxUserSpaceApiFile( const CString& filename )
{GUCEF_TRACE;

    if ( filename.StartsWith( "/proc/" ) )
        return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
TryResolveLinuxSymlinkPath( const CString& symlinkPath, CString& resolvedPath )
{GUCEF_TRACE;

    resolvedPath.Clear();

    if ( !symlinkPath.IsNULLOrEmpty()  )
    {
        Int32 bytesRead = 0;
        CString linkedPath;

        linkedPath.Reserve( 256 );
        if ( GUCEF_NULL != linkedPath.C_String() )
        {
            do
            {
                bytesRead = readlink( symlinkPath.C_String(), linkedPath.C_String(), linkedPath.ByteSize() );
                if ( -1 == bytesRead )
                {
                    return false;
                }
                if ( bytesRead == (Int32) linkedPath.ByteSize() )
                {
                    // truncation may have occured
                    linkedPath.Reserve( linkedPath.ByteSize() * 2 );
                }
            }
            while ( GUCEF_NULL != linkedPath.C_String() && bytesRead == (Int32) linkedPath.ByteSize() );
        }

        linkedPath.DetermineLength();
        resolvedPath = linkedPath.Trim( true );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
GetDeviceUUIDFromUdev( int major, int minor, CString& uuid )
{GUCEF_TRACE;

    uuid.Clear();

    CString udevPath = "/run/udev/data/b" + ToString( (Int32)major ) + ":" + ToString( (Int32)minor );
    std::ifstream udevFile( udevPath.STL_String() );
    if ( udevFile.is_open() )
    {
        std::string line;
        while ( std::getline( udevFile, line ) )
        {
            // P = path (or devpath)
            // N = name
            // L = link_priority - The default is 0
            // S = link (or symlink)
            // E = property. The E is probably because properties are accessed with ENV{key}
            if ( line.find( "E:ID_FS_UUID=" ) != std::string::npos )
            {
                uuid = line.substr(line.find("=") + 1);
                return true;
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

// Function to get the major and minor numbers of a device
bool
GetDeviceVersionNumbers( const CString& device, int& major, int& minor )
{GUCEF_TRACE;

    major = 0;
    minor = 0;

    std::string devPath = "/sys/class/block/" + device + "/dev";
    std::ifstream devFile( devPath );
    std::string devNumbers;
    if ( devFile.is_open() )
    {
        std::getline( devFile, devNumbers );

        std::istringstream iss(devNumbers);
        char colon;
        iss >> major >> colon >> minor;

        return true;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
GetDeviceUUIDForDeviceName( const CString& deviceName, CString& uuid )
{GUCEF_TRACE;

    // Try method 1:
    if ( LoadTextFileAsString( "/sys/class/block/" + deviceName + "/uuid", uuid, false ) )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "GetDeviceUUIDForDeviceName: used /sys/class/block/<device>/uuid to resolve " +
            deviceName + " to " + uuid );
        return true;
    }

    // Try method 2:
    int devMajorVersion = 0;
    int devMinorVersion = 0;
    if ( GetDeviceVersionNumbers( deviceName, devMajorVersion, devMinorVersion ) )
    {
        if ( GetDeviceUUIDFromUdev( devMajorVersion, devMinorVersion, uuid ) )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "GetDeviceUUIDForDeviceName: used /run/udev/data/ to resolve " +
                deviceName + " with version " +
                ToString( devMajorVersion ) + "." + ToString( devMinorVersion ) + " to " + uuid );
            return true;
        }
    }

    // Fallback method 3:
    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    if ( TryToDetermineIfRunningInContainer() )
    #endif
    {
        // We think we are in a container, as such we have no hope of getting the UUID unless the container is
        // running as privileged. As such we will generate a fake unique id as a substitution for our API calls
        if ( 0 != devMajorVersion && 0 != devMinorVersion )
        {
            uuid = "fakeuuid:" + deviceName + "_" + ToString( (Int32)devMajorVersion ) + ":" + ToString( (Int32)devMinorVersion );

            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "GetDeviceUUIDForDeviceName: Suspected containerized environment. Will generate fake substitute uuid since hardware specifics are not accessible from " +
                deviceName + " with version " +
                ToString( (Int32)devMajorVersion ) + ":" + ToString( (Int32)devMinorVersion ) + " to " + uuid );
            return true;
        }

        uuid = "fakeuuid:" + deviceName;

        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "GetDeviceUUIDForDeviceName: Suspected containerized environment. Will generate fake substitute uuid since hardware specifics are not accessible from " +
            deviceName + " without a version to " + uuid );
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

class GUCEF_HIDDEN CLinuxProcMountsInfo
{
    public:

    CString device;
    CString mountPoint;
    CString filesystemType;
    CString mountOptions;
    bool dump;
    Int32 pass;

    CLinuxProcMountsInfo( void )
        : device()
        , mountPoint()
        , filesystemType()
        , mountOptions()
        , dump( false )
        , pass( 0 )
    {GUCEF_TRACE;

    }

    bool TryParseString( const CString& str )
    {GUCEF_TRACE;

        CString::StringVector data = str.ParseElements( ' ', false );

        // the information is stored positionally
        if ( data.size() > 0 )
            device = data[ 0 ];
        if ( data.size() > 1 )
            mountPoint = data[ 1 ];
        if ( data.size() > 2 )
            filesystemType = data[ 2 ];
        if ( data.size() > 3 )
            mountOptions = data[ 3 ];
        if ( data.size() > 4 )
            dump = StringToBool( data[ 4 ], false );
        if ( data.size() > 5 )
            pass = StringToInt32( data[ 5 ], 0 );

        return true;
    }

};

typedef std::vector< CLinuxProcMountsInfo, gucef_allocator< CLinuxProcMountsInfo > >  TLinuxProcMountsInfoVector;

/*-------------------------------------------------------------------------*/

bool
ParseLinuxProcMounts( TLinuxProcMountsInfoVector& mounts )
{GUCEF_TRACE;

    CString mountFileContent;
    if ( LoadTextFileAsString( "/proc/mounts", mountFileContent, true, "\n" ) )
    {
        bool totalSuccess = true;

        CString::StringVector lines = mountFileContent.ParseElements( '\n', false );
        mounts.reserve( lines.size() );

        CString::StringVector::iterator i = lines.begin();
        while ( i != lines.end() )
        {
            const CString& line = (*i);

            mounts.push_back( CLinuxProcMountsInfo() );
            CLinuxProcMountsInfo& info = mounts.back();

            if ( !info.TryParseString( line ) )
            {
                mounts.pop_back();
                totalSuccess = false;
            }
            ++i;
        }

        return totalSuccess;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool GUCEF_HIDDEN
GetMountPathForDeviceName( const CString& deviceName ,
                           CString& mountPath        )
{GUCEF_TRACE;

    mountPath.Clear();

    TLinuxProcMountsInfoVector mounts;
    if ( ParseLinuxProcMounts( mounts ) )
    {
        TLinuxProcMountsInfoVector::iterator i = mounts.begin();
        while ( i != mounts.end() )
        {
            CLinuxProcMountsInfo& info = (*i);
            if ( deviceName == info.device )
            {
                mountPath = info.mountPoint;
                return true;
            }
            ++i;
        }
    }
    return false;
}

#endif /* Linux or Android ? */

/*-------------------------------------------------------------------------*/

UInt64
FileSize( const CString& filename )
{GUCEF_TRACE;

    if ( !filename.IsNULLOrEmpty() )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

        std::wstring wFilename = ToWString( RelativePath( filename ) );

        UInt64 lfilesize = 0;
        WIN32_FIND_DATAW FileInfo;
        HANDLE hFind;
        hFind = FindFirstFileW( wFilename.c_str(), &FileInfo );
        if ( hFind == INVALID_HANDLE_VALUE )
        {
            lfilesize = 0;
        }
        else
        {
            lfilesize = FileInfo.nFileSizeLow;
        }
        FindClose( hFind );
        return lfilesize;

        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

        struct stat statInfo;
        int result;
        result = stat( RelativePath( filename.C_String() ).C_String(), &statInfo );
        if ( result == 0 )
        {
            if ( 0 == statInfo.st_size )
            {
                // A file size of 0 doesnt per se mean its actually 0 on Linux for special system files
                // Those are generated on demand and are user space APIs
                // As such the file size can also fluctuate but in a way that is no different from other
                // file types where in between non-transactional/atomic operations someone could change a file
                if ( IsSpecialLinuxFile( statInfo ) || IsLinuxUserSpaceApiFile( filename ) )
                {
                    FILE* fptr = fopen( filename.C_String(), "rb" );
                    if ( NULL != fptr )
                    {
                        // Only way to get the data size for these is to actually read the data
                        // data size is normally small so this should not take long
                        UInt64 filesize = 0;
                        size_t bytesRead = 0;
                        char dummyBuffer[ 256 ];
                        do
                        {
                            bytesRead = fread( dummyBuffer, 1, 256, fptr );
                            filesize += bytesRead;
                        }
                        while ( bytesRead >= 256 );
                        fclose( fptr );
                        fptr = NULL;

                        return filesize;
                    }

                    return 0;
                }
            }

            return statInfo.st_size;

        }
        return 0;

        #else

        UInt64 filesize = 0;
        FILE *fptr = fopen( RelativePath( filename ).C_String(), "rb" );
        fseek( fptr, 0, SEEK_END );
        filesize = ftell( fptr );
        fclose( fptr );
        return filesize;

        #endif
    }

    return 0;
}

/*-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ) 

CString
ConvertGuidToDeviceName( const CString& volumeGuid )
{GUCEF_TRACE;

    // A device path differes from a Windows UNC path in that it does not have the "\\?\" prefix nor the trailing backslash
    // The device path is also not prefixed with "\\.\\" as is the case for a UNC path
    if ( volumeGuid.IsNULLOrEmpty() )
        return CString::Empty;

    if ( volumeGuid.HasSubstr( "\\\\?\\Volume{", true ) == 0 )
    {
        // The volume GUID is actually already in path format
        return volumeGuid;
    }

    CString volumePath = "\\.\\Volume" + volumeGuid ;
    return volumePath;
}

/*-------------------------------------------------------------------------*/

CString
ConvertGuidToRawVolumePath( const CString& volumeGuid )
{GUCEF_TRACE;

    // A device path differes from a Windows UNC path in that it does not have the "\\?\" prefix nor the trailing backslash
    // The device path is also not prefixed with "\\.\\" as is the case for a UNC path
    if ( volumeGuid.IsNULLOrEmpty() )
        return CString::Empty;

    // check for device path format
    if ( volumeGuid.HasSubstr( "\\\\?\\Volume{", true ) == 0 )
    {
        // The volume GUID is actually already in path format
        return volumeGuid;
    }

    // use extended path format
    CString volumePath = "\\\\?\\Volume" + volumeGuid;
    return volumePath;
}

/*-------------------------------------------------------------------------*/

CString
ConvertGuidToDirPath( const CString& volumeGuid )
{GUCEF_TRACE;

    // A device path differes from a Windows UNC path in that it does not have the "\\?\" prefix nor the trailing backslash
    // The device path is also not prefixed with "\\.\\" as is the case for a UNC path
    if ( volumeGuid.IsNULLOrEmpty() )
        return CString::Empty;

    // check for device path format
    if ( volumeGuid.HasSubstr( "\\\\?\\Volume{", true ) == 0 )
    {
        // The volume GUID is actually already in path format
        return volumeGuid;
    }

    // use extended path format
    CString volumePath = "\\\\?\\Volume" + volumeGuid + "\\";
    return volumePath;
}

/*-------------------------------------------------------------------------*/

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

CString
ConvertGuidToVolumePath( const CString& volumeGuid, bool haveTrailingSlash )
{GUCEF_TRACE;

    if ( volumeGuid.IsNULLOrEmpty() )
        return CString::Empty;

    if ( !volumeGuid.StartsWith( "fakeuuid:" ) )
    {
        if ( '/' == volumeGuid[ 0 ] )
        {
            // The volume GUID is actually already in path format
            return volumeGuid;
        }

        CString volumePath = "/dev/disk/by-uuid/" + volumeGuid;
        return volumePath;
    }
    else
    {
        // since we have a fake uuid we need to go based on device name
        CString deviceName;
        bool hasVersionInfo = false;
        int majorVersion = 0;
        int minorVersion = 0;
        if ( ParseFakeUUID( volumeGuid, deviceName, hasVersionInfo, majorVersion, minorVersion ) )
        {
            CString mountPath;
            if ( GetMountPathForDeviceName( deviceName, mountPath ) )
            {
                return mountPath;
            }
        }
    }

    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

#else

CString
ConvertGuidToVolumePath( const CString& volumeGuid )
{GUCEF_TRACE;

    return CString::Empty; // no implementation for this platform
}

#endif

/*-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

std::wstring GUCEF_HIDDEN
FormatPathForQueryDosDevice( const std::wstring& path ) 
{
    std::wstring formattedPath = path;

    // 1. Check for drive letter
    if (formattedPath.length() == 2 && formattedPath[1] == L':') {
        formattedPath += L'\\';
        return formattedPath;
    }

    // 2. Check for mount point
      if (formattedPath.length() > 1 && formattedPath.back() == L'\\')
      {
          size_t secondLastSlash = formattedPath.rfind(L'\\', formattedPath.length() - 2);
          if (secondLastSlash != std::wstring::npos && secondLastSlash > 0 && formattedPath[secondLastSlash - 1] == L':')
              return formattedPath;
      }

    // 3. Remove trailing backslash (if not drive letter)
    if (!formattedPath.empty() && formattedPath.back() == L'\\') {
        formattedPath.pop_back();
    }

    return formattedPath;
}

/*-------------------------------------------------------------------------*/

bool GUCEF_HIDDEN
GetKernelLevelSymlinkForPath( const CString& path ,
                              CString& devicePath ) 
{GUCEF_TRACE;
    
    if ( path.IsNULLOrEmpty() )
        return false;    

    wchar_t deviceInfo[ MAX_PATH ] = {0};

    // clear the last error (if any)
    UInt64 errorCode = static_cast< UInt64 >( ::GetLastError() );

    // Call QueryDosDeviceW to get the device information. The path needs to be formatted just right
    std::wstring wPath = FormatPathForQueryDosDevice( ToWString( path ) );
    DWORD result = ::QueryDosDeviceW( wPath.c_str(), deviceInfo, MAX_PATH );
    if ( 0 != result ) 
    {                        
        CString foundDeviceInfo = ToString( deviceInfo );
        Int32 offset = foundDeviceInfo.HasSubstr( "HarddiskVolume" );
        if ( offset > 0 )
        {
            devicePath = foundDeviceInfo;
            return true;
        }
    }
    else
    {
        errorCode = static_cast< UInt64 >( ::GetLastError() );
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "GetPhysicalDriveFromGUID: QueryDosDeviceW error code: " + ToString( errorCode ) );
    }

    return false;    
}

/*-------------------------------------------------------------------------*/

bool GUCEF_HIDDEN
Win32GetDiskGeometryFromDriver( CStorageDeviceInformation& info ) 
{GUCEF_TRACE;

    if ( info.deviceId.IsNULLOrEmpty() )
        return false;    

    if ( info.hasGeometry )
        return true; // already has the geometry info

    std::wstring wDeviceId = ToWString( info.deviceId );

    HANDLE hDevice = ::CreateFileW( wDeviceId.c_str(), GENERIC_READ, 
                                FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
                                OPEN_EXISTING, 0, NULL);
    
    if ( hDevice == INVALID_HANDLE_VALUE ) 
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Win32GetDiskGeometry: Failed to open device with id: " + info.deviceId );
        return false;
    }

    ::DISK_GEOMETRY dg;
    memset( &dg, 0, sizeof(dg) ); // Initialize the structure to zero
    DWORD bytesReturned = 0;
    
    BOOL success = ::DeviceIoControl( hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, 
                             &dg, sizeof(dg), &bytesReturned, NULL); 
    if ( 0 != success ) 
    {
        info.geometry.hasCylinderCount = true;
        info.geometry.cylinderCount = dg.Cylinders.QuadPart;
        info.geometry.hasTracksPerCylinder = true;
        info.geometry.tracksPerCylinder = dg.TracksPerCylinder;
        info.geometry.hasSectorsPerTrack = true;
        info.geometry.sectorsPerTrack = dg.SectorsPerTrack;
        info.geometry.hasBytesPerSector = true;
        info.geometry.bytesPerSector = dg.BytesPerSector;
        info.geometry.hasIsRemovable = true;
        info.geometry.isRemovable = (dg.MediaType == RemovableMedia);

        info.hasGeometry = true;

    } 
    else 
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Win32GetDiskGeometry: DeviceIoControl failed to get disk geometry for device with id: " + info.deviceId );
    }

    CloseHandle( hDevice );
    hDevice = INVALID_HANDLE_VALUE;

    return 0 != success;
}

/*-------------------------------------------------------------------------*/

bool GUCEF_HIDDEN
Win32GetDiskGeometry( CStorageDeviceInfoOSData& osData ,
                      CStorageDeviceInformation& info  ) 
{GUCEF_TRACE;

    if ( IsRunningAsElevatedAdmin() )
        return Win32GetDiskGeometryFromDriver( info );

    return true; // we did the best we could, no error per se, check has* flags
}

/*-------------------------------------------------------------------------*/

bool 
Win32GetStoragePerfStatsFromDriver( const std::wstring& filePath ,
                                    CStoragePerfStats& perfStats ) 
{GUCEF_TRACE;

    HANDLE hDevice = ::CreateFileW( filePath.c_str(), GENERIC_READ, 
                                FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
                                OPEN_EXISTING, 0, NULL);

    if ( hDevice == INVALID_HANDLE_VALUE ) 
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Win32GetDiskPerfStats: Failed to open device : " + ToString( filePath ) );
        return false;
    }

    ::DISK_PERFORMANCE diskPerformance;
    memset( &diskPerformance, 0, sizeof(diskPerformance) ); // Initialize the structure to zero
    DWORD bytesReturned;
    
    BOOL success = ::DeviceIoControl( hDevice, IOCTL_DISK_PERFORMANCE, NULL, 0, 
                                        &diskPerformance, sizeof(diskPerformance), 
                                        &bytesReturned, NULL );
    
    if ( 0 != success ) 
    {
        perfStats.hasBytesRead = true;
        perfStats.bytesRead = diskPerformance.BytesRead.QuadPart;
        perfStats.hasBytesWritten = true;
        perfStats.bytesWritten = diskPerformance.BytesWritten.QuadPart;
        perfStats.hasReadTimeInMs = true;
        perfStats.readTimeInMs = diskPerformance.ReadTime.QuadPart / 10000;
        perfStats.hasWriteTimeInMs = true;
        perfStats.writeTimeInMs = diskPerformance.WriteTime.QuadPart / 10000;
        perfStats.hasIdleTimeInMs = true;
        perfStats.idleTimeInMs = diskPerformance.IdleTime.QuadPart / 10000;
        perfStats.hasRequestQueueDepth = true;
        perfStats.requestQueueDepth = diskPerformance.QueueDepth;
        perfStats.hasRequestSplitCount = true;
        perfStats.requestSplitCount = diskPerformance.SplitCount;
    } 
    else 
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Win32GetDiskGeometry: DeviceIoControl failed to get disk performance for device with path: " + ToString( filePath ) );
    }
    
    ::CloseHandle( hDevice );
    hDevice = INVALID_HANDLE_VALUE;
    
    return 0 != success;
}

/*-------------------------------------------------------------------------*/

bool 
Win32GetDiskPerfStatsFromDriver( CStorageDeviceInfoOSData& osData ,
                                 CStorageDeviceInformation& info  ) 
{GUCEF_TRACE;

    // For a device we use the device id for DeviceIoControl
    
    if ( osData.GetDeviceId().IsNULLOrEmpty() )
        return false;    

    std::wstring wDeviceId = ToWString( osData.GetDeviceId() );    
    return info.hasPerfStats = Win32GetStoragePerfStatsFromDriver( wDeviceId, info.perfStats );
}

/*-------------------------------------------------------------------------*/

bool 
Win32GetVolumePerfStatsFromDriver( CStorageVolumeInfoOSData& osData ,
                                   CStorageVolumeInformation& info  ) 
{GUCEF_TRACE;

    // For a volume we use the volume guid for DeviceIoControl
    
    if ( osData.GetVolumeId().IsNULLOrEmpty() )
        return false;    

    std::wstring wVolumeId = ToWString( osData.GetVolumeId() );    
    return info.hasPerfStats = Win32GetStoragePerfStatsFromDriver( wVolumeId, info.perfStats );
}

/*-------------------------------------------------------------------------*/

bool 
Win32GetDiskPerfStats( CStorageDeviceInfoOSData& osData ,
                       CStorageDeviceInformation& info  ) 
{GUCEF_TRACE;

    bool totalSuccess = true;

    if ( IsRunningAsElevatedAdmin() )
    {
        // Running as an elevated admin user providers the highest performance and most accurate data
        // It negates the need to use any fallbacks
        totalSuccess = Win32GetDiskPerfStatsFromDriver( osData, info ) && totalSuccess; 
    }

    // Query the PDH API
    // This is not as accurate but does not require admin rights
    // On Windows its complementory since it uses per second and avg metrics which is different from the
    // raw data we get from the driver which are running totals
    CWindowsComponentAccess* winComponentAccess = CWindowsComponentAccess::Instance();
    CPdhAccess* pdhApi = winComponentAccess->GetPdhAccess();
    if ( GUCEF_NULL != pdhApi && pdhApi->IsValid() )
    {        
        bool hasNewStats = false;
        Int32 errorCode = pdhApi->GetPhysicalDevicePerfStats( osData.GetOsData()->devicePerfStatAccess, info, hasNewStats );
        totalSuccess = ( 0 == errorCode ) && totalSuccess;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool 
Win32GetVolumePerfStats( CStorageVolumeInfoOSData& osData ,
                         CStorageVolumeInformation& info  ) 
{GUCEF_TRACE;

    bool totalSuccess = true;

    if ( IsRunningAsElevatedAdmin() )
    {
        // Running as an elevated admin user providers the highest performance and most accurate data
        // It negates the need to use any fallbacks
        totalSuccess = Win32GetVolumePerfStatsFromDriver( osData, info ) && totalSuccess; 
    }

    // Query the PDH API
    // This is not as accurate but does not require admin rights
    // On Windows its complementory since it uses per second and avg metrics which is different from the
    // raw data we get from the driver which are running totals
    CWindowsComponentAccess* winComponentAccess = CWindowsComponentAccess::Instance();
    CPdhAccess* pdhApi = winComponentAccess->GetPdhAccess();
    if ( GUCEF_NULL != pdhApi && pdhApi->IsValid() )
    {        
        bool hasNewStats = false;
        Int32 errorCode = pdhApi->GetLogicalVolumePerfStats( osData.GetOsData()->volumePerfStatAccess, info, hasNewStats );
        totalSuccess = ( 0 == errorCode ) && totalSuccess;
    }

    return totalSuccess;
}

#endif

/*-------------------------------------------------------------------------*/

bool 
GetPhysicalDeviceIdsForVolumeId( const CString& guid                    ,
                                 CStringMap& partitionId2physicalDiskId ) 
{GUCEF_TRACE;

    partitionId2physicalDiskId.clear();
    
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    // On Windows we can use WMI to get the device IDs
    // Do note that even WMI has a higher probability of not requiring admin rights its not garantueed
    // part of the success will depend on the user account control settings
    CWindowsComponentAccess* winComponentAccess = CWindowsComponentAccess::Instance();
    CWmiAccess* wmiApi = winComponentAccess->GetWmiAccess();
    if ( GUCEF_NULL != wmiApi && wmiApi->IsValid() )
    {
        Int32 errorCode = wmiApi->GetPhysicalDeviceIdsFromWMI( guid, partitionId2physicalDiskId );
        return 0 == errorCode;
    }
    return false;

    #else

    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

CStoragePerfStats::CStoragePerfStats( void )
    : hasBytesRead( false )
    , bytesRead( 0 )
    , hasBytesReadPerSec( false )
    , bytesReadPerSec( 0 )
    , hasBytesWritten( false )
    , bytesWritten( 0 )
    , hasBytesWrittenPerSec( false )
    , bytesWrittenPerSec( 0 )
    , hasReadTimeInMs( false )
    , readTimeInMs( 0 )
    , hasWriteTimeInMs( false )
    , writeTimeInMs( 0 )
    , hasIdleTimeInMs( false )
    , idleTimeInMs( 0 )
    , hasAvgReadTimePerOperationInMs( false )
    , avgReadTimePerOperationInMs( 0.0f )
    , hasAvgWriteTimePerOperationInMs( false )
    , avgWriteTimePerOperationInMs( 0.0f )
    , hasRequestQueueDepth( false )
    , requestQueueDepth( 0 )
    , hasRequestSplitCount( false )
    , requestSplitCount( 0 )
    , hasRequestSplitCountPerSec( false )
    , requestSplitCountPerSec( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CStoragePerfStats::Clear( void )
{GUCEF_TRACE;

    hasBytesRead = false;
    bytesRead = 0;
    hasBytesReadPerSec = false;
    bytesReadPerSec = 0;
    hasBytesWrittenPerSec = false;
    bytesWrittenPerSec = 0;
    hasBytesWritten = false;
    bytesWritten = 0;
    hasReadTimeInMs = false;
    readTimeInMs = 0;
    hasWriteTimeInMs = false;
    writeTimeInMs = 0;
    hasIdleTimeInMs = false;
    idleTimeInMs = 0;
    hasAvgReadTimePerOperationInMs = false;
    avgReadTimePerOperationInMs = 0.0f;
    hasAvgWriteTimePerOperationInMs = false;
    avgWriteTimePerOperationInMs = 0.0f;
    hasRequestQueueDepth = false;
    requestQueueDepth = 0;
    hasRequestSplitCount = false;
    requestSplitCount = 0;
    hasRequestSplitCountPerSec = false;
    requestSplitCountPerSec = 0;    
}

/*-------------------------------------------------------------------------*/

CStorageDeviceGeometry::CStorageDeviceGeometry( void )
    : hasCylinderCount( false )
    , cylinderCount( 0 )
    , hasTracksPerCylinder( false )
    , tracksPerCylinder( 0 )
    , hasSectorsPerTrack( false )
    , sectorsPerTrack( 0 )
    , hasBytesPerSector( false )
    , bytesPerSector( 0 )
    , hasIsRemovable( false )
    , isRemovable( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CStorageDeviceGeometry::Clear( void )
{GUCEF_TRACE;

    hasCylinderCount = false;
    cylinderCount = 0;
    hasTracksPerCylinder = false;
    tracksPerCylinder = 0;
    hasSectorsPerTrack = false;
    sectorsPerTrack = 0;
    hasBytesPerSector = false;
    bytesPerSector = 0;
    hasIsRemovable = false;
    isRemovable = false;
}

/*-------------------------------------------------------------------------*/

CStorageDeviceInformation::CStorageDeviceInformation( void )
    : hasDeviceId( false )
    , deviceId()
    , hasDeviceIndex( false )
    , deviceIndex( 0 )
    , hasGeometry( false )
    , geometry()
    , hasPerfStats( false )
    , perfStats()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CStorageDeviceInformation::Clear( void )
{GUCEF_TRACE;

    hasDeviceId = false;
    deviceId.Clear();
    hasDeviceIndex = false;
    deviceIndex = 0;
    hasGeometry = false;
    geometry.Clear();
    hasPerfStats = false;
    perfStats.Clear();
}

/*-------------------------------------------------------------------------*/

CStorageVolumeInfoOSData::CStorageVolumeInfoOSData( void )
    : m_impl( GUCEF_NEW CStorageVolumeInfoOSDataImpl() )
    , m_volumeId()
    , m_anyPathForVolume()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStorageVolumeInfoOSData::~CStorageVolumeInfoOSData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStorageVolumeInfoOSDataImpl* 
CStorageVolumeInfoOSData::GetOsData( void ) const
{GUCEF_TRACE;

    return m_impl;
}

/*-------------------------------------------------------------------------*/

void 
CStorageVolumeInfoOSData::SetVolumeId( const CString& volumeId )
{GUCEF_TRACE;

    m_volumeId = volumeId;
}

/*-------------------------------------------------------------------------*/

void 
CStorageVolumeInfoOSData::SetAnyPathForVolume( const CString& anyPathForVolume )
{GUCEF_TRACE;

    m_anyPathForVolume = anyPathForVolume;
}

/*-------------------------------------------------------------------------*/

const CString& 
CStorageVolumeInfoOSData::GetVolumeId( void ) const
{GUCEF_TRACE;

    return m_volumeId;
}

/*-------------------------------------------------------------------------*/

const CString& 
CStorageVolumeInfoOSData::GetAnyPathForVolume( void ) const
{GUCEF_TRACE;

    return m_anyPathForVolume;
}

/*-------------------------------------------------------------------------*/

CStorageDeviceInfoOSData::CStorageDeviceInfoOSData( const CString& deviceId )
    : m_impl( GUCEF_NEW CStorageDeviceInfoOSDataImpl() )
    , m_deviceId( deviceId )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStorageDeviceInfoOSData::~CStorageDeviceInfoOSData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStorageDeviceInfoOSDataImpl* 
CStorageDeviceInfoOSData::GetOsData( void ) const
{GUCEF_TRACE;

    return m_impl;
}

/*-------------------------------------------------------------------------*/

const CString& 
CStorageDeviceInfoOSData::GetDeviceId( void ) const
{GUCEF_TRACE;

    return m_deviceId;
}

/*-------------------------------------------------------------------------*/

bool
GetStorageDeviceInformationByDeviceId( CStorageDeviceInfoOSData& osData ,
                                       CStorageDeviceInformation& info  )
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( osData.GetDeviceId().IsNULLOrEmpty() )
        return false;
    
    bool totalSuccess = true;
    
    if ( !info.hasDeviceId || info.deviceId.IsNULLOrEmpty() )
    {
        info.deviceId = osData.GetDeviceId();
        if ( !info.deviceId.IsNULLOrEmpty() )
        {
            info.hasDeviceId = true;
        }
        else
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "GetStorageDeviceInformationByDeviceId: No device id provided" );
            return false; // we cannot continue without a device id
        }
    }
    else
    {
        // We already have the device id
        if ( info.deviceId != osData.GetDeviceId() )
            return false; // we cannot mix device ids
    }

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // On Windows the device index is simply the appended number in the device id
    if ( !info.hasDeviceIndex )
    {
        Int32 offset = info.deviceId.HasSubstr( "PHYSICALDRIVE" );
        if ( offset > 0 )
        {
            CString deviceIndexStr = info.deviceId.CutChars( offset + 13 );
            Int32 deviceIndex = StringToInt32( deviceIndexStr, -1 );
            if ( deviceIndex >= 0 )
            {
                info.hasDeviceIndex = true;
                info.deviceIndex = static_cast< UInt32 >( deviceIndex );
            }
        }
    }

    totalSuccess = Win32GetDiskGeometry( osData, info ) && totalSuccess;
    totalSuccess = Win32GetDiskPerfStats( osData, info ) && totalSuccess;

    #endif

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

CStorageVolumeInformation::CStorageVolumeInformation( void )
    : hasVolumeId( false )
    , volumeId()
    , hasFreeBytesAvailableToCaller( false )
    , freeBytesAvailableToCaller( 0 )
    , hasTotalNumberOfBytes( false )
    , totalNumberOfBytes( 0 )
    , hasTotalNumberOfFreeBytes( false )
    , totalNumberOfFreeBytes( 0 )
    , hasIsReadOnly( false )
    , isReadOnly( false )
    , hasVolumeName( false )
    , volumeName()
    , hasPaths( false )
    , paths()
    , hasPartitionId2deviceIdMapping( false )
    , partitionId2deviceId()
    , hasPerfStats( false )
    , perfStats()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CStorageVolumeInformation::Clear( void )
{GUCEF_TRACE;

    hasVolumeId = false;
    volumeId.Clear();
    hasFreeBytesAvailableToCaller = false;
    freeBytesAvailableToCaller = 0;
    hasTotalNumberOfBytes = false;
    totalNumberOfBytes = 0;
    hasTotalNumberOfFreeBytes = false;
    totalNumberOfFreeBytes = 0;
    hasIsReadOnly = false;
    isReadOnly = false;
    hasVolumeName = false;
    volumeName.Clear();
    hasPaths = false;
    paths.clear();
    hasPartitionId2deviceIdMapping = false;
    partitionId2deviceId.clear();
    hasPerfStats = false;
    perfStats.Clear();
}

/*-------------------------------------------------------------------------*/

bool GUCEF_CORE_PRIVATE_CPP
GetFileSystemStorageVolumeInformationByDirPath_NoClear( CStorageVolumeInfoOSData& osData ,
                                                        CStorageVolumeInformation& info  )
{GUCEF_TRACE;

    if ( osData.GetAnyPathForVolume().IsNULLOrEmpty() )
        return false;
    const CORE::CString& path = osData.GetAnyPathForVolume();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    CString dirPath = RelativePath( path );
    if ( dirPath.Length() != dirPath.HasChar( '\\', false )+1 )
    {
        // A trailing backslash is required for the below Win32 API
        dirPath += '\\';
    }

    bool totalSuccess = true;
    std::wstring wActualPath = ToWString( dirPath );

    ULARGE_INTEGER freeBytesAvailableToCaller;
    ULARGE_INTEGER totalNumberOfBytes;
    ULARGE_INTEGER totalNumberOfFreeBytes;
    BOOL result = ::GetDiskFreeSpaceExW( wActualPath.c_str()         ,
                                         &freeBytesAvailableToCaller ,
                                         &totalNumberOfBytes         ,
                                         &totalNumberOfFreeBytes     );
    if ( TRUE == result )
    {
        info.freeBytesAvailableToCaller = freeBytesAvailableToCaller.QuadPart;
        info.hasFreeBytesAvailableToCaller = true;
        info.totalNumberOfBytes = totalNumberOfBytes.QuadPart;
        info.hasTotalNumberOfBytes = true;
        info.totalNumberOfFreeBytes = totalNumberOfFreeBytes.QuadPart;
        info.hasTotalNumberOfFreeBytes = true;
    }
    else
    {
        totalSuccess = false;
    }

    if ( !info.hasVolumeName || !info.hasIsReadOnly )
    {
        wchar_t volumeNameBuffer[MAX_PATH + 1] = {0};
        wchar_t fileSystemNameBuffer[MAX_PATH + 1] = {0};
        DWORD serialNumber = 0;
        DWORD maxComponentLen = 0;
        DWORD fileSystemFlags = 0;

        result = ::GetVolumeInformationW( wActualPath.c_str()          ,
                                          volumeNameBuffer             ,
                                          sizeof(volumeNameBuffer)     ,
                                          &serialNumber                ,
                                          &maxComponentLen             ,
                                          &fileSystemFlags             ,
                                          fileSystemNameBuffer         ,
                                          sizeof(fileSystemNameBuffer) );

        if ( TRUE == result )
        {
            info.volumeName = ToString( volumeNameBuffer );
            info.hasVolumeName = true;
            info.isReadOnly = (fileSystemFlags & FILE_READ_ONLY_VOLUME) == FILE_READ_ONLY_VOLUME;
            info.hasIsReadOnly = true;
        }
        else
        {
            totalSuccess = false;
        }
    }

    if ( !info.hasVolumeId )
    {
        if ( !osData.GetVolumeId().IsNULLOrEmpty() )
        {
            info.volumeId = osData.GetVolumeId();
            info.hasVolumeId = true;
        }
        else
        {
            info.hasVolumeId = GetFileSystemStorageVolumeIdByDirPath( info.volumeId, path );
            osData.SetVolumeId( info.volumeId );
        }
    }
    
    if ( !info.hasPartitionId2deviceIdMapping && info.hasVolumeId )
    {
        info.hasPartitionId2deviceIdMapping = GetPhysicalDeviceIdsForVolumeId( info.volumeId, info.partitionId2deviceId );
    }

    info.hasPerfStats = Win32GetVolumePerfStats( osData, info );

    return totalSuccess;


    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    struct statvfs fsStats;
    if ( ::statvfs( path.C_String(), &fsStats ) != 0 )
    {
        // Error in getting file system statistics
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "GetFileSystemStorageVolumeInformationByDirPath: statvfs failed with error code: " + ToString( (UInt32) errno ) );
        return false;
    }

    info.freeBytesAvailableToCaller = fsStats.f_bavail * fsStats.f_bsize;
    info.hasFreeBytesAvailableToCaller = true;
    info.totalNumberOfBytes = fsStats.f_blocks * fsStats.f_bsize;
    info.hasTotalNumberOfBytes = true;
    info.totalNumberOfFreeBytes = fsStats.f_bfree * fsStats.f_bsize;
    info.hasTotalNumberOfFreeBytes = true;
    return true;

    #else

    // Unsupported O/S build
    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "GetFileSystemStorageVolumeInformationByDirPath: Platform has no supported implementation" );
    return false;

    #endif
}

/*-------------------------------------------------------------------------*/
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

CString GUCEF_CORE_PRIVATE_CPP
ExtractGuidSectionFromPath( const CString& path )
{GUCEF_TRACE;

    if ( path.IsNULLOrEmpty() )
        return CString();

    Int32 guidStartIndex = path.HasChar( '{', true );
    if ( guidStartIndex >= 0 )
    {
        Int32 guidEndIndex = path.HasChar( '}', (UInt32) guidStartIndex+1, true );
        if ( guidEndIndex >= 0 )
        {
            return path.SubstrFromRange( (UInt32) guidStartIndex, (UInt32) guidEndIndex+1 );
        }
    }

    return CString::Empty;
}

#endif

/*-------------------------------------------------------------------------*/

bool
GetVolumePathForVolumeId( const CString& volumeId ,
                          CString& volumePath     )
{GUCEF_TRACE;

    volumePath.Clear();
    if ( volumeId.IsNULLOrEmpty() )
        return false;

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    // The volume id is a GUID for which the O/S has a device path concept/convention
    volumePath = ConvertGuidToDirPath( volumeId );
    return true;

    #else

    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
GetFileSystemStorageVolumeInformationByVolumeId_NoClear( CORE::CStorageVolumeInfoOSData& volumeInfoOSData , 
                                                         CStorageVolumeInformation& info                  )
{GUCEF_TRACE;

    if ( volumeInfoOSData.GetVolumeId().IsNULLOrEmpty() )
        return false;

    if ( info.hasVolumeId )
    {
        if ( info.volumeId != volumeInfoOSData.GetVolumeId() )
            return false; // We have a different volume id, for an uncleared info object this is a conflict
    }
    else
    {
        info.volumeId = volumeInfoOSData.GetVolumeId();
        info.hasVolumeId = true;
    }

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    if ( !info.hasPaths )
    {
        if ( GetAllFileSystemPathNamesForVolume( volumeInfoOSData.GetVolumeId(), info.paths ) )
        {
            // even if the list is empty we have a valid object because it can truly be a volume with no paths
            // regardless we did the work to gather the info
            info.hasPaths = true; 
            if ( !info.paths.empty() )
                volumeInfoOSData.SetAnyPathForVolume( *info.paths.begin() );
        }
    }

    // Now that we have the paths we can use the path based version of the function
    return GetFileSystemStorageVolumeInformationByDirPath_NoClear( volumeInfoOSData, info );

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    if ( !info.hasPaths )
    {
        info.hasPaths = GetAllFileSystemPathNamesForVolume( volumeId, info.paths );
        if ( info.hasPaths && !info.paths.empty() )
            volumeInfoOSData.SetAnyPathForVolume( *info.paths.begin() );
    }
    
    // Now that we have the paths we can use the path based version of the function
    return GetFileSystemStorageVolumeInformationByDirPath_NoClear( info, path );

    #else

    // Unsupported O/S build
    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "GetFileSystemStorageVolumeInformationByVolumeId: Platform has no supported implementation" );
    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
GetFileSystemTotalStorageVolumeInformation( CStorageVolumeInfoOSData& volumeInfoOSData , 
                                            CStorageVolumeInformation& info            )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // Query the PDH API to get the running totals
    // An alternative would be to sum it all ourselves but why do that if PDH can do it for us
    CWindowsComponentAccess* winComponentAccess = CWindowsComponentAccess::Instance();
    CPdhAccess* pdhApi = winComponentAccess->GetPdhAccess();
    if ( GUCEF_NULL != pdhApi && pdhApi->IsValid() )
    {        
        bool hasNewStats = false;
        Int32 errorCode = pdhApi->GetTotalLogicalVolumePerfStats( volumeInfoOSData.GetOsData()->volumePerfStatAccess, info, hasNewStats );
        return 0 == errorCode;
    }
    return false;

    #else

    // Unsupported O/S build
    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "GetFileSystemTotalStorageVolumeInformation: Platform has no supported implementation" );
    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool GUCEF_CORE_PRIVATE_CPP
GetFileSystemStorageVolumeInformation_NoClear( CStorageVolumeInfoOSData& volumeInfoOSData , 
                                               CStorageVolumeInformation& info            )
{GUCEF_TRACE;

    // Prefer the path version if we have a path to work with
    if ( !volumeInfoOSData.GetAnyPathForVolume().IsNULLOrEmpty() )  
        return GetFileSystemStorageVolumeInformationByDirPath_NoClear( volumeInfoOSData, info );
    else
        return GetFileSystemStorageVolumeInformationByVolumeId_NoClear( volumeInfoOSData, info );    
}

/*-------------------------------------------------------------------------*/

bool
GetFileSystemStorageVolumeInformation( CStorageVolumeInfoOSData& volumeInfoOSData , 
                                       CStorageVolumeInformation& info            )
{GUCEF_TRACE;

    return GetFileSystemStorageVolumeInformation_NoClear( volumeInfoOSData, info );
}

/*-------------------------------------------------------------------------*/
                                                                           
bool
GetFileSystemStorageVolumeInformationByVolumeId( CStorageVolumeInformation& info, const CString& volumeId )
{GUCEF_TRACE;

    info.Clear();
    CORE::CStorageVolumeInfoOSData volumeInfoOSData;
    volumeInfoOSData.SetVolumeId( volumeId );
    return GetFileSystemStorageVolumeInformation_NoClear( volumeInfoOSData, info );
}

/*-------------------------------------------------------------------------*/

bool
GetFileSystemStorageVolumeInformationByDirPath( CStorageVolumeInformation& info, const CString& path )
{GUCEF_TRACE;

    info.Clear();
    CStorageVolumeInfoOSData volumeInfoOSData;
    volumeInfoOSData.SetAnyPathForVolume( path );
    return GetFileSystemStorageVolumeInformation_NoClear( volumeInfoOSData, info );
}

/*-------------------------------------------------------------------------*/

bool
GetFileSystemStorageVolumeIdByDirPath( CString& volumeId, const CString& path )
{GUCEF_TRACE;

    volumeId.Clear();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    CString dirPath = RelativePath( path );
    if ( dirPath.Length() != dirPath.HasChar( '\\', false )+1 )
    {
        // A trailing backslash is required for the below Win32 API
        dirPath += '\\';
    }

    std::wstring wActualPath = ToWString( dirPath );
    if ( TRUE != ::PathIsRootW( wActualPath.c_str() ) )
    {
        // the win32 function below only works with roots
        if ( TRUE != ::PathStripToRootW( (wchar_t*) wActualPath.c_str() ) )
        {
            // failed to determine root
            return false;
        }
    }

    wchar_t volumeNameBuffer[ MAX_PATH+1 ];
    BOOL result = ::GetVolumeNameForVolumeMountPointW( wActualPath.c_str()     ,
                                                       volumeNameBuffer        ,
                                                       MAX_PATH                );
    if ( TRUE == result )
    {
        volumeId = volumeNameBuffer;
        return true;
    }
    return false;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    return false;

    #else

    // Unsupported O/S build
    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "GetFileSystemStorageVolumeIdByDirPath: Platform has no supported implementation" );
    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
GetAllFileSystemStorageVolumes( CString::StringSet& volumeIds )
{GUCEF_TRACE;

    volumeIds.clear();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    WCHAR volNameBuffer[ MAX_PATH ];
    memset( volNameBuffer, 0, sizeof( volNameBuffer ) );

    HANDLE volumeFindHandle = ::FindFirstVolumeW( volNameBuffer, MAX_PATH );
    if ( INVALID_HANDLE_VALUE == volumeFindHandle )
    {
        DWORD errorCode = ::GetLastError();
        if ( ERROR_NO_MORE_FILES == errorCode )
            return true;

        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "GetAllFileSystemStorageVolumes: FindFirstVolumeW failed with error code: " + ToString( (UInt32) errorCode ) );
        return false;
    }

    bool findMoreVolumes = true;
    do
    {
        // These Windows API functions actually return the volume GUID path
        // not the volume ID by itself. As such we will strip the path part to get the volume ID
        CString volumePath = ToString( volNameBuffer );
        CString volumeGuid = ExtractGuidSectionFromPath( volumePath );

        volumeIds.insert( volumeGuid );

        if ( 0 == ::FindNextVolumeW( volumeFindHandle, volNameBuffer, MAX_PATH ) )
        {
            if ( ERROR_NO_MORE_FILES == ::GetLastError() )
                findMoreVolumes = false;
        }
    }
    while ( findMoreVolumes );
    ::FindVolumeClose( volumeFindHandle );

    return true;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    // A storage volume is a mounted filesystem

    CFileSystemIterator fsIterator;
    if ( fsIterator.FindFirst( "/dev/disk/by-uuid/" ) )
    {
        do
        {
            CString fsId = fsIterator.GetResourceName();
            volumeIds.insert( fsId );
        }
        while ( fsIterator.FindNext() );

        return true;
    }
    else
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "GetAllFileSystemStorageVolumes: Failed to open directory: /dev/disk/by-uuid/ switching to fallback" );

        TLinuxProcMountsInfoVector mounts;
        if ( ParseLinuxProcMounts( mounts ) )
        {
            bool foundAnyViaMounts = false;
            for ( size_t i=0; i<mounts.size(); ++i )
            {
                CLinuxProcMountsInfo& mountInfo = mounts[ i ];

                // This functionality is intended to work with storage volumes the way a user interprets them
                // as such we only consider a subset, the actual 'disk' devices
                if ( mountInfo.device.StartsWith( "/dev/" ) )
                {
                    CString uuid;
                    if ( GetDeviceUUIDForDeviceName( mountInfo.device, uuid ) )
                    {
                        volumeIds.insert( uuid );
                        foundAnyViaMounts = true;
                    }
                }
            }
            return foundAnyViaMounts;
        }

        return false;
    }

    #else

    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
GetAllFileSystemPathNamesForVolume( const CString& volumeId       ,
                                    CString::StringSet& pathNames )
{GUCEF_TRACE;

    pathNames.clear();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    std::wstring volumeIdWide = ToWString( ConvertGuidToDirPath( volumeId ) );

    // first get the size of the buffer we need
    DWORD listSizeInWChars = 0;
    BOOL success = ::GetVolumePathNamesForVolumeNameW( volumeIdWide.c_str() ,
                                                       NULL                 ,
                                                       0                    ,
                                                       &listSizeInWChars    );
    if ( success == TRUE )
    {
        // there is nothing to fetch
        // if there is data we would expect ERROR_MORE_DATA
        return true;
    }
    DWORD errorCode = ::GetLastError();
    if ( ERROR_MORE_DATA != errorCode )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "GetAllFileSystemPathNamesForVolume: GetVolumePathNamesForVolumeNameW failed with error code: " + ToString( (UInt32) errorCode ) );
        return false;
    }

    UInt32 requiredBufferBytes = listSizeInWChars * sizeof( wchar_t );
    CORE::CDynamicBuffer volumePathsBuffer( requiredBufferBytes, true );
    volumePathsBuffer.SetBytes( 0 );
    volumePathsBuffer.SetDataSize( requiredBufferBytes );

    // Get the volume paths
    success = ::GetVolumePathNamesForVolumeNameW( volumeIdWide.c_str()                     ,
                                                  volumePathsBuffer.AsTypePtr< wchar_t >() ,
                                                  volumePathsBuffer.GetBufferSize()        ,
                                                  &listSizeInWChars                        );
    if ( success == TRUE )
    {
        wchar_t separatorCodePoint = 0;
        volumePathsBuffer.ParseUniqueWStringElements( separatorCodePoint, pathNames, false );

        GUCEF_DEBUG_LOG( LOGLEVEL_BELOW_NORMAL, "GetAllFileSystemPathNamesForVolume: GetVolumePathNamesForVolumeNameW Found " + ToString( pathNames.size() ) + " paths for volume " + volumeId );
        return true;
    }
    else
    {
        DWORD errorCode = ::GetLastError();
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "GetAllFileSystemPathNamesForVolume: GetVolumePathNamesForVolumeNameW failed with error code: " + ToString( (UInt32) errorCode ) );
        return false;
    }

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    if ( !volumeId.StartsWith( "fakeuuid:" ) )
    {
        CString fsPathByUuid = CombinePath( "/dev/disk/by-uuid/", volumeId );
        CString fsDevPath;
        if ( TryResolveLinuxSymlinkPath( fsPathByUuid, fsDevPath ) )
        {
            CString deviceId = LastSubDir( fsDevPath );

            TLinuxProcMountsInfoVector mounts;
            if ( ParseLinuxProcMounts( mounts ) )
            {
                TLinuxProcMountsInfoVector::iterator i = mounts.begin();
                while ( i != mounts.end() )
                {
                    const CLinuxProcMountsInfo& info = (*i);
                    CString mountDeviceId = LastSubDir( info.device );
                    if ( mountDeviceId == deviceId )
                    {
                        pathNames.insert( info.mountPoint );
                    }
                    ++i;
                }
                return true;
            }
        }
    }
    else
    {
        // since we have a fake uuid we need to go based on device name
        CString deviceName;
        bool hasVersionInfo = false;
        int majorVersion = 0;
        int minorVersion = 0;
        if ( ParseFakeUUID( volumeId, deviceName, hasVersionInfo, majorVersion, minorVersion ) )
        {
            TLinuxProcMountsInfoVector mounts;
            if ( ParseLinuxProcMounts( mounts ) )
            {
                bool itemsAdded = false;
                TLinuxProcMountsInfoVector::const_iterator i = mounts.begin();
                while ( i != mounts.end() )
                {
                    const CLinuxProcMountsInfo& info = (*i);
                    CString mountDeviceId = LastSubDir( info.device );
                    if ( mountDeviceId == deviceName )
                    {
                        pathNames.insert( info.mountPoint );
                        itemsAdded = true;
                    }
                    ++i;
                }
                return itemsAdded;
            }
        }
    }
    return false;

    #else

    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
GetAllFileSystemMountPointsForVolume( const CString& volumeId         ,
                                      CString::StringSet& mountPoints )
{GUCEF_TRACE;

    mountPoints.clear();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    std::wstring volumeIdWide = ToWString( ConvertGuidToDirPath( volumeId ) );

    const DWORD MOUNT_POINT_BUFFER_SIZE = 1024;
    WCHAR volMountBuffer[ MOUNT_POINT_BUFFER_SIZE ];
    memset( volMountBuffer, 0, sizeof( volMountBuffer ) );

    // Get the Volume mount points as the windows equivalent of symlink based mounts
    // Volume mount points are alternative locations in the file system where volumes are mounted,
    // providing a way to access volumes indirectly or integrate them into the directory structure of another volume.

    HANDLE mountFindHandle = ::FindFirstVolumeMountPointW( volumeIdWide.c_str(), volMountBuffer, MOUNT_POINT_BUFFER_SIZE );
    if ( INVALID_HANDLE_VALUE == mountFindHandle )
    {
        DWORD errorCode = ::GetLastError();
        if ( ERROR_NO_MORE_FILES == errorCode )
            return true;

        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "GetAllFileSystemMountPointsForVolume: FindFirstVolumeMountPointW failed with error code: " + ToString( (UInt32) errorCode ) );
        return false;
    }

    bool findMoreMounts = true;
    do
    {
        mountPoints.insert( ToString( volMountBuffer ) );

        if ( 0 == ::FindNextVolumeMountPointW( mountFindHandle, volMountBuffer, MAX_PATH ) )
        {
            if ( ERROR_NO_MORE_FILES == ::GetLastError() )
                findMoreMounts = false;
        }
    }
    while ( findMoreMounts );
    ::FindVolumeMountPointClose( mountFindHandle );

    return true;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    return false;

    #else

    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

// Structure used to store dir iteration data which is O/S dependent
class CFileSystemIterator::CFileSystemIteratorOsData
{
 	public:

 	bool isActive;            // Flag indicating if the iterator is active
    CString rootPath;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

	intptr_t find_handle;        // Unique handle identifying the file or set of files that resulted from a findfirst with the filter provided
	struct _wfinddata64_t find;  // struct that stores entry data

	CFileSystemIteratorOsData( void )
        : isActive( false )  // we start with an inactive iterator. A call to FindFirst will activate it potentially
        , find_handle( 0 )
	{GUCEF_TRACE;

        memset( &find, 0, sizeof find );
	}

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

    DIR* dir;                   // Directory stream
    struct dirent* entry;       // Pointer needed for functions to iterating directory entries. Stores entry name which is used to get stat
    struct stat statinfo;       // Struct needed for determining info about an entry with stat().
    CString linkedPath;         // Stores the resolved symlink linked path if any
    struct stat linkedStatinfo; // Struct needed for determining info about an entry with stat().
    CString resourceGlobFilter; // For Linux we need to keep track of the glob pattern filter ourselves

	CFileSystemIteratorOsData( void )
        : isActive( false )
        , rootPath()
        , dir( GUCEF_NULL )
        , entry( GUCEF_NULL )
        , statinfo()
        , linkedPath()
        , linkedStatinfo()
        , resourceGlobFilter()
	{GUCEF_TRACE;

        memset( &statinfo, 0, sizeof statinfo );
        memset( &linkedStatinfo, 0, sizeof statinfo );
	}

    void
    ClearSymlinkInfo( void )
    {GUCEF_TRACE;

        linkedPath.Clear();
        memset( &linkedStatinfo, 0, sizeof statinfo );
    }

    bool
    ResolveSymlink( void )
    {GUCEF_TRACE;

        if ( GUCEF_NULL != entry && S_ISLNK( statinfo.st_mode ) > 0 )
        {
            if ( linkedPath.IsNULLOrEmpty() )
            {
                CString symlinkPath = entry->d_name;
                if ( TryResolveLinuxSymlinkPath( symlinkPath, linkedPath ) )
                {
                    // Get info on the entry.
                    if ( stat( linkedPath.C_String(), &linkedStatinfo ) >= 0 )
                    {
                        return true;
                    }
                }

                ClearSymlinkInfo();
                return false;
            }

            // Use cached info
            // Expectation is that ClearSymlinkInfo was used for every new entry
            return true;
        }
        else
        {
            ClearSymlinkInfo();
        }
        return false;
    }

    #else

    // -> empty struct because we don't support other OS's atm
    #error Unsupported OS

    #endif
};

/*-------------------------------------------------------------------------*/

CFileSystemIterator::CFileSystemIterator( void )
    : m_osData( GUCEF_NEW CFileSystemIteratorOsData() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFileSystemIterator::~CFileSystemIterator()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_osData )
    {
        FindClose();
        GUCEF_DELETE m_osData;
        m_osData = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemIterator::FindFirst( const CString& path )
{GUCEF_TRACE;

    if ( path.IsNULLOrEmpty() )
        return false;
    if ( GUCEF_NULL == m_osData || m_osData->isActive )
        return false;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    /*
     *	In Win32 we use _findfirst ect. because even though the posix
     *	functions are supported on windows NT they are not supported on
     *	Win98 and WinME and these functions are. No support for Win95 or
     *	older though.
     */

    if ( -1 == path.HasChar( '*', false ) )
    {
       m_osData->rootPath = path;
        CString filterPath = CombinePath( path, "*.*" );
       std::wstring wFilterPath = ToWString( filterPath );
       m_osData->find_handle = _wfindfirst64( wFilterPath.c_str(), &m_osData->find );
    }
    else
    {
        m_osData->rootPath = StripFilename( path );
        std::wstring wPath = ToWString( path );
        m_osData->find_handle = _wfindfirst64( wPath.c_str(), &m_osData->find );
    }

    // Check if findfirst was successful
    if ( m_osData->find_handle == -1 )
    {
        // There was an error
        _findclose( m_osData->find_handle );
        m_osData->find_handle = 0;

        m_osData->rootPath.Clear();
        return false;
    }

    /*
     *	Successfully obtained first entry
     */
    m_osData->isActive = true;
    return true;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

    /*
     *	In Linux we use POSIX functions because these are independant of
     *	the Linux distribution. It may also provide use with support for
     *	other Unix based systems.
     */

    // On Linux we cannot just pass along the glob pattern filter if there is one
    // we need to keep track of and implement that piece ourselves
    // Do keep in mind that * is a valid char in a path so at minimum the * should come after the dir seperator
    Int32 globIndex = path.HasChar( '*', false );
    Int32 dirSepCharIndex = path.HasChar( GUCEF_DIRSEPCHAR, false );
    if ( -1 < globIndex && -1 < dirSepCharIndex && globIndex > dirSepCharIndex )
    {
        m_osData->resourceGlobFilter = path.SubstrToIndex( dirSepCharIndex+1, false );
        m_osData->rootPath = path.SubstrToIndex( dirSepCharIndex, true );
    }
    else
    {
        m_osData->resourceGlobFilter.Clear();
        m_osData->rootPath = path;
    }

    // Attempt to open the directory
    m_osData->dir = ::opendir( m_osData->rootPath.C_String() );
    if ( GUCEF_NULL == m_osData->dir )
    {
        // Could not open directory
	    m_osData->rootPath.Clear();
        m_osData->resourceGlobFilter.Clear();
        return false;
    }
    m_osData->ClearSymlinkInfo();

    // change working dir to be able to read file information
    ::chdir( m_osData->rootPath.C_String() );

    // Read first entry
    m_osData->entry = ::readdir( m_osData->dir );
    if ( GUCEF_NULL != m_osData->entry )
    {
        // Normally the first 2 entries are always '.' and '..'
        // We dont want to complicate the caller's lives so skip them here
        // We also need to take glob patterns into account
        bool isSkipable = false;
        do
        {
            isSkipable = false;
            if ( ( 0 == strcmp( ".", m_osData->entry->d_name ) )  ||
                 ( 0 == strcmp( "..", m_osData->entry->d_name ) ) )
            {
                isSkipable = true;
                m_osData->entry = ::readdir( m_osData->dir );
            }
            else
            if ( !m_osData->resourceGlobFilter.IsNULLOrEmpty() )
            {
                CString entryName( m_osData->entry->d_name );
                if ( !entryName.WildcardEquals( m_osData->resourceGlobFilter, '*', true, false ) )
                {
                    // Skip because we have a pattern to match and the current entry is not a match
                    isSkipable = true;
                    m_osData->entry = ::readdir( m_osData->dir );
                }
            }
        }
        while ( isSkipable && ( GUCEF_NULL != m_osData->entry ) );

        if ( GUCEF_NULL != m_osData->entry )
        {
            // Get info on the entry.
            ::lstat( m_osData->entry->d_name, &m_osData->statinfo );

            m_osData->isActive = true;
            return true;
        }
    }
    m_osData->isActive = false;
    return false;

    #else

    // Unsupported O/S build
    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "FileSystemIterator:FindFirst: Platform has no supported implementation" );
    return false;

    #endif

}

/*-------------------------------------------------------------------------*/

bool
CFileSystemIterator::FindNext( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_osData || !m_osData->isActive )
        return false;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    return !_wfindnext64( m_osData->find_handle, &m_osData->find );

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

    // Read next entry
    m_osData->ClearSymlinkInfo();
    m_osData->entry = readdir( m_osData->dir );
    if ( GUCEF_NULL != m_osData->entry )
    {
        // We need to take glob patterns into account
        if ( !m_osData->resourceGlobFilter.IsNULLOrEmpty() )
        {
            bool isSkipable = false;
            do
            {
                isSkipable = false;
                CString entryName( m_osData->entry->d_name );
                if ( !m_osData->resourceGlobFilter.WildcardEquals( entryName, '*', true, true ) )
                {
                    // Skip because we have a pattern to match and the current entry is not a match
                    isSkipable = true;
                    m_osData->entry = ::readdir( m_osData->dir );
                }
            }
            while ( isSkipable && ( GUCEF_NULL != m_osData->entry ) );
        }

        if ( GUCEF_NULL != m_osData->entry  )
        {
            // Get info on the entry.
            ::lstat( m_osData->entry->d_name, &m_osData->statinfo );
            return true;
        }
    }

    m_osData->isActive = false;
    return false;

    #else

    // Unsupported O/S build
    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "FileSystemIterator:FindNext: Platform has no supported implementation" );
    #endif

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemIterator::FindClose( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_osData || !m_osData->isActive )
        return false;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    ::_findclose( m_osData->find_handle );
    m_osData->find_handle = GUCEF_NULL;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

    m_osData->ClearSymlinkInfo();
    m_osData->resourceGlobFilter.Clear();
    if ( GUCEF_NULL != m_osData->dir )
    {
        ::closedir( m_osData->dir );
        m_osData->dir = GUCEF_NULL;
    }

    #else

    // Unsupported O/S build
    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "FileSystemIterator:FindClose: Platform has no supported implementation" );
    #endif

    m_osData->isActive = false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemIterator::IsADirectory( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_osData && m_osData->isActive )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

        return ( m_osData->find.attrib & _A_SUBDIR );

        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

        if ( S_ISDIR( m_osData->statinfo.st_mode ) > 0 )
            return true;

        // for symlinks check if the link refers to a file or dir
        if ( S_ISLNK( m_osData->statinfo.st_mode ) > 0 )
        {
            if ( m_osData->ResolveSymlink() )
            {
                if ( S_ISDIR( m_osData->linkedStatinfo.st_mode ) > 0 )
                    return true;
            }
        }

        return false;

        #else

        // Unsupported O/S build
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "FileSystemIterator:IsADirectory: Platform has no supported implementation" );
        #endif
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemIterator::IsAFile( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_osData && m_osData->isActive )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

        return !( m_osData->find.attrib & _A_SUBDIR );

        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

        if ( S_ISREG( m_osData->statinfo.st_mode ) > 0 )
            return true;

        // for symlinks check if the link refers to a file or dir
        if ( S_ISLNK( m_osData->statinfo.st_mode ) > 0 )
        {
            if ( m_osData->ResolveSymlink() )
            {
                if ( S_ISREG( m_osData->linkedStatinfo.st_mode ) > 0 )
                    return true;
            }
        }

        return false;

        #else

        // Unsupported O/S build
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "FileSystemIterator:IsAFile: Platform has no supported implementation" );
        #endif
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemIterator::IsSymlink( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_osData && m_osData->isActive )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

        // @TODO: implement for Windows
        return false;

        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

        // for symlinks check if the link refers to a file or dir
        if ( S_ISLNK( m_osData->statinfo.st_mode ) > 0 )
            return true;
        return false;

        #else

        // Unsupported O/S build
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "FileSystemIterator:IsSymlink: Platform has no supported implementation" );
        #endif
    }

    return false;
}

/*-------------------------------------------------------------------------*/

CString
CFileSystemIterator::GetResourceName( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_osData && m_osData->isActive )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

        return m_osData->find.name;

        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

        // Even though we want to return the name of the symlink we will resolve the link
        // just so we know if its a dir or file which helps us to use the correct string function
        // while similar it matters for how trailing slashes are handled
        if ( S_ISLNK( m_osData->statinfo.st_mode ) > 0 )
        {
            if ( m_osData->ResolveSymlink() )
            {
                if ( S_ISREG( m_osData->linkedStatinfo.st_mode ) > 0 )
                {
                    return ExtractFilename( m_osData->entry->d_name );
                }
                else
                {
                    return LastSubDir( m_osData->entry->d_name );
                }
            }
        }

        if ( S_ISREG( m_osData->statinfo.st_mode ) > 0 )
        {
            return ExtractFilename( m_osData->entry->d_name );
        }
        else
        {
            return LastSubDir( m_osData->entry->d_name );
        }

        #else

        // Unsupported O/S build
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "FileSystemIterator:GetResourceName: Platform has no supported implementation" );
        #endif
    }
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

CString
CFileSystemIterator::GetSymlinkedResourceName( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_osData && m_osData->isActive )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

        // @TODO
        return CString::Empty;

        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

        // We want to return the name of resource beyond the symlink
        if ( S_ISLNK( m_osData->statinfo.st_mode ) > 0 )
        {
            if ( m_osData->ResolveSymlink() )
            {
                if ( S_ISREG( m_osData->linkedStatinfo.st_mode ) > 0 )
                {
                    return ExtractFilename( m_osData->linkedPath );
                }
                else
                {
                    return LastSubDir( m_osData->linkedPath );
                }
            }
        }
        return CString::Empty;

        #else

        // Unsupported O/S build
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "FileSystemIterator:GetResourceName: Platform has no supported implementation" );
        #endif
    }
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

CString
CFileSystemIterator::GetResourcePath( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_osData && m_osData->isActive )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

        return CombinePath( m_osData->rootPath, GetResourceName() );

        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

        return m_osData->entry->d_name;

        #else

        // Unsupported O/S build
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "FileSystemIterator:GetResourcePath: Platform has no supported implementation" );
        #endif
    }
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

CString
CFileSystemIterator::GetSymlinkedResourcePath( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_osData && m_osData->isActive )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

        // @TODO
        return CString::Empty;

        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

        if ( S_ISLNK( m_osData->statinfo.st_mode ) > 0 )
        {
            if ( m_osData->ResolveSymlink() )
            {
                return m_osData->linkedPath;
            }
        }
        return CString::Empty;

        #else

        // Unsupported O/S build
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "FileSystemIterator:GetResourcePath: Platform has no supported implementation" );
        #endif
    }
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemIterator::TryReadMetaData( CResourceMetaData& metaData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_osData && m_osData->isActive )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

        metaData.resourceSizeInBytes = m_osData->find.size;
        metaData.hasResourceSizeInBytes = true;
        metaData.creationDateTime = CDateTime( m_osData->find.time_create, true );
        metaData.hasCreationDateTime = true;
        metaData.modifiedDateTime = CDateTime( m_osData->find.time_access, true );
        metaData.hasModifiedDateTime = true;
        metaData.lastAccessedDateTime = CDateTime( m_osData->find.time_write, true );
        metaData.hasLastAccessedDateTime = true;

        m_osData->find.attrib & FILE_ATTRIBUTE_READONLY ? metaData.isReadOnly = true : metaData.isReadOnly = false;
        metaData.hasIsReadOnly = true;
        m_osData->find.attrib & FILE_ATTRIBUTE_HIDDEN ? metaData.isHidden = true : metaData.isHidden = false;
        metaData.hasIsHidden = true;
        m_osData->find.attrib & FILE_ATTRIBUTE_SYSTEM || m_osData->find.attrib & FILE_ATTRIBUTE_DEVICE ? metaData.isSystemResource = true : metaData.isSystemResource = false;
        metaData.hasIsSystemResource = true;
        m_osData->find.attrib & FILE_ATTRIBUTE_ARCHIVE ? metaData.isArchive = true : metaData.isArchive = false;
        metaData.hasIsArchive = true;
        m_osData->find.attrib & FILE_ATTRIBUTE_COMPRESSED ? metaData.isCompressed = true : metaData.isCompressed = false;
        metaData.hasIsCompressed = true;
        m_osData->find.attrib & FILE_ATTRIBUTE_ENCRYPTED ? metaData.isEncrypted = true : metaData.isEncrypted = false;
        metaData.hasIsEncrypted = true;
        m_osData->find.attrib & FILE_ATTRIBUTE_TEMPORARY ? metaData.isTemporary = true : metaData.isTemporary = false;
        metaData.hasIsTemporary = true;
        m_osData->find.attrib & FILE_ATTRIBUTE_OFFLINE ? metaData.isOffline = true : metaData.isOffline = false;
        metaData.hasIsOffline = true;

        metaData.name = GetResourceName();
        metaData.hasName = true;

        return true;

        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

        metaData.creationDateTime = CDateTime( m_osData->statinfo.st_ctime, true );
        metaData.hasCreationDateTime = true;
        metaData.modifiedDateTime = CDateTime( m_osData->statinfo.st_mtim, true );
        metaData.hasModifiedDateTime = true;
        metaData.lastAccessedDateTime = CDateTime( m_osData->statinfo.st_atim, true );
        metaData.hasLastAccessedDateTime = true;
        metaData.resourceSizeInBytes = (UInt64) m_osData->statinfo.st_size;
        metaData.hasResourceSizeInBytes = true;

        metaData.name = GetResourceName();
        metaData.hasName = true;

        return true;

        #else

        /*
         *	Unsupported O/S build
         */
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "FileSystemIterator:TryReadMetaData: Platform has no supported implementation" );
        #endif
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemIterator::TryReadSymlinkedMetaData( CResourceMetaData& metaData )
{GUCEF_TRACE;

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

    if ( S_ISLNK( m_osData->statinfo.st_mode ) > 0 )
    {
        if ( m_osData->ResolveSymlink() )
        {
            metaData.creationDateTime = CDateTime( m_osData->linkedStatinfo.st_ctime, true );
            metaData.hasCreationDateTime = true;
            metaData.modifiedDateTime = CDateTime( m_osData->linkedStatinfo.st_mtim, true );
            metaData.hasModifiedDateTime = true;
            metaData.lastAccessedDateTime = CDateTime( m_osData->linkedStatinfo.st_atim, true );
            metaData.hasLastAccessedDateTime = true;
            metaData.resourceSizeInBytes = (UInt64) m_osData->linkedStatinfo.st_size;
            metaData.hasResourceSizeInBytes = true;

            return true;
        }
    }

    return false;

    #else

    /*
     *	Unsupported O/S build
     */
    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "FileSystemIterator:TryReadSymlinkMetaData: Platform has no supported implementation" );
    return false;
    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
