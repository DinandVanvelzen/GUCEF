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

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

#include <fstream>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/rtnetlink.h>

#include "gucefCOMCORE_CLinuxNetworkInterface.h"

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_CORE_CDATANODEDOCUMENTBUILDER_H
#include "gucefCORE_CDataNodeDocumentBuilder.h"
#define GUCEF_CORE_CDATANODEDOCUMENTBUILDER_H
#endif /* GUCEF_CORE_CDATANODEDOCUMENTBUILDER_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CINIPARSER_H
#include "gucefCORE_CIniParser.h"
#define GUCEF_CORE_CINIPARSER_H
#endif /* GUCEF_CORE_CINIPARSER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMCORE_CCOM_H
#include "CCom.h"
#define GUCEF_COMCORE_CCOM_H
#endif /* GUCEF_COMCORE_CCOM_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CDStoreCodecRegistry::TDStoreCodecPtr
GetYamlCodec( void )
{GUCEF_TRACE;

    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    CORE::CDStoreCodecRegistry& dstoreCodecRegistry = coreGlobal->GetDStoreCodecRegistry();
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr yamlCodec;
    if ( dstoreCodecRegistry.TryGetYamlCodec( yamlCodec ) )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Linux networking: located YAML codec" );
        return yamlCodec;
    }

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Linux networking: Failed to locate a YAML codec" );
    return yamlCodec;
}

/*-------------------------------------------------------------------------*/

CString
GetDefaultLinuxNetworkGateway( void )
{GUCEF_TRACE;

    int sock = ::socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if ( sock < 0 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GetDefaultLinuxNetworkGateway: socket failed" );
        return CString::Empty;
    }

    struct {
        struct nlmsghdr nlmsg;
        struct rtmsg rtmsg;
        char buf[8192];
    } request;

    memset(&request, 0, sizeof(request));
    request.nlmsg.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    request.nlmsg.nlmsg_type = RTM_GETROUTE;
    request.nlmsg.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    request.nlmsg.nlmsg_seq = 1;
    request.nlmsg.nlmsg_pid = getpid();
    request.rtmsg.rtm_family = AF_INET;

    if ( ::send( sock, &request, request.nlmsg.nlmsg_len, 0) < 0 )
    {
        close( sock );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GetDefaultLinuxNetworkGateway: socket send failed" );
        return CString::Empty;
    }

    char buffer[8192];
    int len = ::recv( sock, buffer, sizeof(buffer), 0 );
    if ( len < 0 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GetDefaultLinuxNetworkGateway: socket recv failed" );
        ::close( sock );
        return CString::Empty;
    }

    struct nlmsghdr *nlh = (struct nlmsghdr *)buffer;
    for (; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len))
    {
        if (nlh->nlmsg_type == NLMSG_DONE)
        {
            break;
        }
        if (nlh->nlmsg_type == NLMSG_ERROR)
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GetDefaultLinuxNetworkGateway: Error in netlink message" );
            ::close( sock );
            return CString::Empty;
        }

        struct rtmsg* rtm = (struct rtmsg *)NLMSG_DATA(nlh);
        if (rtm->rtm_table != RT_TABLE_MAIN)
        {
            continue;
        }

        struct rtattr* rta = (struct rtattr *)RTM_RTA(rtm);
        int rta_len = RTM_PAYLOAD(nlh);
        for (; RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len))
        {
            if (rta->rta_type == RTA_GATEWAY)
            {
                char gateway[INET_ADDRSTRLEN];
                ::inet_ntop(AF_INET, RTA_DATA(rta), gateway, sizeof(gateway));
                ::close(sock);
                return CString(gateway);
            }
        }
    }

    ::close(sock);
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "GetDefaultLinuxNetworkGateway: No default gateway found" );
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
GetDhcpLeaseTimestampFromSystemdJournal( const CORE::CString& interfaceName )
{GUCEF_TRACE;

    char buffer[128];
    std::string result;
    std::string command = "journalctl -u systemd-networkd -u NetworkManager --no-pager | grep 'DHCPv4 lease acquired' | grep '" + interfaceName.STL_String() + "'";
    FILE* pipe = ::popen( command.c_str(), "r" );
    if ( GUCEF_NULL == pipe )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "GetDhcpLeaseTimestampFromSystemdJournal: popen() failed" );
        return CString::Empty;
    }

    try
    {
        while ( fgets(buffer, sizeof(buffer), pipe) != GUCEF_NULL )
        {
            result += buffer;
        }
    }
    catch (...)
    {
        ::pclose( pipe );
        return CString::Empty;
    }
    ::pclose( pipe );

    if ( result.empty() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "GetDhcpLeaseTimestampFromSystemdJournal: No DHCP lease timestamp found" );
        return CString::Empty;
    }

    // Extract the timestamp from the log entry
    std::string::size_type pos = result.find(" ");
    if ( pos != std::string::npos )
    {
        return result.substr(0, pos);
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "GetDhcpLeaseTimestampFromSystemdJournal: No DHCP lease timestamp found" );
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

struct EtcNetworkInterface
{
    CString iface;
    CString address;
    CString netmask;
    CString gateway;
};
typedef std::vector< EtcNetworkInterface, gucef_allocator< EtcNetworkInterface > >  TEtcNetworkInterfaceVector;
typedef std::map< CString, struct EtcNetworkInterface >                             TStringToEtcNetworkInterface;

/*-------------------------------------------------------------------------*/

//  Debian-based systems:
//     The traditional location is /etc/network/interfaces.
bool
ParseEtcNetworkInterfacesFile( const CString& filePath                ,
                               TEtcNetworkInterfaceVector& interfaces )
{GUCEF_TRACE;

    std::ifstream file( filePath.C_String() );
    if ( !file.is_open() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ParseEtcNetworkInterfacesFile: Unable to open file " + filePath );
        return false;
    }

    std::string line;
    EtcNetworkInterface currentInterface;
    bool inInterfaceBlock = false;

    while (std::getline(file, line))
    {
        if (line.find("iface") == 0)
        {
            if (inInterfaceBlock)
            {
                interfaces.push_back(currentInterface);
                currentInterface = EtcNetworkInterface();
            }
            inInterfaceBlock = true;
            currentInterface.iface = line;
        }
        else
        if (line.find("address") == 0)
        {
            currentInterface.address = line;
        }
        else
        if (line.find("netmask") == 0)
        {
            currentInterface.netmask = line;
        }
        else
        if (line.find("gateway") == 0)
        {
            currentInterface.gateway = line;
        }
    }

    if (inInterfaceBlock)
    {
        interfaces.push_back(currentInterface);
    }

    file.close();
    return !interfaces.empty();
}

/*-------------------------------------------------------------------------*/

bool
ParseEtcNetworkInterfacesFile( const CString& filePath                  ,
                               TStringToEtcNetworkInterface& interfaces )
{GUCEF_TRACE;

    TEtcNetworkInterfaceVector interfacesVec;
    if ( ParseEtcNetworkInterfacesFile( filePath, interfacesVec ) )
    {
        TEtcNetworkInterfaceVector::iterator i = interfacesVec.begin();
        while ( i != interfacesVec.end() )
        {
            const EtcNetworkInterface& nic = (*i);
            interfaces[ nic.iface ] = nic;
            ++i;
        }
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

class NetplanConfig
{
    public:

    bool usesDhcp4;
    bool usesDhcp6;
    CString macaddressStr;
    CString interfaceName;
    CString setInterfaceName;
    CIPv4Address::TIPv4AddressSet ipv4DnsServers;
    CIPv4Info::TIPv4InfoSet ipv4Infos;

    NetplanConfig( void )
        : usesDhcp4( false )
        , usesDhcp6( false )
        , macaddressStr()
        , interfaceName()
        , setInterfaceName()
        , ipv4DnsServers()
        , ipv4Infos()
    {GUCEF_TRACE;
    }

};
typedef std::vector< NetplanConfig, gucef_allocator< NetplanConfig > >  TNetplanConfigVector;
typedef std::map< CString, NetplanConfig >                              TStringToNetplanConfig;

/*-------------------------------------------------------------------------*/

bool
ParseNetplanFile( const CString& filePath                               ,
                  TNetplanConfigVector& configs                         ,
                  CORE::CDStoreCodecRegistry::TDStoreCodecPtr yamlCodec )
{GUCEF_TRACE;

    bool addedConfigs = false;

    if ( !yamlCodec.IsNULL() )
    {
        // using the codec parse the resource into a usable document
        CORE::CDataNodeDocumentBuilder configDoc;
        if ( yamlCodec->BuildDataTree( &configDoc.document, filePath ) )
        {
            CORE::CDataNode::TDataNodeSet entries = configDoc.document.FindNodesOfType( "ethernets", true );
            CORE::CDataNode::TDataNodeSet::iterator i = entries.begin();
            while ( i != entries.end() )
            {
                CORE::CDataNode* ethernetsEntry = (*i);
                if ( GUCEF_NULL != ethernetsEntry )
                {
                    CORE::CDataNode* interfaceNode = ethernetsEntry->GetChildAtIndex( 0 );
                    if ( GUCEF_NULL != interfaceNode )
                    {
                        NetplanConfig netplanConfig;
                        netplanConfig.interfaceName = interfaceNode->GetName();

                        netplanConfig.usesDhcp4 = interfaceNode->GetAttributeValueOrChildValueByName( "dhcp4" ).AsBool( netplanConfig.usesDhcp4 );
                        netplanConfig.usesDhcp6 = interfaceNode->GetAttributeValueOrChildValueByName( "dhcp6" ).AsBool( netplanConfig.usesDhcp6 );


                        CORE::CDataNode* matchNode = interfaceNode->FindChild( "match" );
                        if ( GUCEF_NULL != matchNode )
                        {
                            netplanConfig.macaddressStr = matchNode->GetAttributeValueOrChildValueByName( "macaddress" ).AsString();
                        }

                        CORE::CDataNode* nameserversNode = interfaceNode->FindChild( "nameservers" );
                        if ( GUCEF_NULL != nameserversNode )
                        {
                            CORE::CDataNode* addressesNode = nameserversNode->FindChild( "addresses" );
                            if ( GUCEF_NULL != addressesNode && GUCEF_DATATYPE_ARRAY == addressesNode->GetNodeType() )
                            {
                                CORE::CDataNode::TVariantVector addressesVars = addressesNode->GetChildrenValues();
                                CORE::CDataNode::TVariantVector::iterator v = addressesVars.begin();
                                while ( v != addressesVars.end() )
                                {
                                    const CORE::CVariant& var = (*v);
                                    CIPv4Address dnsIp;

                                    if ( dnsIp.SetAddress( var.AsString() ) )
                                    {
                                        netplanConfig.ipv4DnsServers.insert( dnsIp );
                                    }
                                    ++v;
                                }
                            }                            
                        }

                        CORE::CDataNode* addressesNode = interfaceNode->FindChild( "addresses" );
                        if ( GUCEF_NULL != addressesNode && GUCEF_DATATYPE_ARRAY == addressesNode->GetNodeType() )
                        {
                            // IP addressess are in CIDR notation
                            CORE::CDataNode::TVariantVector addressesVars = addressesNode->GetChildrenValues();
                            CORE::CDataNode::TVariantVector::iterator v = addressesVars.begin();
                            while ( v != addressesVars.end() )
                            {
                                const CORE::CVariant& var = (*v);
                                CIPv4Info ipInfoForInterface;

                                if ( ipInfoForInterface.TrySetFromCIDRNotationString( var.AsString() ) )
                                {
                                    netplanConfig.ipv4Infos.insert( ipInfoForInterface );
                                }
                                ++v;
                            }
                        }

                        netplanConfig.setInterfaceName = interfaceNode->GetAttributeValueOrChildValueByName( "set-name" ).AsString();

                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ParseNetplanFile: Found config for interface " + netplanConfig.interfaceName );                        
                        configs.push_back( netplanConfig );
                        addedConfigs = true;
                    }
                }
                ++i;
            }
        }
    }

    return addedConfigs;
}

/*-------------------------------------------------------------------------*/

bool
ParseNetplanFile( const CString& filePath                               ,
                  TStringToNetplanConfig& interfaces                    ,
                  CORE::CDStoreCodecRegistry::TDStoreCodecPtr yamlCodec )
{GUCEF_TRACE;

    TNetplanConfigVector interfacesVec;
    if ( ParseNetplanFile( filePath, interfacesVec, yamlCodec ) )
    {
        TNetplanConfigVector::iterator i = interfacesVec.begin();
        while ( i != interfacesVec.end() )
        {
            const NetplanConfig& nic = (*i);

            CString interfaceKeyName = nic.setInterfaceName.IsNULLOrEmpty() ? nic.interfaceName : nic.setInterfaceName;
            interfaces[ interfaceKeyName ] = nic;
            ++i;
        }
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
ParseAllNetplanFiles( const CString& rootPath,
                      TStringToNetplanConfig& configs ,
                      CORE::CDStoreCodecRegistry::TDStoreCodecPtr yamlCodec )
{GUCEF_TRACE;

    CORE::CFileSystemIterator fsIterator;

    if ( fsIterator.FindFirst( rootPath ) )
    {
        bool someSuccess = false;
        do
        {
            CString filePath = fsIterator.GetResourcePath();

            // Netplan files are YAML formatted files with yaml extension
            CString fileExt = CORE::ExtractFileExtention( filePath );
            if ( fileExt.Lowercase() == "yaml" )
            {
                if ( ParseNetplanFile( filePath, configs, yamlCodec ) )
                    someSuccess = true;
            }
        }
        while ( fsIterator.FindNext() );
        return someSuccess;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

class NetworkManagerConfig
{
    public:

    bool usesDhcp4;
    bool usesDhcp6;
    CString macaddressStr;
    CString interfaceName;
    CString setInterfaceName;
    CIPv4Address::TIPv4AddressSet ipv4DnsServers;
    CIPv4Info::TIPv4InfoSet ipv4Infos;

    NetworkManagerConfig( void )
        : usesDhcp4( false )
        , usesDhcp6( false )
        , macaddressStr()
        , interfaceName()
        , setInterfaceName()
        , ipv4DnsServers()
        , ipv4Infos()
    {GUCEF_TRACE;
    }

};
typedef std::vector< NetworkManagerConfig, gucef_allocator< NetworkManagerConfig > >  TNetworkManagerConfigVector;
typedef std::map< CString, NetworkManagerConfig >                                     TStringToNetworkManagerConfig;

/*-------------------------------------------------------------------------*/

bool
ParseNetworkManagerFile( const CString& filePath                   ,
                         TStringToNetworkManagerConfig& interfaces )
{GUCEF_TRACE;

    CORE::CIniParser iniParser;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
ParseAllNetworkManagerFiles( const CString& rootPath                ,
                             TStringToNetworkManagerConfig& configs )
{GUCEF_TRACE;
    
    CORE::CFileSystemIterator fsIterator;

    if ( fsIterator.FindFirst( rootPath ) )
    {
        bool someSuccess = false;
        do
        {
            CString filePath = fsIterator.GetResourcePath();

            // NetworkManager files are ini style files with conf extension
            CString fileExt = CORE::ExtractFileExtention( filePath );
            if ( fileExt.Lowercase() == "conf" )
            {
                if ( ParseNetworkManagerFile( filePath, configs ) )
                    someSuccess = true;
            }
        }
        while ( fsIterator.FindNext() );
        return someSuccess;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

class CLinuxNetworkInterface::NetworkConfigDiscoveryCache
{
    public:

    bool performedDiscovery;
    CIPv4Address defaultIPv4Gateway;
    bool hasDefaultIPv4Gateway;
    TStringToEtcNetworkInterface etcInterfaces;
    TStringToNetplanConfig netplanInterfaces;
    TStringToNetworkManagerConfig networkManagerInterfaces;
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr yamlCodec;

    NetworkConfigDiscoveryCache( void )
        : performedDiscovery( false )
        , defaultIPv4Gateway()
        , hasDefaultIPv4Gateway( false )
        , etcInterfaces()
        , netplanInterfaces()
        , yamlCodec()
    {GUCEF_TRACE;

    }
};

/*-------------------------------------------------------------------------*/

void
CLinuxNetworkInterface::FillExtraInfoFromConfig( NetworkConfigDiscoveryCache& cache )
{GUCEF_TRACE;

    bool discoverySuccess = false;

    if ( !cache.performedDiscovery )
    {
        // Linux typically only has 1 global network gateway
        CString defaultIPv4GatewayStr = GetDefaultLinuxNetworkGateway();
        if ( !defaultIPv4GatewayStr.IsNULLOrEmpty() )
                cache.hasDefaultIPv4Gateway = cache.defaultIPv4Gateway.SetAddress( defaultIPv4GatewayStr );

        // Grab a YAML codec since some config files use the yaml format
        cache.yamlCodec = GetYamlCodec();
    }

    if ( cache.hasDefaultIPv4Gateway )
        m_defGateway = cache.defaultIPv4Gateway;

    //  Debian-based systems:
    //     The traditional location is /etc/network/interfaces.
    TStringToEtcNetworkInterface etcInterfaces;
    if ( cache.performedDiscovery || ParseEtcNetworkInterfacesFile( "/etc/network/interfaces", cache.etcInterfaces ) )
    {
        TStringToEtcNetworkInterface::iterator m = cache.etcInterfaces.find( m_name );
        if ( m != etcInterfaces.end() )
        {
            discoverySuccess = true;
            const EtcNetworkInterface& etcNic = (*m).second;

            CIPv4Address nicGateway;
            if ( nicGateway.SetAddress( etcNic.gateway ) )
            {
                m_gatewayList.push_back( nicGateway );
            }
        }
    }

    // Netplan (used in Ubuntu 18.04 and later):
    //     Configuration files are located in /etc/netplan/.
    if ( cache.performedDiscovery || ParseAllNetplanFiles( "/etc/netplan/", cache.netplanInterfaces, cache.yamlCodec ) )
    {
        TStringToNetplanConfig::iterator m = cache.netplanInterfaces.find( m_name );
        if ( m != cache.netplanInterfaces.end() )
        {
            discoverySuccess = true;
            const NetplanConfig& netplanConfig = (*m).second;

            m_dhcpUsedforIpv4 = netplanConfig.usesDhcp4;
            m_dhcpUsedforIpv6 = netplanConfig.usesDhcp6;

            if ( !netplanConfig.macaddressStr.IsNULLOrEmpty() )
                m_macAddrs.insert( netplanConfig.macaddressStr );

            m_ipv4Addresses = netplanConfig.ipv4Infos;
        }
    }

    // NetworkManager:
    //     Configuration files are typically located in /etc/NetworkManager/system-connections/
    if ( cache.performedDiscovery || ParseAllNetworkManagerFiles( "/etc/NetworkManager/system-connections/", cache.networkManagerInterfaces ) )
    {
        // @TODO
    }

    CString dhcpLeaseStr = GetDhcpLeaseTimestampFromSystemdJournal( m_name );
    if ( !dhcpLeaseStr.IsNULLOrEmpty() )
    {
        int fg=0; // @TODO
    }

    // We need at least one success story before we stop re-doing discovery
    cache.performedDiscovery = discoverySuccess;
}

/*-------------------------------------------------------------------------*/

bool
CLinuxNetworkInterface::EnumNetworkAdapters( TINetworkInterfacePtrVector& interfaces )
{GUCEF_TRACE;   

    struct ifaddrs* ifaddr = GUCEF_NULL;
    struct ifaddrs* ifa = GUCEF_NULL;    

    // Get the list of network interfaces
    if ( ::getifaddrs( &ifaddr ) == -1)
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LinuxNetworkInterface:EnumNetworkAdapters: getifaddrs failed" );
        return false;
    }

    // keep a cache of discovered network config info so we dont have to redo all
    // the work per interface
    struct NetworkConfigDiscoveryCache networkConfigDiscoveryCache;

    // Iterate through the list of interfaces
    typedef std::map< CString, CLinuxNetworkInterfacePtr > TNicMap;
    TNicMap nicMap;
    for ( ifa = ifaddr; ifa != GUCEF_NULL; ifa = ifa->ifa_next )
    {
        if ( ifa->ifa_addr == GUCEF_NULL)
        {
            continue;
        }

        int family = ifa->ifa_addr->sa_family;

        // Check if the interface is an Ethernet interface
        if ( family == AF_INET || family == AF_INET6 )
        {
            if ( ifa->ifa_flags & IFF_LOOPBACK )
            {
                continue; // Skip loopback interfaces
            }

            // Get the IP address
            bool getnameinfoSuccess = true;
            CORE::CString host;
            host.Reserve( (UInt32) NI_MAXHOST );
            int s = 0;
            do
            {
                s = ::getnameinfo( ifa->ifa_addr,
                                   (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                                         sizeof(struct sockaddr_in6),
                                   (char*)host.C_String(),
                                   host.ByteSize(),
                                   GUCEF_NULL,
                                   0,
                                   NI_NUMERICHOST  );
                if ( s == 0 )
                {
                    getnameinfoSuccess = true;

                    // this function provides no way to know how many bytes were written.
                    // only that it wont overflow the input buffer.
                    // As such we need to consider the entire buffer as valid data
                    host.DetermineLength();
                }
                else
                {
                    getnameinfoSuccess = false;
                    if ( s == EAI_OVERFLOW )
                    {
                        // just try again with a larger buffer
                        if ( GUCEF_NULL == host.Reserve( host.ByteSize() * 2 ) )
                            break; // out of memory ?
                    }
                    else
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LinuxNetworkInterface:EnumNetworkAdapters: getnameinfo failed with error code " + CORE::ToString( s ) );
                        s = 0;
                        break;
                    }
                }
            }
            while ( s != 0 );

            if ( !getnameinfoSuccess )
                continue; // no joy, try a different entry if any

            // the same nic id can be encountered multiple times in the list
            // for example once for IPv4 and once for IPv6
            // as such we need to check if we already created a nic with the given id
            CLinuxNetworkInterfacePtr nic;
            TNicMap::iterator n = nicMap.find( host );
            if ( n == nicMap.end() )
            {
                nic = CLinuxNetworkInterfacePtr( GUCEF_NEW CLinuxNetworkInterface() );
                nicMap[ host ] = nic;

                nic->m_name = ifa->ifa_name;
                nic->m_nicIndex = ::if_nametoindex( ifa->ifa_name );
            }
            if ( nic.IsNULL() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "LinuxNetworkInterface:EnumNetworkAdapters: No nic object" );
                continue;
            }

            // Now start filling in information for the NIC

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "LinuxNetworkInterface:EnumNetworkAdapters: found interface \"" +
                nic->m_name + "\" host " + host + " with index " + CORE::ToString( nic->m_nicIndex ) );

            int fd = ::socket( family, SOCK_DGRAM, 0 );
            if ( family == AF_INET )
            {
                CIPv4Address ipv4;
                if ( ipv4.SetAddress( host ) )
                {
                    nic->m_curIpAddr.ip = ipv4;

                    // Get additional IPv4 information via ioctl()

                    if ( fd != -1)
                    {
                        struct ifreq ifr;
                        ::strncpy( ifr.ifr_name, nic->m_name.C_String(), GUCEF_SMALLEST( IFNAMSIZ-1, nic->m_name.ByteSize() ) );
                        ifr.ifr_name[ IFNAMSIZ-1 ] = '\0';

                        int ioctlResult = ::ioctl( fd, SIOCGIFNETMASK, &ifr );
                        if ( ioctlResult != -1 )
                        {
                            struct sockaddr_in* netmask = (struct sockaddr_in*) &ifr.ifr_netmask;
                            char netmask_str[ INET_ADDRSTRLEN ];
                            ::inet_ntop( AF_INET, &netmask->sin_addr, netmask_str, INET_ADDRSTRLEN );

                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "LinuxNetworkInterface:EnumNetworkAdapters: Subnet Mask: " + CString( netmask_str ) );

                            // Calculate the subnet
                            struct in_addr ip_addr, subnet_mask, subnet;
                            ip_addr.s_addr = ipv4.GetAddressInHostByteOrder();

                            ::inet_pton( AF_INET, netmask_str, &subnet_mask );
                            subnet.s_addr = ip_addr.s_addr & subnet_mask.s_addr;

                            char subnet_str[INET_ADDRSTRLEN];
                            ::inet_ntop( AF_INET, &subnet, subnet_str, INET_ADDRSTRLEN );

                            CIPv4Address subnetIpv4;
                            subnetIpv4.SetAddress( subnet.s_addr );

                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "LinuxNetworkInterface:EnumNetworkAdapters: Subnet: " + ToString( subnetIpv4 ) );
                            nic->m_curIpAddr.subnet = subnetIpv4;
                        }
                        else
                        {
                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "LinuxNetworkInterface:EnumNetworkAdapters: ioctl has error getting SIOCGIFNETMASK: " + CString( errno ) );
                        }
                    }
                }
                else
                {
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LinuxNetworkInterface:EnumNetworkAdapters: Failed to parse IPv4 of interface \"" + nic->m_name + "\" host " + host );
                }

            }
            else
            {
                CIPv6Address ipv6;
                //if ( ipv6 )
                    // @TODO
            }

            // Get additional information via ioctl()

            struct ifreq ifr;
            ::strncpy( ifr.ifr_name, nic->m_name.C_String(), GUCEF_SMALLEST( IFNAMSIZ-1, nic->m_name.ByteSize() ) );
            ifr.ifr_name[ IFNAMSIZ-1 ] = '\0';

            int ioctlResult = ::ioctl( fd, SIOCGIFHWADDR, &ifr );
            if ( ioctlResult != -1 )
            {
                unsigned char* mac = reinterpret_cast< unsigned char* >( ifr.ifr_hwaddr.sa_data );

                char macAddress[ 24 ];
                sprintf( macAddress, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );
                CString macAddressStr = macAddress;

                nic->m_macAddrs.insert( macAddressStr );
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "LinuxNetworkInterface:EnumNetworkAdapters: MAC: " + macAddressStr );
            }
            else
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "LinuxNetworkInterface:EnumNetworkAdapters: ioctl has error getting SIOCGIFHWADDR: " + CString( errno ) );
            }

            ::close( fd );

            nic->FillExtraInfoFromConfig( networkConfigDiscoveryCache );

            interfaces.push_back( nic.StaticCast< CINetworkInterface >() );

        }
    }

    // Free the memory allocated by getifaddrs
    ::freeifaddrs( ifaddr );
    ifaddr = GUCEF_NULL;



   /*
    CORE::CString content;
    if ( LoadTextFileAsString( "/proc/net/dev", content, true, "\n" ) )
    {
        CORE::CString::StringVector lines = content.ParseElements( '\n', false );
        CORE::CString::StringVector::iterator i = lines.begin();

        // Skip the first two lines (headers)
        ++i; ++i;

        while ( i != lines.end() )
        {
            const CORE::CString& line = (*i);
            CORE::CString interfaceName;

            Int32 colonIndex = line.HasChar( ':', true );
            if ( colonIndex > 0 )
            {
                interfaceName = line.SubstrToIndex( colonIndex, true ).Trim( true );
                if ( !interfaceName.IsNULLOrEmpty() )
                {
                    CLinuxNetworkInterfacePtr nic = CLinuxNetworkInterfacePtr( GUCEF_NEW CLinuxNetworkInterface() );
                    nic->m_name = interfaceName;

                    interfaces.push_back( nic.StaticCast< CINetworkInterface >() );
                }
            }
            ++i;
        }
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LinuxNetworkInterface: Failed to obtain info from /proc/net/dev" );
    return false;
}
*/
	return true;
}

/*-------------------------------------------------------------------------*/

CLinuxNetworkInterface::CLinuxNetworkInterface( void )
    : CINetworkInterface()
    , m_name()
    , m_desc()
    , m_macAddrs()
    , m_priWins()
    , m_secWins()
    , m_defGateway()
    , m_dhcpAddr()
    , m_curIpAddr()
    , m_nicIndex( 0 )
    , m_adapterType( 0 )
    , m_dhcpUsedforIpv4( false )
    , m_dhcpUsedforIpv6( false )
    , m_winsUsed( false )
    , m_dnsAddresses()
    , m_ipv4Addresses()
    , m_gatewayList()
    , m_leaseObtained( 0 )
    , m_leaseExpires( 0 )
	, m_dataLock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLinuxNetworkInterface::~CLinuxNetworkInterface()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLinuxNetworkInterface::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CLinuxNetworkInterface";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLinuxNetworkInterface::GetCommunicationInterfaceType( void ) const
{GUCEF_TRACE;

    return "network";
}

/*-------------------------------------------------------------------------*/

CString
CLinuxNetworkInterface::GetAdapterName( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return m_name;
}

/*-------------------------------------------------------------------------*/

CString
CLinuxNetworkInterface::GetAdapterDescription( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return m_desc;
}

/*-------------------------------------------------------------------------*/

UInt32
CLinuxNetworkInterface::GetNrOfDnsAddresses( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return (UInt32) m_dnsAddresses.size();
}

/*-------------------------------------------------------------------------*/

bool
CLinuxNetworkInterface::GetDnsAddresses( THostAddressVector& dnsAddresses )
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	THostAddressVector::const_iterator i = m_dnsAddresses.begin();
	while ( i != m_dnsAddresses.end() )
	{
		dnsAddresses.push_back( (*i) );
		++i;
	}
	return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CLinuxNetworkInterface::GetNrOfIPAddresses( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return (UInt32) m_ipv4Addresses.size();
}

/*-------------------------------------------------------------------------*/

bool
CLinuxNetworkInterface::GetIPInfo( TIPv4InfoVector& ipInfo, bool includeUninitialized ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	CIPv4Info::TIPv4InfoSet::const_iterator i = m_ipv4Addresses.begin();
	while ( i != m_ipv4Addresses.end() )
	{
		if ( includeUninitialized || 0 != (*i).ip.GetAddress() )
			ipInfo.push_back( (*i) );
		++i;
	}
	return true;
}

/*-------------------------------------------------------------------------*/

bool
CLinuxNetworkInterface::IsDhcpUsedForIPv4( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return m_dhcpUsedforIpv4;
}

/*-------------------------------------------------------------------------*/

bool
CLinuxNetworkInterface::IsDhcpUsedForIPv6( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return m_dhcpUsedforIpv6;
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime
CLinuxNetworkInterface::GetDhcpLeaseObtainedTime( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	CORE::CDateTime result;
	result.FromUnixEpochBasedTicksInMillisecs( m_leaseObtained * 1000 );
	return result;
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime
CLinuxNetworkInterface::GetDhcpLeaseExpirationTime( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	CORE::CDateTime result;
	result.FromUnixEpochBasedTicksInMillisecs( m_leaseExpires * 1000 );
	return result;
}

/*-------------------------------------------------------------------------*/

bool
CLinuxNetworkInterface::ReleaseAddress( void )
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return false;
}

/*-------------------------------------------------------------------------*/

bool
CLinuxNetworkInterface::RenewAddress( void )
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return false;
}

/*-------------------------------------------------------------------------*/

bool
CLinuxNetworkInterface::IsWinsUsed( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return m_winsUsed;
}

/*-------------------------------------------------------------------------*/

CIPv4Address
CLinuxNetworkInterface::GetPrimaryWinsServer( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return m_priWins;
}

/*-------------------------------------------------------------------------*/

CIPv4Address
CLinuxNetworkInterface::GetSecondaryWinsServer( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return m_secWins;
}

/*-------------------------------------------------------------------------*/

UInt32
CLinuxNetworkInterface::GetOsAdapterIndex( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return m_nicIndex;
}

/*-------------------------------------------------------------------------*/

bool
CLinuxNetworkInterface::GetMetrics( CNetworkInterfaceMetrics& metrics ) const
{GUCEF_TRACE;


   /*
        std::istringstream iss(line);
        std::string interface;
        iss >> interface;

        // Remove the colon at the end of the interface name
        if (interface.back() == ':') {
            interface.pop_back();
        }

        if (interface == iface_name) {
            unsigned long rx_bytes, rx_packets, rx_errs, rx_drop, rx_fifo, rx_frame, rx_compressed, rx_multicast;
            unsigned long tx_bytes, tx_packets, tx_errs, tx_drop, tx_fifo, tx_colls, tx_carrier, tx_compressed;

            iss >> rx_bytes >> rx_packets >> rx_errs >> rx_drop >> rx_fifo >> rx_frame >> rx_compressed >> rx_multicast
                >> tx_bytes >> tx_packets >> tx_errs >> tx_drop >> tx_fifo >> tx_colls >> tx_carrier >> tx_compressed;

            std::cout << "Statistics for interface: " << iface_name << std::endl;
            std::cout << std::setw(15) << "RX Bytes: " << rx_bytes << std::endl;
            std::cout << std::setw(15) << "RX Packets: " << rx_packets << std::endl;
            std::cout << std::setw(15) << "RX Errors: " << rx_errs << std::endl;
            std::cout << std::setw(15) << "RX Dropped: " << rx_drop << std::endl;
            std::cout << std::setw(15) << "TX Bytes: " << tx_bytes << std::endl;
            std::cout << std::setw(15) << "TX Packets: " << tx_packets << std::endl;
            std::cout << std::setw(15) << "TX Errors: " << tx_errs << std::endl;
            std::cout << std::setw(15) << "TX Dropped: " << tx_drop << std::endl;

            return;
        }
    }

    std::cerr << "Interface " << iface_name << " not found." << std::endl;
}


*/

     /*

		MT::CObjectScopeLock lock( this );

			metrics.inboundOctets = interfaceInfo.InOctets;
			metrics.hasInboundOctets = true;
			metrics.inboundErroredPackets = interfaceInfo.InErrors;
			metrics.hasInboundErroredPackets = true;
			metrics.inboundDiscardedPackets = interfaceInfo.InDiscards;
			metrics.hasInboundDiscardedPackets = true;
			metrics.inboundUnknownProtocolPackets = interfaceInfo.InUnknownProtos;
			metrics.hasInboundUnknownProtocolPackets = true;
			metrics.inboundUnicastPackets = interfaceInfo.InUcastPkts;
			metrics.hasInboundUnicastPackets = true;
			metrics.outboundErroredPackets = interfaceInfo.OutErrors;
			metrics.hasOutboundErroredPackets = true;
			metrics.outboundDiscardedPackets = interfaceInfo.OutDiscards;
			metrics.hasOutboundDiscardedPackets = true;
			metrics.outboundOctets = interfaceInfo.OutOctets;
			metrics.hasOutboundOctets = true;
			metrics.outboundUnicastPackets = interfaceInfo.OutUcastPkts;
			metrics.hasOutboundUnicastPackets = true;

			metrics.transmitLinkSpeedBitsPerSec = interfaceInfo.TransmitLinkSpeed;
			metrics.hasTransmitLinkSpeedBitsPerSec = true;
			metrics.receiveLinkSpeedBitsPerSec = interfaceInfo.ReceiveLinkSpeed;
			metrics.hasReceiveLinkSpeedBitsPerSec = true;
			metrics.outboundMulticastOctets = interfaceInfo.OutMulticastOctets;
			metrics.hasOutboundMulticastOctets = true;
			metrics.inboundMulticastOctets = interfaceInfo.InMulticastOctets;
			metrics.hasInboundMulticastOctets = true;
			metrics.outboundBroadcastOctets = interfaceInfo.OutBroadcastOctets;
			metrics.hasOutboundBroadcastOctets = true;
			metrics.inboundBroadcastOctets = interfaceInfo.InBroadcastOctets;
			metrics.hasInboundBroadcastOctets = true;
			metrics.outboundNonUnicastPackets = interfaceInfo.OutNUcastPkts;
			metrics.hasOutboundNonUnicastPackets = true;
			metrics.inboundNonUnicastPackets = interfaceInfo.InNUcastPkts;
			metrics.hasInboundNonUnicastPackets = true;

*/
	return false;
}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CLinuxNetworkInterface::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CLinuxNetworkInterface::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_dataLock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CLinuxNetworkInterface::Unlock( void ) const
{GUCEF_TRACE;

    return m_dataLock.Unlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM ? */
