/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */
 

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestCode_DNS.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::COMCORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDNSTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DNS" );

    GUCEF_TESTFW_TESTCASE( "Test: Host addresses hold a host name and a port" )
        try
        {
            // localhost is resolved without the need for a DNS server
            CHostAddress testDns;
            ASSERT_TRUE( testDns.GetHostname().IsNULLOrEmpty() );
            ASSERT_TRUE( testDns.SetHostnameAndPort( "localhost", 80 ) );
            ASSERT_TRUE( testDns.GetHostname() == "localhost" );
            ASSERT_TRUE( testDns.GetPortInHostByteOrder() == 80 );
            ASSERT_TRUE( testDns.HasDnsBasedHostname() );
            ASSERT_TRUE( !testDns.IsIPv4Only() );

            // host name and port given as one string, with environment variables in it
            CORE::GUCEFSetEnv( "test_hostname", "localhost" );
            CORE::GUCEFSetEnv( "test_port", "80" );
            testDns.Clear();
            ASSERT_TRUE( testDns.SetHostnameAndPort( "$ENVVAR:test_hostname$:$ENVVAR:test_port$" ) );
            ASSERT_TRUE( testDns.GetHostname() == "localhost" );
            ASSERT_TRUE( testDns.GetPortInHostByteOrder() == 80 );
            ASSERT_TRUE( testDns.HasDnsBasedHostname() );  // we know we have a DNS not just an IPv4 in string form
            ASSERT_TRUE( !testDns.IsIPv4Only() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The DNS cache resolves names" )
        try
        {
            CDnsCache& dnsCache = CComCoreGlobal::Instance()->GetDnsCache();
            CDnsCacheEntryPtr localEntry = dnsCache.GetOrAddCacheEntryForDns( "localhost" );
            ASSERT_TRUE( !localEntry.IsNULL() );
            if ( !localEntry.IsNULL() )
            {
                ASSERT_TRUE( "localhost" == localEntry->GetDnsName() );
                CIPv4Address::TIPv4AddressVector ipv4s;
                ASSERT_TRUE( localEntry->GetIPv4Addresses( ipv4s ) );
                ASSERT_TRUE( !ipv4s.empty() );
            }

            // A public name needs a DNS server, a machine which is offline cannot resolve it which is no failure of the cache
            CDnsCacheEntryPtr publicEntry = dnsCache.GetOrAddCacheEntryForDns( "google.com" );
            CIPv4Address::TIPv4AddressVector publicIpv4s;
            if ( publicEntry.IsNULL() || !publicEntry->GetIPv4Addresses( publicIpv4s ) || publicIpv4s.empty() )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DNS test: skipping the public name check, google.com could not be resolved (offline?)" );
            }
            else
            {
                ASSERT_TRUE( "google.com" == publicEntry->GetDnsName() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
