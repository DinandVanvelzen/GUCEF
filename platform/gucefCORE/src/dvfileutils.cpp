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
#define __STDC_WANT_LIB_EXT1__
#include <string.h>             /* standard string utils */
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef GUCEF_CORE_CONFIG_H
#include "gucefCORE_config.h"     /* build defines */
#define GUCEF_CORE_CONFIG_H
#endif /* GUCEF_CORE_CONFIG_H ? */

#include "dvfileutils.h"	/* function prototypes */

#include "dvstrutils.h"         /* My own string utils */
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <windows.h>		/* WIN32 API */
  #undef min
  #undef max
  /* #include <dir.h>: obsolete *//* needed for MAXFILE define */
  #include <io.h>                 /* Dir itteration: findfirst ect. */
  #include <direct.h>             /* dir tools */
  #define MAX_DIR_LENGTH MAX_PATH
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )
  #include <dirent.h>             /* needed for dirent structure */
  #include <unistd.h>             /* POSIX utilities */
  #include <limits.h>             /* Linux OS limits */
  #include <sys/stat.h>           /* needed for stat function */
  #include <errno.h>
  #include <fcntl.h>
  #if !defined(__EMSCRIPTEN__)
    #include <sys/sendfile.h>     /* Linux-specific zero-copy file transfer */
  #endif
  #define MAX_DIR_LENGTH PATH_MAX
#else
  #error Unsupported OS
#endif
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
  #include <android/log.h>
#endif

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace CORE {
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *	Structure used to store dir iteration data so it can be passed between
 *	the DI_ functions in a OS independant manner.
 */
struct SDI_Data
{
    CFileSystemIterator cppIterator;
    CString entryName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SDI_Data*
DI_First_Dir_Entry( const char* path )
{GUCEF_TRACE;

    // map to C++ version
    try
    {
        struct SDI_Data* data = GUCEF_NEW SDI_Data();
        if ( GUCEF_NULL == data )
            return GUCEF_NULL;

        if ( data->cppIterator.FindFirst( path ) )
        {
            return data;
        }
        else
        {
            GUCEF_DELETE data;
            return GUCEF_NULL;

        }
    }
    catch ( const std::exception& )
    {
        return GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
DI_Next_Dir_Entry( struct SDI_Data* data )
{GUCEF_TRACE;

    // map to C++ version
    try
    {
        if ( GUCEF_NULL == data )
            return 0;
        return data->cppIterator.FindNext() ? 1 : 0;
    }
    catch ( const std::exception& )
    {
        return 0;
    }
}

/*-------------------------------------------------------------------------*/

UInt64
DI_Timestamp( struct SDI_Data* data )
{GUCEF_TRACE;

    // map to C++ version
    try
    {
        if ( GUCEF_NULL == data )
            return 0;

        CResourceMetaData metaData;
        if ( data->cppIterator.TryReadMetaData( metaData ) )
        {
            if ( metaData.hasModifiedDateTime )
            {
                return metaData.modifiedDateTime.ToUnixEpochBasedTicksInMillisecs();
            }
            else if ( metaData.hasCreationDateTime )
            {
                return metaData.creationDateTime.ToUnixEpochBasedTicksInMillisecs();
            }
        }
        return 0;
    }
    catch ( const std::exception& )
    {
        return 0;
    }
}

/*-------------------------------------------------------------------------*/

UInt64
DI_Size( struct SDI_Data* data )
{GUCEF_TRACE;

    // map to C++ version
    try
    {
        if ( GUCEF_NULL == data )
            return 0;

        CResourceMetaData metaData;
        if ( data->cppIterator.TryReadMetaData( metaData ) )
        {
            if ( metaData.hasResourceSizeInBytes )
            {
                return metaData.resourceSizeInBytes;
            }
        }
        return 0;
    }
    catch ( const std::exception& )
    {
        return 0;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
DI_Is_It_A_File( struct SDI_Data* data )
{GUCEF_TRACE;

    // map to C++ version
    try
    {
        if ( GUCEF_NULL == data )
            return 0;

        return data->cppIterator.IsAFile() ? 1 : 0;
    }
    catch ( const std::exception& )
    {
        return 0;
    }
}

/*-------------------------------------------------------------------------*/

const char*
DI_Name( struct SDI_Data* data )
{GUCEF_TRACE;

    // map to C++ version
    try
    {
        if ( GUCEF_NULL == data )
            return 0;

        data->entryName = data->cppIterator.GetResourceName();
        return data->entryName.C_String();
    }
    catch ( const std::exception& )
    {
        return 0;
    }
}

/*-------------------------------------------------------------------------*/

void
DI_Cleanup( struct SDI_Data* data )
{GUCEF_TRACE;

    // map to C++ version
    try
    {
        if ( GUCEF_NULL == data )
            return;

        data->cppIterator.FindClose();
        data->entryName.Clear();
        GUCEF_DELETE data;
    }
    catch ( const std::exception& )
    {
    }
}

/*-------------------------------------------------------------------------*/

char*
Get_Current_Dir( char* dest_buffer, UInt32 buf_length )
{GUCEF_TRACE;

    /*
     *	Function that returns the current directory
     */
    if ( !dest_buffer ) return NULL;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    return _getcwd( dest_buffer, buf_length );

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    /*
     *	This call can actually fail: if another process has succeeded
     *	in removing the current directory of a process
     */
    return getcwd( dest_buffer, buf_length );

    #else

    /*
     *	Unsupported O/S build
     */
    return NULL;

    #endif
}

/*-------------------------------------------------------------------------*/

/**
 *	Function that returns maximum possible length of a directory on the
 *	current O/S.
 */
UInt32
Max_Dir_Length( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    return MAX_PATH;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    return PATH_MAX;

    #else

    /*
     *	Unsupported O/S build
     */
    return 0;

    #endif
}

/*-------------------------------------------------------------------------*/

UInt32
Max_Filename_Length( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    return _MAX_FNAME;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    return NAME_MAX+1;

    #else

    /*
     *	Unsupported O/S build
     */
    return 0;

    #endif
}

/*-------------------------------------------------------------------------*/

UInt32
Create_Directory( const char* new_dir )
{GUCEF_TRACE;

    if ( GUCEF_NULL == new_dir )
        return 0;

    // map to C++ version
    return CreateDirs( new_dir ) ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
Create_Path_Directories( const char* path )
{GUCEF_TRACE;

    // map to C++ version
    return CreatePathDirectories( CString( path ) ) ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

/**
 *      Function that attempts to remove the specified directory.
 *      The del_files parameter allows you to specify wheter you wish to delete
 *      all the files in the directory before deleting the directory itself.
 *      If false and files or dirs are found the directory removal will fail.
 */
UInt32
Remove_Directory( const char *dir  ,
                  UInt32 del_files )
{GUCEF_TRACE;

    if ( del_files )
    {
        /*
         *      The user wishes to delete the directory even if the
         *      directory is not empty. We will proceed to delete all
         *      files in the directory first.
         */
        struct SDI_Data* ddata = DI_First_Dir_Entry( dir );
        UInt32 more = 1;
        if ( GUCEF_NULL == ddata )
            return 0;

        while ( more )
        {
            if ( DI_Is_It_A_File( ddata ) )
            {
                /*
                 *      Attempt to delete the file we found in
                 *      the directory.
                 */
                if ( !Delete_File( DI_Name( ddata ) ) )
                    return 0;
            }
            else
            {
                /*
                 *      Attempt to delete the directory we found
                 *      and all files inside it.
                 */
                size_t nameLength = strlen( DI_Name( ddata ) );
                size_t dirLength = strlen( dir );
                size_t subDirBufferLength = dirLength + nameLength + 1;

                char* subdir = (char*) calloc( subDirBufferLength, 1 );
                if ( GUCEF_NULL != subdir )
                {
                    #if defined( __STDC_LIB_EXT1__ ) || defined( __STDC_WANT_SECURE_LIB__ )
                    strcpy_s( subdir, subDirBufferLength, dir );
                    #else
                    strcpy( subdir, dir );
                    #endif
                    Append_To_Path( subdir, subDirBufferLength, DI_Name( ddata ) );
                    if ( !Remove_Directory( subdir, del_files ) )
                    {
                        free( subdir );
                        return 0;
                    }
                    free( subdir );
                }
            }

            /*
             *      Itterate to next directory entry
             */
            more = DI_Next_Dir_Entry( ddata );
        }

        /*
         *      Clean-up our dir itterator
         */
        DI_Cleanup( ddata );
    }

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    /*
     *      Attempt to remove the directory itself. We can call the WIN32
     *      function directly here because it won't delete any files in the
     *      dir.
     */

    return RemoveDirectory( dir );

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    return 0 == rmdir( dir ) ? 1 : 0;

    #else

    /*
     *      Not implemented for your O/S
     */
    return 0;

    #endif
}

/*-------------------------------------------------------------------------*/

/**
 *      Function that lets you obtain the path to the module that houses this
 *      function. So for example if you are calling this function from your
 *      application while the function is part of a dll you use then the path
 *      to the dll is returned. If the dll is the the same dir as the executable
 *      then this function offers a good portable method to get the path to
 *      the application.
 */
UInt32
Module_Path( char *dest, UInt32 dest_size )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    return !GetModuleFileName( NULL, dest, dest_size );

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE )

    std::string get_basepath() {
    std::string path = "./";
    ProcessSerialNumber PSN;
    ProcessInfoRec pinfo;
    FSSpec pspec;
    FSRef fsr;
    OSStatus err;
    /* set up process serial number */
    PSN.highLongOfPSN = 0;
    PSN.lowLongOfPSN = kCurrentProcess;
    /* set up info block */
    pinfo.processInfoLength = sizeof(pinfo);
    pinfo.processName = NULL;
    pinfo.processAppSpec = &pspec;
    /* grab the vrefnum and directory */
    err = GetProcessInformation(&PSN, &pinfo);
    if (! err ) {
    char c_path[2048];
    FSSpec fss2;
    int tocopy;
    err = FSMakeFSSpec(pspec.vRefNum, pspec.parID, 0, &fss2);
    if ( ! err ) {
    err = FSpMakeFSRef(&fss2, &fsr);
    if ( ! err ) {
    char c_path[2049];
    err = (OSErr)FSRefMakePath(&fsr, (UInt8*)c_path, 2048);
    if (! err ) {
    path = c_path;
    }
    }
    }
    }

    return (path);
    }

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    pid_t pid = getpid();

    char linkStr[ 24 ];
    sprintf( linkStr, "%s%d%s", "/proc/", pid, "/exe\0" );

    int ch = readlink( linkStr, dest, dest_size );
    if ( -1 != ch )
    {
        dest[ ch ] = 0;
        return 0;
    }
    return 1;

    #else

    /*
     *  Not supported
     */
    return 0;

    #endif

}

/*-------------------------------------------------------------------------*/

UInt32
Delete_File( const char *filename )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    return DeleteFileA( filename );

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    return 0 == remove( filename ) ? 1 : 0;

    #else

    /*
     *  Not supported
     */
    return 0;

    #endif
}

/*-------------------------------------------------------------------------*/

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

ssize_t
do_sendfile( int out_fd, int in_fd, off_t offset, size_t count )
{GUCEF_TRACE;

    ssize_t bytes_sent;
    size_t total_bytes_sent = 0;
    while ( total_bytes_sent < count )
    {
        if ( ( bytes_sent = sendfile( out_fd,
                                      in_fd,
                                      &offset,
                                      count - total_bytes_sent ) ) <= 0 )
        {
            if ( errno == EINTR || errno == EAGAIN )
            {
                // Interrupted system call/try again
                // Just skip to the top of the loop and try again
                continue;
            }
            return -1;
        }
        total_bytes_sent += bytes_sent;
    }
    return total_bytes_sent;
}

#endif

/*-------------------------------------------------------------------------*/

UInt32
Copy_File( const char *dst, const char *src )
{GUCEF_TRACE;

    if ( 0 == Create_Path_Directories( dst ) ) return 0;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    return 0 != CopyFileA( src, dst, TRUE ) ? 1 : 0;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    {

    int read_fd;
    int write_fd;
    struct stat stat_buf;
    off_t offset = 0;

    /* Open the input file. */
    read_fd = open( src, O_RDONLY );
    if ( read_fd == -1 ) return 0;

    /* Stat the input file to obtain its size. */
    if ( fstat( read_fd, &stat_buf ) == -1 )  return 0;

    /*
     *  Open the output file for writing, with the same permissions as the
     *  source file.
     */
    write_fd = open( dst, O_WRONLY | O_CREAT, stat_buf.st_mode );
    if ( write_fd == -1 )  return 0;

    /* Blast the bytes from one file to the other. */
    if ( -1 == do_sendfile( write_fd, read_fd, offset, stat_buf.st_size ) )
    {
        close( read_fd );
        close( write_fd );
        return 0;
    }

    /* Close up. */
    close( read_fd );
    close( write_fd );

    }

    return 1;

    #else

    {

    char buffer[ 1024*512 ];
    UInt32 rbytes = 1;
    FILE *dst_fptr, *src_fptr = fopen( src, "rb" );
    if ( !src_fptr ) return 0;
    dst_fptr = fopen( dst, "wb" );
    if ( !dst_fptr )
    {
    	fclose(  src_fptr );
    	return 0;
    }
    while ( rbytes )
    {
    	rbytes = (UInt32)fread( buffer, 1, 1024*512, src_fptr );
            if ( rbytes )
            if ( !(rbytes == fwrite( buffer, 1, 1024*512, dst_fptr )) )
            {
            	/*
                 *	Problem writing file section
                 */
                fclose(  dst_fptr );
                fclose(  src_fptr );
                return 0;
            }
    }

    }

    return 1;

    #endif
}

/*-------------------------------------------------------------------------*/

/**
 *	moves a file from one location to the other
 *	If successfull true (1) is returned, otherwise false (0).
 */
UInt32
Move_File( const char* dst, const char* src, char overwriteFlag )
{GUCEF_TRACE;

    try
    {
        // map to C++ version
        return CORE::MoveFile( CString( dst ), CString( src ), 0 != overwriteFlag ) ? 1 : 0;
    }
    catch ( const std::exception& )
    {
        return 0;
    }
}

/*-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

static HANDLE
ExecuteProgramEx( const char *filename,
                  const char *cmdline )
{GUCEF_TRACE;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    BOOL bres;
    char *tmp_lstr;
    si.cb = sizeof( si );
    si.lpReserved = NULL;
    si.lpDesktop = NULL;
    si.lpTitle = NULL,
    si.dwFlags = STARTF_FORCEONFEEDBACK;
    si.cbReserved2 = 0;
    si.lpReserved2 = NULL;
    if ( cmdline )
    {
        /*
         *      We need to concatonize the filename and cmdline strings
         *      We can't just pass them seperatly because winblows
         *      does not follow ANSI rules and will allow you to override
         *      argv[ 0 ] by passing them seperatly. This can have verry
         *      unexpected results.
         */
        tmp_lstr = (char*) calloc( strlen( filename )+strlen( cmdline )+2, 1 );
        sprintf( tmp_lstr, "%s %s", filename, cmdline );
        bres = CreateProcess( NULL, tmp_lstr, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        free( tmp_lstr );
    }
    else
    {
        bres = CreateProcess( filename, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    }
    if ( bres ) return pi.hProcess;
    return NULL;
}

#endif /* GUCEF_PLATFORM_MSWIN ? */

/*-------------------------------------------------------------------------*/

/**
 *      Function that will attempt to execute the given program.
 *      If successfull true (1) is returned, otherwise false (0).
 */
UInt32
Execute_Program( const char *filename ,
                 const char *cmdline  )
{GUCEF_TRACE;

    if ( NULL != filename )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

        HANDLE phdl = ExecuteProgramEx( filename, cmdline );
        if ( phdl )
        {
            CloseHandle( phdl );
            return 1;
        }
        return 0;

        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )


        #endif
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt64
Filesize( const char* filename )
{GUCEF_TRACE;

    try
    {
        // map to C++ version
        return CORE::FileSize( CORE::CString( filename ) );
    }
    catch ( const std::exception& )
    {
        return 0;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
File_Exists( const char *filename )
{GUCEF_TRACE;

    try
    {
        // map to C++ version
        return CORE::FileExists( CString( filename ) ) ? 1 : 0;
    }
    catch ( const std::exception& )
    {
        return 0;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
Dir_Exists( const char *path )
{GUCEF_TRACE;

    try
    {
        // map to C++ version
        return CORE::DirExists( CString( path ) ) ? 1 : 0;
    }
    catch ( const std::exception& )
    {
        return 0;
    }
}

/*-------------------------------------------------------------------------*/

/**
 *      Counts the number of $UPDIR$ segments from the start
 *      of the given string
 */
UInt32
GetUpCount( const char* pathstr ,
            UInt32 pstrlen      ,
            UInt32* offset      )
{
    const char* cp = pathstr;
    Int32 i, max = pstrlen - 7;
    UInt32 count=0;

    *offset = 0;
    for ( i=0; i<max; ++i )
    {
        if ( memcmp( cp, "$UPDIR$", 7 ) == 0 )
        {
            ++count;
            *offset = i+7;
        }
        ++cp;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

/**
 *      Writes a relative path into dest build from pathstr.
 *      The following prefix tags can be used:
 *        $CURWORKDIR$ : this will be replaced with the current working dir.
 *        $MODULEDIR$ : this will be replaced with the module dir.
 *      The following tag can be placed after one of the above mentioned tags
 *        $UPDIR$ : go up one dir
 *      Dir seperator chars will be forced to that of the the target O/S.
 *      It is assumed dest is large enough to store the data.
 *      The function returns the total number of bytes written to dest.
 */
UInt32
Relative_Path( const char *pathstr ,
               char *dest          ,
               UInt32 buffersize   )
{
        #define MAXPATHTAGLENGTH 11

        UInt32 tagfound = 0;
        char tmpbuffer[ MAX_DIR_LENGTH ];
        const char* addition = "\0";
        UInt32 i, upOffset=0, upcount=0, pstrlen = (UInt32) strlen( pathstr );

        /*
         *      Check for the root dir tags that are to be prefixed
         */
        if ( pstrlen >= 11 )
        {
                if ( memcmp( pathstr, "$MODULEDIR$", 11 ) == 0 )
                {
                        Module_Path( tmpbuffer      ,
                                     MAX_DIR_LENGTH );
                        Strip_Filename( tmpbuffer );
                        upcount = GetUpCount( pathstr+11 ,
                                              pstrlen-11 ,
                                              &upOffset  );
                        addition = pathstr+11+upOffset;
                        tagfound = 1;
                }
                else
                if ( pstrlen >= 12 )
                {
                        if ( memcmp( pathstr, "$CURWORKDIR$", 12 ) == 0 )
                        {
                                Get_Current_Dir( tmpbuffer      ,
                                                 MAX_DIR_LENGTH );
                                upcount = GetUpCount( pathstr+12 ,
                                                      pstrlen-12 ,
                                                      &upOffset  );
                                addition = pathstr+12+upOffset;
                                tagfound = 1;
                        }
                }
        }

        if ( !tagfound )
        {
                UInt32 max = pstrlen+1;
                if ( max > buffersize ) max = buffersize;
                memcpy( dest, pathstr, max );
                return max;
        }

        /*
         *      Apply the $UPDIR$ tags
         */
        for ( i=0; i<upcount; ++i )
        {
                Strip_Last_Subdir( tmpbuffer );
        }


        /*
         *      Combine the parsed tag section with the rest of the path
         */
        Append_To_Path( tmpbuffer      ,
                        MAX_DIR_LENGTH ,
                        addition       );

        /*
         *      Copy into the buffer provided by the user
         */
        i = (UInt32) strlen(tmpbuffer)+1;
        memcpy( dest      ,
                tmpbuffer ,
                i         );

        return i;
}

/*-------------------------------------------------------------------------*/

UInt32
Is_Path_Valid( const char* path )
{GUCEF_TRACE;

    // direct to C++ version
    return CORE::PathExists( path ) ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

time_t
FileTimeToUnixTime( const FILETIME* ft )
{
	/*
     *  FILETIME uses ticks where each is 100 nanoseconds
     *
     *  FILETIME epoch is 00:00, January 1, 1601 (UTC)
     *  Unix epoch is 00:00, January 1 1970 (UTC)
     *  As such we need to correct for the epoch offset
     *  Note that leapseconds dont matter for this step since those were introduced in 1972
     *
     *  To get nanoseconds to seconds divide by 1,000,000,000
     *  Since time_t uses the seconds we need to adjust the resolution as well
     */
    /* the reverse of http://support.microsoft.com/kb/167296/en-us */
	ULONGLONG ull = *(const ULONGLONG*)((void*)(ft));
	ull = ( ull / 10000000 ) - 11644473600LL;
	assert(ull < ULONG_MAX);
	return (time_t)(ull);
}

#endif

/*-------------------------------------------------------------------------*/

time_t
Get_Modification_Time( const char* path )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    WIN32_FILE_ATTRIBUTE_DATA data;
    if ( 0 != GetFileAttributesEx( path, GetFileExInfoStandard, &data ) )
    {
        return FileTimeToUnixTime( &data.ftLastWriteTime );
    }
    return -1;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    struct stat buf;

    /* Get File Statistics for stat.c. */
    if( stat( path, &buf ) == 0 )
    {
        /* get the date/time last modified */
        return buf.st_mtime;
    }
    return -1;

    #else

    /*
     *  Unsupported platform
     */
    return (time_t) 0;

    #endif
}


/*-------------------------------------------------------------------------*/

time_t
Get_Creation_Time( const char* path )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    WIN32_FILE_ATTRIBUTE_DATA data;
    if ( 0 != GetFileAttributesEx( path, GetFileExInfoStandard, &data ) )
    {
        return FileTimeToUnixTime( &data.ftCreationTime );
    }
    return -1;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    struct stat buf;

    /* Get File Statistics for stat.c. */
    if( stat( path, &buf ) == 0 )
    {
        /* get the date/time the file was created */
        return buf.st_ctime;
    }
    return -1;

    #else

    /*
     *  Unsupported platform
     */
    return (time_t) 0;

    #endif
}

/*-------------------------------------------------------------------------*/

UInt32
Try_Resolve_Special_Dir( TSpecialDirs dir, char* dest, UInt32 dest_size )
{GUCEF_TRACE;

    try
    {
        // map to C++ version
        CString resolvedPath;
        if ( CORE::TryResolveSpecialDir( dir, resolvedPath ) )
        {
            if ( GUCEF_NULL != dest && dest_size > 0 )
            {
                UInt32 neededBufferSize = resolvedPath.ByteSize();
                if ( neededBufferSize <= dest_size )
                {
                    memcpy( dest, resolvedPath.C_String(), resolvedPath.ByteSize() );
                }
            }
            return resolvedPath.ByteSize();
        }
        return 0;
    }
    catch ( const std::exception& )
    {
        return 0;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace CORE */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/
