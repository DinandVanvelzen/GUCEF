/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_COM_CMCPJOBINFO_H
#include "gucefCOM_CMcpJobInfo.h"
#define GUCEF_COM_CMCPJOBINFO_H
#endif /* GUCEF_COM_CMCPJOBINFO_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREADTOOL_H
#include "gucefASSETSTORE_CAssetStoreReadTool.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREADTOOL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREADTOOL_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINECHANGETOOL_H
#include "gucefASSETSTORE_CAssetPipelineChangeTool.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINECHANGETOOL_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINECHANGETOOL_H ? */

#include "gucefASSETSTORE_CAssetStoreWriteTool.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

void
SetList( CORE::CDataNode& object, const char* name, const CORE::CString::StringSet& values )
{GUCEF_TRACE;

    CORE::CDataNode* node = object.AddChild( name, GUCEF_DATATYPE_ARRAY );
    CORE::CString::StringSet::const_iterator i = values.begin();
    while ( i != values.end() )
    {
        node->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_STRING )->SetValue( CORE::CVariant( *i ) );
        ++i;
    }
}

CAssetCategoryRule
RuleOf( const CORE::CString& key, const CORE::CDataNode* value )
{GUCEF_TRACE;

    CAssetCategoryRule rule;
    rule.ruleId = key;
    if ( GUCEF_NULL != value )
    {
        rule.matchOn = value->GetAttributeValueOrChildValueByName( "matchOn" ).AsString( CORE::CString::Empty, true );
        rule.pattern = value->GetAttributeValueOrChildValueByName( "pattern" ).AsString( CORE::CString::Empty, true );
        rule.categoryId = value->GetAttributeValueOrChildValueByName( "categoryId" ).AsString( CORE::CString::Empty, true );
        rule.kind = value->GetAttributeValueOrChildValueByName( "kind" ).AsString( CORE::CString::Empty, true );
        CAssetToolBase::GetStringSetArgument( *value, "themes", rule.themes );
        CAssetToolBase::GetStringSetArgument( *value, "artStyles", rule.artStyles );
    }
    return rule;
}

/** "<sourceId>/<store category>" */
bool
SplitMapKey( const CORE::CString& key, CORE::CString& sourceId, CORE::CString& storeCategory )
{GUCEF_TRACE;

    const Int32 slash = key.HasChar( '/' );
    if ( slash <= 0 || (UInt32) slash + 1 >= key.Length() )
        return false;
    sourceId = key.SubstrToIndex( (UInt32) slash, true );
    storeCategory = key.CutChars( (UInt32) slash + 1, true );
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetStoreWriteTool::CAssetStoreWriteTool( CAssetToolContext& context, const CORE::CString& name )
    : CAssetToolBase( context, name )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreWriteTool::~CAssetStoreWriteTool()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreWriteTool::DigestOfChange( const CORE::CDataNode& arguments )
{GUCEF_TRACE;

    CORE::CDataNode copy( arguments );
    copy.DelAttribute( "apply" );
    copy.DelAttribute( "changeToken" );
    return CAssetToolContext::DigestArguments( copy );
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetStoreWriteTool::Preview( const COM::CMcpPrincipal& principal, const CORE::CDataNode& arguments, COM::CMcpToolResult& result )
{GUCEF_TRACE;

    CORE::CDataNode document( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* rows = document.AddChild( "items", GUCEF_DATATYPE_ARRAY );
    CORE::CDataNode payload( GUCEF_DATATYPE_OBJECT );
    if ( "store_classify" == m_name )
    {
        const CORE::CDataNode* items = arguments.FindChild( "items" );
        if ( GUCEF_NULL != items )
        {
            payload.AddChild( *items );
            CORE::CDataNode::const_iterator i = items->ConstBegin();
            while ( i != items->ConstEnd() )
            {
                const CORE::CString assetId = GetStringArgument( *(*i), "storeAssetId" );
                const CORE::CString packId = GetStringArgument( *(*i), "storePackId" );
                CORE::CDataNode* row = rows->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                CORE::CDataNode* current = row->AddChild( "current", GUCEF_DATATYPE_OBJECT );
                if ( !assetId.IsNULLOrEmpty() )
                {
                    row->SetAttribute( "storeAssetId", assetId );
                    CAssetRecord asset;
                    if ( ASSETSTORE_STATUS_OK == m_context.GetService().GetAsset( assetId, asset ) )
                        SetTextOrNull( *current, "categoryId", asset.categoryId );
                    else
                        row->SetAttribute( "problem", "there is no such asset" );
                }
                else
                {
                    row->SetAttribute( "storePackId", packId );
                    CAssetPackRecord pack;
                    if ( ASSETSTORE_STATUS_OK == m_context.GetService().GetPack( packId, pack ) )
                    {
                        SetList( *current, "categories", pack.categories );
                        SetList( *current, "themes", pack.themes );
                        SetList( *current, "artStyles", pack.artStyles );
                    }
                    else
                        row->SetAttribute( "problem", "there is no such pack" );
                }
                CORE::CDataNode* proposed = row->AddChild( **i );
                proposed->SetName( "new" );
                ++i;
            }
        }
    }
    else
    {
        const CORE::CDataNode* operations = arguments.FindChild( "operations" );
        if ( GUCEF_NULL != operations )
        {
            payload.AddChild( *operations );
            CORE::CDataNode::const_iterator i = operations->ConstBegin();
            while ( i != operations->ConstEnd() )
            {
                CORE::CDataNode* row = rows->AddChild( **i );
                const CORE::CString table = GetStringArgument( *(*i), "table" );
                const CORE::CString op = GetStringArgument( *(*i), "op" );
                if ( "category_rules" == table && "upsert" == op )
                {
                    CAssetServiceDocument counted;
                    const TAssetStoreStatus status = m_context.GetService().CountRuleMatches( RuleOf( GetStringArgument( *(*i), "key" ), (*i)->FindChild( "value" ) ), counted );
                    if ( ASSETSTORE_STATUS_OK == status )
                        row->SetAttribute( "matchesNow", counted.document.GetAttributeValueOrChildValueByName( "matches" ) );
                    else
                    {
                        SetNull( *row, "matchesNow" );
                        row->SetAttribute( "matchesProblem", CORE::CString( AssetStoreStatusToString( status ) ) + ": the service cannot count the matches of a rule" );
                    }
                }
                ++i;
            }
        }
    }
    Int64 expires = 0;
    const CORE::CString token = m_context.AddPendingChange( principal.subject, m_name, DigestOfChange( arguments ), payload, expires );
    if ( token.IsNULLOrEmpty() )
    {
        m_context.SetError( result, "failed", "No change token could be made", true );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    document.SetAttribute( "changeToken", token );
    document.SetAttribute( "expiresUtc", COM::CMcpJobInfo::UtcMsToText( expires ) );
    document.SetAttribute( "written", false );
    SetNull( document, "target" );
    document.SetAttribute( "hint", "Nothing is written yet. Show the changes to the owner; apply with apply true and this changeToken, with the same arguments." );
    SetStructured( result, document );
    return COM::MCP_EXECUTE_COMPLETE;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreWriteTool::ApplyClassify( const CORE::CString& commandId, const CORE::CDataNode& payload, CORE::CDataNode& document )
{GUCEF_TRACE;

    CORE::CDataNode* rows = document.AddChild( "items", GUCEF_DATATYPE_ARRAY );
    UInt32 written = 0;
    UInt32 unsupported = 0;
    const CORE::CDataNode* items = payload.FindChild( "items" );
    UInt32 index = 0;
    if ( GUCEF_NULL != items )
    {
        CORE::CDataNode::const_iterator i = items->ConstBegin();
        while ( i != items->ConstEnd() )
        {
            const CORE::CString assetId = GetStringArgument( *(*i), "storeAssetId" );
            const CORE::CString objectId = assetId.IsNULLOrEmpty() ? GetStringArgument( *(*i), "storePackId" ) : assetId;
            CAssetClassification classification;
            const CORE::CString categoryId = GetStringArgument( *(*i), "categoryId" );
            if ( !categoryId.IsNULLOrEmpty() )
                classification.categories.insert( categoryId );
            GetStringSetArgument( *(*i), "themes", classification.themes );
            const CORE::CString artStyle = GetStringArgument( *(*i), "artStyle" );
            if ( !artStyle.IsNULLOrEmpty() )
                classification.artStyles.insert( artStyle );
            CAssetServiceResult outcome;
            const TAssetStoreStatus status = m_context.GetService().SetClassification( commandId.IsNULLOrEmpty() ? CORE::CString::Empty : commandId + ":" + CORE::ToString( index ),
                                                                                     objectId, classification, "llm-assisted", outcome );
            CORE::CDataNode* row = rows->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            row->SetAttribute( "objectId", objectId );
            row->SetAttribute( "status", CORE::CString( AssetStoreStatusToString( status ) ) );
            if ( ASSETSTORE_STATUS_OK == status )
                ++written;
            else
            {
                if ( ASSETSTORE_STATUS_UNSUPPORTED == status )
                    ++unsupported;
                row->SetAttribute( "message", ( ASSETSTORE_STATUS_UNSUPPORTED == status ? CORE::CString( "unsupported: " ) : CORE::CString::Empty ) +
                    ( outcome.message.IsNULLOrEmpty() ? CORE::CString( "the service refused the classification" ) : outcome.message ) );
            }
            ++index;
            ++i;
        }
    }
    document.SetAttribute( "writtenCount", written );
    document.SetAttribute( "unsupportedCount", unsupported );
    document.SetAttribute( "written", written > 0 );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreWriteTool::ApplyRules( const CORE::CString& commandId, const CORE::CDataNode& payload, CORE::CDataNode& document )
{GUCEF_TRACE;

    CORE::CDataNode* rows = document.AddChild( "items", GUCEF_DATATYPE_ARRAY );
    UInt32 written = 0;
    UInt32 unsupported = 0;
    const CORE::CDataNode* operations = payload.FindChild( "operations" );
    UInt32 index = 0;
    if ( GUCEF_NULL != operations )
    {
        CORE::CDataNode::const_iterator i = operations->ConstBegin();
        while ( i != operations->ConstEnd() )
        {
            const CORE::CString table = GetStringArgument( *(*i), "table" );
            const CORE::CString op = GetStringArgument( *(*i), "op" );
            const CORE::CString key = GetStringArgument( *(*i), "key" );
            const CORE::CDataNode* value = (*i)->FindChild( "value" );
            const CORE::CString itemCommand = commandId.IsNULLOrEmpty() ? CORE::CString::Empty : commandId + ":" + CORE::ToString( index );
            CAssetServiceResult outcome;
            TAssetStoreStatus status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
            if ( "category_rules" == table )
            {
                if ( "upsert" == op )
                    status = m_context.GetService().SetCategoryRule( itemCommand, RuleOf( key, value ), outcome );
                else
                    status = m_context.GetService().RemoveCategoryRule( itemCommand, key, outcome );
            }
            else
            {
                CORE::CString sourceId;
                CORE::CString storeCategory;
                if ( !SplitMapKey( key, sourceId, storeCategory ) )
                    outcome.message = "a key of category_map is <sourceId>/<store category>";
                else
                if ( "upsert" == op )
                    status = m_context.GetService().SetCategoryMapping( itemCommand, sourceId, storeCategory,
                        GUCEF_NULL != value ? GetStringArgument( *value, "categoryId" ) : CORE::CString::Empty,
                        GUCEF_NULL != value ? GetStringArgument( *value, "theme" ) : CORE::CString::Empty,
                        GUCEF_NULL != value ? GetStringArgument( *value, "kind" ) : CORE::CString::Empty, outcome );
                else
                    status = m_context.GetService().RemoveCategoryMapping( itemCommand, sourceId, storeCategory, outcome );
            }
            CORE::CDataNode* row = rows->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            row->SetAttribute( "table", table );
            row->SetAttribute( "op", op );
            row->SetAttribute( "key", key );
            row->SetAttribute( "status", CORE::CString( AssetStoreStatusToString( status ) ) );
            if ( ASSETSTORE_STATUS_OK == status )
                ++written;
            else
            {
                if ( ASSETSTORE_STATUS_UNSUPPORTED == status )
                    ++unsupported;
                row->SetAttribute( "message", ( ASSETSTORE_STATUS_UNSUPPORTED == status ? CORE::CString( "unsupported: " ) : CORE::CString::Empty ) +
                    ( outcome.message.IsNULLOrEmpty() ? CORE::CString( "the service refused the change" ) : outcome.message ) );
            }
            ++index;
            ++i;
        }
    }
    document.SetAttribute( "writtenCount", written );
    document.SetAttribute( "unsupportedCount", unsupported );
    document.SetAttribute( "written", written > 0 );
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetStoreWriteTool::Compute( COM::CMcpRequestContext& context      ,
                               const CORE::CDataNode& arguments      ,
                               COM::CMcpToolResult& result           ,
                               COM::CMcpInputRequired& inputRequired ,
                               SCallState& state                     )
{GUCEF_TRACE;

    const COM::CMcpPrincipal& principal = context.GetPrincipal();
    const bool previewed = "store_classify" == m_name || "store_edit_rules" == m_name;
    const bool apply = GetBoolArgument( arguments, "apply", false );
    if ( previewed && !apply )
        return Preview( principal, arguments, result );

    const CORE::CString requestKey = GetStringArgument( arguments, "requestKey" );
    const CORE::CString digest = CAssetToolContext::DigestArguments( arguments );
    if ( !requestKey.IsNULLOrEmpty() )
    {
        CORE::CDataNode outcome;
        const CORE::TCommandIdempotencyLookup found = m_context.FindWrite( principal.subject, m_name, requestKey, digest, outcome );
        if ( CORE::COMMANDIDEMPOTENCY_HIT == found )
        {
            SetStructured( result, outcome );
            result.structuredContent.SetAttribute( "replayed", true );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        if ( CORE::COMMANDIDEMPOTENCY_CONFLICT == found )
        {
            m_context.SetError( result, "invalid-arguments", "The requestKey was used before with other arguments: use a new key", false );
            return COM::MCP_EXECUTE_COMPLETE;
        }
    }

    CORE::CDataNode payload;
    if ( previewed )
    {
        const CORE::CString token = GetStringArgument( arguments, "changeToken" );
        if ( token.IsNULLOrEmpty() )
        {
            m_context.SetError( result, "change-token-invalid", "An apply needs the changeToken of a preview: call without apply first", false );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        bool confirmed = false;
        const COM::EMcpExecuteResult confirmation = CAssetPipelineChangeTool::Confirm( m_context, context, m_name, "Write the " + m_name + " change to the primary store?", result, inputRequired, confirmed );
        if ( !confirmed )
            return confirmation;
        CORE::CString problem;
        if ( !m_context.TakePendingChange( token, principal.subject, m_name, DigestOfChange( arguments ), payload, problem ) )
        {
            m_context.SetError( result, "change-token-invalid", problem, false );
            return COM::MCP_EXECUTE_COMPLETE;
        }
    }

    CORE::CString nextAllowed;
    if ( !m_context.TakeWriteQuota( principal.subject, nextAllowed ) )
    {
        m_context.SetError( result, "busy", "The writes of this hour are used up; the next one is allowed at " + nextAllowed, true );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    const CORE::CString commandId = CAssetToolContext::MakeCommandId( principal.subject, m_name, requestKey );
    CORE::CDataNode document( GUCEF_DATATYPE_OBJECT );
    SetNull( document, "target" );
    SetNull( document, "changeToken" );

    if ( "store_classify" == m_name )
    {
        ApplyClassify( commandId, payload, document );
        if ( !document.GetAttributeValueOrChildValueByName( "written" ).AsBool( false, true ) &&
             document.GetAttributeValueOrChildValueByName( "unsupportedCount" ).AsUInt32( 0, true ) > 0 )
        {
            m_context.SetError( result, "unsupported", "unsupported: this service cannot set classifications yet (CPrimaryAssetStore::SetClassification() is not built); nothing was written", false );
            return COM::MCP_EXECUTE_COMPLETE;
        }
    }
    else
    if ( "store_edit_rules" == m_name )
    {
        ApplyRules( commandId, payload, document );
    }
    else
    {
        CAssetServiceResult outcome;
        TAssetStoreStatus status = ASSETSTORE_STATUS_UNSUPPORTED;
        CAssetSavedSearch search;
        if ( "store_save_search" == m_name )
        {
            search.name = GetStringArgument( arguments, "name" );
            // the id of a saved search is the caller's to give: one made of the request key (or the time), so a repeat gives the same id
            search.searchId = "mcp-" + CAssetToolContext::Sha256Hex( principal.subject + "\n" + search.name + "\n" +
                ( requestKey.IsNULLOrEmpty() ? CORE::ToString( (UInt64) CAssetToolContext::NowUtcMs() ) : requestKey ) ).SubstrToIndex( 16, true );
            const CORE::CDataNode* query = arguments.FindChild( "query" );
            if ( GUCEF_NULL != query )
                CAssetStoreReadTool::BuildQuery( *query, search.query );
            status = m_context.GetService().SaveSearch( commandId, search, outcome );
        }
        else
        if ( "store_delete_search" == m_name )
        {
            status = m_context.GetService().DeleteSavedSearch( commandId, GetStringArgument( arguments, "searchId" ), outcome );
        }
        else
        if ( "store_reclassify" == m_name )
        {
            CORE::CString::StringVector packIds;
            GetStringListArgument( arguments, "storePackIds", packIds );
            status = m_context.GetService().Reclassify( commandId, packIds, outcome );
        }
        if ( ASSETSTORE_STATUS_OK != status )
        {
            m_context.SetServiceError( result, status, outcome.message, GUCEF_NULL );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        document.SetAttribute( "written", true );
        document.SetAttribute( "replayedByService", outcome.replayed );
        if ( "store_save_search" == m_name )
            document.SetAttribute( "searchId", outcome.id.IsNULLOrEmpty() ? search.searchId : outcome.id );
        SetTextOrNull( document, "message", outcome.message );
    }
    SetStructured( result, document );
    if ( !requestKey.IsNULLOrEmpty() )
        m_context.StoreWrite( principal.subject, m_name, requestKey, digest, document );
    return COM::MCP_EXECUTE_COMPLETE;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
