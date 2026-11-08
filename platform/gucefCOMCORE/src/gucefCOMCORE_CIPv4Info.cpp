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

#include "gucefCOMCORE_CIPv4Info.h"  

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

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

CIPv4Info::CIPv4Info( void )
    : ip()
    , subnet()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIPv4Info::CIPv4Info( const CString& ipAsString, const CString& subnetAsString, bool resolveDns )
    : ip()
    , subnet()
{GUCEF_TRACE;

    if ( resolveDns )
    {
        CORE::CString::StringVector aliases;
        CIPv4Address::TIPv4AddressVector ipv4s;
        CIPv6Address::TIPv6AddressVector ipv6s;

        CDnsResolver::Resolve( ipAsString, 0, aliases, ipv4s, ipv6s );        
        if ( !ipv4s.empty() )
            ip.SetAddress( ipv4s[ 0 ].GetAddress() );
        
        aliases.clear();
        ipv4s.clear();
        ipv6s.clear();
        CDnsResolver::Resolve( subnetAsString, 0, aliases, ipv4s, ipv6s );        
        if ( !ipv4s.empty() )
            subnet.SetAddress( ipv4s[ 0 ].GetAddress() );
    }
    else
    {
        ip.SetAddress( ipAsString );
        subnet.SetAddress( subnetAsString );
    }
}

/*-------------------------------------------------------------------------*/

CIPv4Info::CIPv4Info( const CIPv4Info& src )
    : ip( src.ip )
    , subnet( src.subnet )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIPv4Info::~CIPv4Info()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIPv4Info& 
CIPv4Info::operator=( const CIPv4Info& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        ip = src.ip;
        subnet = src.subnet;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CIPv4Info::operator<( const CIPv4Info& other ) const
{GUCEF_TRACE;

    return ( ip < other.ip ) && ( subnet < other.subnet );
}

/*-------------------------------------------------------------------------*/

void
CIPv4Info::Clear( void )
{GUCEF_TRACE;

    ip.Clear();
    subnet.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CIPv4Info::TrySetFromCIDRNotationString( const CString& cidrIpv4Str )
{GUCEF_TRACE;

    Clear();

    if ( !cidrIpv4Str.IsNULLOrEmpty() )
    {
        // We only accept CIDR notation thus the / is mandatory
        Int32 lastCpIndex = cidrIpv4Str.HasChar( '/', false );
        if ( lastCpIndex > 0 )
        {
            CString ipv4Str = cidrIpv4Str.SubstrToIndex( lastCpIndex, true );

            if ( ip.SetAddress( ipv4Str ) )
            {                
                CString cidrBlockStr = cidrIpv4Str.SubstrToIndex( lastCpIndex+1, false );
                if ( !cidrBlockStr.IsNULLOrEmpty() )
                {
                    Int32 cidrBlockSize = (Int32) CORE::StringToUInt32( cidrBlockStr, 33 );
                    if ( cidrBlockSize >= 0 && cidrBlockSize <= 32 )
                    {
                        // the mask, which CidrToSubnetMask() gives in host byte order
                        subnet.SetAddressInHostByteOrder( CIPv4Address::CidrToSubnetMask( cidrBlockSize ) );
                        return true;
                    }
                }
            }
        }
    }

    Clear();
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIPv4Info::Contains( const CIPv4Address& address ) const
{GUCEF_TRACE;

    const UInt32 mask = subnet.GetAddressInHostByteOrder();
    return ( address.GetAddressInHostByteOrder() & mask ) == ( ip.GetAddressInHostByteOrder() & mask );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
