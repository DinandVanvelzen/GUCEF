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

#include "gucefCOMCORE_IphlpapiApi.h"

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

MT::CMutex win32ApiLinkerMutex;

UInt32 lphlpDllRefCount = 0;
void* lphlpDllHandle = NULL;
TGetIfEntry2Ptr GetIfEntry2 = NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
UnlinkLPHLPAPI( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( win32ApiLinkerMutex );
    if ( lphlpDllHandle != NULL )
    {
        if ( lphlpDllRefCount == 1 )
        {
            CORE::UnloadModuleDynamicly( lphlpDllHandle );
            lphlpDllHandle = NULL;            
            --lphlpDllRefCount;
        }
    }
}

/*-------------------------------------------------------------------------*/

Int32
LinkLPHLPAPI( void )
{GUCEF_TRACE;
    
    MT::CScopeMutex lock( win32ApiLinkerMutex );
    if ( lphlpDllHandle == NULL )
    {
        lphlpDllHandle = CORE::LoadModuleDynamicly( "iphlpapi.dll" );
        if ( lphlpDllHandle != NULL )
        {
            GetIfEntry2 = (TGetIfEntry2Ptr) CORE::GetFunctionAddress( lphlpDllHandle         ,
                                                                      "GetIfEntry2"          ,
                                                                      sizeof( PMIB_IF_ROW2 ) ).funcPtr;
                                                                          
            bool foundFunctions = ( GetIfEntry2 != NULL );
            if ( !foundFunctions )
            {
                CORE::UnloadModuleDynamicly( lphlpDllHandle );
                lphlpDllHandle = NULL;
                lphlpDllRefCount = 0;
                return false;            
            }
        
            // Successfully linked the module and functions
        }
        else
        {
            lphlpDllHandle = NULL;
            lphlpDllRefCount = 0;            
            return false;            
        }
    }
    
    ++lphlpDllRefCount;
    return true;
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
