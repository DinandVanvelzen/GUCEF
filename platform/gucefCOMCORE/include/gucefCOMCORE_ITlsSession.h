/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_ITLSSESSION_H
#define GUCEF_COMCORE_ITLSSESSION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_COMCORE_TLSTYPES_H
#include "gucefCOMCORE_TlsTypes.h"
#define GUCEF_COMCORE_TLSTYPES_H
#endif /* GUCEF_COMCORE_TLSTYPES_H ? */

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
 *  The TLS state of one connection as a filter over a byte stream, without a socket inside: ciphertext from the
 *  peer goes in, plaintext for the application comes out, and the other way round. It never blocks, never
 *  creates a thread and never touches the network, so every provider can be tested through the same fake peer.
 *  A session is used by one thread at a time. Not copyable.
 *
 *  Output is appended to a buffer of the caller (the rule of the codec instances): a caller that reuses a buffer
 *  and clears it logically allocates nothing once the buffer has its working size.
 *
 *  Typical use of a client:
 *    Start(); PullCiphertext() -> send;
 *    on data from the socket: PushCiphertext(); PullCiphertext() -> send; PullPlaintext() -> application;
 *    on data from the application, once ESTABLISHED: PushPlaintext(); PullCiphertext() -> send.
 */
class GUCEF_COMCORE_EXPORT_CPP ITlsSession
{
    public:

    virtual ~ITlsSession();

    /** Client: makes the first handshake message available to PullCiphertext(). @return false if the session cannot start, see GetError() */
    virtual bool Start( void ) = 0;

    /**
     *  Takes in all of the bytes of the peer, however they are cut up: any segmentation of the stream is fine.
     *  Advances the handshake and decrypts what is application data, to be taken with PullPlaintext().
     *  The output the peer is waiting for (handshake messages, alerts, tickets) is available to PullCiphertext().
     *  @return the state after the data
     */
    virtual ETlsState PushCiphertext( const void* data, UInt32 size ) = 0;

    /** Encrypts application data. Only in ESTABLISHED, the records become available to PullCiphertext(). Takes it all or fails the session */
    virtual ETlsState PushPlaintext( const void* data, UInt32 size ) = 0;

    /** Appends the bytes for the peer to the buffer, false when there were none */
    virtual bool PullCiphertext( CORE::CDynamicBuffer& out ) = 0;

    /** Appends the decrypted application data to the buffer, false when there was none */
    virtual bool PullPlaintext( CORE::CDynamicBuffer& out ) = 0;

    virtual ETlsState GetState( void ) const = 0;

    /** Produces the close notification for the peer, available to PullCiphertext(). No application data can be sent afterwards */
    virtual void InitiateClose( void ) = 0;

    /** What went wrong, valid in the FAILED state */
    virtual const CTlsError& GetError( void ) const = 0;

    /** What was negotiated, valid once ESTABLISHED. peerClosedCleanly is kept up to date until the end */
    virtual const CTlsSessionInfo& GetInfo( void ) const = 0;

    protected:

    ITlsSession( void );

    private:

    ITlsSession( const ITlsSession& src );
    ITlsSession& operator=( const ITlsSession& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< ITlsSession, MT::CMutex > TTlsSessionPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_ITLSSESSION_H ? */
