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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */
#ifndef GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#include "gucefWEB_CIHttpServerRequestHandler.h"
#define GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#endif /* GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#include "gucefASSETSTORE_CAssetStoreJson.h"
#define GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREJSON_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORELIBRARYPAGE_H
#include "gucefASSETSTORE_CAssetStoreLibraryPage.h"
#define GUCEF_ASSETSTORE_CASSETSTORELIBRARYPAGE_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORELIBRARYPAGE_H ? */
#include "TestFakeStoreServer.h"

using namespace GUCEF;

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CFakeStoreHandler : public WEB::CIHttpServerRequestHandler
{
    public:

    CFakeStoreHandler( void ) : WEB::CIHttpServerRequestHandler(), m_server( GUCEF_NULL ) {}
    virtual ~CFakeStoreHandler() {}

    void SetServer( CFakeStoreServer* server ) { m_server = server; }

    virtual bool OnRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL == m_server )
        {
            response.statusCode = 503;
            return true;
        }
        m_server->HandleRequest( request, response );
        return true;
    }

    virtual bool OnRead( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnReadMetaData( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnUpdate( const WEB::CHttpRequestData&, bool, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnCreate( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnDelete( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool SetRouterController( WEB::CIHTTPServerRouterController* ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual WEB::CIHTTPServerRouterController* GetRouterController( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    private:

    CFakeStoreServer* m_server;
};

/*-------------------------------------------------------------------------*/

class CFakeStoreHandlerFactory : public CORE::CTFactory< WEB::CIHttpServerRequestHandler, CFakeStoreHandler, MT::CMutex >
{
    public:

    typedef CORE::CTFactory< WEB::CIHttpServerRequestHandler, CFakeStoreHandler, MT::CMutex > TBase;

    explicit CFakeStoreHandlerFactory( CFakeStoreServer* server ) : TBase(), m_server( server ) {}
    virtual ~CFakeStoreHandlerFactory() {}

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        TProductPtr product = TBase::Create();
        CFakeStoreHandler* handler = static_cast< CFakeStoreHandler* >( product.GetPointerAlways() );
        if ( GUCEF_NULL != handler )
            handler->SetServer( m_server );
        return product;
    }

    private:

    CFakeStoreHandlerFactory( const CFakeStoreHandlerFactory& src );
    CFakeStoreHandlerFactory& operator=( const CFakeStoreHandlerFactory& src );

    CFakeStoreServer* m_server;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CString::StringMap
ParseQuery( const CORE::CString& query )
{
    CORE::CString::StringMap parameters;
    const CORE::CString::StringVector pairs = query.ParseElements( '&', false );
    for ( size_t p=0; p<pairs.size(); ++p )
        parameters[ pairs[ p ].SubstrToChar( '=', true ) ] = pairs[ p ].SubstrToChar( '=', false, true );
    return parameters;
}

CORE::CString
ValueOf( const CORE::CString::StringMap& parameters, const char* name )
{
    CORE::CString::StringMap::const_iterator i = parameters.find( name );
    return i == parameters.end() ? CORE::CString::Empty : (*i).second;
}

CORE::CString
HeaderOf( const WEB::CHttpRequestData& request, const char* name )
{
    WEB::CHttpRequestData::TStringMap::const_iterator i = request.otherHeaders.begin();
    while ( i != request.otherHeaders.end() )
    {
        if ( (*i).first.Equals( name, false ) )
            return (*i).second;
        ++i;
    }
    return CORE::CString::Empty;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFakeFileBehaviour::CFakeFileBehaviour( void )
    : supportsRange( true )
    , fail503( 0 )
    , fail429( 0 )
    , retryAfterSeconds( 1 )
    , expireFirst( 0 )
    , corrupt( false )
{
}

/*-------------------------------------------------------------------------*/

CFakeStoreStats::CFakeStoreStats( void )
    : libraryRequests( 0 )
    , itemRequests( 0 )
    , fileRequests( 0 )
    , rangeRequests( 0 )
    , refusedSessions( 0 )
    , tokensIssued( 0 )
    , expiredAnswers( 0 )
    , rateLimitedAnswers( 0 )
    , unavailableAnswers( 0 )
{
}

/*-------------------------------------------------------------------------*/

CFakeStoreServer::CFakeStoreServer( void )
    : MT::CActiveObject()
    , m_lock()
    , m_pulseGenerator( CORE::CPulseGenerator::CreateSharedObj() )
    , m_server( GUCEF_NULL )
    , m_factory( GUCEF_NULL )
    , m_port( 0 )
    , m_running( false )
    , m_items()
    , m_contents()
    , m_behaviours()
    , m_requireAuth( false )
    , m_tokenEndpointWorks( true )
    , m_currentToken( "token-0" )
    , m_stats()
    , m_libraryFailures( 0 )
    , m_listingWithoutFormats( false )
{
}

/*-------------------------------------------------------------------------*/

CFakeStoreServer::~CFakeStoreServer()
{
    Stop();
}

/*-------------------------------------------------------------------------*/

bool
CFakeStoreServer::Start( void )
{
    if ( m_running )
        return true;
    CFakeStoreHandlerFactory* factory = GUCEF_NEW CFakeStoreHandlerFactory( this );
    m_factory = factory;
    WEB::CHttpServerSettings settings;
    settings.bindAddress = "127.0.0.1";
    settings.keepAliveConnections = false;
    settings.processRequestsAsync = false;
    settings.allowWebSocketUpgrades = false;
    settings.maxClientConnections = 64;
    m_server = GUCEF_NEW WEB::CHTTPServer( m_pulseGenerator, factory, &settings );
    if ( !m_server->ListenOnPort( 0 ) )
        return false;
    m_port = m_server->GetBoundPort();
    m_running = true;
    return Activate( GUCEF_NULL, 1, 1 );
}

/*-------------------------------------------------------------------------*/

void
CFakeStoreServer::Stop( void )
{
    if ( !m_running )
        return;
    m_running = false;
    Deactivate( false, true );
    if ( GUCEF_NULL != m_server )
    {
        m_server->Close();
        GUCEF_DELETE m_server;
        m_server = GUCEF_NULL;
    }
    if ( GUCEF_NULL != m_factory )
    {
        GUCEF_DELETE static_cast< CFakeStoreHandlerFactory* >( m_factory );
        m_factory = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CFakeStoreServer::AsLockable( void ) const
{
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CFakeStoreServer::OnThreadStart( void* taskdata )
{
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeStoreServer::OnThreadCycle( void* taskdata )
{
    m_pulseGenerator->ForceDirectPulse();
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeStoreServer::GetBaseUrl( void ) const
{
    return "http://127.0.0.1:" + CORE::ToString( m_port );
}

/*-------------------------------------------------------------------------*/

void
CFakeStoreServer::AddItem( const ASSETSTORE::CAssetStoreItem& item )
{
    MT::CScopeMutex lock( m_lock );
    m_items.push_back( item );
}

/*-------------------------------------------------------------------------*/

bool
CFakeStoreServer::ReplaceItem( const ASSETSTORE::CAssetStoreItem& item )
{
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_items.size(); ++i )
        if ( m_items[ i ].itemId == item.itemId )
        {
            m_items[ i ] = item;
            return true;
        }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFakeStoreServer::RemoveItem( const CORE::CString& itemId )
{
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_items.size(); ++i )
        if ( m_items[ i ].itemId == itemId )
        {
            m_items.erase( m_items.begin() + i );
            return true;
        }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CFakeStoreServer::SetLibraryFailures( UInt32 failures )
{
    MT::CScopeMutex lock( m_lock );
    m_libraryFailures = failures;
}

/*-------------------------------------------------------------------------*/

void
CFakeStoreServer::SetListingWithoutFormats( bool withoutFormats )
{
    MT::CScopeMutex lock( m_lock );
    m_listingWithoutFormats = withoutFormats;
}

/*-------------------------------------------------------------------------*/

void
CFakeStoreServer::SetFileContent( const CORE::CString& fileId, const CORE::CDynamicBuffer& content )
{
    MT::CScopeMutex lock( m_lock );
    m_contents[ fileId ] = content;
}

/*-------------------------------------------------------------------------*/

void
CFakeStoreServer::SetFileBehaviour( const CORE::CString& fileId, const CFakeFileBehaviour& behaviour )
{
    MT::CScopeMutex lock( m_lock );
    m_behaviours[ fileId ] = behaviour;
}

/*-------------------------------------------------------------------------*/

void
CFakeStoreServer::SetRequireAuth( bool requireAuth )
{
    MT::CScopeMutex lock( m_lock );
    m_requireAuth = requireAuth;
}

/*-------------------------------------------------------------------------*/

void
CFakeStoreServer::SetTokenEndpointWorks( bool works )
{
    MT::CScopeMutex lock( m_lock );
    m_tokenEndpointWorks = works;
}

/*-------------------------------------------------------------------------*/

void
CFakeStoreServer::ResetStats( void )
{
    MT::CScopeMutex lock( m_lock );
    m_stats = CFakeStoreStats();
}

/*-------------------------------------------------------------------------*/

CFakeStoreStats
CFakeStoreServer::GetStats( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_stats;
}

/*-------------------------------------------------------------------------*/

void
CFakeStoreServer::SendJson( WEB::CHttpResponseData& response, int status, const CORE::CDataNode& node )
{
    CORE::CString text;
    ASSETSTORE::CAssetStoreJson::ToText( node, text );
    response.statusCode = status;
    response.contentType = "application/json";
    response.content.Clear();
    response.content.CopyFrom( text, 0, false );
}

/*-------------------------------------------------------------------------*/

void
CFakeStoreServer::HandleRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response )
{
    const CORE::CString path = request.requestUri.SubstrToChar( '?', true );
    CORE::CString query = request.requestUriParams;
    if ( query.IsNULLOrEmpty() && request.requestUri.HasChar( '?' ) >= 0 )
        query = request.requestUri.SubstrToChar( '?', false );
    const CORE::CString::StringMap parameters = ParseQuery( query );
    CORE::CDataNodeSerializableSettings settings;

    if ( path == "/token" )
    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_tokenEndpointWorks )
        {
            response.statusCode = 403;
            return;
        }
        ++m_stats.tokensIssued;
        m_currentToken = "token-" + CORE::ToString( m_stats.tokensIssued );
        response.statusCode = 200;
        response.contentType = "text/plain";
        response.content.CopyFrom( m_currentToken, 0, false );
        return;
    }
    if ( path == "/library" )
    {
        MT::CScopeMutex lock( m_lock );
        ++m_stats.libraryRequests;
        if ( m_libraryFailures > 0 )
        {
            --m_libraryFailures;
            ++m_stats.unavailableAnswers;
            response.statusCode = 503;
            return;
        }
        const CORE::CString text = ValueOf( parameters, "text" ).Lowercase();
        const CORE::CString type = ValueOf( parameters, "type" );
        const UInt32 first = CORE::StringToUInt32( ValueOf( parameters, "cursor" ) );
        UInt32 pageSize = CORE::StringToUInt32( ValueOf( parameters, "pageSize" ) );
        if ( 0 == pageSize )
            pageSize = 50;
        ASSETSTORE::TAssetStoreItemVector matching;
        for ( size_t i=0; i<m_items.size(); ++i )
        {
            if ( !text.IsNULLOrEmpty() && m_items[ i ].title.Lowercase().HasSubstr( text, true ) < 0 )
                continue;
            if ( !type.IsNULLOrEmpty() && m_items[ i ].listingType != type )
                continue;
            matching.push_back( m_items[ i ] );
        }
        ASSETSTORE::CAssetStoreLibraryPage page;
        for ( UInt32 i=first; i<matching.size() && i<first+pageSize; ++i )
        {
            page.items.push_back( matching[ i ] );
            if ( m_listingWithoutFormats )
                page.items.back().formats.clear();
        }
        page.hasTotal = true;
        page.total = matching.size();
        if ( first + pageSize < matching.size() )
            page.nextCursor = CORE::ToString( first + pageSize );
        CORE::CDataNode node( "page", GUCEF_DATATYPE_OBJECT );
        page.Serialize( node, settings );
        SendJson( response, 200, node );
        return;
    }
    if ( 0 == path.HasSubstr( "/items/", true ) )
    {
        const CORE::CString itemId = path.CutChars( 7, true );
        MT::CScopeMutex lock( m_lock );
        ++m_stats.itemRequests;
        for ( size_t i=0; i<m_items.size(); ++i )
        {
            if ( m_items[ i ].itemId == itemId )
            {
                CORE::CDataNode node( "item", GUCEF_DATATYPE_OBJECT );
                m_items[ i ].Serialize( node, settings );
                SendJson( response, 200, node );
                return;
            }
        }
        response.statusCode = 404;
        return;
    }
    if ( 0 == path.HasSubstr( "/files/", true ) )
    {
        HandleFile( request, response, path.CutChars( 7, true ) );
        return;
    }
    response.statusCode = 404;
}

/*-------------------------------------------------------------------------*/

void
CFakeStoreServer::HandleFile( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response, const CORE::CString& fileId )
{
    MT::CScopeMutex lock( m_lock );
    ++m_stats.fileRequests;
    TContentMap::const_iterator content = m_contents.find( fileId );
    if ( content == m_contents.end() )
    {
        response.statusCode = 404;
        return;
    }
    if ( m_requireAuth && HeaderOf( request, "authorization" ) != "Bearer " + m_currentToken )
    {
        ++m_stats.refusedSessions;
        response.statusCode = 401;
        return;
    }
    CFakeFileBehaviour& behaviour = m_behaviours[ fileId ];
    if ( behaviour.expireFirst > 0 )
    {
        --behaviour.expireFirst;
        ++m_stats.expiredAnswers;
        response.statusCode = 403;
        return;
    }
    if ( behaviour.fail429 > 0 )
    {
        --behaviour.fail429;
        ++m_stats.rateLimitedAnswers;
        response.statusCode = 429;
        response.otherHeaders[ "Retry-After" ] = CORE::ToString( behaviour.retryAfterSeconds );
        return;
    }
    if ( behaviour.fail503 > 0 )
    {
        --behaviour.fail503;
        ++m_stats.unavailableAnswers;
        response.statusCode = 503;
        response.contentType = "text/plain";
        response.content.CopyFrom( CORE::CString( "the store is busy, try again later" ), 0, false );
        return;
    }

    CORE::CDynamicBuffer body( (*content).second );
    if ( behaviour.corrupt && body.GetDataSize() > 0 )
        body.AsTypePtr< UInt8 >()[ body.GetDataSize() / 2 ] ^= 0xFF;

    const CORE::CString range = HeaderOf( request, "range" );
    if ( !range.IsNULLOrEmpty() )
        ++m_stats.rangeRequests;
    if ( !range.IsNULLOrEmpty() && behaviour.supportsRange && 0 == range.HasSubstr( "bytes=", true ) )
    {
        const UInt32 start = CORE::StringToUInt32( range.CutChars( 6, true ).SubstrToChar( '-', true ) );
        if ( start >= body.GetDataSize() )
        {
            response.statusCode = 416;
            return;
        }
        response.statusCode = 206;
        response.otherHeaders[ "Content-Range" ] = "bytes " + CORE::ToString( start ) + "-" + CORE::ToString( body.GetDataSize() - 1 ) + "/" + CORE::ToString( body.GetDataSize() );
        response.contentType = "application/octet-stream";
        response.content.Clear();
        response.content.Append( body.GetConstBufferPtr( start ), body.GetDataSize() - start );
        return;
    }
    response.statusCode = 200;
    response.contentType = "application/octet-stream";
    response.content = body;
}

/*-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/
