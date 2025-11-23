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

#ifndef GUCEF_COMCORE_DVSOCKET_H
#define GUCEF_COMCORE_DVSOCKET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"       /* library build defines & macros */
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

  #undef FD_SETSIZE
  #define FD_SETSIZE 1      /* should set the size of the FD set struct to 1 for VC */
  #include <winsock2.h>
  #include <Ws2tcpip.h>
  #include <Wspiapi.h>

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

  #include <errno.h>
  #include <unistd.h>
  #include <sys/socket.h>
  #include <sys/types.h>
  #include <netdb.h>
  #include <arpa/inet.h>
  #include <netinet/tcp.h>

#endif

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    /* TCP_NODELAY is not supported in Emscripten/WebAssembly environment
       WebAssembly uses WebSockets instead of raw TCP sockets */
    #ifndef TCP_NODELAY
    #define TCP_NODELAY 1  /* Define as a placeholder to avoid compilation errors */
    #endif
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

struct hostent;

#if !defined( _WINSOCKAPI_ ) && !defined( _WINSOCK2API_ )
typedef UINT_PTR        SOCKET;
typedef struct hostent HOSTENT;
typedef struct hostent *PHOSTENT;
typedef struct hostent FAR *LPHOSTENT;
#endif

struct fd_set;
struct sockaddr;
struct in_addr;
struct timeval;

#define DVSOCKET_EWOULDBLOCK    WSAEWOULDBLOCK

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

typedef struct hostent*         LPHOSTENT;
typedef struct in_addr*         LPIN_ADDR;
typedef struct sockaddr*        LPSOCKADDR;
typedef struct timeval          TIMEVAL;
typedef int SOCKET;

#define INVALID_SOCKET          (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#define DVSOCKET_EWOULDBLOCK    EWOULDBLOCK
#define SOCKADDR_IN             struct sockaddr_in

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace COMCORE {
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Prevent C++ Name mangeling
 */
#ifdef __cplusplus
   extern "C" {
#endif   /* __cplusplus */

/*--------------------------------------------------------------------------*/

        /* MSWIN specific functions */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
/*--------------------------------------------------------------------------*/

/**
 *      Checks whether winsock has already been initialized.
 *
 *      @return boolean indicating true (>0) or false (0)
 */
UInt32
IsWinsockInitialized( void );

/*-------------------------------------------------------------------------*/

/**
 *      Returns the global winsock
 *
 *      @return returns the global WSADATA* winsock data structure information
 */
const void*
GetWinsockData( void );

/*-------------------------------------------------------------------------*/

/**
 *      Initializes winsock if winsock has not yet been initialized
 *      Should be called before each instance of use for winsock.
 *      ie for each socket. Each call increments a counter that is
 *      decremented with a call to ShutdownWinsock().
 *
 *      @param desiredversion the winsock version you require
 */
void
InitWinsock( UInt16 desiredversion );

/*-------------------------------------------------------------------------*/

void
ShutdownWinsock( void );

/*-------------------------------------------------------------------------*/
#endif /* GUCEF_PLATFORM_MSWIN ? */

        /* End of MSWIN specific functions */

/*-------------------------------------------------------------------------*/

int
dvsocket_bind( SOCKET s                    ,
               const struct sockaddr* type ,
               int namelen                 ,
               int* error                  );

/*-------------------------------------------------------------------------*/

SOCKET
dvsocket_socket( int af       ,
                 int type     ,
                 int protocol ,
                 int* error   );

/*-------------------------------------------------------------------------*/

int
dvsocket_select( int nfds                      ,
                 fd_set* readfds               ,
                 fd_set* writefds              ,
                 fd_set* exceptfds             ,
                 const struct timeval* timeout ,
                 int* error                    );

 /*-------------------------------------------------------------------------*/

int
dvsocket_listen( SOCKET s    ,
                 int backlog ,
                 int* error  );

/*-------------------------------------------------------------------------*/

SOCKET
dvsocket_accept( SOCKET s              ,
                 struct sockaddr* addr ,
                 int* addrlen          ,
                 int* error            );

/*-------------------------------------------------------------------------*/

int
dvsocket_connect( SOCKET s                    ,
                  const struct sockaddr* addr ,
                  int namelen                 ,
                  int* error                  );

/*-------------------------------------------------------------------------*/

int
dvsocket_send( SOCKET s        ,
               const void* buf ,
               int len         ,
               int flags       ,
               int* error      );

/*-------------------------------------------------------------------------*/

int
dvsocket_sendto( SOCKET s                  ,
                 const void* buf           ,
                 int len                   ,
                 int flags                 ,
                 const struct sockaddr* to ,
                 int tolen                 ,
                 int* error                );

/*-------------------------------------------------------------------------*/

int
dvsocket_recv( SOCKET s   ,
               void* buf  ,
               int len    ,
               int flags  ,
               int* error );

/*-------------------------------------------------------------------------*/

int
dvsocket_recvfrom( SOCKET s                  ,
                   void* buf                 ,
                   int len                   ,
                   int flags                 ,
                   struct sockaddr* fromaddr ,
                   socklen_t* fromlen        ,
                   int* error                );

/*-------------------------------------------------------------------------*/

void
dvsocket_inet_ntoa( struct in_addr in ,
                    char* ip          );

/*-------------------------------------------------------------------------*/

LPHOSTENT
dvsocket_gethostbyname( const char* name ,
                        int* error       );

/*-------------------------------------------------------------------------*/

LPHOSTENT
dvsocket_gethostbyaddr( const char* addr ,
                        int len          ,
                        int type         ,
                        int* error       );

/*-------------------------------------------------------------------------*/

int
dvsocket_closesocket( SOCKET s   ,
                      int* error );

/*-------------------------------------------------------------------------*/

int
dvsocket_setsockopt( SOCKET s           ,
                     int level          ,
                     int optname        ,
                     const char* optval ,
                     int optlen         ,
                     int* error         );

/*-------------------------------------------------------------------------*/

int
dvsocket_getsockopt( SOCKET s     , 
                     int level    , 
                     int optname  ,               
                     char* optval ,
                     int* optlen  ,
                     int* error   );

/*-------------------------------------------------------------------------*/

int
dvsocket_setsockopt_and_validate( SOCKET s           ,
                                  int level          ,
                                  int optname        ,
                                  const char* optval ,
                                  int optlen         ,
                                  char* actualOptval ,
                                  int* actualOptlen  ,
                                  int* error         );

/*--------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace COMCORE */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_DVSOCKET_H ? */
