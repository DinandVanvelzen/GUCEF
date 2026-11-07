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

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_COM_CMCPCONFIGURABLEACCESSPOLICY_H
#include "gucefCOM_CMcpConfigurableAccessPolicy.h"
#define GUCEF_COM_CMCPCONFIGURABLEACCESSPOLICY_H
#endif /* GUCEF_COM_CMCPCONFIGURABLEACCESSPOLICY_H ? */

#ifndef GUCEF_COM_CMCPJSONLINESAUDITLOG_H
#include "gucefCOM_CMcpJsonLinesAuditLog.h"
#define GUCEF_COM_CMCPJSONLINESAUDITLOG_H
#endif /* GUCEF_COM_CMCPJSONLINESAUDITLOG_H ? */

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #include <sys/stat.h>
#endif

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#include "TestMcpSupport.h"
#include "TestMcpPolicy.h"

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

/** Keeps the audit records of a server, and the order in which audit and request events came */
class CAuditRecorder : public CORE::CObserver
{
    public:

    CAuditRecorder( COM::CMcpServer& server )
        : CORE::CObserver(), records(), events()
    {
        typedef CORE::CTEventHandlerFunctor< CAuditRecorder > TCallback;
        TCallback onAudit( this, &CAuditRecorder::OnAudit );
        SubscribeTo( &server, COM::CMcpServer::AuditEvent, onAudit );
        TCallback onFinished( this, &CAuditRecorder::OnFinished );
        SubscribeTo( &server, COM::CMcpServer::RequestFinishedEvent, onFinished );
    }

    virtual ~CAuditRecorder()
    {
        UnsubscribeAllFromObserver();
    }

    void OnAudit( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        const COM::TMcpAuditEventData* data = static_cast< const COM::TMcpAuditEventData* >( eventData );
        if ( GUCEF_NULL != data )
            records.push_back( data->GetData() );
        events += 'A';
    }

    void OnFinished( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
    {
        events += 'F';
    }

    /** The last record of a method */
    const COM::CMcpAuditRecord* Last( const char* method ) const
    {
        for ( size_t i=records.size(); i>0; --i )
            if ( records[ i-1 ].method == method )
                return &records[ i-1 ];
        return GUCEF_NULL;
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CAuditRecorder";
        return classTypeName;
    }

    GUCEF::vector< COM::CMcpAuditRecord > records;
    CORE::CString events;
};

/** A tool of which the argument "text" is secret */
class CSecretTool : public CMcpTestTool
{
    public:

    CSecretTool( const char* name ) : CMcpTestTool( name, CMcpTestTool::ECHO ) {}

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        CMcpTestTool::GetDescriptor( descriptor );
        descriptor.secretArguments.insert( "text" );
        return true;
    }
};

bool
RegisterTool( CMcpTestRig& rig, CMcpTestTool* tool )
{
    CORE::CString error;
    return rig.server->GetToolRegistry().Register( COM::CIMcpToolPtr( tool ), error );
}

/** reader (read-only), adder (additive) and remover (destructive) */
void
RegisterThreeTools( CMcpTestRig& rig )
{
    RegisterTool( rig, GUCEF_NEW CMcpTestTool( "reader", CMcpTestTool::ECHO, COM::MCP_EXECUTION_CALLING_THREAD, COM::MCP_TOOLEFFECT_READ_ONLY ) );
    RegisterTool( rig, GUCEF_NEW CMcpTestTool( "adder", CMcpTestTool::ECHO, COM::MCP_EXECUTION_CALLING_THREAD, COM::MCP_TOOLEFFECT_ADDITIVE ) );
    RegisterTool( rig, GUCEF_NEW CMcpTestTool( "remover", CMcpTestTool::ECHO, COM::MCP_EXECUTION_CALLING_THREAD, COM::MCP_TOOLEFFECT_DESTRUCTIVE ) );
}

CORE::CString
ToolNames( CMcpTestRig& rig, const char* id, CMcpTestChannel* target = GUCEF_NULL, CORE::CString* cacheScope = GUCEF_NULL )
{
    CORE::CDataNode response;
    if ( !rig.Call( id, McpTestRequest( id, "tools/list" ), response, 5000, target ) )
        return "<no answer>";
    if ( GUCEF_NULL != cacheScope )
        *cacheScope = McpTestResultAttr( response, "cacheScope" );
    CORE::CString names;
    const CORE::CDataNode* tools = McpTestResultChild( response, "tools" );
    if ( GUCEF_NULL == tools )
        return "<none>";
    for ( CORE::CDataNode::const_iterator i = tools->ConstBegin(); i != tools->ConstEnd(); ++i )
    {
        if ( !names.IsNULLOrEmpty() )
            names += ',';
        names += McpTestAttr( *(*i), "name" );
    }
    return names;
}

/** Calls a tool with a text; the response */
bool
CallTool( CMcpTestRig& rig, const char* id, const char* tool, const char* text, CORE::CDataNode& response, CMcpTestChannel* target = GUCEF_NULL )
{
    return rig.Call( id, McpTestRequest( id, "tools/call", CORE::CString( "\"name\":\"" ) + tool + "\",\"arguments\":{\"text\":\"" + text + "\"}" ), response, 5000, target );
}

CORE::CString
ResourceUris( CMcpTestRig& rig, const char* id )
{
    CORE::CDataNode response;
    if ( !rig.Call( id, McpTestRequest( id, "resources/list" ), response ) )
        return "<no answer>";
    CORE::CString uris;
    const CORE::CDataNode* resources = McpTestResultChild( response, "resources" );
    if ( GUCEF_NULL == resources )
        return "<none>";
    for ( CORE::CDataNode::const_iterator i = resources->ConstBegin(); i != resources->ConstEnd(); ++i )
    {
        if ( !uris.IsNULLOrEmpty() )
            uris += ',';
        uris += McpTestAttr( *(*i), "uri" );
    }
    return uris;
}

bool
ReadResource( CMcpTestRig& rig, const char* id, const char* uri, CORE::CDataNode& response )
{
    return rig.Call( id, McpTestRequest( id, "resources/read", CORE::CString( "\"uri\":\"" ) + uri + "\"" ), response );
}

CORE::CString
Sha256Hex( const CORE::CString& text )
{
    UInt8 digest[ 32 ];
    CORE::CSha256::Hash( text.C_String(), text.Length(), digest );
    return COM::CMcpAuditRecord::DigestToHex( digest );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMcpPolicyTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING MCP POLICY AND AUDIT TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "McpPolicy" );

    if ( !McpTestJsonAvailable() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MCP TESTS skipped: the json codec is not available" );
        return;
    }

    GUCEF_TESTFW_TESTCASE( "Test: The defaults are closed: read-only tools are allowed, the others are hidden and refused as unknown" )
        try
        {
            CMcpTestRig rig;
            RegisterThreeTools( rig );

            // the server's own policy is the configurable one with its defaults
            CORE::CString cacheScope;
            ASSERT_TRUE( ToolNames( rig, "1", GUCEF_NULL, &cacheScope ) == "reader" );
            ASSERT_TRUE( cacheScope == "public" );

            CORE::CDataNode response;
            ASSERT_TRUE( CallTool( rig, "2", "reader", "hi", response ) );
            ASSERT_TRUE( McpTestFirstText( response.FindChild( "result" ) ) == "echo: hi" );
            ASSERT_TRUE( CallTool( rig, "3", "adder", "hi", response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( McpTestErrorMessage( response ) == "Unknown tool: adder" );
            ASSERT_TRUE( CallTool( rig, "4", "remover", "hi", response ) && -32602 == McpTestErrorCode( response ) );

            // the in-process calls are asked too
            COM::CMcpToolResult result;
            CORE::CString error;
            CORE::CDataNode arguments( GUCEF_DATATYPE_OBJECT );
            arguments.SetAttribute( "text", "x" );
            ASSERT_TRUE( rig.server->CallTool( "reader", arguments, COM::CMcpPrincipal(), result, error ) );
            ASSERT_FALSE( rig.server->CallTool( "adder", arguments, COM::CMcpPrincipal(), result, error ) );
            ASSERT_TRUE( error == "Unknown tool: adder" );

            // resources, prompts and subscriptions are allowed by default
            rig.server->GetResourceRegistry().RegisterResource( COM::CIMcpResourcePtr( GUCEF_NEW CMcpTestResource( "test://docs/a", "a" ) ), error );
            ASSERT_TRUE( ResourceUris( rig, "5" ) == "test://docs/a" );
            ASSERT_TRUE( rig.AllSentAreJsonLines() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A write tool is switched on by name, a deny wins over everything" )
        try
        {
            CMcpTestRig rig;
            RegisterThreeTools( rig );
            COM::CMcpConfigurableAccessPolicy* policy = GUCEF_NEW COM::CMcpConfigurableAccessPolicy();
            policy->GetDefaultRules().AddAllowTool( "add*" );
            rig.server->SetAccessPolicy( COM::CIMcpAccessPolicyPtr( policy ) );
            ASSERT_TRUE( ToolNames( rig, "1" ) == "adder,reader" );

            CORE::CDataNode response;
            ASSERT_TRUE( CallTool( rig, "2", "adder", "hi", response ) );
            ASSERT_TRUE( GUCEF_NULL != response.FindChild( "result" ) && McpTestFirstText( response.FindChild( "result" ) ) == "echo: hi" );
            ASSERT_TRUE( CallTool( rig, "3", "remover", "hi", response ) && -32602 == McpTestErrorCode( response ) );

            // alternatives of a glob, and everything allowed with a deny for one
            COM::CMcpConfigurableAccessPolicy* second = GUCEF_NEW COM::CMcpConfigurableAccessPolicy();
            second->GetDefaultRules().AddAllowTool( "{adder,remover}" );
            second->GetDefaultRules().AddDenyTool( "remover" );
            rig.server->SetAccessPolicy( COM::CIMcpAccessPolicyPtr( second ) );
            ASSERT_TRUE( ToolNames( rig, "4" ) == "adder,reader" );

            // a deny also takes a read-only tool away
            COM::CMcpConfigurableAccessPolicy* third = GUCEF_NEW COM::CMcpConfigurableAccessPolicy();
            third->GetDefaultRules().AddAllowTool( "*" );
            third->GetDefaultRules().AddDenyTool( "read*" );
            rig.server->SetAccessPolicy( COM::CIMcpAccessPolicyPtr( third ) );
            ASSERT_TRUE( ToolNames( rig, "5" ) == "adder,remover" );
            ASSERT_TRUE( CallTool( rig, "6", "reader", "hi", response ) && -32602 == McpTestErrorCode( response ) );

            // no read-only tools unless named
            COM::CMcpConfigurableAccessPolicy* fourth = GUCEF_NEW COM::CMcpConfigurableAccessPolicy();
            fourth->GetDefaultRules().SetAllowReadOnlyTools( false );
            rig.server->SetAccessPolicy( COM::CIMcpAccessPolicyPtr( fourth ) );
            ASSERT_TRUE( ToolNames( rig, "7" ).IsNULLOrEmpty() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A denied tool can be listed and refused with a reason the model can read" )
        try
        {
            CMcpTestRig rig;
            RegisterThreeTools( rig );
            COM::CMcpConfigurableAccessPolicy* policy = GUCEF_NEW COM::CMcpConfigurableAccessPolicy();
            policy->GetDefaultRules().SetHideDenied( false );
            rig.server->SetAccessPolicy( COM::CIMcpAccessPolicyPtr( policy ) );
            CAuditRecorder audit( *rig.server );

            ASSERT_TRUE( ToolNames( rig, "1" ) == "adder,reader,remover" );
            CORE::CDataNode response;
            ASSERT_TRUE( CallTool( rig, "2", "adder", "hi", response ) );
            const CORE::CDataNode* result = response.FindChild( "result" );
            ASSERT_TRUE( GUCEF_NULL != result && McpTestAttr( *result, "isError" ).Lowercase() == "true" );
            ASSERT_TRUE( McpTestFirstText( result ).HasSubstr( "Access denied" ) >= 0 );
            ASSERT_TRUE( 0 == McpTestErrorCode( response ) );

            // the denial is in the audit trail with the decision and the reason, and the work never ran
            const COM::CMcpAuditRecord* record = audit.Last( "tools/call" );
            ASSERT_TRUE( GUCEF_NULL != record );
            if ( GUCEF_NULL != record )
            {
                ASSERT_TRUE( record->hasDecision && COM::MCP_ACCESS_DENY == record->decision );
                ASSERT_TRUE( !record->decisionReason.IsNULLOrEmpty() );
                ASSERT_TRUE( COM::MCP_OUTCOME_TOOL_ERROR == record->outcome );
                ASSERT_TRUE( record->target == "adder" );
            }

            // a read-only tool is not denied
            ASSERT_TRUE( CallTool( rig, "3", "reader", "hi", response ) && McpTestFirstText( response.FindChild( "result" ) ) == "echo: hi" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Rules per principal: lists become private, the rules replace, the denies only add up" )
        try
        {
            CMcpTestRig rig( COM::CMcpChannel::CHANNEL_STREAM, "local:admin" );
            RegisterThreeTools( rig );
            COM::CMcpConfigurableAccessPolicy* policy = GUCEF_NEW COM::CMcpConfigurableAccessPolicy();
            policy->GetDefaultRules().AddDenyTool( "remover" );
            COM::CMcpAccessRules* agents = policy->AddPrincipalRules( "local:agent*" );
            ASSERT_TRUE( GUCEF_NULL != agents );
            if ( GUCEF_NULL == agents )
                return;
            agents->SetAllowReadOnlyTools( false );
            agents->AddAllowTool( "adder" );
            agents->AddAllowTool( "remover" );          // a deny of the defaults is not taken away
            ASSERT_TRUE( policy->IsVisibilityPrincipalDependent() );
            ASSERT_TRUE( GUCEF_NULL == policy->AddPrincipalRules( "local:[agent" ) );
            rig.server->SetAccessPolicy( COM::CIMcpAccessPolicyPtr( policy ) );

            CMcpTestChannel* agent = rig.AddChannel( COM::CMcpChannel::CHANNEL_STREAM, "local:agent-7" );
            CORE::CString cacheScope;
            ASSERT_TRUE( ToolNames( rig, "1", GUCEF_NULL, &cacheScope ) == "reader" );
            ASSERT_TRUE( cacheScope == "private" );
            ASSERT_TRUE( ToolNames( rig, "2", agent, &cacheScope ) == "adder" );
            ASSERT_TRUE( cacheScope == "private" );

            CORE::CDataNode response;
            ASSERT_TRUE( CallTool( rig, "3", "adder", "hi", response, agent ) && GUCEF_NULL != response.FindChild( "result" ) );
            ASSERT_TRUE( CallTool( rig, "4", "reader", "hi", response, agent ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( CallTool( rig, "5", "remover", "hi", response, agent ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( CallTool( rig, "6", "adder", "hi", response ) && -32602 == McpTestErrorCode( response ) );

            // an in-process caller has a principal too
            COM::CMcpToolResult result;
            CORE::CString error;
            CORE::CDataNode arguments( GUCEF_DATATYPE_OBJECT );
            arguments.SetAttribute( "text", "x" );
            ASSERT_TRUE( rig.server->CallTool( "adder", arguments, COM::CMcpPrincipal( "in-process", "local:agent-1", true ), result, error ) );
            ASSERT_FALSE( rig.server->CallTool( "adder", arguments, COM::CMcpPrincipal( "in-process", "local:other", true ), result, error ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Resources, prompts and subscriptions: allow and deny lists, hiding or refusing, a URI star crosses slashes" )
        try
        {
            CMcpTestRig rig;
            CORE::CString error;
            const char* uris[] = { "test://docs/a", "test://docs/sub/b", "test://secret/c" };
            for ( int i=0; i<3; ++i )
                rig.server->GetResourceRegistry().RegisterResource( COM::CIMcpResourcePtr( GUCEF_NEW CMcpTestResource( uris[ i ], "r" ) ), error );
            rig.server->GetPromptRegistry().Register( COM::CIMcpPromptPtr( GUCEF_NEW CMcpTestPrompt( "greet" ) ), error );
            rig.server->GetPromptRegistry().Register( COM::CIMcpPromptPtr( GUCEF_NEW CMcpTestPrompt( "internal" ) ), error );

            COM::CMcpConfigurableAccessPolicy* policy = GUCEF_NEW COM::CMcpConfigurableAccessPolicy();
            policy->GetDefaultRules().ClearAllowResources();
            policy->GetDefaultRules().AddAllowResource( "test://docs/*" );
            policy->GetDefaultRules().AddAllowResource( "test://secret/*" );
            policy->GetDefaultRules().AddDenyResource( "test://secret/**" );
            policy->GetDefaultRules().ClearAllowPrompts();
            policy->GetDefaultRules().AddAllowPrompt( "g*" );
            policy->GetDefaultRules().SetAllowSubscriptions( false );
            rig.server->SetAccessPolicy( COM::CIMcpAccessPolicyPtr( policy ) );

            ASSERT_TRUE( ResourceUris( rig, "1" ) == "test://docs/a,test://docs/sub/b" );
            CORE::CDataNode response;
            ASSERT_TRUE( ReadResource( rig, "2", "test://docs/sub/b", response ) && GUCEF_NULL != response.FindChild( "result" ) );
            ASSERT_TRUE( ReadResource( rig, "3", "test://secret/c", response ) && -32602 == McpTestErrorCode( response ) );

            ASSERT_TRUE( rig.Call( "4", McpTestRequest( "4", "prompts/list" ), response ) );
            const CORE::CDataNode* prompts = McpTestResultChild( response, "prompts" );
            ASSERT_TRUE( 1 == McpTestChildCount( prompts ) );
            ASSERT_TRUE( rig.Call( "5", McpTestRequest( "5", "prompts/get", "\"name\":\"internal\",\"arguments\":{\"who\":\"x\"}" ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "6", McpTestRequest( "6", "prompts/get", "\"name\":\"greet\",\"arguments\":{\"who\":\"x\"}" ), response ) && GUCEF_NULL != response.FindChild( "result" ) );

            ASSERT_TRUE( rig.Call( "7", McpTestRequest( "7", "subscriptions/listen", "\"notifications\":{\"toolsListChanged\":true}" ), response ) );
            ASSERT_TRUE( -31003 == McpTestErrorCode( response ) );

            // listed and refused with the access denied error instead
            policy->GetDefaultRules().SetHideDenied( false );
            ASSERT_TRUE( ResourceUris( rig, "8" ) == "test://docs/a,test://docs/sub/b,test://secret/c" );
            ASSERT_TRUE( ReadResource( rig, "9", "test://secret/c", response ) && -31003 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.Call( "10", McpTestRequest( "10", "prompts/get", "\"name\":\"internal\",\"arguments\":{\"who\":\"x\"}" ), response ) && -31003 == McpTestErrorCode( response ) );

            // in-process
            COM::CMcpResourceContents contents;
            ASSERT_TRUE( rig.server->ReadResource( "test://docs/a", COM::CMcpPrincipal(), contents, error ) );
            ASSERT_FALSE( rig.server->ReadResource( "test://secret/c", COM::CMcpPrincipal(), contents, error ) );
            ASSERT_TRUE( rig.AllSentAreJsonLines() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The policy is configured from a configuration, saved, and refuses a configuration with a bad pattern" )
        try
        {
            CORE::CDataNode config;
            ASSERT_TRUE( McpTestParse( "{\"McpAccessPolicy\":{\"allowReadOnlyTools\":true,\"allowTools\":[\"adder\",\"x{1,2}\"],\"denyTools\":\"remover;other\","
                                       "\"hideDenied\":false,\"allowResources\":[\"test://docs/*\"],\"denyPrompts\":[\"internal\"],"
                                       "\"principals\":[{\"subject\":\"local:agent*\",\"allowReadOnlyTools\":false,\"denyTools\":[\"adder\"]}]}}", config ) );

            COM::CMcpConfigurableAccessPolicy policy;
            ASSERT_TRUE( policy.LoadConfig( config ) );
            ASSERT_TRUE( policy.IsVisibilityPrincipalDependent() );
            const COM::CMcpAccessRules& rules = policy.GetDefaultRules();
            ASSERT_TRUE( rules.GetAllowReadOnlyTools() && !rules.GetHideDenied() && rules.GetAllowSubscriptions() );
            ASSERT_TRUE( rules.GetAllowTools().Matches( "adder" ) && rules.GetAllowTools().Matches( "x2" ) && !rules.GetAllowTools().Matches( "x3" ) );
            ASSERT_TRUE( rules.GetDenyTools().Matches( "remover" ) && rules.GetDenyTools().Matches( "other" ) );
            ASSERT_TRUE( rules.GetAllowResources().Matches( "test://docs/a/b" ) && !rules.GetAllowResources().Matches( "test://other/a" ) );
            ASSERT_TRUE( rules.GetDenyPrompts().Matches( "internal" ) && rules.GetAllowPrompts().Matches( "anything" ) );

            // saved and loaded again it decides the same
            CORE::CDataNode saved;
            ASSERT_TRUE( policy.SaveConfig( saved ) );
            CORE::CString text;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ToJsonText( saved, text ) );
            CORE::CDataNode reloaded;
            ASSERT_TRUE( McpTestParse( text, reloaded ) );
            COM::CMcpConfigurableAccessPolicy again;
            ASSERT_TRUE( again.LoadConfig( reloaded ) );
            ASSERT_TRUE( again.IsVisibilityPrincipalDependent() );
            ASSERT_TRUE( again.GetDefaultRules().GetAllowTools().Matches( "x1" ) && again.GetDefaultRules().GetDenyTools().Matches( "other" ) );
            ASSERT_TRUE( again.GetDefaultRules().GetAllowResources().Matches( "test://docs/a/b" ) );
            ASSERT_FALSE( again.GetDefaultRules().GetHideDenied() );

            // a bad pattern, an entry without a subject: refused, and nothing changes
            CORE::CDataNode bad;
            ASSERT_TRUE( McpTestParse( "{\"McpAccessPolicy\":{\"allowTools\":[\"[abc\"]}}", bad ) );
            ASSERT_FALSE( again.LoadConfig( bad ) );
            ASSERT_TRUE( again.GetDefaultRules().GetAllowTools().Matches( "x1" ) );
            ASSERT_TRUE( McpTestParse( "{\"McpAccessPolicy\":{\"principals\":[{\"allowTools\":[\"a\"]}]}}", bad ) );
            ASSERT_FALSE( again.LoadConfig( bad ) );
            ASSERT_TRUE( McpTestParse( "{\"McpAccessPolicy\":{\"principals\":[{\"subject\":\"[x\"}]}}", bad ) );
            ASSERT_FALSE( again.LoadConfig( bad ) );
            ASSERT_TRUE( again.IsVisibilityPrincipalDependent() );

            // copies decide the same and are independent
            COM::CMcpConfigurableAccessPolicy copy( again );
            copy.GetDefaultRules().AddDenyTool( "x1" );
            ASSERT_TRUE( copy.GetDefaultRules().GetDenyTools().Matches( "x1" ) && !again.GetDefaultRules().GetDenyTools().Matches( "x1" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Audit: one record per request with who, what, outcome and the digest of the arguments" )
        try
        {
            CMcpTestRig rig;
            RegisterThreeTools( rig );
            CAuditRecorder audit( *rig.server );

            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "server/discover" ), response ) );
            ASSERT_TRUE( CallTool( rig, "5", "reader", "hi", response ) );
            ASSERT_TRUE( CallTool( rig, "6", "adder", "hi", response ) );
            ASSERT_TRUE( rig.Call( "7", McpTestRequest( "7", "tools/call", "\"name\":\"reader\",\"arguments\":{\"text\":1}" ), response ) );     // fails the schema

            ASSERT_TRUE( 4 == audit.records.size() );
            // the record is raised right after the finished event of its request
            ASSERT_TRUE( audit.events == "FAFAFAFA" );

            const COM::CMcpAuditRecord* discover = audit.Last( "server/discover" );
            ASSERT_TRUE( GUCEF_NULL != discover && !discover->hasArguments && discover->argumentsDigest.IsNULLOrEmpty() );
            if ( GUCEF_NULL != discover )
                ASSERT_TRUE( COM::MCP_OUTCOME_COMPLETE == discover->outcome && !discover->hasDecision );

            // the first call of a tool: the digest is the SHA-256 of the JSON text of the arguments, the same text the endpoint writes
            const COM::CMcpAuditRecord first = audit.records[ 1 ];     // a copy: the vector grows below
            ASSERT_TRUE( "tools/call" == first.method && "reader" == first.target );
            ASSERT_TRUE( "test" == first.transport && "local:test" == first.subject && first.isAuthenticated );
            ASSERT_TRUE( "5" == first.requestId && first.channelId > 0 );
            ASSERT_TRUE( COM::MCP_ERA_MODERN == first.era && "2026-07-28" == first.protocolVersion );
            ASSERT_TRUE( "test-client" == first.clientName && "1.0" == first.clientVersion );
            ASSERT_TRUE( COM::MCP_OUTCOME_COMPLETE == first.outcome && 0 == first.errorCode );
            ASSERT_TRUE( first.hasDecision && COM::MCP_ACCESS_ALLOW == first.decision );
            ASSERT_TRUE( first.hasArguments && first.argumentsDigest.Length() == 64 );
            ASSERT_TRUE( first.argumentsDigest == Sha256Hex( "{\"text\":\"hi\"}" ) );
            ASSERT_TRUE( first.argumentsPreview == "{\"text\":\"hi\"}" );
            ASSERT_TRUE( first.timestampUtc.GetYear() >= 2025 );

            // hidden: a protocol error, with the decision
            const COM::CMcpAuditRecord hidden = audit.records[ 2 ];
            ASSERT_TRUE( "adder" == hidden.target && COM::MCP_OUTCOME_PROTOCOL_ERROR == hidden.outcome && -32602 == hidden.errorCode );
            ASSERT_TRUE( hidden.hasDecision && COM::MCP_ACCESS_HIDE == hidden.decision );

            // refused by the schema: a tool error, still with the digest of what came in
            const COM::CMcpAuditRecord invalid = audit.records[ 3 ];
            ASSERT_TRUE( COM::MCP_OUTCOME_TOOL_ERROR == invalid.outcome );
            ASSERT_TRUE( invalid.argumentsDigest == Sha256Hex( "{\"text\":1}" ) );

            // the same arguments always give the same digest
            ASSERT_TRUE( CallTool( rig, "8", "reader", "hi", response ) );
            ASSERT_TRUE( audit.records.back().argumentsDigest == first.argumentsDigest );

            // an in-process call is audited too
            COM::CMcpToolResult result;
            CORE::CString error;
            CORE::CDataNode arguments( GUCEF_DATATYPE_OBJECT );
            arguments.SetAttribute( "text", "in" );
            ASSERT_TRUE( rig.server->CallTool( "reader", arguments, COM::CMcpPrincipal(), result, error ) );
            ASSERT_TRUE( audit.records.back().transport == "in-process" && audit.records.back().requestId.IsNULLOrEmpty() && 0 == audit.records.back().channelId );
            ASSERT_TRUE( 6 == audit.records.size() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Audit: the preview is redacted, secret arguments are masked, the raw arguments are nowhere" )
        try
        {
            CMcpTestRig rig;
            RegisterTool( rig, GUCEF_NEW CSecretTool( "vault" ) );
            RegisterTool( rig, GUCEF_NEW CMcpTestTool( "reader", CMcpTestTool::ECHO ) );
            CAuditRecorder audit( *rig.server );

            CORE::CDataNode response;
            ASSERT_TRUE( CallTool( rig, "1", "vault", "hunter2", response ) );
            ASSERT_TRUE( McpTestFirstText( response.FindChild( "result" ) ) == "echo: hunter2" );
            const COM::CMcpAuditRecord* record = audit.Last( "tools/call" );
            ASSERT_TRUE( GUCEF_NULL != record );
            if ( GUCEF_NULL != record )
            {
                ASSERT_TRUE( record->argumentsPreview.HasSubstr( "hunter2" ) < 0 );
                ASSERT_TRUE( record->argumentsPreview.HasSubstr( "***" ) >= 0 );
                // the digest is of the real arguments
                ASSERT_TRUE( record->argumentsDigest == Sha256Hex( "{\"text\":\"hunter2\"}" ) );
                CORE::CString line;
                ASSERT_TRUE( record->ToJsonLine( line ) );
                ASSERT_TRUE( line.HasSubstr( "hunter2" ) < 0 && line.HasChar( '\n' ) < 0 );
            }

            // not a secret by name, but it looks like one: the redactor has it
            ASSERT_TRUE( CallTool( rig, "2", "reader", "password=hunter3 and Authorization: Bearer abcdef0123456789abcdef0123456789", response ) );
            record = audit.Last( "tools/call" );
            ASSERT_TRUE( GUCEF_NULL != record );
            if ( GUCEF_NULL != record )
            {
                ASSERT_TRUE( record->argumentsPreview.HasSubstr( "hunter3" ) < 0 );
                ASSERT_TRUE( record->argumentsPreview.HasSubstr( "abcdef0123456789abcdef0123456789" ) < 0 );
                ASSERT_TRUE( record->argumentsPreview.HasSubstr( "password" ) >= 0 );
            }

            // the preview is limited
            CORE::CString longText;
            for ( UInt32 i=0; i<50; ++i )
                longText += "word word ";
            COM::CMcpToolResult result;
            CORE::CString error;
            CORE::CDataNode arguments( GUCEF_DATATYPE_OBJECT );
            arguments.SetAttribute( "text", CORE::CVariant( longText + longText + longText ) );
            ASSERT_TRUE( rig.server->CallTool( "reader", arguments, COM::CMcpPrincipal(), result, error ) || !error.IsNULLOrEmpty() );
            ASSERT_TRUE( audit.records.back().argumentsPreview.Length() <= COM::CMcpAuditRecord::MaxArgumentsPreviewBytes );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Audit: a rate limited call and a cancelled call are audited, the limit is an MCP error" )
        try
        {
            CMcpTestRig rig;
            CMcpTestTool* slow = GUCEF_NEW CMcpTestTool( "slow", CMcpTestTool::COUNT, COM::MCP_EXECUTION_THREAD_POOL );
            slow->steps = 400;
            slow->stepMs = 5;
            RegisterTool( rig, slow );
            RegisterTool( rig, GUCEF_NEW CMcpTestTool( "reader", CMcpTestTool::ECHO ) );
            CAuditRecorder audit( *rig.server );

            // the defaults of the rate limit
            ASSERT_TRUE( 10 == rig.server->GetSettings().toolCallsPerSecond && 20 == rig.server->GetSettings().toolCallBurst );
            ASSERT_TRUE( 4194304 == rig.server->GetSettings().maxMessageBytes && rig.server->GetSettings().stopOnStdinClosed && rig.server->GetSettings().validateStructuredOutput );

            COM::CMcpServerSettings settings = rig.server->GetSettings();
            settings.toolCallsPerSecond = 1;
            settings.toolCallBurst = 2;
            rig.SetSettings( settings );

            CORE::CDataNode response;
            ASSERT_TRUE( CallTool( rig, "1", "reader", "a", response ) && GUCEF_NULL != response.FindChild( "result" ) );
            ASSERT_TRUE( CallTool( rig, "2", "reader", "b", response ) && GUCEF_NULL != response.FindChild( "result" ) );
            ASSERT_TRUE( CallTool( rig, "3", "reader", "c", response ) && -31001 == McpTestErrorCode( response ) );
            ASSERT_TRUE( rig.AllSentAreJsonLines() );
            const COM::CMcpAuditRecord limited = audit.records.back();
            ASSERT_TRUE( "reader" == limited.target && COM::MCP_OUTCOME_PROTOCOL_ERROR == limited.outcome && -31001 == limited.errorCode );
            ASSERT_TRUE( limited.argumentsDigest == Sha256Hex( "{\"text\":\"c\"}" ) );

            // wait for the bucket, start the slow call and cancel it
            MT::ThreadDelay( 1100 );
            rig.Send( McpTestRequest( "4", "tools/call", "\"name\":\"slow\",\"arguments\":{}" ) );
            rig.Pulse( 20 );
            rig.Send( McpTestNotification( "notifications/cancelled", "\"requestId\":4" ) );
            for ( UInt32 i=0; i<2000 && NULL == audit.Last( "tools/call" ) ; ++i )
                rig.Pulse();
            bool sawCancelled = false;
            for ( UInt32 i=0; i<3000 && !sawCancelled; ++i )
            {
                for ( size_t n=0; n<audit.records.size(); ++n )
                    if ( COM::MCP_OUTCOME_CANCELLED == audit.records[ n ].outcome && audit.records[ n ].requestId == "4" )
                        sawCancelled = true;
                if ( !sawCancelled )
                    rig.Pulse();
            }
            ASSERT_TRUE( sawCancelled );
            ASSERT_TRUE( rig.AllSentAreJsonLines() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The JSON lines audit log writes one line per record to any stream, and a failure is counted and not thrown" )
        try
        {
            CMcpTestRig rig;
            RegisterThreeTools( rig );

            CORE::CDynamicBuffer buffer;
            COM::CMcpJsonLinesAuditLog log;
            ASSERT_FALSE( log.WriteRecord( COM::CMcpAuditRecord() ) );              // no output yet
            ASSERT_TRUE( 1 == log.GetWriteFailures() );
            ASSERT_TRUE( log.SetOutput( CORE::IOAccessPtr( GUCEF_NEW CORE::CDynamicBufferAccess( buffer ) ) ) );
            ASSERT_TRUE( log.Attach( *rig.server ) );

            CORE::CDataNode response;
            ASSERT_TRUE( rig.Call( "1", McpTestRequest( "1", "server/discover" ), response ) );
            ASSERT_TRUE( CallTool( rig, "2", "reader", "line one\\nline two", response ) );
            ASSERT_TRUE( CallTool( rig, "3", "adder", "x", response ) );
            ASSERT_TRUE( 3 == log.GetRecordsWritten() );

            CORE::CString text( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() );
            CORE::CString::StringVector lines = text.ParseElements( '\n', false );
            ASSERT_TRUE( '\n' == text[ text.Length() - 1 ] && text.HasChar( '\r' ) < 0 );
            ASSERT_TRUE( lines.size() >= 3 );
            UInt32 parsed = 0;
            for ( size_t i=0; i<lines.size(); ++i )
            {
                if ( lines[ i ].IsNULLOrEmpty() )
                    continue;
                CORE::CDataNode node;
                if ( McpTestParse( lines[ i ], node ) )
                {
                    ++parsed;
                    ASSERT_TRUE( McpTestAttr( node, "transport" ) == "test" && McpTestAttr( node, "subject" ) == "local:test" );
                    ASSERT_TRUE( McpTestAttr( node, "clientInfoVerified" ).Lowercase() == "false" );
                }
            }
            ASSERT_TRUE( 3 == parsed );
            ASSERT_TRUE( text.HasSubstr( "\"outcome\":\"protocol_error\"" ) >= 0 && text.HasSubstr( "\"decision\":\"hide\"" ) >= 0 );

            // after detaching nothing more is written
            log.Detach();
            ASSERT_TRUE( CallTool( rig, "4", "reader", "again", response ) );
            ASSERT_TRUE( 3 == log.GetRecordsWritten() );

            // an output that cannot be written to: counted
            log.Close();
            ASSERT_FALSE( log.WriteRecord( COM::CMcpAuditRecord() ) );
            ASSERT_TRUE( log.GetWriteFailures() >= 2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The JSON lines audit log rolls over to new files and keeps a number of them, without splitting a record" )
        try
        {
            TESTFW::CScopedTestDir scopedDir( "gucefCOM_TestApp_McpAuditRoll" );
            CORE::CString dir = scopedDir.GetPath();
            CORE::CreateDirs( dir );
            const CORE::CString path = CORE::CombinePath( dir, "audit.jsonl" );
            const CORE::CString rolledPrefix = CORE::CombinePath( dir, "audit." );
            // an earlier run that failed half way
            CORE::DeleteFile( path );
            for ( UInt32 i=1; i<64; ++i )
                CORE::DeleteFile( rolledPrefix + CORE::UInt32ToString( i ) + ".jsonl" );

            COM::CMcpJsonLinesAuditLog log;
            ASSERT_FALSE( log.OpenRollingFile( path, 0, 3 ) );
            ASSERT_TRUE( log.OpenRollingFile( path, 600, 3 ) );
            COM::CMcpAuditRecord record;
            record.method = "tools/call";
            record.target = "reader";
            record.transport = "test";
            record.subject = "local:test";
            for ( UInt32 i=0; i<20; ++i )
            {
                record.requestId = CORE::UInt32ToString( i );
                ASSERT_TRUE( log.WriteRecord( record ) );
            }
            ASSERT_TRUE( 20 == log.GetRecordsWritten() );
            log.Close();

            // several files, no more than asked for, every line whole, and the newest records in the file that is written
            UInt32 files = 0;
            UInt32 lines = 0;
            UInt32 wholeLines = 0;
            CORE::CString newestId;
            for ( UInt32 i=0; i<64; ++i )
            {
                CORE::CString file = 0 == i ? path : rolledPrefix + CORE::UInt32ToString( i ) + ".jsonl";
                if ( !CORE::FileExists( file ) )
                    continue;
                ++files;
                CORE::CDynamicBuffer content;
                ASSERT_TRUE( content.LoadContentFromFile( file ) );
                ASSERT_TRUE( content.GetDataSize() <= 600 + 400 );              // a record is never split, so a file ends within one record of the threshold
                CORE::CString text( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() );
                CORE::CString::StringVector fileLines = text.ParseElements( '\n', false );
                for ( size_t n=0; n<fileLines.size(); ++n )
                {
                    if ( fileLines[ n ].IsNULLOrEmpty() )
                        continue;
                    ++lines;
                    CORE::CDataNode node;
                    if ( McpTestParse( fileLines[ n ], node ) )
                    {
                        ++wholeLines;
                        if ( 0 == i )
                            newestId = McpTestAttr( node, "requestId" );
                    }
                }
            }
            ASSERT_TRUE( files > 1 );
            ASSERT_TRUE( files <= 3 );
            ASSERT_TRUE( lines == wholeLines );
            ASSERT_TRUE( lines <= 20 && lines >= 3 );
            ASSERT_TRUE( newestId == "19" );

            // appended to again by a new log: the file is continued
            COM::CMcpJsonLinesAuditLog again;
            ASSERT_TRUE( again.OpenRollingFile( path, 600, 3 ) );
            record.requestId = "20";
            ASSERT_TRUE( again.WriteRecord( record ) );
            again.Close();

            CORE::DeleteFile( path );
            for ( UInt32 i=1; i<64; ++i )
                CORE::DeleteFile( rolledPrefix + CORE::UInt32ToString( i ) + ".jsonl" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Audit lines with text beyond ASCII are written whole, and bytes that are no UTF-8 never reach a line" )
        try
        {
            TESTFW::CScopedTestDir scopedDir( "gucefCOM_TestApp_McpAuditUtf8" );
            CORE::CString dir = scopedDir.GetPath();
            CORE::CreateDirs( dir );
            const CORE::CString path = CORE::CombinePath( dir, "audit.jsonl" );
            CORE::DeleteFile( path );
            {
                COM::CMcpJsonLinesAuditLog log;
                ASSERT_TRUE( log.OpenRollingFile( path, 1048576, 3 ) );
                COM::CMcpAuditRecord record;
                record.method = "tools/call";
                record.target = "caf\xc3\xa9 \xe2\x82\xac \xf0\x9f\x98\x80";
                ASSERT_TRUE( log.WriteRecord( record ) );
                record.target = "bad\xff\xfe";
                ASSERT_TRUE( log.WriteRecord( record ) );
                ASSERT_TRUE( 0 == log.GetWriteFailures() && 2 == log.GetRecordsWritten() );
                log.Close();
            }
            CORE::CDynamicBuffer content;
            ASSERT_TRUE( content.LoadContentFromFile( path ) );
            CORE::CString text( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() );
            ASSERT_TRUE( text.HasSubstr( "\"target\":\"caf\xc3\xa9 \xe2\x82\xac \xf0\x9f\x98\x80\"" ) >= 0 );
            ASSERT_TRUE( text.HasSubstr( "\xff" ) < 0 && text.HasSubstr( "\xfe" ) < 0 );
            CORE::CString::StringVector lines = text.ParseElements( '\n', false );
            UInt32 parsed = 0;
            for ( size_t i=0; i<lines.size(); ++i )
            {
                CORE::CDataNode node;
                if ( !lines[ i ].IsNULLOrEmpty() && McpTestParse( lines[ i ], node ) )
                    ++parsed;
            }
            ASSERT_TRUE( 2 == parsed );
            CORE::DeleteFile( path );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    GUCEF_TESTFW_TESTCASE( "Test: The audit log files are for their owner only, also with umask 0 and after a roll over" )
        try
        {
            TESTFW::CScopedTestDir scopedDir( "gucefCOM_TestApp_McpAuditMode" );
            CORE::CString dir = scopedDir.GetPath();
            CORE::CreateDirs( dir );
            const CORE::CString path = CORE::CombinePath( dir, "audit.jsonl" );
            const CORE::CString rolled = CORE::CombinePath( dir, "audit.1.jsonl" );
            CORE::DeleteFile( path );
            CORE::DeleteFile( rolled );

            const mode_t oldMask = ::umask( 0 );
            bool opened = false;
            {
                COM::CMcpJsonLinesAuditLog log;
                opened = log.OpenRollingFile( path, 300, 3 );
                COM::CMcpAuditRecord record;
                record.method = "tools/call";
                record.subject = "local:test";
                for ( UInt32 i=0; i<6 && opened; ++i )
                    log.WriteRecord( record );
                log.Close();
            }
            ::umask( oldMask );
            ASSERT_TRUE( opened );
            struct stat info;
            ASSERT_TRUE( 0 == ::stat( path.C_String(), &info ) && 0600 == ( info.st_mode & 0777 ) );
            ASSERT_TRUE( 0 == ::stat( rolled.C_String(), &info ) && 0600 == ( info.st_mode & 0777 ) );

            // a log that already exists more open is restricted when it is opened; the restriction can be switched off
            ::chmod( path.C_String(), 0644 );
            {
                COM::CMcpJsonLinesAuditLog log;
                ASSERT_TRUE( log.OpenRollingFile( path, 300, 3 ) );
                log.Close();
            }
            ASSERT_TRUE( 0 == ::stat( path.C_String(), &info ) && 0600 == ( info.st_mode & 0777 ) );
            ::chmod( path.C_String(), 0644 );
            {
                COM::CMcpJsonLinesAuditLog log;
                log.SetRestrictToOwner( false );
                ASSERT_TRUE( log.OpenRollingFile( path, 300, 3 ) );
                log.Close();
            }
            ASSERT_TRUE( 0 == ::stat( path.C_String(), &info ) && 0644 == ( info.st_mode & 0777 ) );
            CORE::DeleteFile( path );
            CORE::DeleteFile( rolled );
            CORE::DeleteFile( CORE::CombinePath( dir, "audit.2.jsonl" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
    #endif

    GUCEF_TESTFW_TESTCASE( "Test: A denied resource URI cannot be reached by another spelling: percent-encoding, dot segments, the case of the scheme" )
        try
        {
            CMcpTestRig rig;
            CORE::CString error;
            ASSERT_TRUE( rig.server->GetResourceRegistry().RegisterTemplate( COM::CIMcpResourceTemplatePtr( GUCEF_NEW CMcpTestTemplate( "items", "test://items/{id}" ) ), error ) );
            COM::CMcpConfigurableAccessPolicy* policy = GUCEF_NEW COM::CMcpConfigurableAccessPolicy();
            policy->GetDefaultRules().AddDenyResource( "test://items/1" );
            rig.server->SetAccessPolicy( COM::CIMcpAccessPolicyPtr( policy ) );

            CORE::CDataNode response;
            ASSERT_TRUE( ReadResource( rig, "1", "test://items/1", response ) && -32602 == McpTestErrorCode( response ) );
            const char* spellings[] = { "test://items/%31", "test://items/./1", "test://items/%2e/1", "test://items/%2E/%31", "test://items/x/../1",
                                        "TEST://items/1", "test://items/%2e%2e/items/1", "test://items/%2", "test://items/%2F1", "test://items/%00" };
            for ( UInt32 i=0; i<10; ++i )
            {
                CORE::CString id = CORE::ToString( 10 + i );
                ASSERT_TRUE( ReadResource( rig, id.C_String(), spellings[ i ], response ) );
                const bool refused = GUCEF_NULL == response.FindChild( "result" ) && McpTestErrorCode( response ) != 0;
                if ( !refused )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpPolicy test: " << spellings[ i ] << " was read: " << McpTestToJson( response ) );
                ASSERT_TRUE( refused );
            }

            // an allowed item by another spelling is read under its one normal form
            ASSERT_TRUE( ReadResource( rig, "30", "test://items/%32", response ) );
            const CORE::CDataNode* contents = McpTestResultChild( response, "contents" );
            ASSERT_TRUE( 1 == McpTestChildCount( contents ) );
            if ( 1 == McpTestChildCount( contents ) )
                ASSERT_TRUE( McpTestAttr( *(*contents->ConstBegin()), "uri" ) == "test://items/2" );

            // in-process callers go the same way
            COM::CMcpResourceContents read;
            ASSERT_FALSE( rig.server->ReadResource( "test://items/%31", COM::CMcpPrincipal(), read, error ) );
            ASSERT_FALSE( rig.server->ReadResource( "test://items/./1", COM::CMcpPrincipal(), read, error ) );
            ASSERT_TRUE( rig.server->ReadResource( "test://items/%32", COM::CMcpPrincipal(), read, error ) );
            ASSERT_TRUE( rig.AllSentAreJsonLines() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The normal form of a resource URI" )
        try
        {
            const char* cases[][ 2 ] = {
                { "test://items/1", "test://items/1" },
                { "test://items/%31", "test://items/1" },
                { "TeSt://Items/A", "test://items/A" },
                { "test://items/%7e%2d", "test://items/~-" },
                { "test://items/a%2fb", "" },
                { "test://items/a%5Cb", "" },
                { "test://items/%c3%a9", "test://items/%C3%A9" },
                { "test://items/%c3", "" },
                { "test://items/%ff", "" },
                { "test://items/%00", "" },
                { "test://items/%zz", "" },
                { "test://items/a b", "" },
                { "test://items/./a/../b/./c", "test://items/b/c" },
                { "test://items/../../x", "test://items/x" },
                { "test://items/a/b/..", "test://items/a/" },
                { "test://items/q?x=%2F&y=%41#f", "test://items/q?x=%2F&y=A#f" },
                { "file:///etc/../passwd", "file:///passwd" },
                { "urn:a:%62", "urn:a:b" },
                { "nocolon", "" },
                { "", "" } };
            for ( UInt32 i=0; i<sizeof( cases ) / sizeof( cases[ 0 ] ); ++i )
            {
                CORE::CString normalized;
                CORE::CString problem;
                const bool ok = COM::CMcpResourceRegistry::NormalizeUri( cases[ i ][ 0 ], normalized, problem );
                const CORE::CString expected = cases[ i ][ 1 ];
                const bool asExpected = expected.IsNULLOrEmpty() ? !ok : ( ok && normalized == expected );
                if ( !asExpected )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpPolicy test: \"" << cases[ i ][ 0 ] << "\" normalized to \"" << normalized << "\" (" << ok << ", " << problem << "), expected \"" << expected << "\"" );
                ASSERT_TRUE( asExpected );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL MCP POLICY AND AUDIT TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
