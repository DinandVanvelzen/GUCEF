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

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <queue>

#ifndef GUCEFCOMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"       /* build defines */
#define GUCEFCOMCORE_MACROS_H
#endif /* GUCEFCOMCORE_MACROS_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#include "CSocket.h"                    /* socket base class */
#include "CTCPServerConnection.h"       /* utils class for 1 client connection */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Forward declarations of types used
 */
struct STCPServerSockData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *      Server socket class for TCP 
 */
class GUCEF_COMCORE_EXPORT_CPP CTCPServerSocket : public CSocket
{
    public:

    struct SConnectionInfo
    {
        CHostAddress hostAddress;
        CTCPServerConnection* connection;
        UInt32 connectionIndex;
    };
    typedef struct SConnectionInfo TConnectionInfo;

    struct SDisconnectInfo
    {
        TConnectionInfo connectionInfo;
        bool closedByClient;
    };
    typedef struct SDisconnectInfo TDisconnectInfo;
    
    public:
    
    static const CORE::CEvent ClientConnectedEvent;
    static const CORE::CEvent ClientDisconnectedEvent;
    static const CORE::CEvent ClientErrorEvent;        
    static const CORE::CEvent ServerSocketOpenedEvent;
    static const CORE::CEvent ServerSocketClosedEvent;
    static const CORE::CEvent ServerSocketErrorEvent;
    static const CORE::CEvent ServerSocketClientErrorEvent;
    static const CORE::CEvent ServerSocketMaxConnectionsChangedEvent;
        
    typedef CORE::TCloneableInt32   TServerSocketErrorEventData;
    typedef CORE::TCloneableInt32   TServerSocketClientErrorEventData;    
    typedef CORE::CTCloneableObj< TConnectionInfo > TClientConnectedEventData;
    typedef CORE::CTCloneableObj< TDisconnectInfo > TClientDisconnectedEventData;
    typedef CORE::TLinkedCloneableBuffer TClientDataRecievedEventData;
    typedef CORE::TCloneableInt32   TServerSocketMaxConnectionsChangedEventData;

    static void RegisterEvents( void );
    
    public:
    
    
    CTCPServerSocket( const CORE::PulseGeneratorPtr& pulseGenerator ,
                      bool blocking                                 ,
                      UInt32 maxConnections = 100UL                 );    
    
    CTCPServerSocket( bool blocking                 ,
                      UInt32 maxConnections = 100UL );

    /*
     *  Obtain pointer to connection data by means of connection index
     */
    CTCPServerConnection* GetConnection( UInt32 index );
    
    /*
     *  Obtain pointer to connection data by means of the socket Id
     */
    CTCPServerConnection* GetConnectionBySocketId( UInt32 socketId );

    UInt32 GetActiveCount( void ) const;

    /*
     *      Methods that activate or de-activate the socket
     */
    bool ListenOnPort( UInt16 port );   /* listen on default NIC with port given */

    bool Listen( void );   /* listen on default NIC with the currently configured port */
    
    void Close( void ); /* stop listening for clients, close server socket */

    bool CloseClientConnection( UInt32 connectionIndex );
    
    bool SetPort( UInt16 port );
    
    UInt16 GetPort( void ) const;
    
    bool IsActive( void ) const;
    
    /**
     *      Returns whether the socket is blocking or non-blocking
     */
    bool IsBlocking( void ) const;
    
    void SetMaxConnections( UInt32 maxConnections, bool dropConnections );
    
    UInt32 GetMaxConnections( void ) const;

    virtual UInt32 GetBytesReceived( bool resetCounter ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetBytesTransmitted( bool resetCounter ) GUCEF_VIRTUAL_OVERRIDE;
    
    void GetListenAddress( CHostAddress& listenAddress ) const;          

    void SetAutoReOpenOnError( bool autoReOpen );

    bool GetAutoReOpenOnError( void ) const;

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

    /**
     *      Send data to ALL client connections
     */
    virtual bool SendToAllClients( const void* dataSource , 
                                   const UInt32 dataSize  );

    /**
     *  Send data to the specific client connection if possible
     */
    bool SendToConnection( UInt32 connectionIndex                             ,
                           const void* dataSource                             , 
                           const UInt32 dataSize                              ,
                           const CIPv4Address* remoteClientIPCheck = GUCEF_NULL );

    void SetDisconnectClientConnectionIfIdle( bool disconnectIfIdle );

    bool GetDisconnectClientConnectionIfIdle( void ) const;

    void SetClientConnectionMaxIdleDurationInMs( UInt32 maxIdleTimeInMs );

    UInt32 GeClientConnectiontMaxIdleDurationInMs( void ) const;    
    
    virtual ~CTCPServerSocket(); 
    
    protected:
    
    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;      
    
    private:
    friend class CTCPServerConnection;

    void OnClientConnectionClosed( CTCPServerConnection* connection ,
                                   const UInt32 connectionid        ,
                                   bool closedbyclient              ,
                                   bool updateActiveLists           );

    private:
    
    typedef CORE::CTEventHandlerFunctor< CTCPServerSocket > TEventCallback;
    
    void OnPulse( CORE::CNotifier* notifier           ,
                  const CORE::CEvent& eventid         ,
                  CORE::CICloneable* eventdata = NULL );
    
    /**
     *	default constructor cannot be used, we need to know 
     *      whether this socket is blocking.
     */
    CTCPServerSocket( void );     
    
    void AcceptClients( void );     
    
    private :
    
    typedef GUCEF::vector< CTCPServerConnection*, gucef_allocator< CTCPServerConnection* > > TConnectionVector;
    typedef GUCEF::set< CTCPServerConnection*, std::less< CTCPServerConnection* >, gucef_allocator< CTCPServerConnection* > > TConnectionSet;
    typedef std::queue< CTCPServerConnection* > TConnectionQueue;
    
    struct STCPServerSockData* _data;
    bool _active; 
    bool _blocking;                        
    TConnectionVector _connections;            /**< array of all pre-allocated connection objects */
    TConnectionSet m_activeConnections;        /**< set of all currently active connections */
    TConnectionQueue m_almostInactiveConnections;
    TConnectionQueue m_almostActiveConnections;
    TConnectionSet m_inactiveConnections;      /**< set of all currently inactive connection objects ready for reuse */
    UInt16 m_port;
    MT::CMutex _datalock;
    UInt32 _timeout;         
    CORE::PulseGeneratorPtr m_pulseGenerator;
    UInt32 m_maxUpdatesPerCycle;               /**< setting aimed at preventing a busy socket from hogging all the processing */
    bool m_autoReopenOnError;                  /**< flag for feature to auto re-open the listen socket after a socket error occurred */
    bool m_lastListenFailed;
    bool m_disconnectClientsIfIdle;
    UInt32 m_maxClientConnectionIdleTime;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* CTCPSERVERSOCKET_H */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 12-03-2005 :
        - Continuation of conversion to direct winsock instead of sdl_net
- 13-01-2004 :
        - Fixed a potential deadlock between the server socket and a connection
          object which could happen on rare occasions.
        - Now uses the CDynamicArray class instead of managing it's own heap.  
- 05-01-2004 :
        - Modified server code so that each client connection can now have
          it's own threading method as designed. This means that several
          handler member functions where moved to the CTCPServerConnection
          class.
        - The server socket will now close if an error occurred.
        - Added member functions to control the threading of client connections
              - ClientCon_Threading_Method()   get+set
              - ClientCon_Default_Threading()  get+set
- 12-11-2003 :
        - Modified the class so that you no longer set a max number of client
          connections but instead the heap gets automatically resized. There is
          a define in the .cpp which define the resize amount. This change will
          make the sockets easier to use.
        - CTCPServerConnection objects are now created when there is a connection
          and not on init of the server socket.
- 30-10-2003 :
        - Changed constructor to use the new self registering system with the com
          manager.
- 23-09-2003 :
        - renamed Set_Max_Connections() to Max_Connections()
- 11-08-2003 :
        - Added mutex locks to Active_Connections().
- 05-08-2003 :
        - Added this section.
        - Changed SDL_Event_Handler into Handle_SDL_Event because of the new
          threading implementation and moved it to protected.
        - Added const keyword here and there.

-----------------------------------------------------------------------------*/


