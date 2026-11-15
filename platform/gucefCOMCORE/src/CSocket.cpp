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

#include <string.h>               /* string utils */

#include "CSocket.h"              /* header for this class */

#ifndef DVSTRUTILS_H
#include "dvstrutils.h"
#define DVSTRUTILS_H
#endif /* DVSTRUTILS_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"              /* we want the log manager for its defines */
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_COMCORE_DVSOCKET_H
#include "dvwinsock.h"
#define GUCEF_COMCORE_DVSOCKET_H
#endif /* GUCEF_COMCORE_DVSOCKET_H ? */

#ifndef CCOM_H
#include "CCom.h"		  /* network manager */
#define CCOM_H
#endif /* CCOM_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

  #undef FD_SETSIZE
  #define FD_SETSIZE 1      /* should set the size of the FD set struct to 1 for VC */
  #include <winsock2.h>
  #include <Ws2tcpip.h>
  #include <Wspiapi.h>

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

  #include <unistd.h>
  #include <sys/socket.h>
  #include <sys/types.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>

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

CSocket::CSocket( void )
{GUCEF_TRACE;

        /*
         *      Register the socket object so that it's capable of
         *      receiving update events.
         */
        CComCoreGlobal::Instance()->GetCom().RegisterSocketObject( this );
}

/*-------------------------------------------------------------------------*/

CSocket::~CSocket()
{GUCEF_TRACE;

        /*
         *      Unregister the socket object so that it's removed
         *      from the update event notification list
         */
        CComCoreGlobal::Instance()->GetCom().UnregisterSocketObject( this );
}

/*-------------------------------------------------------------------------*/

void
CSocket::SetSocketID( UInt32 sid )
{GUCEF_TRACE;
        _sid = sid;
}

/*-------------------------------------------------------------------------*/

UInt32
CSocket::GetSocketID( void ) const
{GUCEF_TRACE;

        return _sid;
}

/*-------------------------------------------------------------------------*/

bool
CSocket::ConvertToIPAddress( const CORE::CString& destaddrstr ,
                             const UInt16 destport            ,
                             CIPv4Address& resolvedDest         )
{GUCEF_TRACE;

    if ( CORE::Check_If_IPv4( destaddrstr.C_String() ) )
    {
        resolvedDest.SetAddress( inet_addr( destaddrstr.C_String() ) );
        if ( resolvedDest.GetAddress() == INADDR_NONE ) 
            return false;
        resolvedDest.SetPortInHostByteOrder( destport );
        return true;
    }
    else
    {
        #if 1

        int errorCode = 0;
        struct hostent* retval = dvsocket_gethostbyname( destaddrstr.C_String(), &errorCode );
        if ( retval != GUCEF_NULL )
        {
            GUCEF_DEBUG_LOG( 1, CORE::CString( "CSocket::ConvertToIPAddress(): gethostbyname(): full name: " ) + retval->h_name );
            char* addrStr = inet_ntoa( *( struct ::in_addr*)( retval->h_addr_list[0] ) );
            Int32 netaddr = inet_addr( addrStr );
            if ( netaddr >= 0 )
            {
                resolvedDest.SetAddress( netaddr );
                resolvedDest.SetPort( destport );
                return true;
            }
        }
        return false;

        #else

        /* Alternate method */

        struct addrinfo* info = GUCEF_NULL;
        CORE::CString portString( CORE::Int32ToString( destport ) );
        int retval = getaddrinfo( destaddrstr.C_String() ,
                                  portString.C_String()  ,
                                  GUCEF_NULL                   ,
                                  &info                  );
        if ( retval == 0 )
        {
            struct in_addr* addr = (struct in_addr*)info->ai_addr;
            DEBUGOUTPUTssss( "CSocket::ConvertToIPAddress(): resolved DNS name ", destaddrstr.C_String(), " to ", inet_ntoa( *addr ) );
            resolvedDest.netaddr = inet_addr( inet_ntoa( *addr ) ); // <- does this actually work ???
            resolvedDest.port = htons( destport );
            return true;
        }
        #ifdef GUCEF_COMCORE_DEBUG_MODE
        DEBUGOUTPUTsi( "CSocket::ConvertToIPAddress(): gethostbyname() failed with code ", retval );
        #endif
        return false;

        #endif
    }
}

/*-------------------------------------------------------------------------*/

bool
CSocket::ConvertFromIPAddress( const CIPv4Address& src     ,
                               CORE::CString& srcaddrstr ,
                               UInt16& srcport           )
{GUCEF_TRACE;

    ::in_addr addrStruct;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    addrStruct.S_un.S_addr = src.GetAddress();
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    addrStruct.s_addr = src.GetAddress();
    #endif

    const char* addrStr = inet_ntoa( addrStruct );
    if ( addrStr != GUCEF_NULL )
    {
        srcaddrstr = addrStr;
        srcport = ntohs( src.GetPort() );
        return true;
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
