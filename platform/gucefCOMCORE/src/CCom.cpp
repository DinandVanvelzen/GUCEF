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

#include <string.h>             /* needed for memset() */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_COMCORE_DVSOCKET_H
#include "dvwinsock.h"          /* socket API */
#define GUCEF_COMCORE_DVSOCKET_H
#endif /* GUCEF_COMCORE_DVSOCKET_H ? */

#ifndef GUCEF_COMCORE_CSOCKET_H
#include "CSocket.h"            /* base class for all sockets */
#define GUCEF_COMCORE_CSOCKET_H
#endif /* GUCEF_COMCORE_CSOCKET_H ? */

#include "CCom.h"               /* definition of CCom class */

#ifdef ACTIVATE_MEMORY_MANAGER
  #ifndef GUCEF_NEW_ON_H
  #include "gucef_new_on.h"   /* Use the GUCEF memory manager instead of the standard manager ? */
  #define GUCEF_NEW_ON_H
  #endif /* GUCEF_NEW_ON_H ? */
#endif /* ACTIVATE_MEMORY_MANAGER ? */

#if ( GUCEF_PLATFORM_MSWIN == GUCEF_PLATFORM )

  #ifndef GUCEF_COMCORE_CWIN32SERIALPORT_H
  #include "gucefCOMCORE_CWin32SerialPort.h"
  #define GUCEF_COMCORE_CWIN32SERIALPORT_H
  #endif /* GUCEF_COMCORE_CWIN32SERIALPORT_H ? */

  #ifndef GUCEF_COMCORE_CWIN32NETWORKINTERFACE_H
  #include "gucefCOMCORE_CWin32NetworkInterface.h"
  #define GUCEF_COMCORE_CWIN32NETWORKINTERFACE_H
  #endif /* GUCEF_COMCORE_CWIN32NETWORKINTERFACE_H ? */

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

  #ifndef GUCEF_COMCORE_CLINUXNETWORKINTERFACE_H
  #include "gucefCOMCORE_CLinuxNetworkInterface.h"
  #define GUCEF_COMCORE_CLINUXNETWORKINTERFACE_H
  #endif /* GUCEF_COMCORE_CLINUXNETWORKINTERFACE_H ? */

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

bool
GetMSWinInternetProxyFromRegistry( CORE::CString& remoteHost ,
                                   UInt16& port              ,
                                   bool& active              )
{GUCEF_TRACE;

    char lszValue[ 255 ];
    HKEY hKey;
    DWORD dwType = REG_SZ;
    DWORD dwSize = 255;

    if( ERROR_SUCCESS == RegOpenKeyEx( HKEY_CURRENT_USER                                                 ,
                                       "SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\INTERNET SETTINGS" ,
                                       0L                                                                ,
                                       KEY_ALL_ACCESS                                                    ,
                                       &hKey                                                             ) )
    {
        if ( ERROR_SUCCESS == RegQueryValueEx( hKey              ,
                                               "ProxyServer"     ,
                                               NULL              ,
                                               &dwType           ,
                                               (LPBYTE)&lszValue ,
                                               &dwSize           ) )
        {
            CORE::CString regValue( lszValue );
            remoteHost = regValue.SubstrToChar( ':', true );
            port = 80;

            CORE::CString portStr( regValue.SubstrToChar( ':', false ) );
            if ( portStr.Length() > 0 )
            {
                port = static_cast< UInt16 >( CORE::StringToInt32( portStr ) );
            }

            active = false;
            if ( ERROR_SUCCESS == RegQueryValueEx( hKey              ,
                                                   "ProxyEnable"     ,
                                                   NULL              ,
                                                   &dwType           ,
                                                   (LPBYTE)&lszValue ,
                                                   &dwSize           ) )
            {
                active = CORE::StringToBool( lszValue );
            }
            return true;
        }
        RegCloseKey( hKey );
    }
    return false;
}

/*-------------------------------------------------------------------------*/
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

typedef std::map< CString, UInt64 >     TStringToUInt64Map;

// Note: On devices that run Android 10 or higher, apps cannot access /proc/net without root
bool
GetProcNetNetstatInfo( TStringToUInt64Map& tcpStats ,
                       TStringToUInt64Map& ipStats  ,
                       TStringToUInt64Map& mtpStats )
{GUCEF_TRACE;

    CORE::CString fileContent;
    if ( CORE::LoadTextFileAsString( "/proc/net/netstat", fileContent, true, "\n" ) )
    {
        CORE::CString::StringVector lines = fileContent.ParseElements( '\n', false );

        CORE::CString::StringVector keys;
        CORE::CString::StringVector values;
        bool isKeyLine = true;

        CORE::CString::StringVector::iterator i = lines.begin();
        while ( i != lines.end() )
        {
            const CORE::CString& line = (*i);
            CORE::CString typePrefix = line.SubstrToChar( ':', true );

            if ( isKeyLine )
            {
                keys = line.SubstrToIndex( typePrefix.Length()+1, false ).ParseElements( ' ', false );
            }
            else
            {
                values = line.SubstrToIndex( typePrefix.Length()+1, false ).ParseElements( ' ', false );

                TStringToUInt64Map* mapToUse = GUCEF_NULL;
                if ( "TcpExt" == typePrefix )
                    mapToUse = &tcpStats;
                else
                if ( "IpExt" == typePrefix )
                    mapToUse = &ipStats;
                else
                if ( "MPTcpExt" == typePrefix )
                    mapToUse = &mtpStats;

                size_t elementCount = GUCEF_SMALLEST( keys.size(), values.size() );
                if ( GUCEF_NULL != mapToUse )
                {
                    for ( size_t n=0; n<elementCount; ++n )
                    {
                        (*mapToUse)[ keys[ n ] ] = StringToUInt64( values[ n ], 0 );
                    }
                }

                keys.clear();
                values.clear();
            }

            ++i;
            isKeyLine = !isKeyLine;
        }

        return !tcpStats.empty() || !ipStats.empty() || !mtpStats.empty();
    }
    else
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "GetProcNetNetstatInfo: Cannot read /proc/net/netstat" );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
GetProcNetNetstatInfo( CNetworkInterfaceMetrics& metrics )
{GUCEF_TRACE;

    TStringToUInt64Map tcpStats;
    TStringToUInt64Map ipStats;
    TStringToUInt64Map mtpStats;

    if ( GetProcNetNetstatInfo( tcpStats ,
                                ipStats  ,
                                mtpStats ) )
    {
        // Lookup all our known ipStats
        TStringToUInt64Map::iterator i = ipStats.find( "InErrs" );
        if ( i != ipStats.end() )
        {
            metrics.inboundErroredPackets = (*i).second;
            metrics.hasInboundErroredPackets = true;
        }
        i = ipStats.find( "OutErrs" );
        if ( i != ipStats.end() )
        {
            metrics.outboundErroredPackets = (*i).second;
            metrics.hasOutboundErroredPackets = true;
        }
        i = ipStats.find( "InDiscards" );
        if ( i != ipStats.end() )
        {
            metrics.inboundDiscardedPackets = (*i).second;
            metrics.hasInboundDiscardedPackets = true;
        }
        i = ipStats.find( "OutDiscards" );
        if ( i != ipStats.end() )
        {
            metrics.outboundDiscardedPackets = (*i).second;
            metrics.hasOutboundDiscardedPackets = true;
        }
        i = ipStats.find( "InOctets" );
        if ( i != ipStats.end() )
        {
            metrics.inboundOctets = (*i).second;
            metrics.hasInboundOctets = true;
        }
        i = ipStats.find( "OutOctets" );
        if ( i != ipStats.end() )
        {
            metrics.outboundOctets = (*i).second;
            metrics.hasOutboundOctets = true;
        }
        i = ipStats.find( "InMcastOctets" );
        if ( i != ipStats.end() )
        {
            metrics.inboundMulticastOctets = (*i).second;
            metrics.hasInboundMulticastOctets = true;
        }
        i = ipStats.find( "OutMcastOctets" );
        if ( i != ipStats.end() )
        {
            metrics.outboundMulticastOctets = (*i).second;
            metrics.hasOutboundMulticastOctets = true;
        }
        i = ipStats.find( "InBcastOctets" );
        if ( i != ipStats.end() )
        {
            metrics.inboundBroadcastOctets = (*i).second;
            metrics.hasInboundBroadcastOctets = true;
        }
        i = ipStats.find( "OutBcastOctets" );
        if ( i != ipStats.end() )
        {
            metrics.outboundBroadcastOctets = (*i).second;
            metrics.hasOutboundBroadcastOctets = true;
        }
        i = ipStats.find( "InUcastOctets" );
        if ( i != ipStats.end() )
        {
            metrics.inboundUnicastOctets = (*i).second;
            metrics.hasInboundUnicastOctets = true;
        }
        i = ipStats.find( "OutUcastOctets" );
        if ( i != ipStats.end() )
        {
            metrics.inboundUnicastOctets = (*i).second;
            metrics.hasInboundUnicastOctets = true;
        }
        i = ipStats.find( "InUnknownProtos" );
        if ( i != ipStats.end() )
        {
            metrics.inboundUnknownProtocolPackets = (*i).second;
            metrics.hasInboundUnknownProtocolPackets = true;
        }
        i = ipStats.find( "OutUnknownProtos" );
        if ( i != ipStats.end() )
        {
            metrics.outboundUnknownProtocolPackets = (*i).second;
            metrics.hasOutboundUnknownProtocolPackets = true;
        }
        i = ipStats.find( "InUcastPkts" );
        if ( i != ipStats.end() )
        {
            metrics.inboundUnicastPackets = (*i).second;
            metrics.hasInboundUnicastPackets = true;
        }
        i = ipStats.find( "OutUcastPkts" );
        if ( i != ipStats.end() )
        {
            metrics.outboundUnicastPackets = (*i).second;
            metrics.hasOutboundUnicastPackets = true;
        }

        UInt64 InMcastPkts = 0;
        bool hasInMcastPkts = false;
        i = ipStats.find( "InMcastPkts" );
        if ( i != ipStats.end() )
        {
            InMcastPkts = (*i).second;
            hasInMcastPkts = true;
        }

        UInt64 InBcastPkts = 0;
        bool hasInBcastPkts = false;
        i = ipStats.find( "InBcastPkts" );
        if ( i != ipStats.end() )
        {
            InBcastPkts = (*i).second;
            hasInBcastPkts = true;
        }

        if ( hasInMcastPkts && hasInBcastPkts )
        {
            metrics.inboundNonUnicastPackets = InMcastPkts + InBcastPkts;
            metrics.hasInboundNonUnicastPackets = true;
        }

        UInt64 OutMcastPkts = 0;
        bool hasOutMcastPkts = false;
        i = ipStats.find( "OutMcastPkts" );
        if ( i != ipStats.end() )
        {
            OutMcastPkts = (*i).second;
            hasOutMcastPkts = true;
        }

        UInt64 OutBcastPkts = 0;
        bool hasOutBcastPkts = false;
        i = ipStats.find( "OutBcastPkts" );
        if ( i != ipStats.end() )
        {
            OutBcastPkts = (*i).second;
            hasOutBcastPkts = true;
        }

        if ( hasOutMcastPkts && hasOutBcastPkts )
        {
            metrics.outboundNonUnicastPackets = OutMcastPkts + OutBcastPkts;
            metrics.hasOutboundNonUnicastPackets = true;
        }


        // Lookup all our known tcpStats
        i = ipStats.find( "RcvRTT" );
        if ( i != ipStats.end() )
        {
            metrics.receiveLinkSpeedBitsPerSec = (*i).second;
            metrics.hasReceiveLinkSpeedBitsPerSec = true;
        }
        i = ipStats.find( "SndRTT" );
        if ( i != ipStats.end() )
        {
            metrics.transmitLinkSpeedBitsPerSec = (*i).second;
            metrics.hasTransmitLinkSpeedBitsPerSec = true;
        }

        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM_MSWIN ? */

/*-------------------------------------------------------------------------*/

CCom::CCom()
    : _keep_gstats( false ) ,
      _scount( 0 )          ,
      m_sockets()           ,
      m_proxyList()
{GUCEF_TRACE;

    memset( &_stats, 0, sizeof(TSocketStats) );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

// @TODO use InternetQueryOption
// http://msdn2.microsoft.com/en-us/library/aa385101.aspx

    /*
     *  Read the O/S proxy server settings
     */

    CORE::CString remoteHost;
    UInt16 port;
    bool active;

    if ( GetMSWinInternetProxyFromRegistry( remoteHost ,
                                            port       ,
                                            active     ) )
    {
        SetSystemWideProxyServer( "http"     ,
                                  remoteHost ,
                                  port       ,
                                  active     );
    }

    #endif /* GUCEF_PLATFORM_MSWIN ? */
}

/*-------------------------------------------------------------------------*/

CCom::~CCom()
{GUCEF_TRACE;

    MT::CScopeMutex lock( _mutex );
    m_sockets.clear();
    memset( &_stats, 0, sizeof(_stats) );
    _scount = 0;
    m_proxyList.clear();
    m_portObjs.clear();
    m_nics.clear();
}

/*-------------------------------------------------------------------------*/

void
CCom::RegisterSocketObject( CSocket* socket )
{GUCEF_TRACE;

    MT::CScopeMutex lock( _mutex );
    m_sockets.insert( socket );
    ++_scount;
}

/*-------------------------------------------------------------------------*/

void
CCom::UnregisterSocketObject( CSocket* socket )
{GUCEF_TRACE;

    MT::CScopeMutex lock( _mutex );
    m_sockets.erase( socket );
    --_scount;
}

/*-------------------------------------------------------------------------*/

UInt32
CCom::GetSocketCount( void ) const
{GUCEF_TRACE;

    return _scount;
}

/*-------------------------------------------------------------------------*/

void
CCom::SetUseGlobalStats( bool keep_gstats )
{GUCEF_TRACE;

    _keep_gstats = keep_gstats;
}

/*-------------------------------------------------------------------------*/

bool
CCom::GetUseGlobalStats( void ) const
{GUCEF_TRACE;

    return _keep_gstats;
}

/*-------------------------------------------------------------------------*/

void
CCom::ResetGlobalStats( void )
{GUCEF_TRACE;

    _stats.bytes_sent = 0;
    _stats.bytes_recieved = 0;
    _stats.bps_sent = 0;
    _stats.bps_recieved = 0;
}

/*-------------------------------------------------------------------------*/

const CCom::TSocketStats&
CCom::GetGlobalStats( void ) const
{GUCEF_TRACE;

    return _stats;
}

/*-------------------------------------------------------------------------*/

bool
CCom::GetCommunicationPortList( const CORE::CString& portType ,
                                TStringList& portList         ) const
{GUCEF_TRACE;

    CORE::CString typeOfPort = portType.Lowercase();
    if ( "serial" == typeOfPort ||
         "com" == typeOfPort     )
    {
        #if ( GUCEF_PLATFORM_MSWIN == GUCEF_PLATFORM )
        CWin32SerialPort::PopulatePortList( portList );
        return true;
        #endif
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CICommunicationInterfacePtr
CCom::GetCommunicationPort( const CORE::CString& portType ,
                            const CORE::CString& portId   )
{GUCEF_TRACE;

    CORE::CString typeOfPort = portType.Lowercase();

    MT::CScopeMutex lock( _mutex );

    // Check to see if we already have an entry for the given port
    TPortIndex::iterator i = m_portObjs.find( typeOfPort );
    if ( i != m_portObjs.end() )
    {
        TPortMap& portMap = (*i).second;
        TPortMap::iterator n = portMap.find( portId );
        if ( n != portMap.end() )
        {
            CICommunicationInterfacePtr port = (*n).second;
            return port;
        }
    }

    // No such port object was found, check to see if we can create it
    if ( "serial" == typeOfPort ||
         "com" == typeOfPort     )
    {
        #if ( GUCEF_PLATFORM_MSWIN == GUCEF_PLATFORM )

        CICommunicationInterfacePtr port = CWin32SerialPort::Create( portId ).StaticCast< CICommunicationInterface >();
        if ( !port.IsNULL() )
        {
            TPortMap& portMap = m_portObjs[ portType ];
            portMap[ portId ] = port;
            return port;
        }

        #endif
    }

    return CICommunicationInterfacePtr();
}

/*-------------------------------------------------------------------------*/

void
CCom::LazyInitNetworkInterfaces( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( _mutex );
    if ( m_nics.empty() )
    {
        #if ( GUCEF_PLATFORM_MSWIN == GUCEF_PLATFORM )

        CWin32NetworkInterface::EnumNetworkAdapters( m_nics );

        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

        CLinuxNetworkInterface::EnumNetworkAdapters( m_nics );

        #else

        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Com:LazyInitNetworkInterfaces: Platform has no supported implementation" );

        #endif
    }
}

/*-------------------------------------------------------------------------*/

bool
CCom::GetNetworkInterfaceList( TStringList& interfaceIDs ) const
{GUCEF_TRACE;

    LazyInitNetworkInterfaces();

    MT::CScopeMutex lock( _mutex );
    TINetworkInterfacePtrVector::const_iterator i = m_nics.begin();
    while ( i != m_nics.end() )
    {
        interfaceIDs.push_back( (*i)->GetAdapterName() );
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CINetworkInterfacePtr
CCom::GetNetworkInterface( const CORE::CString& interfaceID )
{GUCEF_TRACE;

    LazyInitNetworkInterfaces();

    MT::CScopeMutex lock( _mutex );
    TINetworkInterfacePtrVector::const_iterator i = m_nics.begin();
    while ( i != m_nics.end() )
    {
        if ( interfaceID == (*i)->GetAdapterName() )
        {
            return (*i);
        }
        ++i;
    }
    return CINetworkInterfacePtr();
}

/*-------------------------------------------------------------------------*/

bool
CCom::GetAllNetworkInterfaces( CINetworkInterface::TINetworkInterfacePtrVector& interfaces ) const
{GUCEF_TRACE;

    LazyInitNetworkInterfaces();

    MT::CScopeMutex lock( _mutex );
    interfaces = m_nics;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCom::GetAllNetworkInterfaceIPInfo( CINetworkInterface::TIPv4InfoVector& ipInfo )
{GUCEF_TRACE;

    LazyInitNetworkInterfaces();

    MT::CScopeMutex lock( _mutex );
    bool totalSuccess = true;
    TINetworkInterfacePtrVector::const_iterator i = m_nics.begin();
    while ( i != m_nics.end() )
    {
        totalSuccess = (*i)->GetIPInfo( ipInfo, false ) && totalSuccess;
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CCom::GetGlobalNetworkMetrics( CNetworkInterfaceMetrics& metrics )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM_MSWIN == GUCEF_PLATFORM )

    // @TODO
    return false;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    // On Linux these stats are provided pre-aggregated by the system
    return GetProcNetNetstatInfo( metrics );

    #else

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Com:GetGlobalNetworkMetrics: Platform has no supported implementation" );
    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
CCom::SetSystemWideProxyServer( const CORE::CString& protocol ,
                                const bool active             )
{GUCEF_TRACE;

    MT::CScopeMutex lock( _mutex );
    TProxyList::iterator i = m_proxyList.find( protocol );
    if ( i != m_proxyList.end() )
    {
        (*i).second.active = active;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CCom::SetSystemWideProxyServer( const CORE::CString& protocol   ,
                                const CORE::CString& remoteHost ,
                                const UInt16 remotePort         ,
                                const bool active               )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Setting system wide proxy server for protocol " + protocol +
                          " to " + remoteHost + ":" + CORE::UInt16ToString( remotePort ) + " and active state " + CORE::BoolToString( active ) );

    MT::CScopeMutex lock( _mutex );
    TProxyServer& proxyServer = m_proxyList[ protocol ];
    proxyServer.host = remoteHost;
    proxyServer.port = remotePort;
    proxyServer.active = active;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCom::GetSystemWideProxyServer( const CORE::CString& protocol ,
                                CORE::CString& remoteHost     ,
                                UInt16& remotePort            ,
                                bool& active                  ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( _mutex );
    TProxyList::const_iterator i = m_proxyList.find( protocol );
    if ( i != m_proxyList.end() )
    {
        const TProxyServer& proxyServer = (*i).second;
        remoteHost = proxyServer.host;
        remotePort = proxyServer.port;
        active = proxyServer.active;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CCom::IsSystemWideProxyServerActive( const CORE::CString& protocol ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( _mutex );
    TProxyList::const_iterator i = m_proxyList.find( protocol );
    if ( i != m_proxyList.end() )
    {
        return (*i).second.active;
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
