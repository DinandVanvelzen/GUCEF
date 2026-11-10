/*
 *  ArchiveDiffLib: library with some archive diff util functions
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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#include "ArchiveDiffLib.h"

/*-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace ARCHIVEDIFF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define VERSION_NUMBER  1.1

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef std::set< CORE::CString > TStringSet;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
InitializeFileStatus( TFileStatus& fileStatus )
{GUCEF_TRACE;

    fileStatus.resourceState = RESOURCESTATE_UNKNOWN;

    fileStatus.templateArchiveInfo.name.Clear();
    fileStatus.templateArchiveInfo.hash.Clear();
    fileStatus.templateArchiveInfo.sizeInBytes = 0;
    fileStatus.templateArchiveInfo.fileLocations.clear();

    fileStatus.mainSvnArchiveInfo.name.Clear();
    fileStatus.mainSvnArchiveInfo.hash.Clear();
    fileStatus.mainSvnArchiveInfo.sizeInBytes = 0;
    fileStatus.mainSvnArchiveInfo.fileLocations.clear();
}

/*---------------------------------------------------------------------------*/

CORE::CString
CreatePathToUnchangedFileDiff( const CORE::CString& dir )
{GUCEF_TRACE;

    CORE::CString path = dir;
    CORE::AppendToPath( path, "UnchangedFiles.xml" );
    return path;
}

/*---------------------------------------------------------------------------*/

CORE::CString
CreatePathToChangedFileDiff( const CORE::CString& dir )
{GUCEF_TRACE;

    CORE::CString path = dir;
    CORE::AppendToPath( path, "ChangedFiles.xml" );
    return path;
}

/*---------------------------------------------------------------------------*/

CORE::CString
CreatePathToAddedFileDiff( const CORE::CString& dir )
{GUCEF_TRACE;

    CORE::CString path = dir;
    CORE::AppendToPath( path, "AddedFiles.xml" );
    return path;
}

/*---------------------------------------------------------------------------*/

CORE::CString
CreatePathToMissingFileDiff( const CORE::CString& dir )
{GUCEF_TRACE;

    CORE::CString path = dir;
    CORE::AppendToPath( path, "MissingFiles.xml" );
    return path;

}

/*---------------------------------------------------------------------------*/

CORE::CDStoreCodecRegistry::TDStoreCodecPtr
GetXmlDStoreCodec( void )
{GUCEF_TRACE;

    static CORE::CDStoreCodecRegistry::TDStoreCodecPtr codecPtr;
    if ( codecPtr.IsNULL() )
    {
        CORE::CDStoreCodecRegistry* registry = &CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry();
        if ( !registry->TryLookup( "XML", codecPtr, false ) )
        {
            // No codec is registered to handle XML, try and load a plugin for it
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

              #ifdef GUCEF_CORE_DEBUG_MODE
              const char* pathToPlugin = "$MODULEDIR$/dstorepluginPARSIFALXML_d";
              #else
              const char* pathToPlugin = "$MODULEDIR$/dstorepluginPARSIFALXML";
              #endif

            if ( !CORE::CCoreGlobal::Instance()->GetPluginControl().AddPluginFromDir( pathToPlugin    ,
                                                                                      CORE::CString() ,
                                                                                      true            ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Unable to load plugin from " + CORE::CString( pathToPlugin ) );
                return CORE::CDStoreCodecRegistry::TDStoreCodecPtr();
            }

            #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )

              #ifdef GUCEF_CORE_DEBUG_MODE
              const char* pathToPlugin = "$MODULEDIR$/dstorepluginPARSIFALXML_d";
              #else
              const char* pathToPlugin = "$MODULEDIR$/dstorepluginPARSIFALXML";
              #endif

            if ( !CORE::CCoreGlobal::Instance()->GetPluginControl().AddPluginFromDir( pathToPlugin    ,
                                                                                      CORE::CString() ,
                                                                                      true            ) )
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Unable to load plugin from " + CORE::CString( pathToPlugin ) + " attempting alternate location" );

                #ifdef GUCEF_CORE_DEBUG_MODE
                const char* pathToPlugin = "$MODULEDIR$/../lib/dstorepluginPARSIFALXML_d";
                #else
                const char* pathToPlugin = "$MODULEDIR$/../lib/dstorepluginPARSIFALXML";
                #endif

                if ( !CORE::CCoreGlobal::Instance()->GetPluginControl().AddPluginFromDir( pathToPlugin    ,
                                                                                          CORE::CString() ,
                                                                                          true            ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Unable to load plugin from " + CORE::CString( pathToPlugin ) );
                }
            }

            #else

            // Plugin loading not supported
            GUCEF_ERROR_LOG( CORE::LOGVEL_NORMAL, "Plugin loading is not supported for this platform via the ProjectGenerator" );

            #endif

            // Now try and get the codec again
            if ( registry->TryLookup( "XML", codecPtr, false ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Request for data storage codec for xml file, succesfully loaded plugin to handle request" );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Request for data storage codec for xml file but no plugin for it could be loaded!" );
                CORE::ShowErrorMessage( "Missing codec support",
                                        "Request for data storage codec for xml file but no plugin for it could be loaded!" );
            }
        }
    }
    return codecPtr;
}

/*---------------------------------------------------------------------------*/

void
BuildSortedFileEntryMaps( PATCHER::CPatchSetParser::TFileEntry& patchsetFileEntry ,
                          TSortedFileEntryMaps& sortedFileMaps                    ,
                          const CORE::CString& relativePath                       )
{GUCEF_TRACE;

    CORE::CString relFilePath = relativePath;
    CORE::AppendToPath( relFilePath, patchsetFileEntry.name );

    // Add the relative path to the file as a file location
    // If we put the entry in a flat sorted list we need to still be able to know
    // where the entry came from
    PATCHER::CPatchSetParser::TFileLocation diskLocation;
    diskLocation.codec = "FILE";
    diskLocation.URL = relFilePath;
    patchsetFileEntry.fileLocations.push_back( diskLocation );

    sortedFileMaps.hashMap[ patchsetFileEntry.hash ].insert( &patchsetFileEntry );
    sortedFileMaps.nameMap[ patchsetFileEntry.name ].insert( &patchsetFileEntry );

    GUCEF_LOG( CORE::LOGLEVEL_EVERYTHING, "Added relative path file location to file entry and added entry to sorting maps for file at " + relFilePath );

}

/*---------------------------------------------------------------------------*/

void
BuildSortedFileEntryMaps( PATCHER::CPatchSetParser::TDirEntry& patchsetDirEntry ,
                          TSortedFileEntryMaps& sortedFileMaps                  ,
                          const CORE::CString& relativePath                     )
{GUCEF_TRACE;

    PATCHER::CPatchSetParser::TFileEntryList::iterator i = patchsetDirEntry.files.begin();
    while ( i != patchsetDirEntry.files.end() )
    {
        BuildSortedFileEntryMaps( (*i)           ,
                                  sortedFileMaps ,
                                  relativePath   );

        ++i;
    }

    PATCHER::CPatchSetParser::TPatchSet::iterator n = patchsetDirEntry.subDirs.begin();
    while ( n != patchsetDirEntry.subDirs.end() )
    {
        CORE::CString subDirPath = relativePath;
        CORE::AppendToPath( subDirPath, (*n).name );

        BuildSortedFileEntryMaps( (*n)           ,
                                  sortedFileMaps ,
                                  subDirPath     );

        ++n;
    }
}

/*---------------------------------------------------------------------------*/

bool
BuildSortedFileEntryMaps( PATCHER::CPatchSetParser::TPatchSet& patchSet ,
                          TSortedFileEntryMaps& sortedFileMaps          ,
                          const CORE::CString& relativePath             )
{GUCEF_TRACE;

    PATCHER::CPatchSetParser::TPatchSet::iterator i = patchSet.begin();
    while ( i != patchSet.end() )
    {
        CORE::CString subDirPath = relativePath;
        CORE::AppendToPath( subDirPath, (*i).name );

        BuildSortedFileEntryMaps( (*i)           ,
                                  sortedFileMaps ,
                                  subDirPath     );

        ++i;
    }
    return true;
}

/*---------------------------------------------------------------------------*/

bool
BuildSortedFileEntryMaps( PATCHER::CPatchSetParser::TPatchSet& patchSet ,
                          TSortedFileEntryMaps& sortedFileMaps          )
{GUCEF_TRACE;

    return BuildSortedFileEntryMaps( patchSet        ,
                                     sortedFileMaps  ,
                                     CORE::CString() );
}

/*---------------------------------------------------------------------------*/

void
SetPathForFile( GUCEF::PATCHER::CPatchSetParser::TFileEntry& fileEntry ,
                const GUCEF::CORE::CString& path                       )
{GUCEF_TRACE;

    // See if we can overwrite an existing entry
    PATCHER::CPatchSetParser::TFileLocations::iterator i = fileEntry.fileLocations.begin();
    while ( i != fileEntry.fileLocations.end() )
    {
        PATCHER::CPatchSetParser::TFileLocation& location = (*i);
        if ( "FILE" == location.codec )
        {
            location.URL = path;
            return;
        }
        ++i;
    }

    // no entry found, create one
    PATCHER::CPatchSetParser::TFileLocation fileLocation;
    fileLocation.codec = "FILE";
    fileLocation.URL = path;
    fileEntry.fileLocations.push_back( fileLocation );
}

/*---------------------------------------------------------------------------*/

CORE::CString
GetPathForFile( const PATCHER::CPatchSetParser::TFileEntry& fileEntry )
{GUCEF_TRACE;

    PATCHER::CPatchSetParser::TFileLocations::const_iterator i = fileEntry.fileLocations.begin();
    while ( i != fileEntry.fileLocations.end() )
    {
        const PATCHER::CPatchSetParser::TFileLocation& location = (*i);
        if ( "FILE" == location.codec )
        {
            return location.URL;
        }
        ++i;
    }
    return CORE::CString();
}

/*-------------------------------------------------------------------------*/

const char*
ResourceStateToString( TResourceState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case RESOURCESTATE_FILE_UNCHANGED : return "Unchanged";
        case RESOURCESTATE_FILE_UNCHANGED_BUT_MOVED : return "UnchangedButMoved";
        case RESOURCESTATE_FILE_CHANGED : return "Changed";
        case RESOURCESTATE_FILE_MISSING_IN_MAIN : return "MissingInMain";
        case RESOURCESTATE_FILE_MISSING_IN_TEMPLATE : return "MissingInTemplate";
        default:
        {
            return "Unknown";
        }
    }
}

/*-------------------------------------------------------------------------*/

TResourceState
StringToResourceState( const CORE::CString& state )
{GUCEF_TRACE;

    if ( state.Equals( "Unchanged", false ) ) return RESOURCESTATE_FILE_UNCHANGED;
    if ( state.Equals( "UnchangedButMoved", false ) ) return RESOURCESTATE_FILE_UNCHANGED_BUT_MOVED;
    if ( state.Equals( "Changed", false ) ) return RESOURCESTATE_FILE_CHANGED;
    if ( state.Equals( "MissingInMain", false ) ) return RESOURCESTATE_FILE_MISSING_IN_MAIN;
    if ( state.Equals( "MissingInTemplate", false ) ) return RESOURCESTATE_FILE_MISSING_IN_TEMPLATE;
    if ( state.Equals( "Unknown", false ) ) return RESOURCESTATE_UNKNOWN;

    return RESOURCESTATE_UNKNOWN;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GetPathForFileMinusRoot( const PATCHER::CPatchSetParser::TFileEntry& fileEntry )
{GUCEF_TRACE;

    CORE::CString filePath = GetPathForFile( fileEntry );
    CORE::Int32 firstSlashIndex = filePath.HasChar( '\\' );
    CORE::CString filePathMinuxRoot;
    if ( firstSlashIndex < 0 )
    {
        return filePath;
    }
    else
    {
        return filePath.CutChars( firstSlashIndex+1, true );
    }
}

/*---------------------------------------------------------------------------*/

bool
PerformArchiveDiffTemplateToMain( const PATCHER::CPatchSetParser::TFileEntry& templateFileEntry ,
                                  const TSortedFileEntryMaps& mainPatchsetSearchMap             ,
                                  TFileStatusVector& fileStatusList                             ,
                                  TConstFileEntrySet& matchedEntriesInMain                      )
{GUCEF_TRACE;

    CORE::CString pathOfEntry = GetPathForFileMinusRoot( templateFileEntry );

    TFileEntryPtrMap::const_iterator i = mainPatchsetSearchMap.hashMap.find( templateFileEntry.hash );
    if ( i != mainPatchsetSearchMap.hashMap.end() )
    {
        const TFileEntrySet& entries = (*i).second;
        TFileEntrySet::const_iterator n = entries.begin();
        while ( n != entries.end() )
        {
            const PATCHER::CPatchSetParser::TFileEntry* otherEntry = (*n);

            // We already know any entry here will have the same hash.
            // what else do they have in common?
            CORE::CString pathOfOtherEntry = GetPathForFileMinusRoot( *otherEntry );
            if ( pathOfOtherEntry.Equals( pathOfEntry, false ) )
            {
                // The files have the same path & hash
                // We can say with some measure of certainty that this file was unchanged
                TFileStatus fileStatus;
                InitializeFileStatus( fileStatus );
                fileStatus.resourceState = RESOURCESTATE_FILE_UNCHANGED;
                fileStatus.templateArchiveInfo = templateFileEntry;
                fileStatus.mainSvnArchiveInfo = *otherEntry;

                // Patch files include the root dir, diff files should not
                SetPathForFile( fileStatus.templateArchiveInfo, pathOfEntry );
                SetPathForFile( fileStatus.mainSvnArchiveInfo, pathOfOtherEntry );

                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Determined that file \"" + pathOfOtherEntry + "\" was unchanged" );

                matchedEntriesInMain.insert( otherEntry );
                fileStatusList.push_back( fileStatus );
                return true;
            }

            ++n;
        }

        // If we get here then although files where found with the same hash they do not have the same path
        // So perhaps the file was moved but not changed. Note that if there are multiple entries with the same
        // hash this won't suffice since then the risk is too great that we might confuse different files.
        if ( entries.size() == 1 )
        {
            // Since there is only 1 file with the same hash and we can assume with fair certainty that
            // this is the same file but it was moved (including renames)
            const PATCHER::CPatchSetParser::TFileEntry* otherEntry = (*entries.begin());
            TFileStatus fileStatus;
            InitializeFileStatus( fileStatus );
            fileStatus.resourceState = RESOURCESTATE_FILE_UNCHANGED_BUT_MOVED;
            fileStatus.templateArchiveInfo = templateFileEntry;
            fileStatus.mainSvnArchiveInfo = *otherEntry;

            // Patch files include the root dir, diff files should not
            SetPathForFile( fileStatus.templateArchiveInfo, pathOfEntry );
            SetPathForFile( fileStatus.mainSvnArchiveInfo, GetPathForFileMinusRoot( *otherEntry ) );

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Determined that file \"" + pathOfEntry + "\" was unchanged but its path changed from " + GetPathForFileMinusRoot( *otherEntry ) );

            matchedEntriesInMain.insert( otherEntry );
            fileStatusList.push_back( fileStatus );
            return true;
        }
        else
        {
            // We can use 1 other criterea, what if there are multiple entries with the same hash
            // all at different locations then the original, but what if only 1 has the same name as
            // the original? The odds are then pretty good that this is a positive match
            const PATCHER::CPatchSetParser::TFileEntry* entryWithSameName = NULL;
            CORE::UInt32 matchCount = 0;
            while ( n != entries.end() )
            {
                const PATCHER::CPatchSetParser::TFileEntry* otherEntry = (*n);

                // We already know any entry here will have the same hash.
                // We also know they won't have the same path. We want to find exactly 1 name match
                if ( templateFileEntry.name == otherEntry->name )
                {
                    if ( 0 == matchCount )
                    {
                        ++matchCount;
                        entryWithSameName = otherEntry;
                    }
                    else
                    {
                        // Too many matches, this isnt going to work
                        // no point in continueing
                        ++matchCount;
                        break;
                    }
                }
                ++n;
            }

            // remember we only want 1 match
            if ( 1 == matchCount )
            {
                // Yay, we found a file that has the same hash and although there where multiple
                // files for which this was true only 1 had the same name so we found our file
                TFileStatus fileStatus;
                InitializeFileStatus( fileStatus );
                fileStatus.resourceState = RESOURCESTATE_FILE_UNCHANGED_BUT_MOVED;
                fileStatus.templateArchiveInfo = templateFileEntry;
                fileStatus.mainSvnArchiveInfo = *entryWithSameName;

                // Patch files include the root dir, diff files should not
                SetPathForFile( fileStatus.templateArchiveInfo, pathOfEntry );
                SetPathForFile( fileStatus.mainSvnArchiveInfo, GetPathForFileMinusRoot( *entryWithSameName ) );

                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Determined that file \"" + pathOfEntry + "\" was unchanged but its path changed from " + GetPathForFileMinusRoot( *entryWithSameName ) );

                matchedEntriesInMain.insert( entryWithSameName );
                fileStatusList.push_back( fileStatus );
                return true;
            }
        }
    }

    // If we get here then it's going to be harder to find a match
    // Aparently using hash equavalancy did not yield a match so if the file is in the other
    // archive then it must have been changed.
    i = mainPatchsetSearchMap.nameMap.find( templateFileEntry.name );
    if ( i != mainPatchsetSearchMap.nameMap.end() )
    {
        // We know that all entries in here will be files with the same name,
        // we also know we can't count on the hash to help us.
        // We can grab onto path equavalancy,..
        // If the file was altered but kept in the same location then we can still find it
        const TFileEntrySet& entries = (*i).second;
        TFileEntrySet::const_iterator n = entries.begin();
        while ( n != entries.end() )
        {
            const PATCHER::CPatchSetParser::TFileEntry* otherEntry = (*n);
            CORE::CString pathToOtherFile = GetPathForFileMinusRoot( *otherEntry );

            if ( pathOfEntry.Equals( pathToOtherFile, false ) )
            {
                // found altered file in same location, bingo
                TFileStatus fileStatus;
                InitializeFileStatus( fileStatus );
                fileStatus.resourceState = RESOURCESTATE_FILE_CHANGED;
                fileStatus.templateArchiveInfo = templateFileEntry;
                fileStatus.mainSvnArchiveInfo = *otherEntry;

                // Patch files include the root dir, diff files should not
                SetPathForFile( fileStatus.templateArchiveInfo, pathOfEntry );
                SetPathForFile( fileStatus.mainSvnArchiveInfo, pathToOtherFile );

                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Determined that file \"" + pathOfEntry + "\" was changed" );

                matchedEntriesInMain.insert( otherEntry );
                fileStatusList.push_back( fileStatus );
                return true;
            }

            ++n;
        }

        // What if there is only 1 file in the other archive with the same name?
        // If thats the case then we might be able to guess that this is in fact the same file
        // but it was moved and altered,.. lets hope they used unique names...
        if ( 1 == entries.size() )
        {
            // Few that was a close one, we have a match
            const PATCHER::CPatchSetParser::TFileEntry* otherEntry = (*entries.begin());

            TFileStatus fileStatus;
            InitializeFileStatus( fileStatus );
            fileStatus.resourceState = RESOURCESTATE_FILE_CHANGED;
            fileStatus.templateArchiveInfo = templateFileEntry;
            fileStatus.mainSvnArchiveInfo = *otherEntry;

            // Patch files include the root dir, diff files should not
            SetPathForFile( fileStatus.templateArchiveInfo, pathOfEntry );
            SetPathForFile( fileStatus.mainSvnArchiveInfo, GetPathForFileMinusRoot( *otherEntry ) );

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Determined that file \"" + pathOfEntry + "\" was changed and moved from " + GetPathForFileMinusRoot( *otherEntry ) );

            matchedEntriesInMain.insert( otherEntry );
            fileStatusList.push_back( fileStatus );
            return true;
        }

    }

    // If we get here then we are out of ideas on how to match files and this one will
    // need human intervention
    TFileStatus fileStatus;
    InitializeFileStatus( fileStatus );
    fileStatus.resourceState = RESOURCESTATE_UNKNOWN;
    fileStatus.templateArchiveInfo = templateFileEntry;

    // Patch files include the root dir, diff files should not
    SetPathForFile( fileStatus.templateArchiveInfo, pathOfEntry );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Unable to match file \"" + pathOfEntry + "\", its status is unknown" );

    fileStatusList.push_back( fileStatus );
    return true;
}

/*---------------------------------------------------------------------------*/

bool
PerformArchiveDiffTemplateToMain( const TSortedFileEntryMaps& templateArchivePatchsetSearchMap ,
                                  const TSortedFileEntryMaps& mainArchivePatchsetSearchMap     ,
                                  TFileStatusVector& fileStatusList                            ,
                                  TConstFileEntrySet& matchedEntriesInMain                     )
{GUCEF_TRACE;

    TFileEntryPtrMap::const_iterator i = templateArchivePatchsetSearchMap.hashMap.begin();
    while ( i != templateArchivePatchsetSearchMap.hashMap.end() )
    {
        const TFileEntrySet& entries = (*i).second;
        TFileEntrySet::const_iterator n = entries.begin();
        while ( n != entries.end() )
        {
            PerformArchiveDiffTemplateToMain( *(*n)                        ,
                                              mainArchivePatchsetSearchMap ,
                                              fileStatusList               ,
                                              matchedEntriesInMain         );
            ++n;
        }
        ++i;
    }
    return true;
}

/*---------------------------------------------------------------------------*/

void
BuildListOfUnmatchedItemsInMain( const TSortedFileEntryMaps& mainArchivePatchsetSearchMap ,
                                 const TConstFileEntrySet& matchedEntriesInMain           ,
                                 TConstFileEntrySet& unmatchedEntriesInMain               )
{GUCEF_TRACE;

    TFileEntryPtrMap::const_iterator i = mainArchivePatchsetSearchMap.hashMap.begin();
    while ( i != mainArchivePatchsetSearchMap.hashMap.end() )
    {
        const TFileEntrySet& entries = (*i).second;
        TFileEntrySet::const_iterator n = entries.begin();
        while ( n != entries.end() )
        {
            const PATCHER::CPatchSetParser::TFileEntry* entryInMain = (*n);

            if ( matchedEntriesInMain.find( entryInMain ) == matchedEntriesInMain.end() )
            {
                // This entry is not in the list of items in main that where matched up against
                // a template version. As such we will add it to the list of unmatched items
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Determined that file \"" + GetPathForFileMinusRoot( *entryInMain ) + "\" was not matched against a template file" );
                unmatchedEntriesInMain.insert( entryInMain );
            }
            ++n;
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

bool
PerformArchiveDiffMainToTemplate( const TSortedFileEntryMaps& templateArchivePatchsetSearchMap ,
                                  const TSortedFileEntryMaps& mainArchivePatchsetSearchMap     ,
                                  TFileStatusVector& fileStatusList                            ,
                                  const TConstFileEntrySet& unmatchedEntriesInMain             )
{GUCEF_TRACE;

    // For now just add unmatched items in main as 'unknown' status
    // We can come up with something smart to do for those some other time
    TConstFileEntrySet::const_iterator i = unmatchedEntriesInMain.begin();
    while ( i != unmatchedEntriesInMain.end() )
    {
        TFileStatus fileStatus;
        InitializeFileStatus( fileStatus );
        fileStatus.resourceState = RESOURCESTATE_UNKNOWN;
        fileStatus.mainSvnArchiveInfo = *(*i);

        // Patch files include the root dir, diff files should not
        SetPathForFile( fileStatus.mainSvnArchiveInfo, GetPathForFileMinusRoot( fileStatus.mainSvnArchiveInfo ) );

        fileStatusList.push_back( fileStatus );

        ++i;
    }

    return true;
}

/*---------------------------------------------------------------------------*/

bool
PerformArchiveDiff( PATCHER::CPatchSetParser::TPatchSet& templatePatchset    ,
                    PATCHER::CPatchSetParser::TPatchSet& mainArchivePatchset ,
                    TFileStatusVector& fileStatusList                        )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Commencing creation of a sorted map for the template patch set" );

    TSortedFileEntryMaps templateArchivePatchsetSearchMap;
    BuildSortedFileEntryMaps( templatePatchset, templateArchivePatchsetSearchMap );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Sucessfully finished creation of a sorted map for the template patch set" );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Commencing creation of a sorted map for the main patch set" );

    TSortedFileEntryMaps mainArchivePatchsetSearchMap;
    BuildSortedFileEntryMaps( mainArchivePatchset, mainArchivePatchsetSearchMap );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Sucessfully finished creation of a sorted map for the template patch set" );

    TConstFileEntrySet matchedEntriesInMain;
    if ( PerformArchiveDiffTemplateToMain( templateArchivePatchsetSearchMap ,
                                           mainArchivePatchsetSearchMap     ,
                                           fileStatusList                   ,
                                           matchedEntriesInMain             ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Finished perfoming a diff between two patchs sets, commencing search entries in main that where not matched" );

        TConstFileEntrySet unmatchedEntriesInMain;
        BuildListOfUnmatchedItemsInMain( mainArchivePatchsetSearchMap ,
                                         matchedEntriesInMain         ,
                                         unmatchedEntriesInMain       );
        matchedEntriesInMain.clear();

        if ( PerformArchiveDiffMainToTemplate( templateArchivePatchsetSearchMap ,
                                               mainArchivePatchsetSearchMap     ,
                                               fileStatusList                   ,
                                               unmatchedEntriesInMain           ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Sucessfully completed the entire diff" );
            return true;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to perform a diff of main to template using the sorted maps" );
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to perform a diff of template to main using the sorted maps" );
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
SaveXmlFile( const CORE::CString& filePath   ,
             const CORE::CDataNode& dataTree )
{GUCEF_TRACE;

    if ( CORE::FileExists( filePath ) )
    {
        if ( !CORE::DeleteFile( filePath ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "A file already exists and it could not be deleted: " + filePath );
            return false;
        }
    }
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codecPtr = GetXmlDStoreCodec();
    if ( !codecPtr.IsNULL() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully located codec: " + codecPtr->GetTypeName() );

        if ( codecPtr->StoreDataTree( &dataTree ,
                                      filePath  ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully stored data tree to file using codec " + codecPtr->GetTypeName() + " at location " + filePath );
            return true;
        }
    }

    return false;
}

/*---------------------------------------------------------------------------*/

bool
LoadXmlFile( const CORE::CString& filePath  ,
             CORE::CDataNode& dataTree      )
{GUCEF_TRACE;

    if ( CORE::FileExists( filePath ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully located file: " + filePath );

        CORE::CDStoreCodecRegistry::TDStoreCodecPtr codecPtr = GetXmlDStoreCodec();
        if ( !codecPtr.IsNULL() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully located codec: " + codecPtr->GetTypeName() );

            if ( codecPtr->BuildDataTree( &dataTree ,
                                          filePath  ) )
            {
                return true;
            }
        }
    }
    return false;
}

/*---------------------------------------------------------------------------*/

CORE::UInt32
GetFileCount( const PATCHER::CPatchSetParser::TDirEntry& patchDir )
{GUCEF_TRACE;

    CORE::UInt32 count = patchDir.files.size();
    PATCHER::CPatchSetParser::TDirEntryList::const_iterator i = patchDir.subDirs.begin();
    while ( i != patchDir.subDirs.end() )
    {
        count += GetFileCount( (*i) );
        ++i;
    }
    return count;
}

/*---------------------------------------------------------------------------*/

CORE::UInt32
GetFileCount( const PATCHER::CPatchSetParser::TPatchSet& patchSet )
{GUCEF_TRACE;

    CORE::UInt32 count = 0;
    PATCHER::CPatchSetParser::TPatchSet::const_iterator i = patchSet.begin();
    while ( i != patchSet.end() )
    {
        count += GetFileCount( (*i) );
        ++i;
    }
    return count;
}

/*---------------------------------------------------------------------------*/

bool
LoadPatchSet( const CORE::CString& filePath                 ,
              PATCHER::CPatchSetParser::TPatchSet& patchSet )
{GUCEF_TRACE;

    CORE::CDataNode patchSetTree;
    if ( LoadXmlFile( filePath     ,
                      patchSetTree ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully loaded the xml data from file totalling " +
                                          CORE::UInt32ToString( patchSetTree.GetNrOfTreeNodes() ) +
                                          " nodes, parsing data into strongly typed data structures" );

        PATCHER::CPatchSetParser parser;
        if ( parser.ParsePatchSet( patchSetTree ,
                                   patchSet     ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully parsed the data tree into " +
                                              CORE::UInt32ToString( GetFileCount( patchSet ) ) +
                                              " patch set entries" );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
SerializeFileStatus( const TFileStatus& fileStatus ,
                     CORE::CDataNode& parentNode   )
{GUCEF_TRACE;

    static PATCHER::CPatchSetParser parser;

    CORE::CDataNode fileStatusNode;
    fileStatusNode.SetName( "FileStatus" );
    fileStatusNode.SetAttribute( "State", ResourceStateToString( fileStatus.resourceState ) );

    CORE::CDataNode templateFileInfo;
    templateFileInfo.SetName( "TemplateInfo" );
    parser.SerializeFileEntry( fileStatus.templateArchiveInfo ,
                               templateFileInfo               );
    fileStatusNode.AddChild( templateFileInfo );
    templateFileInfo.Clear();

    CORE::CDataNode mainFileInfo;
    mainFileInfo.SetName( "MainInfo" );
    parser.SerializeFileEntry( fileStatus.mainSvnArchiveInfo ,
                               mainFileInfo                  );
    fileStatusNode.AddChild( mainFileInfo );
    mainFileInfo.Clear();

    parentNode.AddChild( fileStatusNode );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
DeserializeFileInfo( const CORE::CDataNode& fileStatusNode           ,
                     const CORE::CString& name                       ,
                     PATCHER::CPatchSetParser::TFileEntry& fileEntry )
{GUCEF_TRACE;

    static PATCHER::CPatchSetParser parser;
    const CORE::CDataNode* infoNodeParent = fileStatusNode.FindChild( name );
    if ( NULL != infoNodeParent )
    {
        const CORE::CDataNode* infoNode = infoNodeParent->GetFirstChild();
        if ( NULL != infoNode )
        {
            if ( parser.ValidateAndParseFileEntry( *infoNode ,
                                                   fileEntry ,
                                                   false     ) )
            {
                return true;
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to locate child node with file info" );
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to locate child node with name " + name );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
DeserializeFileStatus( const CORE::CDataNode& fileStatusNode ,
                       TFileStatus& fileStatus               )
{GUCEF_TRACE;

    if ( fileStatusNode.GetName().Equals( "FileStatus", false ) )
    {
        // This is an actual file status entry
        fileStatus.resourceState = StringToResourceState( fileStatusNode.GetAttributeValue( "State" ) );

        if ( DeserializeFileInfo( fileStatusNode                 ,
                                  "TemplateInfo"                 ,
                                  fileStatus.templateArchiveInfo ) )
        {
            if ( DeserializeFileInfo( fileStatusNode                ,
                                      "MainInfo"                    ,
                                      fileStatus.mainSvnArchiveInfo ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully deserialized an entire file status object for: " + fileStatus.templateArchiveInfo.name );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to deserialize the main file info" );
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to deserialize the template file info" );
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "The given node in the XML document is not a file status node" );
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
SaveFileStatusList( const CORE::CString& filePath           ,
                    const TFileStatusVector& fileStatusList ,
                    const CORE::Int32 maxEntriesPerFile     )
{GUCEF_TRACE;

    CORE::CDataNode fileStatusTree;
    fileStatusTree.SetName( "FileStatusList" );
    fileStatusTree.SetAttribute( "TotalItemCount", CORE::UInt32ToString( fileStatusList.size() ) );

    CORE::UInt32 fileCounter = 1;
    CORE::UInt32 nrOfFiles = 1;
    if ( maxEntriesPerFile >= 0 )
    {
        CORE::Float32 nrOfFilesFl = fileStatusList.size() / ( maxEntriesPerFile * 1.0f);
        nrOfFiles = (CORE::UInt32) nrOfFilesFl;
        if ( nrOfFilesFl > nrOfFiles )
        {
            nrOfFiles++;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "A maximum number of entries per file has been specified, as a result " + CORE::UInt32ToString( nrOfFiles ) + " files be created instead of 1" );
    }
    fileStatusTree.SetAttribute( "TotalNrOfFileStatusListFiles", CORE::UInt32ToString( nrOfFiles ) );
    fileStatusTree.SetAttribute( "IndexOfFileStatusListFile", CORE::UInt32ToString( fileCounter ) );

    CORE::CString actualFilePath;
    CORE::Int32 entryCount = 0;
    TFileStatusVector::const_iterator i = fileStatusList.begin();
    while ( i != fileStatusList.end() )
    {
        const TFileStatus& fileStatus = (*i);
        SerializeFileStatus( fileStatus, fileStatusTree );
        ++entryCount;

        if ( ( maxEntriesPerFile >= 0 )         &&
             ( entryCount >= maxEntriesPerFile ) )
        {
            actualFilePath = filePath + ".(part" + CORE::UInt32ToString( fileCounter ) +
                             "of" + CORE::UInt32ToString( nrOfFiles ) + ").xml";

            if ( !SaveXmlFile( actualFilePath ,
                               fileStatusTree ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to save the generated data tree (part " + CORE::UInt32ToString( fileCounter ) +
                                                        " of " + CORE::UInt32ToString( nrOfFiles ) + ") to file " + filePath );
                return false;
            }

            // reset counter and tree
            fileStatusTree.DelSubTree();
            entryCount = 0;

            // Prepare for next file
            ++fileCounter;
            fileStatusTree.SetAttribute( "IndexOfFileStatusListFile", CORE::UInt32ToString( fileCounter ) );
        }
        ++i;
    }

    if ( nrOfFiles > 1 )
    {
        actualFilePath = filePath + ".(part" + CORE::UInt32ToString( fileCounter ) +
                         "of" + CORE::UInt32ToString( nrOfFiles ) + ").xml";
    }
    else
    {
        actualFilePath = filePath;
    }

    // Save the remaining entries
    if ( SaveXmlFile( actualFilePath ,
                      fileStatusTree ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully saved the generated data tree to file " + filePath );
        return true;
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to save the generated data tree to file " + filePath );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
SaveFileStatusListRelativePaths( const CORE::CString& filePath           ,
                                 const TFileStatusVector& fileStatusList ,
                                 bool templateInsteadOfMainPaths         )
{GUCEF_TRACE;

    CORE::CFileAccess file( filePath, "w" );
    file.Open();

    if ( !file.IsValid() )
    {
        return false;
    }

    TFileStatusVector::const_iterator i = fileStatusList.begin();
    while ( i != fileStatusList.end() )
    {
        const TFileStatus& fileStatus = (*i);
        CORE::CString relativePath;

        if ( templateInsteadOfMainPaths )
        {
            relativePath = GetPathForFile( fileStatus.templateArchiveInfo );
        }
        else
        {
            relativePath = GetPathForFile( fileStatus.mainSvnArchiveInfo );
        }
        if ( relativePath.Length() > 0 )
        {
            file.Write( relativePath );
        }
        ++i;
    }
    file.Close();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
LoadFileStatusListSegment( const CORE::CString& filePath     ,
                           TFileStatusVector& fileStatusList )
{GUCEF_TRACE;

    CORE::CDataNode fileStatusTree;
    if ( LoadXmlFile( filePath       ,
                      fileStatusTree ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully loaded the xml data from file totalling " + CORE::UInt32ToString( fileStatusTree.GetNrOfTreeNodes() ) +
                                          " nodes, parsing data into strongly typed data structures" );

        const CORE::CDataNode* rootNode = fileStatusTree.Find( "FileStatusList" );
        if ( NULL != rootNode )
        {
            CORE::CDataNode::const_iterator i = rootNode->ConstBegin();
            while ( i != rootNode->ConstEnd() )
            {
                const CORE::CDataNode* childNode = (*i);
                TFileStatus fileStatus;
                if ( DeserializeFileStatus( *childNode ,
                                            fileStatus ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully deserialized a file status entry" );
                    fileStatusList.push_back( fileStatus );
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to deserialize a file status entry" );
                }
                ++i;
            }
            return true;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to locate file status list root node in XML document" );
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
GetListOfMultiPartFiles( const CORE::CString& filePath ,
                         TStringSet& multiPartFiles    )
{GUCEF_TRACE;

    CORE::CString rootDir = CORE::StripFilename( filePath );
    CORE::CString rootFilename = CORE::ExtractFilename( filePath );

    struct CORE::SDI_Data* dirEntry = CORE::DI_First_Dir_Entry( rootDir.C_String() );
    if ( dirEntry != NULL )
    {
        do
        {
            CORE::CString entryName = CORE::DI_Name( dirEntry );
            if ( ( entryName != "." )  &&
                 ( entryName != ".." )  )
            {
                if ( 0 != CORE::DI_Is_It_A_File( dirEntry ) )
                {
                    // We found a file lets check if it has our rootFilename in it
                    if ( entryName.HasSubstr( rootFilename, 0, true ) == 0 )
                    {
                        // This file has our rootFilename it it's name
                        CORE::CString fullPathToSegmentFile = rootDir;
                        CORE::AppendToPath( fullPathToSegmentFile, entryName );

                        multiPartFiles.insert( fullPathToSegmentFile );
                    }
                }
            }
        }
        while ( CORE::DI_Next_Dir_Entry( dirEntry ) != 0 );

        // clean up our toys
        CORE::DI_Cleanup( dirEntry );

        // If this is a correct multi-part set we should have 2 files or more
        return multiPartFiles.size() > 1;
    }
    else
    {
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
GetMultiPartFileInfoFromPath( const CORE::CString& filePath ,
                              CORE::UInt32& fileIndex       ,
                              CORE::UInt32& totalFiles      )
{GUCEF_TRACE;

    // Cut such that we get the 'partXofY' segment
    CORE::CString remainder = filePath.CutChars( 5, false );
    remainder = remainder.SubstrToSubstr( ".(part", false );

    // remainder should now be 'XofY'
    CORE::CString indexStr = remainder.SubstrToSubstr( "of", true );
    CORE::CString totalStr = remainder.SubstrToSubstr( "of", false );

    if ( ( indexStr.Length() > 0 ) &&
         ( totalStr.Length() > 0 )  )
    {
        fileIndex = CORE::StringToUInt32( indexStr );
        totalFiles = CORE::StringToUInt32( totalStr );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
LoadFileStatusList( const CORE::CString& filePath     ,
                    TFileStatusVector& fileStatusList )
{GUCEF_TRACE;

    if ( CORE::FileExists( filePath ) )
    {
        // The given file exists as given so it is not a multi-part file
        // loading a single segment loads everything
        return LoadFileStatusListSegment( filePath       ,
                                          fileStatusList );
    }

    // If we get here we have to test for a multi-part diff list
    TStringSet multiPartFiles;
    if ( GetListOfMultiPartFiles( filePath       ,
                                  multiPartFiles ) )
    {
        CORE::UInt32 fileIndex = 0;
        CORE::UInt32 totalFiles = 0;
        const CORE::CString& aFileFromSet = *multiPartFiles.begin();
        if ( GetMultiPartFileInfoFromPath( aFileFromSet ,
                                           fileIndex    ,
                                           totalFiles   ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully located a set of file status files consisting of " + CORE::UInt32ToString( totalFiles ) + " files total" );

            for ( CORE::UInt32 i=1; i<=totalFiles; ++i )
            {
                // first create file path to segment
                CORE::CString segmentFilePath = filePath + ".(part" + CORE::UInt32ToString( i ) + "of" + CORE::UInt32ToString( totalFiles ) + ").xml";
                if ( !LoadFileStatusListSegment( segmentFilePath ,
                                                 fileStatusList  ) )
                {
                    // failed to load segment
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load file status list segment file \"" + segmentFilePath + "\"" );
                    return false;
                }
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully loaded a set of file status files consisting of " +
                                        CORE::UInt32ToString( totalFiles ) + " files total containing a combined total of " +
                                        CORE::UInt32ToString( fileStatusList.size() ) + " status entries" );
            return true;
        }
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load file as either a single file of a multi-part set: \"" + filePath + "\"" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
IsFileStatusListOnDisk( const CORE::CString& filePath )
{GUCEF_TRACE;

    // single file?
    if ( CORE::FileExists( filePath ) )
    {
        return true;
    }

    // maybe a multi-part set?
    TStringSet multiPartFiles;
    return GetListOfMultiPartFiles( filePath       ,
                                    multiPartFiles );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ARCHIVEDIFF */

/*-------------------------------------------------------------------------*/
