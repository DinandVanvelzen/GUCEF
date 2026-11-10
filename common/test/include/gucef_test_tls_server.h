/*
 *  gucef common header: header-only support for tests of TLS against the openssl command line tool
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
 *  Test support: throwaway certificates and a TLS server, both made with the "openssl" command line tool of the
 *  system, launched with CORE::CProcess. Nothing is stored: the certificates (which have validity dates) are
 *  made when the test runs, in a directory that is removed again, and the server is a process that is stopped
 *  by its object. Without the tool IsToolAvailable() is false and a test skips what needs it:
 *
 *    GUCEF_TESTFW_SKIP_IF( !GUCEF::TESTFW::CTlsTestCerts::IsToolAvailable(), "the openssl tool is not installed" );
 *    GUCEF::TESTFW::CTlsTestCerts certs;
 *    GUCEF::TESTFW::CTlsTestServer server;
 *    ASSERT_TRUE( server.Start( certs, "good", GUCEF::TESTFW::CTlsTestServer::SERVE_STATUS_PAGE ) );
 *    ... connect to server.GetPort() with certs.GetPath( "ca.crt" ) as the CA ...
 *
 *  The certificates: ca (the authority), other_ca (an authority nobody trusts), good (names localhost and 127.0.0.1),
 *  wrongname (names other.example), expired (2020 to 2021), future (2040 to 2041), cnonly (a common name, no
 *  subject alternative names), named (names tls-target.test, localhost and 127.0.0.1: for tests of proxies, which are
 *  given a host name that does not resolve), otherca_leaf (names localhost, signed by other_ca) and selfsigned (names localhost).
 *  Only supported where CORE::CProcess launches processes and sockets are BSD sockets (Linux and the like).
 */

#ifndef GUCEF_TEST_TLS_SERVER_H
#define GUCEF_TEST_TLS_SERVER_H

#ifdef __cplusplus

#if defined( __linux__ )

  #include <stdio.h>
  #include <string.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>

#endif /* Linux ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

namespace GUCEF {
namespace TESTFW {

/*-------------------------------------------------------------------------*/

/** Runs the openssl tool with the arguments and waits for it, in the given directory. @return whether it ended with exit code 0 */
inline bool
RunOpenSslTool( const CORE::CProcess::TStringVector& args, const CORE::CString& workingDir, CORE::UInt32 timeoutInMs = 30000 )
{
    CORE::CProcess process;
    process.SetIsolateFromParent( true );
    if ( !process.Launch( "openssl", args, workingDir ) )
        return false;
    CORE::Int32 exitCode = -1;
    if ( !process.WaitForExit( timeoutInMs, exitCode ) )
    {
        process.Terminate( false );
        return false;
    }
    return 0 == exitCode;
}

/*-------------------------------------------------------------------------*/

/** Throwaway certificates for tests, see the description of this header */
class CTlsTestCerts
{
    public:

    CTlsTestCerts( void )
        : m_dir()
        , m_ok( false )
    {
        if ( !IsToolAvailable() )
            return;

        static int counter = 0;
        #if defined( __linux__ )
        const int processId = (int) getpid();
        #else
        const int processId = 0;
        #endif
        m_dir = CORE::ResolveVars( "$TEMPDIR$" );
        CORE::AppendToPath( m_dir, "gucef_tls_test_" + CORE::ToString( processId ) + "_" + CORE::ToString( ++counter ) );
        // makes the directories in front of the last element of the path: that is a file
        if ( !CORE::CreatePathDirectories( GetPath( "placeholder" ) ) )
            return;
        m_ok = Generate();
    }

    ~CTlsTestCerts()
    {
        if ( !m_dir.IsNULLOrEmpty() && CORE::DirExists( m_dir ) )
            CORE::DeleteDir( m_dir, true );
    }

    /** Whether the openssl tool runs on this system */
    static bool IsToolAvailable( void )
    {
        #if defined( __linux__ )
        static int available = -1;
        if ( available < 0 )
        {
            CORE::CProcess::TStringVector args;
            args.push_back( "version" );
            available = RunOpenSslTool( args, CORE::CString::Empty, 10000 ) ? 1 : 0;
        }
        return 1 == available;
        #else
        return false;
        #endif
    }

    /** Whether the certificates were made */
    bool IsReady( void ) const { return m_ok; }

    const CORE::CString& GetDir( void ) const { return m_dir; }

    /** The path of one of the files, for example "ca.crt" or "good.key" */
    CORE::CString GetPath( const char* fileName ) const
    {
        CORE::CString path = m_dir;
        CORE::AppendToPath( path, fileName );
        return path;
    }

    private:

    CTlsTestCerts( const CTlsTestCerts& src );
    CTlsTestCerts& operator=( const CTlsTestCerts& src );

    bool Run( const CORE::CProcess::TStringVector& args ) const
    {
        return RunOpenSslTool( args, m_dir );
    }

    bool MakeAuthority( const char* name, const char* subject ) const
    {
        const CORE::CString key = CORE::CString( name ) + ".key", crt = CORE::CString( name ) + ".crt";
        CORE::CProcess::TStringVector args;
        args.push_back( "req" ); args.push_back( "-x509" ); args.push_back( "-newkey" ); args.push_back( "ec" );
        args.push_back( "-pkeyopt" ); args.push_back( "ec_paramgen_curve:prime256v1" ); args.push_back( "-nodes" );
        args.push_back( "-keyout" ); args.push_back( key ); args.push_back( "-out" ); args.push_back( crt );
        args.push_back( "-days" ); args.push_back( "3650" ); args.push_back( "-subj" ); args.push_back( subject );
        args.push_back( "-addext" ); args.push_back( "basicConstraints=critical,CA:TRUE" );
        args.push_back( "-addext" ); args.push_back( "keyUsage=critical,keyCertSign,cRLSign" );
        return Run( args );
    }

    /** A certificate for the server: key, request, extensions and the signature by the authority. 'san' empty for none, 'validity' the options of x509 for the dates */
    bool MakeLeaf( const char* name, const char* subject, const char* san, const char* authority, const CORE::CProcess::TStringVector& validity ) const
    {
        const CORE::CString key = CORE::CString( name ) + ".key", csr = CORE::CString( name ) + ".csr", ext = CORE::CString( name ) + ".ext", crt = CORE::CString( name ) + ".crt";
        const CORE::CString caCrt = CORE::CString( authority ) + ".crt", caKey = CORE::CString( authority ) + ".key";

        CORE::CProcess::TStringVector args;
        args.push_back( "req" ); args.push_back( "-newkey" ); args.push_back( "ec" ); args.push_back( "-pkeyopt" ); args.push_back( "ec_paramgen_curve:prime256v1" );
        args.push_back( "-nodes" ); args.push_back( "-keyout" ); args.push_back( key ); args.push_back( "-out" ); args.push_back( csr );
        args.push_back( "-subj" ); args.push_back( subject );
        if ( !Run( args ) )
            return false;

        CORE::CString extText = "basicConstraints=CA:FALSE\nkeyUsage=digitalSignature\nextendedKeyUsage=serverAuth\n";
        if ( 0 != strlen( san ) )
            extText += CORE::CString( "subjectAltName=" ) + san + "\n";
        FILE* extFile = fopen( GetPath( ext.C_String() ).C_String(), "wb" );
        if ( NULL == extFile )
            return false;
        fwrite( extText.C_String(), 1, extText.Length(), extFile );
        fclose( extFile );

        args.clear();
        args.push_back( "x509" ); args.push_back( "-req" ); args.push_back( "-in" ); args.push_back( csr ); args.push_back( "-CA" ); args.push_back( caCrt );
        args.push_back( "-CAkey" ); args.push_back( caKey ); args.push_back( "-CAcreateserial" ); args.push_back( "-out" ); args.push_back( crt );
        args.push_back( "-extfile" ); args.push_back( ext );
        for ( size_t i=0; i<validity.size(); ++i )
            args.push_back( validity[ i ] );
        return Run( args );
    }

    static CORE::CProcess::TStringVector Days( const char* days )
    {
        CORE::CProcess::TStringVector v;
        v.push_back( "-days" ); v.push_back( days );
        return v;
    }

    static CORE::CProcess::TStringVector Dates( const char* notBefore, const char* notAfter )
    {
        CORE::CProcess::TStringVector v;
        v.push_back( "-not_before" ); v.push_back( notBefore ); v.push_back( "-not_after" ); v.push_back( notAfter );
        return v;
    }

    bool Generate( void ) const
    {
        if ( !MakeAuthority( "ca", "/CN=GUCEF Test CA" ) || !MakeAuthority( "other_ca", "/CN=GUCEF Other Test CA" ) )
            return false;

        if ( !MakeLeaf( "good", "/CN=localhost", "DNS:localhost,IP:127.0.0.1", "ca", Days( "30" ) ) ||
             !MakeLeaf( "wrongname", "/CN=other.example", "DNS:other.example", "ca", Days( "30" ) ) ||
             !MakeLeaf( "expired", "/CN=localhost", "DNS:localhost,IP:127.0.0.1", "ca", Dates( "20200101000000Z", "20210101000000Z" ) ) ||
             !MakeLeaf( "future", "/CN=localhost", "DNS:localhost,IP:127.0.0.1", "ca", Dates( "20400101000000Z", "20410101000000Z" ) ) ||
             !MakeLeaf( "cnonly", "/CN=localhost", "", "ca", Days( "30" ) ) ||
             !MakeLeaf( "named", "/CN=tls-target.test", "DNS:tls-target.test,DNS:localhost,IP:127.0.0.1", "ca", Days( "30" ) ) ||
             !MakeLeaf( "otherca_leaf", "/CN=localhost", "DNS:localhost,IP:127.0.0.1", "other_ca", Days( "30" ) ) )
            return false;

        // A certificate that is its own authority
        CORE::CProcess::TStringVector args;
        args.push_back( "req" ); args.push_back( "-x509" ); args.push_back( "-newkey" ); args.push_back( "ec" ); args.push_back( "-pkeyopt" );
        args.push_back( "ec_paramgen_curve:prime256v1" ); args.push_back( "-nodes" ); args.push_back( "-keyout" ); args.push_back( "selfsigned.key" );
        args.push_back( "-out" ); args.push_back( "selfsigned.crt" ); args.push_back( "-days" ); args.push_back( "30" ); args.push_back( "-subj" );
        args.push_back( "/CN=localhost" ); args.push_back( "-addext" ); args.push_back( "basicConstraints=critical,CA:FALSE" );
        args.push_back( "-addext" ); args.push_back( "subjectAltName=DNS:localhost,IP:127.0.0.1" );
        return Run( args );
    }

    private:

    CORE::CString m_dir;
    bool m_ok;
};

/*-------------------------------------------------------------------------*/

/** A TLS server: the openssl tool as "s_server" on a local port, stopped by the destructor */
class CTlsTestServer
{
    public:

    enum EMode
    {
        SERVE_STATUS_PAGE,      /**< -www: answers any request with a page of status information */
        SERVE_FILES,            /**< -WWW: serves the files of the working directory for GET requests */
        SERVE_HTTP_FILES        /**< -HTTP: the same, the files are taken as HTTP responses including their headers */
    };

    CTlsTestServer( void )
        : m_process()
        , m_port( 0 )
    {
        m_process.SetIsolateFromParent( true );
    }

    ~CTlsTestServer()
    {
        Stop();
    }

    /**
     *  Starts the server with the certificate "name" (name.crt and name.key of the certificates), in the working directory given
     *  (the certificate directory when empty), with more options of s_server such as "-tls1_2" or "-alpn" "http/1.1". Waits until it accepts connections.
     */
    bool Start( const CTlsTestCerts& certs, const char* name, EMode mode, const CORE::CProcess::TStringVector& extraArgs = CORE::CProcess::TStringVector(), const CORE::CString& workingDir = CORE::CString::Empty )
    {
        Stop();
        #if defined( __linux__ )
        if ( !certs.IsReady() )
            return false;

        m_port = FindFreePort();
        if ( 0 == m_port )
            return false;

        CORE::CProcess::TStringVector args;
        args.push_back( "s_server" ); args.push_back( "-accept" ); args.push_back( "127.0.0.1:" + CORE::ToString( m_port ) );
        args.push_back( "-cert" ); args.push_back( certs.GetPath( ( CORE::CString( name ) + ".crt" ).C_String() ) );
        args.push_back( "-key" ); args.push_back( certs.GetPath( ( CORE::CString( name ) + ".key" ).C_String() ) );
        args.push_back( "-quiet" );
        args.push_back( SERVE_STATUS_PAGE == mode ? "-www" : ( SERVE_FILES == mode ? "-WWW" : "-HTTP" ) );
        for ( size_t i=0; i<extraArgs.size(); ++i )
            args.push_back( extraArgs[ i ] );

        if ( !m_process.Launch( "openssl", args, workingDir.IsNULLOrEmpty() ? certs.GetDir() : workingDir ) )
            return false;

        // ready when it accepts a connection
        for ( int attempt=0; attempt<250; ++attempt )
        {
            if ( !m_process.IsRunning() )
                return false;
            if ( CanConnect( m_port ) )
                return true;
            MT::ThreadDelay( 20 );
        }
        return false;
        #else
        return false;
        #endif
    }

    void Stop( void )
    {
        if ( 0 != m_process.GetProcessId() )
            m_process.Terminate( false );
        m_port = 0;
    }

    CORE::UInt16 GetPort( void ) const { return m_port; }

    bool IsRunning( void ) { return m_process.IsRunning(); }

    /** A local port nothing listens on at this moment */
    static CORE::UInt16 FindFreePort( void )
    {
        #if defined( __linux__ )
        int sock = ::socket( AF_INET, SOCK_STREAM, 0 );
        if ( sock < 0 )
            return 0;
        struct sockaddr_in addr;
        memset( &addr, 0, sizeof( addr ) );
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
        addr.sin_port = 0;
        CORE::UInt16 port = 0;
        if ( 0 == ::bind( sock, (struct sockaddr*) &addr, sizeof( addr ) ) )
        {
            socklen_t length = sizeof( addr );
            if ( 0 == ::getsockname( sock, (struct sockaddr*) &addr, &length ) )
                port = ntohs( addr.sin_port );
        }
        ::close( sock );
        return port;
        #else
        return 0;
        #endif
    }

    /** Whether a TCP connection to the port on this computer can be made, which it is closed again right away */
    static bool CanConnect( CORE::UInt16 port )
    {
        #if defined( __linux__ )
        int sock = ::socket( AF_INET, SOCK_STREAM, 0 );
        if ( sock < 0 )
            return false;
        struct sockaddr_in addr;
        memset( &addr, 0, sizeof( addr ) );
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
        addr.sin_port = htons( port );
        bool ok = 0 == ::connect( sock, (struct sockaddr*) &addr, sizeof( addr ) );
        ::close( sock );
        return ok;
        #else
        return false;
        #endif
    }

    private:

    CTlsTestServer( const CTlsTestServer& src );
    CTlsTestServer& operator=( const CTlsTestServer& src );

    CORE::CProcess m_process;
    CORE::UInt16 m_port;
};

/*-------------------------------------------------------------------------*/

} /* namespace TESTFW */
} /* namespace GUCEF */

#endif /* __cplusplus ? */

#endif /* GUCEF_TEST_TLS_SERVER_H ? */
