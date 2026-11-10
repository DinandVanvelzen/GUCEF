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

#ifndef GUCEF_WEB_CHTTPCLIENTREQUESTRUNNER_H
#define GUCEF_WEB_CHTTPCLIENTREQUESTRUNNER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CHTTPCLIENTBACKENDSELECTION_H
#include "gucefWEB_CHttpClientBackendSelection.h"
#define GUCEF_WEB_CHTTPCLIENTBACKENDSELECTION_H
#endif /* GUCEF_WEB_CHTTPCLIENTBACKENDSELECTION_H ? */

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
 *  Carries out one HTTP request on a pulse generator: the work shared by CHTTPClient::Execute() and
 *  CHTTPClient::ExecuteAsync(), the only difference between those is which thread drives it.
 *
 *  Per hop of the request it picks the backend for the URL scheme, adds the cookies from the cookie jar and the
 *  Range header and stores received cookies. It follows redirects and enforces the timeouts, so all backends
 *  behave alike in these respects.
 */
class GUCEF_HIDDEN CHttpClientRequestRunner : public CIHttpClientBackendObserver
{
    public:

    /**
     *  The request, options, response and transfer info have to remain available while the runner is in use.
     *  The progress observer is optional.
     */
    CHttpClientRequestRunner( CHttpClientBackendSelectionPtr backends          ,
                              const CHttpRequestData& request                  ,
                              const CHttpClientRequestOptions& options         ,
                              CHttpResponseData& response                      ,
                              CHttpClientTransferInfo& transferInfo            ,
                              CIHttpClientBackendObserver* progressObserver    );

    virtual ~CHttpClientRequestRunner() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Starts the request on the given pulse generator, from the thread of that pulse generator
     *  @return false if it could not be started, it is finished in that case
     */
    bool Start( CORE::PulseGeneratorPtr pulseGenerator );

    /**
     *  Checks on the progress, follows redirects and enforces the timeouts. To be called regularly from the
     *  thread of the pulse generator, for example per task cycle.
     *  @return true when the request is finished
     */
    bool Update( void );

    void Cancel( void );

    bool IsFinished( void ) const;

    /**
     *  Starts the request on a pulse generator owned by the calling thread and drives it there until done
     *  @return whether a response was received
     */
    static bool Run( CHttpClientBackendSelectionPtr backends          ,
                     const CHttpRequestData& request                  ,
                     const CHttpClientRequestOptions& options         ,
                     CHttpResponseData& response                      ,
                     CHttpClientTransferInfo& transferInfo            ,
                     CIHttpClientBackendObserver* progressObserver    );

    virtual void OnHttpClientConnected( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnHttpClientResponseHeader( const CString& name, const CString& value ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnHttpClientResponseHeadersComplete( const CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnHttpClientBodyData( const void* data, UInt32 dataSize ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnHttpClientDataSent( UInt32 bytesSent ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CHttpClientRequestRunner( const CHttpClientRequestRunner& src );            /**< not supported */
    CHttpClientRequestRunner& operator=( const CHttpClientRequestRunner& src ); /**< not supported */

    bool StartHop( void );
    bool FollowRedirect( void );
    void Finish( void );
    void Fail( const CString& errorDescription );
    UInt32 GetElapsedMs( UInt64 sinceTicks ) const;

    CHttpClientBackendSelectionPtr m_backends;
    const CHttpRequestData& m_originalRequest;
    const CHttpClientRequestOptions& m_options;
    CHttpResponseData& m_response;
    CHttpClientTransferInfo& m_transferInfo;
    CIHttpClientBackendObserver* m_progressObserver;

    CHttpRequestData m_hopRequest;
    CHttpClientRequestOptions m_hopOptions;
    CIHttpClientBackendRequestPtr m_backendRequest;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    UInt64 m_requestStartTicks;
    UInt64 m_hopStartTicks;
    UInt64 m_requestBodyStartPos;
    bool m_credentialsStripped;     /**< a redirect left the origin of the request, the credentials of the caller are no longer sent */
    bool m_finished;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CHTTPCLIENTREQUESTRUNNER_H ? */
