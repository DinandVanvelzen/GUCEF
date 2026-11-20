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

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
  #include <errno.h>
  #include <fcntl.h>
  #include <pwd.h>
  #include <unistd.h>
  #include <sys/stat.h>
#endif

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#include "gucefLabLink_Protocol.h"
#include "gucefLabLink_WindowsHost.h"
#include "gucefLabLink_CiFiles.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CCiFiles::RegistryFileName = "ci.json";
const char* CCiFiles::TokenFileName = "gateway.token";
const char* CCiFiles::CaFileName = "ca.crt";

/** A file of the directory that is larger is not what this class wrote: it is replaced without being read */
static const UInt32 MaxCompareSizeInBytes = 1024 * 1024;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The size of the text in bytes, without its terminator */
static UInt32
ByteLength( const CORE::CString& text )
{
    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

/** The text of errno */
static CORE::CString
ErrnoText( int error )
{
    const char* text = ::strerror( error );
    return GUCEF_NULL != text ? CORE::CString( text ) : "error " + CORE::ToString( (Int32) error );
}

/*-------------------------------------------------------------------------*/

/** The directory, split into the directory it is in and its own name */
static void
SplitDirectory( const CORE::CString& directory, CORE::CString& parent, CORE::CString& name )
{
    CORE::CString path = directory;
    while ( path.Length() > 1 && path.EndsWith( "/" ) )
        path = path.CutChars( 1, false );
    const Int32 slash = path.HasChar( '/', false );
    if ( slash < 0 )
    {
        parent = ".";
        name = path;
    }
    else
    {
        parent = 0 == slash ? CORE::CString( "/" ) : path.SubstrToIndex( (UInt32) slash, true );
        name = path.CutChars( (UInt32) slash + 1, true );
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  Opens the directory without following a link in its place; makes it (mode 0700, and its parents) when create.
 *  @return the descriptor, -1 when it cannot be opened (missing set when it is not there and create is false)
 */
static int
OpenDirectory( const CORE::CString& directory, bool create, bool& missing, CORE::CString& problem )
{
    missing = false;
    CORE::CString parent, name;
    SplitDirectory( directory, parent, name );
    if ( name.IsNULLOrEmpty() || name == "." || name == ".." )
    {
        problem = directory + " is not a directory for the CI files";
        return -1;
    }
    int parentFd = ::open( parent.C_String(), O_RDONLY | O_DIRECTORY | O_CLOEXEC );
    if ( parentFd < 0 && ENOENT == errno && create && CORE::CreateDirs( parent ) )
        parentFd = ::open( parent.C_String(), O_RDONLY | O_DIRECTORY | O_CLOEXEC );
    if ( parentFd < 0 )
    {
        missing = ENOENT == errno;
        problem = "cannot open " + parent + ": " + ErrnoText( errno );
        return -1;
    }
    if ( create && 0 != ::mkdirat( parentFd, name.C_String(), 0700 ) && EEXIST != errno )
    {
        problem = "cannot make " + directory + ": " + ErrnoText( errno );
        ::close( parentFd );
        return -1;
    }
    const int fd = ::openat( parentFd, name.C_String(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC );
    const int openError = errno;
    ::close( parentFd );
    if ( fd < 0 )
    {
        missing = ENOENT == openError;
        problem = "cannot open " + directory + " as a directory (a link is not followed): " + ErrnoText( openError );
        return -1;
    }
    return fd;
}

/*-------------------------------------------------------------------------*/

/** The ids of the owner; false when there is none to give the files to */
static bool
OwnerIds( const CORE::CString& owner, uid_t& uid, gid_t& gid, CORE::CString& problem )
{
    if ( owner.IsNULLOrEmpty() )
        return false;
    struct passwd* user = ::getpwnam( owner.C_String() );
    if ( GUCEF_NULL == user )
    {
        problem = "the user " + owner + " does not exist";
        return false;
    }
    uid = user->pw_uid;
    gid = user->pw_gid;
    if ( 0 != ::geteuid() && uid != ::geteuid() )
    {
        problem = "only root can give the files to " + owner;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** The owner and mode an open file or directory should have */
static bool
Protect( int fd, bool give, uid_t uid, gid_t gid, mode_t mode, const CORE::CString& path, CORE::CString& problem )
{
    struct stat info;
    if ( 0 != ::fstat( fd, &info ) )
    {
        problem = "cannot look at " + path + ": " + ErrnoText( errno );
        return false;
    }
    if ( give && ( info.st_uid != uid || info.st_gid != gid ) && 0 != ::fchown( fd, uid, gid ) )
    {
        problem = "cannot give " + path + " to its user: " + ErrnoText( errno );
        return false;
    }
    if ( ( info.st_mode & 07777 ) != mode && 0 != ::fchmod( fd, mode ) )
    {
        problem = "cannot limit the access of " + path + ": " + ErrnoText( errno );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Writes one file of the directory when its content differs; protects it either way */
static bool
WriteOne( int dirFd, const CORE::CString& directory, const char* name, const CORE::CString& content, bool give, uid_t uid, gid_t gid,
          bool& changed, CORE::CString& problem )
{
    const CORE::CString path = CORE::CombinePath( directory, name );

    // what is there: the same content is left as it is, only its owner and mode are seen to
    const int existing = ::openat( dirFd, name, O_RDONLY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC );
    if ( existing >= 0 )
    {
        struct stat info;
        bool same = false;
        if ( 0 == ::fstat( existing, &info ) && S_ISREG( info.st_mode ) && (UInt64) info.st_size == (UInt64) ByteLength( content ) &&
             ByteLength( content ) <= MaxCompareSizeInBytes )
        {
            CORE::CDynamicBuffer buffer( ByteLength( content ) + 1, true );
            char* data = static_cast< char* >( buffer.GetBufferPtr() );
            UInt32 got = 0;
            while ( got < ByteLength( content ) )
            {
                const ssize_t n = ::read( existing, data + got, ByteLength( content ) - got );
                if ( n <= 0 )
                    break;
                got += (UInt32) n;
            }
            same = got == ByteLength( content ) && 0 == ::memcmp( data, content.C_String(), ByteLength( content ) );
        }
        if ( same )
        {
            const bool protectedOk = Protect( existing, give, uid, gid, 0600, path, problem );
            ::close( existing );
            return protectedOk;
        }
        ::close( existing );
    }

    // a new file next to it, made here (never one that exists), given away by its descriptor and moved over it
    const CORE::CString temporary = CORE::CString( "." ) + name + ".tmp";
    ::unlinkat( dirFd, temporary.C_String(), 0 );
    const int fd = ::openat( dirFd, temporary.C_String(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600 );
    if ( fd < 0 )
    {
        problem = "cannot make " + CORE::CombinePath( directory, temporary ) + ": " + ErrnoText( errno );
        return false;
    }
    const char* data = content.C_String();
    UInt32 written = 0;
    bool success = true;
    while ( written < ByteLength( content ) )
    {
        const ssize_t n = ::write( fd, data + written, ByteLength( content ) - written );
        if ( n <= 0 )
        {
            if ( n < 0 && EINTR == errno )
                continue;
            problem = "cannot write " + CORE::CombinePath( directory, temporary ) + ": " + ErrnoText( errno );
            success = false;
            break;
        }
        written += (UInt32) n;
    }
    if ( success && 0 != ::fsync( fd ) )
    {
        problem = "cannot write " + CORE::CombinePath( directory, temporary ) + ": " + ErrnoText( errno );
        success = false;
    }
    success = success && Protect( fd, give, uid, gid, 0600, path, problem );
    ::close( fd );
    if ( success && 0 != ::renameat( dirFd, temporary.C_String(), dirFd, name ) )
    {
        problem = "cannot replace " + path + ": " + ErrnoText( errno );
        success = false;
    }
    if ( !success )
    {
        ::unlinkat( dirFd, temporary.C_String(), 0 );
        return false;
    }
    changed = true;
    return true;
}

#else

/*-------------------------------------------------------------------------*/

/** Whether the file holds exactly the content */
static bool
HasContent( const CORE::CString& path, const CORE::CString& content )
{
    if ( !CORE::FileExists( path ) || CORE::FileSize( path ) != (UInt64) ByteLength( content ) )
        return false;
    CORE::CDynamicBuffer buffer;
    if ( !buffer.LoadContentFromFile( path ) )
        return false;
    return buffer.GetDataSize() == ByteLength( content ) && 0 == ::memcmp( buffer.GetConstBufferPtr(), content.C_String(), ByteLength( content ) );
}

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCiFiles::CCiFiles( const CORE::CString& directory, const CORE::CString& owner )
    : m_directory( directory )
    , m_owner( owner )
    , m_ownerProblemReported( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CCiFiles::GetDirectory( void ) const
{GUCEF_TRACE;

    return m_directory;
}

/*-------------------------------------------------------------------------*/

bool
CCiFiles::Write( const CORE::CString& registry      ,
                 const CORE::CString& gatewayToken  ,
                 const CORE::CString& caCertificate ,
                 bool& changed                      ,
                 CORE::CString& problem             )
{GUCEF_TRACE;

    changed = false;
    problem.Clear();
    const char* names[ 3 ] = { RegistryFileName, TokenFileName, CaFileName };
    const CORE::CString contents[ 3 ] = { registry, StripText( gatewayToken ) + "\n", caCertificate };

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    uid_t uid = 0;
    gid_t gid = 0;
    CORE::CString ownerProblem;
    const bool give = OwnerIds( m_owner, uid, gid, ownerProblem );
    if ( !m_owner.IsNULLOrEmpty() && !give && !m_ownerProblemReported )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: The CI files in " << m_directory << " stay with the user of the service: " << ownerProblem );
    m_ownerProblemReported = !m_owner.IsNULLOrEmpty() && !give;

    bool missing = false;
    const int dirFd = OpenDirectory( m_directory, true, missing, problem );
    if ( dirFd < 0 )
        return false;
    bool success = Protect( dirFd, give, uid, gid, 0700, m_directory, problem );
    for ( int i=0; success && i<3; ++i )
        success = WriteOne( dirFd, m_directory, names[ i ], contents[ i ], give, uid, gid, changed, problem );
    if ( changed )
        ::fsync( dirFd );
    ::close( dirFd );
    return success;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // the access of the state directory: SYSTEM and the Administrators (see gucefLabLink_WindowsHost.h)
    if ( !CORE::DirExists( m_directory ) && !ProtectStateDirectory( m_directory, problem ) )
        return false;
    for ( int i=0; i<3; ++i )
    {
        const CORE::CString path = CORE::CombinePath( m_directory, names[ i ] );
        if ( HasContent( path, contents[ i ] ) )
            continue;
        if ( !WriteProtectedStateFile( m_directory, path, contents[ i ].C_String(), ByteLength( contents[ i ] ), problem ) )
            return false;
        changed = true;
    }
    return true;

    #else

    if ( !CORE::DirExists( m_directory ) && !CORE::CreateDirs( m_directory ) && !CORE::DirExists( m_directory ) )
    {
        problem = "cannot make " + m_directory;
        return false;
    }
    CORE::SetOwnerOnlyAccess( m_directory );
    for ( int i=0; i<3; ++i )
    {
        const CORE::CString path = CORE::CombinePath( m_directory, names[ i ] );
        if ( !HasContent( path, contents[ i ] ) )
        {
            if ( CORE::FileExists( path ) )
            {
                if ( !CORE::WriteFileAtomically( path, contents[ i ].C_String(), ByteLength( contents[ i ] ) ) )
                {
                    problem = "cannot write " + path;
                    return false;
                }
            }
            else
            if ( !CORE::CreateOwnerOnlyFile( path, contents[ i ].C_String(), ByteLength( contents[ i ] ), problem ) )
                return false;
            changed = true;
        }
        if ( !CORE::SetOwnerOnlyAccess( path ) )
        {
            problem = "cannot limit the access of " + path;
            return false;
        }
    }
    return true;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
CCiFiles::WriteOwnerOnlyFile( const CORE::CString& path      ,
                              const CORE::CString& content   ,
                              const CORE::CString& owner     ,
                              bool& changed                  ,
                              CORE::CString& ownerProblem    ,
                              CORE::CString& problem         )
{GUCEF_TRACE;

    changed = false;
    ownerProblem.Clear();
    problem.Clear();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    CORE::CString directory, name;
    SplitDirectory( path, directory, name );
    if ( name.IsNULLOrEmpty() || name == "." || name == ".." )
    {
        problem = path + " is not a file name";
        return false;
    }
    uid_t uid = 0;
    gid_t gid = 0;
    const bool give = OwnerIds( owner, uid, gid, ownerProblem );
    const int dirFd = ::open( directory.C_String(), O_RDONLY | O_DIRECTORY | O_CLOEXEC );
    if ( dirFd < 0 )
    {
        problem = "the directory " + directory + " of " + name + " cannot be opened: " + ErrnoText( errno );
        return false;
    }
    const bool success = WriteOne( dirFd, directory, name.C_String(), content, give, uid, gid, changed, problem );
    if ( changed )
        ::fsync( dirFd );
    ::close( dirFd );
    return success;

    #else

    if ( !owner.IsNULLOrEmpty() )
        ownerProblem = "files are given to another user on Linux only";
    if ( !HasContent( path, content ) )
    {
        if ( CORE::FileExists( path ) )
        {
            if ( !CORE::WriteFileAtomically( path, content.C_String(), ByteLength( content ) ) )
            {
                problem = "cannot write " + path;
                return false;
            }
        }
        else
        if ( !CORE::CreateOwnerOnlyFile( path, content.C_String(), ByteLength( content ), problem ) )
            return false;
        changed = true;
    }
    if ( !CORE::SetOwnerOnlyAccess( path ) )
    {
        problem = "cannot limit the access of " + path;
        return false;
    }
    return true;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
CCiFiles::Remove( bool& removed, CORE::CString& problem )
{GUCEF_TRACE;

    removed = false;
    problem.Clear();
    const char* names[ 3 ] = { RegistryFileName, TokenFileName, CaFileName };

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    bool missing = false;
    const int dirFd = OpenDirectory( m_directory, false, missing, problem );
    if ( dirFd < 0 )
    {
        if ( missing )
        {
            problem.Clear();
            return true;
        }
        return false;
    }
    bool success = true;
    for ( int i=0; i<3; ++i )
    {
        if ( 0 == ::unlinkat( dirFd, names[ i ], 0 ) )
            removed = true;
        else
        if ( ENOENT != errno )
        {
            problem = "cannot remove " + CORE::CombinePath( m_directory, names[ i ] ) + ": " + ErrnoText( errno );
            success = false;
        }
    }
    if ( removed )
        ::fsync( dirFd );
    ::close( dirFd );
    return success;

    #else

    bool success = true;
    for ( int i=0; i<3; ++i )
    {
        const CORE::CString path = CORE::CombinePath( m_directory, names[ i ] );
        if ( !CORE::FileExists( path ) )
            continue;
        if ( CORE::DeleteFile( path ) )
            removed = true;
        else
        {
            problem = "cannot remove " + path;
            success = false;
        }
    }
    return success;

    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
