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

#ifndef GUCEF_COMCORE_CCOM_H
#define GUCEF_COMCORE_CCOM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <set>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_COMCORE_CICOMMUNICATIONINTERFACE_H
#include "gucefCOMCORE_CICommunicationInterface.h"
#define GUCEF_COMCORE_CICOMMUNICATIONINTERFACE_H
#endif /* GUCEF_COMCORE_CICOMMUNICATIONINTERFACE_H ? */

#ifndef GUCEF_COMCORE_CINETWORKINTERFACE_H
#include "gucefCOMCORE_CINetworkInterface.h"
#define GUCEF_COMCORE_CINETWORKINTERFACE_H
#endif /* GUCEF_COMCORE_CINETWORKINTERFACE_H ? */

#ifndef GUCEF_COMCORE_CSOCKET_H
#include "CSocket.h"
#define GUCEF_COMCORE_CSOCKET_H
#endif /* GUCEF_COMCORE_CSOCKET_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"      /* build switches and macros */
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

class CSocket;
class CActiveComPump;
class CComCoreGlobal;

/*-------------------------------------------------------------------------*/

/**
 *      Singleton communication manager for sockets.
 *      Provides a global control point for all sockets.
 */
class GUCEF_COMCORE_EXPORT_CPP CCom
{
    public:

    typedef CORE::CString::StringVector                     TStringList;
    typedef CINetworkInterface::TINetworkInterfacePtrVector TINetworkInterfacePtrVector;
    
    struct SSocketStats
    {
        UInt32 bytes_sent;       /** the total number of bytes sent by all sockets */
        UInt32 bytes_recieved;   /** the total number of bytes received by all sockets */
        UInt32 bps_sent;         /** current sent bytes per second ratio of this update cycle */
        UInt32 bps_recieved;     /** current received bytes per second ratio of this update cycle */
    };
    typedef struct SSocketStats TSocketStats;

    /**
     *	returns the total number of sockets that are currently registered
     *	at this manager.
     *
     *      @return the current number of registered sockets
     */
    UInt32 GetSocketCount( void ) const;

    /**
     *
     */
    void SetUseGlobalStats( bool keep_gstats );

    /**
     *
     */
    bool GetUseGlobalStats( void ) const;

    /**
     *
     */
    void ResetGlobalStats( void );

    /**
     *
     */
    const TSocketStats& GetGlobalStats( void ) const;

    /**
     * @brief Collects network metrics across all networking interfaces aggregated
     * @param metrics the object that will hold all resulting metrics information
     * @return whether obtaining the metrics was successfull
     */
    bool GetGlobalNetworkMetrics( CNetworkInterfaceMetrics& metrics );


    /**
     *  Attempts to set the active state of a proxy server for the given protocol
     *  If no proxy server is defined for the given protocol the call will fail
     *
     *  @param protocol the protocol for which to set the active state
     *  @param active the new value for the active state for the given protocol proxy
     *  @return false is returned upon failure
     */
    bool SetSystemWideProxyServer( const CORE::CString& protocol ,
                                   const bool active             );

    /**
     *  Attempts to set the active state of a proxy server for the given protocol
     *  If no proxy server is defined for the given protocol the call will fail
     *
     *  @param protocol the protocol for which to set the active state
     *  @param remoteHost IP or hostname of the proxy server
     *  @param remotePort port of the proxy server where the proxy service is hosted
     *  @param active the active state for the given protocol proxy
     *  @return false is returned upon failure
     */
    bool SetSystemWideProxyServer( const CORE::CString& protocol   ,
                                   const CORE::CString& remoteHost ,
                                   const UInt16 remotePort         ,
                                   const bool active               );

    bool GetSystemWideProxyServer( const CORE::CString& protocol ,
                                   CORE::CString& remoteHost     ,
                                   UInt16& remotePort            ,
                                   bool& active                  ) const;

   /**
     *  Attempts to retrieve the active state of a proxy server for the given protocol
     *  If no proxy server is defined for the given protocol the call will return false
     *
     *  @return the active state for the given protocol proxy, false if no proxy is defined
     */
    bool IsSystemWideProxyServerActive( const CORE::CString& protocol ) const;

    /**
     *  Attempts to retrieve a list port id's for the available ports in the system of the given type
     */
    bool GetCommunicationPortList( const CORE::CString& portType ,
                                   TStringList& portList         ) const;
    
    /**
     *  Attempts to provide access to a communication port of the given type and the given id
     *  If the cannot be accessed NULL will be returned.
     */
    CICommunicationInterfacePtr GetCommunicationPort( const CORE::CString& portType ,
                                                      const CORE::CString& portId   );

    /**
     *  Attempts to retrieve a list of network interfaces on the system
     */
    bool GetNetworkInterfaceList( TStringList& interfaceIDs ) const;

    /**
     *  Attempts to provide access to a network interface with the given id
     *  If the cannot be accessed an empty shared ptr will be returned.
     */
    CINetworkInterfacePtr GetNetworkInterface( const CORE::CString& interfaceID );

    /**
     *  Attempts to provide access to all network interfaces
     */
    bool GetAllNetworkInterfaces( CINetworkInterface::TINetworkInterfacePtrVector& interfaces ) const;

    /**
     *  Attempts to obtain all network IP info from all network interfaces in aggregate
     */
    bool GetAllNetworkInterfaceIPInfo( CINetworkInterface::TIPv4InfoVector& ipInfo );

    private:
    friend class CComCoreGlobal;

    CCom( void );

    ~CCom();

    private:
    friend class CSocket;

    /**
     *      Registers a socket at the manager to receive update events.
     *      Should be used when creating a socket object by the socket object.
     *
     *      @param socket the socket object that you wish to register
     */
    void RegisterSocketObject( CSocket* socket );

    /**
     *      Unregisters a socket at the manager to receive update events
     *      Should be used when destroying a socket object by the socket object.
     *
     *      @param socket the socket object that you wish to unregister
     */
    void UnregisterSocketObject( CSocket* socket );

    private:

    CCom( const CCom& src );            /** not implemented */
    CCom& operator=( const CCom& src ); /** not implemented */

    void LazyInitNetworkInterfaces( void ) const;
    
    private:

    struct SProxyServer
    {
        CORE::CString host;
        UInt16 port;
        bool active;
    };
    typedef struct SProxyServer TProxyServer;
    typedef GUCEF::map< CORE::CString, TProxyServer > TProxyList;

    typedef GUCEF::set< CSocket* > TSocketSet;

    typedef GUCEF::map< CORE::CString, CICommunicationInterfacePtr > TPortMap;
    typedef GUCEF::map< CORE::CString, TPortMap > TPortIndex;
    
    TSocketSet m_sockets;    /** our socket object heap */
    MT::CMutex _mutex; 
    TSocketStats _stats;     /** global socket traffic stats */
    bool _keep_gstats;       /** whether or not to keep track of global stats */
    UInt32 _scount;          /** current number of registered sockets */
    TProxyList m_proxyList;
    TPortIndex m_portObjs;   /** index of all currently created port objects */
    mutable TINetworkInterfacePtrVector m_nics;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CCOM_H ? */
