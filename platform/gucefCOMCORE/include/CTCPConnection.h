/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CTCPCONNECTION_H
#define GUCEF_COMCORE_CTCPCONNECTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/ 

#ifndef GUCEF_COMCORE_CSOCKET_H
#include "CSocket.h"
#define GUCEF_COMCORE_CSOCKET_H
#endif /* GUCEF_COMCORE_CSOCKET_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Base class for TCP based socket connections
 */
class GUCEF_COMCORE_EXPORT_CPP CTCPConnection : public CSocket
{
    public:

    static const CORE::CEvent ConnectedEvent;
    static const CORE::CEvent DisconnectedEvent;
    static const CORE::CEvent DataRecievedEvent;
    static const CORE::CEvent DataSentEvent;
    static const CORE::CEvent SocketErrorEvent;

    typedef CORE::TLinkedCloneableBuffer TDataRecievedEventData;
    typedef CORE::TLinkedCloneableBuffer TDataSentEventData;
    typedef CORE::TCloneableInt32        TSocketErrorEventData;
    
    static void RegisterEvents( void );
    
    public:
    
    CTCPConnection( void );

    virtual ~CTCPConnection();
    
    virtual CIPv4Address GetRemoteIP( void ) const = 0;
    
    virtual const CORE::CString& GetRemoteHostName( void ) const = 0;
    
    virtual UInt16 GetRemoteTCPPort( void ) const = 0;
    
    virtual bool IsActive( void ) const = 0;
    
    virtual void Close( void ) = 0;
    
    virtual bool Send( const void* dataSource ,
                       const UInt32 dataSize  ) = 0;

    bool SendString( const CString& data );

    /**
     *  Allows you to set whether to use data coalescing on sends.
     *  This is commonly refered to as the Nagle algorithm: true leaves it on (TCP_NODELAY 0), false switches it
     *  off (TCP_NODELAY 1, every send goes out at once). The default is false, no coalescing.
     */
    virtual bool SetUseTcpSendCoalescing( bool coaleseData ) = 0;

    virtual bool GetUseTcpSendCoalescing( void ) const = 0;

    virtual const CORE::CString& GetClassTypeName( void ) const;
 
    private:
    
    CTCPConnection( const CTCPConnection& src );            /**< connections are unique and cannot be copied */
    CTCPConnection& operator=( const CTCPConnection& src ); /**< connections are unique and cannot be copied */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CTCPCONNECTION_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 02-03-2007 :
        - Dinand: re-added this header

-----------------------------------------------------------------------------*/
