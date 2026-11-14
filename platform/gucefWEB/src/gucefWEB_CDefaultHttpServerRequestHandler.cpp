/*
 *  gucefWEB: GUCEF module providing Web application functionality 
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_WEB_CDEFAULTHTTPSERVERROUTERCONTROLLER_H
#include "gucefWEB_CDefaultHTTPServerRouterController.h"
#define GUCEF_WEB_CDEFAULTHTTPSERVERROUTERCONTROLLER_H
#endif /* GUCEF_WEB_CDEFAULTHTTPSERVERROUTERCONTROLLER_H ? */

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#ifndef GUCEF_WEB_CGLOBALHTTPCODECLINKS_H
#include "gucefWEB_CGlobalHttpCodecLinks.h"
#define GUCEF_WEB_CGLOBALHTTPCODECLINKS_H
#endif /* GUCEF_WEB_CGLOBALHTTPCODECLINKS_H ? */

#ifndef GUCEF_WEB_CHTTPENCODINGTYPES_H
#include "gucefWEB_CHttpEncodingTypes.h"
#define GUCEF_WEB_CHTTPENCODINGTYPES_H
#endif /* GUCEF_WEB_CHTTPENCODINGTYPES_H ? */

#include "gucefWEB_CDefaultHttpServerRequestHandler.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDefaultHttpServerRequestHandler::CDefaultHttpServerRequestHandler( CIHTTPServerRouterController* routerController /* = GUCEF_NULL */ )
    : CIHttpServerRequestHandler()
    , m_routerController( routerController )
    , m_ownsRouterController( false )
    , m_applyTransferEncodingWhenAble( false )
{GUCEF_TRACE;

    if ( GUCEF_NULL == routerController )
    {
        m_routerController = GUCEF_NEW CDefaultHTTPServerRouterController();
        m_ownsRouterController = true;
    }
}

/*-------------------------------------------------------------------------*/

CDefaultHttpServerRequestHandler::CDefaultHttpServerRequestHandler( const CDefaultHttpServerRequestHandler& src )
    : CIHttpServerRequestHandler( src )
    , m_routerController( src.m_routerController )
    , m_ownsRouterController( false )
    , m_applyTransferEncodingWhenAble( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDefaultHttpServerRequestHandler::~CDefaultHttpServerRequestHandler()
{GUCEF_TRACE;

    SetRouterController( GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CDefaultHttpServerRequestHandler::SetApplyTransferEncodingWhenAble( bool applyEncoding )
{GUCEF_TRACE;

    m_applyTransferEncodingWhenAble = applyEncoding; 
}

/*-------------------------------------------------------------------------*/

bool
CDefaultHttpServerRequestHandler::GetApplyTransferEncodingWhenAble( void ) const
{GUCEF_TRACE;

    return m_applyTransferEncodingWhenAble;
}

/*-------------------------------------------------------------------------*/

bool
CDefaultHttpServerRequestHandler::ProcessTransferEncoding( const CHttpRequestData& request ,
                                                           CHttpResponseData& response     ) const
{GUCEF_TRACE;

    if ( !m_applyTransferEncodingWhenAble )
        return true;
    if ( request.encodingRepresentations.empty() )
        return true;
    if ( 0 == response.content.GetDataSize() )
        return true;

    try
    {
        CGlobalHttpCodecLinks& codecs = CWebGlobal::Instance()->GetGlobalHttpCodecLinks();
        
        bool noEncodingIsOk = false;
        CHttpRequestData::TStringVector::const_iterator n = request.encodingRepresentations.begin();
        while ( n != request.encodingRepresentations.end() )
        {
            // Content type identity means no encoding is also Ok
            if ( CHttpEncodingTypes::EncodingTypeIdentity == (*n) || 
                 CHttpEncodingTypes::EncodingTypeAny == (*n)       )
            {
                noEncodingIsOk = true;    
            }
            else
            {
                CHttpCodecLinks::TEncodingCodecPtr codec = codecs.GetEncodingCodec( (*n) );
                if ( codec )
                {
                    CORE::CDynamicBufferAccess originalContentAccess( &response.content, false );
                    CORE::CDynamicBuffer encodedContent;
                    encodedContent.SetBufferSize( response.content.GetDataSize(), true );
                    CORE::CDynamicBufferAccess encodedContentAccess( &encodedContent, false );

                    if ( codec->Encode( originalContentAccess, encodedContentAccess ) )
                    {
                        response.contentEncoding = (*n);
                        response.content = encodedContent;
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DefaultHttpServerRequestHandler:ProcessEncoding: Encoded content as \"" + (*n) + "\"" );
                        return true;
                    }
                    else
                    {
                        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DefaultHttpServerRequestHandler:ProcessEncoding: Failed to encode content as \"" + (*n) + "\"" );
                    }
                }
            }
            ++n;
        }

        if ( noEncodingIsOk )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DefaultHttpServerRequestHandler:ProcessEncoding: No matching encoding capability was found. Encoding is optional and skipped" );
            return true;
        }
        else
        {
            // No acceptable encoding match was found
            response.statusCode = 406;
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DefaultHttpServerRequestHandler:ProcessEncoding: No matching encoding capability was found and encoding is required" );
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "DefaultHttpServerRequestHandler:ProcessEncoding: Exception: " + CString( e.what() ) );
        return false;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDefaultHttpServerRequestHandler::OnRequest( const CHttpRequestData& request ,
                                             CHttpResponseData& response     )
{GUCEF_TRACE;

    try
    {
        if ( request.requestType == "GET" )
        {
            return OnRead( request, response );
        }
        else
        if ( request.requestType == "HEAD" )
        {
            return OnReadMetaData( request, response );
        }
        else
        if ( request.requestType == "POST" )
        {
            return OnCreate( request, response );
        }
        else
        if ( request.requestType == "PUT" )
        {
            return OnUpdate( request, false, response );
        }
        else
        if ( request.requestType == "PATCH" )
        {
            return OnUpdate( request, true, response );
        }
        else
        if ( request.requestType == "DELETE" )
        {
            return OnDelete( request, response );
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "DefaultHttpServerRequestHandler:OnRequest: Exception: " + CString( e.what() ) );
        return false;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDefaultHttpServerRequestHandler::OnRead( const CHttpRequestData& request ,
                                          CHttpResponseData& response     )
{GUCEF_TRACE;

    return PerformReadOperation( request, true, response );
}

/*-------------------------------------------------------------------------*/

bool
CDefaultHttpServerRequestHandler::OnReadMetaData( const CHttpRequestData& request ,
                                                  CHttpResponseData& response     )
{GUCEF_TRACE;

    return PerformReadOperation( request, false, response );
}

/*-------------------------------------------------------------------------*/

bool
CDefaultHttpServerRequestHandler::PerformReadOperation( const CHttpRequestData& request ,
                                                        bool contentRequested           ,
                                                        CHttpResponseData& response     )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_routerController )
    {
        response.statusCode = 500; // Improperly configured
        return false;
    }

    try
    {
        const CString& resourceURI = request.requestUri;

        CIHTTPServerRouter* resourceRouter = m_routerController->GetHandler( resourceURI );
        if ( GUCEF_NULL == resourceRouter )
        {
            // No handler is mounted to handle this Uri
            response.statusCode = 404;
            return true;
        }

        // Obtain the relative Uri for the given handler
        CString uriAfterBaseAddress = m_routerController->GetUriAfterTheBaseAddress( *resourceRouter, resourceURI );

        // Obtain access to the resource based of the relative Uri
        CIHTTPServerRouter::THTTPServerResourcePtr resource = resourceRouter->ResolveUriToResource( uriAfterBaseAddress );
        if ( resource.IsNULL() )
        {
            // Not found
            response.statusCode = 404;
            return true;
        }

        response.contentType = resource->GetBestMatchedSerializationRepresentation( uriAfterBaseAddress, request.resourceRepresentations );

        // Did we find a supported type match?
        if ( response.contentType.IsNULLOrEmpty() )
        {
            // Unsupported media type
            response.statusCode = 415;
            response.acceptedTypes = resource->GetSupportedSerializationRepresentations( uriAfterBaseAddress );
            return true;
        }

        // Check if we need to perform a version check for efficiency purposes
        // If a specific version was requested we should only send the resource if the resource has
        // been altered thus saving bandwidth
        if ( MatchResourceVersion( resource->GetResourceVersion( uriAfterBaseAddress ), request.resourceVersions ) )
        {
            // The resource has not been changed thus no need to send the resource to the client
            response.statusCode = 304;
            response.eTag = resource->GetResourceVersion( uriAfterBaseAddress );
            return true;
        }
                                                    
        if ( contentRequested )
        {
            // Set the serialized resource as the return data content
            if ( !resource->Serialize( uriAfterBaseAddress, response.content, response.contentType, request.requestUriParams ) )
            {
                // Something went wrong in the handler while serializing the resource
                response.statusCode = 415;
                response.acceptedTypes = resource->GetSupportedSerializationRepresentations( uriAfterBaseAddress );
                return true;
            }

            ProcessTransferEncoding( request, response );
        }

        response.eTag = resource->GetResourceVersion( uriAfterBaseAddress );
        response.lastModified = resource->GetLastModifiedTime( uriAfterBaseAddress );
        response.cacheability = resource->GetCacheability( uriAfterBaseAddress );

        // Inform the client of the actual location of the resource if the request Uri was
        // actually an alias
        CString requestResourceUri = request.ConstructUriWithAuthority();
        CString actualResourceUri = m_routerController->MakeUriAbsolute( *resourceRouter, requestResourceUri, resource->GetURL() );
        if ( requestResourceUri != actualResourceUri )
        {         //@TODO: should the status code be 3xx instead of 200??
            response.location = actualResourceUri;
        }

        // Set the StatusCode to OK
        response.statusCode = 200;
        return true;
    }
    catch ( const std::exception& e )
    {
        // Unhandled exception during execution
        response.statusCode = 500;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "DefaultHttpServerRequestHandler:PerformReadOperation: Exception: " + CString( e.what() ) );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CDefaultHttpServerRequestHandler::OnUpdate( const CHttpRequestData& request ,
                                            bool isDeltaUpdateOnly          ,
                                            CHttpResponseData& response     )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_routerController )
    {
        // No system has been set for providing handlers
        response.statusCode = 500;
        return false;
    }
    try
    {
        const CString& resourceURI = request.requestUri;

        CIHTTPServerRouter* resourceRouter = m_routerController->GetHandler( resourceURI );
        if ( GUCEF_NULL == resourceRouter )
        {
            // No system has been set for providing handlers
            response.statusCode = 500;
            return false;
        }

        // Check if the client send the content for the update
        if ( request.content.GetDataSize() == 0 )
        {
            response.statusCode = 400; // Bad request
            return true;
        }

        CString remainderUri = m_routerController->GetUriAfterTheBaseAddress( *resourceRouter, resourceURI );
        CIHTTPServerRouter::THTTPServerResourcePtr resource = resourceRouter->ResolveUriToResource( remainderUri );

        if ( resource.IsNULL() )
        {
            // Not found
            response.statusCode = 404;
            return true;
        }

        //CString inputRepresentation;
        //if ( request.resourceRepresentations.size() > 0 )
        //{
        //    inputRepresentation = request.resourceRepresentations.front();
        //    bool isSupportedMimeType = false;

        //    const TStringVector& supportedRepresentations = resource->GetSupportedDeserializationRepresentations();
        //    TStringVector::const_iterator i = supportedRepresentations.begin();
        //    while ( i != supportedRepresentations.end() )
        //    {
        //        if ( (*i).Equals( inputRepresentation, false ) )
        //        {
        //            isSupportedMimeType = true;
        //            break;
        //        }
        //        ++i;
        //    }

        //    if ( !isSupportedMimeType )
        //    {
        //        response.statusCode = 415;
        //        response.acceptedTypes = resource->GetSupportedDeserializationRepresentations();
        //        return true;
        //    }
        //}

        // Check if we need to perform a version (concurrency) check
        if ( request.resourceVersions.size() > 0 )
        {
            if ( !MatchResourceVersion( resource->GetResourceVersion( remainderUri ), request.resourceVersions ) )
            {
                // The resource has been changed while the client was making its changes,..
                // We have a race condition and should abort the update
                response.statusCode = 412; // Pre condition failed
                return true;
            }
        }

        // Assign to the entry
        CIHTTPServerResource::TDeserializeState deserializeState = resource->Deserialize( remainderUri, request.content, request.contentRepresentation, isDeltaUpdateOnly );
        if ( CIHTTPServerResource::DESERIALIZESTATE_CORRUPTEDINPUT == deserializeState  )
        {
            // There was a problem in the backend deserializing the content in the negotiated format
            response.statusCode = 400; // <- corrupt data,.. bad request
            response.acceptedTypes = resource->GetSupportedDeserializationRepresentations( remainderUri );
            return true;
        }
        else
        if ( CIHTTPServerResource::DESERIALIZESTATE_UNABLETOUPDATE == deserializeState  )
        {
            // There was a problem in the backend actually updating the resource with the deserialized content
            response.statusCode = 304; // <- resource not modified
        }
        else
        if ( CIHTTPServerResource::DESERIALIZESTATE_NOTSUPPORTED == deserializeState  )
        {
            response.statusCode = 405; 
            PolulateAllowedMethodsUsingResource( resource, response );
        }        
        else
        {
            // Update to the existing entry was successfull
            response.statusCode = 200;
        }

        response.eTag = resource->GetResourceVersion( remainderUri );
        response.lastModified = resource->GetLastModifiedTime( remainderUri );

        // Make sure we only send back absolute Uri's for resource locations
        CString requestResourceUri = request.ConstructUriWithAuthority();
        response.location = m_routerController->MakeUriAbsolute( *resourceRouter, requestResourceUri, resource->GetURL() );

        // Send the serverside representation back to the client in the representation of the update
        if ( resource->Serialize( remainderUri, response.content, request.contentRepresentation , request.requestUriParams ) )
        {
            response.contentType = request.contentRepresentation;
            ProcessTransferEncoding( request, response );
        }
        return true;
    }
    catch ( const std::exception& e )
    {
        // Unhandled exception during execution
        response.statusCode = 500;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "DefaultHttpServerRequestHandler:OnUpdate: Exception: " + CString( e.what() ) );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CDefaultHttpServerRequestHandler::OnCreate( const CHttpRequestData& request ,
                                            CHttpResponseData& response     )
{GUCEF_TRACE;

    if ( request.content.GetDataSize() == 0 )
    {
        //Either the client request could not be parsed or the client has not provided any data
        //to create. So throw bad request.
        response.statusCode = 400;
        return false;
    }

    if ( GUCEF_NULL == m_routerController )
    {
        response.statusCode = 500; // Improperly configured
        return false;
    }

    try
    {
        const CString& resourceURI = request.requestUri;

        CIHTTPServerRouter* resourceRouter = m_routerController->GetHandler( resourceURI );
        if ( GUCEF_NULL == resourceRouter )
        {
            // No system has been set for providing handlers
            // There is nothing the client can do here.
            response.statusCode = 500;
            return true;
        }

        // Get relative collection URI and subsequently the collection itself
        CString containerUri = m_routerController->GetUriAfterTheBaseAddress( *resourceRouter, resourceURI );
        CIHTTPServerRouter::THTTPServerResourcePtr containerResource = resourceRouter->ResolveUriToResource( containerUri );

        if ( containerResource.IsNULL() )
        {
            // Unable to locate the collection in which to place the entry
            response.statusCode = 404;
            return true;
        }

        // create resource in preperation for deserialization
        CIHTTPServerRouter::THTTPServerResourcePtr resource;
        TStringVector supportedRepresentations;

        CIHTTPServerResource::TCreateState createState = containerResource->CreateResource( containerUri                  ,
                                                                                            request.transactionID         ,
                                                                                            request.content               ,
                                                                                            request.contentRepresentation ,
                                                                                            request.requestUriParams      ,
                                                                                            resource                      ,
                                                                                            supportedRepresentations      );

        if ( ( CIHTTPServerResource::CREATESTATE_FAILED == createState ) || ( !resource ) )
        {
            // Failed to finalize (thus persist) the data
            response.statusCode = 500;
            return true;
        }
        if ( CIHTTPServerResource::CREATESTATE_NOTSUPPORTED == createState )
        {
            response.statusCode = 405;
            PolulateAllowedMethodsUsingResource( resource, response );
            return true;
        }
        
        // See if the handler was able to already provide access to the placeholder as a resource
        if ( CIHTTPServerResource::CREATESTATE_CONFLICTING == createState )
        {
            response.statusCode = 409; // Conflict: already exists and it is not the same item
            return true;
        }
        if ( CIHTTPServerResource::CREATESTATE_UNSUPPORTEDREPRESENTATION == createState )
        {
            response.statusCode = 415;
            response.acceptedTypes = supportedRepresentations;
            return true;
        }

        if ( CIHTTPServerResource::CREATESTATE_DESERIALIZATIONFAILED == createState )
        {
            // tell client about the error,.. cannot create because of corrupted stream
            response.statusCode = 400;
            response.acceptedTypes = supportedRepresentations;
            return true;
        }

        if ( CIHTTPServerResource::CREATESTATE_CREATED == createState )
        {
            response.location = resource->GetURL();

            // To comply with protocols like ATOM-pub we need to stream out whatever content is already present
            // as the new resource as part of our reply regardless of whether content was send to the server

            // Perform the serialization
            if ( resource->Serialize( response.location, response.content, request.contentRepresentation, request.requestUriParams ) )
            {
                response.contentType = request.contentRepresentation;
                ProcessTransferEncoding( request, response );
            }

            // Make sure the client gets an absolute path to the resource
            // which may be a placeholder
            CString uriWithAuthority = request.ConstructUriWithAuthority();
            response.location = m_routerController->MakeUriAbsolute( *resourceRouter, uriWithAuthority, response.location );

            // tell client what representation the resource is stored as plus its version
            response.eTag = resource->GetResourceVersion( response.location );
            response.lastModified = resource->GetLastModifiedTime( response.location );

            // Tell the client we succeeded in creating a new resource
            response.statusCode = 201;
            return true;
        }

        // We should not be able to get here,.. bad create state
        response.statusCode = 500;
        return false;

    }
    catch ( const std::exception& e )
    {
        // Unhandled exception during execution
        response.statusCode = 500;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "DefaultHttpServerRequestHandler:OnCreate: Exception: " + CString( e.what() ) );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CDefaultHttpServerRequestHandler::OnDelete( const CHttpRequestData& request ,
                                            CHttpResponseData& response     )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_routerController )
    {
        response.statusCode = 500; // Improperly configured
        return false;
    }

    try
    {
        const CString& resourceURI = request.requestUri;

        CIHTTPServerRouter* resourceRouter = m_routerController->GetHandler( resourceURI );
        if ( GUCEF_NULL == resourceRouter )
        {
            // No system has been set for providing handlers
            response.statusCode = 404;
            return false;
        }

        CString remainderUri = m_routerController->GetUriAfterTheBaseAddress( *resourceRouter, resourceURI );
        CIHTTPServerRouter::THTTPServerResourcePtr resource = resourceRouter->ResolveUriToResource( remainderUri );

        if ( !resource )
        {
            // Not found
            response.statusCode = 404;
            return true;
        }

        if ( request.resourceVersions.size() > 0 )
        {
            // Check if we need to perform a version (concurrency) check
            // Be determine this by checking whether client entered a specific resource version to delete
            if ( !MatchResourceVersion( resource->GetResourceVersion( remainderUri ), request.resourceVersions ) )
            {
                // The resource it of a different version then what the client expected,..
                // We will abort the delete
                response.statusCode = 412;
                response.eTag = resource->GetResourceVersion( remainderUri );
                return true;
            }
        }

        if ( resource->DeleteResource( remainderUri ) )
        {
            // tell client the delete succeeded
            response.statusCode = 200;
            return true;
        }
        else
        {
            response.statusCode = 405; //
            response.allowedMethods.push_back( "GET" ); // Hmm, we're not quite sure are we...
            response.allowedMethods.push_back( "POST" );
            return true;
        }
    }
    catch ( const std::exception& e )
    {
        // Unhandled exception during execution
        response.statusCode = 500;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "DefaultHttpServerRequestHandler:OnDelete: Exception: " + CString( e.what() ) );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CDefaultHttpServerRequestHandler::MatchResourceVersion( const CString& resourceVersion  ,
                                                        const TStringVector& searchList )
{GUCEF_TRACE;

    if ( searchList.size() > 0 && resourceVersion.Length() > 0 )
    {
        TStringVector::const_iterator i = searchList.begin();
        while( i != searchList.end() )
        {
            if ( resourceVersion.Equals( *i, false ) )
            {
                return true;
            }
            ++i;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CDefaultHttpServerRequestHandler::SetRouterController( CIHTTPServerRouterController* routerController )
{GUCEF_TRACE;

    if ( m_ownsRouterController )
    {
        GUCEF_DELETE m_routerController;
        m_routerController = GUCEF_NULL;
    }

    m_routerController = routerController;
    m_ownsRouterController = false;
    return true;
}

/*-------------------------------------------------------------------------*/
    
CIHTTPServerRouterController* 
CDefaultHttpServerRequestHandler::GetRouterController( void ) const
{GUCEF_TRACE;

    return m_routerController;
}

/*-------------------------------------------------------------------------*/

void
CDefaultHttpServerRequestHandler::PolulateAllowedMethodsUsingResource( CIHTTPServerRouter::THTTPServerResourcePtr& resource ,
                                                                       CHttpResponseData& response                          )
{
    if ( !resource.IsNULL() )
    {
        if ( resource->IsCreateSupported() )
            response.allowedMethods.push_back( "POST" );
        if ( resource->IsSerializeSupported() )
            response.allowedMethods.push_back( "GET" );
        if ( resource->IsDeserializeSupported( false ) )
            response.allowedMethods.push_back( "PUT" );
        if ( resource->IsDeserializeSupported( true ) )
            response.allowedMethods.push_back( "PATCH" );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace WEB */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
