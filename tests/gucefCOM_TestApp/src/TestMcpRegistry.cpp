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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_COREECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_COREECODECTYPES_H
#endif /* GUCEF_CORE_COREECODECTYPES_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestMcpSupport.h"
#include "TestMcpRegistry.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A tool that only describes itself, for the registry tests */
class CDescribedTool : public COM::CIMcpTool
{
    public:

    CDescribedTool( const CORE::CString& toolName, const CORE::CString& schemaJson = "{\"type\":\"object\"}" )
        : name( toolName ), schema( schemaJson ), outputSchema(), effect( COM::MCP_TOOLEFFECT_READ_ONLY ) {}

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = name;
        descriptor.effect = effect;
        CORE::CDataNode node;
        if ( !McpTestParse( schema, node ) )
            return false;
        descriptor.inputSchema = node;
        if ( !outputSchema.IsNULLOrEmpty() )
        {
            McpTestParse( outputSchema, node );
            descriptor.outputSchema = node;
            descriptor.hasOutputSchema = true;
        }
        return true;
    }

    virtual COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE { return COM::MCP_EXECUTION_CALLING_THREAD; }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context, const CORE::CDataNode& arguments, COM::CMcpToolResult& result, COM::CMcpInputRequired& inputRequired ) GUCEF_VIRTUAL_OVERRIDE
    {
        result.AddText( name );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    CORE::CString name;
    CORE::CString schema;
    CORE::CString outputSchema;
    COM::EMcpToolEffect effect;
};

/** Counts the events of a registry */
class CRegistryRecorder : public CORE::CObserver
{
    public:

    CRegistryRecorder( COM::CMcpRegistryBase& registry, const CORE::CEvent& registered, const CORE::CEvent& unregistered )
        : CORE::CObserver(), listChanged( 0 ), registeredNames(), unregisteredNames()
    {
        typedef CORE::CTEventHandlerFunctor< CRegistryRecorder > TCallback;
        TCallback onListChanged( this, &CRegistryRecorder::OnListChanged );
        SubscribeTo( &registry, COM::CMcpRegistryBase::ListChangedEvent, onListChanged );
        TCallback onRegistered( this, &CRegistryRecorder::OnRegistered );
        SubscribeTo( &registry, registered, onRegistered );
        TCallback onUnregistered( this, &CRegistryRecorder::OnUnregistered );
        SubscribeTo( &registry, unregistered, onUnregistered );
    }

    virtual ~CRegistryRecorder() { UnsubscribeAllFromObserver(); }

    void OnListChanged( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++listChanged; }

    void OnRegistered( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        registeredNames.push_back( static_cast< COM::CMcpRegistryBase::TNameEventData* >( eventData )->GetData() );
    }

    void OnUnregistered( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        unregisteredNames.push_back( static_cast< COM::CMcpRegistryBase::TNameEventData* >( eventData )->GetData() );
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CRegistryRecorder";
        return classTypeName;
    }

    UInt32 listChanged;
    GUCEF::vector< CORE::CString > registeredNames;
    GUCEF::vector< CORE::CString > unregisteredNames;
};

CORE::PulseGeneratorPtr
MakeTestPulseGenerator( void )
{
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
    pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentTaskID() );
    return pulseGenerator;
}

bool
RegisterTool( COM::CMcpToolRegistry& registry, const CORE::CString& name, const CORE::CString& schema, CORE::CString& error )
{
    COM::CIMcpToolPtr tool( GUCEF_NEW CDescribedTool( name, schema ) );
    return registry.Register( tool, error );
}

COM::CMcpRequestStateSealer::SBinding
MakeBinding( const char* subject, const char* method, const char* target, const char* argumentsJson )
{
    COM::CMcpRequestStateSealer::SBinding binding;
    binding.subject = subject;
    binding.method = method;
    binding.target = target;
    CORE::CDataNode arguments;
    McpTestParse( argumentsJson, arguments );
    COM::CMcpRequestStateSealer::DigestArguments( &arguments, binding.argumentsDigest );
    return binding;
}

UInt64
NowMs( void )
{
    return CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

/** Runs the seal tests that both protections must pass */
void
CheckSealer( const COM::CMcpRequestStateSealer& sealer )
{
    COM::CMcpRequestStateSealer::SBinding binding = MakeBinding( "local:alice", "tools/call", "ask", "{\"x\":1}" );
    CORE::CDataNode state;
    ASSERT_TRUE( McpTestParse( "{\"step\":2,\"items\":[\"a\",\"b\"],\"nested\":{\"k\":true}}", state ) );

    CORE::CString sealed;
    CORE::CString error;
    ASSERT_TRUE( sealer.Seal( binding, state, true, NowMs() + 60000, sealed, error ) );
    ASSERT_FALSE( sealed.IsNULLOrEmpty() );

    // round trip, with the nonce
    CORE::CDataNode unsealed;
    CORE::CString nonce;
    ASSERT_TRUE( COM::CMcpRequestStateSealer::UNSEAL_OK == sealer.Unseal( sealed, binding, NowMs(), unsealed, nonce ) );
    ASSERT_TRUE( McpTestToJson( state ) == McpTestToJson( unsealed ) );
    ASSERT_TRUE( 32 == nonce.Length() );

    // two seals of the same state differ in their nonce
    CORE::CString sealedAgain;
    CORE::CString nonceAgain;
    ASSERT_TRUE( sealer.Seal( binding, state, true, NowMs() + 60000, sealedAgain, error ) );
    ASSERT_TRUE( sealedAgain != sealed );
    ASSERT_TRUE( COM::CMcpRequestStateSealer::UNSEAL_OK == sealer.Unseal( sealedAgain, binding, NowMs(), unsealed, nonceAgain ) );
    ASSERT_TRUE( nonceAgain != nonce );

    // without a nonce
    ASSERT_TRUE( sealer.Seal( binding, state, false, NowMs() + 60000, sealedAgain, error ) );
    ASSERT_TRUE( COM::CMcpRequestStateSealer::UNSEAL_OK == sealer.Unseal( sealedAgain, binding, NowMs(), unsealed, nonceAgain ) );
    ASSERT_TRUE( nonceAgain.IsNULLOrEmpty() );

    // every single character changed is refused
    const char* text = sealed.C_String();
    UInt32 size = sealed.ByteSize() - 1;
    UInt32 accepted = 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        GUCEF::vector< char > tampered( text, text + size );
        tampered[ i ] = ( 'A' == tampered[ i ] ) ? 'B' : 'A';
        if ( '.' == text[ i ] )
            tampered[ i ] = 'x';
        CORE::CString tamperedText( &tampered[ 0 ], size );
        if ( COM::CMcpRequestStateSealer::UNSEAL_OK == sealer.Unseal( tamperedText, binding, NowMs(), unsealed, nonce ) )
            ++accepted;
    }
    ASSERT_TRUE( 0 == accepted );

    // another principal, method, target or arguments
    COM::CMcpRequestStateSealer::EUnsealResult otherBinding = COM::CMcpRequestStateSealer::PROTECTION_AEAD == sealer.GetProtection() ?
        COM::CMcpRequestStateSealer::UNSEAL_TAMPERED : COM::CMcpRequestStateSealer::UNSEAL_WRONG_BINDING;
    ASSERT_TRUE( otherBinding == sealer.Unseal( sealed, MakeBinding( "local:bob", "tools/call", "ask", "{\"x\":1}" ), NowMs(), unsealed, nonce ) );
    ASSERT_TRUE( otherBinding == sealer.Unseal( sealed, MakeBinding( "local:alice", "prompts/get", "ask", "{\"x\":1}" ), NowMs(), unsealed, nonce ) );
    ASSERT_TRUE( otherBinding == sealer.Unseal( sealed, MakeBinding( "local:alice", "tools/call", "other", "{\"x\":1}" ), NowMs(), unsealed, nonce ) );
    ASSERT_TRUE( otherBinding == sealer.Unseal( sealed, MakeBinding( "local:alice", "tools/call", "ask", "{\"x\":2}" ), NowMs(), unsealed, nonce ) );
    ASSERT_TRUE( unsealed.GetAttCount() == 0 && !unsealed.HasChildren() );
    ASSERT_TRUE( nonce.IsNULLOrEmpty() );

    // expired
    ASSERT_TRUE( sealer.Seal( binding, state, false, NowMs() - 1, sealedAgain, error ) );
    ASSERT_TRUE( COM::CMcpRequestStateSealer::UNSEAL_EXPIRED == sealer.Unseal( sealedAgain, binding, NowMs(), unsealed, nonce ) );

    // garbage
    ASSERT_TRUE( COM::CMcpRequestStateSealer::UNSEAL_MALFORMED == sealer.Unseal( "", binding, NowMs(), unsealed, nonce ) );
    ASSERT_TRUE( COM::CMcpRequestStateSealer::UNSEAL_OK != sealer.Unseal( "not a seal at all", binding, NowMs(), unsealed, nonce ) );
    ASSERT_TRUE( COM::CMcpRequestStateSealer::UNSEAL_OK != sealer.Unseal( "AAAA.AAAA", binding, NowMs(), unsealed, nonce ) );
    ASSERT_TRUE( COM::CMcpRequestStateSealer::UNSEAL_OK != sealer.Unseal( "\xc3\xa9\xc3\xa9.\xc3\xa9", binding, NowMs(), unsealed, nonce ) );
    CORE::CString huge;
    for ( UInt32 i=0; i<20000; ++i )
        huge += 'A';
    ASSERT_TRUE( COM::CMcpRequestStateSealer::UNSEAL_MALFORMED == sealer.Unseal( huge, binding, NowMs(), unsealed, nonce ) );

    // a state that is not an object or array, and one that is too large, is refused
    CORE::CDataNode scalar;
    scalar.SetValue( CORE::CVariant( (Int32) 5 ) );
    scalar.SetNodeType( GUCEF_DATATYPE_INT32 );
    ASSERT_FALSE( sealer.Seal( binding, scalar, false, NowMs() + 60000, sealedAgain, error ) );
    CORE::CDataNode large( GUCEF_DATATYPE_OBJECT );
    large.SetAttribute( "blob", CORE::CVariant( huge ) );
    ASSERT_FALSE( sealer.Seal( binding, large, false, NowMs() + 60000, sealedAgain, error ) );
    ASSERT_FALSE( error.IsNULLOrEmpty() );
}

bool
HasAeadCodec( void )
{
    CORE::CCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
    CORE::CString::StringVector types = COM::CMcpRequestStateSealer::DefaultAeadCodecTypes.ParseElements( ',', false );
    for ( size_t i=0; i<types.size(); ++i )
        if ( !registry.TryGetCodec( CORE::CoreCodecTypes::EncryptionCodec, types[ i ] ).IsNULL() )
            return true;
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMcpRegistryTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING MCP VALUE, REGISTRY AND SEALER TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "McpRegistry" );

    if ( !McpTestJsonAvailable() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MCP TESTS skipped: the json codec is not available" );
        return;
    }

    GUCEF_TESTFW_TESTCASE( "Test: A tool descriptor is written as the Tool object, with the hints generated from its effect" )
        try
        {
            COM::CMcpToolDescriptor descriptor;
            descriptor.name = "list_notes";
            descriptor.title = "List notes";
            descriptor.description = "Lists the notes";
            McpTestParse( "[{\"src\":\"https://example.org/icon.png\",\"mimeType\":\"image/png\",\"sizes\":[\"48x48\"]}]", descriptor.icons );
            descriptor.SetNoArguments();
            McpTestParse( "{\"type\":\"array\",\"items\":{\"type\":\"string\"}}", descriptor.outputSchema );
            descriptor.hasOutputSchema = true;
            descriptor.meta.SetAttribute( "com.example/owner", "tests" );

            struct SCase { COM::EMcpToolEffect effect; bool idempotent; bool openWorld; const char* readOnly; const char* destructive; };
            SCase cases[ 3 ] = { { COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "true", "false" },
                                 { COM::MCP_TOOLEFFECT_ADDITIVE, false, true, "false", "false" },
                                 { COM::MCP_TOOLEFFECT_DESTRUCTIVE, false, false, "false", "true" } };
            for ( int c=0; c<3; ++c )
            {
                descriptor.effect = cases[ c ].effect;
                descriptor.idempotent = cases[ c ].idempotent;
                descriptor.openWorld = cases[ c ].openWorld;
                CORE::CDataNode node;
                ASSERT_TRUE( descriptor.ToDataNode( node ) );

                // through text and back, as a client sees it
                CORE::CDataNode tool;
                ASSERT_TRUE( McpTestParse( McpTestToJson( node ), tool ) );
                ASSERT_TRUE( McpTestAttr( tool, "name" ) == "list_notes" );
                ASSERT_TRUE( McpTestAttr( tool, "title" ) == "List notes" );
                ASSERT_TRUE( McpTestAttr( tool, "description" ) == "Lists the notes" );
                const CORE::CDataNode* annotations = tool.FindChild( "annotations" );
                ASSERT_TRUE( GUCEF_NULL != annotations );
                if ( GUCEF_NULL != annotations )
                {
                    ASSERT_TRUE( McpTestAttr( *annotations, "readOnlyHint" ).Lowercase() == cases[ c ].readOnly );
                    ASSERT_TRUE( McpTestAttr( *annotations, "destructiveHint" ).Lowercase() == cases[ c ].destructive );
                    ASSERT_TRUE( annotations->GetAttributeValue( "idempotentHint" ).AsBool() == cases[ c ].idempotent );
                    ASSERT_TRUE( annotations->GetAttributeValue( "openWorldHint" ).AsBool() == cases[ c ].openWorld );
                }
                const CORE::CDataNode* inputSchema = tool.FindChild( "inputSchema" );
                ASSERT_TRUE( GUCEF_NULL != inputSchema && McpTestAttr( *inputSchema, "type" ) == "object" && !inputSchema->GetAttributeValue( "additionalProperties" ).AsBool( true ) );
                ASSERT_TRUE( GUCEF_NULL != tool.FindChild( "outputSchema" ) );
                ASSERT_TRUE( 1 == McpTestChildCount( tool.FindChild( "icons" ) ) );
                const CORE::CDataNode* meta = tool.FindChild( "_meta" );
                ASSERT_TRUE( GUCEF_NULL != meta && McpTestAttr( *meta, "com.example/owner" ) == "tests" );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Content blocks of every type, with annotations" )
        try
        {
            CORE::CDynamicBuffer bytes;
            bytes.Append( "hello", 5 );

            CORE::CDataNode node;
            ASSERT_TRUE( COM::CMcpContent::Text( "line one\nline two \x01" ).ToDataNode( node ) );
            CORE::CString json = McpTestToJson( node );
            ASSERT_TRUE( json.HasChar( '\n' ) < 0 && json.HasSubstr( "\\u0001" ) >= 0 );
            CORE::CDataNode parsed;
            ASSERT_TRUE( McpTestParse( json, parsed ) && McpTestAttr( parsed, "type" ) == "text" && McpTestAttr( parsed, "text" ) == "line one\nline two \x01" );

            ASSERT_TRUE( COM::CMcpContent::Image( bytes, "image/png" ).SetAudience( true, false ).SetPriority( 1.5 ).ToDataNode( node ) );
            ASSERT_TRUE( McpTestParse( McpTestToJson( node ), parsed ) );
            ASSERT_TRUE( McpTestAttr( parsed, "type" ) == "image" && McpTestAttr( parsed, "data" ) == "aGVsbG8=" && McpTestAttr( parsed, "mimeType" ) == "image/png" );
            const CORE::CDataNode* annotations = parsed.FindChild( "annotations" );
            ASSERT_TRUE( GUCEF_NULL != annotations && 1 == McpTestChildCount( annotations->FindChild( "audience" ) ) );
            ASSERT_TRUE( GUCEF_NULL != annotations && 1.0 == annotations->GetAttributeValue( "priority" ).AsFloat64() );   // clamped

            ASSERT_TRUE( COM::CMcpContent::Audio( bytes, "audio/wav" ).ToDataNode( node ) );
            ASSERT_TRUE( McpTestParse( McpTestToJson( node ), parsed ) && McpTestAttr( parsed, "type" ) == "audio" && GUCEF_NULL == parsed.FindChild( "annotations" ) );

            COM::CMcpContent link = COM::CMcpContent::ResourceLink( "file:///a.txt", "a.txt", "text/plain" );
            link.hasSize = true;
            link.size = 42;
            link.SetLastModified( "2025-01-12T15:00:58Z" );
            ASSERT_TRUE( link.ToDataNode( node ) );
            ASSERT_TRUE( McpTestParse( McpTestToJson( node ), parsed ) );
            ASSERT_TRUE( McpTestAttr( parsed, "type" ) == "resource_link" && McpTestAttr( parsed, "uri" ) == "file:///a.txt" && McpTestAttr( parsed, "size" ) == "42" );
            ASSERT_TRUE( GUCEF_NULL != parsed.FindChild( "annotations" ) && McpTestAttr( *parsed.FindChild( "annotations" ), "lastModified" ) == "2025-01-12T15:00:58Z" );

            ASSERT_TRUE( COM::CMcpContent::EmbeddedText( "file:///b.txt", "text/plain", "B" ).ToDataNode( node ) );
            ASSERT_TRUE( McpTestParse( McpTestToJson( node ), parsed ) && McpTestAttr( parsed, "type" ) == "resource" );
            ASSERT_TRUE( GUCEF_NULL != parsed.FindChild( "resource" ) && McpTestAttr( *parsed.FindChild( "resource" ), "text" ) == "B" );
            ASSERT_TRUE( COM::CMcpContent::EmbeddedBlob( "file:///c.bin", "application/octet-stream", bytes ).ToDataNode( node ) );
            ASSERT_TRUE( McpTestParse( McpTestToJson( node ), parsed ) && GUCEF_NULL != parsed.FindChild( "resource" ) );
            ASSERT_TRUE( McpTestAttr( *parsed.FindChild( "resource" ), "blob" ) == "aGVsbG8=" && McpTestAttr( *parsed.FindChild( "resource" ), "text" ) == "<none>" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Tool results, resource contents and prompt results are written with their members" )
        try
        {
            COM::CMcpToolResult result;
            CORE::CDataNode value;
            ASSERT_TRUE( McpTestParse( "{\"temperature\":22.5,\"conditions\":\"cloudy\"}", value ) );
            result.SetStructuredContent( value );
            result.meta.SetAttribute( "com.example/x", (Int32) 1 );
            CORE::CDataNode node( GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( result.ToDataNode( node ) );
            CORE::CDataNode parsed;
            ASSERT_TRUE( McpTestParse( McpTestToJson( node ), parsed ) );
            ASSERT_TRUE( 1 == McpTestChildCount( parsed.FindChild( "content" ) ) );
            CORE::CDataNode fromText;
            ASSERT_TRUE( McpTestParse( McpTestFirstText( &parsed ), fromText ) && McpTestAttr( fromText, "conditions" ) == "cloudy" );   // the JSON as text too
            ASSERT_TRUE( GUCEF_NULL != parsed.FindChild( "structuredContent" ) && 22.5 == parsed.FindChild( "structuredContent" )->GetAttributeValue( "temperature" ).AsFloat64() );
            ASSERT_TRUE( McpTestAttr( parsed, "isError" ).Lowercase() == "false" );
            ASSERT_TRUE( GUCEF_NULL != parsed.FindChild( "_meta" ) );

            // a clone is a full copy, for futures
            CORE::CICloneable* clone = result.Clone();
            ASSERT_TRUE( GUCEF_NULL != CORE::CICloneable::CloneableAs< COM::CMcpToolResult >( clone ) );
            ASSERT_TRUE( 1 == CORE::CICloneable::CloneableAs< COM::CMcpToolResult >( clone )->content.size() );
            GUCEF_DELETE clone;

            result.Clear();
            result.SetError( "it broke" );
            node = CORE::CDataNode( GUCEF_DATATYPE_OBJECT );
            result.ToDataNode( node );
            ASSERT_TRUE( McpTestParse( McpTestToJson( node ), parsed ) && McpTestAttr( parsed, "isError" ).Lowercase() == "true" && McpTestFirstText( &parsed ) == "it broke" );

            CORE::CDynamicBuffer bytes;
            bytes.Append( "\x00\x01\x02", 3 );
            COM::CMcpResourceContents contents;
            contents.AddText( "file:///a", "text/plain", "A" ).AddBlob( "file:///b", "application/octet-stream", bytes );
            node = CORE::CDataNode( GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( contents.ToDataNode( node ) );
            ASSERT_TRUE( McpTestParse( McpTestToJson( node ), parsed ) && 2 == McpTestChildCount( parsed.FindChild( "contents" ) ) );
            CORE::CDataNode::const_iterator item = parsed.FindChild( "contents" )->ConstBegin();
            ASSERT_TRUE( McpTestAttr( *(*item), "text" ) == "A" );
            ++item;
            ASSERT_TRUE( McpTestAttr( *(*item), "blob" ) == "AAEC" );

            COM::CMcpPromptResult prompt;
            prompt.description = "greeting";
            prompt.AddUserText( "hi" ).AddAssistantText( "hello" );
            node = CORE::CDataNode( GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( prompt.ToDataNode( node ) );
            ASSERT_TRUE( McpTestParse( McpTestToJson( node ), parsed ) && McpTestAttr( parsed, "description" ) == "greeting" );
            const CORE::CDataNode* messages = parsed.FindChild( "messages" );
            ASSERT_TRUE( 2 == McpTestChildCount( messages ) );
            if ( 2 == McpTestChildCount( messages ) )
            {
                CORE::CDataNode::const_iterator m = messages->ConstBegin();
                ASSERT_TRUE( McpTestAttr( *(*m), "role" ) == "user" && GUCEF_NULL != (*m)->FindChild( "content" ) && McpTestAttr( *(*m)->FindChild( "content" ), "text" ) == "hi" );
                ++m;
                ASSERT_TRUE( McpTestAttr( *(*m), "role" ) == "assistant" );
            }

            COM::CMcpResourceDescriptor resource;
            resource.uri = "file:///r";
            resource.name = "r";
            resource.hasSize = true;
            resource.size = 7;
            resource.annotations.hasAudience = true;
            resource.annotations.audienceAssistant = true;
            ASSERT_TRUE( resource.ToDataNode( node ) );
            ASSERT_TRUE( McpTestParse( McpTestToJson( node ), parsed ) && McpTestAttr( parsed, "uri" ) == "file:///r" && McpTestAttr( parsed, "size" ) == "7" );
            COM::CMcpResourceTemplateDescriptor resourceTemplate;
            resourceTemplate.name = "items";
            resourceTemplate.uriTemplate = "test://items/{id}";
            ASSERT_TRUE( resourceTemplate.ToDataNode( node ) );
            ASSERT_TRUE( McpTestParse( McpTestToJson( node ), parsed ) && McpTestAttr( parsed, "uriTemplate" ) == "test://items/{id}" );
            COM::CMcpPromptDescriptor promptDescriptor;
            promptDescriptor.name = "greet";
            promptDescriptor.AddArgument( "who", "", true );
            ASSERT_TRUE( promptDescriptor.ToDataNode( node ) );
            ASSERT_TRUE( McpTestParse( McpTestToJson( node ), parsed ) && 1 == McpTestChildCount( parsed.FindChild( "arguments" ) ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Input requests and the client capabilities they need" )
        try
        {
            CORE::CDataNode schema;
            McpTestParse( "{\"type\":\"object\",\"properties\":{\"name\":{\"type\":\"string\"}}}", schema );
            COM::CMcpInputRequired form;
            form.AddFormElicitation( "who", "Your name?", schema );
            CORE::CDataNode required;
            form.GetRequiredCapabilities( required );
            ASSERT_TRUE( McpTestToJson( required ) == "{\"elicitation\":{}}" );
            ASSERT_TRUE( form.GetInputRequests().size() == 1 );
            const COM::CMcpInputRequired::SInputRequest& request = (*form.GetInputRequests().begin()).second;
            ASSERT_TRUE( request.method == "elicitation/create" && McpTestAttr( request.params, "mode" ) == "form" && McpTestAttr( request.params, "message" ) == "Your name?" );
            ASSERT_TRUE( GUCEF_NULL != request.params.FindChild( "requestedSchema" ) );
            ASSERT_FALSE( form.HasState() );

            CORE::CDataNode urlParams;
            McpTestParse( "{\"mode\":\"url\",\"message\":\"Log in\",\"url\":\"https://example.org\"}", urlParams );
            COM::CMcpInputRequired mixed;
            mixed.AddFormElicitation( "a", "?", schema ).AddInputRequest( "b", "elicitation/create", urlParams );
            mixed.GetRequiredCapabilities( required );
            ASSERT_TRUE( McpTestToJson( required ) == "{\"elicitation\":{\"form\":{},\"url\":{}}}" );

            CORE::CDataNode samplingParams( GUCEF_DATATYPE_OBJECT );
            COM::CMcpInputRequired other;
            other.AddInputRequest( "s", "sampling/createMessage", samplingParams ).AddInputRequest( "r", "roots/list", samplingParams );
            other.SetStateNonce();
            other.GetRequiredCapabilities( required );
            ASSERT_TRUE( McpTestToJson( required ) == "{\"roots\":{},\"sampling\":{}}" );
            ASSERT_TRUE( other.HasState() && other.GetWantsStateNonce() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: _meta is parsed with its key names whole, and wrong types are malformed" )
        try
        {
            // keys with '/' and '.' survive the parse, a node and a write
            CORE::CString text = "{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\",\"io.modelcontextprotocol/clientCapabilities\":"
                                 "{\"elicitation\":{},\"sampling\":{\"tools\":{}}},\"io.modelcontextprotocol/clientInfo\":{\"name\":\"c\",\"version\":\"1\"},"
                                 "\"progressToken\":\"p1\",\"traceparent\":\"00-0af7651916cd43dd8448eb211c80319c-00f067aa0ba902b7-01\",\"com.example/x.y\":1}}";
            CORE::CDataNode params;
            ASSERT_TRUE( McpTestParse( text, params ) );
            const CORE::CDataNode* metaNode = params.FindChild( "_meta" );
            ASSERT_TRUE( GUCEF_NULL != metaNode && GUCEF_NULL != metaNode->GetAttribute( "com.example/x.y" ) );
            CORE::CDataNode again;
            ASSERT_TRUE( McpTestParse( McpTestToJson( params ), again ) && McpTestToJson( again ) == McpTestToJson( params ) );

            COM::CMcpRequestMeta meta;
            CORE::CString error;
            ASSERT_TRUE( COM::CMcpRequestMeta::META_OK == meta.Parse( metaNode, error ) );
            ASSERT_TRUE( meta.GetProtocolVersion() == "2026-07-28" );
            ASSERT_TRUE( meta.HasClientInfo() && meta.GetClientInfo().name == "c" );
            ASSERT_TRUE( meta.HasProgressToken() && meta.GetProgressToken().AsString() == "p1" );
            ASSERT_TRUE( meta.GetTraceParent().HasSubstr( "0af7651916cd43dd" ) >= 0 );
            ASSERT_TRUE( meta.HasCapability( "elicitation" ) );
            ASSERT_TRUE( meta.HasCapability( "elicitation/form" ) );      // an empty elicitation object is form mode
            ASSERT_FALSE( meta.HasCapability( "elicitation/url" ) );
            ASSERT_TRUE( meta.HasCapability( "sampling/tools" ) );
            ASSERT_FALSE( meta.HasCapability( "roots" ) );

            CORE::CDataNode capabilities;
            McpTestParse( "{\"elicitation\":{\"url\":{}}}", capabilities );
            ASSERT_FALSE( COM::CMcpRequestMeta::HasCapability( capabilities, "elicitation/form" ) );   // url only
            CORE::CDataNode requiredForm;
            McpTestParse( "{\"elicitation\":{}}", requiredForm );
            ASSERT_FALSE( COM::CMcpRequestMeta::HasAllCapabilities( capabilities, requiredForm ) );
            ASSERT_TRUE( COM::CMcpRequestMeta::HasAllCapabilities( metaNode->FindChild( "io.modelcontextprotocol/clientCapabilities" ) ? *metaNode->FindChild( "io.modelcontextprotocol/clientCapabilities" ) : capabilities, requiredForm ) );

            const char* malformed[] = {
                "{\"io.modelcontextprotocol/protocolVersion\":2026}",
                "{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\"}",
                "{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\",\"io.modelcontextprotocol/clientCapabilities\":\"all\"}",
                "{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\",\"io.modelcontextprotocol/clientCapabilities\":[]}",
                "{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\",\"io.modelcontextprotocol/clientCapabilities\":{},\"io.modelcontextprotocol/clientInfo\":\"me\"}",
                "{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\",\"io.modelcontextprotocol/clientCapabilities\":{},\"io.modelcontextprotocol/clientInfo\":{\"name\":\"x\"}}",
                "{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\",\"io.modelcontextprotocol/clientCapabilities\":{},\"progressToken\":true}",
                "{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\",\"io.modelcontextprotocol/clientCapabilities\":{},\"progressToken\":1.5}",
                "{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\",\"io.modelcontextprotocol/clientCapabilities\":{},\"io.modelcontextprotocol/logLevel\":3}",
                "{\"io.modelcontextprotocol/protocolVersion\":[\"2026-07-28\"],\"io.modelcontextprotocol/clientCapabilities\":{}}" };
            for ( UInt32 i=0; i<sizeof( malformed ) / sizeof( malformed[ 0 ] ); ++i )
            {
                CORE::CDataNode node;
                ASSERT_TRUE( McpTestParse( malformed[ i ], node ) );
                error.Clear();
                ASSERT_TRUE( COM::CMcpRequestMeta::META_MALFORMED == meta.Parse( &node, error ) );
                ASSERT_FALSE( error.IsNULLOrEmpty() );
            }
            CORE::CDataNode noVersion;
            McpTestParse( "{\"progressToken\":1}", noVersion );
            ASSERT_TRUE( COM::CMcpRequestMeta::META_NO_PROTOCOL_VERSION == meta.Parse( &noVersion, error ) );
            ASSERT_TRUE( COM::CMcpRequestMeta::META_NO_PROTOCOL_VERSION == meta.Parse( GUCEF_NULL, error ) );
            CORE::CDataNode integerToken;
            McpTestParse( "{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\",\"io.modelcontextprotocol/clientCapabilities\":{},\"progressToken\":7}", integerToken );
            ASSERT_TRUE( COM::CMcpRequestMeta::META_OK == meta.Parse( &integerToken, error ) && meta.GetProgressToken().IsInteger() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The tool registry checks names, keeps byte order and counts generations" )
        try
        {
            CORE::PulseGeneratorPtr pulseGenerator = MakeTestPulseGenerator();
            COM::CMcpToolRegistry registry( pulseGenerator );
            CRegistryRecorder recorder( registry, COM::CMcpToolRegistry::ToolRegisteredEvent, COM::CMcpToolRegistry::ToolUnregisteredEvent );
            CORE::CString error;

            CORE::CString longName;
            for ( int i=0; i<129; ++i )
                longName += 'a';
            ASSERT_FALSE( RegisterTool( registry, "", "{\"type\":\"object\"}", error ) );
            ASSERT_FALSE( RegisterTool( registry, longName, "{\"type\":\"object\"}", error ) );
            ASSERT_TRUE( RegisterTool( registry, longName.SubstrFromRange( 0, 128 ), "{\"type\":\"object\"}", error ) );
            ASSERT_FALSE( RegisterTool( registry, "with space", "{\"type\":\"object\"}", error ) );
            ASSERT_FALSE( RegisterTool( registry, "comma,name", "{\"type\":\"object\"}", error ) );
            ASSERT_FALSE( RegisterTool( registry, "\xc3\xa9", "{\"type\":\"object\"}", error ) );
            ASSERT_FALSE( error.IsNULLOrEmpty() );
            ASSERT_TRUE( 1 == registry.GetCount() );
            ASSERT_TRUE( registry.Unregister( longName.SubstrFromRange( 0, 128 ) ) );

            UInt64 generation = registry.GetGeneration();
            const char* names[] = { "zeta", "Alpha", "alpha", "admin.tools.list", "DATA_EXPORT_v2", "b-2", "_x" };
            for ( int i=0; i<7; ++i )
                ASSERT_TRUE( RegisterTool( registry, names[ i ], "{\"type\":\"object\"}", error ) );
            ASSERT_TRUE( registry.GetGeneration() == generation + 7 );

            // the order is byte order whatever the registration order
            COM::CMcpToolRegistry::TToolEntryVector entries;
            registry.GetEntries( entries );
            ASSERT_TRUE( 7 == entries.size() );
            for ( size_t i=1; i<entries.size(); ++i )
                ASSERT_TRUE( entries[ i-1 ]->descriptor.name < entries[ i ]->descriptor.name );
            ASSERT_TRUE( entries[ 0 ]->descriptor.name == "Alpha" && entries[ 6 ]->descriptor.name == "zeta" );

            // another tool with a name in use is refused; the same tool again replaces its entry
            ASSERT_FALSE( RegisterTool( registry, "zeta", "{\"type\":\"object\"}", error ) );
            COM::CMcpToolEntryPtr zeta;
            ASSERT_TRUE( registry.TryGet( "zeta", zeta ) );
            UInt64 before = registry.GetGeneration();
            ASSERT_TRUE( registry.Register( zeta->tool, error ) );
            ASSERT_TRUE( registry.GetGeneration() == before + 1 );
            COM::CMcpToolEntryPtr replaced;
            ASSERT_TRUE( registry.TryGet( "zeta", replaced ) && replaced != zeta && replaced->generation == before + 1 );

            // an entry stays alive for whoever holds it after the unregister
            ASSERT_TRUE( registry.Unregister( "zeta" ) );
            ASSERT_FALSE( registry.Unregister( "zeta" ) );
            ASSERT_FALSE( registry.TryGet( "zeta", zeta ) );
            ASSERT_TRUE( replaced->descriptor.name == "zeta" );

            // events: one per registration, the list change once per pulse
            ASSERT_TRUE( recorder.registeredNames.size() == 9 && recorder.unregisteredNames.size() == 2 );
            ASSERT_TRUE( 0 == recorder.listChanged );
            ASSERT_TRUE( registry.IsListChangePending() );
            pulseGenerator->ForceDirectPulse();
            ASSERT_TRUE( 1 == recorder.listChanged );
            pulseGenerator->ForceDirectPulse();
            ASSERT_TRUE( 1 == recorder.listChanged );
            for ( int i=0; i<20; ++i )
                RegisterTool( registry, "burst" + CORE::ToString( i ), "{\"type\":\"object\"}", error );
            pulseGenerator->ForceDirectPulse();
            ASSERT_TRUE( 2 == recorder.listChanged );

            registry.UnregisterAll();
            ASSERT_TRUE( 0 == registry.GetCount() );
            ASSERT_TRUE( recorder.unregisteredNames.size() == 2 + 6 + 20 );
            registry.FlushListChanged();
            ASSERT_TRUE( 3 == recorder.listChanged );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The tool registry compiles the schemas and refuses what it cannot check" )
        try
        {
            CORE::PulseGeneratorPtr pulseGenerator = MakeTestPulseGenerator();
            COM::CMcpToolRegistry registry( pulseGenerator );
            CORE::CString error;
            ASSERT_FALSE( RegisterTool( registry, "a", "{\"type\":\"string\"}", error ) );
            ASSERT_FALSE( RegisterTool( registry, "b", "{\"properties\":{}}", error ) );
            ASSERT_FALSE( RegisterTool( registry, "c", "{\"$schema\":\"http://json-schema.org/draft-07/schema#\",\"type\":\"object\"}", error ) );
            ASSERT_TRUE( error.HasSubstr( "input schema" ) >= 0 );
            ASSERT_FALSE( RegisterTool( registry, "d", "{\"type\":\"object\",\"properties\":{\"x\":{\"type\":\"string\",\"pattern\":\"^a\"}}}", error ) );
            ASSERT_FALSE( RegisterTool( registry, "e", "{\"type\":\"object\",\"properties\":{\"x\":{\"$ref\":\"https://example.org/s.json\"}}}", error ) );
            ASSERT_TRUE( RegisterTool( registry, "f", "{\"type\":\"object\",\"properties\":{\"n\":{\"type\":\"integer\"}},\"required\":[\"n\"]}", error ) );
            COM::CMcpToolEntryPtr entry;
            ASSERT_TRUE( registry.TryGet( "f", entry ) && entry->inputSchemaRequiresProperties && entry->inputSchema.IsCompiled() );

            CDescribedTool* withOutput = GUCEF_NEW CDescribedTool( "g" );
            withOutput->outputSchema = "{\"type\":\"object\",\"properties\":{\"x\":{\"type\":\"string\",\"pattern\":\"y\"}}}";
            ASSERT_FALSE( registry.Register( COM::CIMcpToolPtr( withOutput ), error ) );
            ASSERT_TRUE( error.HasSubstr( "output schema" ) >= 0 );
            CDescribedTool* goodOutput = GUCEF_NEW CDescribedTool( "h" );
            goodOutput->outputSchema = "{\"type\":\"array\",\"items\":{\"type\":\"string\"}}";
            ASSERT_TRUE( registry.Register( COM::CIMcpToolPtr( goodOutput ), error ) );
            ASSERT_TRUE( registry.TryGet( "h", entry ) && entry->hasOutputSchema && entry->outputSchema.IsCompiled() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: x-mcp-header annotations follow the rules of the Streamable HTTP transport" )
        try
        {
            CORE::PulseGeneratorPtr pulseGenerator = MakeTestPulseGenerator();
            COM::CMcpToolRegistry registry( pulseGenerator );
            CORE::CString error;
            ASSERT_TRUE( RegisterTool( registry, "ok", "{\"type\":\"object\",\"properties\":{\"region\":{\"type\":\"string\",\"x-mcp-header\":\"Region\"},"
                                                 "\"opts\":{\"type\":\"object\",\"properties\":{\"limit\":{\"type\":\"integer\",\"x-mcp-header\":\"Limit\"},"
                                                 "\"dry\":{\"type\":\"boolean\",\"x-mcp-header\":\"Dry-Run\"}}}}}", error ) );
            COM::CMcpToolEntryPtr entry;
            ASSERT_TRUE( registry.TryGet( "ok", entry ) && 3 == entry->headerParameters.size() );
            bool foundLimit = false;
            for ( size_t i=0; i<entry->headerParameters.size(); ++i )
            {
                if ( entry->headerParameters[ i ].headerName == "Limit" )
                {
                    foundLimit = true;
                    ASSERT_TRUE( 2 == entry->headerParameters[ i ].propertyPath.size() && entry->headerParameters[ i ].propertyPath[ 0 ] == "opts" &&
                                 entry->headerParameters[ i ].propertyPath[ 1 ] == "limit" && entry->headerParameters[ i ].type == "integer" );
                }
            }
            ASSERT_TRUE( foundLimit );

            ASSERT_FALSE( RegisterTool( registry, "empty", "{\"type\":\"object\",\"properties\":{\"r\":{\"type\":\"string\",\"x-mcp-header\":\"\"}}}", error ) );
            ASSERT_FALSE( RegisterTool( registry, "space", "{\"type\":\"object\",\"properties\":{\"r\":{\"type\":\"string\",\"x-mcp-header\":\"Re gion\"}}}", error ) );
            ASSERT_FALSE( RegisterTool( registry, "colon", "{\"type\":\"object\",\"properties\":{\"r\":{\"type\":\"string\",\"x-mcp-header\":\"Re:gion\"}}}", error ) );
            ASSERT_FALSE( RegisterTool( registry, "notstring", "{\"type\":\"object\",\"properties\":{\"r\":{\"type\":\"string\",\"x-mcp-header\":5}}}", error ) );
            ASSERT_FALSE( RegisterTool( registry, "dup", "{\"type\":\"object\",\"properties\":{\"a\":{\"type\":\"string\",\"x-mcp-header\":\"Name\"},\"b\":{\"type\":\"string\",\"x-mcp-header\":\"name\"}}}", error ) );
            ASSERT_TRUE( error.HasSubstr( "more than once" ) >= 0 );
            ASSERT_FALSE( RegisterTool( registry, "number", "{\"type\":\"object\",\"properties\":{\"a\":{\"type\":\"number\",\"x-mcp-header\":\"A\"}}}", error ) );
            ASSERT_FALSE( RegisterTool( registry, "notype", "{\"type\":\"object\",\"properties\":{\"a\":{\"x-mcp-header\":\"A\"}}}", error ) );
            ASSERT_FALSE( RegisterTool( registry, "items", "{\"type\":\"object\",\"properties\":{\"a\":{\"type\":\"array\",\"items\":{\"type\":\"string\",\"x-mcp-header\":\"A\"}}}}", error ) );
            ASSERT_FALSE( RegisterTool( registry, "anyof", "{\"type\":\"object\",\"anyOf\":[{\"properties\":{\"a\":{\"type\":\"string\",\"x-mcp-header\":\"A\"}}}]}", error ) );
            ASSERT_FALSE( RegisterTool( registry, "defs", "{\"type\":\"object\",\"$defs\":{\"d\":{\"type\":\"string\",\"x-mcp-header\":\"A\"}},\"properties\":{\"a\":{\"$ref\":\"#/$defs/d\"}}}", error ) );
            ASSERT_FALSE( RegisterTool( registry, "root", "{\"type\":\"object\",\"x-mcp-header\":\"Root\"}", error ) );
            ASSERT_TRUE( 1 == registry.GetCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: URI templates support {var} and {+var} only and match with decoding" )
        try
        {
            COM::CMcpUriTemplate matcher;
            CORE::CString error;
            const char* refused[] = { "", "x{", "x}", "a{?q}", "a{#x}", "a{.x}", "a{/x}", "a{;x}", "a{&x}", "a{a,b}", "a{x*}", "a{x:3}", "a{}", "a{x}{y}", "a{{x}}" };
            for ( UInt32 i=0; i<sizeof( refused ) / sizeof( refused[ 0 ] ); ++i )
                ASSERT_FALSE( matcher.Parse( refused[ i ], error ) );

            CORE::CValueList variables;
            ASSERT_TRUE( matcher.Parse( "test://items/{id}/detail", error ) && 1 == matcher.GetVariableCount() );
            ASSERT_TRUE( matcher.Match( "test://items/a%20b/detail", variables ) );
            CORE::CVariant value;
            ASSERT_TRUE( variables.TryGetValue( "id", value ) && value.AsString() == "a b" );
            ASSERT_FALSE( matcher.Match( "test://items/a/b/detail", variables ) );      // {var} does not take a '/'
            ASSERT_FALSE( matcher.Match( "test://items//detail", variables ) );         // nor nothing
            ASSERT_FALSE( matcher.Match( "test://items/x/detailed", variables ) );
            ASSERT_FALSE( matcher.Match( "test://items/%2/detail", variables ) );

            ASSERT_TRUE( matcher.Parse( "file:///{+path}", error ) );
            ASSERT_TRUE( matcher.Match( "file:///dir/sub/file%20name.txt", variables ) );
            ASSERT_TRUE( variables.TryGetValue( "path", value ) && value.AsString() == "dir/sub/file%20name.txt" );

            ASSERT_TRUE( matcher.Parse( "x://{a}-{+b}.json", error ) && 2 == matcher.GetVariableCount() );
            ASSERT_TRUE( matcher.Match( "x://one-two/three.json", variables ) );
            ASSERT_TRUE( variables.TryGetValue( "a", value ) && value.AsString() == "one" );
            ASSERT_TRUE( variables.TryGetValue( "b", value ) && value.AsString() == "two/three" );

            // an ambiguous template on a long URI stops at the step budget
            ASSERT_TRUE( matcher.Parse( "{+a}x{+b}x{+c}x{+d}y", error ) );
            CORE::CString longUri;
            for ( int i=0; i<2000; ++i )
                longUri += 'x';
            ASSERT_FALSE( matcher.Match( longUri, variables ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The resource registry resolves exact resources first, then templates in name order" )
        try
        {
            CORE::PulseGeneratorPtr pulseGenerator = MakeTestPulseGenerator();
            COM::CMcpResourceRegistry registry( pulseGenerator );
            CRegistryRecorder recorder( registry, COM::CMcpResourceRegistry::ResourceRegisteredEvent, COM::CMcpResourceRegistry::ResourceUnregisteredEvent );
            CORE::CString error;
            COM::CIMcpResourcePtr readme( GUCEF_NEW CMcpTestResource( "test://items/7", "seven" ) );
            ASSERT_TRUE( registry.RegisterResource( readme, error ) );
            ASSERT_FALSE( registry.RegisterResource( COM::CIMcpResourcePtr( GUCEF_NEW CMcpTestResource( "test://items/7", "other" ) ), error ) );
            ASSERT_FALSE( registry.RegisterResource( COM::CIMcpResourcePtr( GUCEF_NEW CMcpTestResource( "", "noname" ) ), error ) );
            ASSERT_FALSE( registry.RegisterResource( COM::CIMcpResourcePtr( GUCEF_NEW CMcpTestResource( "test://x", "" ) ), error ) );
            ASSERT_TRUE( registry.RegisterTemplate( COM::CIMcpResourceTemplatePtr( GUCEF_NEW CMcpTestTemplate( "b-items", "test://items/{id}" ) ), error ) );
            ASSERT_TRUE( registry.RegisterTemplate( COM::CIMcpResourceTemplatePtr( GUCEF_NEW CMcpTestTemplate( "a-any", "test://{+rest}" ) ), error ) );
            ASSERT_FALSE( registry.RegisterTemplate( COM::CIMcpResourceTemplatePtr( GUCEF_NEW CMcpTestTemplate( "bad", "test://{?q}" ) ), error ) );
            ASSERT_FALSE( registry.RegisterTemplate( COM::CIMcpResourceTemplatePtr( GUCEF_NEW CMcpTestTemplate( "a-any", "other://{x}" ) ), error ) );

            COM::CMcpResourceEntryPtr resource;
            COM::CMcpResourceTemplateEntryPtr resourceTemplate;
            CORE::CValueList variables;
            ASSERT_TRUE( COM::CMcpResourceRegistry::RESOLVE_RESOURCE == registry.Resolve( "test://items/7", resource, resourceTemplate, variables ) );
            ASSERT_TRUE( resource->descriptor.name == "seven" );
            ASSERT_TRUE( COM::CMcpResourceRegistry::RESOLVE_TEMPLATE == registry.Resolve( "test://items/8", resource, resourceTemplate, variables ) );
            ASSERT_TRUE( resourceTemplate->descriptor.name == "a-any" );        // the first template in name order
            ASSERT_TRUE( COM::CMcpResourceRegistry::RESOLVE_NONE == registry.Resolve( "other://items/8", resource, resourceTemplate, variables ) );

            COM::CMcpResourceRegistry::TTemplateEntryVector templates;
            registry.GetTemplates( templates );
            ASSERT_TRUE( 2 == templates.size() && templates[ 0 ]->descriptor.name == "a-any" );
            COM::CMcpResourceRegistry::TResourceEntryVector resources;
            registry.GetResources( resources );
            ASSERT_TRUE( 1 == resources.size() );

            ASSERT_TRUE( registry.UnregisterTemplate( "a-any" ) );
            ASSERT_TRUE( COM::CMcpResourceRegistry::RESOLVE_TEMPLATE == registry.Resolve( "test://items/8", resource, resourceTemplate, variables ) );
            ASSERT_TRUE( resourceTemplate->descriptor.name == "b-items" );
            ASSERT_TRUE( registry.UnregisterResource( "test://items/7" ) );
            ASSERT_FALSE( registry.UnregisterResource( "test://items/7" ) );
            ASSERT_TRUE( 1 == recorder.registeredNames.size() && 1 == recorder.unregisteredNames.size() );
            pulseGenerator->ForceDirectPulse();
            ASSERT_TRUE( 1 == recorder.listChanged );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The prompt registry refuses unnamed and twice named arguments" )
        try
        {
            CORE::PulseGeneratorPtr pulseGenerator = MakeTestPulseGenerator();
            COM::CMcpPromptRegistry registry( pulseGenerator );
            CORE::CString error;
            ASSERT_TRUE( registry.Register( COM::CIMcpPromptPtr( GUCEF_NEW CMcpTestPrompt( "zz" ) ), error ) );
            ASSERT_TRUE( registry.Register( COM::CIMcpPromptPtr( GUCEF_NEW CMcpTestPrompt( "aa" ) ), error ) );
            ASSERT_FALSE( registry.Register( COM::CIMcpPromptPtr( GUCEF_NEW CMcpTestPrompt( "aa" ) ), error ) );
            ASSERT_FALSE( registry.Register( COM::CIMcpPromptPtr( GUCEF_NEW CMcpTestPrompt( "" ) ), error ) );

            class CTwiceArgument : public CMcpTestPrompt
            {
                public:
                CTwiceArgument( void ) : CMcpTestPrompt( "twice" ) {}
                virtual bool GetDescriptor( COM::CMcpPromptDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
                {
                    CMcpTestPrompt::GetDescriptor( descriptor );
                    descriptor.AddArgument( "who", "again", false );
                    return true;
                }
            };
            ASSERT_FALSE( registry.Register( COM::CIMcpPromptPtr( GUCEF_NEW CTwiceArgument() ), error ) );
            COM::CMcpPromptRegistry::TPromptEntryVector entries;
            registry.GetEntries( entries );
            ASSERT_TRUE( 2 == entries.size() && entries[ 0 ]->descriptor.name == "aa" );
            ASSERT_TRUE( registry.Unregister( "aa" ) && 1 == registry.GetCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The nesting pre-scan counts outside strings only" )
        try
        {
            CORE::CString deep;
            for ( int i=0; i<100; ++i )
                deep += '[';
            ASSERT_TRUE( 65 == COM::CMcpChannel::GetJsonNestingDepth( deep.C_String(), deep.ByteSize() - 1, 64 ) );
            const char* quoted = "{\"a\":\"[[[[{{{{\\\"[[[[\",\"b\":[1,[2,[3]]]}";
            ASSERT_TRUE( 4 == COM::CMcpChannel::GetJsonNestingDepth( quoted, (UInt32) ::strlen( quoted ), 64 ) );
            ASSERT_TRUE( 0 == COM::CMcpChannel::GetJsonNestingDepth( "", 0, 64 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The request state sealer with HMAC-SHA256" )
        try
        {
            COM::CMcpRequestStateSealer sealer;
            CORE::CString error;
            CORE::CString::StringVector aeadTypes = COM::CMcpRequestStateSealer::DefaultAeadCodecTypes.ParseElements( ',', false );
            ASSERT_TRUE( sealer.Initialize( COM::CMcpRequestStateSealer::PROTECTION_HMAC_SHA256, aeadTypes, GUCEF_NULL, error ) );
            ASSERT_TRUE( COM::CMcpRequestStateSealer::PROTECTION_HMAC_SHA256 == sealer.GetProtection() );
            CheckSealer( sealer );

            // the HMAC form: the state can be read by the client, only its integrity is protected
            COM::CMcpRequestStateSealer::SBinding binding = MakeBinding( "s", "tools/call", "t", "{}" );
            CORE::CDataNode state;
            McpTestParse( "{\"visible\":\"marker\"}", state );
            CORE::CString sealed;
            ASSERT_TRUE( sealer.Seal( binding, state, false, NowMs() + 1000, sealed, error ) );
            ASSERT_TRUE( sealed.HasChar( '.' ) > 0 );

            // a seal of another key is refused, one of the same key from a file is accepted
            COM::CMcpRequestStateSealer other;
            ASSERT_TRUE( other.Initialize( COM::CMcpRequestStateSealer::PROTECTION_HMAC_SHA256, aeadTypes, GUCEF_NULL, error ) );
            CORE::CDataNode unsealed;
            CORE::CString nonce;
            ASSERT_TRUE( COM::CMcpRequestStateSealer::UNSEAL_TAMPERED == other.Unseal( sealed, binding, NowMs(), unsealed, nonce ) );

            CORE::CDynamicBuffer key;
            for ( UInt8 i=0; i<32; ++i )
                key.Append( &i, 1 );
            CORE::CDynamicBuffer shortKey;
            shortKey.Append( "abc", 3 );
            ASSERT_FALSE( other.Initialize( COM::CMcpRequestStateSealer::PROTECTION_HMAC_SHA256, aeadTypes, &shortKey, error ) );
            COM::CMcpRequestStateSealer first;
            COM::CMcpRequestStateSealer second;
            ASSERT_TRUE( first.Initialize( COM::CMcpRequestStateSealer::PROTECTION_HMAC_SHA256, aeadTypes, &key, error ) );
            ASSERT_TRUE( second.Initialize( COM::CMcpRequestStateSealer::PROTECTION_HMAC_SHA256, aeadTypes, &key, error ) );
            ASSERT_TRUE( first.Seal( binding, state, false, NowMs() + 1000, sealed, error ) );
            ASSERT_TRUE( COM::CMcpRequestStateSealer::UNSEAL_OK == second.Unseal( sealed, binding, NowMs(), unsealed, nonce ) );

            // key files: raw, base64url, hexadecimal
            CORE::CString path = CORE::RelativePath( "$CURWORKDIR$" );
            CORE::AppendToPath( path, "mcp_test_requeststate.key" );
            CORE::CDynamicBuffer loaded;
            ASSERT_TRUE( key.WriteContentToFile( path, 0, true ) );
            ASSERT_TRUE( COM::CMcpRequestStateSealer::LoadKeyFile( path, loaded, error ) && loaded == key );
            CORE::CString text;
            CORE::Base64UrlEncode( key.GetConstBufferPtr(), 32, text );
            CORE::CDynamicBuffer textFile;
            textFile.CopyFrom( text + "\n", 0, false );
            ASSERT_TRUE( textFile.WriteContentToFile( path, 0, true ) );
            ASSERT_TRUE( COM::CMcpRequestStateSealer::LoadKeyFile( path, loaded, error ) && loaded == key );
            textFile.SetDataSize( 0 );
            textFile.CopyFrom( CORE::CString( "000102030405060708090a0b0c0d0e0f101112131415161718191A1B1C1D1E1F" ), 0, false );
            ASSERT_TRUE( textFile.WriteContentToFile( path, 0, true ) );
            ASSERT_TRUE( COM::CMcpRequestStateSealer::LoadKeyFile( path, loaded, error ) && loaded == key );
            textFile.SetDataSize( 0 );
            textFile.CopyFrom( CORE::CString( "too short" ), 0, false );
            ASSERT_TRUE( textFile.WriteContentToFile( path, 0, true ) );
            ASSERT_FALSE( COM::CMcpRequestStateSealer::LoadKeyFile( path, loaded, error ) );
            CORE::DeleteFile( path );
            ASSERT_FALSE( COM::CMcpRequestStateSealer::LoadKeyFile( path, loaded, error ) );

            COM::CMcpRequestStateSealer::EProtection protection;
            ASSERT_TRUE( COM::CMcpRequestStateSealer::ParseProtection( "AUTO", protection ) && COM::CMcpRequestStateSealer::PROTECTION_AUTO == protection );
            ASSERT_TRUE( COM::CMcpRequestStateSealer::ParseProtection( "aead", protection ) && COM::CMcpRequestStateSealer::PROTECTION_AEAD == protection );
            ASSERT_FALSE( COM::CMcpRequestStateSealer::ParseProtection( "rot13", protection ) );

            // not initialized
            COM::CMcpRequestStateSealer unready;
            ASSERT_FALSE( unready.Seal( binding, state, false, NowMs() + 1000, sealed, error ) );
            ASSERT_TRUE( COM::CMcpRequestStateSealer::UNSEAL_NOT_READY == unready.Unseal( "AAAA.AAAA", binding, NowMs(), unsealed, nonce ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The request state sealer with an AEAD codec hides the state" )
        GUCEF_TESTFW_SKIP_IF( !HasAeadCodec(), "no AEAD codec is registered (no OS provider, no codecspluginMONOCYPHER)" );
        try
        {
            COM::CMcpRequestStateSealer sealer;
            CORE::CString error;
            CORE::CString::StringVector aeadTypes = COM::CMcpRequestStateSealer::DefaultAeadCodecTypes.ParseElements( ',', false );
            ASSERT_TRUE( sealer.Initialize( COM::CMcpRequestStateSealer::PROTECTION_AEAD, aeadTypes, GUCEF_NULL, error ) );
            ASSERT_TRUE( COM::CMcpRequestStateSealer::PROTECTION_AEAD == sealer.GetProtection() );
            ASSERT_FALSE( sealer.GetAeadCodecType().IsNULLOrEmpty() );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "McpRegistry tests: the AEAD codec in use is " << sealer.GetAeadCodecType() );
            CheckSealer( sealer );

            COM::CMcpRequestStateSealer::SBinding binding = MakeBinding( "s", "tools/call", "t", "{}" );
            CORE::CDataNode state;
            McpTestParse( "{\"visible\":\"marker\"}", state );
            CORE::CString sealed;
            ASSERT_TRUE( sealer.Seal( binding, state, false, NowMs() + 1000, sealed, error ) );
            CORE::CDynamicBuffer decoded;
            ASSERT_TRUE( CORE::Base64UrlDecode( sealed, decoded ) );
            CORE::CString raw( static_cast< const char* >( decoded.GetConstBufferPtr() ), decoded.GetDataSize() );
            ASSERT_TRUE( raw.HasSubstr( "marker" ) < 0 );
            ASSERT_TRUE( sealed.HasChar( '.' ) < 0 );

            // auto picks AEAD when it is there; an HMAC seal is not accepted by an AEAD sealer
            COM::CMcpRequestStateSealer automatic;
            ASSERT_TRUE( automatic.Initialize( COM::CMcpRequestStateSealer::PROTECTION_AUTO, aeadTypes, GUCEF_NULL, error ) );
            ASSERT_TRUE( COM::CMcpRequestStateSealer::PROTECTION_AEAD == automatic.GetProtection() );
            COM::CMcpRequestStateSealer hmac;
            ASSERT_TRUE( hmac.Initialize( COM::CMcpRequestStateSealer::PROTECTION_HMAC_SHA256, aeadTypes, GUCEF_NULL, error ) );
            ASSERT_TRUE( hmac.Seal( binding, state, false, NowMs() + 1000, sealed, error ) );
            CORE::CDataNode unsealed;
            CORE::CString nonce;
            ASSERT_TRUE( COM::CMcpRequestStateSealer::UNSEAL_MALFORMED == automatic.Unseal( sealed, binding, NowMs(), unsealed, nonce ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Asking for AEAD without an AEAD codec type is refused, auto falls back to HMAC" )
        try
        {
            COM::CMcpRequestStateSealer sealer;
            CORE::CString error;
            CORE::CString::StringVector none;
            none.push_back( "rot13-poly1305" );
            ASSERT_FALSE( sealer.Initialize( COM::CMcpRequestStateSealer::PROTECTION_AEAD, none, GUCEF_NULL, error ) );
            ASSERT_FALSE( error.IsNULLOrEmpty() );
            ASSERT_FALSE( sealer.IsInitialized() );
            ASSERT_TRUE( sealer.Initialize( COM::CMcpRequestStateSealer::PROTECTION_AUTO, none, GUCEF_NULL, error ) );
            ASSERT_TRUE( COM::CMcpRequestStateSealer::PROTECTION_HMAC_SHA256 == sealer.GetProtection() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
