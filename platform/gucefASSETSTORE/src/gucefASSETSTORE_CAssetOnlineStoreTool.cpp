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

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#include "gucefASSETSTORE_CAssetOnlineStoreTool.h"

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
SetTextList( CORE::CDataNode& object, const char* name, const CORE::CString::StringSet& values )
{GUCEF_TRACE;

    CORE::CDataNode* node = object.AddChild( name, GUCEF_DATATYPE_ARRAY );
    CORE::CString::StringSet::const_iterator i = values.begin();
    while ( i != values.end() )
    {
        node->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_STRING )->SetValue( CORE::CVariant( *i ) );
        ++i;
    }
}

/** "o<offset>" */
bool
ParseOffset( const CORE::CString& cursor, UInt32& offset )
{GUCEF_TRACE;

    offset = 0;
    if ( cursor.IsNULLOrEmpty() )
        return true;
    if ( cursor.Length() < 2 || 'o' != cursor[ 0 ] )
        return false;
    for ( UInt32 i=1; i<cursor.Length(); ++i )
        if ( cursor[ i ] < '0' || cursor[ i ] > '9' )
            return false;
    offset = CORE::StringToUInt32( cursor.CutChars( 1, true ) );
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetOnlineStoreTool::CAssetOnlineStoreTool( CAssetToolContext& context, const CORE::CString& name )
    : CAssetToolBase( context, name )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetOnlineStoreTool::~CAssetOnlineStoreTool()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetOnlineStoreTool::SetStoreError( TAssetStoreStatus status, const CORE::CString& storeId, COM::CMcpToolResult& result )
{GUCEF_TRACE;

    if ( ASSETSTORE_STATUS_NOT_SIGNED_IN == status || ASSETSTORE_STATUS_SESSION_EXPIRED == status )
    {
        m_context.SetError( result, "not-signed-in", "The store " + storeId + " is not signed in: ask the owner to sign in through the viewer, then try again", false );
        return;
    }
    if ( ASSETSTORE_STATUS_NOT_FOUND == status )
    {
        m_context.SetError( result, "not-found", "The store " + storeId + " or the item is not known", false );
        return;
    }
    m_context.SetServiceError( result, status, CORE::CString::Empty, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetOnlineStoreTool::Compute( COM::CMcpRequestContext& context      ,
                                const CORE::CDataNode& arguments      ,
                                COM::CMcpToolResult& result           ,
                                COM::CMcpInputRequired& inputRequired ,
                                SCallState& state                     )
{GUCEF_TRACE;

    CIAssetStoreService& service = m_context.GetService();
    CORE::CDataNode document( GUCEF_DATATYPE_OBJECT );

    if ( "online_stores" == m_name )
    {
        CAssetStoreInfoPage stores;
        const TAssetStoreStatus status = service.ListStores( stores );
        if ( ASSETSTORE_STATUS_OK != status )
        {
            m_context.SetServiceError( result, status, CORE::CString::Empty, GUCEF_NULL );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        CORE::CDataNode* items = document.AddChild( "items", GUCEF_DATATYPE_ARRAY );
        for ( size_t i=0; i<stores.entries.size(); ++i )
        {
            CORE::CDataNode* item = items->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            item->SetAttribute( "storeId", stores.entries[ i ].storeId );
            item->SetAttribute( "displayName", stores.entries[ i ].displayName );
            item->SetAttribute( "signedIn", stores.entries[ i ].signedIn );
        }
        document.SetAttribute( "total", (UInt32) stores.entries.size() );
        SetNull( document, "nextCursor" );
        SetStructured( result, document );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    if ( "online_library" == m_name )
    {
        const CORE::CString storeId = GetStringArgument( arguments, "storeId" );
        CAssetStoreQuery query;
        query.text = GetStringArgument( arguments, "text" );
        query.listingType = GetStringArgument( arguments, "listingType" );
        query.formatGroup = GetStringArgument( arguments, "formatGroup" );
        query.sort = GetStringArgument( arguments, "sort" );
        query.cursor = GetStringArgument( arguments, "cursor" );
        CAssetStoreLibraryPage page;
        const TAssetStoreStatus status = service.QueryLibrary( storeId, query, page );
        if ( ASSETSTORE_STATUS_OK != status )
        {
            SetStoreError( status, storeId, result );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        const UInt32 limit = m_context.GetPageLimit( GetUInt32Argument( arguments, "limit", 0 ) );
        CORE::CDataNode* items = document.AddChild( "items", GUCEF_DATATYPE_ARRAY );
        for ( size_t i=0; i<page.items.size() && i<limit; ++i )
        {
            const CAssetStoreItem& storeItem = page.items[ i ];
            CORE::CDataNode* item = items->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            item->SetAttribute( "itemId", storeItem.itemId );
            item->SetAttribute( "title", storeItem.title );
            item->SetAttribute( "seller", storeItem.seller );
            item->SetAttribute( "listingType", storeItem.listingType );
            CORE::CDataNode* formats = item->AddChild( "formats", GUCEF_DATATYPE_ARRAY );
            for ( size_t f=0; f<storeItem.formats.size(); ++f )
                formats->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_STRING )->SetValue( CORE::CVariant( storeItem.formats[ f ].name ) );
            item->SetAttribute( "licenceName", storeItem.licenceName );
            SetTextList( *item, "restrictions", storeItem.licenceRestrictions );
            SetNull( *item, "inPrimaryStore" );
        }
        SetTextOrNull( document, "nextCursor", page.nextCursor );
        SetNull( document, "total" );
        SetStructured( result, document );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    if ( "online_item" == m_name )
    {
        const CORE::CString storeId = GetStringArgument( arguments, "storeId" );
        CAssetStoreItem storeItem;
        const TAssetStoreStatus status = service.GetStoreItem( storeId, GetStringArgument( arguments, "itemId" ), storeItem );
        if ( ASSETSTORE_STATUS_OK != status )
        {
            SetStoreError( status, storeId, result );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        document.SetAttribute( "storeId", storeItem.providerId );
        document.SetAttribute( "itemId", storeItem.itemId );
        document.SetAttribute( "title", storeItem.title );
        document.SetAttribute( "seller", storeItem.seller );
        document.SetAttribute( "listingType", storeItem.listingType );
        SetTextOrNull( document, "version", storeItem.version );
        SetTextOrNull( document, "url", CORE::CRedactor::ForUrl().RedactUrl( storeItem.url ) );
        CORE::CDataNode* licence = document.AddChild( "licence", GUCEF_DATATYPE_OBJECT );
        licence->SetAttribute( "name", storeItem.licenceName );
        SetTextOrNull( *licence, "url", CORE::CRedactor::ForUrl().RedactUrl( storeItem.licenceUrl ) );
        SetTextList( *licence, "restrictions", storeItem.licenceRestrictions );
        if ( !storeItem.licenceText.IsNULLOrEmpty() )
            licence->AddChild( MakeUntrusted( "licence-text", storeItem.licenceText ) )->SetName( "text" );
        CORE::CDataNode* formats = document.AddChild( "formats", GUCEF_DATATYPE_ARRAY );
        for ( size_t f=0; f<storeItem.formats.size(); ++f )
        {
            CORE::CDataNode* format = formats->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            format->SetAttribute( "formatId", storeItem.formats[ f ].formatId );
            format->SetAttribute( "name", storeItem.formats[ f ].name );
            format->SetAttribute( "group", storeItem.formats[ f ].group );
            CORE::CDataNode* files = format->AddChild( "files", GUCEF_DATATYPE_ARRAY );
            for ( size_t n=0; n<storeItem.formats[ f ].files.size(); ++n )
            {
                CORE::CDataNode* file = files->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                file->SetAttribute( "name", storeItem.formats[ f ].files[ n ].name );
                file->SetAttribute( "size", storeItem.formats[ f ].files[ n ].size );
            }
        }
        CORE::CDataNode* categories = document.AddChild( "categories", GUCEF_DATATYPE_ARRAY );
        for ( size_t c=0; c<storeItem.categories.size() && c<32; ++c )
            categories->AddChild( MakeUntrusted( "store-category", storeItem.categories[ c ] ) );
        document.AddChild( MakeUntrusted( "store-description", storeItem.description ) )->SetName( "description" );
        SetStructured( result, document );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    if ( "settings_get" == m_name )
    {
        // a machine caller: the values that are owner-only-read are not asked for, and it may not change settings
        CAssetSettingsRequest request;
        request.caller = "mcp";
        request.callerName = "MCP tools";
        request.remoteClient = true;
        request.fullRead = false;
        request.mayChange = false;
        CAssetSettingsDescription description;
        const TAssetStoreStatus status = service.DescribeSettings( request, description );
        if ( ASSETSTORE_STATUS_OK != status )
        {
            m_context.SetServiceError( result, status, description.message, GUCEF_NULL );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        const CORE::CString group = GetStringArgument( arguments, "group" );
        const CORE::CString key = GetStringArgument( arguments, "key" );
        CORE::CDataNode* settings = document.AddChild( "settings", GUCEF_DATATYPE_ARRAY );
        for ( size_t e=0; e<description.entries.size(); ++e )
        {
            const CAssetSettingState& entry = description.entries[ e ];
            if ( !entry.descriptor.IsPublic() || ( !group.IsNULLOrEmpty() && !group.Equals( entry.descriptor.group, false ) ) ||
                 ( !key.IsNULLOrEmpty() && key != entry.descriptor.key ) )
                continue;
            CORE::CDataNode* setting = settings->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            setting->SetAttribute( "key", entry.descriptor.key );
            setting->SetAttribute( "group", entry.descriptor.group );
            setting->SetAttribute( "type", entry.descriptor.type );
            setting->SetAttribute( "range", entry.descriptor.GetRangeText() );
            setting->SetAttribute( "default", entry.descriptor.defaultValue );
            setting->SetAttribute( "unit", entry.descriptor.unit );
            setting->SetAttribute( "description", entry.descriptor.description );
            setting->SetAttribute( "applyMode", entry.descriptor.applyMode );
            setting->SetAttribute( "value", entry.value );
            if ( entry.hasPendingValue )
                setting->SetAttribute( "valueAfterRestart", entry.pendingValue );
            setting->SetAttribute( "source", entry.source );
        }
        document.SetAttribute( "changesAllowed", description.changesAllowed );
        document.SetAttribute( "revision", description.revision );
        SetStructured( result, document );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    if ( "sync_marks" == m_name )
    {
        const CORE::CString storeId = GetStringArgument( arguments, "storeId" );
        UInt32 offset = 0;
        if ( !ParseOffset( GetStringArgument( arguments, "cursor" ), offset ) )
        {
            m_context.SetError( result, "invalid-arguments", "The cursor is not one this tool gave", false );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        const UInt32 limit = m_context.GetPageLimit( GetUInt32Argument( arguments, "limit", 0 ) );
        CAssetSyncMarkPage marks;
        const TAssetStoreStatus status = service.GetSyncMarks( storeId, CIAssetStoreService::MakePageToken( offset ), limit, marks );
        if ( ASSETSTORE_STATUS_OK != status )
        {
            SetStoreError( status, storeId, result );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        CORE::CDataNode* items = document.AddChild( "items", GUCEF_DATATYPE_ARRAY );
        for ( size_t m=0; m<marks.entries.size(); ++m )
        {
            CORE::CDataNode* item = items->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            item->SetAttribute( "itemId", marks.entries[ m ].itemId );
            item->SetAttribute( "mark", marks.entries[ m ].mark );
            item->SetAttribute( "markedMs", marks.entries[ m ].markedMs );
            item->AddChild( MakeUntrusted( "sync-mark-reason", marks.entries[ m ].reason ) )->SetName( "reason" );
        }
        document.SetAttribute( "total", marks.total );
        if ( !marks.nextPageToken.IsNULLOrEmpty() )
            document.SetAttribute( "nextCursor", "o" + marks.nextPageToken );
        else
            SetNull( document, "nextCursor" );
        SetStructured( result, document );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    if ( "sync_status" == m_name )
    {
        // the job: by its id, or the last sync job of the store (one that runs first)
        CORE::CString jobId = GetStringArgument( arguments, "jobId" );
        const CORE::CString storeId = GetStringArgument( arguments, "storeId" ).Lowercase();
        if ( jobId.IsNULLOrEmpty() )
        {
            CAssetJobPage jobs;
            if ( ASSETSTORE_STATUS_OK == service.ListJobs( CORE::CString::Empty, CIAssetStoreService::MaxPageEntries, jobs ) )
            {
                UInt64 newest = 0;
                bool running = false;
                for ( size_t j=0; j<jobs.entries.size(); ++j )
                {
                    const CAssetJobEntry& job = jobs.entries[ j ];
                    if ( "sync" != job.subKind || ( !storeId.IsNULLOrEmpty() && storeId != job.providerId ) )
                        continue;
                    const bool jobRunning = !job.IsEnded();
                    if ( jobId.IsNULLOrEmpty() || ( jobRunning && !running ) || ( jobRunning == running && job.createdAtMs >= newest ) )
                    {
                        jobId = job.jobId;
                        newest = job.createdAtMs;
                        running = jobRunning;
                    }
                }
            }
        }
        if ( jobId.IsNULLOrEmpty() )
        {
            m_context.SetError( result, "not-found", storeId.IsNULLOrEmpty() ? CORE::CString( "Name a jobId or a storeId" ) : "There is no sync job of the store " + storeId, false );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        CAssetJobEntry job;
        TAssetStoreStatus status = service.GetJob( jobId, job );
        if ( ASSETSTORE_STATUS_OK != status || "sync" != job.subKind )
        {
            m_context.SetError( result, "not-found", "There is no sync job " + jobId, false );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        UInt32 offset = 0;
        if ( !ParseOffset( GetStringArgument( arguments, "cursor" ), offset ) )
        {
            m_context.SetError( result, "invalid-arguments", "The cursor is not one this tool gave", false );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        CORE::CDataNode* jobNode = document.AddChild( "job", GUCEF_DATATYPE_OBJECT );
        jobNode->SetAttribute( "jobId", job.jobId );
        jobNode->SetAttribute( "store", job.providerId );
        jobNode->SetAttribute( "state", CORE::CString( AssetJobStateToString( job.state ) ) );
        jobNode->SetAttribute( "itemsDone", job.itemsDone );
        jobNode->SetAttribute( "itemsTotal", job.itemsTotal );
        SetTextOrNull( *jobNode, "stateText", m_context.GetRedaction().ForResult( job.stateText ) );
        SetTextOrNull( *jobNode, "message", m_context.GetRedaction().ForResult( job.message ) );
        CAssetServiceDocument plan;
        if ( ASSETSTORE_STATUS_OK == service.GetJobResult( jobId, "plan", CORE::CString::Empty, 0, plan ) )
        {
            const CORE::CDataNode* planNode = plan.document.FindChild( "plan" );
            if ( GUCEF_NULL != planNode )
            {
                CORE::CDataNode* counts = document.AddChild( "plan", GUCEF_DATATYPE_OBJECT );
                counts->SetAttribute( "listed", planNode->GetAttributeValueOrChildValueByName( "listed" ).AsUInt32( 0, true ) );
                counts->SetAttribute( "queuedDownloads", planNode->GetAttributeValueOrChildValueByName( "queuedDownloads" ).AsUInt32( 0, true ) );
                counts->SetAttribute( "estimatedBytes", planNode->GetAttributeValueOrChildValueByName( "estimatedBytes" ).AsUInt64( 0, true ) );
                const CORE::CDataNode* byState = planNode->FindChild( "counts" );
                if ( GUCEF_NULL != byState )
                    counts->AddChild( *byState );
                const CORE::CDataNode* storage = planNode->FindChild( "storage" );
                if ( GUCEF_NULL != storage )
                    counts->AddChild( *storage );
            }
            const CORE::CDataNode* run = plan.document.FindChild( "run" );
            if ( GUCEF_NULL != run )
                document.AddChild( *run );
        }
        const UInt32 limit = m_context.GetPageLimit( GetUInt32Argument( arguments, "limit", 0 ) );
        CAssetServiceDocument lines;
        CORE::CDataNode* items = document.AddChild( "items", GUCEF_DATATYPE_ARRAY );
        status = service.GetJobResult( jobId, "items", CIAssetStoreService::MakePageToken( offset ), limit, lines );
        const CORE::CDataNode* lineNodes = ASSETSTORE_STATUS_OK == status ? lines.document.FindChild( "items" ) : GUCEF_NULL;
        if ( GUCEF_NULL != lineNodes )
        {
            for ( CORE::CDataNode::const_iterator l = lineNodes->ConstBegin(); l != lineNodes->ConstEnd(); ++l )
            {
                CORE::CDataNode* item = items->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                item->SetAttribute( "itemId", (*l)->GetAttributeValueOrChildValueByName( "itemId" ).AsString() );
                item->SetAttribute( "state", (*l)->GetAttributeValueOrChildValueByName( "state" ).AsString() );
                SetTextOrNull( *item, "reason", (*l)->GetAttributeValueOrChildValueByName( "reason" ).AsString() );
                item->AddChild( MakeUntrusted( "store-title", (*l)->GetAttributeValueOrChildValueByName( "title" ).AsString() ) )->SetName( "title" );
            }
        }
        if ( !lines.nextPageToken.IsNULLOrEmpty() )
            document.SetAttribute( "nextCursor", "o" + lines.nextPageToken );
        else
            SetNull( document, "nextCursor" );
        SetStructured( result, document );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    // transfer_queue
    UInt32 offset = 0;
    if ( !ParseOffset( GetStringArgument( arguments, "cursor" ), offset ) )
    {
        m_context.SetError( result, "invalid-arguments", "The cursor is not one this tool gave", false );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    CAssetJobPage jobs;
    const TAssetStoreStatus status = service.ListJobs( CORE::CString::Empty, CIAssetStoreService::MaxPageEntries, jobs );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        m_context.SetServiceError( result, status, CORE::CString::Empty, GUCEF_NULL );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    const CORE::CString stateFilter = GetStringArgument( arguments, "state" );
    const UInt32 limit = m_context.GetPageLimit( GetUInt32Argument( arguments, "limit", 0 ) );
    CORE::CDataNode* items = document.AddChild( "items", GUCEF_DATATYPE_ARRAY );
    UInt32 matched = 0;
    for ( size_t i=0; i<jobs.entries.size(); ++i )
    {
        const CAssetJobEntry& job = jobs.entries[ i ];
        const CORE::CString state = AssetJobStateToString( job.state );
        if ( !stateFilter.IsNULLOrEmpty() && stateFilter != state )
            continue;
        ++matched;
        if ( matched <= offset || matched > offset + limit )
            continue;
        CORE::CDataNode* item = items->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        item->SetAttribute( "jobId", job.jobId );
        item->SetAttribute( "kind", job.kind );
        item->SetAttribute( "name", job.name );
        SetTextOrNull( *item, "store", job.providerId );
        item->SetAttribute( "state", state );
        item->SetAttribute( "doneBytes", job.doneBytes );
        item->SetAttribute( "totalBytes", job.totalBytes );
        item->SetAttribute( "bytesPerSecond", job.bytesPerSecond );
        item->SetAttribute( "secondsLeft", job.secondsLeft );
        item->SetAttribute( "itemsDone", job.itemsDone );
        item->SetAttribute( "itemsTotal", job.itemsTotal );
        SetTextOrNull( *item, "message", job.message );
    }
    document.SetAttribute( "total", matched );
    if ( offset + limit < matched )
        document.SetAttribute( "nextCursor", "o" + CORE::ToString( offset + limit ) );
    else
        SetNull( document, "nextCursor" );
    SetStructured( result, document );
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
