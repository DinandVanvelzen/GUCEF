/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#include "dvcppstringutils.h"
#include "gucefMT_dvmtoswrap.h"
#include "gucefCORE_Logging.h"
#include "gucefCOM_CJsonRpcEndpoint.h"

#ifndef TESTASSETSTORESUPPORT_H
#include "TestAssetStoreSupport.h"
#define TESTASSETSTORESUPPORT_H
#endif /* TESTASSETSTORESUPPORT_H ? */

#include "TestToolsFixture.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace ASSETSTORETEST {

using namespace GUCEF;
using namespace GUCEF::ASSETSTORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CToolsFixture::CToolsFixture( const TAssetStoreServicePtr& servicePtr, const CAssetToolSetSettings& settings, const char* subject )
    : rig( subject )
    , service( servicePtr )
    , toolSet( GUCEF_NULL )
    , ok( false )
    , error()
    , m_nextId( 1 )
{
    // the rate limit of tool calls is the server's own (tested with gucefCOM): off, so that polling under load cannot trip it
    COM::CMcpServerSettings serverSettings = rig.GetServer().GetSettings();
    serverSettings.toolCallsPerSecond = 0;
    rig.GetServer().SetSettings( serverSettings );
    toolSet = GUCEF_NEW CAssetToolSet( servicePtr, settings );
    ok = toolSet->RegisterInto( rig.GetServer(), error );
}

/*-------------------------------------------------------------------------*/

CToolsFixture::~CToolsFixture()
{
    toolSet->Unregister( 5000 );
    GUCEF_DELETE toolSet;
    toolSet = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CToolsFixture::SetExposure( const CAssetToolsExposure& exposure )
{
    toolSet->GetContext().GetExposureControl().SetState( exposure );
    Pump( 5 );
}

/*-------------------------------------------------------------------------*/

void
CToolsFixture::EnableAll( void )
{
    CAssetToolsExposure exposure;
    exposure.convertEnabled = true;
    exposure.authorEnabled = true;
    exposure.storeEnabled = true;
    exposure.origin = "test";
    SetExposure( exposure );
}

/*-------------------------------------------------------------------------*/

void
CToolsFixture::Pump( UInt32 durationInMs )
{
    const UInt64 start = MT::PrecisionTickCount();
    do
    {
        rig.Pump( 1 );
        PumpFor( 1 );
    }
    while ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < durationInMs );
}

/*-------------------------------------------------------------------------*/

bool
CToolsFixture::RequestWith( const CORE::CString& method, const CORE::CString& params, const CORE::CString& capabilities,
                            CORE::CDataNode& response, UInt32 timeoutMs )
{
    const CORE::CString id = CORE::ToString( m_nextId++ );
    if ( !rig.Send( TESTFW::CTestMcpRig::MakeRequest( id, method, params, capabilities ) ) )
        return false;
    const UInt64 start = MT::PrecisionTickCount();
    while ( true )
    {
        if ( rig.GetChannel().FindResponse( id, response ) )
            return true;
        if ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) > timeoutMs )
            return false;
        rig.Pump( 1 );
        PumpFor( 1 );
    }
}

/*-------------------------------------------------------------------------*/

bool
CToolsFixture::Request( const CORE::CString& method, const CORE::CString& params, CORE::CDataNode& response, UInt32 timeoutMs )
{
    return RequestWith( method, params, "{}", response, timeoutMs );
}

/*-------------------------------------------------------------------------*/

bool
CToolsFixture::Call( const CORE::CString& tool, const CORE::CString& argumentsJson, CORE::CDataNode& response, UInt32 timeoutMs, const CORE::CString& progressToken )
{
    const CORE::CString id = CORE::ToString( m_nextId++ );
    CORE::CString extraMeta;
    if ( !progressToken.IsNULLOrEmpty() )
        extraMeta = "\"progressToken\":\"" + progressToken + "\"";
    if ( !rig.Send( TESTFW::CTestMcpRig::MakeRequest( id, "tools/call", "\"name\":\"" + tool + "\",\"arguments\":" + argumentsJson, "{}", extraMeta ) ) )
        return false;
    const UInt64 start = MT::PrecisionTickCount();
    while ( true )
    {
        if ( rig.GetChannel().FindResponse( id, response ) )
        {
            CORE::CString text;
            COM::CJsonRpcEndpoint::ToJsonText( response, text );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ToolsFixture: " << tool << " answered " << text.SubstrToIndex( 1500, true ) );
            return true;
        }
        if ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) > timeoutMs )
            return false;
        rig.Pump( 1 );
        PumpFor( 1 );
    }
}

/*-------------------------------------------------------------------------*/

bool
CToolsFixture::ListTools( CORE::CString::StringVector& names, CORE::CDataNode* list )
{
    names.clear();
    CORE::CDataNode response;
    if ( !Request( "tools/list", CORE::CString::Empty, response ) )
        return false;
    const CORE::CDataNode* result = response.FindChild( "result" );
    const CORE::CDataNode* tools = GUCEF_NULL != result ? result->FindChild( "tools" ) : GUCEF_NULL;
    if ( GUCEF_NULL == tools )
        return false;
    if ( GUCEF_NULL != list )
        list->Copy( *tools );
    CORE::CDataNode::const_iterator i = tools->ConstBegin();
    while ( i != tools->ConstEnd() )
    {
        names.push_back( (*i)->GetAttributeValueOrChildValueByName( "name" ).AsString( CORE::CString::Empty, true ) );
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CToolsFixture::WaitForJobState( const CORE::CString& jobId, const CORE::CString& state, UInt32 timeoutMs )
{
    const UInt64 start = MT::PrecisionTickCount();
    while ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < timeoutMs )
    {
        CORE::CDataNode response;
        if ( Call( "job_status", "{\"jobId\":\"" + jobId + "\"}", response ) && StructuredAttr( response, "state" ) == state )
            return true;
        Pump( 50 );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CToolsFixture::GetTranscript( void ) const
{
    CORE::CString text;
    const TESTFW::CTestMcpChannel::TTextVector& sent = const_cast< TESTFW::CTestMcpRig& >( rig ).GetChannel().GetSent();
    for ( size_t i=0; i<sent.size(); ++i )
        text += sent[ i ] + "\n";
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CToolsFixture::Attr( const CORE::CDataNode* node, const char* name )
{
    if ( GUCEF_NULL == node )
        return "<none>";
    const CORE::CDataNode::TKeyValuePair* attribute = node->GetAttribute( name );
    return GUCEF_NULL != attribute ? (*attribute).second.AsString() : CORE::CString( "<none>" );
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
CToolsFixture::Structured( const CORE::CDataNode& response )
{
    return TESTFW::CTestMcpRig::GetStructuredContent( response );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CToolsFixture::StructuredAttr( const CORE::CDataNode& response, const char* name )
{
    return Attr( Structured( response ), name );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CToolsFixture::ErrorCode( const CORE::CDataNode& response )
{
    const CORE::CDataNode* content = Structured( response );
    return Attr( GUCEF_NULL != content ? content->FindChild( "error" ) : GUCEF_NULL, "code" );
}

/*-------------------------------------------------------------------------*/

bool
CToolsFixture::IsToolError( const CORE::CDataNode& response )
{
    return TESTFW::CTestMcpRig::IsToolError( response );
}

/*-------------------------------------------------------------------------*/

Int32
CToolsFixture::ProtocolError( const CORE::CDataNode& response )
{
    return TESTFW::CTestMcpRig::GetErrorCode( response );
}

/*-------------------------------------------------------------------------*/

UInt32
CToolsFixture::CountContent( const CORE::CDataNode& response, const char* type )
{
    const CORE::CDataNode* result = response.FindChild( "result" );
    const CORE::CDataNode* content = GUCEF_NULL != result ? result->FindChild( "content" ) : GUCEF_NULL;
    if ( GUCEF_NULL == content )
        return 0;
    UInt32 count = 0;
    CORE::CDataNode::const_iterator i = content->ConstBegin();
    while ( i != content->ConstEnd() )
    {
        if ( (*i)->GetAttributeValueOrChildValueByName( "type" ).AsString( CORE::CString::Empty, true ) == type )
            ++count;
        ++i;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CToolsFixture::CountItems( const CORE::CDataNode& response, const char* listName )
{
    const CORE::CDataNode* content = Structured( response );
    const CORE::CDataNode* list = GUCEF_NULL != content ? content->FindChild( listName ) : GUCEF_NULL;
    return GUCEF_NULL != list ? list->GetNrOfDirectChildNodes() : 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/
