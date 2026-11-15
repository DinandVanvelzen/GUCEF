/*
 *  DuplicateFileFinder: Tool for locating duplicate files
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

#include <map>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define VERSION_NUMBER  1.0

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SFileInfo
{
    CORE::CString path;
    CORE::UInt32 size;
    CORE::CString hash;    
};
typedef struct SFileInfo TFileInfo;

typedef std::vector< TFileInfo > TFileInfoList;

struct SDuplicateFileEntry
{
    TFileInfo file1;
    TFileInfo file2;    
};
typedef struct SDuplicateFileEntry TDuplicateFileEntry;

typedef std::vector< TDuplicateFileEntry > TDuplicateFileEntryList;

typedef std::multimap< CORE::UInt32, TFileInfo* > TFileSizeMultiMap;
typedef std::pair< CORE::UInt32, TFileInfo* > TFileSizeMultiMapPair;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX || GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID

void
cls( void )
{
    /* makeme: see:
      http://www.cplusplus.com/forum/beginner/387/ */

}

#endif

/*-------------------------------------------------------------------------*/

CORE::CString
MD5FromFile( const CORE::CString& filePath )
{GUCEF_TRACE;

    CORE::UInt8 md5Digest[ 16 ];
    CORE::CFileAccess fileAccess( filePath, "rb" );
    fileAccess.Open();
    CORE::md5fromfile( fileAccess.CStyleAccess(), md5Digest );
    return CORE::MD5ToString( md5Digest );    
}

/*-------------------------------------------------------------------------*/

void
BuildFileInfoList( const CORE::CString& srcRoot ,
                   TFileInfoList& list          ,
                   bool recursive               ,
                   bool generateHash            )
{GUCEF_TRACE;

    struct CORE::SDI_Data* dirIt = CORE::DI_First_Dir_Entry( srcRoot.C_String() );
    if ( GUCEF_NULL != dirIt )
    {
        do
        {
            if ( CORE::DI_Is_It_A_File( dirIt ) != 0 )
            {
                TFileInfo newFileInfo;
                
                CORE::CString pathToFile = srcRoot;
                CORE::AppendToPath( pathToFile, DI_Name( dirIt ) );                
                newFileInfo.path = pathToFile;                
                newFileInfo.size = CORE::DI_Size( dirIt );
                
                if ( generateHash )
                {
                    newFileInfo.hash = MD5FromFile( pathToFile );
                }
                
                list.push_back( newFileInfo );
            }
            else
            {
                // Check if we are processing sub-dirs
                if ( recursive )
                {
                    if ( 0 != strcmp( ".", DI_Name( dirIt ) ) &&
                         0 != strcmp( "..", DI_Name( dirIt ) ) )
                    {
                        // recursively process the sub-dir
                        CORE::CString subDirRoot = srcRoot;
                        CORE::AppendToPath( subDirRoot, DI_Name( dirIt ) );
                        
                        BuildFileInfoList( subDirRoot   ,
                                           list         ,
                                           recursive    ,
                                           generateHash );
                    }
                }
            }
        }
        while ( CORE::DI_Next_Dir_Entry( dirIt ) != 0 );
        CORE::DI_Cleanup( dirIt );
    }
}

/*-------------------------------------------------------------------------*/

void
BuildSizeBasedIndex( TFileInfoList& list      ,
                     TFileSizeMultiMap& index )
{GUCEF_TRACE;

    TFileInfoList::iterator i = list.begin();
    while ( i != list.end() )
    {
        TFileInfo* fileInfo = &(*i);
        index.insert( TFileSizeMultiMapPair( fileInfo->size, fileInfo ) );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
BuildSizeAndHashBasedDuplicateList( TFileInfoList& list1          ,
                                    TFileInfoList& list2          ,
                                    TDuplicateFileEntryList& list )
{GUCEF_TRACE;

    TFileSizeMultiMap sizeBasedIndex1;
    TFileSizeMultiMap sizeBasedIndex2;
    
    BuildSizeBasedIndex( list1, sizeBasedIndex1 );
    BuildSizeBasedIndex( list2, sizeBasedIndex2 );
    
    // For each size entry in index1 find files with the same size in index2 
    std::pair< TFileSizeMultiMap::iterator, TFileSizeMultiMap::iterator > itPair;
    TFileSizeMultiMap::iterator i = sizeBasedIndex1.begin();
    while ( i != sizeBasedIndex1.end() )
    {
        // Find a match in both indeces
        itPair = sizeBasedIndex2.equal_range( (*i).first );
        
        // Check if entry (*i) has a hash generated for it, if not generate one
        TFileInfo* file1 = (*i).second;
        if ( file1->hash.IsNULLOrEmpty() )
        {
            file1->hash = MD5FromFile( file1->path );
        }
        
        // Iterate the matching set
        TFileSizeMultiMap::iterator n = itPair.first;
        while ( n != itPair.second )
        {
            // Get the fileinfo in index2 which matches up with the file in index1
            TFileInfo* file2 = (*n).second;
            
            // Just because files have the same file size doesnt mean they are the same.
            // We wont check the filenames since that is not a good indicator either
            // this tool has to be smarter then that as to not be fooled by file renames.
            // As such we will now generate a hash per file

            // Check if file2 has a hash generated for it, if not generate one
            if ( file2->hash.IsNULLOrEmpty() )
            {
                file2->hash = MD5FromFile( file2->path );
            }
            
            // We already know the size matches, now lets see if the hash matches
            if ( file1->hash == file2->hash )
            {
                // Both match, we will consider this a duplicate
                TDuplicateFileEntry dupEntry;
                dupEntry.file1 = *file1;
                dupEntry.file2 = *file2;
                list.push_back( dupEntry );
            }
            
            ++n;
        }        
        
        ++i;
    }    
}

/*-------------------------------------------------------------------------*/

void
BuildSizeAndHashBasedDuplicateList( const CORE::CString& srcRoot1 ,
                                    const CORE::CString& srcRoot2 ,
                                    TDuplicateFileEntryList& list )
{GUCEF_TRACE;

    TFileInfoList infoList1;
    TFileInfoList infoList2;
    
    BuildFileInfoList( srcRoot1  ,
                       infoList1 ,
                       true      ,
                       false     );
    BuildFileInfoList( srcRoot2  ,
                       infoList2 ,
                       true      ,
                       false     );

    BuildSizeAndHashBasedDuplicateList( infoList1 ,
                                        infoList2 ,
                                        list      );
}

/*-------------------------------------------------------------------------*/
          
void
PrintHeader( void )
{GUCEF_TRACE;

    cls();
    printf( "*****************************************\n" );
    printf( "*                                       *\n" );
    printf( "*  Duplicate File Finder                *\n" );
    printf( "*                                       *\n" );
    printf( "*    Copyright (C) Dinand Vanvelzen     *\n");
    printf( "*    2002 - 2007. All rights reserved.  *\n");
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
    printf( "  Arguments should be passed in the form \"key=value\"\n" );
    printf( "    'RootDirPath'     : A root directory you wish to include in the search\n" );
    printf( "    'OutputPath'      : The directory where you want the output to be written\n" );
    printf( "    'HashCheck'       : optional parameter: explicitly set whether to perform\n" );
    printf( "                        a hash check on the files, this is a lengthy operation\n" );    
    printf( "    'Filter'          : optional parameter: Filter mask that can be used to\n" );
    printf( "                        exclude files\n" );    
    printf( "    'PluginDir'       : optional parameter: path to a dir where plugins can be\n" );
    printf( "                        found\n" );
    printf( "    'ListCodec'       : optional parameter: the codec to use for the output\n" );
    printf( "                        patch set file, the default codec is 'xml'\n" );
}

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

        for ( int i=1; i<argc; ++i )
        {
            argString += ( ' ' + argv[ i ] );
        }
        
        keyValueList.SetMultiple( argString, '"' );
    }
}


/*-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;
               
    #ifdef GUCEF_CORE_DEBUG_MODE
    //GUCEF::CORE::GUCEF_LogStackToStdOut();
    //GUCEF::CORE::GUCEF_SetStackLogging( 1 );
    #endif /* GUCEF_CORE_DEBUG_MODE ? */

    try 
    {                               
        GUCEF::CORE::CString logFilename = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( logFilename, "DuplicateFileFinder_Log.txt" );
        GUCEF::CORE::CFileAccess logFileAccess( logFilename, "w" );
        
        GUCEF::CORE::CStdLogger logger( logFileAccess );
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        GUCEF::CORE::CPlatformNativeConsoleLogger console;
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

	    if ( 0 == argc )
	    {
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
    	                 
            if ( !argList.HasKey( "RootDirPath" ) ||
                 !argList.HasKey( "OutputPath"  )  )
            {
                printf( "ERROR: Not enough parameters where provided\n\n" );
                PrintHelp();
                getchar();
                return 1;
            }
           
            // Obtain the application arguments
            GUCEF::CORE::CString rootDirPath = argList.GetValueAlways( "RootDirPath" );
            GUCEF::CORE::CString outputPath = argList.GetValueAlways( "OutputPath" );
            GUCEF::CORE::CString listCodec = argList.GetValueAlways( "ListCodec" );
            GUCEF::CORE::CString pluginDir = argList.GetValueAlways( "PluginDir" );
            GUCEF::CORE::CString hashCheckStr = argList.GetValueAlways( "HashCheck" );
            GUCEF::CORE::CString filter = argList.GetValueAlways( "Filter" ); 

            // Prepare some vars
            GUCEF::CORE::CDStoreCodecRegistry::TDStoreCodecPtr codecPtr;
            bool hashCheck = false;
            
            if ( hashCheckStr.Length() != 0 )
            {
                // Override our default which is false
                hashCheck = GUCEF::CORE::StringToBool( hashCheckStr );
            }
            if ( listCodec.Length() == 0 )
            {
                // Use our codec default which is xml
                listCodec = "xml";
            }
            if ( pluginDir.Length() == 0 )
            {
                // Use our codec default plugin path
                pluginDir = GUCEF::CORE::RelativePath( "$MODULEDIR$\\plugins" );
            }
            
            // Attempt to obtain the code
            codecPtr = GUCEF::CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().Lookup( listCodec );
            if ( codecPtr == GUCEF_NULL )
            {
                // No point in proceeding if we don't have a valid codec with which we can
                // store the result
                return 0;
            }

            // Load all plugins, this allows us to support multiple codec's
            // with our minimal console interface
            GUCEF::CORE::CPluginControl* pluginControl = &CORE::CCoreGlobal::Instance()->GetPluginControl();
            pluginControl->AddAllPluginsFromDir( pluginDir, GUCEF::CORE::CString(), true );

            //// Now we ask the VFS to build the list for us.
            //// This operation can take a while
            //GUCEF::CORE::CDataNode fileListRoot;
            //vfs->GetList( fileListRoot ,
            //              ""           ,
            //              true         ,
            //              filter       ,
            //              hashCheck    );

            //// Store the full list to file
            //GUCEF::CORE::CString fullListFilePath( outputPath );
            //GUCEF::CORE::AppendToPath( fullListFilePath, "FullDirAndFileIndex." );
            //fullListFilePath += listCodec;
            //GUCEF::CORE::CFileAccess fullListFile( fullListFilePath );
            //
            //codecPtr->StoreDataTree( &fileListRoot, &fullListFile );
            //                         
            //// Now we have all the data we need to start looking for duplicate 
            //// files with ease
            //GUCEF::CORE::CDataNode duplicateList;
            //BuildDuplicateList( duplicateList ,
            //                    fileListRoot  );
            //                    
            //// Store the duplicate list to file
            //GUCEF::CORE::CString duplicateListFilePath( outputPath );
            //GUCEF::CORE::AppendToPath( duplicateListFilePath, "DuplicateFileList." );
            //duplicateListFilePath += listCodec;
            //GUCEF::CORE::CFileAccess duplicateListFile( fullListFilePath );
            //
            //codecPtr->StoreDataTree( &duplicateList, &duplicateListFile );
                                            
            return 1;
        }
        return 0;
    }
    catch ( GUCEF::CORE::CException& e )
    {
        #ifdef GUCEF_CORE_DEBUG_MODE
        GUCEF::CORE::GUCEF_PrintCallstack();
        GUCEF::CORE::GUCEF_DumpCallstack( "DuplicateFileFinder_callstack.txt" );
        #endif /* GUCEF_CORE_DEBUG_MODE ? */
            
        GUCEF::CORE::CString msg( "GUCEF exception during program execution\n\n" );
        msg += "file: " + GUCEF::CORE::CString( e.GetOriginFile() ) + "\n" + "line: " + GUCEF::CORE::Int32ToString( e.GetOriginLineNr() );
        
        GUCEF::CORE::ShowErrorMessage( "Unhandled GUCEF exception" ,
                                       msg.C_String()              );                  
    }
    catch( std::bad_alloc& e )
    {
        #ifdef GUCEF_CORE_DEBUG_MODE
        GUCEF::CORE::GUCEF_PrintCallstack();
        GUCEF::CORE::GUCEF_DumpCallstack( "DuplicateFileFinder_callstack.txt" );
        #endif /* GUCEF_CORE_DEBUG_MODE ? */
        
        GUCEF::CORE::CString msg( "Memory allocation failure during program execution\n\n" + GUCEF::CORE::CString( e.what() ) );
        GUCEF::CORE::ShowErrorMessage( "Unhandled STL bad_alloc exception" ,
                                       msg.C_String()                      );    
    }
    catch( std::exception& e )
    {
        #ifdef GUCEF_CORE_DEBUG_MODE
        GUCEF::CORE::GUCEF_PrintCallstack();
        GUCEF::CORE::GUCEF_DumpCallstack( "DuplicateFileFinder_callstack.txt" );
        #endif /* GUCEF_CORE_DEBUG_MODE ? */
        
        GUCEF::CORE::CString msg( "STL exception during program execution\n\n" + GUCEF::CORE::CString( e.what() ) );
        GUCEF::CORE::ShowErrorMessage( "Unhandled STL exception" ,
                                       msg.C_String()            );        
    }
    catch ( ... )
    {
        #ifdef GUCEF_CORE_DEBUG_MODE
        GUCEF::CORE::GUCEF_PrintCallstack();
        GUCEF::CORE::GUCEF_DumpCallstack( "DuplicateFileFinder_callstack.txt" );
        #endif /* GUCEF_CORE_DEBUG_MODE ? */
        
        GUCEF::CORE::ShowErrorMessage( "Unknown exception"                                                                 ,
                                       "Unhandled exception during program execution, the application will now terminate"  );                                                         
    }
    return 1;                                                                                                                              

}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 10-12-2006
        - Dinand: Added this comment section

---------------------------------------------------------------------------*/