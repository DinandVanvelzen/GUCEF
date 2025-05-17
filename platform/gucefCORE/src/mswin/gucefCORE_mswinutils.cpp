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

#include "gucefCORE_mswinutils.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef NTSTATUS ( WINAPI *pNtOP )( PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PCLIENT_ID );
typedef NTSTATUS ( WINAPI *pNtRVM )( HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T );
typedef NTSTATUS ( WINAPI *pNtQIP )( HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG );
typedef NTSTATUS ( WINAPI *pNtQSI )( SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG );

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILTIIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

HMODULE 
GetCurrentModuleHandle( void )
{GUCEF_TRACE;

#if _MSC_VER < 1300    // earlier than .NET compiler (VC 6.0)

  // Here's a trick that will get you the handle of the module
  // you're running in without any a-priori knowledge:
  // http://www.dotnet247.com/247reference/msgs/13/65259.aspx

  MEMORY_BASIC_INFORMATION mbi;
  static int dummy;
  VirtualQuery( &dummy, &mbi, sizeof(mbi) );

  return reinterpret_cast<HMODULE>(mbi.AllocationBase);

#else    // VC 7.0

  // from ATL 7.0 sources

  return reinterpret_cast<HMODULE>(&__ImageBase);
#endif
}

/*-------------------------------------------------------------------------*/

void
cls( void )
{GUCEF_TRACE;

    COORD coordScreen = { 0, 0 }; /* here's where we'll home the cursor */
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */
    DWORD dwConSize; /* number of character cells in the current buffer */

    /* get the output console handle */
    HANDLE hConsole=GetStdHandle(STD_OUTPUT_HANDLE);
    
    /* get the number of character cells in the current buffer */
    GetConsoleScreenBufferInfo(hConsole, &csbi);    
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    
    /* fill the entire screen with blanks */
    FillConsoleOutputCharacter(hConsole, (TCHAR) ' ',
      dwConSize, coordScreen, &cCharsWritten);
      
    /* get the current text attribute */
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    
    /* now set the buffer's attributes accordingly */
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes,
      dwConSize, coordScreen, &cCharsWritten);
      
    /* put the cursor at (0, 0) */
    SetConsoleCursorPosition(hConsole, coordScreen);
}

/*-------------------------------------------------------------------------*/

NTSTATUS
TryNtOpenProcess( PHANDLE ProcessHandle               ,
                  ACCESS_MASK DesiredAccess           ,
                  POBJECT_ATTRIBUTES ObjectAttributes ,
                  PCLIENT_ID ClientId                 )
{GUCEF_TRACE;

    static pNtOP NtOpenProcess = (pNtOP) ::GetProcAddress( GetModuleHandle("ntdll.dll"), "NtOpenProcess" );
	if ( NULL == NtOpenProcess ) 
        return NTSTATUS_NOT_SUPPORTED;

    return NtOpenProcess( ProcessHandle, DesiredAccess, ObjectAttributes, ClientId );
}

/*-------------------------------------------------------------------------*/

NTSTATUS
TryNtQueryInformationProcess ( HANDLE ProcessHandle                     ,
                               PROCESSINFOCLASS ProcessInformationClass ,
                               PVOID ProcessInformation                 ,
                               ULONG ProcessInformationLength           ,
                               PULONG ReturnLength                      )
{GUCEF_TRACE;

    static pNtQIP NtQueryInformationProcess = (pNtQIP) ::GetProcAddress( GetModuleHandle("ntdll.dll"), "NtQueryInformationProcess" );
	if ( NULL == NtQueryInformationProcess ) 
        return NTSTATUS_NOT_SUPPORTED;

    return NtQueryInformationProcess( ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength );
}

/*-------------------------------------------------------------------------*/

NTSTATUS
TryNtQuerySystemInformation( SYSTEM_INFORMATION_CLASS SystemInformationClass ,
                             PVOID SystemInformation                         ,
                             ULONG SystemInformationLength                   ,
                             PULONG ReturnLength                             )
{GUCEF_TRACE;

    static pNtQSI NtQuerySystemInformation = (pNtQSI) ::GetProcAddress( GetModuleHandle("ntdll.dll"), "NtQuerySystemInformation" );
	if ( NULL == NtQuerySystemInformation ) 
        return NTSTATUS_NOT_SUPPORTED;

    return NtQuerySystemInformation( SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength );
}

/*-------------------------------------------------------------------------*/

LONG 
TryNtReadVirtualMemory( HANDLE ThreadHandle        ,
                        PVOID BaseAddress          ,
                        PVOID Buffer               ,
                        SIZE_T NumberOfBytesToRead ,
                        PSIZE_T NumberOfBytesRead  )
{GUCEF_TRACE;

    static pNtRVM NtReadVirtualMemory = (pNtRVM) ::GetProcAddress( GetModuleHandle("ntdll.dll"), "NtReadVirtualMemory" );
	if ( NULL == NtReadVirtualMemory ) 
        return NTSTATUS_NOT_SUPPORTED;

    return NtReadVirtualMemory( ThreadHandle, BaseAddress, Buffer, NumberOfBytesToRead, NumberOfBytesRead );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ? */
