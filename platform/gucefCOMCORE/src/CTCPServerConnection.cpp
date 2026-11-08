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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#include "CTCPServerSocket.h"           /* parent server socket */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_COMCORE_SOCKETUTILS_H
#include "socketutils.h"
#define GUCEF_COMCORE_SOCKETUTILS_H
#endif /* GUCEF_COMCORE_SOCKETUTILS_H ? */

#ifdef ACTIVATE_MEMORY_MANAGER
  #ifndef GUCEF_NEW_ON_H
  #include "gucef_new_on.h"   /* Use the GUCEF memory manager instead of the standard manager ? */
  #define GUCEF_NEW_ON_H
  #endif /* GUCEF_NEW_ON_H ? */
#endif /* ACTIVATE_MEMORY_MANAGER ? */

#include "CTCPServerConnection.h"       /* header for this class */

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    #include <netinet/tcp.h>
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define BUFFER_READ_SIZE	25

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct STCPServerConData
{
    UInt32 sockid;
    CORE::CString clientip;
    struct sockaddr_in clientaddr;
    struct timeval timeout;         /* timeout for blocking operations */
};
typedef struct STCPServerConData TTCPServerConData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CTCPServerConnection::CTCPServerConnection( CTCPServerSocket *tcp_serversock ,
                                            UInt32 connection_idx            )
    : CTCPConnection()                    
    , _data( NULL )                       
    , _blocking( false )                  
    , _active( false )                    
    , m_readbuffer()                      
    , m_sendBuffer()                      
    , m_sendOpBuffer()                    
    , _datalock()                         
    , m_maxreadbytes( 0 )                 
    , m_connectionidx( connection_idx )
    , m_userData( GUCEF_NULL )   
    , m_parentsock( tcp_serversock )      
    , m_coaleseDataSends( true )          
    , m_bytesReceived( 0 )
    , m_bytesTransmitted( 0 )
    , m_disconnectIfIdle( false )
    , m_idleTimer( tcp_serversock->m_pulseGenerator, 600000 )
    , m_readinessWaiter()
    , m_readinessId( 0 )
    , m_closeToComplete( false )
    , m_closeToCompleteByUser( false )
    , m_closeToCompleteUpdateLists( false )
    , m_closeToCompleteNotify( false )
    , m_closeBeingTold( false )
    , m_lockDepth( 0 )
    , m_tlsSession()
    , m_tlsHandshakeDeadline( 0 )
    , m_tlsEstablished( false )
    , m_tlsCipherOut()
    , m_tlsPlainIn()
    , m_tlsPendingPlain()
{GUCEF_TRACE;

    _data = GUCEF_NEW TTCPServerConData;
    _data->sockid = 0;
    memset( &_data->timeout, 0, sizeof( _data->timeout ) );
    memset( &_data->clientaddr, 0, sizeof(_data->clientaddr) );

    TEventCallback callback1( this, &CTCPServerConnection::OnIdleTimerTriggered );
    SubscribeTo( &m_idleTimer                   ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback1                      );
}

/*-------------------------------------------------------------------------*/

CTCPServerConnection::~CTCPServerConnection()
{GUCEF_TRACE;

    Close();

    SignalUpcomingObserverDestruction();
    
    GUCEF_DELETE _data;
    _data = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTCPServerConnection::GetRemoteHostName( void ) const
{GUCEF_TRACE;

    return _data->clientip;
}

/*-------------------------------------------------------------------------*/

CIPv4Address
CTCPServerConnection::GetRemoteIP( void ) const
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    return CIPv4Address( _data->clientaddr.sin_addr.S_un.S_addr, _data->clientaddr.sin_port );

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    return CIPv4Address( _data->clientaddr.sin_addr.s_addr, _data->clientaddr.sin_port );

    #endif
}

/*-------------------------------------------------------------------------*/

CHostAddress
CTCPServerConnection::GetRemoteHostAddress( void ) const
{GUCEF_TRACE;

    return CHostAddress( GetRemoteIP(), GetRemoteHostName() );
}

/*-------------------------------------------------------------------------*/

UInt16
CTCPServerConnection::GetRemoteTCPPort( void ) const
{GUCEF_TRACE;

    return ntohs( _data->clientaddr.sin_port );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTCPServerConnection::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    MT::TLockStatus lockStatus = _datalock.Lock( lockWaitTimeoutInMs );
    if ( MT::LOCKSTATUS_OPERATION_SUCCESS == lockStatus )
        ++m_lockDepth;
    return lockStatus;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTCPServerConnection::Unlock( void ) const
{GUCEF_TRACE;

    if ( m_lockDepth > 0 )
        --m_lockDepth;
    return _datalock.Unlock();
}

/*-------------------------------------------------------------------------*/

void
CTCPServerConnection::Close( void )
{GUCEF_TRACE;

    CloseImp( false, true, true, true );
}

/*-------------------------------------------------------------------------*/

void
CTCPServerConnection::CloseImp( bool byUser                ,
                                bool lock                  ,
                                bool updateActiveLists     ,
                                bool notify                ,
                                bool onlyWhenOpenOrPending )
{GUCEF_TRACE;

    if ( lock )
        Lock();

    // Nothing to close and nothing to tell: the close was told already, or another thread is telling it
    if ( onlyWhenOpenOrPending && !_active && !m_closeToComplete && !m_closeToCompleteNotify )
    {
        if ( lock )
            Unlock();
        return;
    }

    // A thread that held the lock before this call (an observer of an event of this connection, a sender that closes
    // under the lock) still holds it once this call gives it up: the server socket must not hear of the close under it.
    // Not when the server socket closes all of its connections, it holds its own lock already and moves them itself
    const bool callerHoldsLock = !lock || ( updateActiveLists && m_lockDepth > 1 );

    bool closed = false;
    if ( _active )
    {
        // the close notification of TLS goes before the socket is closed
        SendTlsCloseNotify();

        // the waiter must not wait on a descriptor number the system may hand to the next socket
        UnregisterFromReadinessWake();

        int errorCode = 0;
        if ( 0 == dvsocket_closesocket( _data->sockid, &errorCode ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "TCPServerConnection(" + CORE::PointerToString( this ) + "): Closed socket for connection " + GetRemoteIP().AddressAndPortAsString() );
        
            _active = false;
            closed = true;

		    m_readbuffer.Clear( false );
		    m_sendBuffer.Clear( false );
		    m_sendOpBuffer.Clear( false );

            m_tlsSession = TTlsSessionPtr();
            m_tlsEstablished = false;
            m_tlsHandshakeDeadline = 0;
            m_tlsPendingPlain.Clear( false );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "TCPServerConnection(" + CORE::PointerToString( this ) + "): Failed close socket at " + GetRemoteIP().AddressAndPortAsString() );
        }
    }

    if ( callerHoldsLock )
    {
        // The caller holds the lock: the server socket and the observers hear of the close once it gave the lock up,
        // see UnlockAndCompleteClose(), or on the next pulse of the server socket (Update()). Until then this object
        // stays among the active connections of the server socket, so it is not handed to a later client
        if ( closed )
        {
            m_closeToComplete = true;
            m_closeToCompleteByUser = byUser;
            m_closeToCompleteUpdateLists = updateActiveLists;
        }
        m_closeToCompleteNotify = m_closeToCompleteNotify || notify;
        if ( lock )
            Unlock();
        return;
    }

    // A close that waited for the lock to be given up is completed by this one
    if ( m_closeToComplete )
    {
        closed = true;
        byUser = m_closeToCompleteByUser;
    }
    notify = notify || m_closeToCompleteNotify;
    m_closeToComplete = false;
    m_closeToCompleteByUser = false;
    m_closeToCompleteUpdateLists = false;
    m_closeToCompleteNotify = false;

    // Until the server socket heard of the close this object is not handed to a later client, also not by a server
    // socket that closed and listens again meanwhile
    if ( closed )
        m_closeBeingTold.Store( true );

    // Lock order: the server socket raises its events and sends to its connections under its own lock, which comes
    // before the lock of a connection. The server socket hears of the close after this lock is given up
    Unlock();

    // The observers first: once the server socket heard of the close it may hand this object to a later client, whose
    // observers must not get the end of this one. False means this object was deleted by one of the observers
    if ( notify && !NotifyObservers( DisconnectedEvent ) )
        return;

    if ( closed )
    {
        m_parentsock->OnClientConnectionClosed( this              ,
                                                m_connectionidx   ,
                                                byUser            ,
                                                updateActiveLists );
        m_closeBeingTold.Store( false );
    }
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::UnlockAndCompleteClose( void )
{GUCEF_TRACE;

    // The thread holds the lock more than once, for example an observer of an event of this connection closed it from
    // a thread that had the lock already: the close stays pending for the outermost hold or the next pulse
    if ( m_lockDepth > 1 )
    {
        Unlock();
        return true;
    }

    const bool closeToComplete = m_closeToComplete;
    const bool byUser = m_closeToCompleteByUser;
    const bool updateActiveLists = m_closeToCompleteUpdateLists;
    const bool notify = m_closeToCompleteNotify;
    m_closeToComplete = false;
    m_closeToCompleteByUser = false;
    m_closeToCompleteUpdateLists = false;
    m_closeToCompleteNotify = false;
    if ( closeToComplete )
        m_closeBeingTold.Store( true );
    Unlock();

    // The observers first, as in CloseImp(): the server socket may hand this object to a later client once it heard of
    // the close. False means this object was deleted by one of the observers
    if ( notify && !NotifyObservers( DisconnectedEvent ) )
        return false;

    if ( closeToComplete )
    {
        m_parentsock->OnClientConnectionClosed( this              ,
                                                m_connectionidx   ,
                                                byUser            ,
                                                updateActiveLists );
        m_closeBeingTold.Store( false );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::Send( const CORE::CDynamicBuffer& content )
{GUCEF_TRACE;

    return Send( content.GetConstBufferPtr(), content.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::SendRawLocked( const void* dataSource ,
                                     UInt32 length          ,
                                     int& socketError       )
{GUCEF_TRACE;

    socketError = 0;

    // Check if we still have data queued to be sent,..
    // TCP has to be in-order so we will have to queue the new data behind the already queued data
    if ( m_sendBuffer.HasBufferedData() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CTCPServerConnection(" << CORE::PointerToString( this ) << "): Delaying sending of data because there is still data queued to be send from the previous send" );
        m_sendBuffer.Write( dataSource, 1, length );
        ArmReadinessWake();
        return true;
    }

    // We will try looping until we have transmitted all the data
    int error = 0;
    UInt32 totalBytesSent = 0;
    Int32 wbytes;
    while ( totalBytesSent < length )
    {
        // perform a send, trying to send as much of the given data as possible
        Int32 remnant = length - totalBytesSent;
        wbytes = dvsocket_send( _data->sockid                                    ,
                                ((const Int8*)dataSource)+totalBytesSent         ,
                                remnant                                          ,
                                0                                                ,
                                &error                                           );
        if ( wbytes != SOCKET_ERROR )
        {
            // we where able to send at least some of the data
            totalBytesSent += wbytes;
            m_bytesTransmitted += wbytes;

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CTCPServerConnection(" << CORE::PointerToString( this ) << "): Succeeded in sending " << wbytes << " bytes of data" );
        }
        else
        {
            // Socket error,..
            // Check if we have to delay sending the data
            if ( DVSOCKET_EWOULDBLOCK == error && !_blocking )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CTCPServerConnection(" << CORE::PointerToString( this ) << "): Delaying sending of data" );
                m_sendBuffer.Write( ((const Int8*)dataSource)+totalBytesSent, 1, remnant );

                // The remainder is queued, it will be sent when the socket can take more data.
                // Without leaving the loop the same remainder would be queued again on every iteration
                ArmReadinessWake();
                break;
            }
            socketError = error;
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::Send( const void* dataSource ,
                            const UInt32 length    )
{GUCEF_TRACE;

    if ( IsActive() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CTCPServerConnection(" << CORE::PointerToString( this ) << "): Sending data of length " << length );

        Lock();

        // Closed while this waited for the lock: the descriptor number of the closed socket may belong to a new socket by
        // now, of another client or of anything else in the process
        if ( !_active )
        {
            Unlock();
            return false;
        }

        // notify observers that we are sending data
        CORE::CDynamicBuffer linkedBuffer;
        linkedBuffer.LinkTo( dataSource, length );
        TDataRecievedEventData cData( &linkedBuffer );
        if ( !NotifyObservers( DataSentEvent, &cData ) ) return false;

        int error = 0;
        bool sent = true;
        if ( m_tlsSession.IsNULL() )
        {
            sent = SendRawLocked( dataSource, length, error );
        }
        else
        if ( m_tlsEstablished && TLSSTATE_ESTABLISHED == m_tlsSession->GetState() )
        {
            if ( TLSSTATE_ESTABLISHED != m_tlsSession->PushPlaintext( dataSource, length ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CTCPServerConnection(" + CORE::PointerToString( this ) + "): TLS cannot encrypt the data for " + GetRemoteIP().AddressAndPortAsString() + ": " + m_tlsSession->GetError().message );
                Unlock();
                return false;
            }
            sent = FlushTlsOutputLocked( error );
        }
        else
        if ( !m_tlsEstablished && ( TLSSTATE_HANDSHAKING == m_tlsSession->GetState() || TLSSTATE_IDLE == m_tlsSession->GetState() ) )
        {
            // sent once the handshake completed
            m_tlsPendingPlain.Append( dataSource, length );
        }
        else
        {
            // the TLS of the connection failed or was closed
            Unlock();
            return false;
        }

        Unlock();

        if ( !sent )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPServerConnection(" + CORE::PointerToString( this ) + "): Socket error occured: " + CORE::Int32ToString( error ) );

            TSocketErrorEventData eData( error );
            NotifyObservers( SocketErrorEvent, &eData );
            return false;
        }
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

void
CTCPServerConnection::SetMaxRead( UInt32 mr )
{GUCEF_TRACE;

    /*
     *      Although the setting of the int value may be atomic we must
     *      use a mutex lock so that a current read proccess will not be
     *      affected. Thus the setting will take effect on the next read
     *      cycle.
     */
    Lock();
    m_maxreadbytes = mr;
    Unlock();
}

/*-------------------------------------------------------------------------*/

UInt32
CTCPServerConnection::GetMaxRead( void ) const
{GUCEF_TRACE;

    return m_maxreadbytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CTCPServerConnection::GetBufferedSendDataSize( void ) const
{GUCEF_TRACE;

    Lock();
    UInt32 bufferedBytes = m_sendBuffer.GetBufferedDataSizeInBytes();
    Unlock();
    return bufferedBytes;
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::Read( char *dest     ,
                            UInt32 size    ,
                            UInt32 &wbytes ,
                            Int32 timeout  )
{GUCEF_TRACE;
            //@TODO: blocking socket support
     //   _datalock.Lock();
     //   recv( _data->sockid              ,
     //         _readbuffer.GetBufferPtr() ,
     //         _readbuffer.GetSize()      ,
     //         0                          );


     /*
        if ( !datalock.Lock_Mutex() ) return false;
	rb_dest = dest;
        rb_size = size;
        rb_wbytes = &wbytes;
	read_flag = true;
        datalock.Unlock_Mutex();

        return Wait_Untill_Read( timeout );   */
        return false;
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::CheckRecieveBuffer( void )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CTCPServerConnection(" << CORE::PointerToString( this ) << "): Checking the recieve buffer for incoming data" );

    /*
     *      Since this is a non-blocking socket we need to poll for received data
     */
    int bytesrecv;
    UInt32 totalrecieved( 0 );
    int errorcode;
    UInt32 readblocksize;
    // a TLS record holds up to 16 KiB
    m_maxreadbytes ? readblocksize = m_maxreadbytes : readblocksize = ( m_tlsSession.IsNULL() ? 1024 : 16384 );
    do
    {
        m_readbuffer.SetDataSize( m_readbuffer.GetDataSize()+readblocksize );
        bytesrecv = dvsocket_recv( _data->sockid                                                 ,
                                   static_cast<char*>(m_readbuffer.GetBufferPtr())+totalrecieved ,
                                   readblocksize                                                 ,
                                   0                                                             ,
                                   &errorcode                                                    );

        // A non-blocking socket which has been read empty reports that nothing more can be read right now,
        // which is the normal end of a read cycle when the amount of data was an exact multiple of the block size
        if ( bytesrecv == SOCKET_ERROR && DVSOCKET_EWOULDBLOCK == errorcode && !_blocking )
        {
            // the buffer was grown to receive more, only what we actually got is valid
            m_readbuffer.SetDataSize( totalrecieved );
            break;
        }

        // Check for an error
        if ( ( bytesrecv == SOCKET_ERROR ) || ( errorcode != 0 ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPServerConnection(" + CORE::PointerToString( this ) + "): Socket error occured: " + CORE::Int32ToString( errorcode ) );

            // Notify our users of the error
            TSocketErrorEventData eData( errorcode );
            if ( !NotifyObservers( SocketErrorEvent, &eData ) )
                return false;

            CloseImp( true, false, true, false );
            return true;
        }

        if ( (  0 == totalrecieved ) &&
             (  0 == bytesrecv     )  )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPServerConnection(" + CORE::PointerToString( this ) + "): Client closed the connection" );

            /*
             *      we arrived here because the read flag was set, however no data is available
             *      This means that the client closed the connection
             */
            CloseImp( true, false, true, true );
            return true;
        }
        totalrecieved += bytesrecv;
        m_readbuffer.SetDataSize( totalrecieved );

        if ( m_maxreadbytes > 0 )
        {
            if ( m_maxreadbytes <= totalrecieved )
            {
                // This is a busy socket, don't yield to the scheduler
                m_parentsock->m_pulseGenerator->RequestImmediatePulse();
                break;
            }
        }
    }
    while ( bytesrecv == readblocksize );

    if ( totalrecieved > 0 )
    {
        m_idleTimer.Reset();
        m_bytesReceived += totalrecieved;

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CTCPServerConnection(" << CORE::PointerToString( this ) << "): Received " << totalrecieved << " bytes of data" );

        if ( !m_tlsSession.IsNULL() )
            return ProcessTlsInput();

        TDataRecievedEventData eData( &m_readbuffer );

        // false means this object was deleted by one of the observers, we must not touch it anymore
        return NotifyObservers( DataRecievedEvent, &eData );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTCPServerConnection::BeginTls( const TTlsSessionPtr& session, UInt32 handshakeTimeoutInMs )
{GUCEF_TRACE;

    Lock();
    m_tlsSession = session;
    m_tlsEstablished = false;
    m_tlsPendingPlain.Clear( false );
    m_tlsHandshakeDeadline = 0;
    if ( !session.IsNULL() && 0 != handshakeTimeoutInMs )
        m_tlsHandshakeDeadline = MT::PrecisionTickCount() + ( (UInt64) handshakeTimeoutInMs * MT::PrecisionTimerResolution() ) / 1000;
    Unlock();
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::FlushTlsOutputLocked( int& socketError )
{GUCEF_TRACE;

    socketError = 0;
    if ( m_tlsSession.IsNULL() )
        return true;

    m_tlsCipherOut.SetDataSize( 0 );
    if ( !m_tlsSession->PullCiphertext( m_tlsCipherOut ) || 0 == m_tlsCipherOut.GetDataSize() )
        return true;
    return SendRawLocked( m_tlsCipherOut.GetConstBufferPtr(), m_tlsCipherOut.GetDataSize(), socketError );
}

/*-------------------------------------------------------------------------*/

void
CTCPServerConnection::SendTlsCloseNotify( void )
{GUCEF_TRACE;

    if ( m_tlsSession.IsNULL() || !m_tlsEstablished )
        return;

    m_tlsSession->InitiateClose();
    int socketError = 0;
    FlushTlsOutputLocked( socketError );
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::ProcessTlsInput( void )
{GUCEF_TRACE;

    ETlsState state = m_tlsSession->PushCiphertext( m_readbuffer.GetConstBufferPtr(), m_readbuffer.GetDataSize() );

    // the answer of the session: the handshake, alerts, tickets
    int socketError = 0;
    if ( !FlushTlsOutputLocked( socketError ) )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPServerConnection(" + CORE::PointerToString( this ) + "): Socket error occured: " + CORE::Int32ToString( socketError ) );
        TSocketErrorEventData eData( socketError );
        if ( !NotifyObservers( SocketErrorEvent, &eData ) )
            return false;
        CloseImp( true, false, true, false );
        return true;
    }

    if ( TLSSTATE_FAILED == state )
    {
        // a client that does not speak TLS, or that refused the certificate: nothing it sent is delivered
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CTCPServerConnection(" + CORE::PointerToString( this ) + "): TLS with the client " + GetRemoteIP().AddressAndPortAsString() +
                   " failed (" + TlsErrorClassToString( m_tlsSession->GetError().errorClass ) + "): " + m_tlsSession->GetError().message );
        CloseImp( true, false, true, true );
        return true;
    }

    // the handshake completed: the session reports what was negotiated from then on
    if ( !m_tlsEstablished && !m_tlsSession->GetInfo().protocolVersion.IsNULLOrEmpty() )
    {
        m_tlsEstablished = true;
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CTCPServerConnection(" + CORE::PointerToString( this ) + "): TLS with the client " + GetRemoteIP().AddressAndPortAsString() + ": " + m_tlsSession->GetInfo().GetSummary() );

        if ( m_tlsPendingPlain.GetDataSize() > 0 && TLSSTATE_ESTABLISHED == state )
        {
            m_tlsSession->PushPlaintext( m_tlsPendingPlain.GetConstBufferPtr(), m_tlsPendingPlain.GetDataSize() );
            m_tlsPendingPlain.Clear( false );
            if ( !FlushTlsOutputLocked( socketError ) )
            {
                TSocketErrorEventData eData( socketError );
                if ( !NotifyObservers( SocketErrorEvent, &eData ) )
                    return false;
                CloseImp( true, false, true, false );
                return true;
            }
        }
    }

    m_tlsPlainIn.SetDataSize( 0 );
    if ( m_tlsSession->PullPlaintext( m_tlsPlainIn ) && m_tlsPlainIn.GetDataSize() > 0 )
    {
        TDataRecievedEventData eData( &m_tlsPlainIn );

        // false means this object was deleted by one of the observers, we must not touch it anymore
        if ( !NotifyObservers( DataRecievedEvent, &eData ) )
            return false;
    }

    // An observer may have closed the connection, which ends its session. The close notification of the client, or
    // one during the handshake, is answered and ends the connection
    if ( _active && !m_tlsSession.IsNULL() && TLSSTATE_CLOSED == m_tlsSession->GetState() )
        CloseImp( true, false, true, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::IsTls( void ) const
{GUCEF_TRACE;

    Lock();
    bool isTls = !m_tlsSession.IsNULL();
    Unlock();
    return isTls;
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::IsTlsEstablished( void ) const
{GUCEF_TRACE;

    Lock();
    bool established = !m_tlsSession.IsNULL() && m_tlsEstablished;
    Unlock();
    return established;
}

/*-------------------------------------------------------------------------*/

CTlsSessionInfo
CTCPServerConnection::GetTlsSessionInfo( void ) const
{GUCEF_TRACE;

    CTlsSessionInfo info;
    Lock();
    if ( !m_tlsSession.IsNULL() && m_tlsEstablished )
        info = m_tlsSession->GetInfo();
    Unlock();
    return info;
}

/*-------------------------------------------------------------------------*/

UInt32
CTCPServerConnection::GetConnectionIndex( void ) const
{GUCEF_TRACE;

    return m_connectionidx;
}

/*-------------------------------------------------------------------------*/

void
CTCPServerConnection::OnIdleTimerTriggered( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CTCPServerConnection(" + CORE::PointerToString( this ) + "): Idle timer tiggered. Closing connection due to inactivity" );
    Close();
}

/*-------------------------------------------------------------------------*/

void 
CTCPServerConnection::SetDisconnectIfIdle( bool disconnectIfIdle )
{GUCEF_TRACE;

    if ( m_disconnectIfIdle != disconnectIfIdle )
    {
        m_disconnectIfIdle = disconnectIfIdle;
        m_idleTimer.SetEnabled( m_disconnectIfIdle );
    }
}

/*-------------------------------------------------------------------------*/

bool 
CTCPServerConnection::GetDisconnectIfIdle( void ) const
{GUCEF_TRACE;

    return m_disconnectIfIdle;
}

/*-------------------------------------------------------------------------*/

void 
CTCPServerConnection::SetMaxIdleDurationInMs( UInt32 maxIdleTimeInMs )
{GUCEF_TRACE;

    m_idleTimer.SetInterval( maxIdleTimeInMs );
}

/*-------------------------------------------------------------------------*/

UInt32 
CTCPServerConnection::GetMaxIdleDurationInMs( void ) const
{GUCEF_TRACE;

    return m_idleTimer.GetInterval();
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::CompletePendingClose( void )
{GUCEF_TRACE;

    Lock();
    if ( m_closeToComplete || m_closeToCompleteNotify )
        return UnlockAndCompleteClose();
    Unlock();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::Update( UInt32 maxUpdatesPerCycle )
{GUCEF_TRACE;

    // a close made by a thread that held the lock of this connection is told to the server socket here, once that
    // thread gave the lock up
    if ( !_active )
        return CompletePendingClose();

    if ( !_blocking && _active )
    {
        fd_set readfds;      /* Setup the read variable for the select function */
        fd_set exceptfds;    /* Setup the except variable for the select function */

        FD_ZERO( &readfds );
        FD_ZERO( &exceptfds );

        Lock();

        // a client that connected and did not complete the TLS handshake in time
        if ( !m_tlsSession.IsNULL() && !m_tlsEstablished && 0 != m_tlsHandshakeDeadline && MT::PrecisionTickCount() > m_tlsHandshakeDeadline )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CTCPServerConnection(" + CORE::PointerToString( this ) + "): The client " + GetRemoteIP().AddressAndPortAsString() + " did not complete the TLS handshake in time, closing the connection" );
            CloseImp( true, false, true, true );
            return UnlockAndCompleteClose();
        }

        FD_SET( _data->sockid, &readfds );
        FD_SET( _data->sockid, &exceptfds );

        int errorCode = 0;
        if ( dvsocket_select( _data->sockid+1   ,
                              &readfds          ,
                              NULL              , // We don't care about socket writes here
                              &exceptfds        ,
                              &_data->timeout   ,
                              &errorCode        ) != SOCKET_ERROR )
        {
            /* something happened on the socket */

            if ( FD_ISSET( _data->sockid, &exceptfds ) )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPServerConnection(" + CORE::PointerToString( this ) + "): Socket error occured: " + CORE::Int32ToString( errorCode ) );

                TSocketErrorEventData eData( errorCode );
                if ( !NotifyObservers( SocketErrorEvent, &eData ) ) return false;

                CloseImp( true, false, true, false );

                return UnlockAndCompleteClose();
            }
            else
            if ( FD_ISSET( _data->sockid, &readfds ) )
            {
                /* data can be read from the socket */
                // false means this object was deleted by one of the observers, we must not touch it anymore
                if ( !CheckRecieveBuffer() )
                    return false;
            }
        }
        else
        {
            /* select call failed */
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPServerConnection(" + CORE::PointerToString( this ) + "): Socket error occured (select call failed): " + CORE::Int32ToString( errorCode ) );

            TSocketErrorEventData eData( errorCode );
            NotifyObservers( SocketErrorEvent, &eData );

            CloseImp( false, false, true, false );
        }

        // Check if we still have data queued to be sent,..
        while ( m_sendBuffer.HasBufferedData() )
        {
            // read an entire block
            if ( 0 != m_sendBuffer.ReadBlockTo( m_sendOpBuffer ) )
            {
                // We will try looping until we have transmitted all the data
                int error;
                UInt32 totalBytesSent = 0;
                Int32 wbytes;
                while ( totalBytesSent < m_sendOpBuffer.GetDataSize() )
                {
                    // perform a send, trying to send as much of the given data as possible
                    const Int8* data = static_cast< const Int8* >( m_sendOpBuffer.GetConstBufferPtr() );
                    Int32 remnant = m_sendOpBuffer.GetDataSize() - totalBytesSent;
                    wbytes = dvsocket_send( _data->sockid       ,
                                            data+totalBytesSent ,
                                            remnant             ,
                                            0                   ,
                                            &error              );
                    if ( wbytes != SOCKET_ERROR )
                    {
                        // we where able to send at least some of the data
                        totalBytesSent += wbytes;
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CTCPServerConnection(" << CORE::PointerToString( this ) << "): Succeeded in delayed sending " << wbytes << "bytes of queued data" );
                    }
                    else
                    {
                        // Socket error,..
                        // Check if we have to delay sending the data
                        if ( DVSOCKET_EWOULDBLOCK == error && !_blocking )
                        {
                            // Cannot send now,... try again next pulse
                            // We have to place remaining data we grabbed from the send buffer back in
                            // the send buffer in a FILO manner
                            m_sendBuffer.WriteAtFrontOfQueue( data+totalBytesSent ,
                                                              remnant             ,
                                                              1                   );

                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CTCPServerConnection(" << CORE::PointerToString( this ) << "): Unable to delayed send queued data at this time" );

                            // writable again ends the wait
                            ArmReadinessWake();
                            Unlock();
                            return true;
                        }
                        else
                        {
                            Unlock();

                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CTCPServerConnection(" + CORE::PointerToString( this ) + "): Socket error occured: " + CORE::Int32ToString( error ) );

                            TSocketErrorEventData eData( error );
                            NotifyObservers( SocketErrorEvent, &eData );
                            return true;
                        }
                    }
                }
            }
        }

        ArmReadinessWake();
        return UnlockAndCompleteClose();
    }

    return true;
}

/*-------------------------------------------------------------------------*/

void
CTCPServerConnection::RegisterForReadinessWake( const CSocketReadinessWaiterPtr& waiter )
{GUCEF_TRACE;

    UnregisterFromReadinessWake();
    m_readinessWaiter = waiter;
    if ( !m_readinessWaiter.IsNULL() && !_blocking )
        m_readinessId = m_readinessWaiter->Register( (UInt64) _data->sockid, m_parentsock->m_pulseGenerator );
}

/*-------------------------------------------------------------------------*/

void
CTCPServerConnection::ArmReadinessWake( void )
{GUCEF_TRACE;

    if ( 0 == m_readinessId || !_active )
        return;
    UInt32 interest = CSocketReadinessWaiter::ReadinessRead;
    if ( m_sendBuffer.HasBufferedData() )
        interest |= CSocketReadinessWaiter::ReadinessWrite;
    m_readinessWaiter->Arm( m_readinessId, interest );
}

/*-------------------------------------------------------------------------*/

void
CTCPServerConnection::UnregisterFromReadinessWake( void )
{GUCEF_TRACE;

    if ( 0 == m_readinessId )
        return;
    m_readinessWaiter->Unregister( m_readinessId );
    m_readinessId = 0;
}

/*-------------------------------------------------------------------------*/

void
CTCPServerConnection::SetUserData( void* userData )
{GUCEF_TRACE;

    m_userData = userData;
}

/*-------------------------------------------------------------------------*/

void*
CTCPServerConnection::GetUserData( void ) const
{GUCEF_TRACE;

    return m_userData;
}

/*-------------------------------------------------------------------------*/

UInt32
CTCPServerConnection::GetBytesReceived( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        UInt32 bytesReceived = m_bytesReceived;
        m_bytesReceived = 0;
        return bytesReceived;
    }
    else
        return m_bytesReceived;
}

/*-------------------------------------------------------------------------*/

UInt32 
CTCPServerConnection::GetBytesTransmitted( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        UInt32 bytesTransmitted = m_bytesTransmitted;
        m_bytesTransmitted = 0;
        return bytesTransmitted;
    }
    else
        return m_bytesTransmitted;
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::IsActive( void ) const
{GUCEF_TRACE;

    return _active;
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::SetUseTcpSendCoalescing( bool coaleseData )
{
    if ( _active )
    {
        int flag = (coaleseData ? 1 : 0);
        int errorCode = 0;
        if ( -1 == dvsocket_setsockopt( _data->sockid, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag), &errorCode ) )
        {
            return false;
        }
    }

    m_coaleseDataSends = coaleseData;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTCPServerConnection::GetUseTcpSendCoalescing( void ) const
{
    return m_coaleseDataSends;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
