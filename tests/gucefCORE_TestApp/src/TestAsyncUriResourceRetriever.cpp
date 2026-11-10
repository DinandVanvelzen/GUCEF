/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#include "gucefCORE_CUriResourceAccessorFactory.h"
#define GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#endif /* GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H ? */

#ifndef GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H
#include "gucefCORE_CAsyncUriResourceRetriever.h"
#define GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H
#endif /* GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H ? */

#ifndef GUCEF_CORE_CPROGRESSREPORTINGIOACCESS_H
#include "gucefCORE_CProgressReportingIOAccess.h"
#define GUCEF_CORE_CPROGRESSREPORTINGIOACCESS_H
#endif /* GUCEF_CORE_CPROGRESSREPORTINGIOACCESS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CILOGGER_H
#include "CILogger.h"
#define GUCEF_CORE_CILOGGER_H
#endif /* GUCEF_CORE_CILOGGER_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAsyncUriResourceRetriever.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** counts the bytes written and can be told to abort at a given amount */
class CCountingObserver : public CORE::CIIOProgressObserver
{
    public:

    UInt32 reports;
    UInt64 lastRead;
    UInt64 lastWritten;
    UInt64 abortAtWritten;

    CCountingObserver( void ) : reports( 0 ), lastRead( 0 ), lastWritten( 0 ), abortAtWritten( 0 ) {}

    virtual bool OnIOProgress( UInt64 totalBytesRead, UInt64 totalBytesWritten ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++reports;
        lastRead = totalBytesRead;
        lastWritten = totalBytesWritten;
        return 0 == abortAtWritten || totalBytesWritten < abortAtWritten;
    }
};

/*-------------------------------------------------------------------------*/

/** Observer which records what the retriever reports */
class CRetrievalRecorder : public CORE::CObservingNotifier
{
    public:

    UInt32 started;
    UInt32 progress;
    UInt32 completed;
    UInt32 failed;
    UInt32 cancelled;
    UInt64 lastBytes;
    Int64 lastTotal;
    CORE::CString lastError;
    UInt64 maxProgressBytes;

    CRetrievalRecorder( CORE::CAsyncUriResourceRetriever& retriever )
        : CORE::CObservingNotifier()
        , started( 0 ), progress( 0 ), completed( 0 ), failed( 0 ), cancelled( 0 )
        , lastBytes( 0 ), lastTotal( -1 ), lastError(), maxProgressBytes( 0 )
    {
        SubscribeTo( &retriever );
    }

    bool IsFinished( void ) const { return completed + failed + cancelled > 0; }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        const CORE::CAsyncUriRetrievalEventData* data = static_cast< const CORE::CAsyncUriRetrievalEventData* >( eventdata );
        if ( GUCEF_NULL == data )
            return;
        lastBytes = data->bytesTransferred;
        lastTotal = data->totalBytes;
        if ( eventid == CORE::CAsyncUriResourceRetriever::RetrievalStartedEvent ) ++started;
        else if ( eventid == CORE::CAsyncUriResourceRetriever::RetrievalProgressEvent ) { ++progress; if ( data->bytesTransferred > maxProgressBytes ) maxProgressBytes = data->bytesTransferred; }
        else if ( eventid == CORE::CAsyncUriResourceRetriever::RetrievalCompletedEvent ) ++completed;
        else if ( eventid == CORE::CAsyncUriResourceRetriever::RetrievalFailedEvent ) { ++failed; lastError = data->errorText; }
        else if ( eventid == CORE::CAsyncUriResourceRetriever::RetrievalCancelledEvent ) ++cancelled;
    }
};

/*-------------------------------------------------------------------------*/

/** Keeps the text of every log message that is logged while it is attached to the log manager */
class CRetrieverLogCapture : public CORE::CILogger
{
    public:

    CRetrieverLogCapture( void )
        : CORE::CILogger()
        , m_text()
        , m_lock()
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( this );
    }

    virtual ~CRetrieverLogCapture()
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( this );
    }

    /** logs that are still on their way to the loggers are delivered first */
    CORE::CString GetText( void )
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
        MT::CScopeMutex lock( m_lock );
        return m_text;
    }

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex lock( m_lock );
        m_text += logMessage;
        m_text += "\n";
    }

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage, threadId, timestamp );
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
    }

    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE {}

    virtual void SetMinimalLogLevel( const Int32 logLevel ) GUCEF_VIRTUAL_OVERRIDE {}

    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::LOGLEVEL_EVERYTHING; }

    private:

    CORE::CString m_text;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

/**
 *  Accessor of the scheme "slowtest" which delivers a lot of data in small chunks with a pause in between,
 *  for as long as it is not stopped by the destination refusing data. Its resource is 100000 chunks of 1000 bytes.
 */
class CSlowTestAccessor : public CORE::CUriResourceAccessor
{
    public:

    static const CORE::CString SchemeName;

    virtual bool CreateResource( const CORE::CUri& uri, CORE::CIOAccess& src ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual bool GetResource( const CORE::CUri& uri, CORE::CIOAccess& destination ) GUCEF_VIRTUAL_OVERRIDE
    {
        char chunk[ 1000 ];
        memset( chunk, 'x', sizeof( chunk ) );
        for ( UInt32 i=0; i<100000; ++i )
        {
            if ( 1 != destination.Write( chunk, sizeof( chunk ), 1 ) )
                return false;
            MT::ThreadDelay( 2 );
        }
        return true;
    }

    virtual bool GetPartialResource( const CORE::CUri& uri, UInt64 byteOffset, Int64 bytesToGet, CORE::CIOAccess& destination ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool GetResourceAccess( const CORE::CUri& uri, CORE::IOAccessPtr& accessToResource, TURI_RESOURCEACCESS_MODE mode ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual bool GetResourceMetaData( const CORE::CUri& uri, CORE::CResourceMetaData& metaData ) GUCEF_VIRTUAL_OVERRIDE
    {
        metaData.hasResourceSizeInBytes = true;
        metaData.resourceSizeInBytes = 100000000;
        return true;
    }

    virtual bool UpdateResource( const CORE::CUri& uri, CORE::CIOAccess& src ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool UpdatePartialResource( const CORE::CUri& uri, UInt64 byteOffset, Int64 bytesToWrite, CORE::CIOAccess& src ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool UpdatePartialResource( const CORE::CUri& uri, CORE::CIOAccess& src ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool GetSupportedOperations( const CORE::CUri& uri, CUriResourceAccessorOperations& ops ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool DeleteResource( const CORE::CUri& uri ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool IsACollectionResource( const CORE::CUri& uri ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool ResolveResourcesInCollection( const CORE::CUri& uri, UriVector& resources, bool recursive, bool addCollectionUris ) GUCEF_VIRTUAL_OVERRIDE { return false; }
};

const CORE::CString CSlowTestAccessor::SchemeName = "slowtest";

typedef CORE::CTFactory< CORE::CUriResourceAccessor, CSlowTestAccessor, MT::CMutex > TSlowTestAccessorFactory;

/** Condition for PumpUntil(): the recorder saw progress of a retrieval */
class CHasProgress
{
    public:

    CHasProgress( const CRetrievalRecorder& recorder ) : m_recorder( recorder ) {}
    bool operator()( void ) const { return m_recorder.progress > 0; }

    private:

    const CRetrievalRecorder& m_recorder;
};

/*-------------------------------------------------------------------------*/

/** Condition for PumpUntil(): the recorder saw the end of a retrieval */
class CIsFinished
{
    public:

    CIsFinished( const CRetrievalRecorder& recorder ) : m_recorder( recorder ) {}
    bool operator()( void ) const { return m_recorder.IsFinished(); }

    private:

    const CRetrievalRecorder& m_recorder;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename Condition >
static bool
PumpUntil( Condition condition, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs; ++waited )
    {
        if ( condition() )
            return true;
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return condition();
}

/*-------------------------------------------------------------------------*/

static CORE::CString
FileUri( const CORE::CString& path )
{
    return "file://" + path;     // path is absolute on posix, the usual "file:///dir/file" form
}

/*-------------------------------------------------------------------------*/

void
PerformAsyncUriResourceRetrieverTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING async uri resource retriever TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "AsyncUriResourceRetriever" );

    TSlowTestAccessorFactory slowFactory;
    CORE::CCoreGlobal::Instance()->GetUriResourceAccessorFactory().RegisterConcreteFactory( CSlowTestAccessor::SchemeName, &slowFactory );

    GUCEF_TESTFW_TESTCASE( "Test: ProgressReportingIOAccess counts, reports per interval and aborts on request" )
        try
        {
            CORE::CDynamicBuffer sink;
            CORE::CDynamicBufferAccess sinkAccess( sink );
            CCountingObserver observer;

            {
                CORE::CProgressReportingIOAccess access( sinkAccess, &observer, 0 );
                char data[ 100 ];
                memset( data, 'a', sizeof( data ) );
                ASSERT_TRUE( 1 == access.Write( data, sizeof( data ), 1 ) );
                ASSERT_TRUE( 1 == access.Write( data, sizeof( data ), 1 ) );
                ASSERT_TRUE( 200 == access.GetTotalBytesWritten() );
                ASSERT_TRUE( 0 == access.GetTotalBytesRead() );
                ASSERT_TRUE( 2 == observer.reports );      // interval 0 reports every time
                ASSERT_TRUE( 200 == observer.lastWritten );
                ASSERT_TRUE( 200 == sink.GetDataSize() );

                // reading is counted too
                access.Seek( 0, SEEK_SET );
                char readBack[ 50 ];
                ASSERT_TRUE( 50 == access.Read( readBack, 1, 50 ) );
                ASSERT_TRUE( 50 == access.GetTotalBytesRead() && 50 == observer.lastRead );
            }

            // reports only every so many bytes
            CORE::CDynamicBuffer sink2;
            CORE::CDynamicBufferAccess sink2Access( sink2 );
            CCountingObserver observer2;
            CORE::CProgressReportingIOAccess spaced( sink2Access, &observer2, 1000 );
            char data[ 100 ];
            memset( data, 'b', sizeof( data ) );
            for ( UInt32 i=0; i<25; ++i )
                spaced.Write( data, sizeof( data ), 1 );
            ASSERT_TRUE( 2 == observer2.reports );          // at 1000 and 2000 bytes
            ASSERT_TRUE( 2500 == sink2.GetDataSize() );

            // the observer can abort, after which nothing goes through anymore
            CORE::CDynamicBuffer sink3;
            CORE::CDynamicBufferAccess sink3Access( sink3 );
            CCountingObserver observer3;
            observer3.abortAtWritten = 300;
            CORE::CProgressReportingIOAccess aborting( sink3Access, &observer3, 0 );
            ASSERT_TRUE( 1 == aborting.Write( data, sizeof( data ), 1 ) );
            ASSERT_TRUE( 1 == aborting.Write( data, sizeof( data ), 1 ) );
            ASSERT_FALSE( aborting.IsAborted() );
            ASSERT_TRUE( 1 == aborting.Write( data, sizeof( data ), 1 ) );     // this one reaches the limit
            ASSERT_TRUE( aborting.IsAborted() );
            ASSERT_TRUE( 0 == aborting.Write( data, sizeof( data ), 1 ) );
            ASSERT_TRUE( 300 == sink3.GetDataSize() );
            ASSERT_FALSE( aborting.IsWriteable() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A file uri without authority holds an absolute path and a uri with authority a relative one" )
        try
        {
            CORE::CUri absolute( "file:///dir/sub/file.txt" );
            ASSERT_TRUE( "file" == absolute.GetScheme() );
            ASSERT_FALSE( absolute.HasAuthority() );
            ASSERT_TRUE( "/dir/sub/file.txt" == absolute.GetAuthorityAndPath() );
            ASSERT_TRUE( absolute == CORE::CUri( absolute.ToUriInStringForm() ) );

            CORE::CUri relative( "file://dir/sub/file.txt" );
            ASSERT_TRUE( "dir/sub/file.txt" == relative.GetAuthorityAndPath() );

            CORE::CUri httpUri( "http://example.com/a/b?x=1#f" );
            ASSERT_TRUE( "example.com" == httpUri.GetAuthority() && "a/b" == httpUri.GetPath() && "x=1" == httpUri.GetQuery() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CString tempDir = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "asyncuritemp" );
    CORE::CreateDirs( tempDir );
    CORE::CString bigFile = CORE::CombinePath( tempDir, "big.bin" );

    GUCEF_TESTFW_TESTCASE( "Test: A file is retrieved into a buffer and into an access object with progress and completion" )
        try
        {
            // 1MiB file with a known pattern
            CORE::CDynamicBuffer content;
            content.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );    // filled byte by byte
            for ( UInt32 i=0; i<1024*1024; ++i )
            {
                UInt8 value = (UInt8) ( i % 251 );
                content.Append( &value, 1 );
            }
            {
                CORE::CFileAccess out( bigFile, "wb" );
                ASSERT_TRUE( out.IsValid() );
                ASSERT_TRUE( 1 == out.Write( content.GetConstBufferPtr(), content.GetDataSize(), 1 ) );
            }

            CORE::CAsyncUriResourceRetriever retriever;
            retriever.SetProgressReportIntervalInBytes( 16 * 1024 );
            retriever.SetQueryTotalSize( true );
            CRetrievalRecorder recorder( retriever );
            ASSERT_FALSE( retriever.IsBusy() );

            CORE::CDynamicBuffer received;
            CORE::CFutureResult future = retriever.Retrieve( CORE::CUri( FileUri( bigFile ) ), received );
            ASSERT_TRUE( future.HasAFuture() );
            ASSERT_TRUE( retriever.IsBusy() );

            // only one at a time
            CORE::CDynamicBuffer other;
            ASSERT_TRUE( retriever.Retrieve( CORE::CUri( FileUri( bigFile ) ), other ).HasNoFuture() );

            ASSERT_TRUE( PumpUntil( [&recorder]() { return recorder.IsFinished(); }, 10000 ) );
            ASSERT_TRUE( 1 == recorder.completed && 0 == recorder.failed && 0 == recorder.cancelled );
            ASSERT_TRUE( 1 == recorder.started );
            ASSERT_TRUE( recorder.progress > 1 );
            ASSERT_TRUE( 1024 * 1024 == recorder.lastBytes );
            ASSERT_TRUE( 1024 * 1024 == recorder.lastTotal );
            ASSERT_FALSE( retriever.IsBusy() );
            ASSERT_TRUE( received.GetDataSize() == content.GetDataSize() && 0 == memcmp( received.GetConstBufferPtr(), content.GetConstBufferPtr(), content.GetDataSize() ) );

            // and again into an access object, the retriever is reusable
            CRetrievalRecorder recorder2( retriever );
            CORE::CDynamicBuffer received2;
            CORE::CDynamicBufferAccess received2Access( received2 );
            CORE::CFutureResult future2 = retriever.Retrieve( CORE::CUri( FileUri( bigFile ) ), received2Access );
            ASSERT_TRUE( future2.HasAFuture() );
            ASSERT_TRUE( PumpUntil( [&recorder2]() { return recorder2.IsFinished(); }, 10000 ) );
            ASSERT_TRUE( 1 == recorder2.completed );
            ASSERT_TRUE( received2.GetDataSize() == content.GetDataSize() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A part of a file is retrieved" )
        try
        {
            CORE::CAsyncUriResourceRetriever retriever;
            CRetrievalRecorder recorder( retriever );
            CORE::CDynamicBuffer received;
            ASSERT_TRUE( retriever.GetPartial( CORE::CUri( FileUri( bigFile ) ), 1000, 500, received ).HasAFuture() );
            ASSERT_TRUE( PumpUntil( [&recorder]() { return recorder.IsFinished(); }, 10000 ) );
            ASSERT_TRUE( 1 == recorder.completed );
            ASSERT_TRUE( 500 == received.GetDataSize() );
            if ( 500 == received.GetDataSize() )
                ASSERT_TRUE( (UInt8) ( 1000 % 251 ) == *static_cast< const UInt8* >( received.GetConstBufferPtr() ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A data uri is retrieved and failures are reported" )
        try
        {
            CORE::CAsyncUriResourceRetriever retriever;
            CRetrievalRecorder recorder( retriever );
            CORE::CDynamicBuffer received;
            ASSERT_TRUE( retriever.Retrieve( CORE::CUri( "data:text/plain;base64,SGVsbG8gYXN5bmM=" ), received ).HasAFuture() );
            ASSERT_TRUE( PumpUntil( [&recorder]() { return recorder.IsFinished(); }, 10000 ) );
            ASSERT_TRUE( 1 == recorder.completed );
            ASSERT_TRUE( 11 == received.GetDataSize() && 0 == memcmp( received.GetConstBufferPtr(), "Hello async", 11 ) );

            // a file that is not there
            CRetrievalRecorder missingRecorder( retriever );
            CORE::CDynamicBuffer nothing;
            ASSERT_TRUE( retriever.Retrieve( CORE::CUri( FileUri( CORE::CombinePath( tempDir, "doesnotexist.bin" ) ) ), nothing ).HasAFuture() );
            ASSERT_TRUE( PumpUntil( [&missingRecorder]() { return missingRecorder.IsFinished(); }, 10000 ) );
            ASSERT_TRUE( 1 == missingRecorder.failed && 0 == missingRecorder.completed );
            ASSERT_FALSE( missingRecorder.lastError.IsNULLOrEmpty() );

            // a scheme nobody handles
            CRetrievalRecorder schemeRecorder( retriever );
            ASSERT_TRUE( retriever.Retrieve( CORE::CUri( "nosuchscheme://something/here" ), nothing ).HasAFuture() );
            ASSERT_TRUE( PumpUntil( [&schemeRecorder]() { return schemeRecorder.IsFinished(); }, 10000 ) );
            ASSERT_TRUE( 1 == schemeRecorder.failed );
            ASSERT_FALSE( retriever.IsBusy() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A busy retrieval is cancelled midway, which also frees the destination" )
        try
        {
            CORE::CAsyncUriResourceRetriever retriever;
            retriever.SetProgressReportIntervalInBytes( 4000 );
            CRetrievalRecorder recorder( retriever );
            CORE::CDynamicBuffer received;
            ASSERT_TRUE( retriever.Retrieve( CORE::CUri( "slowtest://anything" ), received ).HasAFuture() );
            ASSERT_TRUE( PumpUntil( [&recorder]() { return recorder.progress > 1; }, 10000 ) );
            ASSERT_FALSE( recorder.IsFinished() );

            retriever.Cancel( true );
            ASSERT_TRUE( PumpUntil( [&recorder]() { return recorder.IsFinished(); }, 10000 ) );
            ASSERT_TRUE( 1 == recorder.cancelled && 0 == recorder.completed && 0 == recorder.failed );
            ASSERT_FALSE( retriever.IsBusy() );

            // we stopped long before the end and the buffer does not change anymore
            UInt32 sizeAfterCancel = received.GetDataSize();
            ASSERT_TRUE( sizeAfterCancel < 100000000 / 10 );
            MT::ThreadDelay( 50 );
            ASSERT_TRUE( sizeAfterCancel == received.GetDataSize() );

            // and the retriever is good for another go
            CRetrievalRecorder recorder2( retriever );
            CORE::CDynamicBuffer received2;
            ASSERT_TRUE( retriever.Retrieve( CORE::CUri( "data:text/plain;base64,SGVsbG8gYXN5bmM=" ), received2 ).HasAFuture() );
            ASSERT_TRUE( PumpUntil( [&recorder2]() { return recorder2.IsFinished(); }, 10000 ) );
            ASSERT_TRUE( 1 == recorder2.completed );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The log and the error text of a retrieval hold no secrets of the uri" )
        try
        {
            CRetrieverLogCapture capture;
            CORE::CAsyncUriResourceRetriever retriever;
            CRetrievalRecorder recorder( retriever );
            CORE::CDynamicBuffer received;
            ASSERT_TRUE( retriever.Retrieve( CORE::CUri( "slowtest://anything" ), received ).HasAFuture() );
            ASSERT_TRUE( PumpUntil( CHasProgress( recorder ), 10000 ) );

            // refused since the retriever is busy
            CORE::CDynamicBuffer refused;
            ASSERT_FALSE( retriever.Retrieve( CORE::CUri( "slowtest://user:SECRETPASSWORD@host/p?token=SECRETTOKEN#SECRETFRAGMENT" ), refused ).HasAFuture() );
            retriever.Cancel( true );
            ASSERT_TRUE( PumpUntil( CIsFinished( recorder ), 10000 ) );

            // a retrieval which fails, the resource accessor gets the whole uri
            CRetrievalRecorder failRecorder( retriever );
            CORE::CDynamicBuffer nothing;
            ASSERT_TRUE( retriever.Retrieve( CORE::CUri( FileUri( CORE::CombinePath( tempDir, "doesnotexist.bin" ) ) + "?token=SECRETTOKEN" ), nothing ).HasAFuture() );
            ASSERT_TRUE( PumpUntil( CIsFinished( failRecorder ), 10000 ) );
            ASSERT_TRUE( 1 == failRecorder.failed );
            ASSERT_FALSE( failRecorder.lastError.IsNULLOrEmpty() );
            ASSERT_TRUE( failRecorder.lastError.HasSubstr( "SECRET", true ) < 0 );
            ASSERT_TRUE( failRecorder.lastError.HasSubstr( "doesnotexist.bin", true ) >= 0 );

            CORE::CString logged = capture.GetText();
            ASSERT_TRUE( logged.HasSubstr( "already busy", true ) >= 0 );
            ASSERT_TRUE( logged.HasSubstr( "SECRET", true ) < 0 );
            ASSERT_TRUE( logged.HasSubstr( "slowtest://***@host/p?token=***#***", true ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Destroying the retriever while it is busy stops the retrieval" )
        try
        {
            CORE::CDynamicBuffer received;
            {
                CORE::CAsyncUriResourceRetriever retriever;
                retriever.SetProgressReportIntervalInBytes( 4000 );
                CRetrievalRecorder recorder( retriever );
                ASSERT_TRUE( retriever.Retrieve( CORE::CUri( "slowtest://anything" ), received ).HasAFuture() );
                ASSERT_TRUE( PumpUntil( [&recorder]() { return recorder.progress > 0; }, 10000 ) );
            }
            // the retriever is gone and the destination is ours again
            UInt32 sizeAfterDestroy = received.GetDataSize();
            MT::ThreadDelay( 50 );
            ASSERT_TRUE( sizeAfterDestroy == received.GetDataSize() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CCoreGlobal::Instance()->GetUriResourceAccessorFactory().UnregisterConcreteFactory( CSlowTestAccessor::SchemeName );
    CORE::DeleteFile( bigFile );
    CORE::Remove_Directory( tempDir.C_String(), 1 );
}

/*-------------------------------------------------------------------------*/
