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

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"       /* build defines */
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #define FD_SETSIZE 1      /* should set the size of the FD set struct to 1 for VC */
  #include <winsock2.h>
  #include <Ws2tcpip.h>
  #include <Wspiapi.h>
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #include <fcntl.h>
  #include <unistd.h>
  #include <sys/socket.h>
  #include <sys/types.h>
#endif

#include "socketutils.h"

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

bool
SetBlockingMode( SOCKET sock ,
                 bool block  )
{
    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

	int opts = 0;

	opts = fcntl( sock,F_GETFL);
	if ( opts < 0 )
	{
        return false;
	}
	if ( block )
	{
	    opts = (opts & ~O_NONBLOCK);
	}
	else
	{
	    opts = (opts | O_NONBLOCK);
    }

	if ( fcntl( sock, F_SETFL, opts ) < 0 )
	{
        return false;
	}
	return true;

	#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // About ioctlsocket():
    // *argp value       Nonblocking mode
    // 0                 Disabled
    // nonzero           Enabled

	u_long blockmode = ( block ? 0 : 1 );
    return ioctlsocket( sock                    ,
                        FIONBIO                 ,
                        (u_long FAR*)&blockmode ) != SOCKET_ERROR;
    
    #else
    return false;
    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
