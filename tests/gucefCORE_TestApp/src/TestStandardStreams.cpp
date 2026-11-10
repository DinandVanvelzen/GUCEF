/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#include <string.h>
#include <stdlib.h>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CSTANDARDINPUTREADER_H
#include "gucefCORE_CStandardInputReader.h"
#define GUCEF_CORE_CSTANDARDINPUTREADER_H
#endif /* GUCEF_CORE_CSTANDARDINPUTREADER_H ? */

#ifndef GUCEF_CORE_CSTANDARDOUTPUTWRITER_H
#include "gucefCORE_CStandardOutputWriter.h"
#define GUCEF_CORE_CSTANDARDOUTPUTWRITER_H
#endif /* GUCEF_CORE_CSTANDARDOUTPUTWRITER_H ? */

#ifndef GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H
#include "gucefCORE_CPlatformNativeConsoleLogger.h"
#define GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H
#endif /* GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H ? */

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#include "gucefCORE_CIMonotonicClock.h"
#define GUCEF_CORE_CIMONOTONICCLOCK_H
#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */

#ifndef GUCEF_CORE_CTIMESTAMP_H
#include "gucefCORE_CTimestamp.h"
#define GUCEF_CORE_CTIMESTAMP_H
#endif /* GUCEF_CORE_CTIMESTAMP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
  #ifndef GUCEF_CORE_CTERMINALCONSOLELOGGER_H
  #include "gucefCORE_CTerminalConsoleLogger.h"
  #define GUCEF_CORE_CTERMINALCONSOLELOGGER_H
  #endif /* GUCEF_CORE_CTERMINALCONSOLELOGGER_H ? */
#endif

#if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
  #include <unistd.h>
  #include <fcntl.h>
  #include <poll.h>
  #include <signal.h>
  #include <errno.h>
#endif

#include "TestStandardStreams.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )

static UInt64
NowInMs( void )
{
    return CSystemMonotonicClock::Instance().GetMonotonicTimeInMs();
}

/*-------------------------------------------------------------------------*/

namespace {

/** A pipe made by the test; the ends that are still open are closed when it goes */
class CTestPipe
{
    public:

    int readEnd;
    int writeEnd;

    CTestPipe( void )
        : readEnd( -1 )
        , writeEnd( -1 )
    {
        int ends[ 2 ] = { -1, -1 };
        if ( 0 == pipe( ends ) )
        {
            readEnd = ends[ 0 ];
            writeEnd = ends[ 1 ];
        }
    }

    ~CTestPipe()
    {
        CloseRead();
        CloseWrite();
    }

    bool IsOpen( void ) const
    {
        return -1 != readEnd && -1 != writeEnd;
    }

    void CloseRead( void )
    {
        if ( -1 != readEnd )
            close( readEnd );
        readEnd = -1;
    }

    void CloseWrite( void )
    {
        if ( -1 != writeEnd )
            close( writeEnd );
        writeEnd = -1;
    }

    bool Write( const char* text )
    {
        size_t size = strlen( text );
        size_t done = 0;
        while ( done < size )
        {
            ssize_t written = write( writeEnd, text + done, size - done );
            if ( written < 0 )
            {
                if ( EINTR == errno )
                    continue;
                return false;
            }
            done += (size_t) written;
        }
        return true;
    }

    /** Writes until the pipe takes no more: a reader that stopped reading */
    bool Fill( void )
    {
        char block[ 4096 ];
        memset( block, 'f', sizeof( block ) );
        for ( UInt32 i=0; i<100000; ++i )
        {
            struct pollfd waitFor;
            waitFor.fd = writeEnd;
            waitFor.events = POLLOUT;
            waitFor.revents = 0;
            if ( poll( &waitFor, 1, 0 ) <= 0 || 0 == ( waitFor.revents & POLLOUT ) )
                return true;
            if ( write( writeEnd, block, sizeof( block ) ) < 0 )
                return false;
        }
        return false;
    }

    /** Reads until 'bytes' bytes arrived, the writer closed its end, or the timeout passed */
    CString ReadUpTo( size_t bytes, UInt32 timeoutInMs )
    {
        GUCEF::vector< char > data;
        UInt64 deadline = NowInMs() + timeoutInMs;
        char buffer[ 4096 ];
        while ( data.size() < bytes )
        {
            UInt64 now = NowInMs();
            if ( now >= deadline )
                break;
            struct pollfd waitFor;
            waitFor.fd = readEnd;
            waitFor.events = POLLIN;
            waitFor.revents = 0;
            if ( poll( &waitFor, 1, (int) ( deadline - now ) ) <= 0 )
                continue;
            ssize_t got = read( readEnd, buffer, sizeof( buffer ) );
            if ( got <= 0 )
                break;
            data.insert( data.end(), buffer, buffer + got );
        }
        return data.empty() ? CString() : CString( &data[ 0 ], (UInt32) data.size() );
    }

    private:

    CTestPipe( const CTestPipe& src );
    CTestPipe& operator=( const CTestPipe& src );
};

/*-------------------------------------------------------------------------*/

/** Collects what a CStandardInputReader reports; the events come in on the reader thread */
class CReaderListener : public CObservingNotifier
{
    public:

    explicit CReaderListener( CStandardInputReader& reader )
        : CObservingNotifier()
        , m_lines()
        , m_tooLongSizes()
        , m_endOfInputCount( 0 )
        , m_readErrorCount( 0 )
        , m_dataLock()
    {
        TEventCallback lineCallback( this, &CReaderListener::OnLineReceived );
        SubscribeTo( &reader, CStandardInputReader::LineReceivedEvent, lineCallback );
        TEventCallback tooLongCallback( this, &CReaderListener::OnLineTooLong );
        SubscribeTo( &reader, CStandardInputReader::LineTooLongEvent, tooLongCallback );
        TEventCallback endCallback( this, &CReaderListener::OnEndOfInput );
        SubscribeTo( &reader, CStandardInputReader::EndOfInputEvent, endCallback );
        TEventCallback errorCallback( this, &CReaderListener::OnReadError );
        SubscribeTo( &reader, CStandardInputReader::ReadErrorEvent, errorCallback );
    }

    virtual ~CReaderListener() GUCEF_VIRTUAL_OVERRIDE
    {
        UnsubscribeAllFromObserver();
    }

    /** Waits until the end of the input was reported or the timeout passed */
    bool WaitForEnd( UInt32 timeoutInMs )
    {
        UInt64 deadline = NowInMs() + timeoutInMs;
        while ( NowInMs() < deadline )
        {
            if ( GetEndOfInputCount() > 0 || GetReadErrorCount() > 0 )
                return true;
            MT::ThreadDelay( 5 );
        }
        return false;
    }

    /** Waits until at least 'count' lines arrived or the timeout passed */
    bool WaitForLines( UInt32 count, UInt32 timeoutInMs )
    {
        UInt64 deadline = NowInMs() + timeoutInMs;
        while ( NowInMs() < deadline )
        {
            if ( GetLines().size() >= count )
                return true;
            MT::ThreadDelay( 5 );
        }
        return false;
    }

    GUCEF::vector< CString > GetLines( void ) const
    {
        MT::CScopeMutex lock( m_dataLock );
        return m_lines;
    }

    GUCEF::vector< UInt32 > GetTooLongSizes( void ) const
    {
        MT::CScopeMutex lock( m_dataLock );
        return m_tooLongSizes;
    }

    UInt32 GetEndOfInputCount( void ) const
    {
        MT::CScopeMutex lock( m_dataLock );
        return m_endOfInputCount;
    }

    UInt32 GetReadErrorCount( void ) const
    {
        MT::CScopeMutex lock( m_dataLock );
        return m_readErrorCount;
    }

    private:

    typedef CTEventHandlerFunctor< CReaderListener > TEventCallback;

    void OnLineReceived( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        MT::CScopeMutex lock( m_dataLock );
        m_lines.push_back( static_cast< CStandardInputReader::TLineReceivedEventData* >( eventData )->GetData() );
    }

    void OnLineTooLong( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        MT::CScopeMutex lock( m_dataLock );
        m_tooLongSizes.push_back( static_cast< CStandardInputReader::TLineTooLongEventData* >( eventData )->GetData() );
    }

    void OnEndOfInput( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        MT::CScopeMutex lock( m_dataLock );
        ++m_endOfInputCount;
    }

    void OnReadError( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        MT::CScopeMutex lock( m_dataLock );
        ++m_readErrorCount;
    }

    GUCEF::vector< CString > m_lines;
    GUCEF::vector< UInt32 > m_tooLongSizes;
    UInt32 m_endOfInputCount;
    UInt32 m_readErrorCount;
    MT::CMutex m_dataLock;
};

/*-------------------------------------------------------------------------*/

/** Counts BrokenPipeEvent of a CStandardOutputWriter */
class CWriterListener : public CObservingNotifier
{
    public:

    explicit CWriterListener( CStandardOutputWriter& writer )
        : CObservingNotifier()
        , m_brokenCount( 0 )
        , m_dataLock()
    {
        TEventCallback brokenCallback( this, &CWriterListener::OnBrokenPipe );
        SubscribeTo( &writer, CStandardOutputWriter::BrokenPipeEvent, brokenCallback );
    }

    virtual ~CWriterListener() GUCEF_VIRTUAL_OVERRIDE
    {
        UnsubscribeAllFromObserver();
    }

    UInt32 GetBrokenCount( void ) const
    {
        MT::CScopeMutex lock( m_dataLock );
        return m_brokenCount;
    }

    bool WaitForBroken( UInt32 timeoutInMs )
    {
        UInt64 deadline = NowInMs() + timeoutInMs;
        while ( NowInMs() < deadline )
        {
            if ( GetBrokenCount() > 0 )
                return true;
            MT::ThreadDelay( 5 );
        }
        return false;
    }

    private:

    typedef CTEventHandlerFunctor< CWriterListener > TEventCallback;

    void OnBrokenPipe( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        MT::CScopeMutex lock( m_dataLock );
        ++m_brokenCount;
    }

    UInt32 m_brokenCount;
    MT::CMutex m_dataLock;
};

/*-------------------------------------------------------------------------*/

/** Holds a stdout claim for as long as it exists, so a failing test cannot leave the process claimed */
class CStdoutClaimScope
{
    public:

    CStdoutClaimScope( void )
        : m_claimed( CStandardOutputWriter::ClaimExclusive() )
    {
    }

    ~CStdoutClaimScope()
    {
        if ( m_claimed )
            CStandardOutputWriter::ReleaseClaim();
    }

    bool IsClaimed( void ) const
    {
        return m_claimed;
    }

    private:

    CStdoutClaimScope( const CStdoutClaimScope& src );
    CStdoutClaimScope& operator=( const CStdoutClaimScope& src );

    bool m_claimed;
};

/*-------------------------------------------------------------------------*/

/** Points a standard file descriptor at a pipe for as long as it exists */
class CStandardFdRedirectScope
{
    public:

    CStandardFdRedirectScope( int standardFd, int target )
        : m_standardFd( standardFd )
        , m_saved( dup( standardFd ) )
    {
        dup2( target, standardFd );
    }

    ~CStandardFdRedirectScope()
    {
        if ( -1 != m_saved )
        {
            dup2( m_saved, m_standardFd );
            close( m_saved );
        }
    }

    private:

    CStandardFdRedirectScope( const CStandardFdRedirectScope& src );
    CStandardFdRedirectScope& operator=( const CStandardFdRedirectScope& src );

    int m_standardFd;
    int m_saved;
};

/*-------------------------------------------------------------------------*/

/** Sets an environment variable for as long as it exists */
class CStreamsEnvironmentScope
{
    public:

    CStreamsEnvironmentScope( const char* name, const char* value )
        : m_name( name )
        , m_hadValue( GUCEF_NULL != getenv( name ) )
        , m_oldValue( GUCEF_NULL != getenv( name ) ? getenv( name ) : "" )
    {
        setenv( name, value, 1 );
    }

    ~CStreamsEnvironmentScope()
    {
        if ( m_hadValue )
            setenv( m_name.C_String(), m_oldValue.C_String(), 1 );
        else
            unsetenv( m_name.C_String() );
    }

    private:

    CStreamsEnvironmentScope( const CStreamsEnvironmentScope& src );
    CStreamsEnvironmentScope& operator=( const CStreamsEnvironmentScope& src );

    CString m_name;
    bool m_hadValue;
    CString m_oldValue;
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

static void
PerformReaderTests( void )
{
    GUCEF_TESTFW_TESTCASE( "CStandardInputReader: lines, CRLF and the end of the input" )
        try
        {
            CTestPipe input;
            ASSERT_TRUE( input.IsOpen() );
            CStandardInputReader reader( 64, input.readEnd );
            CReaderListener listener( reader );
            ASSERT_TRUE( reader.Start() );
            ASSERT_FALSE( reader.Start() );

            ASSERT_TRUE( input.Write( "one\ntwo\r\n\nthree\n" ) );
            ASSERT_TRUE( input.Write( "par" ) );
            MT::ThreadDelay( 30 );
            ASSERT_TRUE( input.Write( "tial\nlast without a line end" ) );
            input.CloseWrite();
            ASSERT_TRUE( listener.WaitForEnd( 5000 ) );

            GUCEF::vector< CString > lines = listener.GetLines();
            ASSERT_TRUE( 6 == lines.size() );
            if ( 6 == lines.size() )
            {
                ASSERT_TRUE( "one" == lines[ 0 ] );
                ASSERT_TRUE( "two" == lines[ 1 ] );
                ASSERT_TRUE( lines[ 2 ].IsNULLOrEmpty() );
                ASSERT_TRUE( "three" == lines[ 3 ] );
                ASSERT_TRUE( "partial" == lines[ 4 ] );
                ASSERT_TRUE( "last without a line end" == lines[ 5 ] );
            }
            ASSERT_TRUE( 1 == listener.GetEndOfInputCount() );
            ASSERT_TRUE( 0 == listener.GetReadErrorCount() );
            ASSERT_TRUE( listener.GetTooLongSizes().empty() );
            reader.Stop();
            ASSERT_FALSE( reader.IsStarted() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CStandardInputReader: a line beyond maxLineBytes is skipped and reported" )
        try
        {
            CTestPipe input;
            CStandardInputReader reader( 10, input.readEnd );
            CReaderListener listener( reader );
            ASSERT_TRUE( reader.Start() );

            CString longLine;
            for ( UInt32 i=0; i<50; ++i )
                longLine += "x";
            ASSERT_TRUE( input.Write( "short\n" ) );
            ASSERT_TRUE( input.Write( ( longLine + "\n" ).C_String() ) );
            ASSERT_TRUE( input.Write( "exactly 10\n" ) );
            // a long line that arrives in parts
            ASSERT_TRUE( input.Write( "12345678" ) );
            MT::ThreadDelay( 30 );
            ASSERT_TRUE( input.Write( "90abcdef\nafter\n" ) );
            input.CloseWrite();
            ASSERT_TRUE( listener.WaitForEnd( 5000 ) );

            GUCEF::vector< CString > lines = listener.GetLines();
            ASSERT_TRUE( 3 == lines.size() );
            if ( 3 == lines.size() )
            {
                ASSERT_TRUE( "short" == lines[ 0 ] );
                ASSERT_TRUE( "exactly 10" == lines[ 1 ] );
                ASSERT_TRUE( "after" == lines[ 2 ] );
            }
            GUCEF::vector< UInt32 > tooLong = listener.GetTooLongSizes();
            ASSERT_TRUE( 2 == tooLong.size() );
            if ( 2 == tooLong.size() )
            {
                ASSERT_TRUE( 50 == tooLong[ 0 ] );
                ASSERT_TRUE( 16 == tooLong[ 1 ] );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CStandardInputReader: many lines across the read buffer are all delivered in order" )
        try
        {
            CTestPipe input;
            CStandardInputReader reader( 1024, input.readEnd );
            CReaderListener listener( reader );
            ASSERT_TRUE( reader.Start() );

            // more than the pipe holds and more than one read buffer: the writes wait for the reader
            const UInt32 lineCount = 20000;
            CString block;
            for ( UInt32 i=0; i<lineCount; ++i )
            {
                block += "{\"line\":" + UInt32ToString( i ) + "}\r\n";
                if ( 0 == ( i % 1000 ) )
                {
                    ASSERT_TRUE( input.Write( block.C_String() ) );
                    block = CString();
                }
            }
            ASSERT_TRUE( input.Write( block.C_String() ) );
            input.CloseWrite();
            ASSERT_TRUE( listener.WaitForEnd( 20000 ) );

            GUCEF::vector< CString > lines = listener.GetLines();
            ASSERT_TRUE( lineCount == lines.size() );
            bool inOrder = lineCount == lines.size();
            for ( UInt32 i=0; i<lines.size() && inOrder; ++i )
                inOrder = ( "{\"line\":" + UInt32ToString( i ) + "}" ) == lines[ i ];
            ASSERT_TRUE( inOrder );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CStandardInputReader: Stop() while the thread waits for input" )
        try
        {
            CTestPipe input;
            CStandardInputReader reader( 64, input.readEnd );
            CReaderListener listener( reader );
            ASSERT_TRUE( reader.Start() );
            ASSERT_TRUE( input.Write( "before\n" ) );
            ASSERT_TRUE( listener.WaitForLines( 1, 5000 ) );

            // the writer keeps its end open: the thread waits until it is woken
            UInt64 start = NowInMs();
            reader.Stop();
            UInt64 duration = NowInMs() - start;
            ASSERT_TRUE( duration < 2000 );
            ASSERT_FALSE( reader.IsStarted() );
            ASSERT_TRUE( 0 == listener.GetEndOfInputCount() );

            // what comes after the stop stays in the stream for the next start
            ASSERT_TRUE( input.Write( "after\n" ) );
            ASSERT_TRUE( reader.Start() );
            ASSERT_TRUE( listener.WaitForLines( 2, 5000 ) );
            GUCEF::vector< CString > lines = listener.GetLines();
            ASSERT_TRUE( 2 == lines.size() && "after" == lines[ 1 ] );
            reader.Stop();

            // and the destructor stops a running reader too
            CStandardInputReader* other = GUCEF_NEW CStandardInputReader( 64, input.readEnd );
            ASSERT_TRUE( other->Start() );
            MT::ThreadDelay( 20 );
            start = NowInMs();
            GUCEF_DELETE other;
            ASSERT_TRUE( NowInMs() - start < 2000 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CStandardInputReader: a stream that is not open is a read error" )
        try
        {
            // a file descriptor number nothing has open
            CStandardInputReader reader( 64, 999999 );
            CReaderListener listener( reader );
            ASSERT_TRUE( reader.Start() );
            ASSERT_TRUE( listener.WaitForEnd( 5000 ) );
            ASSERT_TRUE( 1 == listener.GetReadErrorCount() );
            ASSERT_TRUE( 0 == listener.GetEndOfInputCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static bool
SameSigPipeDisposition( const struct sigaction& a, const struct sigaction& b )
{
    return a.sa_handler == b.sa_handler;
}

/*-------------------------------------------------------------------------*/

static void
PerformWriterTests( void )
{
    GUCEF_TESTFW_TESTCASE( "CStandardOutputWriter: the stdout claim is process wide and exclusive" )
        try
        {
            ASSERT_FALSE( CStandardOutputWriter::IsClaimed() );
            {
                CStdoutClaimScope claim;
                ASSERT_TRUE( claim.IsClaimed() );
                ASSERT_TRUE( CStandardOutputWriter::IsClaimed() );
                ASSERT_FALSE( CStandardOutputWriter::ClaimExclusive() );
            }
            ASSERT_FALSE( CStandardOutputWriter::IsClaimed() );
            ASSERT_TRUE( CStandardOutputWriter::ClaimExclusive() );
            CStandardOutputWriter::ReleaseClaim();
            ASSERT_FALSE( CStandardOutputWriter::IsClaimed() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CStandardOutputWriter: lines are written whole, each with one line end" )
        try
        {
            CTestPipe output;
            CStandardOutputWriter writer( 4 * 1024 * 1024, output.writeEnd );
            ASSERT_FALSE( writer.WriteLine( "not started" ) );
            ASSERT_TRUE( writer.Start() );
            ASSERT_FALSE( writer.Start() );

            ASSERT_TRUE( writer.WriteLine( "a" ) );
            ASSERT_TRUE( writer.WriteLine( "" ) );
            ASSERT_TRUE( writer.WriteLine( "{\"jsonrpc\":\"2.0\",\"id\":1}" ) );
            ASSERT_FALSE( writer.WriteLine( "two\nlines" ) );
            ASSERT_TRUE( writer.WriteLine( "carriage\rreturn is no line end" ) );

            CString expected = "a\n\n{\"jsonrpc\":\"2.0\",\"id\":1}\ncarriage\rreturn is no line end\n";
            CString got = output.ReadUpTo( expected.ByteSize() - 1, 5000 );
            ASSERT_TRUE( expected == got );

            // many lines, more than the pipe holds, read while they are written
            CString line;
            for ( UInt32 i=0; i<1000; ++i )
                line += "0123456789";
            const UInt32 lineCount = 200;
            for ( UInt32 i=0; i<lineCount; ++i )
                ASSERT_TRUE( writer.WriteLine( line ) );
            CString all = output.ReadUpTo( lineCount * ( line.ByteSize() ), 10000 );
            ASSERT_TRUE( lineCount * line.ByteSize() == all.ByteSize() - 1 );
            bool whole = true;
            const char* data = all.C_String();
            for ( UInt32 i=0; i<lineCount && whole && all.ByteSize() - 1 == lineCount * line.ByteSize(); ++i )
                whole = 0 == memcmp( data + i * line.ByteSize(), line.C_String(), line.ByteSize() - 1 ) && '\n' == data[ ( i + 1 ) * line.ByteSize() - 1 ];
            ASSERT_TRUE( whole );

            ASSERT_TRUE( writer.WriteLine( "last" ) );
            writer.Stop( true );
            ASSERT_TRUE( "last\n" == output.ReadUpTo( 5, 5000 ) );
            ASSERT_FALSE( writer.WriteLine( "after the stop" ) );
            ASSERT_FALSE( writer.IsBroken() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CStandardOutputWriter: a reader that stops reading fills the queue, which refuses lines without blocking" )
        try
        {
            CTestPipe output;
            ASSERT_TRUE( output.Fill() );
            CStandardOutputWriter writer( 16 * 1024, output.writeEnd );
            ASSERT_TRUE( writer.Start() );

            CString line;
            for ( UInt32 i=0; i<100; ++i )
                line += "0123456789";
            bool refused = false;
            UInt64 start = NowInMs();
            for ( UInt32 i=0; i<10000 && !refused; ++i )
            {
                if ( !writer.WriteLine( line ) )
                    refused = true;
                else if ( 0 == ( i % 100 ) )
                    MT::ThreadDelay( 1 );       // the writer thread fills the pipe meanwhile
            }
            ASSERT_TRUE( refused );
            ASSERT_TRUE( NowInMs() - start < 5000 );
            MT::ThreadDelay( 50 );
            ASSERT_TRUE( writer.GetQueuedBytes() > 0 );
            ASSERT_TRUE( writer.GetQueuedBytes() <= 16 * 1024 );
            ASSERT_FALSE( writer.IsBroken() );

            // a flushing stop gives up at its deadline, a stalled reader cannot hold it up
            start = NowInMs();
            writer.Stop( true, 200 );
            UInt64 duration = NowInMs() - start;
            GUCEF_LOG( LOGLEVEL_NORMAL, "StandardStreams test: the flushing stop with a stalled reader took " << duration << " ms" );
            ASSERT_TRUE( duration >= 150 );
            ASSERT_TRUE( duration < 3000 );

            // and a stop without flush is quick
            ASSERT_TRUE( writer.Start() );
            for ( UInt32 i=0; i<10; ++i )
                writer.WriteLine( line );
            start = NowInMs();
            writer.Stop( false );
            ASSERT_TRUE( NowInMs() - start < 2000 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CStandardOutputWriter: a broken pipe is an event, not a SIGPIPE, and the process disposition stays" )
        try
        {
            struct sigaction before;
            memset( &before, 0, sizeof( before ) );
            ASSERT_TRUE( 0 == sigaction( SIGPIPE, GUCEF_NULL, &before ) );

            CTestPipe output;
            CStandardOutputWriter writer( 1024 * 1024, output.writeEnd );
            CWriterListener listener( writer );
            ASSERT_TRUE( writer.Start() );
            ASSERT_TRUE( writer.WriteLine( "read" ) );
            ASSERT_TRUE( "read\n" == output.ReadUpTo( 5, 5000 ) );

            output.CloseRead();
            for ( UInt32 i=0; i<100 && !writer.IsBroken(); ++i )
            {
                writer.WriteLine( "into a closed pipe" );
                MT::ThreadDelay( 5 );
            }
            ASSERT_TRUE( listener.WaitForBroken( 5000 ) );
            ASSERT_TRUE( writer.IsBroken() );
            ASSERT_TRUE( 1 == listener.GetBrokenCount() );
            ASSERT_FALSE( writer.WriteLine( "after the break" ) );
            writer.Stop( true );

            struct sigaction after;
            memset( &after, 0, sizeof( after ) );
            ASSERT_TRUE( 0 == sigaction( SIGPIPE, GUCEF_NULL, &after ) );
            ASSERT_TRUE( SameSigPipeDisposition( before, after ) );

            sigset_t pending;
            sigemptyset( &pending );
            sigpending( &pending );
            ASSERT_FALSE( 0 != sigismember( &pending, SIGPIPE ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CStandardOutputWriter: the process-wide SIGPIPE disposition is an explicit setting" )
        try
        {
            struct sigaction original;
            memset( &original, 0, sizeof( original ) );
            ASSERT_TRUE( 0 == sigaction( SIGPIPE, GUCEF_NULL, &original ) );

            ASSERT_TRUE( CStandardOutputWriter::SetSigPipeIgnoredProcessWide( true ) );
            struct sigaction current;
            memset( &current, 0, sizeof( current ) );
            sigaction( SIGPIPE, GUCEF_NULL, &current );
            ASSERT_TRUE( SIG_IGN == current.sa_handler );

            ASSERT_TRUE( CStandardOutputWriter::SetSigPipeIgnoredProcessWide( false ) );
            sigaction( SIGPIPE, GUCEF_NULL, &current );
            ASSERT_TRUE( SIG_DFL == current.sa_handler );

            sigaction( SIGPIPE, &original, GUCEF_NULL );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CStandardOutputWriter: a stopped writer starts again" )
        try
        {
            CTestPipe output;
            CStandardOutputWriter writer( 1024, output.writeEnd );
            for ( UInt32 round=0; round<3; ++round )
            {
                ASSERT_TRUE( writer.Start() );
                ASSERT_TRUE( writer.WriteLine( "round" ) );
                writer.Stop( true );
                ASSERT_TRUE( "round\n" == output.ReadUpTo( 6, 5000 ) );
                ASSERT_TRUE( 0 == writer.GetQueuedBytes() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformConsoleLoggerRuleTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Console logger: the console kind while stdout is claimed" )
        try
        {
            typedef CPlatformNativeConsoleLogger L;
            // without a forced kind: stderr, never a window of its own
            ASSERT_TRUE( L::CONSOLEKIND_STDERR == L::ChooseConsoleKind( GUCEF_NULL, true, true, false, true ) );
            ASSERT_TRUE( L::CONSOLEKIND_STDERR == L::ChooseConsoleKind( GUCEF_NULL, false, true, false, true ) );
            ASSERT_TRUE( L::CONSOLEKIND_STDERR == L::ChooseConsoleKind( GUCEF_NULL, false, true, true, true ) );
            ASSERT_TRUE( L::CONSOLEKIND_STDERR == L::ChooseConsoleKind( "auto", true, true, true, true ) );
            ASSERT_TRUE( L::CONSOLEKIND_STDERR == L::ChooseConsoleKind( "", false, false, false, true ) );
            // a forced kind goes over the claim
            ASSERT_TRUE( L::CONSOLEKIND_NONE == L::ChooseConsoleKind( "none", true, true, false, true ) );
            ASSERT_TRUE( L::CONSOLEKIND_DESKTOP_TERMINAL == L::ChooseConsoleKind( "desktop", false, true, false, true ) );
            ASSERT_TRUE( L::CONSOLEKIND_TERMINAL == L::ChooseConsoleKind( "terminal", true, true, false, true ) );
            // the stderr kind is also there without a claim
            ASSERT_TRUE( L::CONSOLEKIND_STDERR == L::ChooseConsoleKind( "stderr", false, false, false, false ) );
            ASSERT_TRUE( L::CONSOLEKIND_STDERR == L::ChooseConsoleKind( "stderr", true, true, true, false ) );
            // and without a claim nothing changes
            ASSERT_TRUE( L::CONSOLEKIND_TERMINAL == L::ChooseConsoleKind( GUCEF_NULL, true, true, false, false ) );
            ASSERT_TRUE( L::CONSOLEKIND_TERMINAL == L::ChooseConsoleKind( GUCEF_NULL, true, true, false ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    GUCEF_TESTFW_TESTCASE( "Console logger: with stdout claimed the terminal logger never writes to stdout" )
        try
        {
            CTestPipe stdoutPipe;
            CTestPipe stderrPipe;
            CTerminalConsoleLogger madeBefore( STDOUT_FILENO );
            {
                CStdoutClaimScope claim;
                ASSERT_TRUE( claim.IsClaimed() );
                ASSERT_TRUE( STDOUT_FILENO != CTerminalConsoleLogger::GetTerminalFileDescriptor() );

                CStandardFdRedirectScope stdoutRedirect( STDOUT_FILENO, stdoutPipe.writeEnd );
                CStandardFdRedirectScope stderrRedirect( STDERR_FILENO, stderrPipe.writeEnd );
                madeBefore.LogWithoutFormatting( LOG_STANDARD, LOGLEVEL_CRITICAL, "made before the claim", 0, CTimestamp() );
            }
            stdoutPipe.CloseWrite();
            stderrPipe.CloseWrite();
            ASSERT_TRUE( stdoutPipe.ReadUpTo( 1, 200 ).IsNULLOrEmpty() );
            ASSERT_TRUE( stderrPipe.ReadUpTo( 1024, 2000 ).HasSubstr( "made before the claim" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Console logger: the stderr kind writes text lines to stderr, also when it is a pipe" )
        try
        {
            CTestPipe stdoutPipe;
            CTestPipe stderrPipe;
            {
                // forced, so this cannot open a window whatever the environment of the test is
                CStreamsEnvironmentScope kind( "GUCEF_CONSOLE_LOGGER", "stderr" );
                CStdoutClaimScope claim;
                CStandardFdRedirectScope stdoutRedirect( STDOUT_FILENO, stdoutPipe.writeEnd );
                CStandardFdRedirectScope stderrRedirect( STDERR_FILENO, stderrPipe.writeEnd );

                CPlatformNativeConsoleLogger console;
                CIConsoleLogger* logger = console.GetLogger();
                ASSERT_TRUE( GUCEF_NULL != logger );
                if ( GUCEF_NULL != logger )
                    logger->LogWithoutFormatting( LOG_STANDARD, LOGLEVEL_CRITICAL, "to the stderr console", 0, CTimestamp() );
            }
            stdoutPipe.CloseWrite();
            stderrPipe.CloseWrite();
            ASSERT_TRUE( stdoutPipe.ReadUpTo( 1, 200 ).IsNULLOrEmpty() );
            ASSERT_TRUE( stderrPipe.ReadUpTo( 1024, 2000 ).HasSubstr( "to the stderr console" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Console logger: the stderr console survives a reader that went away" )
        try
        {
            CTestPipe stderrPipe;
            CTerminalConsoleLogger logger( stderrPipe.writeEnd );
            stderrPipe.CloseRead();
            // without the SIGPIPE guard this ends the test application
            logger.LogWithoutFormatting( LOG_STANDARD, LOGLEVEL_CRITICAL, "nobody reads this", 0, CTimestamp() );
            ASSERT_TRUE( 1 == logger.GetDroppedLineCount() );
            sigset_t pending;
            sigemptyset( &pending );
            sigpending( &pending );
            ASSERT_FALSE( 0 != sigismember( &pending, SIGPIPE ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    #endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ? */
}

#endif /* GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN ? */

/*-------------------------------------------------------------------------*/

void
PerformStandardStreamsTests( void )
{
    GUCEF_LOG( LOGLEVEL_NORMAL, "COMMENCING standard streams TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "StandardStreams" );

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )

    PerformReaderTests();
    PerformWriterTests();
    PerformConsoleLoggerRuleTests();

    #else

    GUCEF_TESTFW_TESTCASE( "Standard streams: the tests use POSIX pipes" )
        GUCEF_TESTFW_SKIP( "the standard stream tests are written for POSIX pipes" );
    GUCEF_TESTFW_TESTCASE_END;

    #endif
}

/*-------------------------------------------------------------------------*/
