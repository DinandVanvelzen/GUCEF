/*
 *  SVNMagicMerge: Little tool to help merge 2 convoluted archives
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

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SMergeException
{
    TFileStatus fileStatus;
    CORE::CString exceptionReason;
};
typedef struct SMergeException TMergeException;
typedef std::vector< TMergeException > TMergeExceptionList;

/*-------------------------------------------------------------------------*/

typedef std::vector< CORE::CString > TStringVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX || GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID

void cls()
{
    // @TODO: Implement for these platforms
}

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX || GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID */


/*-------------------------------------------------------------------------*/

CORE::CString
CreatePathToUnchangedFileDiff( const CORE::CString& dir )
{GUCEF_TRACE;
    
    CORE::CString path = dir;
    CORE::AppendToPath( path, "SVNMagicMerge_UnchangedFiles.xml" );
    return path;
}

/*---------------------------------------------------------------------------*/

CORE::CString
CreatePathToUnchangedButMovedFileDiff( const CORE::CString& dir )
{GUCEF_TRACE;
    
    CORE::CString path = dir;
    CORE::AppendToPath( path, "SVNMagicMerge_UnchangedButMovedFiles.xml" );
    return path;
}

/*---------------------------------------------------------------------------*/

CORE::CString
CreatePathToChangedFileDiff( const CORE::CString& dir )
{GUCEF_TRACE;
    
    CORE::CString path = dir;
    CORE::AppendToPath( path, "SVNMagicMerge_ChangedFiles.xml" );
    return path;
}

/*---------------------------------------------------------------------------*/

CORE::CString
CreatePathToMissingInTemplateFileDiff( const CORE::CString& dir )
{GUCEF_TRACE;
    
    CORE::CString path = dir;
    CORE::AppendToPath( path, "SVNMagicMerge_MissingInTemplateFiles.xml" );
    return path;
}

/*---------------------------------------------------------------------------*/

CORE::CString
CreatePathToMissingInMainFileDiff( const CORE::CString& dir )
{GUCEF_TRACE;
    
    CORE::CString path = dir;
    CORE::AppendToPath( path, "SVNMagicMerge_MissingInMainFiles.xml" );
    return path;
}

/*---------------------------------------------------------------------------*/

CORE::CString
CreatePathToUnknownStatusFileDiff( const CORE::CString& dir )
{GUCEF_TRACE;
    
    CORE::CString path = dir;
    CORE::AppendToPath( path, "SVNMagicMerge_UnknownStatusFiles.xml" );
    return path;

}

/*---------------------------------------------------------------------------*/

CORE::CString
CreatePathToMergeExceptionsFile( const CORE::CString& dir )
{GUCEF_TRACE;
    
    CORE::CString path = dir;
    CORE::AppendToPath( path, "SVNMagicMerge_MergeExceptions.xml" );
    return path;

}

/*-------------------------------------------------------------------------*/

bool
CreateUnknownStatusFilesList( const TFileStatusVector& fileStatusList   ,
                              TFileStatusVector& subset                 )
{GUCEF_TRACE;

    TFileStatusVector::const_iterator i = fileStatusList.begin();
    while ( i != fileStatusList.end() )
    {
        const TFileStatus& fileStatus = (*i);
        
        if ( fileStatus.resourceState == RESOURCESTATE_UNKNOWN )
        {
            subset.push_back( fileStatus );
        }
        
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CreateMissingInMainFilesList( const TFileStatusVector& fileStatusList   ,
                              TFileStatusVector& subset                 )
{GUCEF_TRACE;

    TFileStatusVector::const_iterator i = fileStatusList.begin();
    while ( i != fileStatusList.end() )
    {
        const TFileStatus& fileStatus = (*i);
        
        if ( fileStatus.resourceState == RESOURCESTATE_FILE_MISSING_IN_MAIN )
        {
            subset.push_back( fileStatus );
        }
        
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CreateMissingInTemplateFilesList( const TFileStatusVector& fileStatusList   ,
                                  TFileStatusVector& subset                 )
{GUCEF_TRACE;

    TFileStatusVector::const_iterator i = fileStatusList.begin();
    while ( i != fileStatusList.end() )
    {
        const TFileStatus& fileStatus = (*i);
        
        if ( fileStatus.resourceState == RESOURCESTATE_FILE_MISSING_IN_TEMPLATE )
        {
            subset.push_back( fileStatus );
        }
        
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CreateChangedFilesList( const TFileStatusVector& fileStatusList   ,
                        TFileStatusVector& subset                 )
{GUCEF_TRACE;

    TFileStatusVector::const_iterator i = fileStatusList.begin();
    while ( i != fileStatusList.end() )
    {
        const TFileStatus& fileStatus = (*i);
        
        if ( fileStatus.resourceState == RESOURCESTATE_FILE_CHANGED )
        {
            subset.push_back( fileStatus );
        }
        
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CreateUnchangedButMovedFilesList( const TFileStatusVector& fileStatusList   ,
                                  TFileStatusVector& unchangedButMovedFiles )
{GUCEF_TRACE;

    TFileStatusVector::const_iterator i = fileStatusList.begin();
    while ( i != fileStatusList.end() )
    {
        const TFileStatus& fileStatus = (*i);
        
        if ( fileStatus.resourceState == RESOURCESTATE_FILE_UNCHANGED_BUT_MOVED )
        {
            unchangedButMovedFiles.push_back( fileStatus );
        }
        
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CreateUnchangedFilesList( const TFileStatusVector& fileStatusList ,
                          TFileStatusVector& unchangedFiles       )
{GUCEF_TRACE;

    TFileStatusVector::const_iterator i = fileStatusList.begin();
    while ( i != fileStatusList.end() )
    {
        const TFileStatus& fileStatus = (*i);
        
        if ( fileStatus.resourceState == RESOURCESTATE_FILE_UNCHANGED )
        {
            unchangedFiles.push_back( fileStatus );
        }
        
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

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
    printf( "*  SVN Magic Merge                      *\n" );
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
    printf( "    'ArchiveDiffFile'         : path to a archive diff file\n" );
    printf( "    'MainArchiveRootPath'     : path to root of the main archive the diff file\n" );
    printf( "                                applies too\n" );
    printf( "    'TemplateArchiveRootPath' : path to root of the template archive the diff\n" ); 
    printf( "                                file applies too\n" );
    printf( "    'InfoOutputDir'    : optional param: path to dir where you want to tool\n" ); 
    printf( "                         output to be stored\n" );
    printf( "    'Plugins'          : optional param: comma seperated list of plugins\n" );
    printf( "                         to load\n" );
    printf( "    'SplitDiffOnly'    : optional param: boolean, if true all the tool will do\n" );
    printf( "                         is split the diff file into catagories, default false\n" );
    printf( "    'SvnToolDir'       : optional param: Allows you to specify the system dir\n");
    printf( "                         containing the SVN binaries (default = no path)\n" );
    printf( "    'ReuseCatagorizedOutput'  : optional param: Allows you to specify whether\n");
    printf( "                                the catagorized output should be reused\n" );
}

/*-------------------------------------------------------------------------*/

bool
PerformSvnCommit( const CORE::CString& svnCommitRoot )
{
    return false;
}

/*-------------------------------------------------------------------------*/

bool
DoesDirHaveSvnSubDir( const CORE::CString& path )
{GUCEF_TRACE;
    
    CORE::CString svnSubDir = path;
    CORE::AppendToPath( svnSubDir, ".svn" );
    if ( !CORE::IsPathValid( svnSubDir ) )
    {
        svnSubDir = path;
        CORE::AppendToPath( svnSubDir, "_svn" );
        if ( !CORE::IsPathValid( svnSubDir ) )
        {
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
PerformSvnLastSubDirAdd( const CORE::CString& path    ,
                         CORE::CString& failureReason )
{GUCEF_TRACE;

    if ( !DoesDirHaveSvnSubDir( path ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Last subdir of " + path + " is not under version control, thus we need to add it" );
        
        // we are now ready to perform the actual SVN add
        CORE::CString params = "add \"" + path + "\"";
        if ( 0 != CORE::Execute_Program( "svn"             , 
                                         params.C_String() ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SVN client reported success in adding directory: " + path );
            return true;
        }
        else
        {
            failureReason += "SVN tool reported failure executing add dir for: " + path + ";";
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, failureReason  );
            return false;
        }
    }
    
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Last subdir of " + path + " is already under version control thus no need to add it" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
PerformSvnMakeSurePathExists( const CORE::CString& givenPath   ,
                              const CORE::CString& archiveRoot ,
                              CORE::CString& failureReason     )
{GUCEF_TRACE;
    
    // Make sure the given path does not include a filename
    CORE::CString path = CORE::StripFilename( givenPath );
    
    // First check if the path exists on disk at all
    if ( !CORE::IsPathValid( path ) )
    {
        if ( CORE::CreateDirs( path ) )
        {
            // Failed to create the path using O/S
            failureReason += "garanteeing versioned path exists failed because the path could not be created using O/S functions at " + path + ";";
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, failureReason );
            return false;
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Used O/S functions to create path: " + path );
        }
    }
    
    // Now check if the path is versioned from the archive root down
    CORE::UInt32 equality = path.FindMaxSubstrEquality( archiveRoot, 0, true, false );
    CORE::CString relativePath = path.CutChars( equality, true );
    TStringVector relativePathSegments = relativePath.ParseElements( '\\', false );
        
    // test archive root first
    CORE::CString testPath = archiveRoot;    
    if ( !PerformSvnLastSubDirAdd( testPath      ,
                                   failureReason ) )
    {
        failureReason += "garanteeing versioned path exists failed because the last subdir of " + testPath + " could not be added as a versioned directory;";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, failureReason );
        return false;
    }
    
    // iterate down testing each sub-dir       
    TStringVector::iterator i = relativePathSegments.begin();
    while ( i != relativePathSegments.end() )
    {
        CORE::AppendToPath( testPath, (*i) );
        if ( !PerformSvnLastSubDirAdd( testPath      ,
                                       failureReason ) )
        {
            failureReason += "garanteeing versioned path exists failed because the last subdir of " + testPath + " could not be added as a versioned directory;";
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, failureReason );
            return false;
        }
        
        ++i;
    }
    
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully garanteed that path \"" + path + "\" exists and is versioned");
    return true;
}
/*-------------------------------------------------------------------------*/

bool
PerformSvnIsFileVersionedCheck( const CORE::CString& path    ,
                                CORE::CString& failureReason )
{GUCEF_TRACE;

    if ( CORE::FileExists( path ) )
    {    
        CORE::CString filename = CORE::ExtractFilename( path );
        CORE::CString fileDir = path.CutChars( filename.Length(), false );
        
        if ( CORE::IsPathValid( fileDir ) )
        {
            // try first (common) type of svn subdir
            CORE::CString svnSubDir = fileDir;
            CORE::AppendToPath( svnSubDir, ".svn\\text-base" );
            
            CORE::CString svnSubDirFile = svnSubDir;
            CORE::AppendToPath( svnSubDirFile, filename );
            
            if ( CORE::FileExists( svnSubDirFile ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "File at location \"" + path + "\" has a corresponding SVN entry at \"" + svnSubDirFile + "\" thus it is versioned" );
                return true;
            }
            
            // try second type of svn subdir
            svnSubDir = fileDir;
            CORE::AppendToPath( svnSubDir, "_svn\\text-base" );
            
            svnSubDirFile = svnSubDir;
            CORE::AppendToPath( svnSubDirFile, filename );
            
            if ( CORE::FileExists( svnSubDirFile ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "File at location \"" + path + "\" has a corresponding SVN entry at \"" + svnSubDirFile + "\" thus it is versioned" );
                return true;
            }
            
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "File at location \"" + path + "\" does not have a corresponding SVN entry, thus it is not versioned" );
        }
        else
        {
            // not existing = not versioned
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "File dir not found at location \"" + fileDir + "\" thus is cannot be a versioned item" );
        } 
    }
    else
    {
        // not existing = not versioned
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "File not found at location \"" + path + "\" thus is cannot be a versioned item" );
    }    
    return false;
}

/*-------------------------------------------------------------------------*/

bool
PerformSvnDeleteFile( const CORE::CString& path    ,
                      CORE::CString& failureReason )
{GUCEF_TRACE;

    if ( CORE::FileExists( path ) )
    {
        if ( PerformSvnIsFileVersionedCheck( path          ,
                                             failureReason ) )
        {
            // we are now ready to perform the actual SVN delete
            CORE::CString params = "delete \"" + path + "\"";
            if ( 0 != CORE::Execute_Program( "svn"             , 
                                             params.C_String() ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SVN client reported success in deleting file: " + path );
                return true;
            }
            else
            {
                failureReason += "SVN tool reported failure executing add dir for: " + path + ";";
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, failureReason  );
                return false;
            }
        }
        else
        {
            if ( CORE::DeleteFile( path ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Deleted unversioned file at : " + path );
                return true;
            }
            else
            {
                failureReason += "Failed to delete unversioned file at : " + path + ";";
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, failureReason );
            }
        }    
        return false;
    }
    
    // No such file, thus delete sucecssfull since we dont have to delete it
    return true;
}

/*-------------------------------------------------------------------------*/

bool
PerformSvnAddFile( const CORE::CString& source      ,
                   const CORE::CString& destination ,
                   CORE::CString& failureReason     )
{GUCEF_TRACE;

    // source must exist
    if ( CORE::FileExists( source ) )
    {
        // destination must be available
        if ( CORE::FileExists( destination ) )
        {
            // Check if destination file is under version control
            if ( PerformSvnIsFileVersionedCheck( destination   ,
                                                 failureReason ) )
            {
                failureReason += "Cannot SVN add file from " + source + " to " + destination + " because the there is already a versioned file at that location;";
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, failureReason  );
                return false;
            }
            else
            {
                // Since we are going to replace the file anyway with new content we can delete the
                // file at this location.
                if ( CORE::DeleteFile( destination ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "deleted unversioned file at destination location: " + destination );
                }
                else
                {
                    failureReason += "Cannot SVN add file from " + source + " to " + destination + " because a unversioned file already exists at the destination location and an attempt to delete it failed;";
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, failureReason  );
                    return false;
                }
            }
        }
        else
        {
            failureReason += "Cannot SVN add file from " + source + " to " + destination + " because the there is already a versioned file at that location;";
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, failureReason  );
            return false;
        }
        
        // Copy the file locally
        if ( 0 != CORE::Copy_File( source.C_String()      ,
                                   destination.C_String() ) )
        {
            // we are now ready to perform the actual SVN add
            CORE::CString params = "add \"" + destination + "\"";
            if ( 0 != CORE::Execute_Program( "svn"             , 
                                             params.C_String() ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SVN client reported success in adding file from " + source + " to " + destination );
                return true;
            }
            else
            {
                failureReason += "SVN tool reported failure executing adding file from " + source + " to " + destination + ";";
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, failureReason  );
                return false;
            }
        }
        else
        {
            failureReason += "Failed to perform an O/S file copy from " + source + " to " + destination + ";";
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, failureReason  );
            return false;
        }
    }
    else
    {
        failureReason += "Cannot SVN add file from " + source + " to " + destination + " because the source file does not exist;";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, failureReason  );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
PerformSvnMoveFile( const CORE::CString& source                 ,
                    const CORE::CString& destination            ,
                    const CORE::CString& destinationArchiveRoot ,
                    CORE::CString& failureReason                )
{GUCEF_TRACE;
    
    // sanity check
    if ( !CORE::FileExists( source ) )
    {
        failureReason += "Cannot SVN move file from " + source + " to " + destination + " because the source file does not exist;";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, failureReason  );
        return false;
    }
    
    // check if there is already a file at the destination
    if ( CORE::FileExists( destination ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Determined that a file already exists at a SVN Move destination: " + destination );
        
        // Check if destination file is under version control
        if ( !PerformSvnIsFileVersionedCheck( destination   ,
                                              failureReason ) )
        {
            // Since we are going to replace the file anyway with new content we can delete the
            // file at this location.
            if ( CORE::DeleteFile( destination ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "deleted unversioned file at destination location: " + destination );
            }
            else
            {
                failureReason += "Cannot SVN move file from " + source + " to " + destination + " because a unversioned file already exists at the destination location and an attempt to delete it failed;";
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, failureReason  );
                return false;
            }
        }
        else
        {
            failureReason += "Cannot SVN move file from " + source + " to " + destination + " because a versioned file already exists at the destination location;";
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, failureReason  );
            return false;
        }

    }
    
    // Make sure we have someplace to move the file to that is versioned
    if ( PerformSvnMakeSurePathExists( destination            ,
                                       destinationArchiveRoot ,
                                       failureReason          ) )
    {
        // we are now ready to perform the actual SVN move
        CORE::CString params = "move \"" + source + "\" \"" + destination + "\"";
        if ( 0 != CORE::Execute_Program( "svn"             , 
                                         params.C_String() ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SVN client reported success in move file from " + source + " to " + destination );
            return true;
        }
        else
        {
            failureReason += "SVN tool reported failure executing move file from " + source + " to " + destination + ";";
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, failureReason  );
            return false;
        }
    }
    else
    {
        failureReason += "SVN tool reported failure executing move file from " + source + " to " + destination + ";";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, failureReason  );
        return false;
    }
    
    return false;
}

/*-------------------------------------------------------------------------*/

bool
ProcessUnchangedButMovedFiles( const CORE::CString& InfoOutputDir           , 
                               const CORE::CString& mainArchiveRootPath     ,
                               const CORE::CString& templateArchiveRootPath ,
                               TMergeExceptionList& mergeExceptions         )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Commening processing of unchanged but moved files" );
    
    TFileStatusVector fileStatusList;
    CORE::CString indexFilePath = CreatePathToUnchangedButMovedFileDiff( InfoOutputDir );    
    if ( LoadFileStatusList( indexFilePath  ,
                             fileStatusList ) )  
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully loaded list of unchanged but moved files" );
        
        TFileStatusVector::iterator i = fileStatusList.begin();
        while ( i != fileStatusList.end() )
        {
            TFileStatus& fileStatus = (*i);
            
            // get relative paths to files
            CORE::CString relPathToFileInMain = GetPathForFile( fileStatus.mainSvnArchiveInfo );
            CORE::CString relPathToFileInTemplate = GetPathForFile( fileStatus.templateArchiveInfo );
            
            // create absolute paths from relative paths
            CORE::CString absPathToFileInMain = mainArchiveRootPath;
            CORE::AppendToPath( absPathToFileInMain, relPathToFileInMain );
            
            // Note that because the files ifs unchanged we can use the root of the main archive
            CORE::CString absPathToFileAsInTemplate = mainArchiveRootPath;
            CORE::AppendToPath( absPathToFileAsInTemplate, relPathToFileInTemplate );
            
            // Peform SVN move of file within the same archive
            CORE::CString failureReason;
            if ( PerformSvnMoveFile( absPathToFileInMain       , 
                                     absPathToFileAsInTemplate ,
                                     mainArchiveRootPath       ,
                                     failureReason             ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully moved unchanged file from " + absPathToFileInMain + " to " + absPathToFileAsInTemplate );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to SVN move unchanged but moved file from " + absPathToFileInMain + " to " + absPathToFileAsInTemplate  );

                TMergeException mergeException;
                mergeException.exceptionReason = failureReason;                
                mergeException.fileStatus = fileStatus;
                mergeExceptions.push_back( mergeException );
            }
            
            ++i;
        }
        
        return true;
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load list of unchanged but moved files" );
    }
    return false;    
}

/*-------------------------------------------------------------------------*/

bool
ProcessMissingInMainFiles( const CORE::CString& InfoOutputDir           , 
                           const CORE::CString& mainArchiveRootPath     ,
                           const CORE::CString& templateArchiveRootPath ,
                           TMergeExceptionList& mergeExceptions         )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Commening processing of files that are missing in the main archive" );
    
    TFileStatusVector fileStatusList;
    CORE::CString indexFilePath = CreatePathToMissingInMainFileDiff( InfoOutputDir );    
    if ( LoadFileStatusList( indexFilePath  ,
                             fileStatusList ) )  
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully loaded list of files that are missing in the main archive" );
        
        TFileStatusVector::iterator i = fileStatusList.begin();
        while ( i != fileStatusList.end() )
        {
            TFileStatus& fileStatus = (*i);
            
            // get relative paths to file
            CORE::CString relPathToFileInTemplate = GetPathForFile( fileStatus.templateArchiveInfo );
            
            // create absolute paths from relative paths
            CORE::CString absPathToFileInMain = mainArchiveRootPath;
            CORE::AppendToPath( absPathToFileInMain, relPathToFileInTemplate );
            
            // Note that because the files ifs unchanged we can use the root of the main archive
            CORE::CString absPathToFileInTemplate = templateArchiveRootPath;
            CORE::AppendToPath( absPathToFileInTemplate, relPathToFileInTemplate );
            
            // Peform SVN add for this file
            CORE::CString info;
            if ( PerformSvnAddFile( absPathToFileInTemplate , 
                                    absPathToFileInMain     ,
                                    info                    ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully added file from " + absPathToFileInMain + " to " + absPathToFileInTemplate );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to SVN move unchanged but moved file from " + absPathToFileInMain + " to " + absPathToFileInTemplate  );

                TMergeException mergeException;
                mergeException.exceptionReason = info;                
                mergeException.fileStatus = fileStatus;
                mergeExceptions.push_back( mergeException );
            }
            
            ++i;
        }
        
        return true;
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load list of files that are missing in the main archive" );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
ProcessFiles( const CORE::CString& InfoOutputDir           ,
              const CORE::CString& mainArchiveRootPath     ,
              const CORE::CString& templateArchiveRootPath ,
              TMergeExceptionList& mergeExceptions         )
{GUCEF_TRACE;

    if ( ProcessUnchangedButMovedFiles( InfoOutputDir           ,
                                        mainArchiveRootPath     ,
                                        templateArchiveRootPath ,
                                        mergeExceptions         ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully processed unchanged but moved files" );
        
        return true;
        
        //if ( ProcessMissingInMainFiles( InfoOutputDir           ,
        //                                mainArchiveRootPath     ,
        //                                templateArchiveRootPath ,
        //                                mergeExceptions         ) )
        //{
        //    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully processed files that where missing in the main archive" );
        //}
        //else
        //{
        //    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failure trying to process files that are missing in the main archive" );
        //}
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failure trying to process unchanged but moved files" );
    }
    
    return false;    
}

/*-------------------------------------------------------------------------*/

bool
SerializeMergeException( const TMergeException& mergeException ,
                         CORE::CDataNode& parentNode           )
{GUCEF_TRACE;

    CORE::CDataNode mergeExceptionNode;
    mergeExceptionNode.SetName( "MergeException" );
    mergeExceptionNode.SetAttribute( "Reason", mergeException.exceptionReason );

    if ( !SerializeFileStatus( mergeException.fileStatus ,
                               mergeExceptionNode        ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failure trying to serialize a merge exception's file status. Exception has reason: " + mergeException.exceptionReason );
        return false;
    }

    parentNode.AddChild( mergeExceptionNode );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
SaveMergeExceptions( const TMergeExceptionList& mergeExceptions ,
                     CORE::CDataNode& dataTree                  )
{GUCEF_TRACE;

    TMergeExceptionList::const_iterator i = mergeExceptions.begin();
    while ( i != mergeExceptions.end() )
    {
        if ( !SerializeMergeException( (*i)     ,
                                       dataTree ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failure trying to serialize merge exception with reason: " + (*i).exceptionReason );
            return false;
        }
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
SaveMergeExceptions( const TMergeExceptionList& mergeExceptions ,
                     const CORE::CString& InfoOutputDir         )
{GUCEF_TRACE;    
    
    CORE::CDataNode dataTree;
    dataTree.SetName( "SVNMagicMergeMergeExceptions" );
    
    if ( SaveMergeExceptions( mergeExceptions ,
                              dataTree        ) )
    {
        CORE::CString mergeExceptionsFilePath = CreatePathToMergeExceptionsFile( InfoOutputDir );
        if ( SaveXmlFile( mergeExceptionsFilePath ,
                          dataTree                ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully saved merged conditions data tree to file: " + mergeExceptionsFilePath );
            return true;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failure writing merge exceptions data tree to file: " + mergeExceptionsFilePath );
        }
    }
    return false;    
}

/*-------------------------------------------------------------------------*/

bool
BreakUpFileStatusListIntoSubsets( const TFileStatusVector& fileStatusList    ,
                                  const CORE::CString& InfoOutputDir         ,
                                  const GUCEF::CORE::Int32 maxEntriesPerFile )
{GUCEF_TRACE;

    // First we break up the large list into smaller more manageable ones
    TFileStatusVector subSetFileStatusList;
    if ( CreateUnchangedFilesList( fileStatusList       ,
                                   subSetFileStatusList ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully seperated list of unchanged files from the file status list" );
        CORE::CString subSetFilePath = CreatePathToUnchangedFileDiff( InfoOutputDir );
        if ( SaveFileStatusList( subSetFilePath       ,  
                                 subSetFileStatusList ,
                                 maxEntriesPerFile    ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully saved list of unchanged files to " + subSetFilePath );
            
            CORE::CString pathsOnlyFile = subSetFilePath + ".TemplatePathsOnly.txt";
            SaveFileStatusListRelativePaths( pathsOnlyFile, subSetFileStatusList, true );
            pathsOnlyFile = subSetFilePath + ".MainPathsOnly.txt";
            SaveFileStatusListRelativePaths( pathsOnlyFile, subSetFileStatusList, false );
                        
            subSetFileStatusList.clear();
            if ( CreateUnchangedButMovedFilesList( fileStatusList       ,
                                                   subSetFileStatusList ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully seperated list of unchanged but moved files from the file status list" );
                subSetFilePath = CreatePathToUnchangedButMovedFileDiff( InfoOutputDir );
                if ( SaveFileStatusList( subSetFilePath       ,  
                                         subSetFileStatusList ,
                                         maxEntriesPerFile    ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully saved list of unchanged but moved files to " + subSetFilePath );

                    pathsOnlyFile = subSetFilePath + ".TemplatePathsOnly.txt";
                    SaveFileStatusListRelativePaths( pathsOnlyFile, subSetFileStatusList, true );
                    pathsOnlyFile = subSetFilePath + ".MainPathsOnly.txt";
                    SaveFileStatusListRelativePaths( pathsOnlyFile, subSetFileStatusList, false );
                    
                    subSetFileStatusList.clear();
                    if ( CreateChangedFilesList( fileStatusList       ,
                                                 subSetFileStatusList ) )
                    {
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully seperated list of changed files from the file status list" );
                        subSetFilePath = CreatePathToChangedFileDiff( InfoOutputDir );
                        if ( SaveFileStatusList( subSetFilePath       ,  
                                                 subSetFileStatusList ,
                                                 maxEntriesPerFile    ) )
                        {
                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully saved list of changed files to " + subSetFilePath );
                            
                            pathsOnlyFile = subSetFilePath + ".TemplatePathsOnly.txt";
                            SaveFileStatusListRelativePaths( pathsOnlyFile, subSetFileStatusList, true );
                            pathsOnlyFile = subSetFilePath + ".MainPathsOnly.txt";
                            SaveFileStatusListRelativePaths( pathsOnlyFile, subSetFileStatusList, false );

                            subSetFileStatusList.clear();
                            if ( CreateMissingInTemplateFilesList( fileStatusList       ,
                                                                   subSetFileStatusList ) )
                            {
                                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully seperated list of files missing from the template archive out of the file status list" );
                                subSetFilePath = CreatePathToMissingInTemplateFileDiff( InfoOutputDir );
                                if ( SaveFileStatusList( subSetFilePath       ,  
                                                         subSetFileStatusList ,
                                                         maxEntriesPerFile    ) )
                                {
                                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully saved list of files missing from the template archive to " + subSetFilePath );
                                    
                                    pathsOnlyFile = subSetFilePath + ".TemplatePathsOnly.txt";
                                    SaveFileStatusListRelativePaths( pathsOnlyFile, subSetFileStatusList, true );
                                    pathsOnlyFile = subSetFilePath + ".MainPathsOnly.txt";
                                    SaveFileStatusListRelativePaths( pathsOnlyFile, subSetFileStatusList, false );

                                    subSetFileStatusList.clear();
                                    if ( CreateMissingInMainFilesList( fileStatusList       ,
                                                                       subSetFileStatusList ) )
                                    {
                                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully seperated list of files missing from the main archive out of the file status list" );
                                        subSetFilePath = CreatePathToMissingInMainFileDiff( InfoOutputDir  );
                                        if ( SaveFileStatusList( subSetFilePath       ,  
                                                                 subSetFileStatusList ,
                                                                 maxEntriesPerFile    ) )
                                        {
                                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully saved list of files missing from the main archive to " + subSetFilePath );
                                            
                                            pathsOnlyFile = subSetFilePath + ".TemplatePathsOnly.txt";
                                            SaveFileStatusListRelativePaths( pathsOnlyFile, subSetFileStatusList, true );
                                            pathsOnlyFile = subSetFilePath + ".MainPathsOnly.txt";
                                            SaveFileStatusListRelativePaths( pathsOnlyFile, subSetFileStatusList, false );

                                            subSetFileStatusList.clear();
                                            if ( CreateUnknownStatusFilesList( fileStatusList       ,
                                                                               subSetFileStatusList ) )
                                            {
                                                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully seperated list of files with an unknown status out of the file status list" );
                                                subSetFilePath = CreatePathToUnknownStatusFileDiff( InfoOutputDir  );
                                                if ( SaveFileStatusList( subSetFilePath       ,  
                                                                         subSetFileStatusList ,
                                                                         maxEntriesPerFile    ) )
                                                {
                                                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully saved list of files with an unknown status to " + subSetFilePath );
                                                    
                                                    pathsOnlyFile = subSetFilePath + ".TemplatePathsOnly.txt";
                                                    SaveFileStatusListRelativePaths( pathsOnlyFile, subSetFileStatusList, true );
                                                    pathsOnlyFile = subSetFilePath + ".MainPathsOnly.txt";
                                                    SaveFileStatusListRelativePaths( pathsOnlyFile, subSetFileStatusList, false );

                                                    subSetFileStatusList.clear();
                                                    return true;
                                                }
                                                else
                                                {
                                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to save a list of files with unknown status to " + subSetFilePath );
                                                }
                                            }
                                            else
                                            {
                                                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failure trying to process the file status list for files with an unknown state" );
                                            }
                                        }
                                        else
                                        {
                                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to save a list of files missing from the main archive to " + subSetFilePath );
                                        }
                                    }
                                    else
                                    {
                                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failure trying to process the file status list for files missing in the main archive" );
                                    }
                                }
                                else
                                {
                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to save a list of files missing from the template archive to " + subSetFilePath );
                                }
                            }
                            else
                            {
                                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failure trying to process the file status list for files missing in the template archive" );
                            }
                        }
                        else
                        {
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to save a list of changed files at " + subSetFilePath );
                        }                        
                    }
                    else
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failure trying to process the file status list for changed files" );
                    }
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to save a list of unchanged but moved files at " + subSetFilePath );
                }
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failure trying to process the file status list for unchanged but moved files" );
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to save a list of unchanged files at " + subSetFilePath );
        }
    }
    else
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Failure trying to process the file status list" );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
AreCatagorizedSubsetsOnDisk( const CORE::CString& InfoOutputDir )
{GUCEF_TRACE;
    
    CORE::CString unchangedFileDiff = CreatePathToUnchangedFileDiff( InfoOutputDir );
    CORE::CString unchangedButMovedFileDiff = CreatePathToUnchangedButMovedFileDiff( InfoOutputDir );
    CORE::CString changedFileDiff = CreatePathToChangedFileDiff( InfoOutputDir );
    CORE::CString missingInTemplateFileDiff = CreatePathToMissingInTemplateFileDiff( InfoOutputDir );
    CORE::CString missingInMainFileDiff = CreatePathToMissingInMainFileDiff( InfoOutputDir );
    CORE::CString unknownFileDiff = CreatePathToUnknownStatusFileDiff( InfoOutputDir );
    
    return IsFileStatusListOnDisk( unchangedFileDiff ) &&
           IsFileStatusListOnDisk( unchangedButMovedFileDiff ) &&
           IsFileStatusListOnDisk( changedFileDiff ) &&
           IsFileStatusListOnDisk( missingInTemplateFileDiff ) &&
           IsFileStatusListOnDisk( missingInMainFileDiff ) &&
           IsFileStatusListOnDisk( unknownFileDiff );
}

/*-------------------------------------------------------------------------*/

bool
CreateCatagorizedSubsets( const CORE::CString& archiveDiffFile ,
                          const CORE::CString& infoOutputDir   ,
                          const CORE::Int32 maxEntriesPerFile  ,
                          bool reuseCatagorizedOutput          )
{GUCEF_TRACE;

    if ( reuseCatagorizedOutput )
    {
        // test if files exist. If they do then do not re-generate
        if ( AreCatagorizedSubsetsOnDisk( infoOutputDir ) )
        {
            // The files already exist, skip this step
            return true;
        }
    }

    TFileStatusVector fileStatusList;
    if ( LoadFileStatusList( archiveDiffFile , 
                             fileStatusList  ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully loaded the file status list" );
        
        // First we break up the large list into smaller more manageable ones
        if ( BreakUpFileStatusListIntoSubsets( fileStatusList    ,
                                               infoOutputDir     ,
                                               maxEntriesPerFile ) )
        {
            // Reduce memory footprint
            fileStatusList.clear();
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully seperated file status list into subsets" );
            return true;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failure trying to break up the files list into action based subsets" );
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load and parse the index for the template archive" );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    CORE::CString logFilename = CORE::RelativePath( "$CURWORKDIR$" );
    CORE::AppendToPath( logFilename, "SVNMagicMerge_Log.txt" );
    CORE::CFileAccess logFileAccess( logFilename, "w" );

    CORE::CStdLogger logger( logFileAccess );
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

    #ifdef GUCEF_MSWIN_BUILD
    CORE::CMSWinConsoleLogger consoleOut;
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &consoleOut );
    #endif /* GUCEF_MSWIN_BUILD ? */
    
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
	                 
        if ( !argList.HasKey( "ArchiveDiffFile" )        ||
             !argList.HasKey( "MainArchiveRootPath" )    ||
             !argList.HasKey( "TemplateArchiveRootPath" ) )
        {
            printf( "ERROR: Not enough parameters where provided\n\n" );
            PrintHelp();
            getchar();
            return 1;
        }
        
        CORE::CString archiveDiffFile = argList[ "ArchiveDiffFile" ];
        CORE::CString mainArchiveRootPath = argList[ "MainArchiveRootPath" ];
        CORE::CString templateArchiveRootPath = argList[ "TemplateArchiveRootPath" ];
        CORE::CString pluginsListStr = argList.GetValueAlways( "Plugins" );
        CORE::CString infoOutputDir = argList.GetValueAlways( "InfoOutputDir" );
        CORE::CString splitDiffOnlyStr = argList.GetValueAlways( "SplitDiffOnly" );
        CORE::CString reuseCatagorizedOutputStr = argList.GetValueAlways( "ReuseCatagorizedOutput" );
        CORE::CString maxEntriesPerDiffFileStr = argList.GetValueAlways( "MaxEntriesPerDiffFile" );
        
        bool reuseCatagorizedOutput = false;
        if ( reuseCatagorizedOutputStr.Length() > 0 )
        {
            reuseCatagorizedOutput = CORE::StringToBool( reuseCatagorizedOutputStr );
        }        
        CORE::Int32 maxEntriesPerFile = -1;
        if ( 0 != maxEntriesPerDiffFileStr.Length() )
        {
            maxEntriesPerFile = CORE::StringToInt32( maxEntriesPerDiffFileStr );
        }
        if ( 0 == infoOutputDir.Length() )
        {
            infoOutputDir = CORE::RelativePath( "$MODULEDIR$" );
        }
        
        bool splitDiffOnly = false;
        if ( splitDiffOnlyStr.Length() > 0 )
        {
            splitDiffOnly = CORE::StringToBool( splitDiffOnlyStr );
        }        
        
        if ( CreateCatagorizedSubsets( archiveDiffFile        , 
                                       infoOutputDir          ,
                                       maxEntriesPerFile      ,
                                       reuseCatagorizedOutput ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully located (and created?) catagory subset files" );

            if ( splitDiffOnly )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SplitDiffOnly is set to true so no further processing will take place" );
                return 1;
            }
            
            // Process sub-sets
            TMergeExceptionList mergeExceptions;
            if ( ProcessFiles( infoOutputDir           ,
                               mainArchiveRootPath     ,
                               templateArchiveRootPath ,
                               mergeExceptions         ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully processed all files without fatal errors, saving merge exceptions totalling " + CORE::UInt32ToString( mergeExceptions.size() ) );
                
                if ( SaveMergeExceptions( mergeExceptions ,
                                          infoOutputDir   ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully saved a list of merge exceptions in dir: " + infoOutputDir );
                    return 1;
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failure trying to save merge exceptions to file in dir: " + infoOutputDir );
                }
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failure trying to process the files" );
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failure trying to create catagorized subsets of the diff" );
        }        
    }
	return 0;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------*/
