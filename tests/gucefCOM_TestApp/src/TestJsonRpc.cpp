/*
 *  gucefCOM_TestApp: GUCEF test application for the COM module
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
#include <deque>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestJsonRpc.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

static bool
EnsurePluginLoaded( const CORE::CString& pluginName )
{
    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        return true;
    return pluginControl.AddPluginFromDir( secondPluginPath, "test", true );
}

static bool
JsonAvailable( void )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        return true;
    EnsurePluginLoaded( "dstorepluginJSONPARSER" );
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false );
}

/** Parses JSON text for inspecting what an endpoint sent */
static bool
ParseText( const CORE::CString& text, CORE::CDataNode& root )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        return false;
    root.Clear();
    CORE::CDynamicBuffer buffer;
    buffer.CopyFrom( text.ByteSize() - 1, text.C_String() );
    CORE::CDynamicBufferAccess access( buffer );
    return codec->BuildDataTree( &root, &access );
}

/** The id of a message as text: "null" for a null id, "-" if there is none */
/** The number of direct children. CDataNode::GetNrOfChildNodes() counts all descendants */
static UInt32
ChildCount( const CORE::CDataNode& node )
{
    UInt32 count = 0;
    for ( CORE::CDataNode::const_iterator i = node.ConstBegin(); i != node.ConstEnd(); ++i )
        ++count;
    return count;
}

static CORE::CString
IdOf( const CORE::CDataNode& message )
{
    const CORE::CDataNode::TKeyValuePair* id = message.GetAttribute( "id" );
    if ( GUCEF_NULL != id )
        return ( (*id).second.IsNIL() || (*id).second.IsNULL() ) ? CORE::CString( "null" ) : (*id).second.AsString();
    if ( GUCEF_NULL != message.Find( "id" ) )
        return "null";
    return "-";
}

static Int32
ErrorCodeOf( const CORE::CDataNode& message )
{
    const CORE::CDataNode* error = message.Find( "error" );
    if ( GUCEF_NULL == error || GUCEF_NULL == error->GetAttribute( "code" ) )
        return 0;
    return error->GetAttribute( "code" )->second.AsInt32();
}

static bool
HasResultInt( const CORE::CDataNode& message, Int32 expected )
{
    const CORE::CDataNode::TKeyValuePair* result = message.GetAttribute( "result" );
    return GUCEF_NULL != result && (*result).second.AsInt32() == expected;
}

/*-------------------------------------------------------------------------*/

/** Remembers what was sent */
class CCaptureTransport : public COM::CIJsonRpcTransport
{
    public:
    virtual bool SendMessageText( const CORE::CString& messageText ) GUCEF_VIRTUAL_OVERRIDE
    {
        sent.push_back( messageText );
        return true;
    }
    GUCEF::vector< CORE::CString > sent;
};

/**
 *  Connects two endpoints. Messages are delivered right away from within the send call, or when
 *  Flush() is called if 'queued' is set
 */
class CMemoryTransport : public COM::CIJsonRpcTransport
{
    public:
    CMemoryTransport( void ) : peer( GUCEF_NULL ), queued( false ), connected( true ), messagesSent( 0 ) {}

    virtual bool SendMessageText( const CORE::CString& messageText ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !connected || GUCEF_NULL == peer )
            return false;
        ++messagesSent;
        if ( queued )
            pending.push_back( messageText );
        else
            peer->ProcessIncoming( messageText );
        return true;
    }

    void Flush( void )
    {
        while ( !pending.empty() )
        {
            CORE::CString message = pending.front();
            pending.pop_front();
            peer->ProcessIncoming( message );
        }
    }

    COM::CJsonRpcEndpoint* peer;
    bool queued;
    bool connected;
    UInt32 messagesSent;
    std::deque< CORE::CString > pending;
};

/** Answers the methods used in the examples of the specification */
class CSpecMethods : public COM::CIJsonRpcRequestHandler
{
    public:
    CSpecMethods( void ) : deferredCalls( 0 ), throwOnNext( false ) {}

    virtual COM::EJsonRpcRequestResult OnJsonRpcRequest( COM::CJsonRpcEndpoint& endpoint     ,
                                                         const COM::SJsonRpcRequest& request ,
                                                         COM::SJsonRpcResponse& response     ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( throwOnNext )
            throw std::exception();

        if ( request.method == "subtract" && request.hasParams )
        {
            Int32 minuend = 0;
            Int32 subtrahend = 0;
            if ( GUCEF_DATATYPE_ARRAY == request.params.GetNodeType() )
            {
                CORE::CDataNode::const_iterator i = request.params.ConstBegin();
                minuend = (*i)->GetValue().AsInt32();
                ++i;
                subtrahend = (*i)->GetValue().AsInt32();
            }
            else
            {
                minuend = request.params.GetAttributeValue( "minuend" ).AsInt32();
                subtrahend = request.params.GetAttributeValue( "subtrahend" ).AsInt32();
            }
            response.result.SetValue( CORE::CVariant( (Int32) ( minuend - subtrahend ) ) );
            response.result.SetNodeType( response.result.GetValue().GetTypeId() );
            return COM::JSONRPC_REQUEST_HANDLED;
        }
        if ( request.method == "sum" && request.hasParams )
        {
            Int32 total = 0;
            for ( CORE::CDataNode::const_iterator i = request.params.ConstBegin(); i != request.params.ConstEnd(); ++i )
                total += (*i)->GetValue().AsInt32();
            response.result.SetValue( CORE::CVariant( total ) );
            response.result.SetNodeType( response.result.GetValue().GetTypeId() );
            return COM::JSONRPC_REQUEST_HANDLED;
        }
        if ( request.method == "get_data" )
        {
            response.result.SetNodeType( GUCEF_DATATYPE_ARRAY );
            response.result.AddValueAsChild( CORE::CVariant( CORE::CString( "hello" ) ) );
            response.result.AddValueAsChild( CORE::CVariant( (Int32) 5 ) );
            return COM::JSONRPC_REQUEST_HANDLED;
        }
        if ( request.method == "echo" )
        {
            response.result = request.params;
            return COM::JSONRPC_REQUEST_HANDLED;
        }
        if ( request.method == "later" )
        {
            ++deferredCalls;
            deferredId = request.id;
            return COM::JSONRPC_REQUEST_DEFERRED;
        }
        if ( request.method == "silent" )
        {
            return COM::JSONRPC_REQUEST_DEFERRED;   // and never answered
        }
        if ( request.method == "fail" )
        {
            response.isError = true;
            response.error.code = -32000;
            response.error.message = "Server error";
            response.error.hasData = true;
            response.error.data.SetNodeType( GUCEF_DATATYPE_OBJECT );
            response.error.data.SetAttribute( "reason", "because" );
            return COM::JSONRPC_REQUEST_HANDLED;
        }

        response.isError = true;
        response.error.code = COM::JSONRPC_ERROR_METHOD_NOT_FOUND;
        response.error.message = "Method not found";
        return COM::JSONRPC_REQUEST_HANDLED;
    }

    UInt32 deferredCalls;
    CORE::CVariant deferredId;
    bool throwOnNext;
};

/** Collects the events of an endpoint */
class CRpcRecorder : public CORE::CObserver
{
    public:
    CRpcRecorder( void ) : responses( 0 ), notifications( 0 ) {}

    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata        ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventid == COM::CJsonRpcEndpoint::ResponseEvent )
        {
            responseList.push_back( static_cast< COM::TJsonRpcResponseEventData* >( eventdata )->GetData() );
            ++responses;
        }
        else
        if ( eventid == COM::CJsonRpcEndpoint::NotificationEvent )
        {
            notificationList.push_back( static_cast< COM::TJsonRpcNotificationEventData* >( eventdata )->GetData() );
            ++notifications;
        }
    }

    UInt32 responses;
    UInt32 notifications;
    GUCEF::vector< COM::SJsonRpcResponse > responseList;
    GUCEF::vector< COM::SJsonRpcNotification > notificationList;
};


/** Deletes an endpoint from within a pulse of a pulse generator, as a transport owner that drops a failed connection does */
class CEndpointDeletingPulseHandler : public CORE::CObserver
{
    public:

    CEndpointDeletingPulseHandler( CORE::PulseGeneratorPtr pulseGenerator, COM::CJsonRpcEndpoint* endpointToDelete, UInt32 deleteOnPulse )
        : CORE::CObserver()
        , endpoint( endpointToDelete )
        , pulses( 0 )
        , m_deleteOnPulse( deleteOnPulse )
    {
        TEventCallback callback( this, &CEndpointDeletingPulseHandler::OnPulse );
        SubscribeTo( pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, callback );
    }

    COM::CJsonRpcEndpoint* endpoint;
    UInt32 pulses;

    private:

    typedef CORE::CTEventHandlerFunctor< CEndpointDeletingPulseHandler > TEventCallback;

    void OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++pulses;
        if ( pulses == m_deleteOnPulse && GUCEF_NULL != endpoint )
        {
            COM::CJsonRpcEndpoint* doomed = endpoint;
            endpoint = GUCEF_NULL;
            delete doomed;
        }
    }

    UInt32 m_deleteOnPulse;
};

/** Counts the calls it gets and answers each with the number 1 */
class CCountingMethods : public COM::CIJsonRpcRequestHandler
{
    public:
    CCountingMethods( void ) : calls( 0 ) {}

    virtual COM::EJsonRpcRequestResult OnJsonRpcRequest( COM::CJsonRpcEndpoint& endpoint     ,
                                                         const COM::SJsonRpcRequest& request ,
                                                         COM::SJsonRpcResponse& response     ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++calls;
        if ( request.method == "later" )
            return COM::JSONRPC_REQUEST_DEFERRED;
        response.result.SetValue( CORE::CVariant( (Int32) 1 ) );
        response.result.SetNodeType( response.result.GetValue().GetTypeId() );
        return COM::JSONRPC_REQUEST_HANDLED;
    }

    UInt32 calls;
};

/** Makes the C library use a decimal comma while such a locale is installed, restores the locale at the end of the scope */
class CCommaLocaleScope
{
    public:

    CCommaLocaleScope( void )
        : m_active( false )
        , m_previous()
    {
        const char* previous = setlocale( LC_NUMERIC, GUCEF_NULL );
        m_previous = GUCEF_NULL != previous ? previous : "C";

        const char* candidates[] = { "de_DE.UTF-8", "de_DE.utf8", "de_DE", "fr_FR.UTF-8", "fr_FR.utf8", "nl_NL.UTF-8", "nl_NL.utf8", "en_DK.UTF-8", "en_DK.utf8", "da_DK.UTF-8", "da_DK.utf8" };
        for ( size_t i=0; i<sizeof( candidates ) / sizeof( candidates[ 0 ] ) && !m_active; ++i )
        {
            if ( GUCEF_NULL != setlocale( LC_NUMERIC, candidates[ i ] ) )
            {
                const struct lconv* conv = localeconv();
                m_active = GUCEF_NULL != conv && GUCEF_NULL != conv->decimal_point && ',' == conv->decimal_point[ 0 ];
            }
        }
        if ( !m_active )
            setlocale( LC_NUMERIC, m_previous.C_String() );
    }

    ~CCommaLocaleScope()
    {
        setlocale( LC_NUMERIC, m_previous.C_String() );
    }

    bool IsActive( void ) const { return m_active; }

    private:

    bool m_active;
    CORE::CString m_previous;
};

static bool
BytesEqual( const CORE::CString& text, const char* expected )
{
    return text.ByteSize() == strlen( expected ) + 1 && 0 == memcmp( text.C_String(), expected, strlen( expected ) );
}

/** The JSON text of an object with one string member: the string value is written as is, the quotes added */
static CORE::CString
WriteStringMember( const char* value )
{
    CORE::CDataNode object( GUCEF_DATATYPE_OBJECT );
    object.SetAttribute( "s", CORE::CString( value ) );
    CORE::CString text;
    COM::CJsonRpcEndpoint::ToJsonText( object, text );
    return text;
}

static bool
PumpUntilCount( const UInt32& counter, UInt32 expected, UInt32 timeoutInMs = 5000 )
{
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    UInt64 startTicks = MT::PrecisionTickCount();
    Float64 ticksPerMs = MT::PrecisionTimerResolution() / 1000.0;
    while ( counter < expected )
    {
        if ( ( MT::PrecisionTickCount() - startTicks ) / ticksPerMs > timeoutInMs )
            return false;
        pulseGenerator->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Counts the pulses of a pulse generator, apart for the thread that owns it and for any other thread.
 *  Can also deliver the queued messages of a transport on every pulse, so they only arrive when that pulse generator is pumped.
 */
class CPulseCounter : public CORE::CObserver
{
    public:

    CPulseCounter( CORE::PulseGeneratorPtr pulseGenerator, UInt32 ownerThreadId )
        : CORE::CObserver()
        , flushOnPulse( GUCEF_NULL )
        , m_ownerThreadId( ownerThreadId )
        , m_ownerPulses( 0 )
        , m_otherPulses( 0 )
        , m_lock()
    {
        TEventCallback callback( this, &CPulseCounter::OnPulse );
        SubscribeTo( pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, callback );
    }

    UInt32 GetOwnerPulses( void ) const { MT::CScopeMutex lock( m_lock ); return m_ownerPulses; }
    UInt32 GetOtherPulses( void ) const { MT::CScopeMutex lock( m_lock ); return m_otherPulses; }

    CMemoryTransport* flushOnPulse;

    private:

    typedef CORE::CTEventHandlerFunctor< CPulseCounter > TEventCallback;

    void OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        {
            MT::CScopeMutex lock( m_lock );
            if ( MT::GetCurrentTaskID() == m_ownerThreadId )
                ++m_ownerPulses;
            else
                ++m_otherPulses;
        }
        if ( GUCEF_NULL != flushOnPulse )
            flushOnPulse->Flush();
    }

    UInt32 m_ownerThreadId;
    UInt32 m_ownerPulses;
    UInt32 m_otherPulses;
    MT::CMutex m_lock;
};

/** Keeps the text of every log message that is logged while it is attached to the log manager */
class CRpcLogCapture : public CORE::CILogger
{
    public:

    CRpcLogCapture( void )
        : CORE::CILogger()
        , m_text()
        , m_lock()
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( this );
    }

    virtual ~CRpcLogCapture()
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

/** Makes one CallAndWait() of "sum" on a thread of its own */
class CCallAndWaitThread : public MT::CActiveObject
{
    public:

    CCallAndWaitThread( COM::CJsonRpcEndpoint& endpoint )
        : MT::CActiveObject()
        , result( false )
        , response()
        , threadId( 0 )
        , m_endpoint( endpoint )
        , m_done( false )
    {}

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        return true;
    }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        threadId = MT::GetCurrentTaskID();
        CORE::CDataNode params( GUCEF_DATATYPE_ARRAY );
        params.AddValueAsChild( CORE::CVariant( (Int32) 40 ) );
        params.AddValueAsChild( CORE::CVariant( (Int32) 2 ) );
        result = m_endpoint.CallAndWait( "sum", &params, response, 500 );
        m_done.Store( true, MT::MEMORDER_RELEASE );
        return true; // stop cycling
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    /** Runs the call and waits until the thread is gone. The results may only be read after this returned true */
    bool Run( void )
    {
        if ( !Activate( GUCEF_NULL, 0, 0 ) )
            return false;
        UInt32 waitedInMs = 0;
        while ( !m_done.Load( MT::MEMORDER_ACQUIRE ) && waitedInMs < 10000 )
        {
            MT::PrecisionDelay( 1 );
            ++waitedInMs;
        }
        while ( m_done.Load( MT::MEMORDER_ACQUIRE ) && IsActive() && waitedInMs < 15000 )
        {
            MT::PrecisionDelay( 1 );
            ++waitedInMs;
        }
        return m_done.Load( MT::MEMORDER_ACQUIRE ) && !IsActive();
    }

    bool result;
    COM::SJsonRpcResponse response;
    UInt32 threadId;

    private:

    COM::CJsonRpcEndpoint& m_endpoint;
    MT::CTAtomic< bool > m_done;
};

/*-------------------------------------------------------------------------*/

void
PerformJsonRpcTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING JSON-RPC TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "JsonRpc" );

    if ( !JsonAvailable() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "JSON-RPC TESTS skipped: the json codec is not available" );
        return;
    }

    // The examples of section 7 of the JSON-RPC 2.0 specification: what is sent in and what must come back
    GUCEF_TESTFW_TESTCASE( "Test: Spec examples: calls with positional and named parameters" )
        try
        {
            CCaptureTransport transport;
            CSpecMethods methods;
            COM::CJsonRpcEndpoint endpoint( &transport );
            endpoint.SetRequestHandler( &methods );
            CORE::CDataNode response;

            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": [42, 23], \"id\": 1}" ) );
            ASSERT_TRUE( 1 == transport.sent.size() );
            ASSERT_TRUE( ParseText( transport.sent[ 0 ], response ) );
            ASSERT_TRUE( response.GetAttributeValue( "jsonrpc" ).AsString() == "2.0" );
            ASSERT_TRUE( HasResultInt( response, 19 ) && IdOf( response ) == "1" );

            response = CORE::CDataNode();
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": [23, 42], \"id\": 2}" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 1 ], response ) );
            ASSERT_TRUE( HasResultInt( response, -19 ) && IdOf( response ) == "2" );

            response = CORE::CDataNode();
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": {\"subtrahend\": 23, \"minuend\": 42}, \"id\": 3}" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 2 ], response ) );
            ASSERT_TRUE( HasResultInt( response, 19 ) && IdOf( response ) == "3" );

            response = CORE::CDataNode();
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": {\"minuend\": 42, \"subtrahend\": 23}, \"id\": 4}" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 3 ], response ) );
            ASSERT_TRUE( HasResultInt( response, 19 ) && IdOf( response ) == "4" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Spec examples: notifications get no response" )
        try
        {
            CCaptureTransport transport;
            CSpecMethods methods;
            COM::CJsonRpcEndpoint endpoint( &transport );
            endpoint.SetRequestHandler( &methods );
            CRpcRecorder recorder;
            recorder.SubscribeTo( &endpoint );

            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"update\", \"params\": [1,2,3,4,5]}" ) );
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"foobar\"}" ) );
            ASSERT_TRUE( transport.sent.empty() );
            ASSERT_TRUE( 2 == recorder.notifications );
            if ( 2 == recorder.notifications )
            {
                ASSERT_TRUE( recorder.notificationList[ 0 ].method == "update" );
                ASSERT_TRUE( recorder.notificationList[ 0 ].hasParams && 5 == ChildCount( recorder.notificationList[ 0 ].params ) );
                ASSERT_TRUE( recorder.notificationList[ 1 ].method == "foobar" );
                ASSERT_FALSE( recorder.notificationList[ 1 ].hasParams );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Spec examples: error responses" )
        try
        {
            CCaptureTransport transport;
            COM::CJsonRpcEndpoint endpoint( &transport );
            CSpecMethods methods;
            endpoint.SetRequestHandler( &methods );
            CORE::CDataNode response;

            // non-existent method: the id is echoed with its string type
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"foobar\", \"id\": \"1\"}" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 0 ], response ) );
            ASSERT_TRUE( -32601 == ErrorCodeOf( response ) && IdOf( response ) == "1" );

            // invalid json
            response = CORE::CDataNode();
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_INVALID == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"foobar, \"params\": \"bar\", \"baz]" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 1 ], response ) );
            ASSERT_TRUE( -32700 == ErrorCodeOf( response ) && IdOf( response ) == "null" );

            // invalid request object
            response = CORE::CDataNode();
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_INVALID == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": 1, \"params\": \"bar\"}" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 2 ], response ) );
            ASSERT_TRUE( -32600 == ErrorCodeOf( response ) && IdOf( response ) == "null" );

            // params which are not structured
            response = CORE::CDataNode();
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_INVALID == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"sum\", \"params\": \"bar\", \"id\": 8}" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 3 ], response ) );
            ASSERT_TRUE( -32602 == ErrorCodeOf( response ) && IdOf( response ) == "8" );

            // an error with data
            response = CORE::CDataNode();
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"fail\", \"id\": 9}" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 4 ], response ) );
            ASSERT_TRUE( -32000 == ErrorCodeOf( response ) );
            const CORE::CDataNode* error = response.Find( "error" );
            ASSERT_TRUE( GUCEF_NULL != error && error->GetAttributeValue( "message" ).AsString() == "Server error" );
            ASSERT_TRUE( GUCEF_NULL != error && GUCEF_NULL != error->Find( "data" ) && error->Find( "data" )->GetAttributeValue( "reason" ).AsString() == "because" );

            // a handler that throws becomes an internal error instead of taking the endpoint down
            methods.throwOnNext = true;
            response = CORE::CDataNode();
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"sum\", \"params\": [1], \"id\": 10}" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 5 ], response ) );
            ASSERT_TRUE( -32603 == ErrorCodeOf( response ) && IdOf( response ) == "10" );

            // a call when nobody handles calls
            COM::CJsonRpcEndpoint noHandler( &transport );
            response = CORE::CDataNode();
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == noHandler.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"sum\", \"id\": 11}" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 6 ], response ) );
            ASSERT_TRUE( -32601 == ErrorCodeOf( response ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Spec examples: batches" )
        try
        {
            CCaptureTransport transport;
            CSpecMethods methods;
            COM::CJsonRpcEndpoint endpoint( &transport );
            endpoint.SetRequestHandler( &methods );
            CORE::CDataNode response;

            // invalid json in a batch
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_INVALID == endpoint.ProcessIncoming( "[ {\"jsonrpc\": \"2.0\", \"method\": \"sum\", \"params\": [1,2,4], \"id\": \"1\"}, {\"jsonrpc\": \"2.0\", \"method\" ]" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 0 ], response ) );
            ASSERT_TRUE( -32700 == ErrorCodeOf( response ) );

            // empty batch
            response = CORE::CDataNode();
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_INVALID == endpoint.ProcessIncoming( "[]" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 1 ], response ) );
            ASSERT_TRUE( -32600 == ErrorCodeOf( response ) && IdOf( response ) == "null" );

            // batch with an invalid entry gives a batch response with an error for it
            response = CORE::CDataNode();
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_INVALID == endpoint.ProcessIncoming( "[1]" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 2 ], response ) );
            ASSERT_TRUE( GUCEF_DATATYPE_ARRAY == response.GetNodeType() && 1 == ChildCount( response ) );
            ASSERT_TRUE( -32600 == ErrorCodeOf( **response.ConstBegin() ) );

            // batch of invalid entries
            response = CORE::CDataNode();
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_INVALID == endpoint.ProcessIncoming( "[1,2,3]" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 3 ], response ) );
            ASSERT_TRUE( 3 == ChildCount( response ) );

            // the big example: calls, a notification, an invalid entry and a missing method
            response = CORE::CDataNode();
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_INVALID == endpoint.ProcessIncoming(
                "[ {\"jsonrpc\": \"2.0\", \"method\": \"sum\", \"params\": [1,2,4], \"id\": \"1\"},"
                "  {\"jsonrpc\": \"2.0\", \"method\": \"notify_hello\", \"params\": [7]},"
                "  {\"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": [42,23], \"id\": \"2\"},"
                "  {\"foo\": \"boo\"},"
                "  {\"jsonrpc\": \"2.0\", \"method\": \"foo.get\", \"params\": {\"name\": \"myself\"}, \"id\": \"5\"},"
                "  {\"jsonrpc\": \"2.0\", \"method\": \"get_data\", \"id\": \"9\"} ]" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 4 ], response ) );
            ASSERT_TRUE( 5 == ChildCount( response ) );
            if ( 5 == ChildCount( response ) )
            {
                CORE::CDataNode::const_iterator i = response.ConstBegin();
                const CORE::CDataNode& sum = **i;
                ++i;
                const CORE::CDataNode& subtract = **i;
                ++i;
                const CORE::CDataNode& invalid = **i;
                ++i;
                const CORE::CDataNode& missing = **i;
                ++i;
                const CORE::CDataNode& data = **i;
                ASSERT_TRUE( IdOf( sum ) == "1" && HasResultInt( sum, 7 ) );
                ASSERT_TRUE( IdOf( subtract ) == "2" && HasResultInt( subtract, 19 ) );
                ASSERT_TRUE( IdOf( invalid ) == "null" && -32600 == ErrorCodeOf( invalid ) );
                ASSERT_TRUE( IdOf( missing ) == "5" && -32601 == ErrorCodeOf( missing ) );
                const CORE::CDataNode* resultArray = data.Find( "result" );
                ASSERT_TRUE( IdOf( data ) == "9" && GUCEF_NULL != resultArray && GUCEF_DATATYPE_ARRAY == resultArray->GetNodeType() && 2 == ChildCount( *resultArray ) );
            }

            // only notifications: nothing comes back at all
            size_t sentBefore = transport.sent.size();
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "[ {\"jsonrpc\": \"2.0\", \"method\": \"notify_sum\", \"params\": [1,2,4]}, {\"jsonrpc\": \"2.0\", \"method\": \"notify_hello\", \"params\": [7]} ]" ) );
            ASSERT_TRUE( sentBefore == transport.sent.size() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Calls between two endpoints" )
        try
        {
            CMemoryTransport aToB;
            CMemoryTransport bToA;
            CSpecMethods methods;
            COM::CJsonRpcEndpoint a( &aToB );
            COM::CJsonRpcEndpoint b( &bToA );
            aToB.peer = &b;
            bToA.peer = &a;
            b.SetRequestHandler( &methods );
            CRpcRecorder recorder;
            recorder.SubscribeTo( &a );

            // parameters, result and the id which ties them together
            CORE::CDataNode params( GUCEF_DATATYPE_ARRAY );
            params.AddValueAsChild( CORE::CVariant( (Int32) 5 ) );
            params.AddValueAsChild( CORE::CVariant( (Int32) 6 ) );
            UInt64 firstId = 0;
            UInt64 secondId = 0;
            ASSERT_TRUE( a.Call( "sum", &params, firstId ) );
            ASSERT_TRUE( a.Call( "sum", &params, secondId ) );
            ASSERT_TRUE( firstId != secondId && firstId > 0 );
            ASSERT_TRUE( 2 == recorder.responses );
            ASSERT_TRUE( 0 == a.GetPendingCallCount() );
            if ( 2 == recorder.responses )
            {
                ASSERT_TRUE( recorder.responseList[ 0 ].id.AsUInt64() == firstId && recorder.responseList[ 1 ].id.AsUInt64() == secondId );
                ASSERT_FALSE( recorder.responseList[ 0 ].isError );
                ASSERT_TRUE( 11 == recorder.responseList[ 0 ].result.GetValue().AsInt32() );
            }

            // errors come back with their code, message and data
            UInt64 failId = 0;
            ASSERT_TRUE( a.Call( "fail", GUCEF_NULL, failId ) );
            ASSERT_TRUE( 3 == recorder.responses );
            if ( 3 == recorder.responses )
            {
                const COM::SJsonRpcResponse& failed = recorder.responseList[ 2 ];
                ASSERT_TRUE( failed.isError && !failed.isLocalError );
                ASSERT_TRUE( -32000 == failed.error.code && failed.error.message == "Server error" );
                ASSERT_TRUE( failed.error.hasData && failed.error.data.GetAttributeValue( "reason" ).AsString() == "because" );
            }

            // a method nobody knows
            UInt64 unknownId = 0;
            ASSERT_TRUE( a.Call( "nonsense", GUCEF_NULL, unknownId ) );
            ASSERT_TRUE( 4 == recorder.responses && -32601 == recorder.responseList[ 3 ].error.code );

            // structured results survive
            UInt64 dataId = 0;
            ASSERT_TRUE( a.Call( "get_data", GUCEF_NULL, dataId ) );
            ASSERT_TRUE( 5 == recorder.responses );
            if ( 5 == recorder.responses )
            {
                const CORE::CDataNode& result = recorder.responseList[ 4 ].result;
                ASSERT_TRUE( GUCEF_DATATYPE_ARRAY == result.GetNodeType() && 2 == ChildCount( result ) );
                ASSERT_TRUE( (*result.ConstBegin())->GetValue().AsString() == "hello" );
            }

            // invalid parameters are refused locally
            CORE::CDataNode scalar;
            scalar.SetValue( CORE::CVariant( (Int32) 1 ) );
            scalar.SetNodeType( scalar.GetValue().GetTypeId() );
            UInt64 badId = 0;
            ASSERT_FALSE( a.Call( "sum", &scalar, badId ) );
            ASSERT_FALSE( a.Call( "", GUCEF_NULL, badId ) );
            ASSERT_TRUE( 0 == a.GetPendingCallCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Both sides can call each other and send notifications" )
        try
        {
            CMemoryTransport aToB;
            CMemoryTransport bToA;
            CSpecMethods methodsOfA;
            CSpecMethods methodsOfB;
            COM::CJsonRpcEndpoint a( &aToB );
            COM::CJsonRpcEndpoint b( &bToA );
            aToB.peer = &b;
            bToA.peer = &a;
            a.SetRequestHandler( &methodsOfA );
            b.SetRequestHandler( &methodsOfB );
            CRpcRecorder recorderA;
            CRpcRecorder recorderB;
            recorderA.SubscribeTo( &a );
            recorderB.SubscribeTo( &b );

            UInt64 idA = 0;
            UInt64 idB = 0;
            ASSERT_TRUE( a.Call( "get_data", GUCEF_NULL, idA ) );
            ASSERT_TRUE( b.Call( "get_data", GUCEF_NULL, idB ) );
            ASSERT_TRUE( 1 == recorderA.responses && 1 == recorderB.responses );

            CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
            params.SetAttribute( "event", "tick" );
            ASSERT_TRUE( a.Notify( "news", &params ) );
            ASSERT_TRUE( 1 == recorderB.notifications );
            ASSERT_TRUE( 0 == recorderA.notifications );
            if ( 1 == recorderB.notifications )
            {
                ASSERT_TRUE( recorderB.notificationList[ 0 ].method == "news" );
                ASSERT_TRUE( recorderB.notificationList[ 0 ].params.GetAttributeValue( "event" ).AsString() == "tick" );
            }
            ASSERT_FALSE( a.Notify( "", GUCEF_NULL ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Answers arriving later, in other order or never" )
        try
        {
            CMemoryTransport aToB;
            CMemoryTransport bToA;
            CSpecMethods methods;
            COM::CJsonRpcEndpoint a( &aToB );
            COM::CJsonRpcEndpoint b( &bToA );
            aToB.peer = &b;
            bToA.peer = &a;
            b.SetRequestHandler( &methods );
            CRpcRecorder recorder;
            recorder.SubscribeTo( &a );

            // deferred: the answer is sent later through the endpoint
            UInt64 laterId = 0;
            ASSERT_TRUE( a.Call( "later", GUCEF_NULL, laterId ) );
            ASSERT_TRUE( 0 == recorder.responses && 1 == a.GetPendingCallCount() && 1 == methods.deferredCalls );
            CORE::CDataNode result;
            result.SetValue( CORE::CVariant( CORE::CString( "done" ) ) );
            result.SetNodeType( result.GetValue().GetTypeId() );
            ASSERT_TRUE( b.SendResponse( methods.deferredId, result ) );
            ASSERT_TRUE( 1 == recorder.responses && 0 == a.GetPendingCallCount() );
            ASSERT_TRUE( recorder.responseList[ 0 ].result.GetValue().AsString() == "done" );

            // a response for something that is not pending (late or unknown) is ignored
            ASSERT_TRUE( b.SendResponse( CORE::CVariant( (UInt64) 424242 ), result ) );
            ASSERT_TRUE( 1 == recorder.responses );

            // a queued transport delivers later: the call is pending until then
            aToB.queued = true;
            bToA.queued = true;
            UInt64 queuedId = 0;
            CORE::CDataNode params( GUCEF_DATATYPE_ARRAY );
            params.AddValueAsChild( CORE::CVariant( (Int32) 1 ) );
            ASSERT_TRUE( a.Call( "sum", &params, queuedId ) );
            ASSERT_TRUE( 1 == a.GetPendingCallCount() );
            aToB.Flush();
            ASSERT_TRUE( 1 == a.GetPendingCallCount() );
            bToA.Flush();
            ASSERT_TRUE( 0 == a.GetPendingCallCount() && 2 == recorder.responses );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Calls that are not answered time out and calls on a lost transport fail" )
        try
        {
            CMemoryTransport aToB;
            CMemoryTransport bToA;
            CSpecMethods methods;
            COM::CJsonRpcEndpoint a( &aToB );
            COM::CJsonRpcEndpoint b( &bToA );
            aToB.peer = &b;
            bToA.peer = &a;
            b.SetRequestHandler( &methods );
            CRpcRecorder recorder;
            recorder.SubscribeTo( &a );

            UInt64 silentId = 0;
            ASSERT_TRUE( a.Call( "silent", GUCEF_NULL, silentId, 50 ) );
            UInt64 longId = 0;
            ASSERT_TRUE( a.Call( "silent", GUCEF_NULL, longId, 600000 ) );
            ASSERT_TRUE( 2 == a.GetPendingCallCount() );

            UInt32 expired = 0;
            ASSERT_TRUE( a.ExpireTimedOutCalls( &expired ) );
            ASSERT_TRUE( 0 == expired );
            MT::ThreadDelay( 100 );
            ASSERT_TRUE( a.ExpireTimedOutCalls( &expired ) );
            ASSERT_TRUE( 1 == expired && 1 == recorder.responses && 1 == a.GetPendingCallCount() );
            if ( 1 == recorder.responses )
            {
                ASSERT_TRUE( recorder.responseList[ 0 ].isError && recorder.responseList[ 0 ].isLocalError );
                ASSERT_TRUE( COM::JSONRPC_ERROR_CALL_TIMEOUT == recorder.responseList[ 0 ].error.code );
                ASSERT_TRUE( recorder.responseList[ 0 ].id.AsUInt64() == silentId );
            }

            // the transport goes away
            ASSERT_TRUE( a.FailAllPendingCalls( COM::JSONRPC_ERROR_TRANSPORT_CLOSED, "gone" ) );
            ASSERT_TRUE( 2 == recorder.responses && 0 == a.GetPendingCallCount() );
            ASSERT_TRUE( COM::JSONRPC_ERROR_TRANSPORT_CLOSED == recorder.responseList[ 1 ].error.code );
            ASSERT_TRUE( recorder.responseList[ 1 ].id.AsUInt64() == longId );

            // a transport that cannot send fails the call right away and leaves nothing pending
            aToB.connected = false;
            UInt64 failedId = 0;
            ASSERT_FALSE( a.Call( "sum", GUCEF_NULL, failedId ) );
            ASSERT_TRUE( 0 == a.GetPendingCallCount() );

            // no transport at all
            COM::CJsonRpcEndpoint lonely;
            ASSERT_FALSE( lonely.Call( "sum", GUCEF_NULL, failedId ) );
            ASSERT_FALSE( lonely.Notify( "sum", GUCEF_NULL ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Batch calls" )
        try
        {
            CMemoryTransport aToB;
            CMemoryTransport bToA;
            CSpecMethods methods;
            COM::CJsonRpcEndpoint a( &aToB );
            COM::CJsonRpcEndpoint b( &bToA );
            aToB.peer = &b;
            bToA.peer = &a;
            b.SetRequestHandler( &methods );
            CRpcRecorder recorderA;
            CRpcRecorder recorderB;
            recorderA.SubscribeTo( &a );
            recorderB.SubscribeTo( &b );

            COM::CJsonRpcEndpoint::TBatchItemVector items( 3 );
            items[ 0 ].method = "sum";
            items[ 0 ].params = CORE::CDataNode( GUCEF_DATATYPE_ARRAY );
            items[ 0 ].params.AddValueAsChild( CORE::CVariant( (Int32) 20 ) );
            items[ 0 ].params.AddValueAsChild( CORE::CVariant( (Int32) 22 ) );
            items[ 0 ].hasParams = true;
            items[ 1 ].method = "note";
            items[ 1 ].isNotification = true;
            items[ 2 ].method = "get_data";
            ASSERT_TRUE( a.CallBatch( items ) );

            // one batch message, answered by one batch message, handled as individual responses
            ASSERT_TRUE( 1 == aToB.messagesSent && 1 == bToA.messagesSent );
            ASSERT_TRUE( 2 == recorderA.responses && 1 == recorderB.notifications );
            if ( 2 == recorderA.responses )
            {
                ASSERT_TRUE( recorderA.responseList[ 0 ].id.AsUInt64() == items[ 0 ].callId && 42 == recorderA.responseList[ 0 ].result.GetValue().AsInt32() );
                ASSERT_TRUE( recorderA.responseList[ 1 ].id.AsUInt64() == items[ 2 ].callId );
            }
            COM::CJsonRpcEndpoint::TBatchItemVector noItems;
            ASSERT_FALSE( a.CallBatch( noItems ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Batches are accepted by default and can be refused" )
        try
        {
            const char* batch = "[ {\"jsonrpc\": \"2.0\", \"method\": \"sum\", \"params\": [1,2], \"id\": 1} ]";
            CSpecMethods methods;

            CCaptureTransport allowedTransport;
            COM::CJsonRpcEndpoint allowed( &allowedTransport );
            allowed.SetRequestHandler( &methods );
            ASSERT_TRUE( allowed.GetAllowBatches() );
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == allowed.ProcessIncoming( batch ) );
            CORE::CDataNode response;
            ASSERT_TRUE( 1 == allowedTransport.sent.size() && ParseText( allowedTransport.sent[ 0 ], response ) );
            ASSERT_TRUE( GUCEF_DATATYPE_ARRAY == response.GetNodeType() && 1 == ChildCount( response ) );

            CCaptureTransport refusedTransport;
            COM::CJsonRpcEndpoint refused( &refusedTransport );
            refused.SetRequestHandler( &methods );
            refused.SetAllowBatches( false );
            ASSERT_FALSE( refused.GetAllowBatches() );
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_INVALID == refused.ProcessIncoming( batch ) );
            response = CORE::CDataNode();
            ASSERT_TRUE( 1 == refusedTransport.sent.size() && ParseText( refusedTransport.sent[ 0 ], response ) );
            ASSERT_TRUE( GUCEF_DATATYPE_ARRAY != response.GetNodeType() );
            ASSERT_TRUE( -32600 == ErrorCodeOf( response ) && IdOf( response ) == "null" );

            // a single message is unaffected
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == refused.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"sum\", \"params\": [1,2], \"id\": 2}" ) );
            ASSERT_TRUE( 2 == refusedTransport.sent.size() );
            response = CORE::CDataNode();
            ASSERT_TRUE( ParseText( refusedTransport.sent[ 1 ], response ) && HasResultInt( response, 3 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A null request id is accepted by default and can be refused" )
        try
        {
            const char* request = "{\"jsonrpc\": \"2.0\", \"method\": \"sum\", \"params\": [1,2], \"id\": null}";
            CSpecMethods methods;

            CCaptureTransport allowedTransport;
            COM::CJsonRpcEndpoint allowed( &allowedTransport );
            allowed.SetRequestHandler( &methods );
            ASSERT_TRUE( allowed.GetAllowNullRequestId() );
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == allowed.ProcessIncoming( request ) );
            CORE::CDataNode response;
            ASSERT_TRUE( 1 == allowedTransport.sent.size() && ParseText( allowedTransport.sent[ 0 ], response ) );
            ASSERT_TRUE( HasResultInt( response, 3 ) );

            CCaptureTransport refusedTransport;
            COM::CJsonRpcEndpoint refused( &refusedTransport );
            refused.SetRequestHandler( &methods );
            refused.SetAllowNullRequestId( false );
            ASSERT_FALSE( refused.GetAllowNullRequestId() );
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_INVALID == refused.ProcessIncoming( request ) );
            response = CORE::CDataNode();
            ASSERT_TRUE( 1 == refusedTransport.sent.size() && ParseText( refusedTransport.sent[ 0 ], response ) );
            ASSERT_TRUE( -32600 == ErrorCodeOf( response ) && IdOf( response ) == "null" );
            ASSERT_TRUE( !HasResultInt( response, 3 ) );

            // a real id and a notification are unaffected
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == refused.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"sum\", \"params\": [1,2], \"id\": 7}" ) );
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == refused.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"note\"}" ) );
            ASSERT_TRUE( 2 == refusedTransport.sent.size() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A batch is sent with its items in the order given" )
        try
        {
            CCaptureTransport transport;
            COM::CJsonRpcEndpoint endpoint( &transport );
            COM::CJsonRpcEndpoint::TBatchItemVector items( 4 );
            items[ 0 ].method = "first";
            items[ 1 ].method = "second";
            items[ 1 ].isNotification = true;
            items[ 2 ].method = "third";
            items[ 3 ].method = "fourth";
            ASSERT_TRUE( endpoint.CallBatch( items ) );
            ASSERT_TRUE( 1 == transport.sent.size() );
            CORE::CDataNode batch;
            ASSERT_TRUE( ParseText( transport.sent[ 0 ], batch ) && GUCEF_DATATYPE_ARRAY == batch.GetNodeType() && 4 == ChildCount( batch ) );
            const char* expected[] = { "first", "second", "third", "fourth" };
            UInt32 index = 0;
            for ( CORE::CDataNode::const_iterator i = batch.ConstBegin(); i != batch.ConstEnd() && index < 4; ++i, ++index )
                ASSERT_TRUE( (*i)->GetAttributeValue( "method" ).AsString() == expected[ index ] );

            // ids are assigned to the calls in order and not to the notification
            ASSERT_TRUE( items[ 0 ].callId < items[ 2 ].callId && items[ 2 ].callId < items[ 3 ].callId );
            ASSERT_TRUE( 0 == items[ 1 ].callId );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CallAndWait returns the answer" )
        try
        {
            CMemoryTransport aToB;
            CMemoryTransport bToA;
            CSpecMethods methods;
            COM::CJsonRpcEndpoint a( &aToB );
            COM::CJsonRpcEndpoint b( &bToA );
            aToB.peer = &b;
            bToA.peer = &a;
            b.SetRequestHandler( &methods );
            CRpcRecorder recorder;
            recorder.SubscribeTo( &a );

            CORE::CDataNode params( GUCEF_DATATYPE_ARRAY );
            params.AddValueAsChild( CORE::CVariant( (Int32) 40 ) );
            params.AddValueAsChild( CORE::CVariant( (Int32) 2 ) );
            COM::SJsonRpcResponse response;
            ASSERT_TRUE( a.CallAndWait( "sum", &params, response, 2000 ) );
            ASSERT_FALSE( response.isError );
            ASSERT_TRUE( 42 == response.result.GetValue().AsInt32() );
            ASSERT_TRUE( 0 == recorder.responses );       // not delivered as an event as it was returned

            // an error answer is still an answer
            ASSERT_TRUE( a.CallAndWait( "fail", GUCEF_NULL, response, 2000 ) );
            ASSERT_TRUE( response.isError && -32000 == response.error.code );

            // nobody answers: the wait ends with a timeout error
            ASSERT_TRUE( a.CallAndWait( "silent", GUCEF_NULL, response, 100 ) );
            ASSERT_TRUE( response.isError && response.isLocalError && COM::JSONRPC_ERROR_CALL_TIMEOUT == response.error.code );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CallAndWait pumps the pulse generator set on the endpoint" )
        try
        {
            const UInt32 thisThreadId = MT::GetCurrentTaskID();
            CORE::PulseGeneratorPtr globalPulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();

            CMemoryTransport aToB;
            CMemoryTransport bToA;
            CSpecMethods methods;
            COM::CJsonRpcEndpoint a( &aToB );
            COM::CJsonRpcEndpoint b( &bToA );
            aToB.peer = &b;
            bToA.peer = &a;
            b.SetRequestHandler( &methods );

            // by default the application's pulse generator drives the endpoint
            ASSERT_TRUE( a.GetPulseGenerator() == globalPulseGenerator );

            // a pulse generator of this thread, the only one that delivers the call to the other side
            CORE::PulseGeneratorPtr localPulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
            localPulseGenerator->SetPulseDriverThreadId( thisThreadId );
            a.SetPulseGenerator( localPulseGenerator );
            ASSERT_TRUE( a.GetPulseGenerator() == localPulseGenerator );
            aToB.queued = true;
            CPulseCounter localPulses( localPulseGenerator, thisThreadId );
            localPulses.flushOnPulse = &aToB;
            CPulseCounter globalPulses( globalPulseGenerator, thisThreadId );

            CORE::CDataNode params( GUCEF_DATATYPE_ARRAY );
            params.AddValueAsChild( CORE::CVariant( (Int32) 40 ) );
            params.AddValueAsChild( CORE::CVariant( (Int32) 2 ) );
            COM::SJsonRpcResponse response;
            ASSERT_TRUE( a.CallAndWait( "sum", &params, response, 2000 ) );
            ASSERT_FALSE( response.isError );
            ASSERT_TRUE( 42 == response.result.GetValue().AsInt32() );
            ASSERT_TRUE( 0 < localPulses.GetOwnerPulses() && 0 == localPulses.GetOtherPulses() );
            ASSERT_TRUE( 0 == globalPulses.GetOwnerPulses() && 0 == globalPulses.GetOtherPulses() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CallAndWait from a thread that does not drive the pulse generator is refused" )
        try
        {
            const UInt32 thisThreadId = MT::GetCurrentTaskID();
            CORE::PulseGeneratorPtr globalPulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();

            // this thread drives the application's pulse generator: its driver belongs to the thread that initialized the platform
            ASSERT_TRUE( globalPulseGenerator->GetPulseDriverThreadId() == thisThreadId );

            CMemoryTransport aToB;
            CMemoryTransport bToA;
            CSpecMethods methods;
            COM::CJsonRpcEndpoint a( &aToB );
            COM::CJsonRpcEndpoint b( &bToA );
            aToB.peer = &b;
            bToA.peer = &a;
            b.SetRequestHandler( &methods );

            // the endpoint uses the application's pulse generator, the call is made from another thread.
            // Calls are held by the transport so that waiting for the answer would have to pump.
            {
                aToB.queued = true;
                CPulseCounter globalPulses( globalPulseGenerator, thisThreadId );
                CRpcLogCapture logCapture;
                CCallAndWaitThread caller( a );
                ASSERT_TRUE( caller.Run() );
                ASSERT_TRUE( caller.threadId != thisThreadId );
                ASSERT_FALSE( caller.result );
                ASSERT_TRUE( 0 == aToB.messagesSent );                    // the call was not made
                ASSERT_TRUE( 0 == a.GetPendingCallCount() );
                ASSERT_TRUE( 0 == globalPulses.GetOtherPulses() );        // the other thread did not pump this thread's pulse generator
                ASSERT_TRUE( 0 <= logCapture.GetText().HasSubstr( "CallAndWait() refused for method \"sum\"" ) );
                aToB.queued = false;
            }

            // the same with a pulse generator of its own which this thread drives
            {
                CORE::PulseGeneratorPtr localPulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
                localPulseGenerator->SetPulseDriverThreadId( thisThreadId );
                a.SetPulseGenerator( localPulseGenerator );
                CPulseCounter localPulses( localPulseGenerator, thisThreadId );
                localPulses.flushOnPulse = &aToB;
                aToB.queued = true;
                CCallAndWaitThread caller( a );
                ASSERT_TRUE( caller.Run() );
                ASSERT_FALSE( caller.result );
                ASSERT_TRUE( 0 == aToB.messagesSent );
                ASSERT_TRUE( 0 == localPulses.GetOwnerPulses() && 0 == localPulses.GetOtherPulses() );

                // while from this thread the answer comes back as before
                COM::SJsonRpcResponse response;
                CORE::CDataNode params( GUCEF_DATATYPE_ARRAY );
                params.AddValueAsChild( CORE::CVariant( (Int32) 40 ) );
                params.AddValueAsChild( CORE::CVariant( (Int32) 2 ) );
                ASSERT_TRUE( a.CallAndWait( "sum", &params, response, 2000 ) );
                ASSERT_FALSE( response.isError );
                ASSERT_TRUE( 42 == response.result.GetValue().AsInt32() );
                ASSERT_TRUE( 1 == aToB.messagesSent );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CallAndWait is refused while it is not known which thread drives the pulse generator" )
        try
        {
            const UInt32 thisThreadId = MT::GetCurrentTaskID();

            CMemoryTransport aToB;
            CMemoryTransport bToA;
            CSpecMethods methods;
            COM::CJsonRpcEndpoint a( &aToB );
            COM::CJsonRpcEndpoint b( &bToA );
            aToB.peer = &b;
            bToA.peer = &a;
            b.SetRequestHandler( &methods );

            // a pulse generator without driver which was never pumped belongs to no thread yet
            CORE::PulseGeneratorPtr localPulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
            ASSERT_TRUE( 0 == localPulseGenerator->GetPulseDriverThreadId() );
            a.SetPulseGenerator( localPulseGenerator );
            CPulseCounter localPulses( localPulseGenerator, thisThreadId );
            CRpcLogCapture logCapture;
            COM::SJsonRpcResponse response;
            ASSERT_FALSE( a.CallAndWait( "silent", GUCEF_NULL, response, 100 ) );
            ASSERT_TRUE( 0 == aToB.messagesSent );
            ASSERT_TRUE( 0 == localPulses.GetOwnerPulses() && 0 == localPulses.GetOtherPulses() );
            ASSERT_TRUE( 0 == a.GetPendingCallCount() );

            // once this thread is known as its driver the call is made
            localPulseGenerator->SetPulseDriverThreadId( thisThreadId );
            ASSERT_TRUE( a.CallAndWait( "fail", GUCEF_NULL, response, 2000 ) );
            ASSERT_TRUE( response.isError && -32000 == response.error.code );

            // no pulse generator at all
            a.SetPulseGenerator( CORE::PulseGeneratorPtr() );
            ASSERT_FALSE( a.CallAndWait( "fail", GUCEF_NULL, response, 2000 ) );
            ASSERT_TRUE( 1 == aToB.messagesSent );
            CORE::CString logText = logCapture.GetText();
            ASSERT_TRUE( 0 <= logText.HasSubstr( "it is not known which thread drives the pulse generator" ) );
            ASSERT_TRUE( 0 <= logText.HasSubstr( "the endpoint has no pulse generator" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Lenient dialect and extra envelope fields" )
        try
        {
            CCaptureTransport transport;
            COM::CJsonRpcEndpoint endpoint( &transport );
            endpoint.SetDialect( COM::JSONRPC_DIALECT_LENIENT );
            endpoint.SetOutgoingCallField( "sessionId", CORE::CVariant( CORE::CString( "ABC123" ) ) );
            CRpcRecorder recorder;
            recorder.SubscribeTo( &endpoint );

            // what goes out has no version field but does have the extra field
            UInt64 callId = 0;
            CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
            params.SetAttribute( "url", "http://example.org/" );
            ASSERT_TRUE( endpoint.Call( "Page.navigate", &params, callId ) );
            ASSERT_TRUE( 1 == transport.sent.size() );
            CORE::CDataNode sent;
            ASSERT_TRUE( ParseText( transport.sent[ 0 ], sent ) );
            ASSERT_TRUE( GUCEF_NULL == sent.GetAttribute( "jsonrpc" ) );
            ASSERT_TRUE( sent.GetAttributeValue( "method" ).AsString() == "Page.navigate" );
            ASSERT_TRUE( sent.GetAttributeValue( "sessionId" ).AsString() == "ABC123" );
            ASSERT_TRUE( GUCEF_NULL != sent.Find( "params" ) );

            // a CDP style answer without version field is accepted and the extra fields are available
            CORE::CString answer = "{\"id\":" + CORE::UInt64ToString( callId ) + ",\"result\":{\"frameId\":\"F1\"},\"sessionId\":\"ABC123\"}";
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( answer ) );
            ASSERT_TRUE( 1 == recorder.responses );
            if ( 1 == recorder.responses )
            {
                ASSERT_TRUE( recorder.responseList[ 0 ].result.GetAttributeValue( "frameId" ).AsString() == "F1" );
                ASSERT_TRUE( recorder.responseList[ 0 ].envelope.GetAttributeValue( "sessionId" ).AsString() == "ABC123" );
            }

            // an event, which is a notification without version field
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"method\":\"Page.loadEventFired\",\"params\":{\"timestamp\":1.5},\"sessionId\":\"ABC123\"}" ) );
            ASSERT_TRUE( 1 == recorder.notifications );

            // fields can be removed again
            endpoint.SetOutgoingCallField( "", CORE::CVariant() );
            ASSERT_TRUE( endpoint.Notify( "x", GUCEF_NULL ) );
            sent = CORE::CDataNode();
            ASSERT_TRUE( ParseText( transport.sent[ 1 ], sent ) );
            ASSERT_TRUE( GUCEF_NULL == sent.GetAttribute( "sessionId" ) );

            // the strict dialect insists on the version field
            CCaptureTransport strictTransport;
            COM::CJsonRpcEndpoint strict( &strictTransport );
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_INVALID == strict.ProcessIncoming( "{\"method\":\"sum\",\"id\":1}" ) );
            CORE::CDataNode strictResponse;
            ASSERT_TRUE( 1 == strictTransport.sent.size() && ParseText( strictTransport.sent[ 0 ], strictResponse ) );
            ASSERT_TRUE( -32600 == ErrorCodeOf( strictResponse ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END


    GUCEF_TESTFW_TESTCASE( "Test: CallAndWait returns when a pulse handler deletes the endpoint while it waits" )
        try
        {
            const UInt32 thisThreadId = MT::GetCurrentTaskID();
            CORE::PulseGeneratorPtr localPulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
            localPulseGenerator->SetPulseDriverThreadId( thisThreadId );

            // the calls are held by the transport, so nothing answers and the wait goes on until the pulse handler deletes the endpoint
            CMemoryTransport aToB;
            aToB.queued = true;
            COM::CJsonRpcEndpoint* endpoint = new COM::CJsonRpcEndpoint( &aToB );
            endpoint->SetPulseGenerator( localPulseGenerator );
            aToB.peer = endpoint;
            CEndpointDeletingPulseHandler deleter( localPulseGenerator, endpoint, 3 );

            COM::SJsonRpcResponse response;
            const bool answered = endpoint->CallAndWait( "silent", GUCEF_NULL, response, 5000 );
            ASSERT_FALSE( answered );
            ASSERT_TRUE( 3 == deleter.pulses );
            ASSERT_TRUE( GUCEF_NULL == deleter.endpoint );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: numbers are written with a decimal point whatever the locale and doubles read back as written" )
        try
        {
            if ( !JsonAvailable() )
                return;

            // the text of a double is the same in any locale
            CORE::CDataNode numbers( GUCEF_DATATYPE_ARRAY );
            numbers.AddValueAsChild( CORE::CVariant( (Float64) 0.5 ) );
            numbers.AddValueAsChild( CORE::CVariant( (Float64) 2.25 ) );
            numbers.AddValueAsChild( CORE::CVariant( (Float64) -1234.5678 ) );
            numbers.AddValueAsChild( CORE::CVariant( (Float64) 1e22 ) );
            numbers.AddValueAsChild( CORE::CVariant( (Float64) 3.0 ) );
            CORE::CString text;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ToJsonText( numbers, text ) );
            ASSERT_TRUE( BytesEqual( text, "[0.5,2.25,-1234.5678,1e+22,3]" ) );

            // timestamps of both byte orders are written as the signed count they hold
            {
                CORE::CVariant bigEndianMs;
                CORE::CVariant littleEndianNanos;
                ASSERT_TRUE( bigEndianMs.SetFromString( GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH, "-5" ) );
                ASSERT_TRUE( littleEndianNanos.SetFromString( GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH, "1700000000123456789" ) );
                CORE::CDataNode timestamps( GUCEF_DATATYPE_ARRAY );
                timestamps.AddValueAsChild( bigEndianMs );
                timestamps.AddValueAsChild( littleEndianNanos );
                ASSERT_TRUE( COM::CJsonRpcEndpoint::ToJsonText( timestamps, text ) );
                ASSERT_TRUE( BytesEqual( text, "[-5,1700000000123456789]" ) );
            }

            // -0.0 is written as -0, which is valid JSON and reads back as zero
            CORE::CDataNode negativeZero( GUCEF_DATATYPE_ARRAY );
            negativeZero.AddValueAsChild( CORE::CVariant( (Float64) -0.0 ) );
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ToJsonText( negativeZero, text ) );
            ASSERT_TRUE( BytesEqual( text, "[-0]" ) );

            // NaN and infinity have no JSON form and are written as null
            CORE::CDataNode nonFinite( GUCEF_DATATYPE_ARRAY );
            nonFinite.AddValueAsChild( CORE::CVariant( (Float64) ( HUGE_VAL - HUGE_VAL ) ) );
            nonFinite.AddValueAsChild( CORE::CVariant( (Float64) HUGE_VAL ) );
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ToJsonText( nonFinite, text ) );
            ASSERT_TRUE( BytesEqual( text, "[null,null]" ) );

            {
                CCommaLocaleScope comma;
                if ( !comma.IsActive() )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "JsonRpc: no locale with a decimal comma is installed, the writer is tested without it" );
                }
                else
                {
                    char probe[ 32 ];
                    snprintf( probe, sizeof( probe ), "%g", 0.5 );
                    ASSERT_TRUE( 0 == strcmp( probe, "0,5" ) );      // the locale is in effect for the C library
                }
                ASSERT_TRUE( COM::CJsonRpcEndpoint::ToJsonText( numbers, text ) );
                ASSERT_TRUE( BytesEqual( text, "[0.5,2.25,-1234.5678,1e+22,3]" ) );
            }

            // random doubles: the written text read by the C library gives back the same double
            UInt64 state = 0x9E3779B97F4A7C15ULL;
            UInt32 checked = 0;
            for ( UInt32 i=0; i<3000; ++i )
            {
                state ^= state << 13; state ^= state >> 7; state ^= state << 17;
                UInt64 bits = state;
                Float64 value = 0.0;
                memcpy( &value, &bits, sizeof( value ) );
                if ( value != value || value - value != 0.0 )
                    continue;
                CORE::CDataNode array( GUCEF_DATATYPE_ARRAY );
                array.AddValueAsChild( CORE::CVariant( value ) );
                ASSERT_TRUE( COM::CJsonRpcEndpoint::ToJsonText( array, text ) );
                ASSERT_TRUE( text.ByteSize() > 3 );
                CORE::CString number = text.SubstrFromRange( 1, text.Length() - 1 );
                char* end = GUCEF_NULL;
                Float64 read = strtod( number.C_String(), &end );
                ASSERT_TRUE( GUCEF_NULL != end && '\0' == *end && read == value );
                ++checked;
            }
            ASSERT_TRUE( checked > 2900 );

            // through the parser of the json codec: values in the usual range come back within a rounding error of the last digit.
            // That parser builds a double from the digits itself so it is not exact for every double
            for ( UInt32 i=0; i<500; ++i )
            {
                state ^= state << 13; state ^= state >> 7; state ^= state << 17;
                Float64 value = ( (Float64) ( state >> 11 ) / 9007199254740992.0 - 0.5 ) * pow( 10.0, (Float64) ( (Int32) ( state % 20 ) - 6 ) );
                CORE::CDataNode array( GUCEF_DATATYPE_ARRAY );
                array.AddValueAsChild( CORE::CVariant( value ) );
                ASSERT_TRUE( COM::CJsonRpcEndpoint::ToJsonText( array, text ) );
                CORE::CDataNode parsed;
                ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( text, parsed ) );
                CORE::CDataNode::const_iterator item = parsed.ConstBegin();
                ASSERT_TRUE( item != parsed.ConstEnd() );
                Float64 read = (*item)->GetValue().AsFloat64();
                ASSERT_TRUE( fabs( read - value ) <= fabs( value ) * 1e-12 );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: a request id that is not a string, a number or null is an invalid request and the handler does not run" )
        try
        {
            if ( !JsonAvailable() )
                return;

            const char* invalidIds[] = { "{\"a\":1}", "{}", "[1,2]", "[]", "true", "false" };
            for ( UInt32 i=0; i<sizeof( invalidIds ) / sizeof( invalidIds[ 0 ] ); ++i )
            {
                CCaptureTransport transport;
                CCountingMethods methods;
                COM::CJsonRpcEndpoint endpoint( &transport );
                endpoint.SetRequestHandler( &methods );

                CORE::CString message = CORE::CString( "{\"jsonrpc\":\"2.0\",\"method\":\"sum\",\"params\":[1],\"id\":" ) + invalidIds[ i ] + "}";
                ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_INVALID == endpoint.ProcessIncoming( message ) );
                ASSERT_TRUE( 0 == methods.calls );
                CORE::CDataNode response;
                ASSERT_TRUE( 1 == transport.sent.size() && ParseText( transport.sent[ 0 ], response ) );
                ASSERT_TRUE( -32600 == ErrorCodeOf( response ) );
                ASSERT_TRUE( CORE::CString( "null" ) == IdOf( response ) );

                // also as a notification-like message without the id being usable, and with the version of the lenient dialect
                CCaptureTransport lenientTransport;
                COM::CJsonRpcEndpoint lenient( &lenientTransport );
                lenient.SetDialect( COM::JSONRPC_DIALECT_LENIENT );
                lenient.SetRequestHandler( &methods );
                message = CORE::CString( "{\"method\":\"sum\",\"params\":[1],\"id\":" ) + invalidIds[ i ] + "}";
                lenient.ProcessIncoming( message );
                ASSERT_TRUE( 0 == methods.calls );
                ASSERT_TRUE( 1 == lenientTransport.sent.size() && ParseText( lenientTransport.sent[ 0 ], response ) );
                ASSERT_TRUE( -32600 == ErrorCodeOf( response ) && CORE::CString( "null" ) == IdOf( response ) );
            }

            // inside a batch the invalid one is answered with an error and the others still run
            {
                CCaptureTransport transport;
                CCountingMethods methods;
                COM::CJsonRpcEndpoint endpoint( &transport );
                endpoint.SetRequestHandler( &methods );
                endpoint.ProcessIncoming( "[{\"jsonrpc\":\"2.0\",\"method\":\"sum\",\"id\":{\"a\":1}},{\"jsonrpc\":\"2.0\",\"method\":\"sum\",\"id\":7}]" );
                ASSERT_TRUE( 1 == methods.calls );
                CORE::CDataNode batch;
                ASSERT_TRUE( 1 == transport.sent.size() && ParseText( transport.sent[ 0 ], batch ) && GUCEF_DATATYPE_ARRAY == batch.GetNodeType() );
                ASSERT_TRUE( 2 == ChildCount( batch ) );
            }

            // the ids the specification allows run the handler and are echoed
            const char* validIds[] = { "\"abc\"", "7", "0", "-3", "null" };
            const char* echoed[] = { "abc", "7", "0", "-3", "null" };
            for ( UInt32 i=0; i<sizeof( validIds ) / sizeof( validIds[ 0 ] ); ++i )
            {
                CCaptureTransport transport;
                CCountingMethods methods;
                COM::CJsonRpcEndpoint endpoint( &transport );
                endpoint.SetRequestHandler( &methods );
                CORE::CString message = CORE::CString( "{\"jsonrpc\":\"2.0\",\"method\":\"sum\",\"params\":[1],\"id\":" ) + validIds[ i ] + "}";
                ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( message ) );
                ASSERT_TRUE( 1 == methods.calls );
                CORE::CDataNode response;
                ASSERT_TRUE( 1 == transport.sent.size() && ParseText( transport.sent[ 0 ], response ) );
                ASSERT_TRUE( CORE::CString( echoed[ i ] ) == IdOf( response ) );
            }

            // a number with a fraction is not recommended but allowed: it is echoed as it was sent
            {
                CCaptureTransport transport;
                CCountingMethods methods;
                COM::CJsonRpcEndpoint endpoint( &transport );
                endpoint.SetRequestHandler( &methods );
                ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"jsonrpc\":\"2.0\",\"method\":\"sum\",\"params\":[1],\"id\":1.5}" ) );
                ASSERT_TRUE( 1 == methods.calls && 1 == transport.sent.size() && transport.sent[ 0 ].HasSubstr( "\"id\":1.5," ) >= 0 );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: strings are written as valid UTF-8: ill-formed sequences become U+FFFD" )
        try
        {
            if ( !JsonAvailable() )
                return;

            // well-formed text is untouched
            ASSERT_TRUE( BytesEqual( WriteStringMember( "plain" ), "{\"s\":\"plain\"}" ) );
            ASSERT_TRUE( BytesEqual( WriteStringMember( "h\xC3\xA9llo" ), "{\"s\":\"h\xC3\xA9llo\"}" ) );
            ASSERT_TRUE( BytesEqual( WriteStringMember( "\xE2\x82\xAC" ), "{\"s\":\"\xE2\x82\xAC\"}" ) );
            ASSERT_TRUE( BytesEqual( WriteStringMember( "\xF0\x9F\x98\x80" ), "{\"s\":\"\xF0\x9F\x98\x80\"}" ) );
            ASSERT_TRUE( BytesEqual( WriteStringMember( "\xF4\x8F\xBF\xBF" ), "{\"s\":\"\xF4\x8F\xBF\xBF\"}" ) );     // U+10FFFF
            ASSERT_TRUE( BytesEqual( WriteStringMember( "\xEF\xBF\xBD" ), "{\"s\":\"\xEF\xBF\xBD\"}" ) );
            ASSERT_TRUE( BytesEqual( WriteStringMember( "\xED\x9F\xBF" ), "{\"s\":\"\xED\x9F\xBF\"}" ) );     // U+D7FF, just below the surrogates
            ASSERT_TRUE( BytesEqual( WriteStringMember( "\xEE\x80\x80" ), "{\"s\":\"\xEE\x80\x80\"}" ) );     // U+E000, just above them

            // ill-formed: every maximal ill-formed subsequence becomes one U+FFFD
            ASSERT_TRUE( BytesEqual( WriteStringMember( "a\xC0\xAF" "b" ), "{\"s\":\"a\xEF\xBF\xBD\xEF\xBF\xBD" "b\"}" ) );                          // overlong '/'
            ASSERT_TRUE( BytesEqual( WriteStringMember( "\xE0\x9F\xBF" ), "{\"s\":\"\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\"}" ) );       // overlong 3 byte
            ASSERT_TRUE( BytesEqual( WriteStringMember( "\xF0\x8F\xBF\xBF" ), "{\"s\":\"\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\"}" ) ); // overlong 4 byte
            ASSERT_TRUE( BytesEqual( WriteStringMember( "\xED\xA0\x80" ), "{\"s\":\"\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\"}" ) );       // U+D800 surrogate
            ASSERT_TRUE( BytesEqual( WriteStringMember( "\xED\xBF\xBF" ), "{\"s\":\"\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\"}" ) );       // U+DFFF surrogate
            ASSERT_TRUE( BytesEqual( WriteStringMember( "\xF4\x90\x80\x80" ), "{\"s\":\"\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\"}" ) ); // above U+10FFFF
            ASSERT_TRUE( BytesEqual( WriteStringMember( "\xF5\x80\x80\x80" ), "{\"s\":\"\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\"}" ) );
            ASSERT_TRUE( BytesEqual( WriteStringMember( "\xFF" ), "{\"s\":\"\xEF\xBF\xBD\"}" ) );
            ASSERT_TRUE( BytesEqual( WriteStringMember( "\x80" ), "{\"s\":\"\xEF\xBF\xBD\"}" ) );                                           // stray continuation byte
            ASSERT_TRUE( BytesEqual( WriteStringMember( "\xE2\x82" ), "{\"s\":\"\xEF\xBF\xBD\"}" ) );                                       // truncated at the end
            ASSERT_TRUE( BytesEqual( WriteStringMember( "\xE2\x82" "A" ), "{\"s\":\"\xEF\xBF\xBD" "A\"}" ) );                               // truncated, then text
            ASSERT_TRUE( BytesEqual( WriteStringMember( "\xF0\x9F\x98" ), "{\"s\":\"\xEF\xBF\xBD\"}" ) );                                   // truncated 4 byte
            ASSERT_TRUE( BytesEqual( WriteStringMember( "\xC3" "(" ), "{\"s\":\"\xEF\xBF\xBD(\"}" ) );                                       // lead byte then ASCII

            // names of members and the strings of arrays and nested objects too
            CORE::CDataNode object( GUCEF_DATATYPE_OBJECT );
            object.SetAttribute( "k\xFF", CORE::CString( "v" ) );
            CORE::CDataNode* array = object.AddChild( "list", GUCEF_DATATYPE_ARRAY );
            array->AddValueAsChild( CORE::CVariant( CORE::CString( "x\xC0" ) ) );
            CORE::CString text;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ToJsonText( object, text ) );
            ASSERT_TRUE( BytesEqual( text, "{\"k\xEF\xBF\xBD\":\"v\",\"list\":[\"x\xEF\xBF\xBD\"]}" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // The responses of a batch are one array when they are all known when the batch has been processed. An answer that a
    // handler defers is sent when it is given, as the single response it is: the endpoint does not hold a batch open
    // for answers that may come late or never. MCP does not use batches, the channel of the MCP server refuses them.
    GUCEF_TESTFW_TESTCASE( "Test: answers deferred by the handlers of a batch are sent as individual responses when they are given" )
        try
        {
            if ( !JsonAvailable() )
                return;

            CCaptureTransport transport;
            CCountingMethods methods;
            COM::CJsonRpcEndpoint endpoint( &transport );
            endpoint.SetRequestHandler( &methods );

            // every call of the batch defers: nothing is sent yet
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming(
                "[{\"jsonrpc\":\"2.0\",\"method\":\"later\",\"id\":1},{\"jsonrpc\":\"2.0\",\"method\":\"later\",\"id\":2}]" ) );
            ASSERT_TRUE( 2 == methods.calls );
            ASSERT_TRUE( 0 == transport.sent.size() );

            CORE::CDataNode result;
            result.SetValue( CORE::CVariant( (Int32) 10 ) );
            result.SetNodeType( result.GetValue().GetTypeId() );
            ASSERT_TRUE( endpoint.SendResponse( CORE::CVariant( (Int32) 2 ), result ) );
            ASSERT_TRUE( endpoint.SendResponse( CORE::CVariant( (Int32) 1 ), result ) );
            CORE::CDataNode first, second;
            ASSERT_TRUE( 2 == transport.sent.size() && ParseText( transport.sent[ 0 ], first ) && ParseText( transport.sent[ 1 ], second ) );
            ASSERT_TRUE( GUCEF_DATATYPE_OBJECT == first.GetNodeType() && GUCEF_DATATYPE_OBJECT == second.GetNodeType() );
            ASSERT_TRUE( CORE::CString( "2" ) == IdOf( first ) && CORE::CString( "1" ) == IdOf( second ) );

            // a batch with an answer that is known right away and one that is deferred: the known one is sent as a batch of its own
            transport.sent.clear();
            endpoint.ProcessIncoming( "[{\"jsonrpc\":\"2.0\",\"method\":\"sum\",\"id\":3},{\"jsonrpc\":\"2.0\",\"method\":\"later\",\"id\":4}]" );
            CORE::CDataNode batch;
            ASSERT_TRUE( 1 == transport.sent.size() && ParseText( transport.sent[ 0 ], batch ) && GUCEF_DATATYPE_ARRAY == batch.GetNodeType() );
            ASSERT_TRUE( 1 == ChildCount( batch ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // The json codec of the platform reads integers into 64 bits and builds a double of a larger one. After a number
    // with a fraction it gets that double wrong, ten times too small. That text is refused as a parse error. Without
    // a fraction before it the double is right and the value is left for the receiver to judge, as it always was
    GUCEF_TESTFW_TESTCASE( "Test: integers beyond the 64 bit range are refused where the parser would misread them" )
        try
        {
            if ( !JsonAvailable() )
                return;

            CORE::CDataNode root;
            ASSERT_FALSE( COM::CJsonRpcEndpoint::ParseJsonText( "[1.5,18446744073709551616]", root ) );
            ASSERT_FALSE( COM::CJsonRpcEndpoint::ParseJsonText( "{\"a\":2.5,\"b\":-9223372036854775809}", root ) );
            ASSERT_FALSE( COM::CJsonRpcEndpoint::ParseJsonText( "[0.5,[123456789012345678901234567890]]", root ) );
            ASSERT_FALSE( COM::CJsonRpcEndpoint::ParseJsonText( "[1.5,9223372036854775808]", root ) );

            // without a fraction before it the value is a double of the right size
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( "[18446744073709551616]", root ) );
            ASSERT_TRUE( fabs( (*root.ConstBegin())->GetValue().AsFloat64() / 1.8446744073709552e19 - 1.0 ) < 1e-12 );
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( "{\"a\":-9223372036854775809}", root ) );

            // the limits themselves, numbers with a fraction or an exponent, and digits inside strings are fine after a fraction too
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( "[1.5,9223372036854775807]", root ) );
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( "[1.5,-9223372036854775808]", root ) );
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( "[9223372036854775807]", root ) );
            ASSERT_TRUE( 9223372036854775807LL == (*root.ConstBegin())->GetValue().AsInt64() );
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( "[1.5,1e30,2.5e-3]", root ) );
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( "[1.5,\"18446744073709551616\",1]", root ) );
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( "[1.5,\"a\\\"18446744073709551616\",2]", root ) );

            // an incoming message with such a number is a parse error for the peer
            CCaptureTransport transport;
            CCountingMethods methods;
            COM::CJsonRpcEndpoint endpoint( &transport );
            endpoint.SetRequestHandler( &methods );
            ASSERT_TRUE( COM::CJsonRpcEndpoint::PROCESS_INVALID == endpoint.ProcessIncoming(
                "{\"jsonrpc\":\"2.0\",\"method\":\"sum\",\"params\":[1.5,99999999999999999999],\"id\":1}" ) );
            ASSERT_TRUE( 0 == methods.calls );
            CORE::CDataNode response;
            ASSERT_TRUE( 1 == transport.sent.size() && ParseText( transport.sent[ 0 ], response ) );
            ASSERT_TRUE( -32700 == ErrorCodeOf( response ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL JSON-RPC TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
