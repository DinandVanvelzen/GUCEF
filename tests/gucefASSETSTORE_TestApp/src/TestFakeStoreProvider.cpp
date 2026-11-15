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

#include <algorithm>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#include "gucefASSETSTORE_CAssetStoreJson.h"
#define GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREJSON_H ? */
#include "TestFakeStoreProvider.h"

using namespace GUCEF;

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The server of the fake providers: test state, set by the tests before the providers are made */
CFakeStoreServer* g_fakeStoreServer = GUCEF_NULL;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CFakeStoreProvider::SetServer( CFakeStoreServer* server )
{
    g_fakeStoreServer = server;
}

/*-------------------------------------------------------------------------*/

CFakeStoreProvider::CFakeStoreProvider( void )
    : ASSETSTORE::CIAssetStoreProvider()
    , m_providerId( "fakestore" )
    , m_token( CORE::CString( "stale-token" ) )
    , m_planCount( 0 )
    , m_renewCount( 0 )
    , m_lock()
{
}

/*-------------------------------------------------------------------------*/

CFakeStoreProvider::~CFakeStoreProvider()
{
    WaitForAsyncOperations();
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

void
CFakeStoreProvider::SetProviderId( const CORE::CString& providerId )
{
    MT::CScopeMutex lock( m_lock );
    m_providerId = providerId;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFakeStoreProvider::GetProviderId( void ) const
{
    return m_providerId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeStoreProvider::GetDisplayName( void ) const
{
    return "Fake store " + m_providerId;
}

/*-------------------------------------------------------------------------*/

bool
CFakeStoreProvider::IsSignedIn( void ) const
{
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeStoreProvider::GetPlanCount( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_planCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeStoreProvider::GetRenewCount( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_renewCount;
}

/*-------------------------------------------------------------------------*/

void
CFakeStoreProvider::SetToken( const CORE::CString& token )
{
    MT::CScopeMutex lock( m_lock );
    m_token.Set( token );
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CFakeStoreProvider::Get( const CORE::CString& path, CORE::CDataNode& json ) const
{
    if ( GUCEF_NULL == g_fakeStoreServer )
        return ASSETSTORE::ASSETSTORE_STATUS_NOT_SIGNED_IN;
    WEB::CHTTPClient client;
    WEB::CHttpRequestData request;
    request.SetUrl( g_fakeStoreServer->GetBaseUrl() + path );
    request.requestType = "GET";
    WEB::CHttpResponseData response;
    if ( !client.Execute( request, response ) )
        return ASSETSTORE::ASSETSTORE_STATUS_NETWORK_ERROR;
    if ( 404 == response.statusCode )
        return ASSETSTORE::ASSETSTORE_STATUS_NOT_FOUND;
    if ( 200 != response.statusCode )
        return ASSETSTORE::ASSETSTORE_STATUS_HTTP_ERROR;
    CORE::CString text;
    text.Set( response.content.AsConstTypePtr< char >(), response.content.GetDataSize() );
    return ASSETSTORE::CAssetStoreJson::FromText( text, json ) ? ASSETSTORE::ASSETSTORE_STATUS_OK : ASSETSTORE::ASSETSTORE_STATUS_FAILED;
}

/*-------------------------------------------------------------------------*/

bool
CFakeStoreProvider::RenewSession( void )
{
    {
        MT::CScopeMutex lock( m_lock );
        ++m_renewCount;
    }
    if ( GUCEF_NULL == g_fakeStoreServer )
        return false;
    WEB::CHTTPClient client;
    WEB::CHttpRequestData request;
    request.SetUrl( g_fakeStoreServer->GetBaseUrl() + "/token" );
    request.requestType = "POST";
    WEB::CHttpResponseData response;
    if ( !client.Execute( request, response ) || 200 != response.statusCode )
        return false;
    CORE::CString token;
    token.Set( response.content.AsConstTypePtr< char >(), response.content.GetDataSize() );
    SetToken( token );
    return true;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CFakeStoreProvider::QueryLibrary( const ASSETSTORE::CAssetStoreQuery& query, ASSETSTORE::CAssetStoreLibraryPage& page )
{
    CORE::CDataNode json;
    const ASSETSTORE::TAssetStoreStatus status = Get( "/library?cursor=" + query.cursor + "&pageSize=" + CORE::ToString( query.pageSize ) +
                                                      "&text=" + query.text + "&type=" + query.listingType, json );
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status )
        return status;
    CORE::CDataNodeSerializableSettings settings;
    return page.Deserialize( json, settings ) ? ASSETSTORE::ASSETSTORE_STATUS_OK : ASSETSTORE::ASSETSTORE_STATUS_FAILED;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CFakeStoreProvider::GetItem( const CORE::CString& itemId, ASSETSTORE::CAssetStoreItem& item )
{
    CORE::CDataNode json;
    const ASSETSTORE::TAssetStoreStatus status = Get( "/items/" + itemId, json );
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status )
        return status;
    CORE::CDataNodeSerializableSettings settings;
    if ( !item.Deserialize( json, settings ) )
        return ASSETSTORE::ASSETSTORE_STATUS_FAILED;
    item.providerId = m_providerId;
    return ASSETSTORE::ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CFakeStoreProvider::PlanDownload( const CORE::CString& itemId, const CORE::CString& formatId,
                                  const CORE::CString::StringVector& fileIds, ASSETSTORE::CAssetStoreDownloadPlan& plan )
{
    {
        MT::CScopeMutex lock( m_lock );
        ++m_planCount;
    }
    ASSETSTORE::CAssetStoreItem item;
    const ASSETSTORE::TAssetStoreStatus status = GetItem( itemId, item );
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status )
        return status;
    const ASSETSTORE::CAssetStoreFormat* format = item.FindFormat( formatId );
    if ( GUCEF_NULL == format )
        return ASSETSTORE::ASSETSTORE_STATUS_NOT_FOUND;
    plan = ASSETSTORE::CAssetStoreDownloadPlan();
    plan.providerId = m_providerId;
    plan.itemId = itemId;
    plan.formatId = formatId;
    plan.fileIds = fileIds;
    for ( size_t f=0; f<format->files.size(); ++f )
    {
        const ASSETSTORE::CAssetStoreFile& file = format->files[ f ];
        if ( !fileIds.empty() && fileIds.end() == std::find( fileIds.begin(), fileIds.end(), file.fileId ) )
            continue;
        ASSETSTORE::CAssetStoreTransfer transfer;
        transfer.fileId = file.fileId;
        transfer.url = g_fakeStoreServer->GetBaseUrl() + "/files/" + file.fileId;
        transfer.size = file.size;
        transfer.digest = file.digest;
        transfer.digestAlgorithm = file.digestAlgorithm;
        transfer.targetName = file.name;
        plan.transfers.push_back( transfer );
    }
    return ASSETSTORE::ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

bool
CFakeStoreProvider::AuthorizeTransferRequest( const ASSETSTORE::CAssetStoreTransfer& transfer, WEB::CHttpRequestData& request )
{
    MT::CScopeMutex lock( m_lock );
    request.otherHeaders[ "Authorization" ] = "Bearer " + m_token.Reveal();
    return true;
}

/*-------------------------------------------------------------------------*/

CFakeStoreProviderFactory::CFakeStoreProviderFactory( const CORE::CString& providerId )
    : TBase()
    , m_providerId( providerId )
{
}

/*-------------------------------------------------------------------------*/

CFakeStoreProviderFactory::~CFakeStoreProviderFactory()
{
}

/*-------------------------------------------------------------------------*/

CFakeStoreProviderFactory::TProductPtr
CFakeStoreProviderFactory::Create( void )
{
    TProductPtr product = TBase::Create();
    CFakeStoreProvider* provider = static_cast< CFakeStoreProvider* >( product.GetPointerAlways() );
    if ( GUCEF_NULL != provider )
        provider->SetProviderId( m_providerId );
    return product;
}

/*-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/
