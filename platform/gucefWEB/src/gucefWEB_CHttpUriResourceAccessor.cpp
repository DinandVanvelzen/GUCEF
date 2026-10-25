/*
 *  gucefWEB: GUCEF module providing WEB application functionality
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CRESOURCEMETADATA_H
#include "gucefCORE_CResourceMetaData.h"
#define GUCEF_CORE_CRESOURCEMETADATA_H
#endif /* GUCEF_CORE_CRESOURCEMETADATA_H ? */

#include "gucefWEB_CHttpUriResourceAccessor.h"

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CHttpUriResourceAccessor::SchemeNameHttp = "http";
const CString CHttpUriResourceAccessor::SchemeNameHttps = "https";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Gives a CIOAccess that someone else owns to code that wants a shared pointer to one, without taking ownership.
 *  Everything is passed on to the access it refers to, which has to outlive this object.
 */
class GUCEF_HIDDEN CIOAccessReference : public CORE::CIOAccess
{
    public:

    explicit CIOAccessReference( CORE::CIOAccess& target ) : CORE::CIOAccess(), m_target( target ) {}
    virtual ~CIOAccessReference() GUCEF_VIRTUAL_OVERRIDE {}

    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE { m_target.Open(); }
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE { m_target.Close(); }
    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_target.Opened(); }
    virtual UInt32 Read( void* dest, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE { return m_target.Read( dest, esize, elements ); }
    virtual UInt32 Write( const void* srcdata, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE { return m_target.Write( srcdata, esize, elements ); }
    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_target.Tell(); }
    virtual Int32 Seek( Int64 offset, Int32 origin ) GUCEF_VIRTUAL_OVERRIDE { return m_target.Seek( offset, origin ); }
    virtual UInt32 Setpos( UInt64 position ) GUCEF_VIRTUAL_OVERRIDE { return m_target.Setpos( position ); }
    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE { return m_target.GetChar(); }
    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_target.Eof(); }
    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_target.IsReadable(); }
    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_target.IsWriteable(); }
    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE { return m_target.IsValid(); }
    virtual UInt64 GetSize( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_target.GetSize(); }
    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE { m_target.Flush(); }
    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CIOAccessReference( m_target ); }

    private:

    CORE::CIOAccess& m_target;
};

/*-------------------------------------------------------------------------*/

static bool
IsSuccessStatus( int statusCode )
{
    return statusCode >= 200 && statusCode < 300;
}

/*-------------------------------------------------------------------------*/

/** header names are kept as the server sent them, so the lookup ignores case */
static bool
FindHeader( const CHttpResponseData& response, const CString& name, CString& value )
{
    CHttpResponseData::TStringMap::const_iterator i = response.otherHeaders.begin();
    while ( i != response.otherHeaders.end() )
    {
        if ( (*i).first.Equals( name, false ) )
        {
            value = (*i).second;
            return true;
        }
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CHttpUriResourceAccessor::CHttpUriResourceAccessor( void )
    : CORE::CUriResourceAccessor()
    , m_client()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpUriResourceAccessor::~CHttpUriResourceAccessor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CHttpUriResourceAccessor::Perform( const CORE::CUri& uri             ,
                                   const CString& method             ,
                                   CORE::CIOAccess* requestBody      ,
                                   CORE::CIOAccess* responseBodySink ,
                                   bool hasRange                     ,
                                   UInt64 rangeStart                 ,
                                   Int64 rangeLength                 ,
                                   CHttpResponseData& response       )
{GUCEF_TRACE;

    if ( !uri.GetScheme().Equals( SchemeNameHttp, false ) && !uri.GetScheme().Equals( SchemeNameHttps, false ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpUriResourceAccessor: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CHttpRequestData request;
    if ( !request.SetUrl( uri.ToUriInStringForm() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpUriResourceAccessor: Not a valid URL: " << CORE::CRedactor::ForUrl().RedactUrl( uri.ToUriInStringForm() ) );
        return false;
    }
    request.requestType = method;

    CHttpClientRequestOptions options = m_client.GetDefaultOptions();
    if ( GUCEF_NULL != requestBody )
        options.requestBody = CORE::IOAccessPtr( GUCEF_NEW CIOAccessReference( *requestBody ) );
    if ( GUCEF_NULL != responseBodySink )
        options.responseBodySink = CORE::IOAccessPtr( GUCEF_NEW CIOAccessReference( *responseBodySink ) );
    options.hasRange = hasRange;
    options.rangeStart = rangeStart;
    options.rangeLength = rangeLength;

    CHttpClientTransferInfo info;
    if ( !m_client.Execute( request, options, response, &info ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpUriResourceAccessor: " << method << " " << CORE::CRedactor::ForUrl().RedactUrl( uri.ToUriInStringForm() ) << " failed: " << info.errorDescription );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHttpUriResourceAccessor::CreateResource( const CORE::CUri& uri ,
                                          CORE::CIOAccess& src  )
{GUCEF_TRACE;

    return UpdateResource( uri, src );
}

/*-------------------------------------------------------------------------*/

bool
CHttpUriResourceAccessor::GetResource( const CORE::CUri& uri        ,
                                       CORE::CIOAccess& destination )
{GUCEF_TRACE;

    CHttpResponseData response;
    return Perform( uri, "GET", GUCEF_NULL, &destination, false, 0, -1, response ) && IsSuccessStatus( response.statusCode );
}

/*-------------------------------------------------------------------------*/

bool
CHttpUriResourceAccessor::GetPartialResource( const CORE::CUri& uri        ,
                                              UInt64 byteOffset            ,
                                              Int64 bytesToGet             ,
                                              CORE::CIOAccess& destination )
{GUCEF_TRACE;

    CHttpResponseData response;
    if ( !Perform( uri, "GET", GUCEF_NULL, &destination, true, byteOffset, bytesToGet, response ) )
        return false;

    // A server which does not support ranges answers with the entire resource which is only
    // what was asked for when the request was for all of it
    if ( 206 == response.statusCode )
        return true;
    if ( 200 == response.statusCode && 0 == byteOffset && bytesToGet <= 0 )
        return true;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpUriResourceAccessor:GetPartialResource: Server answered with status " << response.statusCode <<
        " instead of 206 for " << CORE::CRedactor::ForUrl().RedactUrl( uri.ToUriInStringForm() ) );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CHttpUriResourceAccessor::GetResourceAccess( const CORE::CUri& uri               ,
                                             CORE::IOAccessPtr& accessToResource ,
                                             TURI_RESOURCEACCESS_MODE mode       )
{GUCEF_TRACE;

    if ( !IsReadOnlyResourceAccessMode( mode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpUriResourceAccessor:GetResourceAccess: Only read access is supported" );
        return false;
    }

    // The resource is fetched into memory and the access owns that memory
    CORE::CDynamicBuffer* content = GUCEF_NEW CORE::CDynamicBuffer();
    CORE::IOAccessPtr access( GUCEF_NEW CORE::CDynamicBufferAccess( content, true ) );
    if ( !GetResource( uri, *access ) )
        return false;

    access->Setpos( 0 );
    accessToResource = access;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHttpUriResourceAccessor::GetResourceMetaData( const CORE::CUri& uri             ,
                                               CORE::CResourceMetaData& metaData )
{GUCEF_TRACE;

    CHttpResponseData response;
    if ( !Perform( uri, "HEAD", GUCEF_NULL, GUCEF_NULL, false, 0, -1, response ) || !IsSuccessStatus( response.statusCode ) )
        return false;

    // the part of the path after the last slash
    metaData.name = uri.GetPath().SubstrToChar( '/', false, false );
    metaData.hasName = !metaData.name.IsNULLOrEmpty();

    CString contentLength;
    if ( FindHeader( response, "Content-Length", contentLength ) )
    {
        metaData.resourceSizeInBytes = CORE::StringToUInt64( contentLength );
        metaData.hasResourceSizeInBytes = true;
    }
    if ( !response.contentType.IsNULLOrEmpty() )
    {
        metaData.mimeType = response.contentType;
        metaData.hasMimeType = true;
    }
    if ( !response.eTag.IsNULLOrEmpty() )
    {
        metaData.version = response.eTag;
        metaData.hasVersion = true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHttpUriResourceAccessor::UpdateResource( const CORE::CUri& uri ,
                                          CORE::CIOAccess& src  )
{GUCEF_TRACE;

    CHttpResponseData response;
    return Perform( uri, "PUT", &src, GUCEF_NULL, false, 0, -1, response ) && IsSuccessStatus( response.statusCode );
}

/*-------------------------------------------------------------------------*/

bool
CHttpUriResourceAccessor::UpdatePartialResource( const CORE::CUri& uri ,
                                                 UInt64 byteOffset     ,
                                                 Int64 bytesToWrite    ,
                                                 CORE::CIOAccess& src  )
{GUCEF_TRACE;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpUriResourceAccessor:UpdatePartialResource: Not supported" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CHttpUriResourceAccessor::UpdatePartialResource( const CORE::CUri& uri ,
                                                 CORE::CIOAccess& src  )
{GUCEF_TRACE;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpUriResourceAccessor:UpdatePartialResource: Not supported" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CHttpUriResourceAccessor::GetSupportedOperations( const CORE::CUri& uri               ,
                                                  CUriResourceAccessorOperations& ops )
{GUCEF_TRACE;

    if ( !uri.GetScheme().Equals( SchemeNameHttp, false ) && !uri.GetScheme().Equals( SchemeNameHttps, false ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpUriResourceAccessor:GetSupportedOperations: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    ops.createResource = true;
    ops.deleteResource = true;
    ops.getPartialResource = true;
    ops.getResource = true;
    ops.getResourceMetaData = true;
    ops.getResourceReadAccess = true;
    ops.getResourceWriteAccess = false;
    ops.getResourceAppendAccess = false;
    ops.updatePartialResource = false;
    ops.updateResource = true;
    ops.collectionDetermnination = false;
    ops.collectionResolution = false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHttpUriResourceAccessor::DeleteResource( const CORE::CUri& uri )
{GUCEF_TRACE;

    CHttpResponseData response;
    return Perform( uri, "DELETE", GUCEF_NULL, GUCEF_NULL, false, 0, -1, response ) && IsSuccessStatus( response.statusCode );
}

/*-------------------------------------------------------------------------*/

bool
CHttpUriResourceAccessor::IsACollectionResource( const CORE::CUri& uri ) const
{GUCEF_TRACE;

    // HTTP has no notion of collections
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CHttpUriResourceAccessor::ResolveResourcesInCollection( const CORE::CUri& uri  ,
                                                        UriVector& resources   ,
                                                        bool recursive         ,
                                                        bool addCollectionUris )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "HttpUriResourceAccessor:ResolveResourcesInCollection: Not supported" );
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
