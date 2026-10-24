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

#ifndef GUCEF_COM_CSTUNCLIENT_H
#define GUCEF_COM_CSTUNCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"      /* often used gucefCOM macros */
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

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

/**
 *  Finds the address a UDP socket has as seen from the outside, with RFC 5389 STUN Binding requests.
 *
 *  The requests go out over a socket of the caller, so the reflexive address in the answer is that of the
 *  mapping the NAT made for that very socket: the base for hole punching. The socket keeps its other uses,
 *  the client only picks out the STUN answers. Requests are sent again after 500 ms, 1 s and 2 s without an
 *  answer, the future fails after the last wait.
 */
class GUCEF_COM_PUBLIC_CPP CStunClient : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent ReflexiveAddressEvent;    /**< CStunResult: a server told the reflexive address */
    static const CORE::CEvent StunErrorEvent;           /**< TStunErrorEventData: a description of the problem */

    typedef CORE::TCloneableString TStunErrorEventData;

    static const UInt32 MagicCookie;                    /**< 0x2112A442 */

    /** The result data of the futures and the data of ReflexiveAddressEvent */
    class GUCEF_COM_PUBLIC_CPP CStunResult : public CORE::CICloneable
    {
        public:

        COMCORE::CIPv4Address reflexiveAddress;     /**< the address and port of the socket as the server saw it */
        COMCORE::CIPv4Address server;               /**< the server that answered */

        CStunResult( void );
        CStunResult( const CStunResult& src );
        virtual ~CStunResult();
        CStunResult& operator=( const CStunResult& src );
        virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    };

    /**
     *  @param socket the socket to send the requests over, the caller keeps it alive while the client lives
     */
    explicit CStunClient( COMCORE::CUDPSocket& socket );

    CStunClient( COMCORE::CUDPSocket& socket, const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CStunClient();

    /**
     *  Asks the server for the reflexive address of the socket
     *
     *  @param server the STUN server, usually on port 3478
     *  @param future receives the future, its result data is a CStunResult
     *  @return false if the request could not be sent at all
     */
    bool QueryReflexiveAddress( const COMCORE::CHostAddress& server ,
                                CORE::CFutureResult& future         );

    /**
     *  @param initialIntervalInMs the wait after the first request, doubled after each next one
     *  @param maxSends how many times a request is sent before the query fails
     */
    void SetRetransmission( UInt32 initialIntervalInMs, UInt32 maxSends );

    /** The number of queries waiting for an answer */
    UInt32 GetPendingQueryCount( void ) const;

    /** Gets the result out of a ready future, false if the query failed or is not done */
    static bool TryGetResultFromFuture( const CORE::CFutureResult& future, CStunResult& result );

    /** Writes a Binding request with the given transaction id, replacing the content of the buffer */
    static void EncodeBindingRequest( const UInt8* transactionId, CORE::CDynamicBuffer& message );

    /**
     *  Reads a Binding response
     *
     *  @param transactionId receives the 12 byte transaction id
     *  @param isSuccess false for an error response, then errorCode says why
     *  @return false if the data is not a STUN Binding response
     */
    static bool DecodeBindingResponse( const void* data                     ,
                                       UInt32 dataSize                      ,
                                       UInt8* transactionId                 ,
                                       bool& isSuccess                      ,
                                       COMCORE::CIPv4Address& mappedAddress ,
                                       UInt16& errorCode                    );

    /** Quick check of the header: the first two bits zero and the magic cookie in place */
    static bool IsStunMessage( const void* data, UInt32 dataSize );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SQuery
    {
        UInt8 transactionId[ 12 ];
        COMCORE::CIPv4Address server;
        UInt32 sends;
        UInt32 intervalInMs;
        UInt64 nextActionInMs;
        CORE::CFutureResultSource source;
    };
    typedef struct SQuery TQuery;
    typedef GUCEF::vector< TQuery > TQueryVector;

    CStunClient( const CStunClient& src );
    CStunClient& operator=( const CStunClient& src );

    bool Send( TQuery& query );
    bool OnTimer( void );
    bool OnPacket( const COMCORE::CIPv4Address& source, const CORE::CDynamicBuffer& packet );
    static UInt64 GetTimeInMs( void );

    private:

    COMCORE::CUDPSocket* m_socket;
    CORE::CTimer m_timer;
    TQueryVector m_queries;
    UInt32 m_initialIntervalInMs;
    UInt32 m_maxSends;
    CORE::CDynamicBuffer m_sendBuffer;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CSTUNCLIENT_H ? */
