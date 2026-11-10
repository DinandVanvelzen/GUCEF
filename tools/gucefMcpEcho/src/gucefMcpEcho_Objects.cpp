/*
 *  gucefMcpEcho: a small reference MCP server built on gucefCOM
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMcpEcho_Objects.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpEcho {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A schema of an object with the given properties: { "type": "object", "properties": { name: { "type": type, ... } }, "required": [...] } */
static void
AddProperty( CORE::CDataNode& schema, const char* name, const char* type, const char* description )
{
    CORE::CDataNode* properties = schema.FindChild( "properties" );
    if ( GUCEF_NULL == properties )
        properties = schema.AddChild( "properties", GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* property = properties->AddChild( name, GUCEF_DATATYPE_OBJECT );
    property->SetAttribute( "type", type );
    property->SetAttribute( "description", description );
}

/*-------------------------------------------------------------------------*/

static void
AddRequired( CORE::CDataNode& schema, const char* name )
{
    CORE::CDataNode* required = schema.FindChild( "required" );
    if ( GUCEF_NULL == required )
        required = schema.AddChild( "required", GUCEF_DATATYPE_ARRAY );
    required->AddValueAsChild( CORE::CVariant( name ) );
}

/*-------------------------------------------------------------------------*/

static CORE::CDataNode
MakeObjectSchema( void )
{
    CORE::CDataNode schema( GUCEF_DATATYPE_OBJECT );
    schema.SetAttribute( "type", "object" );
    return schema;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE TOOLS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

class CEchoTool : public COM::CIMcpTool
{
    public:

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "echo";
        descriptor.title = "Echo";
        descriptor.description = "Returns the text it is given";
        descriptor.effect = COM::MCP_TOOLEFFECT_READ_ONLY;
        descriptor.idempotent = true;
        CORE::CDataNode schema = MakeObjectSchema();
        AddProperty( schema, "text", "string", "the text to return" );
        schema.FindChild( "properties" )->FindChild( "text" )->SetAttribute( "maxLength", (Int32) 4096 );
        AddRequired( schema, "text" );
        schema.SetAttribute( "additionalProperties", false );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return COM::MCP_EXECUTION_CALLING_THREAD;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {
        result.AddText( arguments.GetAttributeValue( "text" ).AsString() );
        return COM::MCP_EXECUTE_COMPLETE;
    }
};

/*-------------------------------------------------------------------------*/

class CSlowCountTool : public COM::CIMcpTool
{
    public:

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "slow_count";
        descriptor.title = "Slow count";
        descriptor.description = "Counts to a number, slowly, and reports progress. Stops when it is cancelled";
        descriptor.effect = COM::MCP_TOOLEFFECT_READ_ONLY;
        descriptor.idempotent = true;
        CORE::CDataNode schema = MakeObjectSchema();
        AddProperty( schema, "steps", "integer", "how far to count, 1 to 1000" );
        AddProperty( schema, "stepMs", "integer", "the pause per step in milliseconds, 0 to 1000" );
        CORE::CDataNode* properties = schema.FindChild( "properties" );
        properties->FindChild( "steps" )->SetAttribute( "minimum", (Int32) 1 );
        properties->FindChild( "steps" )->SetAttribute( "maximum", (Int32) 1000 );
        properties->FindChild( "stepMs" )->SetAttribute( "minimum", (Int32) 0 );
        properties->FindChild( "stepMs" )->SetAttribute( "maximum", (Int32) 1000 );
        schema.SetAttribute( "additionalProperties", false );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return COM::MCP_EXECUTION_THREAD_POOL;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt32 steps = GUCEF_NULL != arguments.GetAttribute( "steps" ) ? arguments.GetAttributeValue( "steps" ).AsUInt32() : 5;
        const UInt32 stepMs = GUCEF_NULL != arguments.GetAttribute( "stepMs" ) ? arguments.GetAttributeValue( "stepMs" ).AsUInt32() : 100;
        for ( UInt32 i=1; i<=steps; ++i )
        {
            if ( context.IsCancellationRequested() )
            {
                result.SetError( "cancelled" );
                return COM::MCP_EXECUTE_COMPLETE;
            }
            MT::ThreadDelay( stepMs );
            context.ReportProgress( (Float64) i, (Float64) steps, "counting" );
        }
        result.AddText( "counted to " + CORE::UInt32ToString( steps ) );
        return COM::MCP_EXECUTE_COMPLETE;
    }
};

/*-------------------------------------------------------------------------*/

class CAskNameTool : public COM::CIMcpTool
{
    public:

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "ask_name";
        descriptor.title = "Ask the name";
        descriptor.description = "Asks the user for a name by elicitation and greets that name";
        descriptor.effect = COM::MCP_TOOLEFFECT_READ_ONLY;
        descriptor.SetNoArguments();
        return true;
    }

    virtual COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return COM::MCP_EXECUTION_CALLING_THREAD;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {
        const CORE::CDataNode* answer = context.HasInputResponses() ? context.GetInputResponses().FindChild( "name_request" ) : GUCEF_NULL;
        const CORE::CDataNode* content = GUCEF_NULL != answer ? answer->FindChild( "content" ) : GUCEF_NULL;
        if ( GUCEF_NULL == content )
        {
            if ( !context.HasClientCapability( "elicitation/form" ) )
            {
                result.SetError( "this tool needs a client that can ask the user a question (form elicitation)" );
                return COM::MCP_EXECUTE_COMPLETE;
            }
            CORE::CDataNode requested = MakeObjectSchema();
            AddProperty( requested, "name", "string", "your name" );
            AddRequired( requested, "name" );
            inputRequired.AddFormElicitation( "name_request", "What is your name?", requested );
            CORE::CDataNode state( GUCEF_DATATYPE_OBJECT );
            state.SetAttribute( "step", (Int32) 1 );
            inputRequired.SetState( state );
            return COM::MCP_EXECUTE_INPUT_REQUIRED;
        }
        result.AddText( "Hello " + content->GetAttributeValue( "name" ).AsString() );
        return COM::MCP_EXECUTE_COMPLETE;
    }
};

/*-------------------------------------------------------------------------*/

class CAddNoteTool : public COM::CIMcpTool
{
    public:

    explicit CAddNoteTool( CNoteStore& notes ) : m_notes( notes ) {}

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "add_note";
        descriptor.title = "Add a note";
        descriptor.description = "Stores a note, which echo://notes/{id} then returns";
        descriptor.effect = COM::MCP_TOOLEFFECT_ADDITIVE;
        descriptor.idempotent = false;
        CORE::CDataNode schema = MakeObjectSchema();
        AddProperty( schema, "title", "string", "the title of the note" );
        AddProperty( schema, "text", "string", "the text of the note" );
        AddRequired( schema, "title" );
        AddRequired( schema, "text" );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return COM::MCP_EXECUTION_CALLING_THREAD;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt32 id = m_notes.Add( arguments.GetAttributeValue( "title" ).AsString(), arguments.GetAttributeValue( "text" ).AsString() );
        result.AddText( "stored note " + CORE::UInt32ToString( id ) );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    private:

    CNoteStore& m_notes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE RESOURCES AND THE PROMPT                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

class CAboutResource : public COM::CIMcpResource
{
    public:

    virtual bool GetDescriptor( COM::CMcpResourceDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.uri = "echo://about";
        descriptor.name = "about";
        descriptor.title = "About this server";
        descriptor.mimeType = "text/plain";
        return true;
    }

    virtual COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return COM::MCP_EXECUTION_CALLING_THREAD;
    }

    virtual COM::EMcpExecuteResult Read( COM::CMcpRequestContext& context, COM::CMcpResourceContents& contents, COM::CMcpInputRequired& inputRequired ) GUCEF_VIRTUAL_OVERRIDE
    {
        contents.AddText( "echo://about", "text/plain", "gucefMcpEcho, the reference MCP server of GUCEF" );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    virtual void GetCacheHints( UInt32& ttlMs, COM::EMcpCacheScope& scope ) const GUCEF_VIRTUAL_OVERRIDE
    {
        ttlMs = 60000;
        scope = COM::MCP_CACHESCOPE_PUBLIC;
    }
};

/*-------------------------------------------------------------------------*/

class CNoteTemplate : public COM::CIMcpResourceTemplate
{
    public:

    explicit CNoteTemplate( CNoteStore& notes ) : m_notes( notes ) {}

    virtual bool GetDescriptor( COM::CMcpResourceTemplateDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "note";
        descriptor.uriTemplate = "echo://notes/{id}";
        descriptor.title = "A stored note";
        descriptor.mimeType = "text/plain";
        return true;
    }

    virtual COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return COM::MCP_EXECUTION_CALLING_THREAD;
    }

    virtual COM::EMcpExecuteResult Read( COM::CMcpRequestContext& context       ,
                                         const CORE::CString& uri               ,
                                         const CORE::CValueList& variables      ,
                                         COM::CMcpResourceContents& contents    ,
                                         COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CVariant id;
        CORE::CString title;
        CORE::CString text;
        if ( !variables.TryGetValue( "id", id ) || !m_notes.TryGet( CORE::StringToUInt32( id.AsString() ), title, text ) )
        {
            context.SetProtocolError( COM::MCP_ERROR_INVALID_PARAMS, "Resource not found" );
            return COM::MCP_EXECUTE_PROTOCOL_ERROR;
        }
        contents.AddText( uri, "text/plain", title + "\n" + text );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    private:

    CNoteStore& m_notes;
};

/*-------------------------------------------------------------------------*/

class CGreetPrompt : public COM::CIMcpPrompt
{
    public:

    virtual bool GetDescriptor( COM::CMcpPromptDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "greet";
        descriptor.title = "Greet someone";
        descriptor.description = "A greeting for a name";
        descriptor.AddArgument( "name", "who to greet", true );
        descriptor.AddArgument( "style", "formal or casual", false );
        return true;
    }

    virtual COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return COM::MCP_EXECUTION_CALLING_THREAD;
    }

    virtual COM::EMcpExecuteResult Get( COM::CMcpRequestContext& context       ,
                                        const CORE::CValueList& arguments      ,
                                        COM::CMcpPromptResult& result          ,
                                        COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CVariant name;
        CORE::CVariant style;
        arguments.TryGetValue( "name", name );
        const bool formal = arguments.TryGetValue( "style", style ) && style.AsString() == "formal";
        result.description = "A greeting";
        result.AddUserText( CORE::CString( "Write a " ) + ( formal ? "formal" : "friendly" ) + " greeting for " + name.AsString() );
        return COM::MCP_EXECUTE_COMPLETE;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CNoteStore::CNoteStore( void )
    : m_lock()
    , m_titles()
    , m_texts()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CNoteStore::Add( const CORE::CString& title, const CORE::CString& text )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_titles.push_back( title );
    m_texts.push_back( text );
    return (UInt32) m_titles.size();
}

/*-------------------------------------------------------------------------*/

bool
CNoteStore::TryGet( UInt32 id, CORE::CString& title, CORE::CString& text ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( 0 == id || id > m_titles.size() )
        return false;
    title = m_titles[ id-1 ];
    text = m_texts[ id-1 ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
RegisterDemoObjects( COM::CMcpServer& server, CNoteStore& notes )
{GUCEF_TRACE;

    CORE::CString error;
    bool allRegistered = true;

    // the registries keep the objects alive for as long as a call runs, the objects themselves are made here for the life of the process
    allRegistered = server.GetToolRegistry().Register( COM::CIMcpToolPtr( GUCEF_NEW CEchoTool() ), error ) && allRegistered;
    allRegistered = server.GetToolRegistry().Register( COM::CIMcpToolPtr( GUCEF_NEW CSlowCountTool() ), error ) && allRegistered;
    allRegistered = server.GetToolRegistry().Register( COM::CIMcpToolPtr( GUCEF_NEW CAskNameTool() ), error ) && allRegistered;
    allRegistered = server.GetToolRegistry().Register( COM::CIMcpToolPtr( GUCEF_NEW CAddNoteTool( notes ) ), error ) && allRegistered;
    allRegistered = server.GetResourceRegistry().RegisterResource( COM::CIMcpResourcePtr( GUCEF_NEW CAboutResource() ), error ) && allRegistered;
    allRegistered = server.GetResourceRegistry().RegisterTemplate( COM::CIMcpResourceTemplatePtr( GUCEF_NEW CNoteTemplate( notes ) ), error ) && allRegistered;
    allRegistered = server.GetPromptRegistry().Register( COM::CIMcpPromptPtr( GUCEF_NEW CGreetPrompt() ), error ) && allRegistered;
    if ( !allRegistered )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpEcho: A demo object was refused by the registries: " << error );
    return allRegistered;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------*/
