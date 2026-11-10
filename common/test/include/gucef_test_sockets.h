/*
 *  gucef common header: header-only counting of the sockets a process has open, for tests
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

/*
 *  Test support: the sockets and the descriptors this process has open, so that a test can tell that something it
 *  started released all of what it took. On Linux they come from /proc/self/fd, a descriptor that is a socket reads
 *  as "socket:[inode]". Elsewhere the count is not available (-1) and a test skips what needs it:
 *
 *    const int before = GUCEF::TESTFW::CountOpenSockets();
 *    GUCEF_TESTFW_SKIP_IF( before < 0, "the open sockets of this process cannot be counted on this platform" );
 *    ... start something, stop it ...
 *    ASSERT_TRUE( before == GUCEF::TESTFW::CountOpenSockets() );
 */

#ifndef GUCEF_TEST_SOCKETS_H
#define GUCEF_TEST_SOCKETS_H

#ifdef __cplusplus

#if defined( __linux__ ) || defined( __ANDROID__ )

  #include <dirent.h>
  #include <unistd.h>
  #include <string.h>
  #include <stdlib.h>

#endif /* Linux ? */

namespace GUCEF {
namespace TESTFW {

/** The number of descriptors of this process that are sockets, -1 when this platform cannot tell */
inline int
CountOpenSockets( void )
{
    #if defined( __linux__ ) || defined( __ANDROID__ )

    DIR* dir = opendir( "/proc/self/fd" );
    if ( NULL == dir )
        return -1;
    int count = 0;
    const int self = dirfd( dir );
    struct dirent* entry = NULL;
    while ( NULL != ( entry = readdir( dir ) ) )
    {
        if ( '.' == entry->d_name[ 0 ] )
            continue;
        const int fd = atoi( entry->d_name );
        if ( fd == self )
            continue;
        char path[ 64 ];
        char target[ 64 ];
        strcpy( path, "/proc/self/fd/" );
        strncat( path, entry->d_name, 20 );
        const ssize_t length = readlink( path, target, sizeof( target ) - 1 );
        if ( length > 7 )
        {
            target[ length ] = '\0';
            if ( 0 == strncmp( target, "socket:", 7 ) )
                ++count;
        }
    }
    closedir( dir );
    return count;

    #else

    return -1;

    #endif
}

/** The number of descriptors of this process that are open at all, -1 when this platform cannot tell */
inline int
CountOpenDescriptors( void )
{
    #if defined( __linux__ ) || defined( __ANDROID__ )

    DIR* dir = opendir( "/proc/self/fd" );
    if ( NULL == dir )
        return -1;
    int count = 0;
    const int self = dirfd( dir );
    struct dirent* entry = NULL;
    while ( NULL != ( entry = readdir( dir ) ) )
    {
        if ( '.' == entry->d_name[ 0 ] )
            continue;
        if ( atoi( entry->d_name ) != self )
            ++count;
    }
    closedir( dir );
    return count;

    #else

    return -1;

    #endif
}

}; /* namespace TESTFW */
}; /* namespace GUCEF */

#endif /* __cplusplus ? */

#endif /* GUCEF_TEST_SOCKETS_H ? */
