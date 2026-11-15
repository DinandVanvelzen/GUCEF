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
