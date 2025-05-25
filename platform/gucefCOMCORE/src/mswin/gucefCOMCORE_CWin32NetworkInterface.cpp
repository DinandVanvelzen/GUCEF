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

#include "gucefCOMCORE_CWin32NetworkInterface.h"
 
#if GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN

/*
 *	Per https://learn.microsoft.com/en-us/windows/win32/api/iptypes/ns-iptypes-ip_adapter_info
 * "To properly use the IP_ADAPTER_INFO structure on a 32-bit platform, define _USE_32BIT_TIME_T 
 *  when compiling the application to force the time_t datatype to a 4-byte datatype."
 */
#ifdef GUCEF_32BIT
	#define _USE_32BIT_TIME_T
#endif

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include <windows.h>
#include <tchar.h>
#include <iphlpapi.h>
//#include <ipifcons.h>

#ifndef GUCEF_COMCORE_IPHLPAPI_H
#include "gucefCOMCORE_IphlpapiApi.h"
#define GUCEF_COMCORE_IPHLPAPI_H
#endif /* GUCEF_COMCORE_IPHLPAPI_H ? */

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

bool
CWin32NetworkInterface::SetupAdapterInfo( void* pAdaptInfoVoid ) 
{GUCEF_TRACE;

	IP_ADAPTER_INFO* pAdaptInfo = (IP_ADAPTER_INFO*) pAdaptInfoVoid;
	IP_ADDR_STRING* pNext			= NULL;
	IP_PER_ADAPTER_INFO* pPerAdapt	= NULL;
	ULONG ulLen						= 0;
	ULONG errorCode					= 0;
	
	if( NULL == pAdaptInfo )
		return false;
	
	m_name = pAdaptInfo->AdapterName;		
	m_desc = pAdaptInfo->Description;

	m_priWins.SetAddress( pAdaptInfo->PrimaryWinsServer.IpAddress.String );
	m_secWins.SetAddress( pAdaptInfo->SecondaryWinsServer.IpAddress.String );
	m_nicIndex = pAdaptInfo->Index;		
	m_adapterType = pAdaptInfo->Type;	
	m_dhcpUsedForIPv4 = 0 != pAdaptInfo->DhcpEnabled;
	m_winsUsed = 0 != pAdaptInfo->HaveWins;	
	m_leaseObtained = pAdaptInfo->LeaseObtained;
	m_leaseExpires = pAdaptInfo->LeaseExpires;
	m_dhcpAddr.SetAddress( pAdaptInfo->DhcpServer.IpAddress.String );
		
	if( pAdaptInfo->CurrentIpAddress ) 
	{
		m_curIpAddr.ip.SetAddress( pAdaptInfo->CurrentIpAddress->IpAddress.String );
		m_curIpAddr.subnet.SetAddress( pAdaptInfo->CurrentIpAddress->IpMask.String );
	}
	else
	{
		m_curIpAddr.Clear();
	}

	// since an adapter may have more than one ip address we need
	// to populate the array we have setup with all available
	// ip addresses.
	pNext = &( pAdaptInfo->IpAddressList );
	while( pNext ) 
	{
		m_ipAddresses.push_back( CIPv4Info( pNext->IpAddress.String, pNext->IpMask.String, false ) );
		pNext = pNext->Next;
	}

	// an adapter usually has just one gateway however the provision exists
	// for more than one so to "play" as nice as possible we allow for it here
	// as well.
	pNext = &( pAdaptInfo->GatewayList );
	while( pNext ) 
	{
		CIPv4Address address;
		if ( address.SetAddress( pNext->IpAddress.String ) )
			m_gatewayList.push_back( address );
		pNext = pNext->Next;
	}	
			
	// we need to generate a IP_PER_ADAPTER_INFO structure in order
	// to get the list of dns addresses used by this adapter.
	bool obtainedtPerAdapterInfo = false;
	CORE::CDynamicBuffer perAdapterInfoBuffer;
	errorCode = ::GetPerAdapterInfo( m_nicIndex, NULL, &ulLen );
	if( errorCode == ERROR_BUFFER_OVERFLOW || errorCode == ERROR_INVALID_PARAMETER ) 
	{
		if ( perAdapterInfoBuffer.SetBufferSize( ulLen ) )
		{
			pPerAdapt = (IP_PER_ADAPTER_INFO*) perAdapterInfoBuffer.GetBufferPtr();
			errorCode = ::GetPerAdapterInfo( m_nicIndex, pPerAdapt, &ulLen );
			if( errorCode == ERROR_SUCCESS )
			{
				obtainedtPerAdapterInfo = true;
				perAdapterInfoBuffer.SetDataSize( ulLen );
			}
			else
			{
				GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWin32NetworkInterface: Failed to obtain per adapter information using Win32 GetPerAdapterInfo()" );
			}
		}
		else
		{
			GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWin32NetworkInterface: Failed to allocate required storage to hold per adapter information. Need " + CORE::UInt32ToString( ulLen ) + " bytes" );
		}
	}
	else
	{
		GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWin32NetworkInterface: Failed to get required storage size to hold per adapter information using Win32 GetPerAdapterInfo()" );
	}

	if ( obtainedtPerAdapterInfo )
	{
		pNext = &( pPerAdapt->DnsServerList );
		while( pNext != NULL ) 
		{
			CHostAddress address( pNext->IpAddress.String );
			m_dnsAddresses.push_back( address );
			pNext = pNext->Next;
		}				
	}

	return obtainedtPerAdapterInfo;
}

/*-------------------------------------------------------------------------*/

bool 
CWin32NetworkInterface::EnumNetworkAdapters( TINetworkInterfacePtrVector& interfaces ) 
{GUCEF_TRACE;

	IP_ADAPTER_INFO* pAdptInfo	= NULL;
	IP_ADAPTER_INFO* pNextAd	= NULL;	
	ULONG ulLen					= 0;
	ULONG errorCode				= 0;
	
	CORE::CDynamicBuffer adapterInfoBuffer;
	errorCode = ::GetAdaptersInfo( NULL, &ulLen );
	if( errorCode == ERROR_BUFFER_OVERFLOW ) 
	{
		if ( adapterInfoBuffer.SetBufferSize( ulLen ) )
		{
			pAdptInfo = (IP_ADAPTER_INFO*) adapterInfoBuffer.GetBufferPtr();
			errorCode = ::GetAdaptersInfo( pAdptInfo, &ulLen );
			if( errorCode != ERROR_SUCCESS )
			{
				GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWin32NetworkInterface: Failed to obtain adapter information using Win32 GetAdaptersInfo()" );
				return false;
			}
			else
			{
				adapterInfoBuffer.SetDataSize( ulLen );	
			}
		}
		else
		{
			GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWin32NetworkInterface: Failed to allocate required storage to hold adapter information. Need " + CORE::UInt32ToString( ulLen ) + " bytes" );
			return false;
		}
	}
	else
	{
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWin32NetworkInterface: Failed to get required storage size to hold adapter information using GetAdaptersInfo()" );
		return false;
	}

	// loop through for all available interfaces and setup an associated
	// CNetworkAdapter class.
	pNextAd = pAdptInfo;
	while( pNextAd != NULL ) 
	{
		CWin32NetworkInterfacePtr nic = CWin32NetworkInterfacePtr( GUCEF_NEW CWin32NetworkInterface() );
		if ( nic->SetupAdapterInfo( pNextAd ) )
		{
			interfaces.push_back( nic.StaticCast< CINetworkInterface >() );
		}
		pNextAd = pNextAd->Next;
	}

	return true;
}

/*-------------------------------------------------------------------------*/

CWin32NetworkInterface::CWin32NetworkInterface( void )
    : CINetworkInterface()
    , m_name()
    , m_desc()
    , m_priWins()
    , m_secWins()
    , m_defGateway()
    , m_dhcpAddr()
    , m_curIpAddr()
    , m_nicIndex( 0 )
    , m_adapterType( 0 )	
    , m_dhcpUsedForIPv4( false )
    , m_dhcpUsedForIPv6( false )
    , m_winsUsed( false )		
    , m_dnsAddresses()
    , m_ipAddresses()
    , m_gatewayList()
    , m_leaseObtained( 0 )
    , m_leaseExpires( 0 )
	, m_dataLock()
{GUCEF_TRACE;
          
}

/*-------------------------------------------------------------------------*/
    
CWin32NetworkInterface::~CWin32NetworkInterface()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWin32NetworkInterface::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CWin32NetworkInterface"; 
    return classTypeName;
}

/*-------------------------------------------------------------------------*/
    
CORE::CString
CWin32NetworkInterface::GetCommunicationInterfaceType( void ) const
{GUCEF_TRACE;

    return "network";
}

/*-------------------------------------------------------------------------*/

CString 
CWin32NetworkInterface::GetAdapterName( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return m_name;
}

/*-------------------------------------------------------------------------*/

CString 
CWin32NetworkInterface::GetAdapterDescription( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return m_desc;
}

/*-------------------------------------------------------------------------*/

UInt32 
CWin32NetworkInterface::GetNrOfDnsAddresses( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return (UInt32) m_dnsAddresses.size();
}

/*-------------------------------------------------------------------------*/
    
bool 
CWin32NetworkInterface::GetDnsAddresses( THostAddressVector& dnsAddresses )
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
CWin32NetworkInterface::GetNrOfIPAddresses( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return (UInt32) m_ipAddresses.size();
}

/*-------------------------------------------------------------------------*/

bool 
CWin32NetworkInterface::GetIPInfo( TIPv4InfoVector& ipInfo, bool includeUninitialized ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	TIPv4InfoVector::const_iterator i = m_ipAddresses.begin();
	while ( i != m_ipAddresses.end() )
	{
		if ( includeUninitialized || 0 != (*i).ip.GetAddress() )
			ipInfo.push_back( (*i) );
		++i;
	}
	return true;
}

/*-------------------------------------------------------------------------*/
		
bool 
CWin32NetworkInterface::IsDhcpUsedForIPv4( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return m_dhcpUsedForIPv4;
}

/*-------------------------------------------------------------------------*/

bool
CWin32NetworkInterface::IsDhcpUsedForIPv6( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return m_dhcpUsedForIPv6;
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime 
CWin32NetworkInterface::GetDhcpLeaseObtainedTime( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	CORE::CDateTime result;
	result.FromUnixEpochBasedTicksInMillisecs( m_leaseObtained * 1000 );
	return result;
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime 
CWin32NetworkInterface::GetDhcpLeaseExpirationTime( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	CORE::CDateTime result;
	result.FromUnixEpochBasedTicksInMillisecs( m_leaseExpires * 1000 );
	return result;
}

/*-------------------------------------------------------------------------*/

bool 
CWin32NetworkInterface::ReleaseAddress( void )
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return false;
}

/*-------------------------------------------------------------------------*/

bool 
CWin32NetworkInterface::RenewAddress( void )
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return false;
}

/*-------------------------------------------------------------------------*/

bool 
CWin32NetworkInterface::IsWinsUsed( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return m_winsUsed;
}

/*-------------------------------------------------------------------------*/

CIPv4Address
CWin32NetworkInterface::GetPrimaryWinsServer( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return m_priWins;
}

/*-------------------------------------------------------------------------*/

CIPv4Address 
CWin32NetworkInterface::GetSecondaryWinsServer( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return m_secWins;
}

/*-------------------------------------------------------------------------*/

UInt32 
CWin32NetworkInterface::GetOsAdapterIndex( void ) const
{GUCEF_TRACE;

	MT::CObjectScopeLock lock( this );
	return m_nicIndex;
}

/*-------------------------------------------------------------------------*/

bool 
CWin32NetworkInterface::GetMetrics( CNetworkInterfaceMetrics& metrics ) const
{GUCEF_TRACE;

	if ( GUCEF_NULL != COMCORE::GetIfEntry2 )
	{
		MT::CObjectScopeLock lock( this );

		// Make sure the struct is zeroed out
		COMCORE::MIB_IF_ROW2 interfaceInfo;
		SecureZeroMemory((PVOID) &interfaceInfo, sizeof(COMCORE::MIB_IF_ROW2) );
		interfaceInfo.InterfaceIndex = m_nicIndex;

		lock.EarlyUnlock();

		DWORD queryResult = COMCORE::GetIfEntry2( &interfaceInfo );
		if ( queryResult == NO_ERROR )
		{
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

			return true;
		}
		else
		{
			GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWin32NetworkInterface: Failed to obtain adapter stats using Win32 GetIfEntry2()" );
		}
	}
	else
	{
		MT::CObjectScopeLock lock( this );

		// Make sure the struct is zeroed out
		::MIB_IFROW interfaceInfo;
		SecureZeroMemory((PVOID) &interfaceInfo, sizeof(::MIB_IFROW) );
		interfaceInfo.dwIndex = m_nicIndex;

		lock.EarlyUnlock();

		DWORD queryResult = ::GetIfEntry( &interfaceInfo );
		if ( queryResult == NO_ERROR )
		{
			metrics.inboundOctets = interfaceInfo.dwInOctets;
			metrics.hasInboundOctets = true;
			metrics.inboundErroredPackets = interfaceInfo.dwInErrors;
			metrics.hasInboundErroredPackets = true;
			metrics.inboundDiscardedPackets = interfaceInfo.dwInDiscards;
			metrics.hasInboundDiscardedPackets = true;
			metrics.inboundUnknownProtocolPackets = interfaceInfo.dwInUnknownProtos;
			metrics.hasInboundUnknownProtocolPackets = true;
			metrics.inboundUnicastPackets = interfaceInfo.dwInUcastPkts;
			metrics.hasInboundUnicastPackets = true;
			metrics.outboundErroredPackets = interfaceInfo.dwOutErrors;
			metrics.hasOutboundErroredPackets = true;
			metrics.outboundDiscardedPackets = interfaceInfo.dwOutDiscards;
			metrics.hasOutboundDiscardedPackets = true;
			metrics.outboundOctets = interfaceInfo.dwOutOctets;
			metrics.hasOutboundOctets = true;
			metrics.outboundUnicastPackets = interfaceInfo.dwOutUcastPkts;
			metrics.hasOutboundUnicastPackets = true;

			return true;
		}
		else
		{
			GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWin32NetworkInterface: Failed to obtain adapter stats using Win32 GetIfEntry()" );
		}
	}
	return false;
}

/*-------------------------------------------------------------------------*/

const MT::CILockable* 
CWin32NetworkInterface::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CWin32NetworkInterface::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_dataLock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CWin32NetworkInterface::Unlock( void ) const
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

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ? */
