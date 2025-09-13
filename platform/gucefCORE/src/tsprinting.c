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

#include <stdlib.h>
#include <malloc.h>	     /* memory management */	
#include <stdarg.h>          /* needed for var argument list */
#include <string.h>          /* standard string utils */

#include "tsprinting.h"	     /* function prototypes */

#ifdef GUCEF_MSWIN_BUILD
#include <windows.h>
#include <wincon.h>          /* for COORD */
#include <conio.h>           /* need conio.h under WIN32 for clrscr() */
#endif /* GUCEF_MSWIN_BUILD ? */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */ 

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/* #define USE_TSP_MUTEX */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef USE_TSP_MUTEX
  #ifndef MUTEX_H
    #include "mutex.h"           /* C mutex related functions */
    #define MUTEX_H
  #endif /* MUTEX_H ? */
#endif /* USE_TSP_MUTEX ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace CORE {
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static int init = 0;

#ifdef USE_TSP_MUTEX
static struct SMutex* = NULL;
#endif /* USE_TSP_MUTEX ? */

static char *coutfile = NULL;
static FILE *fptr = NULL;
static UInt32 usecoutfile = 0;
static UInt32 consoleout = 1;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *	Threadsafe version of printf(). This function does not garantee any order
 *	in which the output is generated from multiple threads but it does
 *	prevent output from getting mixed.
 */
int
tsprintf( const char * __format, ... )
{
	va_list arglist;
        int rt = 0;
	if ( init )
        {
                #ifdef USE_TSP_MUTEX
                MutexLock( lock );
                #endif /* USE_TSP_MUTEX ? */
               
                va_start( arglist, __format );
               // #ifdef CONSOLE_OUT
            //    if ( consoleout )
           //     {
                        rt = vprintf( __format, arglist );
            //    }
              //  #endif /* CONSOLE_OUT ? */
                if ( usecoutfile )
                {
                	if ( !fptr )
                        {
                        	fptr = fopen( coutfile, "a+" );
                        }
                        rt = vfprintf( fptr, __format, arglist );
                }
                va_end( arglist );
                
                #ifdef USE_TSP_MUTEX
                MutexUnlock( lock );
                #endif /* USE_TSP_MUTEX ? */                                
                return rt;
        }

        va_start( arglist, __format );
        #ifdef CONSOLE_OUT
        rt = vprintf( __format, arglist );
        #endif /* CONSOLE_OUT ? */
        if ( usecoutfile && coutfile )
        {
                if ( !fptr )
                {
                        fptr = fopen( coutfile, "a+" );
                }
                rt = vfprintf( fptr, __format, arglist );
             /*   #ifdef GUCEF_CORE_DEBUG_MODE */
                fflush( fptr );
             /*   #endif *//* GUCEF_CORE_DEBUG_MODE */
        }
        va_end( arglist );
        return rt;
}

/*-------------------------------------------------------------------------*/

/**
 *	Threadsafe version of fprintf(). This function does not garantee any order
 *	in which the output is generated from multiple threads but it does
 *	prevent output from getting mixed.
 */
int 
tsfprintf( FILE* __stream, const char* __format, ... )
{
	va_list arglist;
        int rt;
	if ( init )
        {
                #ifdef USE_TSP_MUTEX
                MutexLock( lock );
                #endif /* USE_TSP_MUTEX ? */ 
                
                va_start( arglist, __format );
                rt = vfprintf( __stream, __format, arglist );
                va_end( arglist );
                
                #ifdef USE_TSP_MUTEX
                MutexUnlock( lock );
                #endif /* USE_TSP_MUTEX ? */ 
                return rt;
        }
        va_start( arglist, __format );
        rt = vfprintf( __stream, __format, arglist );
        va_end( arglist );
        return rt;
}

/*-------------------------------------------------------------------------*/

/**
 *	Initializes the mutex's needed for these functions to work 
 */
void
tspinit( void )
{
        /*
         *      In WIN32 we will reopen the st out and in sources to
         *      allow ts functions to work even when this is not a
         *      console application.
         */
        //#ifdef GUCEF_MSWIN_BUILD
        //  #ifdef GUCEF_CORE_DEBUG_MODE        
        //  AllocConsole();
        //  #endif
        //
        ///* reopen stdin handle as console window input */
        //freopen("CONIN$","rb",stdin);
        ///* reopen stout handle as console window output */
        //freopen("CONOUT$","wb",stdout);
        ///* reopen stderr handle as console window output */
        //freopen("CONOUT$","wb",stderr);
        //#endif /* GUCEF_MSWIN_BUILD */
                
        if ( !init )
        {
                #ifdef USE_TSP_MUTEX
                lock = MutexCreate();
                if ( !lock ) return;
                #endif /* USE_TSP_MUTEX ? */  
                                
                init = 1;
        }
}

/*-------------------------------------------------------------------------*/

/**
 *	Cleanup after a tspinit() has been used.
 */
void
tspshutdown( void )
{
	if ( init )
        {
              //  #ifdef GUCEF_CORE_DEBUG_MODE
              //  FreeConsole();
              //  #endif
                
        	init = 0;
                #ifdef USE_TSP_MUTEX
                MutexDestroy( lock );       	
                lock = NULL;
                #endif /* USE_TSP_MUTEX ? */                  
        }
        if ( coutfile )
        {
        	free( coutfile );
        	coutfile = NULL;
        }
        if ( fptr )
        {
        	fclose( fptr );
                fptr = NULL;
        }
}

/*-------------------------------------------------------------------------*/

/**
 *	Function that allows you to set the console output file.
 *	The console output file will be used if tsusecoutfile() was called
 *	with argument true.
 */
void
tssetcoutfile( const char *cout_file )
{
	UInt32 len;
        #ifdef USE_TSP_MUTEX
        if ( init ) MutexLock( lock );
        #endif /* USE_TSP_MUTEX ? */ 	
        
        if ( fptr ) fclose( fptr );
        fptr = NULL;        
        len = (UInt32) strlen( cout_file )+1;
	free( coutfile );
        coutfile = (char*)malloc( len );
        strncpy( coutfile, cout_file, len );
        
        #ifdef USE_TSP_MUTEX
        if ( init ) MutexUnlock( lock );
        #endif /* USE_TSP_MUTEX ? */ 	
}

/*-------------------------------------------------------------------------*/

/**
 *	Set wheter or not to use a file for console output.
 *	the file specified with tssetcoutfile() will be used.
 */
void
tsusecoutfile( UInt32 use )
{
        usecoutfile = use;
}

/*-------------------------------------------------------------------------*/

/**
 *      Set wheter tsprintf() should output to console.
 */
void
tspconsoleout( UInt32 use )
{
        consoleout = use;
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MSWIN_BUILD
static void
my_win32_clrscr( void )
{
/*   Version 1 */
/*   Does not work for BCB6 combined with AllocConsole() */
/*        char a[80];
        printf("\033[2J");
        printf("\033[0;0f");    */

/*   Version 2 */
/*   Works for BCB6 combined with AllocConsole() !!! */
/*   Works for MSVC combined with AllocConsole() !!! */
        COORD coordScreen = { 0, 0 };
        DWORD cCharsWritten;
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        DWORD dwConSize;
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
        FillConsoleOutputCharacter(hConsole, TEXT(' '), dwConSize, coordScreen, &cCharsWritten);
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);
        SetConsoleCursorPosition(hConsole, coordScreen);      

/*   Version 3 */
/*   Works for BCB6 combined with AllocConsole() !!! */
/*        COORD coordScreen = { 0,0 };
        BOOL bSuccess;
        DWORD cCharsWritten;
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        DWORD dwConSize;
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        TCHAR text = ' ';
        bSuccess = GetConsoleScreenBufferInfo(hConsole, &csbi);
        dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
        FillConsoleOutputCharacter(hConsole, text, dwConSize,
          coordScreen, &cCharsWritten);
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        FillConsoleOutputAttribute(hConsole, csbi.wAttributes,
          dwConSize, coordScreen, &cCharsWritten);
        SetConsoleCursorPosition(hConsole, coordScreen);    */
}
#endif /* WIN32_BUILD ? */

/*-------------------------------------------------------------------------*/

/**
 *      It is recommended to use this fuction for clearing the contents of a
 *      console window. It wraps the appropriote O/S function and uses the
 *      proper file stream used by tspinit(). Under WIN32 the outpur streams get
 *      reopened for a non-console application so you can't clear the console
 *      window using a standard O/S function.
 */
void
console_clrscr( void )
{
        #ifdef GUCEF_MSWIN_BUILD
        my_win32_clrscr();
        #else
        system( "cls" );
        #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace CORE */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/
