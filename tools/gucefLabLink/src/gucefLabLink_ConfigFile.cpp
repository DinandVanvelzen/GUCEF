/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#include <string.h>
#include <errno.h>

#include "gucefCORE_macros.h"

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || \
      ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) )
  #define GUCEF_LABLINK_POSIX_CONFIG_FILE
  #include <fcntl.h>
  #include <unistd.h>
  #include <sys/stat.h>
#endif

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefLabLink_ConfigFile.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 MaxConfigLineSize = 4096;

/*-------------------------------------------------------------------------*/

static bool
IsBlank( char c )
{GUCEF_TRACE;

    return ' ' == c || '\t' == c || '\r' == c;
}

/*-------------------------------------------------------------------------*/

static bool
IsNameChar( char c )
{GUCEF_TRACE;

    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '-' == c;
}

/*-------------------------------------------------------------------------*/

/** One line, without its line end; false with 'problem' when it is refused */
static bool
ParseConfigLine( const char* line                         ,
                 UInt32 size                              ,
                 const CORE::CString& where               ,
                 CORE::CString::StringVector& arguments   ,
                 CORE::CString& problem                   )
{GUCEF_TRACE;

    UInt32 begin = 0;
    UInt32 end = size;
    while ( begin < end && IsBlank( line[ begin ] ) )
        ++begin;
    while ( end > begin && IsBlank( line[ end-1 ] ) )
        --end;
    if ( begin == end || '#' == line[ begin ] || ';' == line[ begin ] )
        return true;

    if ( end - begin >= 2 && '-' == line[ begin ] && '-' == line[ begin+1 ] )
        begin += 2;
    UInt32 nameEnd = begin;
    while ( nameEnd < end && IsNameChar( line[ nameEnd ] ) )
        ++nameEnd;
    if ( nameEnd == begin || '-' == line[ begin ] )
    {
        problem = where + ": an option name is expected";
        return false;
    }
    if ( nameEnd < end && !IsBlank( line[ nameEnd ] ) && '=' != line[ nameEnd ] )
    {
        problem = where + ": an option name holds letters, digits and '-' only";
        return false;
    }
    const CORE::CString name( line + begin, nameEnd - begin );
    if ( name == "config" || name == "console" )
    {
        problem = where + ": " + name + " is an option of the command line, not of the configuration file";
        return false;
    }

    UInt32 valueBegin = nameEnd;
    while ( valueBegin < end && IsBlank( line[ valueBegin ] ) )
        ++valueBegin;
    if ( valueBegin == end )
    {
        // an option without a value
        arguments.push_back( "--" + name );
        return true;
    }
    if ( '=' == line[ valueBegin ] )
    {
        ++valueBegin;
        while ( valueBegin < end && IsBlank( line[ valueBegin ] ) )
            ++valueBegin;
        if ( valueBegin == end )
        {
            problem = where + ": " + name + " = has no value";
            return false;
        }
    }
    UInt32 valueEnd = end;
    if ( '"' == line[ valueBegin ] )
    {
        if ( valueEnd - valueBegin < 2 || '"' != line[ valueEnd-1 ] )
        {
            problem = where + ": the quote of the value of " + name + " is not closed";
            return false;
        }
        ++valueBegin;
        --valueEnd;
    }
    arguments.push_back( "--" + name );
    arguments.push_back( CORE::CString( line + valueBegin, valueEnd - valueBegin ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
ParseConfigText( const CORE::CString& text                ,
                 const CORE::CString& sourceName          ,
                 CORE::CString::StringVector& arguments   ,
                 CORE::CString& problem                   )
{GUCEF_TRACE;

    arguments.clear();
    problem.Clear();
    const char* data = text.C_String();
    UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    if ( GUCEF_NULL == data )
        size = 0;
    if ( size > MaxConfigFileSize )
    {
        problem = sourceName + " is larger than " + CORE::ToString( MaxConfigFileSize ) + " bytes";
        return false;
    }

    UInt32 position = 0;
    if ( size >= 3 && 0 == ::memcmp( data, "\xEF\xBB\xBF", 3 ) )
        position = 3;
    UInt32 lineNumber = 0;
    while ( position < size )
    {
        ++lineNumber;
        UInt32 lineEnd = position;
        while ( lineEnd < size && '\n' != data[ lineEnd ] )
            ++lineEnd;
        const CORE::CString where = sourceName + " line " + CORE::ToString( lineNumber );
        if ( lineEnd - position > MaxConfigLineSize )
        {
            problem = where + " is longer than " + CORE::ToString( MaxConfigLineSize ) + " bytes";
            arguments.clear();
            return false;
        }
        if ( !ParseConfigLine( data + position, lineEnd - position, where, arguments, problem ) )
        {
            arguments.clear();
            return false;
        }
        position = lineEnd + 1;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_LABLINK_POSIX_CONFIG_FILE

/** POSIX: the content of a regular file of root or the effective user that the group and others cannot write */
static bool
ReadAdminOnlyWritableFile( const CORE::CString& path, CORE::CDynamicBuffer& content, bool& missing, CORE::CString& problem )
{GUCEF_TRACE;

    int flags = O_RDONLY | O_NOFOLLOW | O_NONBLOCK;
    #ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
    #endif
    const int fd = ::open( path.C_String(), flags );
    if ( fd < 0 )
    {
        missing = ENOENT == errno;
        problem = missing ? CORE::CString( "it does not exist" ) : "it cannot be opened: " + CORE::CString( ::strerror( errno ) );
        return false;
    }
    struct stat info;
    bool safe = 0 == ::fstat( fd, &info );
    if ( !safe || !S_ISREG( info.st_mode ) )
        problem = "it is no regular file";
    else
    if ( 0 != info.st_uid && ::geteuid() != info.st_uid )
        problem = "it is not owned by root or by the user of the service";
    else
    if ( 0 != ( info.st_mode & ( S_IWGRP | S_IWOTH ) ) )
        problem = "the group or others can change it";
    else
    if ( info.st_size > (off_t) MaxConfigFileSize )
        problem = "it is larger than " + CORE::ToString( MaxConfigFileSize ) + " bytes";
    safe = problem.IsNULLOrEmpty();

    if ( safe )
    {
        content.SetBufferSize( (UInt32) info.st_size + 1, false );
        UInt32 total = 0;
        for ( ;; )
        {
            const ssize_t got = ::read( fd, static_cast< char* >( content.GetBufferPtr() ) + total, (size_t) ( content.GetBufferSize() - total ) );
            if ( got < 0 && EINTR == errno )
                continue;
            if ( got < 0 )
            {
                problem = "it cannot be read";
                safe = false;
                break;
            }
            if ( 0 == got )
                break;
            total += (UInt32) got;
            if ( total > MaxConfigFileSize )
            {
                problem = "it is larger than " + CORE::ToString( MaxConfigFileSize ) + " bytes";
                safe = false;
                break;
            }
            if ( total == content.GetBufferSize() )
                content.SetBufferSize( total + 4096, true );
        }
        content.SetDataSize( safe ? total : 0 );
    }
    ::close( fd );
    return safe;
}

#endif /* GUCEF_LABLINK_POSIX_CONFIG_FILE ? */

/*-------------------------------------------------------------------------*/

bool
ReadConfigFile( const CORE::CString& path                 ,
                CORE::CString::StringVector& arguments    ,
                bool& missing                             ,
                CORE::CString& problem                    )
{GUCEF_TRACE;

    arguments.clear();
    missing = false;
    problem.Clear();
    CORE::CDynamicBuffer content;
    CORE::CString why;

    #if defined( GUCEF_LABLINK_POSIX_CONFIG_FILE )
    const bool read = ReadAdminOnlyWritableFile( path, content, missing, why );
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    const bool read = CORE::ReadOwnerOnlyFile( path, content, why, &missing );
    #else
    const bool read = content.LoadContentFromFile( path );
    missing = !read && !CORE::FileExists( path );
    why = "it cannot be read";
    #endif

    if ( !read )
    {
        problem = "the configuration file " + path + " is refused: " + why;
        return false;
    }
    const char* bytes = static_cast< const char* >( content.GetConstBufferPtr() );
    const UInt32 size = content.GetDataSize();
    if ( size > 0 && GUCEF_NULL != ::memchr( bytes, '\0', size ) )
    {
        problem = "the configuration file " + path + " is no text";
        return false;
    }
    const CORE::CString text = 0 == size ? CORE::CString() : CORE::CString( bytes, size );
    return ParseConfigText( text, path, arguments, problem );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
