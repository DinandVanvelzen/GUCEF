/*
 *  gucefCOM: GUCEF module providing communication implementations 
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

#ifndef GUCEF_COM_CASYNCHTTPSERVERRESPONSEHANDLER_H
#define GUCEF_COM_CASYNCHTTPSERVERRESPONSEHANDLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CHTTPRESPONSEDATA_H
#include "gucefCOM_CHttpResponseData.h"
#define GUCEF_COM_CHTTPRESPONSEDATA_H
#endif /* GUCEF_COM_CHTTPRESPONSEDATA_H ? */

#ifndef GUCEF_COM_CASYNCHTTPSERVERREQUESTHANDLER_H
#include "gucefCOM_CAsyncHttpServerRequestHandler.h"
#define GUCEF_COM_CASYNCHTTPSERVERREQUESTHANDLER_H
#endif /* GUCEF_COM_CASYNCHTTPSERVERREQUESTHANDLER_H ? */


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_HIDDEN CAsyncHttpServerResponseHandler : public CORE::CTaskConsumer
{
    public:

    static const CORE::CEvent AsyncHttpServerResponseHandlingCompletedEvent;
    static const CORE::CString TaskType;
    
    static void RegisterEvents( void );

    CAsyncHttpServerResponseHandler();
    CAsyncHttpServerResponseHandler( const CAsyncHttpServerResponseHandler& src );
    virtual ~CAsyncHttpServerResponseHandler();

    virtual bool OnTaskStart( CORE::CICloneable* taskData ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool OnTaskCycle( CORE::CICloneable* taskData ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual void OnTaskEnding( CORE::CICloneable* taskdata ,
                               bool willBeForced           ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual void OnTaskEnded( CORE::CICloneable* taskdata ,
                               bool wasForced             ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CORE::CTaskConsumer, CAsyncHttpServerResponseHandler > TAsyncHttpServerResponseHandlerFactory;

/*-------------------------------------------------------------------------*/

class CHTTPServer;

/*-------------------------------------------------------------------------*/

class GUCEF_HIDDEN CAsyncHttpResponseData : public CHttpResponseData
{
    public:

    CHTTPServer* httpServer;
    UInt32 clientConnectionId;
    COMCORE::CHostAddress remoteClientAddress;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CAsyncHttpResponseData( CAsyncHttpRequestData* request = GUCEF_NULL );
    CAsyncHttpResponseData( const CAsyncHttpResponseData& src );
    virtual ~CAsyncHttpResponseData();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_COM_CASYNCHTTPSERVERRESPONSEHANDLER_H ? */
