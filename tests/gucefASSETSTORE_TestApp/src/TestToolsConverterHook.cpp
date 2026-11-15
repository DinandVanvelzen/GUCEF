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

#include "gucefMT_CScopeMutex.h"
#include "gucefMT_dvmtoswrap.h"
#include "dvcppstringutils.h"
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#include "gucefASSETSTORE_CAssetToolUris.h"

#include "TestToolsConverterHook.h"

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

CToolsTestConverterHook::CToolsTestConverterHook( void )
    : CTestConverterHook()
    , m_hookLock()
    , m_steps( 4 )
    , m_stepDelayMs( 5 )
    , m_echo()
    , m_tokens( 0 )
    , m_openTokens()
    , m_counts()
{
}

/*-------------------------------------------------------------------------*/

CToolsTestConverterHook::~CToolsTestConverterHook()
{
}

/*-------------------------------------------------------------------------*/

void
CToolsTestConverterHook::SetJobSteps( UInt32 steps, UInt32 stepDelayMs )
{
    MT::CScopeMutex lock( m_hookLock );
    m_steps = steps;
    m_stepDelayMs = stepDelayMs;
}

/*-------------------------------------------------------------------------*/

void
CToolsTestConverterHook::SetEcho( const CORE::CString& text )
{
    MT::CScopeMutex lock( m_hookLock );
    m_echo = text;
}

/*-------------------------------------------------------------------------*/

UInt32
CToolsTestConverterHook::GetOperationCount( const CORE::CString& operation ) const
{
    MT::CScopeMutex lock( m_hookLock );
    GUCEF::map< CORE::CString, UInt32 >::const_iterator i = m_counts.find( operation );
    return i != m_counts.end() ? (*i).second : 0;
}

/*-------------------------------------------------------------------------*/

bool
CToolsTestConverterHook::SupportsPipelineOperation( const CORE::CString& operation ) const
{
    return CAssetPipelineOperations::IsKnown( operation ) && !CAssetPipelineOperations::IsRender( operation );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CToolsTestConverterHook::RunPipelineOperation( const CAssetPipelineRequest& request ,
                                               CIAssetConversionObserver* observer ,
                                               CAssetServiceDocument& result       )
{
    CORE::CString echo;
    UInt32 steps = 0;
    UInt32 stepDelayMs = 0;
    {
        MT::CScopeMutex lock( m_hookLock );
        ++m_counts[ request.operation ];
        echo = m_echo;
        steps = m_steps;
        stepDelayMs = m_stepDelayMs;
    }
    result.Clear();
    result.document.SetName( "result" );
    result.document.SetNodeType( GUCEF_DATATYPE_OBJECT );
    result.document.SetAttribute( "operation", request.operation );
    result.document.SetAttribute( "packId", request.packId );
    if ( !echo.IsNULLOrEmpty() )
        result.document.SetAttribute( "note", "the converter says " + echo );

    if ( CAssetPipelineOperations::LicenceCheck == request.operation )
    {
        const bool withheld = 0 == request.packId.HasSubstr( "nml", true );
        const bool refused = 0 == request.packId.HasSubstr( "refused", true );
        result.document.SetAttribute( "recordFound", true );
        result.document.SetAttribute( "conversionAllowed", !refused );
        result.document.SetAttribute( "reason", refused ? "the record says ueOnly" : "" );
        result.document.SetAttribute( "contentExposure", withheld ? "withheld" : "allowed" );
        result.document.SetAttribute( "exposureReason", withheld ? "the record carries noMachineLearning" : "" );
        return ASSETSTORE_STATUS_OK;
    }
    if ( CAssetPipelineOperations::PackList == request.operation )
    {
        CORE::CDataNode* items = result.document.AddChild( "items", GUCEF_DATATYPE_ARRAY );
        const char* ids[ 3 ] = { "plain", "nml-pack", "refused-pack" };
        for ( UInt32 i=0; i<3; ++i )
        {
            CORE::CDataNode* item = items->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            item->SetAttribute( "packId", ids[ i ] );
            item->SetAttribute( "title", CORE::CString( "Pack " ) + ids[ i ] );
        }
        result.document.SetAttribute( "total", (UInt32) 3 );
        return ASSETSTORE_STATUS_OK;
    }
    if ( CAssetPipelineOperations::IsJob( request.operation ) )
    {
        for ( UInt32 i=1; i<=steps; ++i )
        {
            if ( GUCEF_NULL != observer && observer->IsCancelRequested() )
            {
                result.message = "the conversion was cancelled";
                result.document.SetAttribute( "cancelled", true );
                return ASSETSTORE_STATUS_CANCELLED;
            }
            MT::ThreadDelay( stepDelayMs );
            if ( GUCEF_NULL != observer )
                observer->OnConversionProgress( i, steps, "Meshes/asset" + CORE::ToString( i ) + ".fbx" + ( echo.IsNULLOrEmpty() ? CORE::CString::Empty : " " + echo ) );
        }
        CORE::CDataNode* counts = result.document.AddChild( "counts", GUCEF_DATATYPE_OBJECT );
        counts->SetAttribute( "converted", steps );
        counts->SetAttribute( "failed", (UInt32) 2 );
        CORE::CDataNode* failures = result.document.AddChild( "failures", GUCEF_DATATYPE_ARRAY );
        for ( UInt32 f=0; f<3; ++f )
        {
            CORE::CDataNode* failure = failures->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            failure->SetAttribute( "assetPath", "Meshes/broken" + CORE::ToString( f ) + ".fbx" );
            failure->SetAttribute( "message", "no mesh" );
        }
        CORE::CDataNode* links = result.document.AddChild( "links", GUCEF_DATATYPE_ARRAY );
        CORE::CDataNode* link = links->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        link->SetAttribute( "uri", CAssetToolUris::ForPack( request.packId, "report" ) );
        link->SetAttribute( "name", "report" );
        link->SetAttribute( "mimeType", "application/json" );
        return ASSETSTORE_STATUS_OK;
    }
    if ( CAssetPipelineOperations::ReadResource == request.operation )
    {
        const CORE::CString uri = request.arguments.GetAttributeValueOrChildValueByName( "uri" ).AsString( CORE::CString::Empty, true );
        if ( 0 > uri.HasSubstr( "/plain/" ) && 0 > uri.HasSubstr( "manifest-reference" ) && 0 > uri.HasSubstr( "import-settings" ) )
        {
            result.document.SetAttribute( "errorCode", "not-found" );
            result.message = "no such resource";
            return ASSETSTORE_STATUS_NOT_FOUND;
        }
        result.document.SetAttribute( "uri", uri );
        result.document.SetAttribute( "mimeType", "application/json" );
        result.document.SetAttribute( "text", "{\"uri\":\"" + uri + "\",\"note\":\"" + echo + "\"}" );
        return ASSETSTORE_STATUS_OK;
    }
    if ( CAssetPipelineOperations::IsWrite( request.operation ) )
    {
        result.document.SetAttribute( "target", "manifests:" + request.packId + ".assetpack.json" );
        if ( !request.apply )
        {
            CORE::CString token;
            {
                MT::CScopeMutex lock( m_hookLock );
                token = "tok-" + CORE::ToString( ++m_tokens );
                m_openTokens.insert( token );
            }
            result.changeToken = token;
            result.document.SetAttribute( "changeToken", token );
            result.document.SetAttribute( "diff", "-old\n+new" );
            return ASSETSTORE_STATUS_OK;
        }
        bool known = false;
        {
            MT::CScopeMutex lock( m_hookLock );
            known = m_openTokens.end() != m_openTokens.find( request.changeToken );
            m_openTokens.erase( request.changeToken );
        }
        if ( !known )
        {
            result.document.SetAttribute( "errorCode", "change-token-invalid" );
            result.message = "the change token is not known";
            return ASSETSTORE_STATUS_FAILED;
        }
        result.document.SetAttribute( "written", true );
        return ASSETSTORE_STATUS_OK;
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/
