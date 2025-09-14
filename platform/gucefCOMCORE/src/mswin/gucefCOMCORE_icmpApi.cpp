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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "gucefCOMCORE_icmpApi.h"

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MSWIN_BUILD

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

/* #include <Icmpapi.h> -> this is the header for the functions that we dynamicly link */  
#define WIN32_LEAN_AND_MEAN
#define _STD_USING
#include <winsock.h>                  /* windows networking API, used here for it's type declarations */

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32 DllRefCount = 0;
void* DllHandle = NULL;
TIcmpCreateFilePtr IcmpCreateFile = NULL;
TIcmpCloseHandlePtr IcmpCloseHandle = NULL;
TIcmpParseRepliesPtr IcmpParseReplies = NULL;
TIcmpSendEchoPtr IcmpSendEcho = NULL;
TIcmpSendEcho2Ptr IcmpSendEcho2 = NULL;
TIcmpSendEcho2VistaPtr IcmpSendEcho2Vista = NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
LinkICMPModule( const char* moduleName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( win32ApiLinkerMutex );
    if ( DllHandle == NULL )
    {
        DllHandle = CORE::LoadModuleDynamicly( moduleName );
        if ( DllHandle != NULL )
        {
            IcmpCreateFile = (TIcmpCreateFilePtr) CORE::GetFunctionAddress( DllHandle         ,
                                                                            "IcmpCreateFile"  ,
                                                                            0                 ).funcPtr;
            IcmpCloseHandle = (TIcmpCloseHandlePtr) CORE::GetFunctionAddress( DllHandle         ,
                                                                              "IcmpCloseHandle" ,
                                                                              sizeof( HANDLE )  ).funcPtr;
            IcmpParseReplies = (TIcmpParseRepliesPtr) CORE::GetFunctionAddress( DllHandle         ,
                                                                                "IcmpParseReplies" ,
                                                                                sizeof(LPVOID) + sizeof(DWORD) ).funcPtr;
            IcmpSendEcho = (TIcmpSendEchoPtr) CORE::GetFunctionAddress( DllHandle         ,
                                                                        "IcmpSendEcho"    ,
                                                                        sizeof( HANDLE ) + sizeof( IPAddr ) + 2*sizeof( LPVOID ) + sizeof( WORD ) + sizeof( PIP_OPTION_INFORMATION ) + 2*sizeof( DWORD ) ).funcPtr;
            IcmpSendEcho2Vista = (TIcmpSendEcho2VistaPtr) CORE::GetFunctionAddress( DllHandle         ,
                                                                                    "IcmpSendEcho2"    ,
                                                                                    2*sizeof( HANDLE ) + sizeof( PIO_APC_ROUTINE_NEW ) + sizeof( PVOID ) + sizeof( IPAddr ) + 2*sizeof( LPVOID ) + sizeof( WORD ) + sizeof( PIP_OPTION_INFORMATION ) + 2*sizeof( DWORD ) ).funcPtr;
            IcmpSendEcho2 = (TIcmpSendEcho2Ptr) CORE::GetFunctionAddress( DllHandle         ,
                                                                          "IcmpSendEcho2"    ,
                                                                          2*sizeof( HANDLE ) + sizeof( PIO_APC_ROUTINE_OLD ) + sizeof( PVOID ) + sizeof( IPAddr ) + 2*sizeof( LPVOID ) + sizeof( WORD ) + sizeof( PIP_OPTION_INFORMATION ) + 2*sizeof( DWORD ) ).funcPtr;
                                                                          
            bool foundFunctions = ( IcmpCreateFile != NULL ) && ( IcmpCloseHandle != NULL ) && ( IcmpSendEcho != NULL ) && ( IcmpParseReplies != NULL );
            if ( !foundFunctions )
            {
                CORE::UnloadModuleDynamicly( DllHandle );
                DllHandle = NULL;
                DllRefCount = 0;
                return false;            
            }
        
            // Successfully linked the module and functions
        }
        else
        {
            DllHandle = NULL;
            DllRefCount = 0;
            return false;            
        }
    }
    
    ++DllRefCount;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
LinkICMPOlderThenXP( void )
{GUCEF_TRACE;

    return LinkICMPModule( "ICMP.dll" );
}

/*-------------------------------------------------------------------------*/

bool
LinkICMPForXPAndHigher( void )
{GUCEF_TRACE;

    return LinkICMPModule( "iphlpapi.dll" );
}

/*-------------------------------------------------------------------------*/

void
UnlinkICMP( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( win32ApiLinkerMutex );    
    if ( DllHandle != NULL )
    {
        if ( DllRefCount == 1 )
        {
            CORE::UnloadModuleDynamicly( DllHandle );
            DllHandle = NULL;            
            --DllRefCount;
        }
    }
}

/*-------------------------------------------------------------------------*/

Int32
LinkICMP( void )
{GUCEF_TRACE;
    
    if ( !LinkICMPForXPAndHigher() )
    {
        if ( LinkICMPOlderThenXP() )
        {
            return 1;
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COMCORE ? */
} /* namespace GUCEF ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MSWIN_BUILD ? */
