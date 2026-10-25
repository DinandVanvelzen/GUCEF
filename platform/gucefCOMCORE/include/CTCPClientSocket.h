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

#ifndef GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#define GUCEF_COMCORE_CTCPCLIENTSOCKET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CCYCLICDYNAMICBUFFER_H
#include "CCyclicDynamicBuffer.h"
#define GUCEF_CORE_CCYCLICDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CCYCLICDYNAMICBUFFER_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_COMCORE_CTCPCONNECTION_H
#include "CTCPConnection.h"                     /* TCP connection base class */
#define GUCEF_COMCORE_CTCPCONNECTION_H
#endif /* GUCEF_COMCORE_CTCPCONNECTION_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"  /* GUCEF COMCORE specific macros and build defs */
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
 *      TCP Client socket class
 */
class GUCEF_COMCORE_EXPORT_CPP CTCPClientSocket : public CTCPConnection
{
    public:
    
    static const CORE::CEvent ConnectingEvent;

    static void RegisterEvents( void );
    
    public:
        
    CTCPClientSocket( CORE::PulseGeneratorPtr pulseGenerator ,
                      bool blocking                          );
    
    CTCPClientSocket( bool blocking );
    
    virtual ~CTCPClientSocket();

    bool ConnectTo( const CORE::CString& address , 
                    UInt16 port                  ,
                    bool blocking = false        ); 

    bool ConnectTo( const CIPv4Address& address ,
                    bool blocking = false     );

    bool ConnectTo( const CHostAddress& address ,
                    bool blocking = false       );

    /**
     *      Attempts to reconnect to the server provided with
     *      Connect_To(). If ConnectTo() has not yet been called then this
     *      member function has no effect.
     */
    bool Reconnect( bool blocking = false );
    
    /**
     *  close the socket connection 
     */
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;  
    
    virtual bool Send( const void* dataSource , 
                       const UInt32 dataSize  ) GUCEF_VIRTUAL_OVERRIDE;    

    /**
     *      Attempt to send the data and returns immediately.
     *      The return value indicates the success/failure.
     *      You can call this from multiple threads, however if you do then
     *      the order in which the data will be send should be considered
     *      to be random.        
     */
    bool Send( const void* data   , 
               UInt32 length      ,
               UInt32 timeout     );

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
    
    UInt32 GetMaxRead( void ) const;

    UInt32 GetBufferedDataToSendInBytes( void ) const;

    virtual UInt32 GetBytesReceived( bool resetCounter ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetBytesTransmitted( bool resetCounter ) GUCEF_VIRTUAL_OVERRIDE;

    /** 
     *
     */
    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE; 

    /**
     *      
     */       
    bool IsBlocking( void ) const;

    virtual const CORE::CString& GetRemoteHostName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual UInt16 GetRemoteTCPPort( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual CIPv4Address GetRemoteIP( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Allows you to set whether to use data coalescing on sends.
     *  This is commonly refered to as the Nagle algorithm
     */
    virtual bool SetUseTcpSendCoalescing( bool coaleseData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetUseTcpSendCoalescing( void ) const GUCEF_VIRTUAL_OVERRIDE;

    bool SetUseTcpKeepAlive( bool keepAlive );

    bool GetUseTcpKeepAlive( void ) const;

    bool SetTcpKeepAliveProbeFailMax( UInt8 maxFails );

    UInt8 GetTcpKeepAliveProbeFailMax( void ) const;

    void SetAutoReconnectOnError( bool autoReOpen );

    bool GetAutoReconnectOnError( void ) const;

    /**
     *  sets the max number of updates to perform on the socket for a given update cycle
     *  This setting applies to running a socket in non-blocking event driven mode.
     *  If you have a very busy socket you don't want the socket to get 'stuck' reading from the socket for example
     *  due to the fact that data is coming in so fast that by the time you finished reading from the socket more 
     *  data is waiting on the socket. In such cases you should use this setting to avoid this scenario.
     *
     *  @param maxUpdates number of updates to perform per update cycle
     */
    void SetMaxUpdatesPerCycle( UInt32 maxUpdates );

    UInt32 GetMaxUpdatesPerCycle( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    public:
    
    struct STCPClientSockData;
    
    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CTlsClientConnection;      /* cycles the socket itself for a blocking connect that includes the TLS handshake */

    typedef CORE::CTEventHandlerFunctor< CTCPClientSocket > TEventCallback;
    
    void OnPulse( CORE::CNotifier* notifier           ,
                  const CORE::CEvent& eventid         ,
                  CORE::CICloneable* eventdata = NULL );

    CTCPClientSocket( void );                /* default constructor cannot be used since we need to register the socket */
    CTCPClientSocket& operator=( const CTCPClientSocket& src ); /* making a copy of a socket doesn't make sense */
    
    /**
     *  @return false if this object was deleted as a result of notifying observers, if so it must not be touched anymore
     */
    bool CheckRecieveBuffer( void );

    bool Connect( bool blocking );

    /**
     *  @return false if this object was deleted as a result of notifying observers of the disconnect, if so it must not be touched anymore
     */
    bool CloseImp( void );
    
    private:
            
    struct STCPClientSockData* _data;
    bool _blocking;
    bool m_isConnecting;
    bool _active;
    MT::CMutex datalock;
    CORE::CDynamicBuffer m_readbuffer;
    CORE::CCyclicDynamicBuffer m_sendBuffer;
    CORE::CDynamicBuffer m_sendOpBuffer;
    UInt32 m_maxreadbytes;                  /**< max number of bytes to receive before processing it */ 
    CHostAddress m_hostAddress;             /**< network order IP address */
    CIPv4Address m_ipv4Target;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    bool m_coaleseDataSends;
    UInt32 m_maxUpdatesPerCycle;            /**< setting aimed at preventing a busy socket from hogging all the processing */
    bool m_autoReconnectOnError;            /**< flag for feature to auto reconnect after a socket error occurred */
    bool m_lastConnFailed;
    bool m_useTcpKeepAlive;
    UInt8 m_maxKeepAliveProbeFails;
    UInt32 m_bytesReceived;
    UInt32 m_bytesTransmitted;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CTCPCLIENTSOCKET_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 12-03-2005 :
        - Initial version of the new implementation using winsock directly,
          got rid of all the SDL crap.
- 19-04-2004 :
        - The Ping() member function will from now on only be available if
          NETWORK_ICMP_SUPPORT is defined. 
- 17-04-2004 :
        - Added Send_Data_And_Wait(). This member function helps get around the
          problems that occur when using a separate wait function since then a
          response could be received before the wait is reached.
- 05-12-2003 :
        - Changed behavior. If a socket error occurs the socket will be closed.
- 20-12-2003 :
        - Fixed a small bug when resizing buffer which caused a '\0' to replace
          some characters.
- 06-12-2003 :
        - Changed code in the Handle_Recieved() member function. Allocation of
          storage now happens less because the buffer is recycled between
          transmissions. This meant the addition of 2 private data members. 
- 30-10-2003 :
        - Changed constructor to use the new self registering system with the com
          manager.
- 23-09-2003 :
        - Added Wait_Untill_Read() in order to provide a method of operation
          similar to a blocking socket. Also added Read_Data() which will allow
          a blocking socket implementation to be used. You should only call these
          member functions from a thread other then the main application thread.
- 05-08-2003 :
        - Changed SDL_Event_Handler into Handle_SDL_Event because of the new
          threading implementation and moved it to protected.
- 13-02-2003 :
        - Added this section.

-----------------------------------------------------------------------------*/
