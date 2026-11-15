/*
 *  gucefMT: GUCEF module providing multithreading solutions
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

#ifndef GUCEF_MT_THREADNAME_H
#include "gucefMT_ThreadName.h"
#define GUCEF_MT_THREADNAME_H
#endif /* GUCEF_MT_THREADNAME_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <windows.h>
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || \
        ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPHONEOS ) || \
        ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_IPADOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_TVOS ) )
  #include <pthread.h>
  #define GUCEF_MT_THREADNAME_PTHREAD
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

typedef HRESULT ( WINAPI *TSetThreadDescriptionFunc )( HANDLE, PCWSTR );
typedef HRESULT ( WINAPI *TGetThreadDescriptionFunc )( HANDLE, PWSTR* );

#endif

/*-------------------------------------------------------------------------*/

bool
SetCurrentThreadName( const char* name )
{
    if ( GUCEF_NULL == name )
        return false;

    #if defined( GUCEF_MT_THREADNAME_PTHREAD ) && ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    // the kernel keeps 15 bytes and a zero: a longer name makes pthread_setname_np() fail with ERANGE, so it is cut
    // here, never inside a UTF-8 sequence
    char shortName[ GUCEF_MT_MAX_OS_THREAD_NAME_BYTES + 1 ];
    size_t length = strlen( name );
    if ( length > GUCEF_MT_MAX_OS_THREAD_NAME_BYTES )
    {
        length = GUCEF_MT_MAX_OS_THREAD_NAME_BYTES;
        while ( length > 0 && 0x80 == ( ( (unsigned char) name[ length ] ) & 0xC0 ) )
            --length;
    }
    memcpy( shortName, name, length );
    shortName[ length ] = '\0';
    return 0 == pthread_setname_np( pthread_self(), shortName );

    #elif defined( GUCEF_MT_THREADNAME_PTHREAD )

    // Apple: only the calling thread can be named
    return 0 == pthread_setname_np( name );

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // SetThreadDescription() exists from Windows 10 1607 on: looked up at run time so older systems still load us
    HMODULE kernel = ::GetModuleHandleW( L"kernel32.dll" );
    if ( GUCEF_NULL == kernel )
        return false;
    TSetThreadDescriptionFunc setDescription = (TSetThreadDescriptionFunc) ::GetProcAddress( kernel, "SetThreadDescription" );
    if ( GUCEF_NULL == setDescription )
        return false;

    int wideLength = ::MultiByteToWideChar( CP_UTF8, 0, name, -1, GUCEF_NULL, 0 );
    if ( wideLength <= 0 )
        return false;
    wchar_t* wideName = new wchar_t[ wideLength ];
    bool success = false;
    if ( ::MultiByteToWideChar( CP_UTF8, 0, name, -1, wideName, wideLength ) > 0 )
        success = SUCCEEDED( setDescription( ::GetCurrentThread(), wideName ) );
    delete[] wideName;
    return success;

    #else

    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
GetCurrentThreadName( char* buffer, UInt32 bufferSize )
{
    if ( GUCEF_NULL == buffer || 0 == bufferSize )
        return false;
    buffer[ 0 ] = '\0';

    #if defined( GUCEF_MT_THREADNAME_PTHREAD )

    char name[ 256 ];
    if ( 0 != pthread_getname_np( pthread_self(), name, sizeof( name ) ) )
        return false;
    name[ sizeof( name ) - 1 ] = '\0';
    size_t length = strlen( name );
    if ( length > bufferSize - 1 )
        length = bufferSize - 1;
    memcpy( buffer, name, length );
    buffer[ length ] = '\0';
    return true;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    HMODULE kernel = ::GetModuleHandleW( L"kernel32.dll" );
    if ( GUCEF_NULL == kernel )
        return false;
    TGetThreadDescriptionFunc getDescription = (TGetThreadDescriptionFunc) ::GetProcAddress( kernel, "GetThreadDescription" );
    if ( GUCEF_NULL == getDescription )
        return false;
    PWSTR wideName = GUCEF_NULL;
    if ( !SUCCEEDED( getDescription( ::GetCurrentThread(), &wideName ) ) || GUCEF_NULL == wideName )
        return false;
    int written = ::WideCharToMultiByte( CP_UTF8, 0, wideName, -1, buffer, (int) bufferSize, GUCEF_NULL, GUCEF_NULL );
    ::LocalFree( wideName );
    if ( written <= 0 )
    {
        buffer[ 0 ] = '\0';
        return false;
    }
    buffer[ bufferSize - 1 ] = '\0';
    return true;

    #else

    return false;

    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
