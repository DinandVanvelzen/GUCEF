/*
 *  ArchiveDiff: Little tool to help diff 2 patch sets
 *
 *  Copyright (C) 2002 - 2010.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef ARCHIVEDIFFLIB_H
#include "ArchiveDiffLib.h"
#define ARCHIVEDIFFLIB_H
#endif /* ARCHIVEDIFFLIB_H ? */

/*-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace ARCHIVEDIFF;

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

#ifdef GUCEF_LINUX_BUILD
void cls()
{
    system("cls");
}
#endif

/*-------------------------------------------------------------------------*/

void
ParseParams( const int argc                        ,
             char* argv[]                          ,
             GUCEF::CORE::CValueList& keyValueList )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Parsing parameters" );

    keyValueList.DeleteAll();
    GUCEF::CORE::CString argString;
    if ( argc > 0 )
    {
        argString = *argv;

        for ( int i=1; i<argc; ++i )
        {
            argString += ( ' ' + argv[ i ] );
        }

        keyValueList.SetMultiple( argString, '\'' );
    }
}

/*---------------------------------------------------------------------------*/

void
PrintHeader( void )
{GUCEF_TRACE;

    cls();
    printf( "*****************************************\n" );
    printf( "*                                       *\n" );
    printf( "*  Archive Diff                         *\n" );
    printf( "*                                       *\n" );
    printf( "*    Copyright (C) Dinand Vanvelzen     *\n" );
    printf( "*    2002 - 2010. LGPL Licenced.        *\n" );
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
    printf( "  Arguments should be passed in the form \'key=value\'\n" );
    printf( "    'TemplateArchiveIndex'  : path to patch set containing the index of the\n" );
    printf( "                              template archive\n" );
    printf( "    'MainArchiveIndex'      : path to patch set containing the index of the\n" );
    printf( "                              main SVN archive\n" );
    printf( "    'Plugins'               : optional parameter: comma seperated list of\n" );
    printf( "                              plugins to load\n" );
    printf( "    'DiffOutputDir'         : optional param to set diff file output dir,\n" );
    printf( "                              default is the tool executable dir\n" );
    printf( "    'MaxEntriesPerDiffFile' : optional param specifying the entries in diff\n" );
    printf( "                              file after which multiple files will be used\n" );
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MSWIN_BUILD

int __stdcall
WinMain( HINSTANCE hinstance     ,
         HINSTANCE hprevinstance ,
         LPSTR lpcmdline         ,
         int ncmdshow            )
{GUCEF_TRACE;

    int argc = 0;
    char** argv = &lpcmdline;
    if ( lpcmdline != GUCEF_NULL )
    {
        if ( *lpcmdline != '\0' )
        {
            argc = 1;
        }
    }

#else

int
main( int argc , char* argv[] )
{GUCEF_TRACE;

#endif

    CORE::CString logFilename = CORE::RelativePath( "$CURWORKDIR$" );
    CORE::AppendToPath( logFilename, "ArchiveDiff_Log.txt" );
    CORE::CFileAccess logFileAccess( logFilename, "w" );

    CORE::CStdLogger logger( logFileAccess );
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

    CORE::CPlatformNativeConsoleLogger nativeConsoleLogger;
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( nativeConsoleLogger.GetLogger() );

    CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "This tool was compiled on: " __DATE__ " @ " __TIME__ );

	if ( 0 == argc )
	{
	    #ifdef GUCEF_MSWIN_BUILD
	    CMSWin32ConsoleWindow console;
	    #endif

	    PrintHeader();
	    PrintHelp();
	    getchar();
	}
	else
	{
	    GUCEF::CORE::CValueList argList;
	    ParseParams( argc    ,
	                 argv    ,
	                 argList );

        if ( !argList.HasKey( "TemplateArchiveIndex" ) ||
             !argList.HasKey( "MainArchiveIndex" )      )
        {
            printf( "ERROR: Not enough parameters where provided\n\n" );
            PrintHelp();
            getchar();
            return 1;
        }

        CORE::CString templateArchiveIndex = argList[ "TemplateArchiveIndex" ];
        CORE::CString mainArchiveIndex = argList[ "MainArchiveIndex" ];
        CORE::CString pluginsListStr = argList.GetValueAlways( "Plugins" );
        CORE::CString diffOutputDir = argList.GetValueAlways( "DiffOutputDir" );
        CORE::CString maxEntriesPerDiffFileStr = argList.GetValueAlways( "MaxEntriesPerDiffFile" );
        if ( 0 == diffOutputDir.Length() )
        {
            diffOutputDir = CORE::RelativePath( "$MODULEDIR$" );
        }

        CORE::Int32 maxEntriesPerDiffFile = -1;
        if ( 0 != maxEntriesPerDiffFileStr )
        {
            maxEntriesPerDiffFile = CORE::StringToInt32( maxEntriesPerDiffFileStr );
        }

        PATCHER::CPatchSetParser::TPatchSet templatePatchset;
        PATCHER::CPatchSetParser::TPatchSet mainArchivePatchset;

        if ( LoadPatchSet( templateArchiveIndex ,
                           templatePatchset     ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully loaded and parsed the index for the template archive" );

            if ( LoadPatchSet( mainArchiveIndex    ,
                               mainArchivePatchset ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully loaded and parsed the index for the main SVN archive" );

                TFileStatusVector fileStatusList;
                if ( PerformArchiveDiff( templatePatchset    ,
                                         mainArchivePatchset ,
                                         fileStatusList      ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully completed determination of differences between the two archives" );

                    CORE::CString diffFilePath = diffOutputDir;
                    CORE::AppendToPath( diffFilePath, "ArchiveDiff.xml" );

                    if ( SaveFileStatusList( diffFilePath          ,
                                             fileStatusList        ,
                                             maxEntriesPerDiffFile ) )
                    {
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully saved the archive diff to file: " + diffFilePath );
                        return 1;
                    }
                    else
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to save the diff list to file: " + diffFilePath );
                    }
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to diff the archives" );
                }
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load and parse the index for the main SVN archive" );
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load and parse the index for the template archive" );
        }

    }
	return 0;
}

/*-------------------------------------------------------------------------*/
