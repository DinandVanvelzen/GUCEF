/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include "gucefMT_mswinutils.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef NTSTATUS( WINAPI *pNtQIT )( HANDLE, LONG, PVOID, ULONG, PULONG );

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#if _MSC_VER >= 1300    // for VC 7.0
  // from ATL 7.0 sources
  #ifndef _delayimp_h
  extern "C" IMAGE_DOS_HEADER __ImageBase;
  #endif
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILTIIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

LONG 
TryNtQueryInformationThread( HANDLE ThreadHandle                                   ,
                             WIN32_THREAD_INFORMATION_CLASS ThreadInformationClass ,
                             void* ThreadInformation                               ,
                             ULONG ThreadInformationLength                         ,
                             PULONG ReturnLength                                   )
{GUCEF_TRACE;

    static pNtQIT NtQueryInformationThread = (pNtQIT) GetProcAddress( GetModuleHandle("ntdll.dll"), "NtQueryInformationThread" );
	if ( GUCEF_NULL == NtQueryInformationThread ) 
        return NTSTATUS_NOT_SUPPORTED;

    return NtQueryInformationThread( ThreadHandle, (LONG) ThreadInformationClass, ThreadInformation, ThreadInformationLength, ReturnLength );
}

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ? */
