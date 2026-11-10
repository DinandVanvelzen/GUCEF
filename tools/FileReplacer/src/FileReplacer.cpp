/*
 *  FileReplacer: Tool for replacing files
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CTRACER_H
#include "CTracer.h"
#define GUCEF_CORE_CTRACER_H
#endif /* GUCEF_CORE_CTRACER_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define VERSION_NUMBER  1.0

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_MSWIN_BUILD

void cls( void )
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
    return;
}

/*---------------------------------------------------------------------------*/

class CMSWin32ConsoleWindow
{
    public:

    CMSWin32ConsoleWindow( void )
    {
        AllocConsole();

        /* reopen stdin handle as console window input */
        freopen( "CONIN$", "rb", stdin );

        /* reopen stout handle as console window output */
        freopen( "CONOUT$", "wb", stdout );

        /* reopen stderr handle as console window output */
        freopen( "CONOUT$", "wb", stderr );
    }

    ~CMSWin32ConsoleWindow()
    {
        FreeConsole();
    }
};

#endif /* GUCEF_MSWIN_BUILD ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
void cls( void )
{
    system( "cls" );
}
#endif

/*---------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM != GUCEF_PLATFORM_ANDROID )

/*---------------------------------------------------------------------------*/

void
PrintHeader( void )
{GUCEF_TRACE;

    cls();
    printf( "*****************************************\n" );
    printf( "*                                       *\n" );
    printf( "*  File Replacer                        *\n" );
    printf( "*                                       *\n" );
    printf( "*    Copyright (C) Dinand Vanvelzen     *\n");
    printf( "*    2002 - 2008. All rights reserved.  *\n");
    printf( "*                                       *\n" );
    printf( "*****************************************\n" );
    printf( "\n" );
    printf( " - Tool Version %f\n" , VERSION_NUMBER );
}

/*---------------------------------------------------------------------------*/

void
PrintHelp( void )
{GUCEF_TRACE;

    printf( "\n" );
    printf( " - Program arguments:\n" );
    printf( "  Arguments should be passed in the form \"old=new\"\n" );
    printf( "    old = the file you want to replace\n" );
    printf( "    new = the file you want to replace the old file with\n" );
}

/*---------------------------------------------------------------------------*/

#endif /* ( GUCEF_PLATFORM != GUCEF_PLATFORM_ANDROID ) */

/*---------------------------------------------------------------------------*/

void
ParseParams( const int argc                        ,
             char* argv[]                          ,
             GUCEF::CORE::CValueList& keyValueList )
{GUCEF_TRACE;

    keyValueList.DeleteAll();
    GUCEF::CORE::CString argString;
    if ( argc > 0 )
    {
        argString = *argv;

        // Combine the argument strings back into a single string because we don't want to use
        // a space as the seperator
        for ( int i=1; i<argc; ++i )
        {
            argString += ( ' ' + GUCEF::CORE::CString( argv[ i ] ) );
        }

        // Parse the param list based on the " symbol
        keyValueList.SetMultiple( argString, '"' );
    }
}

/*---------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

	if ( 0 == argc )
	{
	    #if GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN

	    CMSWin32ConsoleWindow console;

	    #endif

        #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_ANDROID )

	    PrintHeader();
	    PrintHelp();
	    getchar();

        #endif
	}
	else
	{
	    GUCEF::CORE::CValueList argList;
	    ParseParams( argc    ,
	                 argv    ,
	                 argList );

        for ( GUCEF::CORE::UInt32 i=0; i<argList.GetCount(); ++i )
        {
            GUCEF::CORE::CString destPath = argList.GetKey( i );
            if ( GUCEF::CORE::FileExists( destPath ) )
            {
                if ( !GUCEF::CORE::DeleteFile( destPath ) )
                {
                    printf( GUCEF::CORE::CString( "ERROR: Failed to delete file: " + destPath ).C_String() );

                    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_ANDROID )
                    getchar();
                    #endif

                    continue;
                }
                else
                {
                    printf( GUCEF::CORE::CString( "Deleted file: " + destPath ).C_String() );
                }
            }

            // Move the new file to the desired final location
            GUCEF::CORE::CString originPath = argList.GetValue( i );
            if ( !GUCEF::CORE::MoveFile( originPath, destPath, true ) )
            {
                printf( GUCEF::CORE::CString( "ERROR: Failed to move file: " + originPath + " -> " + destPath ).C_String() );

                #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_ANDROID )
                getchar();
                #endif
            }
            else
            {
                printf( GUCEF::CORE::CString( "Moved file: " + originPath + " -> " + destPath ).C_String() );
            }
        }

	}

	return 0;
}
GUCEF_OSMAIN_END

/*---------------------------------------------------------------------------*/
