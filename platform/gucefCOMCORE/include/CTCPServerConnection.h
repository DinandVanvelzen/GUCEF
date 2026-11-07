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

#ifndef GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#define GUCEF_COMCORE_CTCPSERVERCONNECTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCYCLICDYNAMICBUFFER_H
#include "CCyclicDynamicBuffer.h"
#define GUCEF_CORE_CCYCLICDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CCYCLICDYNAMICBUFFER_H ? */ 
                
#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_COMCORE_CTCPCONNECTION_H
#include "CTCPConnection.h"                     /* TCP connection base class */
#define GUCEF_COMCORE_CTCPCONNECTION_H
#endif /* GUCEF_COMCORE_CTCPCONNECTION_H ? */

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

struct STCPServerConData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Forward declarations of classes used
 */
class CTCPServerSocket;

/*-------------------------------------------------------------------------*/

/**
 *      Class that represents a single connection on a TCP server socket
 */
class GUCEF_COMCORE_EXPORT_CPP CTCPServerConnection : public CTCPConnection
{
    public:

    /**
     *      Closes the connection to the client.
     *      This can cause this object to be deleted by the parent server.
     *      After a call to Close() the object should no longer be accessed
     *      by the user.
     */
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Allows you to read data from the socket when using a blocking
     *      socket approach. This member function always returns false for
     *      non-blocking sockets.         
     *      
     */
    bool Read( char *dest     , 
               UInt32 size    , 
               UInt32 &wbytes , 
               Int32 timeout  );

    /**
     *      Send data to client.
     */
    virtual bool Send( const void* dataSource , 
                       const UInt32 dataSize  ) GUCEF_VIRTUAL_OVERRIDE;

    bool Send( const CORE::CDynamicBuffer& content );

    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Set's the maximum number of bytes to be read from the socket
     *      received data buffer. A value of 0 means infinite. Setting this
     *      value to non-zero allows you to avoid the server connection or
     *      even the entire server socket (if the server socket is not using
     *      a separate thread) from getting stuck reading data. If data is
     *      being sent in such a fast rate that the continues stream will
     *      be considered to be a single transmission the server will not
     *      be able to stop reading. This is where the max read value comes
     *      in. No matter how much new data is available the reading will
     *      stop at the set number of bytes. If you have a fixed transmission
     *      length then this is easy to deal with by using a factor of the
     *      transmission length as the max read value. Otherwise you will
     *      have to check what data you need and what data should be kept.
     *      The data you think that should be kept will be prefixed to the
     *      next data buffer. You can control this process by setting the
     *      keepbytes value in the OnClientRead() event handler.
     *      In short, this helps prevent a DOS attack on the software.
     */
    void SetMaxRead( UInt32 mr );

    /**
     *      This returns the current max read value.
     *      This represents the current setting for the maximum number of
     *      bytes that will be read from the data buffer.
     */
    UInt32 GetMaxRead( void ) const;

    /**
     *  The bytes that were handed to Send() and that the socket did not take yet: Send() queues what the socket
     *  cannot take at once and the pulses of the server socket send it later
     */
    UInt32 GetBufferedSendDataSize( void ) const;
    
    virtual const CORE::CString& GetRemoteHostName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual UInt16 GetRemoteTCPPort( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual CIPv4Address GetRemoteIP( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CHostAddress GetRemoteHostAddress( void ) const;

    /**
     *	Constructor, init vars
     */
    CTCPServerConnection( CTCPServerSocket *tcp_serversock ,
                          UInt32 connection_idx            );

    virtual ~CTCPServerConnection();
    
    UInt32 GetConnectionIndex( void ) const;

    /**
     *  Slot for a pointer owned by whoever is processing this connection, for example a protocol handler which
     *  took the connection over. It saves such users from having to look up the connection in their own
     *  administration on every event. It is cleared when the connection object is used for a new client
     *  and never touched by the connection itself.
     */
    void SetUserData( void* userData );

    void* GetUserData( void ) const;

    /**
     *  Allows you to set whether to use data coalescing on sends.
     *  This is commonly refered to as the Nagle algorithm
     */
    virtual bool SetUseTcpSendCoalescing( bool coaleseData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetUseTcpSendCoalescing( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetBytesReceived( bool resetCounter ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetBytesTransmitted( bool resetCounter ) GUCEF_VIRTUAL_OVERRIDE;

    void SetDisconnectIfIdle( bool disconnectIfIdle );

    bool GetDisconnectIfIdle( void ) const;

    void SetMaxIdleDurationInMs( UInt32 maxIdleTimeInMs );

    UInt32 GetMaxIdleDurationInMs( void ) const;
    
    protected:
    friend class CTCPServerSocket;

    /**                 
     *      polls the socket ect. as needed and update stats.
     *
     *      @param maxUpdatesPerCycle Max number of socket operation update cycles per pulse
     *      @return false if this connection object was deleted as a result of notifying observers, if so
     *              the object must not be touched anymore
     */
    virtual bool Update( UInt32 maxUpdatesPerCycle );
    
    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE; 
                         
    private:
    
    /**
     *  @return false if this object was deleted as a result of notifying observers, if so it must not be touched anymore
     */
    bool CheckRecieveBuffer( void );

    void CloseImp( bool byUser, bool lock, bool updateActiveLists, bool notify );

    void
    OnIdleTimerTriggered( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData );
    
    private:
    friend class CTCPServerSocket;

    typedef CORE::CTEventHandlerFunctor< CTCPServerConnection > TEventCallback;
    
    struct STCPServerConData* _data;
    bool _blocking;  
    bool _active;
    CORE::CDynamicBuffer m_readbuffer;
    CORE::CCyclicDynamicBuffer m_sendBuffer;
    CORE::CDynamicBuffer m_sendOpBuffer;
    MT::CMutex _datalock;
    UInt32 m_maxreadbytes;
    UInt32 m_connectionidx;
    void* m_userData;      
    CTCPServerSocket* m_parentsock;
    bool m_coaleseDataSends;
    UInt32 m_bytesReceived;
    UInt32 m_bytesTransmitted;
    bool m_disconnectIfIdle;
    CORE::CTimer m_idleTimer;

    CTCPServerConnection( void ); /* private default constructor because we need data */                             
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CTCPSERVERCONNECTION_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 09-06-2007 :
        - Dinand: Finally got around to fixing this class and make it 
          compatible with the infrastructure that is also used by the TCP 
          client socket
- 29-06-2004 :
        - Dinand: Fixed a bug in the set Max_Read(). An old check that made sure the
          mr value was > 0 was still present causing a problem with the new
          max read protection and could potentially cause data to 'drop'. 
- 20-01-2004 :
        - Dinand: Added the set and get for Max_Read(). This addition is meant to
          increase the resistance of the server to data spam-ing.
- 13-01-2004 :
        - Dinand: This class now overrides the CSocket's mutex lock and unlock member
          functions so we can also lock the server socket. This prevents a
          deadlock from occurring between the two.
- 11-01-2004 :
        - Dinand: Modified code so that this object also closes the NET2 socket itself.
          All code for manipulating the lower level socket functions for the
          connection is now located in this class. The server socket should
          delete this object in a connection closing member function or a loop
          will occur causing access of a deleted object.
- 05-01-2004 :
        - Dinand: Modified the class code so that it will now process events for this
          connection itself instead of letting the parent server socket do it.
          This means setting the threading method for this connection now has
          an effect as intended.
        - Dinand: The socket will now close after an error occurred.
- 23-09-2003 :
        - Dinand: Added this section.
        - Dinand: Added Wait_Untill_Read() in order to provide a method of operation
          similar to a blocking socket. Also added Read_Data() which will allow
          a blocking socket implementation to be used. You should only call these
          member functions from a thread other then the main application thread.

-----------------------------------------------------------------------------*/

