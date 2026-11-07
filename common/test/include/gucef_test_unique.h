/*
 *  gucef common header: header-only unique names, directories and free ports for tests
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

/*
 *  Test support: two instances of a test application (parallel CI jobs, parallel developers on one host) share the
 *  temporary directory and the port space of the host, so a test must not use a fixed file name in there or a fixed port.
 *
 *    - A server that is only reached by the client of the same test listens on port 0 and reads the bound port back
 *      (CTCPServerSocket::GetBoundPort(), CUDPSocket::GetPort(), CTestTcpServer::GetPort()).
 *    - When both sides need the port before anything is open (several UDP sockets that talk to each other, a
 *      multicast group), FindFreeUdpPort() / FindFreeTcpPort() ask the system for a port that is free now and hand
 *      out a port only once per process.
 *    - Files and directories: UniqueTestName() adds the process id and a counter to a name, CScopedTestDir makes a
 *      directory with such a name below the temporary directory and deletes it with its contents when it goes out
 *      of scope, also when an assertion leaves the test early.
 *
 *    GUCEF::TESTFW::CScopedTestDir scratch( "gucefCOM_TestApp_Audit" );
 *    const GUCEF::CORE::CString path = GUCEF::CORE::CombinePath( scratch.GetPath(), "audit.jsonl" );
 *
 *  The port lookup is implemented on Linux (BSD sockets). Elsewhere it returns the port that is given as fallback.
 */

#ifndef GUCEF_TEST_UNIQUE_H
#define GUCEF_TEST_UNIQUE_H

#ifdef __cplusplus

#include <string.h>

#if defined( __linux__ ) || defined( __ANDROID__ )

  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>

#elif defined( _WIN32 )

  #include <process.h>

#endif /* platform ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

namespace GUCEF {
namespace TESTFW {

/*-------------------------------------------------------------------------*/

/** The id of this process, 0 when this platform cannot tell */
inline UInt32
TestProcessId( void )
{
    #if defined( __linux__ ) || defined( __ANDROID__ )
    return (UInt32) ::getpid();
    #elif defined( _WIN32 )
    return (UInt32) ::_getpid();
    #else
    return 0;
    #endif
}

/*-------------------------------------------------------------------------*/

/** The name with the process id and a counter behind it: different for every call, and for every instance of the application */
inline CORE::CString
UniqueTestName( const CORE::CString& baseName )
{
    static MT::CTAtomic< UInt32 > counter( 0 );
    return baseName + "_" + CORE::ToString( TestProcessId() ) + "_" + CORE::ToString( counter.FetchAdd( 1 ) );
}

/*-------------------------------------------------------------------------*/

/** A directory with a unique name below the temporary directory, deleted with its contents by the destructor */
class CScopedTestDir
{
    public:

    explicit CScopedTestDir( const CORE::CString& baseName )
        : m_path( CORE::CombinePath( CORE::TempDir(), UniqueTestName( baseName ) ) )
    {
        // a leftover of a crashed run that had the same process id
        if ( CORE::DirExists( m_path ) )
            CORE::DeleteDir( m_path, true );
        CORE::CreateDirs( m_path );
    }

    ~CScopedTestDir()
    {
        if ( CORE::DirExists( m_path ) )
            CORE::DeleteDir( m_path, true );
    }

    const CORE::CString& GetPath( void ) const { return m_path; }

    private:

    CScopedTestDir( const CScopedTestDir& src );
    CScopedTestDir& operator=( const CScopedTestDir& src );

    CORE::CString m_path;
};

/*-------------------------------------------------------------------------*/

/** Asks the system for a port that is free now, for UDP or TCP, and does not hand out the same port twice in this process */
inline UInt16
FindFreePort( bool udp, UInt16 fallbackPort )
{
    #if defined( __linux__ ) || defined( __ANDROID__ )

    static UInt16 handedOut[ 256 ];
    static MT::CTAtomic< UInt32 > handedOutCount( 0 );

    for ( int attempt=0; attempt<64; ++attempt )
    {
        int fd = ::socket( AF_INET, udp ? SOCK_DGRAM : SOCK_STREAM, 0 );
        if ( fd < 0 )
            return fallbackPort;
        struct sockaddr_in addr;
        memset( &addr, 0, sizeof( addr ) );
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl( INADDR_ANY );
        addr.sin_port = 0;
        UInt16 port = 0;
        socklen_t length = sizeof( addr );
        if ( 0 == ::bind( fd, (struct sockaddr*) &addr, sizeof( addr ) ) && 0 == ::getsockname( fd, (struct sockaddr*) &addr, &length ) )
            port = ntohs( addr.sin_port );
        ::close( fd );
        if ( 0 == port )
            continue;

        // the port numbers of the two protocols are separate, but one list keeps this simple and a repeat only costs another attempt
        const UInt32 count = handedOutCount.Load();
        bool repeat = false;
        for ( UInt32 i=0; i<count && i<256; ++i )
            if ( handedOut[ i ] == port )
                repeat = true;
        if ( repeat )
            continue;
        const UInt32 slot = handedOutCount.FetchAdd( 1 );
        if ( slot < 256 )
            handedOut[ slot ] = port;
        return port;
    }
    return fallbackPort;

    #else

    return fallbackPort;

    #endif
}

/*-------------------------------------------------------------------------*/

inline UInt16
FindFreeUdpPort( UInt16 fallbackPort = 0 )
{
    return FindFreePort( true, fallbackPort );
}

/*-------------------------------------------------------------------------*/

inline UInt16
FindFreeTcpPort( UInt16 fallbackPort = 0 )
{
    return FindFreePort( false, fallbackPort );
}

/*-------------------------------------------------------------------------*/

}; /* namespace TESTFW */
}; /* namespace GUCEF */

#endif /* __cplusplus ? */

#endif /* GUCEF_TEST_UNIQUE_H ? */
