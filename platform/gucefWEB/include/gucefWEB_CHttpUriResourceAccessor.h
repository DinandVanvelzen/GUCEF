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

#ifndef GUCEF_WEB_CHTTPURIRESOURCEACCESSOR_H
#define GUCEF_WEB_CHTTPURIRESOURCEACCESSOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CURIRESOURCEACCESSOR_H
#include "gucefCORE_CUriResourceAccessor.h"
#define GUCEF_CORE_CURIRESOURCEACCESSOR_H
#endif /* GUCEF_CORE_CURIRESOURCEACCESSOR_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Blocking access to resources on a HTTP server, abstracted through a http or https URI.
 *  Registered with the URI resource accessor factory of CORE for both schemes.
 *  It uses the blocking Execute() of CHTTPClient so every call runs on the calling thread.
 *  Whether https works depends on the HTTP client backends that are available, see CHTTPClient.
 *
 *  Operations map onto HTTP methods:
 *  - GetResource / GetPartialResource: GET, the second with a Range header
 *  - GetResourceMetaData: HEAD
 *  - CreateResource / UpdateResource: PUT
 *  - DeleteResource: DELETE
 *  - GetResourceAccess: read access only, the resource is fetched into memory
 *  Updating part of a resource and collections are not supported.
 *
 *  A response with a 2xx status is a success. The body of a response is written to the destination as it arrives,
 *  which is before the status is known, so the destination also receives the body of an error response.
 */
class GUCEF_WEB_PUBLIC_CPP CHttpUriResourceAccessor : public CORE::CUriResourceAccessor
{
    public:

    static const CString SchemeNameHttp;
    static const CString SchemeNameHttps;

    CHttpUriResourceAccessor( void );

    virtual ~CHttpUriResourceAccessor() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CreateResource( const CORE::CUri& uri ,
                                 CORE::CIOAccess& src  ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetResource( const CORE::CUri& uri        ,
                              CORE::CIOAccess& destination ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetPartialResource( const CORE::CUri& uri        ,
                                     UInt64 byteOffset            ,
                                     Int64 bytesToGet             ,
                                     CORE::CIOAccess& destination ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetResourceAccess( const CORE::CUri& uri               ,
                                    CORE::IOAccessPtr& accessToResource ,
                                    TURI_RESOURCEACCESS_MODE mode       ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetResourceMetaData( const CORE::CUri& uri             ,
                                      CORE::CResourceMetaData& metaData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool UpdateResource( const CORE::CUri& uri ,
                                 CORE::CIOAccess& src  ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool UpdatePartialResource( const CORE::CUri& uri    ,
                                        UInt64 byteOffset        ,
                                        Int64 bytesToWrite       ,
                                        CORE::CIOAccess& src     ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool UpdatePartialResource( const CORE::CUri& uri    ,
                                        CORE::CIOAccess& src     ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetSupportedOperations( const CORE::CUri& uri                     ,
                                         CUriResourceAccessorOperations& ops       ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DeleteResource( const CORE::CUri& uri ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsACollectionResource( const CORE::CUri& uri ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ResolveResourcesInCollection( const CORE::CUri& uri        ,
                                               UriVector& resources         ,
                                               bool recursive               ,
                                               bool addCollectionUris       ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    bool Perform( const CORE::CUri& uri                  ,
                  const CString& method                  ,
                  CORE::CIOAccess* requestBody           ,
                  CORE::CIOAccess* responseBodySink      ,
                  bool hasRange                          ,
                  UInt64 rangeStart                      ,
                  Int64 rangeLength                      ,
                  CHttpResponseData& response            );

    private:

    CHttpUriResourceAccessor( const CHttpUriResourceAccessor& src );                /**< not implemented */
    CHttpUriResourceAccessor& operator=( const CHttpUriResourceAccessor& src );     /**< not implemented */

    private:

    CHTTPClient m_client;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CHTTPURIRESOURCEACCESSOR_H ? */
