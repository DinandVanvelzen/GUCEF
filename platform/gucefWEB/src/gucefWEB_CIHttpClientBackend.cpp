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

#include "gucefWEB_CIHttpClientBackend.h"

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

CIHttpClientBackendObserver::~CIHttpClientBackendObserver()
{GUCEF_TRACE;
}

void CIHttpClientBackendObserver::OnHttpClientConnected( void ) {GUCEF_TRACE; }
void CIHttpClientBackendObserver::OnHttpClientResponseHeader( const CString& name, const CString& value ) {GUCEF_TRACE; }
void CIHttpClientBackendObserver::OnHttpClientResponseHeadersComplete( const CHttpResponseData& response ) {GUCEF_TRACE; }
void CIHttpClientBackendObserver::OnHttpClientBodyData( const void* data, UInt32 dataSize ) {GUCEF_TRACE; }
void CIHttpClientBackendObserver::OnHttpClientDataSent( UInt32 bytesSent ) {GUCEF_TRACE; }
void CIHttpClientBackendObserver::OnHttpClientRedirect( const CString& newUrl ) {GUCEF_TRACE; }

/*-------------------------------------------------------------------------*/

CIHttpClientBackendRequest::~CIHttpClientBackendRequest()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CIHttpClientBackend::~CIHttpClientBackend()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
