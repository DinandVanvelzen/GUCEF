/*
 *  gucefCOM_TestApp: GUCEF test application for the COM module
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

#ifndef GUCEF_COM_TESTAPP_TESTMCPSUPPORT_H
#define GUCEF_COM_TESTAPP_TESTMCPSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

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

#ifndef GUCEF_COM_CMCPSERVER_H
#include "gucefCOM_CMcpServer.h"
#define GUCEF_COM_CMCPSERVER_H
#endif /* GUCEF_COM_CMCPSERVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  What the MCP tests of this test app share: a channel that records what the server sends, a rig with a server on a
 *  pulse generator of the test thread, and tools, resources and prompts to call. Messages are JSON text as a client
 *  writes them, and what comes back is parsed with the json codec, as a client would.
 */

/** Loads the json dstore codec plugin the endpoint needs, the way the other JSON-RPC tests do */
inline bool
McpTestJsonAvailable( void )
{
    GUCEF::CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( GUCEF::CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        return true;
    const GUCEF::CORE::CString pluginName = "dstorepluginJSONPARSER";
    #ifdef GUCEF_DEBUG_MODE
    GUCEF::CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    GUCEF::CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    GUCEF::CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    GUCEF::CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif
    GUCEF::CORE::CPluginControl& pluginControl = GUCEF::CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        pluginControl.AddPluginFromDir( secondPluginPath, "test", true );
    return GUCEF::CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false );
}

/*-------------------------------------------------------------------------*/

inline bool
McpTestParse( const GUCEF::CORE::CString& text, GUCEF::CORE::CDataNode& root )
{
    return GUCEF::COM::CJsonRpcEndpoint::ParseJsonText( text, root );
}

/*-------------------------------------------------------------------------*/

inline GUCEF::CORE::CString
McpTestToJson( const GUCEF::CORE::CDataNode& node )
{
    GUCEF::CORE::CString text;
    GUCEF::COM::CJsonRpcEndpoint::ToJsonText( node, text );
    return text;
}

/*-------------------------------------------------------------------------*/

/** The _meta of a modern request with the given client capabilities (JSON object text) and extra members */
inline GUCEF::CORE::CString
McpTestMeta( const GUCEF::CORE::CString& capabilities = "{}", const GUCEF::CORE::CString& extra = GUCEF::CORE::CString::Empty )
{
    GUCEF::CORE::CString meta = "\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\","
                                "\"io.modelcontextprotocol/clientInfo\":{\"name\":\"test-client\",\"version\":\"1.0\"},"
                                "\"io.modelcontextprotocol/clientCapabilities\":" + capabilities;
    if ( !extra.IsNULLOrEmpty() )
        meta += "," + extra;
    meta += "}";
    return meta;
}

/*-------------------------------------------------------------------------*/

/** A modern request: id is JSON text (7 or "a"), params the members besides _meta, without braces */
inline GUCEF::CORE::CString
McpTestRequest( const GUCEF::CORE::CString& id                                       ,
                const GUCEF::CORE::CString& method                                   ,
                const GUCEF::CORE::CString& params = GUCEF::CORE::CString::Empty     ,
                const GUCEF::CORE::CString& capabilities = "{}"                      ,
                const GUCEF::CORE::CString& extraMeta = GUCEF::CORE::CString::Empty  )
{
    GUCEF::CORE::CString text = "{\"jsonrpc\":\"2.0\",\"id\":" + id + ",\"method\":\"" + method + "\",\"params\":{" + McpTestMeta( capabilities, extraMeta );
    if ( !params.IsNULLOrEmpty() )
        text += "," + params;
    text += "}}";
    return text;
}

/*-------------------------------------------------------------------------*/

/** A request without _meta, as a 2025-11-25 client writes it */
inline GUCEF::CORE::CString
McpTestLegacyRequest( const GUCEF::CORE::CString& id, const GUCEF::CORE::CString& method, const GUCEF::CORE::CString& params = GUCEF::CORE::CString::Empty )
{
    GUCEF::CORE::CString text = "{\"jsonrpc\":\"2.0\",\"id\":" + id + ",\"method\":\"" + method + "\"";
    if ( !params.IsNULLOrEmpty() )
        text += ",\"params\":{" + params + "}";
    text += "}";
    return text;
}

/*-------------------------------------------------------------------------*/

inline GUCEF::CORE::CString
McpTestNotification( const GUCEF::CORE::CString& method, const GUCEF::CORE::CString& params = GUCEF::CORE::CString::Empty )
{
    GUCEF::CORE::CString text = "{\"jsonrpc\":\"2.0\",\"method\":\"" + method + "\"";
    if ( !params.IsNULLOrEmpty() )
        text += ",\"params\":{" + params + "}";
    text += "}";
    return text;
}

/*-------------------------------------------------------------------------*/

inline GUCEF::CORE::CString
McpTestAttr( const GUCEF::CORE::CDataNode& node, const char* name )
{
    const GUCEF::CORE::CDataNode::TKeyValuePair* attribute = node.GetAttribute( name );
    return GUCEF_NULL != attribute ? (*attribute).second.AsString() : GUCEF::CORE::CString( "<none>" );
}

/*-------------------------------------------------------------------------*/

inline GUCEF::Int32
McpTestErrorCode( const GUCEF::CORE::CDataNode& message )
{
    const GUCEF::CORE::CDataNode* error = message.FindChild( "error" );
    if ( GUCEF_NULL == error || GUCEF_NULL == error->GetAttribute( "code" ) )
        return 0;
    return error->GetAttribute( "code" )->second.AsInt32();
}

/*-------------------------------------------------------------------------*/

inline GUCEF::CORE::CString
McpTestErrorMessage( const GUCEF::CORE::CDataNode& message )
{
    const GUCEF::CORE::CDataNode* error = message.FindChild( "error" );
    return GUCEF_NULL == error ? GUCEF::CORE::CString::Empty : McpTestAttr( *error, "message" );
}

/*-------------------------------------------------------------------------*/

/** The data of an error response, GUCEF_NULL when it has none */
inline const GUCEF::CORE::CDataNode*
McpTestErrorData( const GUCEF::CORE::CDataNode& message )
{
    const GUCEF::CORE::CDataNode* error = message.FindChild( "error" );
    return GUCEF_NULL == error ? GUCEF_NULL : error->FindChild( "data" );
}

/*-------------------------------------------------------------------------*/

/** An attribute of the result of a response, "<no result>" when the response has none */
inline GUCEF::CORE::CString
McpTestResultAttr( const GUCEF::CORE::CDataNode& response, const char* name )
{
    const GUCEF::CORE::CDataNode* result = response.FindChild( "result" );
    return GUCEF_NULL != result ? McpTestAttr( *result, name ) : GUCEF::CORE::CString( "<no result>" );
}

/*-------------------------------------------------------------------------*/

/** A member node of the result of a response, GUCEF_NULL when either is missing */
inline const GUCEF::CORE::CDataNode*
McpTestResultChild( const GUCEF::CORE::CDataNode& response, const char* name )
{
    const GUCEF::CORE::CDataNode* result = response.FindChild( "result" );
    return GUCEF_NULL != result ? result->FindChild( name ) : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

/** The text of the first content block of a tool result */
inline GUCEF::CORE::CString
McpTestFirstText( const GUCEF::CORE::CDataNode* result )
{
    if ( GUCEF_NULL == result )
        return "<no result>";
    const GUCEF::CORE::CDataNode* content = result->FindChild( "content" );
    if ( GUCEF_NULL == content || !content->HasChildren() )
        return GUCEF::CORE::CString::Empty;
    return McpTestAttr( *(*content->ConstBegin()), "text" );
}

/*-------------------------------------------------------------------------*/

inline GUCEF::UInt32
McpTestChildCount( const GUCEF::CORE::CDataNode* node )
{
    GUCEF::UInt32 count = 0;
    if ( GUCEF_NULL != node )
        for ( GUCEF::CORE::CDataNode::const_iterator i = node->ConstBegin(); i != node->ConstEnd(); ++i )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A channel that keeps what the server sends, the way a transport would write it */
class CMcpTestChannel : public GUCEF::COM::CMcpChannel
{
    public:

    CMcpTestChannel( EKind kind, const GUCEF::COM::CMcpPrincipal& principal )
        : GUCEF::COM::CMcpChannel( kind, principal )
        , sent()
        , errorCodes()
        , legacyRequests( false )
    {}

    /** As the HTTP exchange channel does for a request with "MCP-Protocol-Version: 2025-11-25" */
    virtual bool IsLegacyRequest( const GUCEF::COM::SJsonRpcRequest& request ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return legacyRequests || GUCEF::COM::CMcpChannel::IsLegacyRequest( request );
    }

    virtual bool SendMessageText( const GUCEF::CORE::CString& messageText ) GUCEF_VIRTUAL_OVERRIDE
    {
        sent.push_back( messageText );
        return true;
    }

    virtual void OnErrorCode( GUCEF::Int32 code ) GUCEF_VIRTUAL_OVERRIDE
    {
        errorCodes.push_back( code );
    }

    GUCEF::vector< GUCEF::CORE::CString > sent;
    GUCEF::vector< GUCEF::Int32 > errorCodes;
    bool legacyRequests;
};

/*-------------------------------------------------------------------------*/

/** Counts the request events of a server, per outcome */
class CMcpTestEventRecorder : public GUCEF::CORE::CObserver
{
    public:

    CMcpTestEventRecorder( GUCEF::COM::CMcpServer& server )
        : GUCEF::CORE::CObserver()
        , started( 0 )
        , finished( 0 )
        , outcomes()
        , channelsOpened( 0 )
        , channelsClosed( 0 )
    {
        typedef GUCEF::CORE::CTEventHandlerFunctor< CMcpTestEventRecorder > TCallback;
        TCallback onStarted( this, &CMcpTestEventRecorder::OnStarted );
        SubscribeTo( &server, GUCEF::COM::CMcpServer::RequestStartedEvent, onStarted );
        TCallback onFinished( this, &CMcpTestEventRecorder::OnFinished );
        SubscribeTo( &server, GUCEF::COM::CMcpServer::RequestFinishedEvent, onFinished );
        TCallback onOpened( this, &CMcpTestEventRecorder::OnOpened );
        SubscribeTo( &server, GUCEF::COM::CMcpServer::ChannelOpenedEvent, onOpened );
        TCallback onClosed( this, &CMcpTestEventRecorder::OnClosed );
        SubscribeTo( &server, GUCEF::COM::CMcpServer::ChannelClosedEvent, onClosed );
    }

    virtual ~CMcpTestEventRecorder()
    {
        UnsubscribeAllFromObserver();
    }

    void OnStarted( GUCEF::CORE::CNotifier*, const GUCEF::CORE::CEvent&, GUCEF::CORE::CICloneable* ) { ++started; }

    void OnFinished( GUCEF::CORE::CNotifier*, const GUCEF::CORE::CEvent&, GUCEF::CORE::CICloneable* eventData )
    {
        ++finished;
        const GUCEF::COM::TMcpRequestEventData* data = static_cast< const GUCEF::COM::TMcpRequestEventData* >( eventData );
        if ( GUCEF_NULL != data )
            outcomes.push_back( data->GetData() );
    }

    void OnOpened( GUCEF::CORE::CNotifier*, const GUCEF::CORE::CEvent&, GUCEF::CORE::CICloneable* ) { ++channelsOpened; }

    void OnClosed( GUCEF::CORE::CNotifier*, const GUCEF::CORE::CEvent&, GUCEF::CORE::CICloneable* ) { ++channelsClosed; }

    /** How many finished requests had the outcome */
    GUCEF::UInt32 CountOutcome( GUCEF::COM::EMcpRequestOutcome outcome ) const
    {
        GUCEF::UInt32 count = 0;
        for ( size_t i=0; i<outcomes.size(); ++i )
            if ( outcomes[ i ].outcome == outcome )
                ++count;
        return count;
    }

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const GUCEF::CORE::CString classTypeName = "CMcpTestEventRecorder";
        return classTypeName;
    }

    GUCEF::UInt32 started;
    GUCEF::UInt32 finished;
    GUCEF::vector< GUCEF::COM::SMcpRequestEventInfo > outcomes;
    GUCEF::UInt32 channelsOpened;
    GUCEF::UInt32 channelsClosed;
};

/*-------------------------------------------------------------------------*/

/**
 *  A server on a pulse generator of the test thread, with a channel. The test thread drives the pulse generator, so it
 *  is the protocol thread of the server; pool tasks run on the threads of the pool.
 */
class CMcpTestRig
{
    public:

    CMcpTestRig( GUCEF::COM::CMcpChannel::EKind kind = GUCEF::COM::CMcpChannel::CHANNEL_STREAM, const char* subject = "local:test" )
        : pulseGenerator( GUCEF::CORE::CPulseGenerator::CreateSharedObj() )
        , server( GUCEF_NULL )
        , channelPtr()
        , channel( GUCEF_NULL )
        , extraChannels()
    {
        pulseGenerator->SetPulseDriverThreadId( GUCEF::MT::GetCurrentTaskID() );
        server = GUCEF_NEW GUCEF::COM::CMcpServer( pulseGenerator );
        GUCEF::COM::CMcpImplementationInfo info;
        info.name = "test-server";
        info.version = "2.3.4";
        server->SetServerInfo( info );
        channel = AddChannel( kind, subject );
        channelPtr = extraChannels.back();
    }

    ~CMcpTestRig()
    {
        for ( size_t i=0; i<extraChannels.size(); ++i )
            server->CloseChannel( extraChannels[ i ] );
        extraChannels.clear();
        channelPtr.Unlink();
        GUCEF_DELETE server;
        server = GUCEF_NULL;
    }

    CMcpTestChannel* AddChannel( GUCEF::COM::CMcpChannel::EKind kind, const char* subject )
    {
        CMcpTestChannel* newChannel = GUCEF_NEW CMcpTestChannel( kind, GUCEF::COM::CMcpPrincipal( "test", subject, true ) );
        GUCEF::COM::CMcpChannelPtr ptr( newChannel );
        server->OpenChannel( ptr );
        extraChannels.push_back( ptr );
        return newChannel;
    }

    void SetSettings( const GUCEF::COM::CMcpServerSettings& settings )
    {
        server->SetSettings( settings );
        // channels made before the change keep the limits they were opened with: open the main one again
        server->CloseChannel( channelPtr );
        GUCEF::COM::CMcpChannel::EKind kind = channel->GetKind();
        GUCEF::COM::CMcpPrincipal principal = channel->GetPrincipal();
        channel = GUCEF_NEW CMcpTestChannel( kind, principal );
        channelPtr = GUCEF::COM::CMcpChannelPtr( channel );
        server->OpenChannel( channelPtr );
        extraChannels.push_back( channelPtr );
    }

    void Pulse( GUCEF::UInt32 times = 1 )
    {
        for ( GUCEF::UInt32 i=0; i<times; ++i )
        {
            pulseGenerator->ForceDirectPulse();
            GUCEF::MT::ThreadDelay( 1 );
        }
    }

    bool Send( const GUCEF::CORE::CString& text, CMcpTestChannel* target = GUCEF_NULL )
    {
        return ( GUCEF_NULL != target ? target : channel )->ProcessIncomingMessage( text );
    }

    /** Finds the response with the given id (as text) among what was sent on the channel */
    bool FindResponse( const GUCEF::CORE::CString& idText, GUCEF::CORE::CDataNode& message, CMcpTestChannel* target = GUCEF_NULL ) const
    {
        // the id as JSON text or as its value: "\"a\"" and "a" both find the string id a
        GUCEF::CORE::CString id = idText;
        if ( id.Length() >= 2 && '"' == id[ 0 ] && '"' == id[ id.Length() - 1 ] )
            id = id.SubstrFromRange( 1, id.Length() - 1 );
        const CMcpTestChannel* source = GUCEF_NULL != target ? target : channel;
        for ( size_t i=0; i<source->sent.size(); ++i )
        {
            GUCEF::CORE::CDataNode node;
            if ( !McpTestParse( source->sent[ i ], node ) )
                continue;
            if ( GUCEF_NULL != node.GetAttribute( "method" ) )
                continue;
            const GUCEF::CORE::CDataNode::TKeyValuePair* responseId = node.GetAttribute( "id" );
            if ( GUCEF_NULL != responseId && (*responseId).second.AsString() == id )
            {
                message = node;
                return true;
            }
        }
        return false;
    }

    /** Sends and returns the response, pulsing until it came */
    bool Call( const GUCEF::CORE::CString& id, const GUCEF::CORE::CString& text, GUCEF::CORE::CDataNode& response, GUCEF::UInt32 timeoutInMs = 5000, CMcpTestChannel* target = GUCEF_NULL )
    {
        Send( text, target );
        return WaitForResponse( id, response, timeoutInMs, target );
    }

    bool WaitForResponse( const GUCEF::CORE::CString& id, GUCEF::CORE::CDataNode& response, GUCEF::UInt32 timeoutInMs = 5000, CMcpTestChannel* target = GUCEF_NULL )
    {
        for ( GUCEF::UInt32 waited=0; waited<=timeoutInMs; ++waited )
        {
            if ( FindResponse( id, response, target ) )
                return true;
            Pulse();
        }
        return false;
    }

    /** The notifications of a method that were sent, in order */
    GUCEF::UInt32 GetNotifications( const GUCEF::CORE::CString& method, GUCEF::vector< GUCEF::CORE::CDataNode >& found, CMcpTestChannel* target = GUCEF_NULL ) const
    {
        found.clear();
        const CMcpTestChannel* source = GUCEF_NULL != target ? target : channel;
        for ( size_t i=0; i<source->sent.size(); ++i )
        {
            GUCEF::CORE::CDataNode node;
            if ( McpTestParse( source->sent[ i ], node ) && McpTestAttr( node, "method" ) == method )
                found.push_back( node );
        }
        return (GUCEF::UInt32) found.size();
    }

    /** The index of the first sent message that contains the text, -1 when none */
    GUCEF::Int32 IndexOf( const GUCEF::CORE::CString& text, CMcpTestChannel* target = GUCEF_NULL ) const
    {
        const CMcpTestChannel* source = GUCEF_NULL != target ? target : channel;
        for ( size_t i=0; i<source->sent.size(); ++i )
            if ( source->sent[ i ].HasSubstr( text ) >= 0 )
                return (GUCEF::Int32) i;
        return -1;
    }

    /** Every message the channel got is one line of JSON text that parses */
    bool AllSentAreJsonLines( CMcpTestChannel* target = GUCEF_NULL ) const
    {
        const CMcpTestChannel* source = GUCEF_NULL != target ? target : channel;
        for ( size_t i=0; i<source->sent.size(); ++i )
        {
            GUCEF::CORE::CDataNode node;
            if ( source->sent[ i ].HasChar( '\n' ) >= 0 || source->sent[ i ].HasChar( '\r' ) >= 0 || !McpTestParse( source->sent[ i ], node ) )
                return false;
        }
        return true;
    }

    GUCEF::CORE::PulseGeneratorPtr pulseGenerator;
    GUCEF::COM::CMcpServer* server;
    GUCEF::COM::CMcpChannelPtr channelPtr;
    CMcpTestChannel* channel;
    GUCEF::vector< GUCEF::COM::CMcpChannelPtr > extraChannels;
};

/*-------------------------------------------------------------------------*/

/**
 *  A tool whose behaviour the test picks: echo of "text", optionally slow (steps of 'stepMs' with progress), a structured
 *  result, input required and so on. The same Execute() serves every execution mode.
 */
class CMcpTestTool : public GUCEF::COM::CIMcpTool
{
    public:

    enum EBehaviour
    {
        ECHO                ,   /**< text content with the "text" argument */
        COUNT               ,   /**< 'steps' steps of 'stepMs', progress per step, stops when cancelled */
        DEFERRED            ,   /**< answers DEFERRED; the test completes it through 'lastContext' */
        STRUCTURED          ,   /**< structuredContent { "sum": a + b } */
        STRUCTURED_WRONG    ,   /**< structuredContent that breaks the output schema */
        ASK_NAME            ,   /**< form elicitation of a name, then greets */
        STATE_ONLY          ,   /**< input required with state only, then completes on the retry */
        INPUT_NOTHING       ,   /**< input required without requests and without state: a bug */
        PROTOCOL_ERROR      ,   /**< -32602 with data */
        THROWS              ,   /**< throws */
        SAMPLING                /**< input required with a sampling request */
    };

    CMcpTestTool( const char* toolName, EBehaviour toolBehaviour, GUCEF::COM::EMcpExecutionMode mode = GUCEF::COM::MCP_EXECUTION_CALLING_THREAD,
                  GUCEF::COM::EMcpToolEffect toolEffect = GUCEF::COM::MCP_TOOLEFFECT_READ_ONLY )
        : name( toolName )
        , behaviour( toolBehaviour )
        , executionMode( mode )
        , effect( toolEffect )
        , steps( 10 )
        , stepMs( 5 )
        , maxDurationMs( 0 )
        , executions( 0 )
        , sawCancellation( 0 )
        , progressRefused( 0 )
        , lastContextLock()
        , lastContext()
        , lastNonce()
        , lastStateStep()
    {}

    virtual bool GetDescriptor( GUCEF::COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = name;
        descriptor.title = "Test tool " + name;
        descriptor.description = "A tool of the MCP tests";
        descriptor.effect = effect;
        descriptor.idempotent = true;
        descriptor.maxDurationMs = maxDurationMs;
        GUCEF::CORE::CDataNode schema;
        if ( STRUCTURED == behaviour || STRUCTURED_WRONG == behaviour )
        {
            McpTestParse( "{\"type\":\"object\",\"properties\":{\"a\":{\"type\":\"integer\"},\"b\":{\"type\":\"integer\"}},\"required\":[\"a\",\"b\"]}", schema );
            descriptor.inputSchema = schema;
            GUCEF::CORE::CDataNode output;
            McpTestParse( "{\"type\":\"object\",\"properties\":{\"sum\":{\"type\":\"integer\"}},\"required\":[\"sum\"],\"additionalProperties\":false}", output );
            descriptor.outputSchema = output;
            descriptor.hasOutputSchema = true;
        }
        else
        {
            // built in code, not parsed: tools are registered from several threads at once in the concurrency test
            GUCEF::CORE::CDataNode built( GUCEF_DATATYPE_OBJECT );
            built.SetAttribute( "type", "object" );
            GUCEF::CORE::CDataNode* properties = built.AddChild( "properties", GUCEF_DATATYPE_OBJECT );
            GUCEF::CORE::CDataNode* text = properties->AddChild( "text", GUCEF_DATATYPE_OBJECT );
            text->SetAttribute( "type", "string" );
            if ( ECHO == behaviour )
            {
                text->SetAttribute( "maxLength", (GUCEF::Int32) 64 );
                built.AddChild( "required", GUCEF_DATATYPE_ARRAY )->AddValueAsChild( GUCEF::CORE::CVariant( "text" ) );
            }
            else
            {
                properties->AddChild( "steps", GUCEF_DATATYPE_OBJECT )->SetAttribute( "type", "integer" );
            }
            descriptor.inputSchema = built;
        }
        return true;
    }

    virtual GUCEF::COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return executionMode;
    }

    virtual GUCEF::COM::EMcpExecuteResult Execute( GUCEF::COM::CMcpRequestContext& context        ,
                                                   const GUCEF::CORE::CDataNode& arguments        ,
                                                   GUCEF::COM::CMcpToolResult& result             ,
                                                   GUCEF::COM::CMcpInputRequired& inputRequired   ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++executions;
        switch ( behaviour )
        {
            case ECHO:
            {
                result.AddText( "echo: " + arguments.GetAttributeValue( "text" ).AsString() );
                return GUCEF::COM::MCP_EXECUTE_COMPLETE;
            }
            case COUNT:
            {
                GUCEF::UInt32 count = arguments.GetAttribute( "steps" ) != GUCEF_NULL ? arguments.GetAttributeValue( "steps" ).AsUInt32() : steps;
                for ( GUCEF::UInt32 i=1; i<=count; ++i )
                {
                    if ( context.IsCancellationRequested() )
                    {
                        sawCancellation.Store( 1 );
                        result.SetError( "cancelled" );
                        return GUCEF::COM::MCP_EXECUTE_COMPLETE;
                    }
                    GUCEF::MT::ThreadDelay( stepMs );
                    if ( !context.ReportProgress( (GUCEF::Float64) i, (GUCEF::Float64) count, "step" ) )
                        ++progressRefused;
                    if ( 2 == i && !context.ReportProgress( 1.0 ) )     // not increasing: refused
                        ++progressRefused;
                }
                result.AddText( "counted" );
                return GUCEF::COM::MCP_EXECUTE_COMPLETE;
            }
            case DEFERRED:
            {
                GUCEF::MT::CScopeMutex lock( lastContextLock );
                lastContext = context.GetSharedPtr();
                return GUCEF::COM::MCP_EXECUTE_DEFERRED;
            }
            case STRUCTURED:
            case STRUCTURED_WRONG:
            {
                GUCEF::CORE::CDataNode value( GUCEF_DATATYPE_OBJECT );
                GUCEF::Int64 sum = arguments.GetAttributeValue( "a" ).AsInt64() + arguments.GetAttributeValue( "b" ).AsInt64();
                if ( STRUCTURED == behaviour )
                    value.SetAttribute( "sum", sum );
                else
                    value.SetAttribute( "total", sum );
                result.SetStructuredContent( value );
                return GUCEF::COM::MCP_EXECUTE_COMPLETE;
            }
            case ASK_NAME:
            {
                const GUCEF::CORE::CDataNode* answer = context.HasInputResponses() ? context.GetInputResponses().FindChild( "name_request" ) : GUCEF_NULL;
                const GUCEF::CORE::CDataNode* content = GUCEF_NULL != answer ? answer->FindChild( "content" ) : GUCEF_NULL;
                if ( GUCEF_NULL == content || !context.HasRequestState() )
                {
                    GUCEF::CORE::CDataNode requested;
                    McpTestParse( "{\"type\":\"object\",\"properties\":{\"name\":{\"type\":\"string\"}},\"required\":[\"name\"]}", requested );
                    inputRequired.AddFormElicitation( "name_request", "What is your name?", requested );
                    GUCEF::CORE::CDataNode state( GUCEF_DATATYPE_OBJECT );
                    state.SetAttribute( "step", (GUCEF::Int32) 1 );
                    inputRequired.SetState( state );
                    inputRequired.SetStateNonce();
                    return GUCEF::COM::MCP_EXECUTE_INPUT_REQUIRED;
                }
                {
                    GUCEF::MT::CScopeMutex lock( lastContextLock );
                    lastNonce = context.GetStateNonce();
                    lastStateStep = context.GetRequestState().GetAttributeValue( "step" ).AsString();
                }
                result.AddText( "Hello " + content->GetAttributeValue( "name" ).AsString() );
                return GUCEF::COM::MCP_EXECUTE_COMPLETE;
            }
            case STATE_ONLY:
            {
                if ( !context.HasRequestState() )
                {
                    GUCEF::CORE::CDataNode state( GUCEF_DATATYPE_OBJECT );
                    state.SetAttribute( "attempt", (GUCEF::Int32) 1 );
                    inputRequired.SetState( state );
                    return GUCEF::COM::MCP_EXECUTE_INPUT_REQUIRED;
                }
                result.AddText( "attempt " + context.GetRequestState().GetAttributeValue( "attempt" ).AsString() );
                return GUCEF::COM::MCP_EXECUTE_COMPLETE;
            }
            case INPUT_NOTHING:
            {
                return GUCEF::COM::MCP_EXECUTE_INPUT_REQUIRED;
            }
            case PROTOCOL_ERROR:
            {
                GUCEF::CORE::CDataNode data( GUCEF_DATATYPE_OBJECT );
                data.SetAttribute( "why", "test" );
                context.SetProtocolError( -32602, "the thing does not exist", &data );
                return GUCEF::COM::MCP_EXECUTE_PROTOCOL_ERROR;
            }
            case THROWS:
            {
                throw std::exception();
            }
            case SAMPLING:
            {
                // built in code: this runs on a pool thread
                GUCEF::CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
                GUCEF::CORE::CDataNode* message = params.AddChild( "messages", GUCEF_DATATYPE_ARRAY )->AddChild( GUCEF::CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                message->SetAttribute( "role", "user" );
                GUCEF::CORE::CDataNode* content = message->AddChild( "content", GUCEF_DATATYPE_OBJECT );
                content->SetAttribute( "type", "text" );
                content->SetAttribute( "text", "hi" );
                params.SetAttribute( "maxTokens", (GUCEF::Int32) 10 );
                inputRequired.AddInputRequest( "sample", GUCEF::COM::CMcpMethods::SamplingCreateMessage, params );
                return GUCEF::COM::MCP_EXECUTE_INPUT_REQUIRED;
            }
        }
        return GUCEF::COM::MCP_EXECUTE_COMPLETE;
    }

    /** Completes the last deferred call, from whatever thread calls this */
    bool CompleteLast( const GUCEF::CORE::CString& text )
    {
        GUCEF::COM::CMcpRequestContextPtr context;
        {
            GUCEF::MT::CScopeMutex lock( lastContextLock );
            context = lastContext;
            lastContext.Unlink();
        }
        if ( context.IsNULL() )
            return false;
        GUCEF::COM::CMcpToolResult result;
        result.AddText( text );
        return context->CompleteWithToolResult( result );
    }

    bool HasDeferred( void )
    {
        GUCEF::MT::CScopeMutex lock( lastContextLock );
        return !lastContext.IsNULL();
    }

    GUCEF::CORE::CString name;
    EBehaviour behaviour;
    GUCEF::COM::EMcpExecutionMode executionMode;
    GUCEF::COM::EMcpToolEffect effect;
    GUCEF::UInt32 steps;
    GUCEF::UInt32 stepMs;
    GUCEF::UInt32 maxDurationMs;
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > executions;
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > sawCancellation;
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > progressRefused;
    GUCEF::MT::CMutex lastContextLock;
    GUCEF::COM::CMcpRequestContextPtr lastContext;
    GUCEF::CORE::CString lastNonce;
    GUCEF::CORE::CString lastStateStep;
};

/*-------------------------------------------------------------------------*/

/** A fixed text resource, or a template "test://items/{id}" that knows items 1 to 9 */
class CMcpTestResource : public GUCEF::COM::CIMcpResource
{
    public:

    CMcpTestResource( const char* resourceUri, const char* resourceName, GUCEF::UInt32 ttl = 0, bool needsInput = false )
        : uri( resourceUri ), name( resourceName ), ttlMs( ttl ), askInput( needsInput ), mode( GUCEF::COM::MCP_EXECUTION_CALLING_THREAD ) {}

    virtual bool GetDescriptor( GUCEF::COM::CMcpResourceDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.uri = uri;
        descriptor.name = name;
        descriptor.mimeType = "text/plain";
        descriptor.annotations.hasPriority = true;
        descriptor.annotations.priority = 0.5;
        return true;
    }

    virtual GUCEF::COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE { return mode; }

    virtual GUCEF::COM::EMcpExecuteResult Read( GUCEF::COM::CMcpRequestContext& context, GUCEF::COM::CMcpResourceContents& contents, GUCEF::COM::CMcpInputRequired& inputRequired ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( askInput && !context.HasRequestState() )
        {
            GUCEF::CORE::CDataNode state( GUCEF_DATATYPE_OBJECT );
            state.SetAttribute( "confirmed", false );
            inputRequired.SetState( state );
            return GUCEF::COM::MCP_EXECUTE_INPUT_REQUIRED;
        }
        contents.AddText( uri, "text/plain", "contents of " + name );
        return GUCEF::COM::MCP_EXECUTE_COMPLETE;
    }

    virtual void GetCacheHints( GUCEF::UInt32& ttl, GUCEF::COM::EMcpCacheScope& scope ) const GUCEF_VIRTUAL_OVERRIDE
    {
        ttl = ttlMs;
        scope = ttlMs > 0 ? GUCEF::COM::MCP_CACHESCOPE_PUBLIC : GUCEF::COM::MCP_CACHESCOPE_PRIVATE;
    }

    GUCEF::CORE::CString uri;
    GUCEF::CORE::CString name;
    GUCEF::UInt32 ttlMs;
    bool askInput;
    GUCEF::COM::EMcpExecutionMode mode;
};

/*-------------------------------------------------------------------------*/

class CMcpTestTemplate : public GUCEF::COM::CIMcpResourceTemplate
{
    public:

    CMcpTestTemplate( const char* templateName, const char* uriTemplate ) : name( templateName ), pattern( uriTemplate ) {}

    virtual bool GetDescriptor( GUCEF::COM::CMcpResourceTemplateDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = name;
        descriptor.uriTemplate = pattern;
        descriptor.mimeType = "application/json";
        return true;
    }

    virtual GUCEF::COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF::COM::MCP_EXECUTION_THREAD_POOL; }

    virtual GUCEF::COM::EMcpExecuteResult Read( GUCEF::COM::CMcpRequestContext& context      ,
                                                const GUCEF::CORE::CString& readUri          ,
                                                const GUCEF::CORE::CValueList& variables     ,
                                                GUCEF::COM::CMcpResourceContents& contents   ,
                                                GUCEF::COM::CMcpInputRequired& inputRequired ) GUCEF_VIRTUAL_OVERRIDE
    {
        GUCEF::CORE::CVariant id;
        if ( !variables.TryGetValue( "id", id ) || id.AsString().Length() != 1 || id.AsString() == "0" )
        {
            context.SetProtocolError( -32602, "Resource not found" );
            return GUCEF::COM::MCP_EXECUTE_PROTOCOL_ERROR;
        }
        contents.AddText( readUri, "application/json", "{\"item\":\"" + id.AsString() + "\"}" );
        contents.SetCacheHints( 1234, GUCEF::COM::MCP_CACHESCOPE_PUBLIC );
        return GUCEF::COM::MCP_EXECUTE_COMPLETE;
    }

    GUCEF::CORE::CString name;
    GUCEF::CORE::CString pattern;
};

/*-------------------------------------------------------------------------*/

class CMcpTestPrompt : public GUCEF::COM::CIMcpPrompt
{
    public:

    CMcpTestPrompt( const char* promptName ) : name( promptName ), askInput( false ) {}

    virtual bool GetDescriptor( GUCEF::COM::CMcpPromptDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = name;
        descriptor.description = "Greets someone";
        descriptor.AddArgument( "who", "the one to greet", true );
        descriptor.AddArgument( "style", "formal or not", false );
        return true;
    }

    virtual GUCEF::COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF::COM::MCP_EXECUTION_CALLING_THREAD; }

    virtual GUCEF::COM::EMcpExecuteResult Get( GUCEF::COM::CMcpRequestContext& context        ,
                                               const GUCEF::CORE::CValueList& arguments       ,
                                               GUCEF::COM::CMcpPromptResult& result           ,
                                               GUCEF::COM::CMcpInputRequired& inputRequired   ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( askInput && !context.HasRequestState() )
        {
            GUCEF::CORE::CDataNode state( GUCEF_DATATYPE_OBJECT );
            state.SetAttribute( "asked", true );
            inputRequired.SetState( state );
            return GUCEF::COM::MCP_EXECUTE_INPUT_REQUIRED;
        }
        GUCEF::CORE::CVariant who;
        arguments.TryGetValue( "who", who );
        result.description = "a greeting";
        result.AddUserText( "Please greet " + who.AsString() );
        result.AddAssistantText( "Hello " + who.AsString() );
        return GUCEF::COM::MCP_EXECUTE_COMPLETE;
    }

    GUCEF::CORE::CString name;
    bool askInput;
};

/*-------------------------------------------------------------------------*/

/** A policy the tests set up: everything allowed unless a flag says otherwise */
class CMcpTestPolicy : public GUCEF::COM::CIMcpAccessPolicy
{
    public:

    CMcpTestPolicy( void )
        : principalDependent( false ), denyTool(), hideTool(), allowOnlySubject(), denyResources( false ), denySubscriptions( false ), denyPrompts( false ) {}

    virtual bool IsVisibilityPrincipalDependent( void ) const GUCEF_VIRTUAL_OVERRIDE { return principalDependent; }

    virtual bool IsToolVisible( const GUCEF::COM::CMcpPrincipal& principal, const GUCEF::COM::CMcpToolEntry& tool ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( tool.descriptor.name == hideTool )
            return false;
        return allowOnlySubject.IsNULLOrEmpty() || principal.subject == allowOnlySubject || tool.descriptor.effect == GUCEF::COM::MCP_TOOLEFFECT_READ_ONLY;
    }

    virtual GUCEF::COM::EMcpAccessDecision AuthorizeToolCall( const GUCEF::COM::CMcpRequestContext& context ,
                                                              const GUCEF::COM::CMcpToolEntry& tool          ,
                                                              const GUCEF::CORE::CDataNode& arguments        ,
                                                              GUCEF::CORE::CString& reason                   ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( tool.descriptor.name == denyTool )
        {
            reason = "the test denies it";
            return GUCEF::COM::MCP_ACCESS_DENY;
        }
        return IsToolVisible( context.GetPrincipal(), tool ) ? GUCEF::COM::MCP_ACCESS_ALLOW : GUCEF::COM::MCP_ACCESS_HIDE;
    }

    virtual bool IsResourceVisible( const GUCEF::COM::CMcpPrincipal& principal, const GUCEF::CORE::CString& uri ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return uri.HasSubstr( "hidden" ) < 0;
    }

    virtual GUCEF::COM::EMcpAccessDecision AuthorizeResourceRead( const GUCEF::COM::CMcpRequestContext& context, const GUCEF::CORE::CString& uri, GUCEF::CORE::CString& reason ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return denyResources ? GUCEF::COM::MCP_ACCESS_DENY : GUCEF::COM::MCP_ACCESS_ALLOW;
    }

    virtual bool IsPromptVisible( const GUCEF::COM::CMcpPrincipal& principal, const GUCEF::COM::CMcpPromptEntry& prompt ) const GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual GUCEF::COM::EMcpAccessDecision AuthorizePromptGet( const GUCEF::COM::CMcpRequestContext& context, const GUCEF::COM::CMcpPromptEntry& prompt, GUCEF::CORE::CString& reason ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return denyPrompts ? GUCEF::COM::MCP_ACCESS_DENY : GUCEF::COM::MCP_ACCESS_ALLOW;
    }

    virtual GUCEF::COM::EMcpAccessDecision AuthorizeSubscription( const GUCEF::COM::CMcpRequestContext& context, GUCEF::CORE::CString& reason ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( denySubscriptions )
            reason = "no subscriptions in this test";
        return denySubscriptions ? GUCEF::COM::MCP_ACCESS_DENY : GUCEF::COM::MCP_ACCESS_ALLOW;
    }

    bool principalDependent;
    GUCEF::CORE::CString denyTool;
    GUCEF::CORE::CString hideTool;
    GUCEF::CORE::CString allowOnlySubject;     /**< when set, tools that are not read-only are visible to this subject only */
    bool denyResources;
    bool denySubscriptions;
    bool denyPrompts;
};

/*-------------------------------------------------------------------------*/

/** Runs a function object style job on a thread of its own: Job() is called once */
class CMcpTestThread : public GUCEF::MT::CActiveObject
{
    public:

    CMcpTestThread( void ) : GUCEF::MT::CActiveObject(), m_done( 0 ) {}

    virtual ~CMcpTestThread() {}

    virtual void Job( void ) = 0;

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        Job();
        m_done.Store( 1, GUCEF::MT::MEMORDER_RELEASE );
        return true;
    }

    virtual const GUCEF::MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool Start( void ) { return Activate( GUCEF_NULL, 0, 0 ); }

    bool IsDone( void ) const { return 0 != m_done.Load( GUCEF::MT::MEMORDER_ACQUIRE ); }

    /** Waits for the job and the thread, pulsing the rig meanwhile when there is one */
    bool Join( CMcpTestRig* rig, GUCEF::UInt32 timeoutInMs = 20000 )
    {
        GUCEF::UInt32 waited = 0;
        while ( ( !IsDone() || IsActive() ) && waited < timeoutInMs )
        {
            if ( GUCEF_NULL != rig )
                rig->Pulse();
            else
                GUCEF::MT::ThreadDelay( 1 );
            ++waited;
        }
        return IsDone() && !IsActive();
    }

    private:
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > m_done;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_TESTAPP_TESTMCPSUPPORT_H ? */
