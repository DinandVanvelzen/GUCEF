/*
 *  ReferenceUpdater: Little tool to help keep binary references up to date
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

#ifndef GUCEF_CORE_DVSTRUTILS_H
#include "dvstrutils.h"
#define GUCEF_CORE_DVSTRUTILS_H
#endif /* GUCEF_CORE_DVSTRUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

/*-------------------------------------------------------------------------*/

using namespace GUCEF;

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

struct SFileEntry
{
    CORE::CString filename;
    CORE::CString filedir;
};
typedef struct SFileEntry TFileEntry;

typedef std::vector< TFileEntry > TFileEntryVector;

/*-------------------------------------------------------------------------*/

struct SMatchEntry
{
    TFileEntry source;
    TFileEntryVector destinations;
};
typedef struct SMatchEntry TMatchEntry;

typedef std::vector< TMatchEntry > TMatchEntryVector;

/*-------------------------------------------------------------------------*/

typedef std::set< CORE::CString > TStringSet;
typedef std::vector< CORE::CString > TStringVector;
typedef std::map< CORE::CString, TStringVector > TStringVectorMap;

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

void
BuildFileList( const CORE::CString& srcDir         ,
               const CORE::CString& relativeSrcDir ,
               TFileEntryVector& files             ,
               const TStringSet* fileTypes         ,
               const TStringSet* dirsToIgnore      )
{GUCEF_TRACE;

    struct CORE::SDI_Data* dirEntry = CORE::DI_First_Dir_Entry( srcDir.C_String() );
    if ( dirEntry != NULL )    
    {
        do 
        {
            CORE::CString entryName = CORE::DI_Name( dirEntry );
            if ( ( entryName != "." )  &&
                 ( entryName != ".." )  )
            {
                // Check if this is a dir we have to ignore
                if ( 0 == CORE::DI_Is_It_A_File( dirEntry ) )
                {
                    if ( NULL != dirsToIgnore )
                    {
                        if ( dirsToIgnore->find( entryName.Lowercase() ) != dirsToIgnore->end() )
                        {
                            // do not process further
                            CORE::CString ignoredDir = srcDir;
                            CORE::AppendToPath( ignoredDir, entryName );
                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Ignoring directory \"" + ignoredDir + "\"" );
                            continue;
                        }
                    }
                }
                
                if ( 0 != CORE::DI_Is_It_A_File( dirEntry ) )
                {
                    if ( NULL != fileTypes )
                    {
                        CORE::CString fileExt = CORE::Extract_File_Ext( entryName.C_String() );
                        fileExt = fileExt.Lowercase();
                        if ( fileTypes->find( fileExt ) == fileTypes->end() )
                        {
                            // skip this entry as instructed, not a matching filetype
                            continue;
                        }
                    }
                    
                    TFileEntry fileEntry;
                    fileEntry.filedir = relativeSrcDir;
                    fileEntry.filename = entryName;
                    
                    files.push_back( fileEntry );
                }
                else
                {
                    // We found a sub-dir, process it
                    CORE::CString subDirPath = srcDir;
                    CORE::AppendToPath( subDirPath, entryName );
                    
                    CORE::CString subRelSrcDir = relativeSrcDir;
                    CORE::AppendToPath( subRelSrcDir, entryName );
                    
                    BuildFileList( subDirPath   ,
                                   subRelSrcDir ,
                                   files        ,
                                   fileTypes    ,
                                   dirsToIgnore );
                }
            }            
        } 
        while ( CORE::DI_Next_Dir_Entry( dirEntry ) != 0 );
        
        // clean up our toys
        CORE::DI_Cleanup( dirEntry );
    }
}

/*-------------------------------------------------------------------------*/

void
BuildFileList( const CORE::CString& srcDir         ,
               TFileEntryVector& files             ,
               const TStringSet* fileTypes         ,
               const TStringSet* dirsToIgnore      )
{GUCEF_TRACE;

    BuildFileList( srcDir          ,
                   CORE::CString() ,
                   files           ,
                   fileTypes       ,
                   dirsToIgnore    );
}

/*-------------------------------------------------------------------------*/

const TStringSet&
GetIncludeFileTypes( void )
{GUCEF_TRACE;

    static TStringSet fileTypes;
    if ( fileTypes.empty() )
    {
        fileTypes.insert( "h" );
        fileTypes.insert( "hpp" );
    }
    return fileTypes;
}

/*-------------------------------------------------------------------------*/

const TStringSet&
GetBinaryFileTypes( void )
{GUCEF_TRACE;

    static TStringSet fileTypes;
    if ( fileTypes.empty() )
    {
        fileTypes.insert( "pdb" );
        fileTypes.insert( "dll" );
        fileTypes.insert( "so" );
        fileTypes.insert( "lib" );
        fileTypes.insert( "manifest" );
    }
    return fileTypes;
}

///*---------------------------------------------------------------------------*/
//
//CORE::CDStoreCodecRegistry::TDStoreCodecPtr
//GetXmlDStoreCodec( void )
//{GUCEF_TRACE;
//
//    static CORE::CDStoreCodecRegistry::TDStoreCodecPtr codecPtr;
//    if ( codecPtr.IsNULL() )
//    {
//        CORE::CDStoreCodecRegistry* registry = CORE::CDStoreCodecRegistry::Instance();
//        if ( !registry->TryLookup( "XML", codecPtr, false ) )
//        {
//            // No codec is registered to handle XML, try and load a plugin for it
//
//            CORE::CPluginManager::TPluginPtr codecPlugin =
//
//                #ifdef GUCEF_CORE_DEBUG_MODE
//                CORE::CDStoreCodecPluginManager::Instance()->LoadPlugin( "$MODULEDIR$/dstorepluginPARSIFALXML_d" );
//                #else
//                CORE::CDStoreCodecPluginManager::Instance()->LoadPlugin( "$MODULEDIR$/dstorepluginPARSIFALXML" );
//                #endif
//
//            if ( NULL != codecPlugin )
//            {
//                // Now try and get the codec again
//                registry->TryLookup( "XML", codecPtr, false );
//                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Request for data storage codec for xml file, succesfully loaded plugin to handle request" );
//            }
//            else
//            {
//                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Request for data storage codec for xml file but no plugin for it could be loaded!" );
//                CORE::ShowErrorMessage( "Missing codec support",
//                                        "Request for data storage codec for xml file but no plugin for it could be loaded!" );
//            }
//        }
//    }
//    return codecPtr;
//}
//
///*---------------------------------------------------------------------------*/
//
//bool
//LoadXmlFile( const CORE::CString& filePath  ,
//             CORE::CDataNode& dataTree      )
//{GUCEF_TRACE;
//
//    if ( CORE::FileExists( filePath ) )
//    {
//        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully located file: " + filePath );
//        
//        CORE::CDStoreCodecRegistry::TDStoreCodecPtr codecPtr = GetXmlDStoreCodec();
//        if ( !codecPtr.IsNULL() )
//        {
//            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully located codec: " + codecPtr->GetTypeName() );
//            
//            if ( codecPtr->BuildDataTree( &dataTree ,
//                                          filePath  ) )
//            {
//                return true;
//            }
//        }
//    }
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//ParseDirPackageMap( const CORE::CDataNode& rootNode ,
//                    TStringVectorMap& packageMap    )
//{GUCEF_TRACE;
//    const CDataNode* packageListRoot = rootNode.Find( "PackageList" );
//    if ( NULL == packageListRoot ) return false;
//    
//    CDataNode::TConstDataNodeSet packageNodes = packageListRoot->FindChildrenOfType( "Package" );
//    CDataNode::TConstDataNodeSet::iterator i = packageNodes.begin();
//    while ( i != packageNodes.end() )
//    {        
//        const CDataNode* packageNode = (*i);
//        
//        CORE::CString packageName = packageNode->GetAttributeValue( "name" );
//        TStringVector& dirVector = packageMap[ packageName ];
//        
//        CDataNode::TConstDataNodeSet dirNodes = packageNode->FindChildrenOfType( "Dir" );
//        CDataNode::TConstDataNodeSet::iterator n = dirNodes.begin();
//        while ( n != dirNodes.end() )
//        {
//            dirVector.push_back( (*n)->GetAttributeValue( "name" ); );            
//            ++n;
//        }
//        ++i;
//    }
//    return true;    
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//ParseDirPackageMap( const CString& packageDefinitionsFilePath ,
//                    TStringVectorMap& packageMap              )
//{GUCEF_TRACE;
//
//    CORE::CDataNode dataTree;
//    if ( LoadXmlFile( packageDefinitionsFilePath, dataTree ) )
//    {
//        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully loaded package definitions xml from: " + packageDefinitionsFilePath );
//        
//        if ( ParseDirPackageMap( dataTree, packageMap ) )
//        {
//            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully parsed " + CORE::UInt32ToString( packageMap.size() ) + " package definition lists" );
//            return true;
//        }        
//    }
//    else
//    {
//        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load package definitions from: " + packageDefinitionsFilePath );
//    }
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//GetPackagesUsedByTarget( const CString& destRootDir             ,
//                         const TStringVectorMap& sourcePackages ,
//                         TStringVectorMap& destPackageList      )
//{GUCEF_TRACE;
//
//    TStringVectorMap::const_iterator i = sourcePackages.begin();
//    while ( i != sourcePackageMap.end() )
//    {
//        CORE::CString destPackagePath = destRootDir;
//        CORE::AppendToPath( destPackagePath, (*i).first );
//        
//        if ( CORE::IsPathValid( destPackagePath ) )
//        {
//            // present and accounted for at destination location
//            destPackageList.push_back( (*i).first );
//        }
//        
//        ++i;
//    }
//        
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//GetListOfApplicablePackages( const CORE::CString& srcDir        ,
//                             const CString& destRootDir         ,
//                             TStringVectorMap& packageList      )
//{GUCEF_TRACE;
//
//    CORE::CString packageDefinitionsFilePath = srcDir;
//    CORE::AppendToPath( packageDefinitionsFilePath, "PackageDefinitions.xml" );
//    
//    TStringVectorMap sourcePackageMap;
//    if ( ParseDirPackageMap( packageDefinitionsFilePath, sourcePackageMap ) )
//    {
//        GetPackagesUsedByTarget( destDir, sourcePackageMap, destPackageList );
//        return true;
//    }
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//GetListOfApplicablePackageDirs( const CORE::CString& srcDir ,
//                                const CString& destRootDir  ,
//                                TStringVector& packagDirs   )
//{GUCEF_TRACE;
//
//    TStringVectorMap fullPackageList;
//    if ( GetListOfApplicablePackages( srcDir, destRootDir, fullPackageList ) )
//    {
//        TStringVectorMap::iterator i = fullPackageList.begin();
//        while ( i != fullPackageList.end() )
//        {
//            TStringVector& subList = (*i).second;
//            TStringVector::iterator n = subList.begin();
//            while ( n != subList.end() )
//            {
//                packagDirs.push_back( (*n) );                
//                ++n;
//            }            
//            ++i;
//        }
//        return true;
//    }
//    return false;
//}

/*-------------------------------------------------------------------------*/

void
BuildSourceFileList( const CORE::CString& srcDir    ,
                     TMatchEntryVector& matches     ,
                     const TStringSet* fileTypes    ,
                     const TStringSet* dirsToIgnore )
{GUCEF_TRACE;

    // get a list of all the files
    TFileEntryVector files;
    BuildFileList( srcDir, files, fileTypes, dirsToIgnore );
    
    // add entry for each file
    TFileEntryVector::iterator i = files.begin();
    while ( i != files.end() ) 
    {
        TMatchEntry matchEntry;
        matchEntry.source = (*i);        
        matches.push_back( matchEntry );        
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
BuildSourceFileList( const CORE::CString& srcDir                 ,
                     const TStringVector& listOfRootSubDirsToUse ,
                     TMatchEntryVector& matches                  ,
                     const TStringSet* fileTypes                 ,
                     const TStringSet* dirsToIgnore              )
{GUCEF_TRACE;

    // get a list of all the files
    TFileEntryVector files;
    BuildFileList( srcDir, files, fileTypes, dirsToIgnore );
    
    TStringVector::const_iterator i = listOfRootSubDirsToUse.begin();
    while ( i != listOfRootSubDirsToUse.end() )
    {
        CORE::CString subRoot = srcDir;
        CORE::AppendToPath( subRoot, (*i) );

        // get a list of all the files
        TFileEntryVector subFiles;
        BuildFileList( subRoot, subFiles, fileTypes, dirsToIgnore );
        
        // Fix relative paths to include rootdir
        TFileEntryVector::iterator n = subFiles.begin();
        while ( n != subFiles.end() )
        {
            // Put root dir prefix in place again to fix relative path
            CORE::CString actualRelPath = (*i);
            CORE::AppendToPath( actualRelPath, (*n).filedir );
            (*n).filedir = actualRelPath;
            
            ++n;
        }
        
        ++i;
    }
    
    // add entry for each file
    TFileEntryVector::iterator m = files.begin();
    while ( m != files.end() ) 
    {
        TMatchEntry matchEntry;
        matchEntry.source = (*m);        
        matches.push_back( matchEntry );        
        ++m;
    }
}

/*-------------------------------------------------------------------------*/

void
FindMatchesForFile( const TFileEntryVector& totalSet ,
                    const CORE::CString& filename    ,
                    TFileEntryVector& matches        )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Attempting to match file \"" + filename + "\"" );
    
    TFileEntryVector::const_iterator i = totalSet.begin();
    while ( i != totalSet.end() ) 
    {
        const TFileEntry& fileEntry = (*i);        
        if ( fileEntry.filename == filename )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully matched file \"" + filename + "\" to file at \"" + fileEntry.filedir + "\"" );
            matches.push_back( fileEntry );
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
FindMatchesForSourceFiles( TMatchEntryVector& matches        ,
                           const CORE::CString& destDir      ,
                           const TStringSet* fileTypes       ,
                           const TStringSet* dirsToIgnore    )
{GUCEF_TRACE;

    // get a list of all destination files
    TFileEntryVector destFiles;
    BuildFileList( destDir, destFiles, fileTypes, dirsToIgnore );
    
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Attempting to match " + CORE::UInt32ToString( matches.size() ) + " files to " + 
                                        CORE::UInt32ToString( destFiles.size() ) + " destination files" );
    
    // Find destination matches for each source file
    TMatchEntryVector::iterator i = matches.begin();
    while ( i != matches.end() ) 
    {
        TMatchEntry& matchEntry = (*i);
        
        FindMatchesForFile( destFiles                  ,
                            matchEntry.source.filename ,
                            matchEntry.destinations    );       
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
LocateFileMatches( const CORE::CString& srcDir       ,
                   const CORE::CString& destDir      , 
                   TMatchEntryVector& matches        ,
                   const TStringSet* fileTypes       ,
                   const TStringSet* dirsToIgnore    ,
                   bool usePackageDefinitions = true )
{GUCEF_TRACE;
    
    //if ( usePackageDefinitions )
    //{
    //    TStringVector packageDirs;
    //    if ( GetListOfApplicablePackageDirs( srcDir, destDir, packageDirs ) )
    //    {
    //        // Build source list taking into account that we only need/want the dirs
    //        // for the applicable packages
    //        BuildSourceFileList( srcDir, packageDirs, matches, fileTypes, dirsToIgnore );  
    //        
    //        // match source files to destination files
    //        FindMatchesForSourceFiles( matches, destDir, fileTypes, dirsToIgnore );
    //    }
    //}
    //else
    {
        // first build a list of source files
        BuildSourceFileList( srcDir, matches, fileTypes, dirsToIgnore );

        // match source files to destination files
        FindMatchesForSourceFiles( matches, destDir, fileTypes, dirsToIgnore );
    }
}

/*-------------------------------------------------------------------------*/

bool
DeleteFileBackup( const CORE::CString& filepath )
{GUCEF_TRACE;

    CORE::CString newFilepath = filepath + ".backup";
    if ( CORE::FileExists( newFilepath ) )
    {
        if ( CORE::DeleteFile( newFilepath ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "File \"" + newFilepath + "\" has been deleted" );
            return true;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to delete file \"" + newFilepath + "\"" );
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
UndoRenameFileAsBackup( const CORE::CString& filepath )
{GUCEF_TRACE;

    CORE::CString newFilepath = filepath + ".backup";
     
    if ( CORE::MoveFile( newFilepath, filepath, true ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "File \"" + filepath + "\" has been renamed to \"" + newFilepath + "\"" );
        return true;
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to rename file \"" + filepath + "\" to \"" + newFilepath + "\"" );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
RenameFileAsBackup( const CORE::CString& filepath )
{GUCEF_TRACE;

    CORE::CString newFilepath = filepath + ".backup";     
    if ( CORE::MoveFile( filepath, newFilepath, true ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "File \"" + filepath + "\" has been renamed to \"" + newFilepath + "\"" );
        return true;
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to rename file \"" + filepath + "\" to \"" + newFilepath + "\"" );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CopyOverMatchedFiles( const CORE::CString& srcDirRoot  ,
                      const CORE::CString& destDirRoot ,
                      TMatchEntryVector& matches       )
{GUCEF_TRACE;

    TMatchEntryVector::iterator i = matches.begin();
    while ( i != matches.end() )
    {
        TMatchEntry& matchEntry = (*i);
        
        CORE::CString sourceFilePath = srcDirRoot;
        CORE::AppendToPath( sourceFilePath, matchEntry.source.filedir );
        CORE::AppendToPath( sourceFilePath, matchEntry.source.filename );
        
        TFileEntryVector& destinations = matchEntry.destinations;

        if ( destinations.empty() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "File \"" + sourceFilePath + "\" cannot be copied since it doesnt have any destination locations" );
        }
        else
        { 
            TFileEntryVector::iterator n = destinations.begin();
            while ( n != destinations.end() )
            {
                TFileEntry& destEntry = (*n);
                
                CORE::CString destFilePath = destDirRoot;
                CORE::AppendToPath( destFilePath, destEntry.filedir );
                CORE::AppendToPath( destFilePath, destEntry.filename );
                
                // Delete whatever backup file might have gotten left behind from a previous run                
                DeleteFileBackup( destFilePath );
                
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Preparing to copy file from \"" + sourceFilePath + "\" to \"" + destFilePath + "\"" );            
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Backing up destination file \"" + destFilePath + "\"" ); 
                if ( RenameFileAsBackup( destFilePath ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully renamed destination file \"" + destFilePath + "\"" );
                    
                    if ( 0 != CORE::Copy_File( destFilePath.C_String()   , 
                                               sourceFilePath.C_String() ) )
                    {
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully copied file from \"" + sourceFilePath + "\" to \"" + destFilePath + "\"" );
                        DeleteFileBackup( destFilePath );
                    }
                    else
                    {
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Failed to copy file from \"" + sourceFilePath + "\" to \"" + destFilePath + "\"" );
                        UndoRenameFileAsBackup( destFilePath );
                    }
                }
                else
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Failed to delete destination file \"" + destFilePath + "\"" );
                }
                
                ++n;
            }
        }        
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
MirrorExistingDirsAndFiles( const CORE::CString& srcDir    ,
                            const CORE::CString& dstDir    ,
                            const TStringSet* fileTypes    ,
                            const TStringSet* dirsToIgnore )
{
    // Get a list of all destination files
    TFileEntryVector files;
    BuildFileList( dstDir       ,
                   files        ,
                   fileTypes    ,
                   dirsToIgnore );

    // Iterate the list of files and make sure we mirror all of them
    TFileEntryVector::iterator i = files.begin();
    while ( i != files.end() )
    {
        TFileEntry& fileEntry = (*i);

        CORE::CString srcFilePath = srcDir;
        CORE::AppendToPath( srcFilePath, fileEntry.filedir );
        CORE::AppendToPath( srcFilePath, fileEntry.filename );
        
        CORE::CString destFilePath = dstDir;
        CORE::AppendToPath( destFilePath, fileEntry.filedir );
        CORE::AppendToPath( destFilePath, fileEntry.filename );
        
        if ( !CORE::FileExists( srcFilePath ) )
        {
            // Skip this one, we are configured to only mirror files for which 
            // a target file already exists
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Skipping file \"" + destFilePath + "\" because there is no source file available" );
            ++i;
            continue;
        }

        if ( !RenameFileAsBackup( destFilePath ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Unable to rename existing destination file: " + destFilePath );
            ++i;
            continue;
        }
            
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully renamed destination file \"" + destFilePath + "\"" );
            
        if ( 0 != CORE::Copy_File( destFilePath.C_String()  , 
                                   srcFilePath.C_String() ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully copied file from \"" + srcFilePath + "\" to \"" + destFilePath + "\"" ); 
            if ( DeleteFileBackup( destFilePath ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully deleted backup file after successfull copy for file \"" + destFilePath + "\"" );
            }
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Failed to copy file from \"" + srcFilePath + "\" to \"" + destFilePath + "\"" );
            if ( !UndoRenameFileAsBackup( destFilePath ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to undo renaming of file: " + destFilePath );
            }
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
MirrorDirsAndFiles( const CORE::CString& srcDir    ,
                    const CORE::CString& dstDir    ,
                    bool mirrorExistingFilesOnly   ,
                    const TStringSet* fileTypes    ,
                    const TStringSet* dirsToIgnore )
{GUCEF_TRACE;

    if ( mirrorExistingFilesOnly )    
    {
        // We have a seperare function for mirroring existing files only
        // It is more efficient since it can use the destination as the template
        // avoiding unnessary operations
        MirrorExistingDirsAndFiles( srcDir       ,
                                    dstDir       ,
                                    fileTypes    ,
                                    dirsToIgnore );
        return;
    }

    // Get a list of all source files
    TFileEntryVector files;
    BuildFileList( srcDir       ,
                   files        ,
                   fileTypes    ,
                   dirsToIgnore );

    // Iterate the list of files and make sure we mirror all of them
    TFileEntryVector::iterator i = files.begin();
    while ( i != files.end() )
    {
        TFileEntry& fileEntry = (*i);

        CORE::CString srcFilePath = srcDir;
        CORE::AppendToPath( srcFilePath, fileEntry.filedir );
        CORE::AppendToPath( srcFilePath, fileEntry.filename );
        
        CORE::CString destFilePath = dstDir;
        CORE::AppendToPath( destFilePath, fileEntry.filedir );
        
        // Make sure destination directory exists
        if ( CORE::CreateDirs( destFilePath ) )
        {
            CORE::AppendToPath( destFilePath, fileEntry.filename );
            
            bool fileExisted = CORE::FileExists( destFilePath );                        
            if ( fileExisted )
            {
                if ( !RenameFileAsBackup( destFilePath ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Unable to rename existing destination file: " + destFilePath );
                    ++i;
                    continue;
                }
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully renamed destination file \"" + destFilePath + "\"" );
            }
                        
            if ( 0 != CORE::Copy_File( destFilePath.C_String()  , 
                                       srcFilePath.C_String() ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully copied file from \"" + srcFilePath + "\" to \"" + destFilePath + "\"" );
                if ( fileExisted )
                {
                    if ( DeleteFileBackup( destFilePath ) )
                    {
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully deleted backup file after successfull copy for file \"" + destFilePath + "\"" );
                    }
                }
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Failed to copy file from \"" + srcFilePath + "\" to \"" + destFilePath + "\"" );
                if ( fileExisted )
                {
                    if ( !UndoRenameFileAsBackup( destFilePath ) )
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to undo renaming of file: " + destFilePath );
                    }
                }
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Unable to create directory: " + destFilePath );
        }
        ++i;
    }
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

/*-------------------------------------------------------------------------*/

void
PrintHeader( void )
{GUCEF_TRACE;

    cls();
    printf( "*****************************************\n" );
    printf( "*                                       *\n" );
    printf( "*  Reference Updater                    *\n" );
    printf( "*                                       *\n" );
    printf( "*****************************************\n" );
    printf( "\n" );
    printf( " - Tool Version %f\n" , VERSION_NUMBER );    
}

/*-------------------------------------------------------------------------*/

void
PrintHelp( void )
{GUCEF_TRACE;

    printf( "\n" );
    printf( " - Program arguments:\n" );
    printf( "  Arguments should be passed in the form \'key=value\'\n" );
    printf( "    'SrcIncludeDir' : path to the source dir with the includes\n" );
    printf( "    'DstIncludeDir' : path to the destination dir with the includes\n" );
    printf( "    'SrcBinDir'     : path to the source dir with the binaries\n" );
    printf( "    'DestBinDir'    : path to the destination dir with the binaries\n" );
    printf( "    'DirsToIgnore'  : optional param of ; seperated directory names\n" );
    printf( "                      of directories that should be ignored\n" );
}

/*-------------------------------------------------------------------------*/

TStringSet
VectorToSet( const TStringVector& src )
{GUCEF_TRACE;

    TStringSet set;
    TStringVector::const_iterator i = src.begin();
    while ( i != src.end() )
    {
        set.insert( (*i) );
        ++i;
    }
    return set;
}

/*-------------------------------------------------------------------------*/

CORE::CString
GetUniqueFileName( const CORE::CString& filename )
{GUCEF_TRACE;

    CORE::CString fileExt = CORE::ExtractFileExtention( filename );
    CORE::CString fileWithoutExt;

    if ( fileExt.IsNULLOrEmpty() )
    {
        fileWithoutExt = filename;
    }
    else
    {
        fileWithoutExt = filename.CutChars( fileExt.Length()+1, false );
    }

    CORE::UInt32 i = 2;
    CORE::CString filenameToTest = filename;
    while ( CORE::FileExists( filenameToTest ) )
    {
        filenameToTest = fileWithoutExt + '(' + CORE::UInt32ToString( i ) + ")." + fileExt;
        ++i;
    }

    return filenameToTest;
}

/*-------------------------------------------------------------------------*/

GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    CORE::CString logFilename = CORE::RelativePath( "$CURWORKDIR$" );
    CORE::AppendToPath( logFilename, "ReferenceUpdater_Log.txt" );
    logFilename = GetUniqueFileName( logFilename );
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
	                 
        // Check if we have at least 1 of the two pairs
        if ( ( !argList.HasKey( "SrcIncludeDir" )  ||
               !argList.HasKey( "DestIncludeDir" )  ) &&
             ( !argList.HasKey( "SrcBinDir" )      ||
               !argList.HasKey( "DestBinDir" )      ) )
        {
            printf( "ERROR: Not enough parameters were provided\n\n" );
            PrintHelp();
            getchar();
            return 1;
        }
        
        CORE::CString srcIncludeDir = argList.GetValueAlways( "SrcIncludeDir" );
        CORE::CString destIncludeDir = argList.GetValueAlways( "DestIncludeDir" );
        CORE::CString srcBinDir = argList.GetValueAlways( "SrcBinDir" );
        CORE::CString destBinDir = argList.GetValueAlways( "DestBinDir" );
        TStringSet dirsToIgnore = VectorToSet( argList.GetValueAlways( "DirsToIgnore" ).Lowercase().ParseElements( ';', false ) );
        CORE::CString mirrorExistingFilesOnlyStr = argList.GetValueAlways( "MirrorExistingFilesOnly" );
        bool mirrorExistingFilesOnly = true;
        if ( !mirrorExistingFilesOnlyStr.IsNULLOrEmpty() )
        {
            mirrorExistingFilesOnly = CORE::StringToBool( mirrorExistingFilesOnlyStr );
        }
                
        if ( !srcIncludeDir.IsNULLOrEmpty() && !destIncludeDir.IsNULLOrEmpty() )
        {
            // match the includes
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Mirroring the include files and their directory structure" );
            MirrorDirsAndFiles( srcIncludeDir           , 
                                destIncludeDir          , 
                                mirrorExistingFilesOnly ,
                                &GetIncludeFileTypes()  ,
                                &dirsToIgnore           ); 
        }
                
        if ( !srcBinDir.IsNULLOrEmpty() && !destBinDir.IsNULLOrEmpty() )
        {
            // mirror the binaries
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Locating file matches for the binary files" );
            TMatchEntryVector binaryMatches;
            LocateFileMatches( srcBinDir, destBinDir, binaryMatches, &GetBinaryFileTypes(), &dirsToIgnore );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Matched " + CORE::UInt32ToString( binaryMatches.size() ) + " binary files" );

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Commencing copy of binary files,..." );
            CopyOverMatchedFiles( srcBinDir, destBinDir, binaryMatches );
        }
        
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Finished all application operations" );
    }
    return 0;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------*/
