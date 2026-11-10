/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include <string.h>            /* needed for memcpy() */
#include <errno.h>

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"       /* cross-platform file utils */
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h" 
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */ 

#include "CFileAccess.h"      /* definition of the class implemented here */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */ 

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    #include <io.h>
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/stat.h>          /* needed for _fstat64() */
    #define GUCEF_FILEACCESS_POSIX_LIKE_FSTAT
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) || \
       ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPHONEOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPADOS ) || \
       ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_TVOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_VISIONOS ) )
    #include <stdlib.h>
    #include <stdio.h>
    #include <unistd.h>            /* needed for fsync() */
    #include <fcntl.h>             /* needed for F_FULLFSYNC on the Apple platforms */
    #include <sys/stat.h>          /* needed for fstat() */
    #define GUCEF_FILEACCESS_POSIX_LIKE_FSTAT
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

// Offset capable (64 bit where the platform allows) stdio positioning
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    #define GUCEF_FILEACCESS_FSEEK( file, offset, origin )  _fseeki64( (file), (__int64) (offset), (origin) )
    #define GUCEF_FILEACCESS_FTELL( file )                  _ftelli64( (file) )
#else
    #define GUCEF_FILEACCESS_FSEEK( file, offset, origin )  fseeko( (file), (off_t) (offset), (origin) )
    #define GUCEF_FILEACCESS_FTELL( file )                  ftello( (file) )
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Determines the size of an open file: of the file the stream has open, never of a path (which may name another file
 *  by now, after a replace by rename). From the descriptor (fstat) for a regular file, else by seeking to the end of the
 *  stream with the position restored. A stream that was written to must be flushed first.
 */
static UInt64
GetSizeOfOpenFile( FILE* file )
{GUCEF_TRACE;

    if ( GUCEF_NULL == file )
        return 0;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    struct _stat64 info;
    if ( 0 == _fstat64( _fileno( file ), &info ) && 0 != ( info.st_mode & _S_IFREG ) )
        return info.st_size > 0 ? (UInt64) info.st_size : 0;
    #elif defined( GUCEF_FILEACCESS_POSIX_LIKE_FSTAT )
    struct stat info;
    if ( 0 == fstat( fileno( file ), &info ) && S_ISREG( info.st_mode ) )
        return info.st_size > 0 ? (UInt64) info.st_size : 0;
    #endif

    fflush( file );
    const Int64 originalPos = (Int64) GUCEF_FILEACCESS_FTELL( file );
    if ( originalPos < 0 || 0 != GUCEF_FILEACCESS_FSEEK( file, 0, SEEK_END ) )
        return 0;

    const Int64 size = (Int64) GUCEF_FILEACCESS_FTELL( file );
    GUCEF_FILEACCESS_FSEEK( file, originalPos, SEEK_SET );
    return size > 0 ? (UInt64) size : 0;
}

/*-------------------------------------------------------------------------*/

CFileAccess::CFileAccess( void )
    : CIOAccess()    
    , _writeable( false )
    , _readable( false )
    , m_mode()       
    , m_file( GUCEF_NULL )
    , m_filename()   
    , _size( 0 )    
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFileAccess::CFileAccess( const CString& file           ,
                          const char* mode /* = "rb" */ )
    : CIOAccess()
    , _writeable( false )
    , _readable( false )
    , m_mode()       
    , m_file( GUCEF_NULL )
    , m_filename()   
    , _size( 0 )    
{GUCEF_TRACE;

    Open( file, mode );
}

/*-------------------------------------------------------------------------*/

CFileAccess::CFileAccess( const CString& file               ,
                          const CResourceMetaData& metaData ,
                          const char* mode                  )
    : CIOAccess()
    , _writeable( false )
    , _readable( false )
    , m_mode()       
    , m_file( GUCEF_NULL )
    , m_filename()   
    , _size( 0 ) 
{GUCEF_TRACE;

    Open( file, metaData, mode );
}

/*-------------------------------------------------------------------------*/

CFileAccess::~CFileAccess()
{GUCEF_TRACE;

    Close();    
}

/*-------------------------------------------------------------------------*/

CString
CFileAccess::GetErrorString( int errorCode )
{GUCEF_TRACE;

    switch ( errorCode )
    {
        case EACCES:
            return "EACCES: Search permission is denied on a component of the path prefix, or the file exists and the permissions specified by mode are denied, or the file does not exist and write permission is denied for the parent directory of the file to be created.";
        case EINTR:
            return "EINTR: A signal was caught during fopen()";
        case EISDIR:
            return "EISDIR: The named file is a directory and mode requires write access";
        case ELOOP:
            return "ELOOP: A loop exists in symbolic links encountered during resolution of the path argument";
        case EMFILE:
            return "EMFILE: {OPEN_MAX} file descriptors are currently open in the calling process";
        case ENAMETOOLONG:
            return "ENAMETOOLONG: The length of the filename argument exceeds {PATH_MAX} or a pathname component is longer than {NAME_MAX}";
        case ENFILE:
            return "ENFILE: The maximum allowable number of files is currently open in the system";
        case ENOENT:
            return "ENOENT: A component of filename does not name an existing file or filename is an empty string";
        case ENOSPC:
            return "ENOSPC: The directory or file system that would contain the new file cannot be expanded, the file does not exist, and the file was to be created";
        case ENOTDIR:
            return "ENOTDIR: A component of the path prefix is not a directory";
        case ENXIO:
            return "ENXIO: The named file is a character special or block special file, and the device associated with this special file does not exist";
        case EOVERFLOW:
            return "EOVERFLOW: The named file is a regular file and the size of the file cannot be represented correctly in an object of type off_t";
        case EROFS:
            return "EROFS: The named file resides on a read-only file system and mode requires write access";
        default:
            return Int32ToString( errorCode );
    }
}

/*-------------------------------------------------------------------------*/

void 
CFileAccess::Open( void )
{GUCEF_TRACE;

    Close();
        
    m_file = fopen( m_filename.C_String() ,
                    m_mode.C_String()    );                      
}

/*-------------------------------------------------------------------------*/

bool
CFileAccess::Open( const CString& file ,
                   const char* mode    )
{GUCEF_TRACE;

    Close();
    
    m_filename = RelativePath( file );
    m_mode = mode;
    
    _readable = ( strchr( mode, 'r' ) != GUCEF_NULL ) || ( strchr( mode, 'a' ) != GUCEF_NULL );
    _writeable = ( strchr( mode, 'w' ) != GUCEF_NULL ) || ( strchr( mode, 'a' ) != GUCEF_NULL );

    if ( _writeable )
    {
        // Note that an empty path means the file is in the current working directory
        CString path = StripFilename( m_filename );
        if ( !path.IsNULLOrEmpty() && !DirExists( path ) )
        {
            if ( !CreateDirs( path ) )
            {
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "FileAccess:Open: Failed to recursively create directories" );
                return false;
            }
        }
    }
    if ( _readable )
    {
        _size = FileSize( file );
    }
    
    errno = 0;
    m_file = fopen( m_filename.C_String(), mode );
    if ( m_file == GUCEF_NULL && 0 != errno )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "FileAccess:Open: Failed to open file \"" + file + "\". mode=" + CString(mode) + " error=" + GetErrorString( errno ) );
    }
    
    return m_file != GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CFileAccess::Open( const CString& file               ,
                   const CResourceMetaData& metaData ,
                   const char* mode                  )
{GUCEF_TRACE;

    Close();
    
    m_filename = RelativePath( file );
    m_mode = mode;
    
    _readable = ( strchr( mode, 'r' ) != NULL ) || ( strchr( mode, 'a' ) != NULL );
    _writeable = ( strchr( mode, 'w' ) != NULL ) || ( strchr( mode, 'a' ) != NULL );

    if ( _writeable )
    {
        // Note that an empty path means the file is in the current working directory
        CString path = StripFilename( file );
        if ( !path.IsNULLOrEmpty() && !DirExists( path ) && !CreateDirs( path ) )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "FileAccess:Open: Failed to recursively create directories" );
            return false;
        }
    }
    if ( _readable )
    {
        _size = FileSize( file );
    }
    
    errno = 0;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // Get the current file attributes
    // we only want to change the flags we have and keep the rest as-is
    CResourceMetaData originalMetaData;
    if ( FileExists( m_filename ) )
    {
        if ( !GetFileMetaData( m_filename, originalMetaData ) )
            return false;
    }

    std::wstring wFilename = ToWString( m_filename );

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

    // Open using Win32
    HANDLE hFile = INVALID_HANDLE_VALUE;
    if ( _readable && !_writeable )
        hFile = ::CreateFileW( wFilename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, data.dwFileAttributes, NULL );
    else if ( _readable && _writeable )
        hFile = ::CreateFileW( wFilename.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, data.dwFileAttributes, NULL );
    else if ( !_readable && _writeable )
        hFile = ::CreateFileW( wFilename.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, data.dwFileAttributes, NULL );

    if ( INVALID_HANDLE_VALUE == hFile ) 
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "FileAccess:Open: Failed to open handle to file \"" + file + "\". mode=" + CString(mode) + " error=" + GetErrorString( errno ) );
        return false;
    }

    // Convert date time to windows file time as needed
    metaData.hasCreationDateTime ? data.ftCreationTime = metaData.creationDateTime.ToWindowsFiletime() : data.ftCreationTime = originalMetaData.creationDateTime.ToWindowsFiletime();
    metaData.hasModifiedDateTime ? data.ftLastWriteTime = metaData.modifiedDateTime.ToWindowsFiletime() : data.ftLastWriteTime = originalMetaData.modifiedDateTime.ToWindowsFiletime();
    metaData.hasLastAccessedDateTime ? data.ftLastAccessTime = metaData.lastAccessedDateTime.ToWindowsFiletime() : data.ftLastAccessTime = originalMetaData.lastAccessedDateTime.ToWindowsFiletime();

    if ( 0 == ::SetFileTime( hFile, &data.ftCreationTime, &data.ftLastAccessTime, &data.ftLastWriteTime ) )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "FileAccess:Open: SetFileTime failed with error code: " + ToString( (UInt32) ::GetLastError() ) );
    }

    // Convert HANDLE to file descriptor
    // This transfers ownership of the HANDLE to the file descriptor
    int fd = ::_open_osfhandle( (intptr_t) hFile, _O_RDONLY );
    if ( fd == -1 ) 
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "FileAccess:Open: Failed to convert HANDLE to file descriptor" );
        ::CloseHandle( hFile );
        hFile = INVALID_HANDLE_VALUE;
        return false;
    }

    // Convert file descriptor to FILE*
    // This transfers ownership of the file descriptor to the FILE*
    m_file = ::_fdopen( fd, mode );
    if ( m_file == NULL) 
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "FileAccess:Open: Failed to convert file descriptor to FILE*" );
        _close( fd );
    }
    
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

    #else

    /*
     *  Unsupported platform
     */

    #endif

    if ( m_file == NULL && 0 != errno )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "FileAccess:Open: Failed to open file with meta-data \"" + file + "\". mode=" + CString(mode) + " error=" + GetErrorString( errno ) );
    }
    
    return m_file != NULL;
}

/*-------------------------------------------------------------------------*/

bool
CFileAccess::OpenFromFileDescriptor( int fd             ,
                                     const char* mode   ,
                                     bool takeOwnership )
{GUCEF_TRACE;

    Close();

    m_filename.Clear();
    m_mode = mode;
    _readable = ( strchr( mode, 'r' ) != GUCEF_NULL ) || ( strchr( mode, 'a' ) != GUCEF_NULL );
    _writeable = ( strchr( mode, 'w' ) != GUCEF_NULL ) || ( strchr( mode, 'a' ) != GUCEF_NULL );
    _size = 0;

    if ( fd < 0 )
        return false;

    int useFd = fd;
    if ( !takeOwnership )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        useFd = _dup( fd );
        #else
        useFd = dup( fd );
        #endif

        if ( useFd < 0 )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "FileAccess:OpenFromFileDescriptor: Failed to duplicate file descriptor " << fd );
            return false;
        }
    }

    errno = 0;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    m_file = _fdopen( useFd, mode );
    #else
    m_file = fdopen( useFd, mode );
    #endif

    if ( GUCEF_NULL == m_file )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "FileAccess:OpenFromFileDescriptor: Failed to open file descriptor " << fd << " using mode=" << mode << " error=" << GetErrorString( errno ) );

        // Only the duplicate is ours to clean up, the original is the caller's
        if ( !takeOwnership )
        {
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
            _close( useFd );
            #else
            close( useFd );
            #endif
        }
        return false;
    }

    if ( _readable )
        _size = GetSizeOfOpenFile( m_file );
    return true;
}

/*-------------------------------------------------------------------------*/

void 
CFileAccess::Close( void )
{GUCEF_TRACE;

    if ( NULL != m_file )
    { 
        fclose( m_file );
        m_file = NULL;
    }
}

/*-------------------------------------------------------------------------*/

bool 
CFileAccess::Opened( void ) const
{GUCEF_TRACE;

    return m_file != NULL;
}

/*-------------------------------------------------------------------------*/

UInt32 
CFileAccess::Read( void *dest      ,
                    UInt32 esize    ,
                    UInt32 elements )
{GUCEF_TRACE;
    
    if ( NULL != m_file )
    {
        return (UInt32) fread( dest     ,
                               esize    ,
                               elements ,
                               m_file   );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt64 
CFileAccess::Tell( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_file )
    {
        const Int64 pos = (Int64) GUCEF_FILEACCESS_FTELL( m_file );
        return pos < 0 ? 0 : (UInt64) pos;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

Int32 
CFileAccess::Seek( Int64 offset ,
                   Int32 origin )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_file )
    {
        return GUCEF_FILEACCESS_FSEEK( m_file   ,
                                       offset   ,
                                       origin   );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32 
CFileAccess::Setpos( UInt64 position )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_file )
    {
        return GUCEF_FILEACCESS_FSEEK( m_file    ,
                                       position  ,
                                       SEEK_SET  );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

char 
CFileAccess::GetChar( void )
{GUCEF_TRACE;

    if ( NULL != m_file )
    {
        return (char) fgetc( m_file );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

bool 
CFileAccess::Eof( void ) const
{GUCEF_TRACE;

    if ( NULL != m_file )
    {
        return feof( m_file ) != 0;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CFileAccess::IsReadable( void ) const
{GUCEF_TRACE;

    return _readable;
}

/*-------------------------------------------------------------------------*/

bool 
CFileAccess::IsWriteable( void ) const
{GUCEF_TRACE;

    return _writeable;
}

/*-------------------------------------------------------------------------*/
       
UInt32 
CFileAccess::Write( const void* srcdata ,
                    UInt32 esize        ,
                    UInt32 elements     )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_file )
    {
        return (UInt32) fwrite( srcdata  , 
                                esize    , 
                                elements , 
                                m_file    );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CFileAccess::Write( CIOAccess& sourceData ,
                    Int64 bytesToWrite    )
{GUCEF_TRACE;
    
    return CIOAccess::Write( sourceData, bytesToWrite );
}

/*-------------------------------------------------------------------------*/

UInt32
CFileAccess::Write( const CString& string ,
                    bool addEol           )
{GUCEF_TRACE;

    return CIOAccess::Write( string, addEol );
}

/*-------------------------------------------------------------------------*/

bool 
CFileAccess::IsValid( void )
{GUCEF_TRACE;
    
    // A file opened from a descriptor has no name to check, the open stream is the proof
    if ( m_filename.IsNULLOrEmpty() )
        return GUCEF_NULL != m_file;

    return FileExists( m_filename );
}

/*-------------------------------------------------------------------------*/

UInt64
CFileAccess::GetSize( void ) const
{GUCEF_TRACE;
        
    if ( GUCEF_NULL != m_file )
    {        
        if ( _writeable )
            fflush( m_file );

        // the open file, as fstat() gives it, also when the path names another file by now (as ftell() would answer)
        return GetSizeOfOpenFile( m_file );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

void
CFileAccess::Flush( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_file )
    {
        fflush( m_file );
    }
}

/*-------------------------------------------------------------------------*/

bool
CFileAccess::Sync( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_file )
        return false;

    if ( 0 != fflush( m_file ) )
        return false;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    return 0 == _commit( _fileno( m_file ) );

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

    return 0 == fsync( fileno( m_file ) );

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPHONEOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPADOS ) || \
           ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_TVOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_VISIONOS ) )

    // fsync() on the Apple platforms only hands the data to the drive, F_FULLFSYNC asks the drive to write its cache
    // to the medium; file systems that do not support it fail the call, fsync() is the best they offer then
    const int fd = fileno( m_file );
    return 0 == fcntl( fd, F_FULLFSYNC ) || 0 == fsync( fd );

    #else

    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "CFileAccess::Sync: Platform has no supported implementation" );
    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

CICloneable* 
CFileAccess::Clone( void ) const
{GUCEF_TRACE;

    // Without a path (opened from a descriptor) we cannot open a separate access with its own file offset
    if ( IsReadable() && !m_filename.IsNULLOrEmpty() )
    {
        return GUCEF_NEW CFileAccess( m_filename, "rb" );
    }
    
    // Cannot be cloned
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CString&
CFileAccess::GetFilename( void ) const
{GUCEF_TRACE;

    return m_filename; 
}

/*-------------------------------------------------------------------------*/

bool
CFileAccess::SetFileToUse( const CString& filename  ,
                           const char* mode         ,
                           bool moveIfCurrentlyOpen )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_file )
    {
        // We already have a file open.
        fclose( m_file );
        m_file = NULL;

        if ( moveIfCurrentlyOpen )
        {
            if ( !MoveFile( filename, m_filename, true ) )
                return false;
        }
    }

    m_file = fopen( filename.C_String(), mode );
    if ( GUCEF_NULL != m_file )
    {
        m_filename = filename;
        m_mode = mode;
        return true;
    }

    m_filename.Clear();
    m_mode.Clear();
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
