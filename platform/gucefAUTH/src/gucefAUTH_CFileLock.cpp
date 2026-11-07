/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <windows.h>
  #include <string>
  #define GUCEF_AUTH_FILELOCK_LOCKFILEEX
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) || \
        ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPHONEOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPADOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_TVOS ) || \
        ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_VISIONOS ) )
  #include <errno.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <sys/file.h>
  #define GUCEF_AUTH_FILELOCK_FLOCK
#endif

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefAUTH_CFileLock.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CFileLock::MaxPollDelayInMs = 16;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFileLock::CFileLock( void )
    : m_handle( GUCEF_NULL )
    , m_fd( -1 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFileLock::~CFileLock()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

bool
CFileLock::IsSupported( void )
{GUCEF_TRACE;

    #if defined( GUCEF_AUTH_FILELOCK_LOCKFILEEX ) || defined( GUCEF_AUTH_FILELOCK_FLOCK )
    return true;
    #else
    return false;
    #endif
}

/*-------------------------------------------------------------------------*/

bool
CFileLock::IsHeld( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_handle || m_fd >= 0;
}

/*-------------------------------------------------------------------------*/

CFileLock::TLockOutcome
CFileLock::Acquire( const CORE::CString& lockFilePath, bool exclusive, UInt32 timeoutInMs )
{GUCEF_TRACE;

    Release();

    #if defined( GUCEF_AUTH_FILELOCK_LOCKFILEEX )

    std::wstring wPath = CORE::ToWString( lockFilePath );
    HANDLE handle = ::CreateFileW( wPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( INVALID_HANDLE_VALUE == handle )
        return LOCKOUTCOME_UNAVAILABLE;
    m_handle = handle;
    DWORD flags = exclusive ? LOCKFILE_EXCLUSIVE_LOCK : 0;
    if ( 0 == timeoutInMs )
    {
        OVERLAPPED overlapped;
        memset( &overlapped, 0, sizeof( overlapped ) );
        if ( FALSE != ::LockFileEx( handle, flags, 0, 1, 0, &overlapped ) )
            return LOCKOUTCOME_TAKEN;
        Release();
        return LOCKOUTCOME_UNAVAILABLE;
    }
    UInt32 waited = 0;
    UInt32 delay = 1;
    while ( true )
    {
        OVERLAPPED overlapped;
        memset( &overlapped, 0, sizeof( overlapped ) );
        if ( FALSE != ::LockFileEx( handle, flags | LOCKFILE_FAIL_IMMEDIATELY, 0, 1, 0, &overlapped ) )
            return LOCKOUTCOME_TAKEN;
        DWORD error = ::GetLastError();
        if ( ERROR_LOCK_VIOLATION != error && ERROR_IO_PENDING != error )
        {
            Release();
            return LOCKOUTCOME_UNAVAILABLE;
        }
        if ( waited >= timeoutInMs )
        {
            Release();
            return LOCKOUTCOME_TIMED_OUT;
        }
        MT::PrecisionDelay( delay );
        waited += delay;
        if ( delay < MaxPollDelayInMs )
            delay *= 2;
    }

    #elif defined( GUCEF_AUTH_FILELOCK_FLOCK )

    int openFlags = O_RDWR | O_CREAT;
    #ifdef O_CLOEXEC
    openFlags |= O_CLOEXEC;
    #endif
    #ifdef O_NOFOLLOW
    openFlags |= O_NOFOLLOW;
    #endif
    m_fd = ::open( lockFilePath.C_String(), openFlags, 0600 );
    if ( m_fd < 0 )
        return LOCKOUTCOME_UNAVAILABLE;
    int operation = exclusive ? LOCK_EX : LOCK_SH;
    if ( 0 == timeoutInMs )
    {
        while ( 0 != ::flock( m_fd, operation ) )
        {
            if ( EINTR != errno )
            {
                Release();
                return LOCKOUTCOME_UNAVAILABLE;
            }
        }
        return LOCKOUTCOME_TAKEN;
    }
    UInt32 waited = 0;
    UInt32 delay = 1;
    while ( true )
    {
        if ( 0 == ::flock( m_fd, operation | LOCK_NB ) )
            return LOCKOUTCOME_TAKEN;
        if ( EWOULDBLOCK != errno && EINTR != errno )
        {
            Release();
            return LOCKOUTCOME_UNAVAILABLE;
        }
        if ( waited >= timeoutInMs )
        {
            Release();
            return LOCKOUTCOME_TIMED_OUT;
        }
        MT::PrecisionDelay( delay );
        waited += delay;
        if ( delay < MaxPollDelayInMs )
            delay *= 2;
    }

    #else

    // one process on this platform: the mutex of the instance is all there is
    return LOCKOUTCOME_UNAVAILABLE;

    #endif
}

/*-------------------------------------------------------------------------*/

void
CFileLock::Release( void )
{GUCEF_TRACE;

    #if defined( GUCEF_AUTH_FILELOCK_LOCKFILEEX )
    if ( GUCEF_NULL != m_handle )
    {
        // closing the handle releases a lock it holds
        ::CloseHandle( static_cast< HANDLE >( m_handle ) );
        m_handle = GUCEF_NULL;
    }
    #elif defined( GUCEF_AUTH_FILELOCK_FLOCK )
    if ( m_fd >= 0 )
    {
        // closing the only descriptor of the open file description releases the lock
        ::close( m_fd );
        m_fd = -1;
    }
    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
