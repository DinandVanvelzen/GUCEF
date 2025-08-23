/*
 *  ProjectGen: GUCEF module providing the business logic to generate module/project files
 *
 *  Copyright (C) 1998 - 2025.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H
#include "CDStoreCodecPluginManager.h"
#define GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H
#endif /* GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_PROJECTGEN_CPROJECTGENGLOBAL_H
#include "gucefProjectGen_CProjectGenGlobal.h"
#define GUCEF_PROJECTGEN_CPROJECTGENGLOBAL_H
#endif /* GUCEF_PROJECTGEN_CPROJECTGENGLOBAL_H ? */

#ifndef GUCEF_PROJECTGEN_CDIRPREPROCESSORMANAGER_H
#include "gucefProjectGen_CDirPreprocessorManager.h"
#define GUCEF_PROJECTGEN_CDIRPREPROCESSORMANAGER_H
#endif /* GUCEF_PROJECTGEN_CDIRPREPROCESSORMANAGER_H ? */

#include "gucefProjectGen_CDirCrawlingProjectInfoGatherer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PROJECTGEN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const CORE::CString AllPlatforms = "all";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

//  Forward declarations:

const TDirProcessingInstructions*
GetProcessingInstructions( const CProjectInfo& projectInfo ,
                           const CORE::CString& dir        );


bool
IsStringInList( const TStringVector& list       ,
                bool caseSensitive              ,
                const CORE::CString& testString ,
                bool wildcardMatching = false   );

/*---------------------------------------------------------------------------*/

static const TStringSetMap&
GetSupportedPlatformDirMap( const CProjectInfo& projectInfo )
{GUCEF_TRACE;

    static TStringSetMap platformMap;
    if ( platformMap.empty() )
    {
        TPlatformDefinitionMap::const_iterator i = projectInfo.platforms.begin();
        while ( i != projectInfo.platforms.end() )
        {
            platformMap[ (*i).first ] = (*i).second.platformDirs;
            ++i;
        }
    }
    return platformMap;
}

/*---------------------------------------------------------------------------*/

const TStringVector&
GetSourceFileExtensions( void )
{GUCEF_TRACE;

    static TStringVector fileTypes;

    if ( fileTypes.empty() )
    {
        fileTypes.push_back( "c" );
        fileTypes.push_back( "cc" );
        fileTypes.push_back( "cpp" );
        fileTypes.push_back( "cxx" );
        fileTypes.push_back( "asm" );
        fileTypes.push_back( "cs" );
    }
    return fileTypes;
}

/*--------------------------------------------------------------------------*/

const TStringVector&
GetHeaderFileExtensions( void )
{GUCEF_TRACE;

    static TStringVector fileTypes;

    if ( fileTypes.empty() )
    {
        fileTypes.push_back( "h" );
        fileTypes.push_back( "hpp" );
        fileTypes.push_back( "hxx" );
    }
    return fileTypes;
}

/*---------------------------------------------------------------------------*/

static const TStringSet&
GetSupportedPlatforms( const CProjectInfo& projectInfo )
{GUCEF_TRACE;

    static TStringSet platforms;
    if ( platforms.empty() )
    {
        const TStringSetMap& dirMap = GetSupportedPlatformDirMap( projectInfo );
        TStringSetMap::const_iterator i = dirMap.begin();
        while ( i != dirMap.end() )
        {
            platforms.insert( (*i).first );
            ++i;
        }
    }
    return platforms;
}

/*---------------------------------------------------------------------------*/

const TStringSet&
GetSupportedPlatformDirs( const CProjectInfo& projectInfo )
{GUCEF_TRACE;

    static TStringSet platformDirs;
    if ( platformDirs.empty() )
    {
        const TStringSetMap& dirMap = GetSupportedPlatformDirMap( projectInfo );
        TStringSetMap::const_iterator i = dirMap.begin();
        while ( i != dirMap.end() )
        {
            const TStringSet& dirs = (*i).second;
            TStringSet::const_iterator n = dirs.begin();
            while ( n != dirs.end() )
            {
                platformDirs.insert( (*n) );
                ++n;
            }
            ++i;
        }
    }
    return platformDirs;
}

/*---------------------------------------------------------------------------*/

bool
IsDirAPlatformDir( const CProjectInfo& projectInfo  ,
                   const CORE::CString& path        ,
                   bool checkProcessingInstructions )
{GUCEF_TRACE;

    if ( checkProcessingInstructions )
    {
        // First check the processing instructions

        CORE::CString instructionPath = path;
        UInt32 lastPathLenght = 0;
        CORE::CString prevLastSubDir, lastSubDir;
        do
        {
            lastPathLenght = instructionPath.Length();
            lastSubDir = CORE::LastSubDir( instructionPath );

            const TDirProcessingInstructions* instructionStorage = GetProcessingInstructions( projectInfo, instructionPath );
            if ( GUCEF_NULL != instructionStorage )
            {
                TStringVectorMap::const_iterator i = instructionStorage->dirIncludeList.begin();
                while ( i != instructionStorage->dirIncludeList.end() )
                {
                    const CORE::CString& platform = (*i).first;
                    if ( !platform.IsNULLOrEmpty() && platform != AllPlatforms )
                    {
                        if ( IsStringInList( (*i).second, true, lastSubDir ) )
                            return true;
                        if ( !prevLastSubDir.IsNULLOrEmpty() && IsStringInList( (*i).second, true, prevLastSubDir ) )
                            return true;
                    }
                    ++i;
                }
            }

            prevLastSubDir = lastSubDir;
            instructionPath = CORE::StripLastSubDir( instructionPath );
        }
        while ( lastPathLenght != instructionPath.Length() );
    }

    // A directory is a platform dir not only if the last subdir matches a platform dir name
    // but also if a parent dir is a platform dir, since platform dirs can also have dir hierachies of course

    TStringVector searchPathSegs = path.Lowercase().ReplaceChar( '\\', '/' ).ParseElements( '/', false );
    const TStringSet& supportedPlatformDirs = GetSupportedPlatformDirs( projectInfo );

    TStringVector::const_iterator i = searchPathSegs.begin();
    while ( i != searchPathSegs.end() )
    {
        if ( supportedPlatformDirs.end() != supportedPlatformDirs.find( (*i) ) )
        {
            // Found a platform dir in the path hierarchy
            return true;
        }
        ++i;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
IsDirAPlatformDirForPlatform( const CProjectInfo& projectInfo  ,
                              const CORE::CString& path        ,
                              const CORE::CString& platform    ,
                              bool checkProcessingInstructions )
{GUCEF_TRACE;

    if ( checkProcessingInstructions )
    {
        // First check the processing instructions
        // We need to check all segments of the path because the processing instructions could have been defined at a higher level dir
        CORE::CString instructionPath = path;
        UInt32 lastPathLenght = 0;
        CORE::CString prevLastSubDir, lastSubDir;
        do
        {
            lastPathLenght = instructionPath.Length();
            lastSubDir = CORE::LastSubDir( instructionPath );

            const TDirProcessingInstructions* instructionStorage = GetProcessingInstructions( projectInfo, instructionPath );
            if ( GUCEF_NULL != instructionStorage )
            {
                TStringVectorMap::const_iterator i = instructionStorage->dirIncludeList.find( platform );
                if ( i != instructionStorage->dirIncludeList.end() )
                {
                    if ( IsStringInList( (*i).second, true, lastSubDir ) )
                        return true;
                    if ( !prevLastSubDir.IsNULLOrEmpty() && IsStringInList( (*i).second, true, prevLastSubDir ) )
                        return true;
                }
            }

            prevLastSubDir = lastSubDir;
            instructionPath = CORE::StripLastSubDir( instructionPath );
        }
        while ( lastPathLenght != instructionPath.Length() );
    }

    // Next check the actual path name against reserved directory names for certain platforms
    const TStringSetMap& platformDirMap = GetSupportedPlatformDirMap( projectInfo );
    TStringSetMap::const_iterator i = platformDirMap.find( platform );
    if ( i != platformDirMap.end() )
    {
        // A directory is a platform dir not only if the last subdir matches a platform dir name
        // but also if a parent dir is a platform dir, since platform dirs can also have dir hierachies of course
        TStringVector searchPathSegs = path.Lowercase().ReplaceChar( '\\', '/' ).ParseElements( '/', false );
        const TStringSet& dirsForPlatform = (*i).second;

        TStringVector::const_iterator n = searchPathSegs.begin();
        while ( n != searchPathSegs.end() )
        {
            if ( dirsForPlatform.end() != dirsForPlatform.find( (*n) ) )
            {
                // Found a platform dir belonging to the given platform in the path hierarchy
                return true;
            }
            ++n;
        }
    }
    return false;
}

/*--------------------------------------------------------------------------*/

bool
RemoveString( TStringVector& list            ,
              const CORE::CString& searchStr )
{GUCEF_TRACE;

    //@TODO: make wildcard processing more advanced then this :)
    if ( searchStr == "*" )
    {
        list.clear();
        return true;
    }

    bool removedString = false;
    TStringVector::iterator i = list.begin();
    while ( i != list.end() )
    {
        if ( (*i) == searchStr )
        {
            list.erase( i );
            i = list.begin();
            removedString = true;
        }
        else
        {
            ++i;
        }
    }
    return removedString;
}

/*--------------------------------------------------------------------------*/

bool
RemoveString( TStringSet& list               ,
              const CORE::CString& searchStr )
{GUCEF_TRACE;

    //@TODO: make wildcard processing more advanced then this :)
    if ( searchStr == "*" )
    {
        list.clear();
        return true;
    }

    bool removedString = false;
    TStringSet::iterator i = list.begin();
    while ( i != list.end() )
    {
        if ( (*i) == searchStr )
        {
            list.erase( i );
            i = list.begin();
            removedString = true;
        }
        else
        {
            ++i;
        }
    }
    return removedString;
}

/*---------------------------------------------------------------------------*/

bool
IsStringInList( const TStringVector& list       ,
                bool caseSensitive              ,
                const CORE::CString& testString ,
                bool wildcardMatching           )
{GUCEF_TRACE;

    TStringVector::const_iterator i = list.begin();
    while ( i != list.end() )
    {
        if ( (*i).Equals( testString, caseSensitive ) )
        {
            return true;
        }
        if ( wildcardMatching )
        {
            if ( testString.WildcardEquals( (*i), '*', caseSensitive ) )
            {
                return true;
            }
        }
        ++i;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

static bool
IsStringInList( const TStringSet& list          ,
                bool caseSensitive              ,
                const CORE::CString& testString ,
                bool wildcardMatching = false   )
{GUCEF_TRACE;

    TStringSet::const_iterator i = list.begin();
    while ( i != list.end() )
    {
        if ( (*i).Equals( testString, caseSensitive ) )
        {
            return true;
        }
        if ( wildcardMatching )
        {
            if ( testString.WildcardEquals( (*i), '*', caseSensitive ) )
            {
                return true;
            }
        }
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CModuleInfoEntryPtr
FindNextModuleAccordingToBuildOrder( CProjectInfo& projectInfo               ,
                                     CModuleInfoEntryPtr& currentModuleEntry ,
                                     const CORE::CString& targetPlatform     ,
                                     Int64 desiredBuildOrder = -1            )
{GUCEF_TRACE;

    if ( desiredBuildOrder == -1 )
    {
        desiredBuildOrder = currentModuleEntry->GetBuildOrder( targetPlatform ) + 1;
    }

    CModuleInfoEntryPtr platformEntry = projectInfo.FindModuleAccordingToBuildOrder( targetPlatform    ,
                                                                                     desiredBuildOrder );
    if ( platformEntry.IsNULL() && targetPlatform != AllPlatforms )
    {
        // If we get here we did not find a module with build order 0
        // instead of a platform specific build order check the AllPlatforms build order
        platformEntry = projectInfo.FindModuleAccordingToBuildOrder( AllPlatforms      ,
                                                                     desiredBuildOrder );
    }

    return platformEntry;
}

/*---------------------------------------------------------------------------*/

void
FilterStringVectorForFilesWithExtensions( TStringVector& outputVector         ,
                                          const TStringVector& extensions     ,
                                          const TStringVector& originalVector )
{GUCEF_TRACE;

    TStringVector::const_iterator i = originalVector.begin();
    while ( i != originalVector.end() )
    {
        CORE::CString foundExtension = CORE::ExtractFileExtention( (*i) );
        if ( IsStringInList( extensions, false, foundExtension ) )
        {
            outputVector.push_back( (*i) );
        }
        ++i;
    }
}
/*---------------------------------------------------------------------------*/

bool
IsDirALegacyProjectDir( const CORE::CString& dir )
{GUCEF_TRACE;

    // The dir is a module dir if it has a suffix file in it
    CORE::CString suffixFilePath = dir;
    CORE::AppendToPath( suffixFilePath, "CMakeListsSuffix.txt" );

    return CORE::FileExists( suffixFilePath );
}

/*---------------------------------------------------------------------------*/

bool
IsDirAProjectDir( const CORE::CString& dir )
{GUCEF_TRACE;

    // The dir is a module dir if it has a suffix file in it
    CORE::CString moduleInfoFilePath = dir;
    CORE::AppendToPath( moduleInfoFilePath, "ModuleInfo.xml" );

    if ( !CORE::FileExists( moduleInfoFilePath ) )
    {
        return IsDirALegacyProjectDir( dir );
    }
    return true;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GetProcessingInstructionsPath( const CORE::CString& dir )
{GUCEF_TRACE;

    CORE::CString instructionsFile = dir;
    CORE::AppendToPath( instructionsFile, "ProjectGenInstructions.xml" );
    return instructionsFile;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GetLegacyProcessingInstructionsPath( const CORE::CString& dir )
{GUCEF_TRACE;

    CORE::CString instructionsFile = dir;
    CORE::AppendToPath( instructionsFile, "CMakeGenInstructions.xml" );
    return instructionsFile;
}

/*---------------------------------------------------------------------------*/

bool
GetProcessingInstructions( const CORE::CString& dir      ,
                           CORE::CDataNode& instructions )
{GUCEF_TRACE;

    CORE::CString instructionsFile = GetProcessingInstructionsPath( dir );
    if ( !CORE::FileExists( instructionsFile ) )
    {
        instructionsFile = GetLegacyProcessingInstructionsPath( dir );
        if ( !CORE::FileExists( instructionsFile ) )
        {
            return false;
        }
    }

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codecPtr = GetXmlDStoreCodec();
    if ( !codecPtr.IsNULL() )
    {
        if ( codecPtr->BuildDataTree( &instructions    ,
                                      instructionsFile ) )
        {
            return true;
        }
    }
    return false;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GetExcludeListPath( const CORE::CString& dir )
{GUCEF_TRACE;

    CORE::CString excludeFile = dir;
    CORE::AppendToPath( excludeFile, "ProjectGenExcludeList.txt" );
    return excludeFile;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GetLegacyExcludeListPath( const CORE::CString& dir )
{GUCEF_TRACE;

    CORE::CString excludeFile = dir;
    CORE::AppendToPath( excludeFile, "CMakeGenExcludeList.txt" );
    return excludeFile;
}

/*---------------------------------------------------------------------------*/

TStringVector
GetExcludeList( const CORE::CString& dir )
{GUCEF_TRACE;

    CORE::CString excludeFile = GetExcludeListPath( dir );
    if ( !CORE::FileExists( excludeFile ) )
    {
        // try legacy filename
        excludeFile = GetLegacyExcludeListPath( dir );
        if ( !CORE::FileExists( excludeFile ) )
        {
            return TStringVector();
        }
    }

    CORE::CString excludeFileContent;
    if ( CORE::LoadTextFileAsString( excludeFile        ,
                                     excludeFileContent ) )
    {
        excludeFileContent = excludeFileContent.RemoveChar( '\r' );

        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Loaded simple exclude list for directory \"" + dir + "\"" );
        return excludeFileContent.ParseElements( '\n' );
    }
    return TStringVector();
}

/*---------------------------------------------------------------------------*/

void
ExcludeOrIncludeEntriesAsSpecifiedForDir( const TDirProcessingInstructions& allInstructions ,
                                          const CORE::CString& platform                     ,
                                          TStringSet& allEntries                            )
{GUCEF_TRACE;

    // see if we have info for this platform
    TStringVectorMap::const_iterator q = allInstructions.dirExcludeList.find( platform );
    if ( q != allInstructions.dirExcludeList.end() )
    {
        // apply changes
        const TStringVector& excludeList = (*q).second;
        TStringVector::const_iterator n = excludeList.begin();
        while ( n != excludeList.end() )
        {
            if ( RemoveString( allEntries, (*n) ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Excluded the directory \"" + (*n) + "\" based on the exclude list for this dir" );
            }
            ++n;
        }
    }

    // see if we have info for this platform
    q = allInstructions.dirIncludeList.find( platform );
    if ( q != allInstructions.dirIncludeList.end() )
    {
        // apply changes
        const TStringVector& includeList = (*q).second;
        TStringVector::const_iterator n = includeList.begin();
        while ( n != includeList.end() )
        {
            allEntries.insert( (*n) );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Included the directory \"" + (*n) + "\" based on the include list for this dir" );
            ++n;
        }
    }
}

/*---------------------------------------------------------------------------*/

void
ExcludeEntriesAsSpecifiedForFile( const TDirProcessingInstructions& allInstructions ,
                                  const CORE::CString& platform                     ,
                                  TStringSet& allEntries                            )
{GUCEF_TRACE;

    // see if we have info for this platform
    TStringVectorMap::const_iterator q = allInstructions.fileExcludeList.find( platform );
    if ( q != allInstructions.fileExcludeList.end() )
    {
        // apply changes
        const TStringVector& excludeList = (*q).second;
        TStringVector::const_iterator n = excludeList.begin();
        while ( n != excludeList.end() )
        {
            if ( RemoveString( allEntries, (*n) ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Excluded the file \"" + (*n) + "\" based on the exclude list for this dir" );
            }
            ++n;
        }
    }
}

/*---------------------------------------------------------------------------*/

void
IncludeEntriesAsSpecifiedForFile( const TDirProcessingInstructions& allInstructions ,
                                  const CORE::CString& platform                     ,
                                  const CORE::CString& currentPath                  ,
                                  TStringSetMap& allEntries                         ,
                                  const TStringVector& fileTypes                    )
{GUCEF_TRACE;

    // see if we have info for this platform
    TStringVectorMap::const_iterator q = allInstructions.fileIncludeList.find( platform );
    if ( q != allInstructions.fileIncludeList.end() )
    {
        // apply changes
        const TStringVector& includeList = (*q).second;
        TStringVector::const_iterator n = includeList.begin();
        while ( n != includeList.end() )
        {
            const CORE::CString& fullEntry = (*n);
            CORE::CString fileExt = CORE::ExtractFileExtention( fullEntry );

            if ( IsStringInList( fileTypes, false, fileExt ) )
            {
                CORE::CString filename = CORE::ExtractFilename( fullEntry );
                CORE::CString path = currentPath;
                CORE::AppendToPath( path, fullEntry.CutChars( filename.Length(), false ) );
                path = CORE::RelativePath( path );
                path = path.ReplaceChar( '\\', '/' );

                allEntries[ path ].insert( filename );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Included the file \"" + filename + "\" based on the include list for this dir using path " + path );
            }
            ++n;
        }
    }
}

/*---------------------------------------------------------------------------*/

void
ExcludeOrIncludeDirEntriesAsSpecifiedForDir( const CORE::CString& dir                          ,
                                             const TDirProcessingInstructions& allInstructions ,
                                             const CORE::CString& platform                     ,
                                             bool applyPlatformChangesOnly                     ,
                                             TStringSet& allEntries                            )
{GUCEF_TRACE;

    if ( !applyPlatformChangesOnly )
    {
        ExcludeOrIncludeEntriesAsSpecifiedForDir( allInstructions ,
                                                  AllPlatforms    ,
                                                  allEntries      );

    }

    if ( !platform.IsNULLOrEmpty() && platform != AllPlatforms )
    {
        ExcludeOrIncludeEntriesAsSpecifiedForDir( allInstructions ,
                                                  platform        ,
                                                  allEntries      );
    }
}

/*---------------------------------------------------------------------------*/

void
ExcludeFileEntriesAsSpecifiedForDir( const TDirProcessingInstructions& allInstructions ,
                                     const CORE::CString& platform                     ,
                                     bool applyPlatformChangesOnly                     ,
                                     TStringSet& allEntries                            )
{GUCEF_TRACE;

    if ( !applyPlatformChangesOnly )
    {
        ExcludeEntriesAsSpecifiedForFile( allInstructions ,
                                          AllPlatforms    ,
                                          allEntries      );
    }

    if ( !platform.IsNULLOrEmpty() && platform != AllPlatforms )
    {
        ExcludeEntriesAsSpecifiedForFile( allInstructions ,
                                          platform        ,
                                          allEntries      );
    }
}

/*---------------------------------------------------------------------------*/

void
IncludeFileEntriesAsSpecifiedForDir( const TDirProcessingInstructions& allInstructions ,
                                     const CORE::CString& platform                     ,
                                     const CORE::CString& currentPath                  ,
                                     bool applyPlatformChangesOnly                     ,
                                     TStringSetMap& allEntries                         ,
                                     const TStringVector& fileTypes                    )
{GUCEF_TRACE;

    if ( !applyPlatformChangesOnly )
    {
        IncludeEntriesAsSpecifiedForFile( allInstructions ,
                                          AllPlatforms    ,
                                          currentPath     ,
                                          allEntries      ,
                                          fileTypes       );
    }

    if ( !platform.IsNULLOrEmpty() && platform != AllPlatforms )
    {
        IncludeEntriesAsSpecifiedForFile( allInstructions ,
                                          platform        ,
                                          currentPath     ,
                                          allEntries      ,
                                          fileTypes       );
    }
}

/*---------------------------------------------------------------------------*/

bool
IsProcessingInstructionsItemADir( const CORE::CString& instructionDir ,
                                  const CORE::CString& itemName       )
{GUCEF_TRACE;

    CORE::CString itemPath = instructionDir;
    CORE::AppendToPath( itemPath, itemName );

    if ( CORE::FileExists( itemPath ) )
    {
        return false;
    }
    else
    if ( CORE::IsPathValid( itemPath ) )
    {
        return true;
    }
    else
    {
        // Unable to determine by checking if the item exists on disk because it isnt there
        // As a fallback we will try to check for an extension, its the best we can do
        return CORE::ExtractFileExtention( itemName ).Length() == 0;
    }
}

/*---------------------------------------------------------------------------*/

bool
IsProcessingInstructionsItemADir( const CORE::CString& instructionDir ,
                                  const CORE::CString& itemName       ,
                                  const CORE::CString& itemType       )
{GUCEF_TRACE;

    if ( itemType.IsNULLOrEmpty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "The type of an item (\"" + itemName + "\") in the processing instructions is not specified, will try to auto-determine the type" );
        if ( IsProcessingInstructionsItemADir( instructionDir, itemName ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Determined that item \"" + itemName + "\" is a directory" );
            return true;
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Determined that item \"" + itemName + "\" is a file" );
            return false;
        }
    }
    else
    {
        static const CORE::CString typeAttribDirValue = "DIR";
        return itemType.Equals( typeAttribDirValue, false );
    }
}

/*---------------------------------------------------------------------------*/

void
ParseProcessingInstructions( const CProjectInfo& projectInfo                ,
                             const CORE::CString& instructionsDir           ,
                             TDirProcessingInstructions& instructionStorage )
{GUCEF_TRACE;

    // Now we exclude based on the processing instructions for this dir
    // if any exist
    const CORE::CDataNode& instructions = instructionStorage.processingInstructions;
    if ( instructions.HasChildren() )
    {
        // write down the tags we will parse for
        static const CORE::CString oldRootNodeName = "CMAKELISTGENERATOR";
        static const CORE::CString rootNodeName = "PROJECTGENERATOR";
        static const CORE::CString excludesNodeName = "EXCLUDES";
        static const CORE::CString includesNodeName = "INCLUDES";
        static const CORE::CString platformNodeName = "PLATFORM";
        static const CORE::CString platformsNodeName = "PLATFORMS";
        static const CORE::CString itemNodeName = "ITEM";
        static const CORE::CString nameAttribName = "NAME";
        static const CORE::CString typeAttribName = "TYPE";
        static const CORE::CString allPlatformsValue = "ALL";

        // Parse the instructions for the information we are looking for
        CORE::CString leftOver;
        const CORE::CDataNode* rootNode = instructions.Find( rootNodeName );
        if ( 0 == rootNode )
        {
            rootNode = instructions.Find( oldRootNodeName );
        }
        if ( 0 != rootNode )
        {
            CORE::CDataNode::TConstDataNodeSet platformsNodes = rootNode->FindChildrenOfType( platformsNodeName, false );
            CORE::CDataNode::TConstDataNodeSet::iterator w = platformsNodes.begin();
            while ( w != platformsNodes.end() )
            {
                CORE::CDataNode::TConstDataNodeSet platformNodes = rootNode->FindChildrenOfType( platformNodeName, false );
                CORE::CDataNode::TConstDataNodeSet::iterator q = platformsNodes.begin();
                while ( q != platformsNodes.end() )
                {
                    const CORE::CDataNode* platformNode = (*q);
                    TStringVector platformNames = platformNode->GetAttributeValueOrChildValueByName( "NAME" ).AsString().Lowercase().ParseElements( ';', false );
                    TStringVector::iterator j = platformNames.begin();
                    while ( j != platformNames.end() )
                    {
                        const CORE::CString& platformName = (*j);
                        if ( !platformName.IsNULLOrEmpty() )
                        {
                            TPlatformDefinition& platformDefinition = instructionStorage.platforms[ platformName ];
                            platformDefinition.aliases = StringVectorToStringSet( platformNode->GetAttributeValueOrChildValueByName( "ALIASES" ).AsString().Lowercase().ParseElements( ';', false ) );
                            platformDefinition.platformDirs = StringVectorToStringSet( platformNode->GetAttributeValueOrChildValueByName( "PLATFORMDIRS" ).AsString().ParseElements( ';', false ) );
                        }
                    }
                    ++q;
                }
                ++w;
            }

            CORE::CDataNode::const_iterator i = rootNode->ConstBegin();
            while ( i != rootNode->End() )
            {
                const CORE::CDataNode* curNode = (*i);
                if ( curNode->GetName() == excludesNodeName )
                {
                    // Store whether the instructions apply to a particular platform
                    CORE::CString platformValue = curNode->GetAttributeValue( platformNodeName ).AsString().Lowercase();

                    // apply 1 to n platform mappings if applicable
                    TStringSet platforms;
                    if ( !projectInfo.platforms.empty() )
                        platforms = ResolveMultiPlatformName( platformValue, &projectInfo.platforms );
                    if ( !instructionStorage.platforms.empty() )
                        MergeStringSet( platforms, ResolveMultiPlatformName( platformValue, &instructionStorage.platforms ), false );
                    TStringSet::iterator p = platforms.begin();
                    while ( p != platforms.end() )
                    {
                        const CORE::CString& platformName = (*p);

                        // Parse instructions
                        const CORE::CDataNode* excludesNode = curNode;
                        CORE::CDataNode::const_iterator n = excludesNode->ConstBegin();
                        while ( n != excludesNode->ConstEnd() )
                        {
                            // Process all the items in this instruction set
                            const CORE::CDataNode* curNode2 = (*n);
                            if ( curNode2->GetName() == itemNodeName )
                            {
                                // Get the name and type of the item to add to the list
                                CORE::CString itemName = curNode2->GetAttributeValue( nameAttribName );
                                if ( !itemName.IsNULLOrEmpty() )
                                {
                                    CORE::CString itemType = curNode2->GetAttributeValue( typeAttribName );
                                    bool itemIsADir = IsProcessingInstructionsItemADir( instructionsDir, itemName, itemType );

                                    // We found an item to add to our list
                                    if ( platformName.IsNULLOrEmpty() || platformName.Equals( allPlatformsValue, false ) )
                                    {
                                        // Add as non-platform specific
                                        if ( itemName.HasChar( '*' ) != -1 && itemType.IsNULLOrEmpty() )
                                        {
                                            instructionStorage.dirExcludeList[ AllPlatforms ].push_back( itemName );
                                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added wildcard directory exclude entry \"" + itemName + "\" based on the processing instructions for this dir" );
                                            instructionStorage.fileExcludeList[ AllPlatforms ].push_back( itemName );
                                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added wildcard file exclude entry \"" + itemName + "\" based on the processing instructions for this dir" );
                                        }
                                        else
                                        {
                                            if ( itemIsADir )
                                            {
                                                instructionStorage.dirExcludeList[ AllPlatforms ].push_back( itemName );
                                                GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added directory exclude entry \"" + itemName + "\" based on the processing instructions for this dir" );
                                            }
                                            else
                                            {
                                                instructionStorage.fileExcludeList[ AllPlatforms ].push_back( itemName );
                                                GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added file exclude entry \"" + itemName + "\" based on the processing instructions for this dir" );
                                            }
                                        }
                                    }
                                    else
                                    {
                                        // Add for the given platform
                                        if ( itemName.HasChar( '*' ) != -1 && itemType.IsNULLOrEmpty() )
                                        {
                                            instructionStorage.dirExcludeList[ platformName ].push_back( itemName );
                                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added wildcard directory exclude entry \"" + itemName + "\" for platform " + platformName + " based on the processing instructions for this dir" );
                                            instructionStorage.fileExcludeList[ platformName ].push_back( itemName );
                                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added wildcard file exclude entry \"" + itemName + "\" for platform " + platformName + " based on the processing instructions for this dir" );
                                        }
                                        else
                                        {
                                            if ( itemIsADir )
                                            {
                                                instructionStorage.dirExcludeList[ platformName ].push_back( itemName );
                                                GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added directory exclude entry \"" + itemName + "\" for platform " + platformName + " based on the processing instructions for this dir" );
                                            }
                                            else
                                            {
                                                instructionStorage.fileExcludeList[ platformName ].push_back( itemName );
                                                GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added file exclude entry \"" + itemName + "\" for platform " + platformName + " based on the processing instructions for this dir" );
                                            }
                                        }
                                    }
                                }
                            }
                            ++n;
                        }
                        ++p;
                    }
                }
                else
                if ( curNode->GetName() == includesNodeName )
                {
                    // Store whether the instructions apply to a particular platform
                    CORE::CString platformValue = curNode->GetAttributeValue( platformNodeName ).AsString().Lowercase();

                    // apply 1 to n platform mappings if applicable
                    TStringSet platforms;
                    if ( !projectInfo.platforms.empty() )
                        platforms = ResolveMultiPlatformName( platformValue, &projectInfo.platforms );
                    if ( !instructionStorage.platforms.empty() )
                        MergeStringSet( platforms, ResolveMultiPlatformName( platformValue, &instructionStorage.platforms ), false );
                    TStringSet::iterator p = platforms.begin();
                    while ( p != platforms.end() )
                    {
                        CORE::CString platformName = (*p);

                        // Parse instructions
                        const CORE::CDataNode* includesNode = curNode;
                        CORE::CDataNode::const_iterator n = includesNode->ConstBegin();
                        while ( n != includesNode->ConstEnd() )
                        {
                            // Process all the items in this instruction set
                            const CORE::CDataNode* curNode2 = (*n);
                            if ( curNode2->GetName() == itemNodeName )
                            {
                                // Get the name of the item to add to the list
                                CORE::CString itemName = curNode2->GetAttributeValue( nameAttribName );
                                if ( !itemName.IsNULLOrEmpty() )
                                {
                                    CORE::CString itemType = curNode2->GetAttributeValue( typeAttribName );
                                    bool itemIsADir = IsProcessingInstructionsItemADir( instructionsDir, itemName, itemType );

                                    // We found an item to add to our list
                                    if ( platformName.IsNULLOrEmpty() || platformName.Equals( allPlatformsValue, false ) )
                                    {
                                        // Add as non-platform specific
                                        if ( itemName.HasChar( '*' ) != -1 && itemType.IsNULLOrEmpty() )
                                        {
                                            instructionStorage.dirIncludeList[ AllPlatforms ].push_back( itemName );
                                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added wildcard directory include entry \"" + itemName + "\" based on the processing instructions for this dir" );
                                            instructionStorage.fileIncludeList[ AllPlatforms ].push_back( itemName );
                                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added wildcard file include entry \"" + itemName + "\" based on the processing instructions for this dir" );
                                        }
                                        else
                                        if ( itemIsADir )
                                        {
                                            instructionStorage.dirIncludeList[ AllPlatforms ].push_back( itemName );
                                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added directory include entry \"" + itemName + "\" based on the processing instructions for this dir" );
                                        }
                                        else
                                        {
                                            instructionStorage.fileIncludeList[ AllPlatforms ].push_back( itemName );
                                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added file include entry \"" + itemName + "\" based on the processing instructions for this dir" );
                                        }
                                    }
                                    else
                                    {
                                        // Add for the given platform
                                        if ( itemName.HasChar( '*' ) != -1 && itemType.IsNULLOrEmpty() )
                                        {
                                            instructionStorage.dirIncludeList[ platformName ].push_back( itemName );
                                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added wildcard directory include entry \"" + itemName + "\" for platform " + platformName + " based on the processing instructions for this dir" );
                                            instructionStorage.fileIncludeList[ platformName ].push_back( itemName );
                                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added wildcard file include entry \"" + itemName + "\" for platform " + platformName + " based on the processing instructions for this dir" );
                                        }
                                        else
                                        {
                                            if ( itemIsADir )
                                            {
                                                instructionStorage.dirIncludeList[ platformName ].push_back( itemName );
                                                GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added directory include entry \"" + itemName + "\" for platform " + platformName + " based on the processing instructions for this dir" );
                                            }
                                            else
                                            {
                                                instructionStorage.fileIncludeList[ platformName ].push_back( itemName );
                                                GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added file include entry \"" + itemName + "\" for platform " + platformName + " based on the processing instructions for this dir" );
                                            }
                                        }
                                    }
                                }
                            }
                            ++n;
                        }
                        ++p;
                    }
                }

                ++i;
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

bool
AreProcessingInstructionsOnDisk( const CORE::CString& dir )
{GUCEF_TRACE;

    return CORE::FileExists( GetExcludeListPath( dir ) )               ||
           CORE::FileExists( GetLegacyExcludeListPath( dir ) )         ||
           CORE::FileExists( GetProcessingInstructionsPath( dir ) )    ||
           CORE::FileExists( GetLegacyProcessingInstructionsPath( dir ) );
}

/*---------------------------------------------------------------------------*/

const TDirProcessingInstructions*
GetProcessingInstructions( const CProjectInfo& projectInfo                          ,
                           const CORE::CString& dir                                 ,
                           TDirProcessingInstructionsMap& newProcessingInstructions )
{GUCEF_TRACE;

    // See if we have already stored instructions for this directory
    TDirProcessingInstructionsMap::const_iterator i = projectInfo.dirProcessingInstructions.find( dir );
    if ( i != projectInfo.dirProcessingInstructions.end() )
    {
        return &( (*i).second );
    }

    // Perhaps it was newly added but not added to the overall collection yet
    i = newProcessingInstructions.find( dir );
    if ( i != newProcessingInstructions.end() )
    {
        return &( (*i).second );
    }

    // We have nothing for this directory yet, see if we can find instructions on disk
    if ( AreProcessingInstructionsOnDisk( dir ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Located processing instructions for directory \"" + dir + "\"" );

        // See if we can load instructions for this directory
        TDirProcessingInstructions& instructions = newProcessingInstructions[ dir ];

        // Load the simple exclude list and put the excludes in the correct list
        TStringVector simpleExcludeList = GetExcludeList( dir );
        TStringVector::iterator n = simpleExcludeList.begin();
        while ( n != simpleExcludeList.end() )
        {
            CORE::CString& entry = (*n);
            if ( !entry.IsNULLOrEmpty() )
            {
                if ( entry.HasChar( '*' ) != -1 )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Determined that the simple exclude list wildcard item \"" + entry + "\" is a directory" );
                    instructions.dirExcludeList[ AllPlatforms ].push_back( entry );
                    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Determined that the simple exclude list wildcard item \"" + entry + "\" is a file" );
                    instructions.fileExcludeList[ AllPlatforms ].push_back( entry );
                }
                else
                if ( IsProcessingInstructionsItemADir( dir, entry ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Determined that the simple exclude list item \"" + entry + "\" is a directory" );
                    instructions.dirExcludeList[ AllPlatforms ].push_back( entry );
                }
                else
                {
                    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Determined that the simple exclude list item \"" + entry + "\" is a file" );
                    instructions.fileExcludeList[ AllPlatforms ].push_back( entry );
                }
            }
            ++n;
        }

        // Now process the more fancy processing instructions
        if ( GetProcessingInstructions( dir, instructions.processingInstructions ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Parsing advanced processing instructions for directory \"" + dir + "\"" );
            ParseProcessingInstructions( projectInfo, dir, instructions );
        }
        return &instructions;
    }

    return GUCEF_NULL;
}

/*---------------------------------------------------------------------------*/

const TDirProcessingInstructions*
GetProcessingInstructions( const CProjectInfo& projectInfo ,
                           const CORE::CString& dir        )
{GUCEF_TRACE;

    TDirProcessingInstructionsMap::const_iterator i = projectInfo.dirProcessingInstructions.find( dir );
    if ( i != projectInfo.dirProcessingInstructions.end() )
    {
        return &( (*i).second );
    }
    return GUCEF_NULL;
}

/*---------------------------------------------------------------------------*/

void
LoadAllProcessingInstructions( const CProjectInfo& projectInfo                          ,
                               const CORE::CString& rootDir                             ,
                               TDirProcessingInstructionsMap& newProcessingInstructions )
{GUCEF_TRACE;

    // Load instructions for the root dir itself
    const TDirProcessingInstructions* dirInstructions = GetProcessingInstructions( projectInfo, rootDir, newProcessingInstructions );

    // Recurse through sub-dirs to find more instructions
    CORE::SDI_Data* sdiData = CORE::DI_First_Dir_Entry( rootDir.C_String() );
    if ( NULL != sdiData )
    {
        do
        {
            // make sure we are dealing with a directory
            if ( 0 == DI_Is_It_A_File( sdiData ) )
            {
                CORE::CString dirName = DI_Name( sdiData );
                if ( ( dirName != "." ) && ( dirName != ".." ) )
                {
                    CORE::CString subRoot = rootDir;
                    CORE::AppendToPath( subRoot, dirName );

                    // Optimization: Check to see if the sub-dir is excluded
                    // If so then we don't even look for processing instructions in said sub-dir
                    // Note that this optimization can only be applied for dirs that are excluded on all platforms
                    bool skipSubDir = false;
                    if ( IsStringInList( projectInfo.globalDirExcludeList, true, dirName, true ) )
                    {
                        skipSubDir = true;
                        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Skipping the search for processing instructions, based on global dir exclude list, for dir \"" + subRoot + "\"" );
                    }
                    else
                    if ( NULL != dirInstructions )
                    {
                        TStringVectorMap::const_iterator i = dirInstructions->dirExcludeList.find( AllPlatforms );
                        if ( i != dirInstructions->dirExcludeList.end() )
                        {
                            skipSubDir = IsStringInList( (*i).second, true, dirName, true );
                            if ( skipSubDir )
                            {
                                GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Skipping the search for processing instructions, based on already located processing instructions, for dir \"" + subRoot + "\"" );
                            }
                        }
                    }

                    if ( !skipSubDir )
                    {
                        // Recurse into sub-dir
                        LoadAllProcessingInstructions( projectInfo, subRoot, newProcessingInstructions );
                    }
                }
            }
        }
        while ( 0 != DI_Next_Dir_Entry( sdiData ) );
        DI_Cleanup( sdiData );
    }
}

/*---------------------------------------------------------------------------*/

void
ExcludeOrIncludeDirEntriesAsSpecifiedForDir( const CProjectInfo& projectInfo                          ,
                                             const CORE::CString& dir                                 ,
                                             const CORE::CString& platform                            ,
                                             bool applyPlatformChangesOnly                            ,
                                             TStringSet& allEntries                                   ,
                                             TDirProcessingInstructionsMap& newProcessingInstructions )
{GUCEF_TRACE;

    // Fetch processing instructions from directory
    const TDirProcessingInstructions* instructionStorage = GetProcessingInstructions( projectInfo, dir, newProcessingInstructions );

    // Perform processing of global dir excludes
    TStringVector::const_iterator n = projectInfo.globalDirExcludeList.begin();
    while ( n != projectInfo.globalDirExcludeList.end() )
    {
        if ( RemoveString( allEntries, (*n) ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Excluded the dir \"" + (*n) + "\" based on the global dir exclude list" );
        }
        ++n;
    }

    if ( GUCEF_NULL != instructionStorage )
    {
        // Carry out the process using the fetched instructions
        ExcludeOrIncludeDirEntriesAsSpecifiedForDir( dir, *instructionStorage, platform, applyPlatformChangesOnly, allEntries );
    }
}

/*---------------------------------------------------------------------------*/

void
ExcludeFileEntriesAsSpecifiedForDir( const CProjectInfo& projectInfo                          ,
                                     const CORE::CString& dir                                 ,
                                     const CORE::CString& platform                            ,
                                     bool applyPlatformChangesOnly                            ,
                                     TStringSet& allEntries                                   ,
                                     TDirProcessingInstructionsMap& newProcessingInstructions )
{GUCEF_TRACE;

    // Fetch processing instructions from directory
    const TDirProcessingInstructions* instructionStorage = GetProcessingInstructions( projectInfo, dir, newProcessingInstructions );

    if ( GUCEF_NULL != instructionStorage )
    {
        // Carry out the process using the fetched instructions
        ExcludeFileEntriesAsSpecifiedForDir( *instructionStorage, platform, applyPlatformChangesOnly, allEntries );
    }
}

/*---------------------------------------------------------------------------*/

void
IncludeFileEntriesAsSpecifiedForDir( const CProjectInfo& projectInfo                          ,
                                     const CORE::CString& dir                                 ,
                                     const CORE::CString& platform                            ,
                                     const CORE::CString& currentPath                         ,
                                     bool applyPlatformChangesOnly                            ,
                                     TStringSetMap& allEntries                                ,
                                     const TStringVector& fileTypes                           ,
                                     TDirProcessingInstructionsMap& newProcessingInstructions )
{GUCEF_TRACE;

    // Fetch processing instructions from directory
    const TDirProcessingInstructions* instructionStorage = GetProcessingInstructions( projectInfo, dir, newProcessingInstructions );

    if ( GUCEF_NULL != instructionStorage )
    {
        // Carry out the process using the fetched instructions
        IncludeFileEntriesAsSpecifiedForDir( *instructionStorage      ,
                                             platform                 ,
                                             currentPath              ,
                                             applyPlatformChangesOnly ,
                                             allEntries               ,
                                             fileTypes                );
    }
}

/*---------------------------------------------------------------------------*/

void
ExcludeOrIncludeDirEntriesAsSpecifiedForDir( const CProjectInfo& projectInfo                          ,
                                             const CORE::CString& dir                                 ,
                                             TStringSet& allEntries                                   ,
                                             TDirProcessingInstructionsMap& newProcessingInstructions )
{GUCEF_TRACE;

    ExcludeOrIncludeDirEntriesAsSpecifiedForDir( projectInfo               ,
                                                 dir                       ,
                                                 CORE::CString::Empty      ,
                                                 false                     ,
                                                 allEntries                ,
                                                 newProcessingInstructions );
}

/*---------------------------------------------------------------------------*/

void
ExcludeFileEntriesAsSpecifiedForDir( const CProjectInfo& projectInfo                          ,
                                     const CORE::CString& dir                                 ,
                                     TStringSet& allEntries                                   ,
                                     TDirProcessingInstructionsMap& newProcessingInstructions )
{GUCEF_TRACE;

    ExcludeFileEntriesAsSpecifiedForDir( projectInfo               ,
                                         dir                       ,
                                         CORE::CString::Empty      ,
                                         false                     ,
                                         allEntries                ,
                                         newProcessingInstructions );
}

/*---------------------------------------------------------------------------*/

void
IncludeFileEntriesAsSpecifiedForDir( const CProjectInfo& projectInfo                          ,
                                     const CORE::CString& dir                                 ,
                                     TStringSetMap& allEntries                                ,
                                     const TStringVector& fileTypes                           ,
                                     const CORE::CString& currentPath                         ,
                                     TDirProcessingInstructionsMap& newProcessingInstructions )
{GUCEF_TRACE;

    IncludeFileEntriesAsSpecifiedForDir( projectInfo               ,
                                         dir                       ,
                                         CORE::CString::Empty      ,
                                         currentPath               ,
                                         false                     ,
                                         allEntries                ,
                                         fileTypes                 ,
                                         newProcessingInstructions );
}

/*-------------------------------------------------------------------------*/

void
PopulateFileListFromDir( const CProjectInfo& projectInfo ,
                         const CORE::CString& path       ,
                         const TStringVector& fileTypes  ,
                         TStringSet& fileList            ,
                         const CORE::CString& platform   )
{GUCEF_TRACE;

    if ( platform.IsNULLOrEmpty() || platform == AllPlatforms )
    {
        // current dir cannot be a platform dir
        if ( IsDirAPlatformDir( projectInfo, path, true ) )
            return;
    }
    else
    {
        // current dir must be a dir which is considered to be a platform dir
        // for the platform specified
        if ( !IsDirAPlatformDirForPlatform( projectInfo, path, platform, true ) )
            return;
    }

    CORE::SDI_Data* sdiData = CORE::DI_First_Dir_Entry( path.C_String() );
    if ( NULL != sdiData )
    {
        do
        {
            if ( 0 != DI_Is_It_A_File( sdiData ) )
            {
                CORE::CString filename = DI_Name( sdiData );
                CORE::CString fileExt = CORE::ExtractFileExtention( filename );

                if ( IsStringInList( fileTypes, false, fileExt ) )
                {
                    fileList.insert( filename );
                }
            }
        }
        while ( 0 != DI_Next_Dir_Entry( sdiData ) );
        DI_Cleanup( sdiData );
    }
}

/*---------------------------------------------------------------------------*/

void
PopulateDirListFromDir( const CProjectInfo& projectInfo ,
                        const CORE::CString& path       ,
                        TStringSet& dirList             ,
                        const CORE::CString& platform   ,
                        bool excludeGenericDirs         )
{GUCEF_TRACE;

    if ( ( platform.IsNULLOrEmpty() || platform == AllPlatforms ) || !excludeGenericDirs )
    {
        // Get a list of all platform dirs
        const TStringSet& platformsDirs = GetSupportedPlatformDirs( projectInfo );

        // Go through the dir adding dirs which qualify
        CORE::SDI_Data* sdiData = CORE::DI_First_Dir_Entry( path.C_String() );
        if ( NULL != sdiData )
        {
            do
            {
                if ( 0 == DI_Is_It_A_File( sdiData ) )
                {
                    // Add the dir if it is a real dir and if its not a platform dir
                    CORE::CString dirName = DI_Name( sdiData );
                    if ( ( dirName != "." ) && ( dirName != ".." )               &&
                         ( platformsDirs.find( dirName ) == platformsDirs.end() ) )
                    {
                        dirList.insert( dirName );
                    }
                }
            }
            while ( 0 != DI_Next_Dir_Entry( sdiData ) );
            DI_Cleanup( sdiData );
        }
    }

    if ( !platform.IsNULLOrEmpty() && platform != AllPlatforms )
    {
        // We are looking for dirs for the given platform
        const TStringSetMap& platformsDirMap = GetSupportedPlatformDirMap( projectInfo );
        TStringSetMap::const_iterator i = platformsDirMap.find( platform );
        if ( i != platformsDirMap.end() )
        {
            const TStringSet& dirsForPlatform = (*i).second;

            // Check each dir to see if it exists
            CORE::SDI_Data* sdiData = CORE::DI_First_Dir_Entry( path.C_String() );
            if ( NULL != sdiData )
            {
                do
                {
                    if ( 0 == DI_Is_It_A_File( sdiData ) )
                    {
                        // Add the dir if it is a real dir and if its a platform dir for
                        // the specified platform
                        CORE::CString dirName = DI_Name( sdiData );
                        if ( ( dirName != "." ) && ( dirName != ".." )                               &&
                             ( dirsForPlatform.find( dirName.Lowercase() ) != dirsForPlatform.end() ) )
                        {
                            dirList.insert( dirName );
                        }
                    }
                }
                while ( 0 != DI_Next_Dir_Entry( sdiData ) );
                DI_Cleanup( sdiData );
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

void
GetListOfAllModuleDirs( CModuleInfoPtr& moduleInfo   ,
                        TStringSet& moduleDirs       ,
                        bool relativePaths           ,
                        const CORE::CString& rootDir )
{GUCEF_TRACE;

    TStringSetMap::const_iterator i = moduleInfo->GetIncludeDirs().begin();
    while ( i != moduleInfo->GetIncludeDirs().end() )
    {
        if ( relativePaths )
        {
            moduleDirs.insert( (*i).first );
        }
        else
        {
            CORE::CString fullPath = rootDir;
            CORE::AppendToPath( fullPath, (*i).first );

            moduleDirs.insert( fullPath );
        }
        ++i;
    }
    i = moduleInfo->GetSourceDirs().begin();
    while ( i != moduleInfo->GetSourceDirs().end() )
    {
        if ( relativePaths )
        {
            moduleDirs.insert( (*i).first );
        }
        else
        {
            CORE::CString fullPath = rootDir;
            CORE::AppendToPath( fullPath, (*i).first );

            moduleDirs.insert( fullPath );
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
GetListOfAllModuleDirs( CModuleInfoEntryPtr& moduleInfoEntry ,
                        TStringSet& moduleDirs               ,
                        bool relativePaths                   ,
                        const CORE::CString& platform        )
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator i = moduleInfoEntry->GetModulesPerPlatform().find( platform );
    if ( i != moduleInfoEntry->GetModulesPerPlatform().end() )
    {
        CModuleInfoPtr moduleInfo = (*i).second;
        GetListOfAllModuleDirs( moduleInfo               ,
                                moduleDirs               ,
                                relativePaths            ,
                                moduleInfoEntry->rootDir );
    }
}

/*---------------------------------------------------------------------------*/

void
GetListOfAllModuleDirs( const CProjectInfo& projectInfo      ,
                        CModuleInfoEntryPtr& moduleInfoEntry ,
                        TStringSet& moduleDirs               ,
                        bool relativePaths                   )
{GUCEF_TRACE;

    // Add generic dirs
    GetListOfAllModuleDirs( moduleInfoEntry ,
                            moduleDirs      ,
                            relativePaths   ,
                            AllPlatforms    );

    // Add all platform dirs
    const TStringSet& platforms = GetSupportedPlatforms( projectInfo );
    TStringSet::const_iterator i = platforms.begin();
    while ( i != platforms.end() )
    {
        GetListOfAllModuleDirs( moduleInfoEntry ,
                                moduleDirs      ,
                                relativePaths   ,
                                (*i)            );
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

TStringVector
CMakeParseFileLines( const CORE::CString& fileSuffix )
{GUCEF_TRACE;

    CORE::CString testStr = fileSuffix.ReplaceChar( '\r', '\n' );
    testStr = testStr.CompactRepeatingChar( '\n' );
    return testStr.ParseElements( '\n', false );
}

/*---------------------------------------------------------------------------*/

TStringSet
CMakeParseIncludeDirs( const CORE::CString& fileSuffix )
{GUCEF_TRACE;

    TStringSet includeDirs;

    TStringVector suffixFileLines = CMakeParseFileLines( fileSuffix );
    TStringVector::iterator i = suffixFileLines.begin();
    while ( i != suffixFileLines.end() )
    {
        CORE::CString testStr = (*i).Lowercase();
        CORE::Int32 subStrIdx = testStr.HasSubstr( "include_directories(", true );
        CORE::Int32 commentCharIdx = testStr.HasChar( '#', true );

        // Is this function call commented out?
        if ( commentCharIdx > -1 && commentCharIdx < subStrIdx )
        {
            // Then ignore it
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Found include_directories function in suffix file but it is commented out, ignoring it" );
            ++i;
            continue;
        }

        // Does this line have the function call we are looking for?
        if ( subStrIdx >= 0 )
        {
            // It does, parse the parameters
            CORE::CString dependenciesStr = (*i).SubstrToChar( ')', (CORE::UInt32)subStrIdx+20, true );
            dependenciesStr = dependenciesStr.CompactRepeatingChar( ' ' );
            dependenciesStr = dependenciesStr.Trim( true );
            dependenciesStr = dependenciesStr.Trim( false );
            TStringVector elements = dependenciesStr.ParseElements( ' ' );
            if ( !elements.empty() )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Found " + CORE::ToString( elements.size() ) + " include dirs in suffix file" );
            }

            // Add this collection to the list of all include dirs we found
            TStringVector::iterator n = elements.begin();
            while ( n != elements.end() )
            {
                includeDirs.insert( (*n) );
                ++n;
            }
        }
        ++i;
    }
    return includeDirs;
}

/*---------------------------------------------------------------------------*/

void
CMakeParseSuffixFile( CModuleInfo& moduleInfo, const CORE::CString& cmakeListSuffixFileContent )
{GUCEF_TRACE;

    TStringVector suffixFileLines = CMakeParseFileLines( cmakeListSuffixFileContent );
    TStringVector::iterator i = suffixFileLines.begin();
    while ( i != suffixFileLines.end() )
    {
        CORE::CString testStr = (*i).Lowercase();
        CORE::Int32 subStrIdx = testStr.HasSubstr( "target_link_libraries(", true );
        CORE::Int32 commentCharIdx = testStr.HasChar( '#', true );

        // Is this function call commented out?
        if ( commentCharIdx > -1 && commentCharIdx < subStrIdx )
        {
            // Then ignore it
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Found target_link_libraries function in suffix file but it is commented out, ignoring it" );
            ++i;
            continue;
        }

        // Does this line have the function call we are looking for?
        if ( subStrIdx >= 0 )
        {
            // It does, parse the parameters
            CORE::CString dependenciesStr = (*i).SubstrToChar( ')', (CORE::UInt32)subStrIdx+22, true );
            dependenciesStr = dependenciesStr.CompactRepeatingChar( ' ' );
            dependenciesStr = dependenciesStr.Trim( true );
            dependenciesStr = dependenciesStr.Trim( false );
            TStringVector elements = dependenciesStr.ParseElements( ' ' );
            if ( !elements.empty() )
            {
                // the first element is the name of the module, we don't need it
                elements.erase( elements.begin() );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Found " + CORE::ToString( elements.size() ) + " libraries to link to in suffix file" );
            }

            // Add this collection to the list of all linked libraries we found
            TStringVector::iterator n = elements.begin();
            while ( n != elements.end() )
            {
                CLinkedLibrarySettingsPtr linkedLibrary;
                if ( moduleInfo.linkerSettings.TryGetLinkedLibrary( (*n), linkedLibrary, true ) )
                    linkedLibrary->SetModuleType( MODULETYPE_UNDEFINED );
                ++n;
            }
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

TStringSet
CMakeParseDependencies( const CORE::CString& fileSuffix ,
                        CORE::CString& moduleName       )
{GUCEF_TRACE;

    TStringSet dependencies;

    TStringVector suffixFileLines = CMakeParseFileLines( fileSuffix );
    TStringVector::iterator i = suffixFileLines.begin();
    while ( i != suffixFileLines.end() )
    {
        CORE::CString testStr = (*i).Lowercase();
        CORE::Int32 subStrIdx = testStr.HasSubstr( "add_dependencies(", true );
        CORE::Int32 commentCharIdx = testStr.HasChar( '#', true );

        // Is this function call commented out?
        if ( commentCharIdx > -1 && commentCharIdx < subStrIdx )
        {
            // Then ignore it
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Found add_dependencies function in suffix file but it is commented out, ignoring it" );
            ++i;
            continue;
        }

        // Does this line have the function call we are looking for?
        if ( subStrIdx >= 0 )
        {
            // It does, parse the parameters
            CORE::CString dependenciesStr = (*i).SubstrToChar( ')', (CORE::UInt32)subStrIdx+17, true );
            dependenciesStr = dependenciesStr.CompactRepeatingChar( ' ' );
            dependenciesStr = dependenciesStr.Trim( true );
            dependenciesStr = dependenciesStr.Trim( false );
            TStringVector elements = dependenciesStr.ParseElements( ' ' );
            if ( !elements.empty() )
            {
                moduleName = *(elements.begin());
                elements.erase( elements.begin() );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Found " + CORE::ToString( elements.size() ) + " dependencies in suffix file" );
            }

            // Add this collection to the list of all dependencies we found
            TStringVector::iterator n = elements.begin();
            while ( n != elements.end() )
            {
                dependencies.insert( (*n) );
                ++n;
            }
        }
        ++i;
    }
    return dependencies;
}

/*---------------------------------------------------------------------------*/

bool
CMakeParseModuleProperties( const CORE::CString& fileSuffix ,
                            CORE::CString& moduleName       ,
                            bool& isExecutable              )
{GUCEF_TRACE;

    CORE::CString testStr = fileSuffix.Lowercase();
    CORE::Int32 subStrIdx = testStr.HasSubstr( "add_library(", true );
    if ( subStrIdx >= 0 )
    {
        isExecutable = false;

        CORE::CString dependenciesStr = fileSuffix.SubstrToChar( ')', (CORE::UInt32)subStrIdx+12, true );
        dependenciesStr = dependenciesStr.CompactRepeatingChar( ' ' );
        dependenciesStr = dependenciesStr.Trim( true );
        dependenciesStr = dependenciesStr.Trim( false );
        TStringVector elements = dependenciesStr.ParseElements( ' ' );
        if ( !elements.empty() )
        {
            moduleName = *(elements.begin());
            return true;
        }

        // Malformed file!
        return false;
    }
    else
    {
        subStrIdx = testStr.HasSubstr( "add_executable(", true );
        if ( subStrIdx >= 0 )
        {
            isExecutable = true;

            CORE::CString dependenciesStr = fileSuffix.SubstrToChar( ')', (CORE::UInt32)subStrIdx+15, true );
            dependenciesStr = dependenciesStr.CompactRepeatingChar( ' ' );
            dependenciesStr = dependenciesStr.Trim( true );
            dependenciesStr = dependenciesStr.Trim( false );
            TStringVector elements = dependenciesStr.ParseElements( ' ' );
            if ( !elements.empty() )
            {
                moduleName = *(elements.begin());
                return true;
            }

            // Malformed file!
            return false;
        }
    }

    return false;
}

/*---------------------------------------------------------------------------*/

const CModuleInfoPtr
GetModuleInfo( const CProjectInfo& projectInfo ,
               const CORE::CString& moduleName ,
               const CORE::CString& platform   )
{GUCEF_TRACE;

    TModuleInfoEntryPtrVector::const_iterator i = projectInfo.modules.begin();
    while ( i != projectInfo.modules.end() )
    {
        CModuleInfoPtr moduleInfo;
        const CORE::CString* nameOfCurrentModule = (*i)->GetModuleName( platform, &moduleInfo );
        if ( NULL != nameOfCurrentModule )
        {
            if ( *nameOfCurrentModule == moduleName )
            {
                return moduleInfo;
            }
        }
        ++i;
    }
    return CModuleInfoPtr();
}

/*---------------------------------------------------------------------------*/

void
FindSubDirsWithFileTypes( const CProjectInfo& projectInfo                          ,
                          TStringSetMap& fileMap                                   ,
                          const TStringVector& fileTypes                           ,
                          const CORE::CString& platform                            ,
                          bool applyOnlyPlatformInstructions                       ,
                          const CORE::CString& curRootDir                          ,
                          const CORE::CString& curRootDirSeg                       ,
                          TDirProcessingInstructionsMap& newProcessingInstructions )
{GUCEF_TRACE;

    TStringSet fileList;
    PopulateFileListFromDir( projectInfo, curRootDir, fileTypes, fileList, platform );

    // Now we add/subtract files based on generator instructions
    ExcludeFileEntriesAsSpecifiedForDir( projectInfo                   ,
                                         curRootDir                    ,
                                         platform                      ,
                                         applyOnlyPlatformInstructions ,
                                         fileList                      ,
                                         newProcessingInstructions     );
    IncludeFileEntriesAsSpecifiedForDir( projectInfo                   ,
                                         curRootDir                    ,
                                         platform                      ,
                                         curRootDirSeg                 ,
                                         applyOnlyPlatformInstructions ,
                                         fileMap                       ,
                                         fileTypes                     ,
                                         newProcessingInstructions     );

    if ( fileList.size() > 0 )
    {
        // found files in the current root
        TStringSetMap::iterator i = fileMap.find( curRootDirSeg );
        if ( i == fileMap.end() )
        {
            if ( !fileMap.insert( std::pair< CORE::CString, TStringSet >( curRootDirSeg, fileList ) ).second )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to add files to the file map for subdir \"" + curRootDirSeg + "\"" );
            }
        }
        else
        {
            // We already have a list of files for this dir,.. merge
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Subdir \"" + curRootDirSeg + "\" already has files defined that "
                     "should be used for the module, this list of files will be merged with files we automatically locate in the same dir" );

            MergeStringSet( (*i).second, fileList, true );
        }
    }

    // Get a list of sub-dirs
    TStringSet dirList;
    PopulateDirListFromDir( projectInfo, curRootDir, dirList, platform, false );

    // Now we add/subtract dirs based on generator instructions
    ExcludeOrIncludeDirEntriesAsSpecifiedForDir( projectInfo                   ,
                                                 curRootDir                    ,
                                                 platform                      ,
                                                 applyOnlyPlatformInstructions ,
                                                 dirList                       ,
                                                 newProcessingInstructions     );

    TStringSet::iterator i = dirList.begin();
    while ( i != dirList.end() )
    {
        CORE::CString subDir = CORE::CombinePath( curRootDir, (*i) );

        // Do not recurse into other module dirs
        if ( !IsDirAProjectDir( subDir ) )
        {
            CORE::CString subDirSeg = curRootDirSeg;
            subDirSeg = CORE::CombinePath( subDirSeg, (*i) );
            subDirSeg = subDirSeg.ReplaceChar( '\\', '/' );

            FindSubDirsWithFileTypes( projectInfo                   ,
                                      fileMap                       ,
                                      fileTypes                     ,
                                      platform                      ,
                                      applyOnlyPlatformInstructions ,
                                      subDir                        ,
                                      subDirSeg                     ,
                                      newProcessingInstructions     );
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
FillHeaderSubDirIncludes( const CProjectInfo& projectInfo      ,
                          CModuleInfoEntryPtr& moduleInfoEntry ,
                          const CORE::CString& platform        ,
                          const TStringSetMap& exclusions      )
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator n = moduleInfoEntry->GetModulesPerPlatform().find( platform );
    if ( n != moduleInfoEntry->GetModulesPerPlatform().end() )
    {
        CModuleInfoPtr moduleInfo = (*n).second;

        if ( MODULETYPE_HEADER_INCLUDE_LOCATION == moduleInfo->moduleType )
        {
            // the very notion of a header include location is that it is a location
            // as such we include the root dir as a include dir
            moduleInfo->AddIncludeFiles( CORE::CString::Empty, CORE::CString::EmptyStringSet );
        }
        
        if ( !moduleInfo->hasConsiderSubDirs || ( moduleInfo->hasConsiderSubDirs && moduleInfo->considerSubDirs ) )
        {
            TStringSetMap newDirs;
            TStringSetMap::const_iterator d = moduleInfo->GetIncludeDirs().begin();
            while ( d != moduleInfo->GetIncludeDirs().end() )
            {
                const CORE::CString fullDir = (*d).first;
                TStringVector elements = fullDir.ParseElements( '/', false );
                if ( elements.size() > 1 )
                {
                    CORE::CString segments;
                    TStringVector::iterator s = elements.begin();
                    while ( s != elements.end() )
                    {
                        if ( segments.IsNULLOrEmpty() )
                            segments = (*s);
                        else
                            segments += '/' + (*s);

                        if ( exclusions.find( segments ) == exclusions.end() )
                            newDirs[ segments ];
                        ++s;
                    }
                }
                ++d;
            }
            if ( !newDirs.empty() )
                moduleInfo->AddIncludeFiles( newDirs );
        }
    }
}

/*---------------------------------------------------------------------------*/

void
FillHeaderSubDirIncludes( const CProjectInfo& projectInfo      ,
                          CModuleInfoEntryPtr& moduleInfoEntry )
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator n = moduleInfoEntry->GetModulesPerPlatform().find( AllPlatforms );
    if ( n != moduleInfoEntry->GetModulesPerPlatform().end() )
    {
        TStringSetMap dummy;
        FillHeaderSubDirIncludes( projectInfo     ,
                                  moduleInfoEntry ,
                                  AllPlatforms    ,
                                  dummy           );

        CModuleInfoPtr allPlatformsModuleInfo = (*n).second;
        TModuleInfoPtrMap::const_iterator m = moduleInfoEntry->GetModulesPerPlatform().begin();
        while ( m != moduleInfoEntry->GetModulesPerPlatform().end() )
        {
            if ( !(*m).first.IsNULLOrEmpty() && (*m).first != AllPlatforms )
            {
                FillHeaderSubDirIncludes( projectInfo                         ,
                                          moduleInfoEntry                     ,
                                          (*m).first                          ,
                                          allPlatformsModuleInfo->GetIncludeDirs() );
            }
            ++m;
        }
    }
}

/*---------------------------------------------------------------------------*/

void
FindSubDirsWithHeaders( const CProjectInfo& projectInfo                          ,
                        CModuleInfoEntryPtr& moduleInfoEntry                     ,
                        const CORE::CString& platform                            ,
                        TDirProcessingInstructionsMap& newProcessingInstructions )
{GUCEF_TRACE;

    TStringSetMap fileMap;
    FindSubDirsWithFileTypes( projectInfo               ,
                              fileMap                   ,
                              GetHeaderFileExtensions() ,
                              platform                  ,
                              false                     ,
                              moduleInfoEntry->rootDir  ,
                              CORE::CString::Empty      ,
                              newProcessingInstructions );

    if ( !fileMap.empty() )
    {
        TModuleInfoPtrMap::const_iterator i = moduleInfoEntry->GetModulesPerPlatform().find( platform );
        if ( i != moduleInfoEntry->GetModulesPerPlatform().end() )
        {
            CModuleInfoPtr moduleInfo = (*i).second;
            if ( !moduleInfo->hasConsiderSubDirs || ( moduleInfo->hasConsiderSubDirs && moduleInfo->considerSubDirs ) )
            {
                moduleInfo->AddIncludeFiles( fileMap );
            }
            else
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Skipping processing subdirs since recursive processing is disabled" );
            }
        }
        else
        {
            // If the files we are looking for are not by definition for a specific
            // platform via a platform dir then how they are processed depends on
            // the module definition. If the module is specific to a certain platform
            // then we don't want to add a all platforms definition but rather apply
            // the files to all platforms for which the module is valid according to
            // its module definition. This is a critical difference between 'AllPlatforms'
            // and specific platforms which is important for proper overrides/merging etc
            if ( AllPlatforms == platform || platform.IsNULLOrEmpty() )
            {
                i = moduleInfoEntry->GetModulesPerPlatform().begin();
                while ( i != moduleInfoEntry->GetModulesPerPlatform().end() )
                {
                    CModuleInfoPtr moduleInfo = (*i).second;
                    if ( !moduleInfo->hasConsiderSubDirs || ( moduleInfo->hasConsiderSubDirs && moduleInfo->considerSubDirs ) )
                    {
                        moduleInfo->AddIncludeFiles( fileMap );
                    }
                    else
                    {
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Skipping processing subdirs since recursive processing is disabled" );
                    }
                    ++i;
                }
            }
            else
            {
                CModuleInfoPtr moduleInfo = moduleInfoEntry->FindOrCreateModuleInfoForPlatform( platform, true );
                moduleInfo->SetIncludeFiles( fileMap );
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

void
FindSubDirsWithHeaders( const CProjectInfo& projectInfo                          ,
                        CModuleInfoEntryPtr& moduleInfoEntry                     ,
                        TDirProcessingInstructionsMap& newProcessingInstructions )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Locating headers which apply to all platforms" );

    // Add all generic headers
    FindSubDirsWithHeaders( projectInfo               ,
                            moduleInfoEntry           ,
                            AllPlatforms              ,
                            newProcessingInstructions );

    // Add platform specific headers
    const TStringSet& platforms = GetSupportedPlatforms( projectInfo );
    TStringSet::const_iterator i = platforms.begin();
    while ( i != platforms.end() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Locating headers which apply to platform " + (*i) );

        FindSubDirsWithHeaders( projectInfo               ,
                                moduleInfoEntry           ,
                                (*i)                      ,
                                newProcessingInstructions );

        ++i;
    }

    // Various projects include part of the relative paths inside a include statement as a means of segregating
    // fairly generic file names from crashes, using dir names instead of more unique file names
    // Such projects are sensitive to the inclusion of header parent dirs as include dirs in order for the include paths
    // to be correct. To cover this case we add relative parent dirs of dirs with headers
    FillHeaderSubDirIncludes( projectInfo, moduleInfoEntry );
}

/*---------------------------------------------------------------------------*/

void
FindSubDirsWithSource( const CProjectInfo& projectInfo                          ,
                       CModuleInfoEntryPtr& moduleInfoEntry                     ,
                       const CORE::CString& platform                            ,
                       TDirProcessingInstructionsMap& newProcessingInstructions )
{GUCEF_TRACE;

    TStringSetMap fileMap;
    FindSubDirsWithFileTypes( projectInfo               ,
                              fileMap                   ,
                              GetSourceFileExtensions() ,
                              platform                  ,
                              false                     ,
                              moduleInfoEntry->rootDir  ,
                              CORE::CString::Empty      ,
                              newProcessingInstructions );

    if ( !fileMap.empty() )
    {
        TModuleInfoPtrMap::const_iterator i = moduleInfoEntry->GetModulesPerPlatform().find( platform );
        if ( i != moduleInfoEntry->GetModulesPerPlatform().end() )
        {
            CModuleInfoPtr moduleInfo = (*i).second;
            if ( !moduleInfo->hasConsiderSubDirs || ( moduleInfo->hasConsiderSubDirs && moduleInfo->considerSubDirs ) )
            {
                moduleInfo->AddSourceFiles( fileMap );
            }
            else
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Skipping processing subdirs since recursive processing is disabled" );
            }
        }
        else
        {
            // If the files we are looking for are not by definition for a specific
            // platform via a platform dir then how they are processed depends on
            // the module definition. If the module is specific to a certain platform
            // then we don't want to add a all platforms definition but rather apply
            // the files to all platforms for which the module is valid according to
            // its module definition. This is a critical difference between 'AllPlatforms'
            // and specific platforms which is important for proper overrides/merging etc
            if ( AllPlatforms == platform || platform.IsNULLOrEmpty() )
            {
                i = moduleInfoEntry->GetModulesPerPlatform().begin();
                while ( i != moduleInfoEntry->GetModulesPerPlatform().end() )
                    {
                    CModuleInfoPtr moduleInfo = (*i).second;
                    if ( !moduleInfo->hasConsiderSubDirs || ( moduleInfo->hasConsiderSubDirs && moduleInfo->considerSubDirs ) )
                    {
                        moduleInfo->AddSourceFiles( fileMap );
                    }
                    else
                    {
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Skipping processing subdirs since recursive processing is disabled" );
                    }

                    ++i;
                }
            }
            else
            {
                CModuleInfoPtr moduleInfo = moduleInfoEntry->FindOrCreateModuleInfoForPlatform( platform, true );
                moduleInfo->SetSourceFiles( fileMap );
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

void
FindSubDirsWithSource( const CProjectInfo& projectInfo                          ,
                       CModuleInfoEntryPtr& moduleInfoEntry                     ,
                       TDirProcessingInstructionsMap& newProcessingInstructions )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Locating sources which apply to all platforms" );

    // Add all generic sources
    FindSubDirsWithSource( projectInfo               ,
                           moduleInfoEntry           ,
                           AllPlatforms              ,
                           newProcessingInstructions );

    // Add platform specific source
    const TStringSet& platforms = GetSupportedPlatforms( projectInfo );
    TStringSet::const_iterator i = platforms.begin();
    while ( i != platforms.end() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Locating sources which apply to platform " + (*i) );

        FindSubDirsWithSource( projectInfo               ,
                               moduleInfoEntry           ,
                               (*i)                      ,
                               newProcessingInstructions );

        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
LegacyCMakeProcessProjectDir( const CProjectInfo& projectInfo      ,
                              CModuleInfoEntryPtr& moduleInfoEntry )
{GUCEF_TRACE;

    CORE::CString pathToSuffixFile = moduleInfoEntry->rootDir;
    CORE::AppendToPath( pathToSuffixFile, "CMakeListsSuffix.txt" );

    CORE::CString cmakeListSuffixFileContent;
    if ( !CORE::LoadTextFileAsString( pathToSuffixFile, cmakeListSuffixFileContent ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load legacy project file: " + pathToSuffixFile );
        return;
    }

    if ( CORE::LoadTextFileAsString( pathToSuffixFile, cmakeListSuffixFileContent ) )
    {
        // get a platform entry to use
        // this legacy system only supported AllPlatforms via CMake
        CModuleInfo& moduleInfo = *moduleInfoEntry->FindOrCreateModuleInfoForPlatform( AllPlatforms, true );

        // Set a project name based off the module sub-dir name
        // Best we can do unless we can get it from the suffix file later
        moduleInfo.name = CORE::LastSubDir( moduleInfoEntry->rootDir );

        // Fill in the dependencies as specified in the suffix file
        CORE::CString actualModuleName;
        moduleInfo.SetNamesOfDependencies( CMakeParseDependencies( cmakeListSuffixFileContent, actualModuleName ) );
        moduleInfo.dependencyIncludeDirs = CMakeParseIncludeDirs( cmakeListSuffixFileContent );
        bool isExecutable = false;
        CMakeParseModuleProperties( cmakeListSuffixFileContent, actualModuleName, isExecutable );

        // This legacy system only supports this simple test
        moduleInfo.moduleType = isExecutable ? MODULETYPE_EXECUTABLE : MODULETYPE_SHARED_LIBRARY;

        CMakeParseSuffixFile( moduleInfo, cmakeListSuffixFileContent );
        if ( actualModuleName != moduleInfo.name )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Detected module name that differs from module sub-dir name, correcting module name from \"" + moduleInfo.name + "\" to \"" + actualModuleName + "\"" );
            moduleInfo.name = actualModuleName;
        }

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Processed suffix file for project " + moduleInfo.name );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Unable to locate the required module file " + pathToSuffixFile );
    }
}

/*---------------------------------------------------------------------------*/

bool
ProcessProjectDir( CProjectInfoPtr projectInfo                                ,
                   CORE::CString rootDir                                      ,
                   TModuleInfoEntryPtrVectorPtr moduleInfoEntries             ,
                   TDirProcessingInstructionsMapPtr newProcessingInstructions )
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock projectInfoReaderLock( projectInfo.GetPointerAlways() );

    CORE::CString pathToModuleInfoFile = rootDir;
    CORE::AppendToPath( pathToModuleInfoFile, "ModuleInfo.xml" );

    if ( CORE::FileExists( pathToModuleInfoFile ) )
    {
        // temp hack: we need to ensure that the project info is set as user data on the settings object
        CORE::CDataNodeSerializableSettings settings;
        settings.SetUserData( projectInfo.GetPointerAlways() );

        // load the ModuleInfoEntry entries from the file
        // typically there is only 1 but its possible to have more
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Processing ModuleInfo file " + pathToModuleInfoFile );
        if ( projectInfo->DeserializeModuleEntries( pathToModuleInfoFile, settings, *moduleInfoEntries.GetPointer() ) )
        {
            // Do some extra processing which does not apply to the legacy cmake files...

            if ( !IsAnyLicenseDefined( *moduleInfoEntries.GetPointer() ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "No license information is available in the ModuleInfo" );

                CORE::CString licenceFilePath;
                if ( DirHasLicenseFile( rootDir, licenceFilePath ) )
                {
                    CORE::CString licenceFileContent;
                    if ( CORE::LoadTextFileAsString( licenceFilePath, licenceFileContent, false ) )
                    {
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully loaded license file: " + licenceFilePath );

                        CORE::CString detectedLicense;
                        if ( TryAutoLicenceDetection( licenceFileContent,
                                                      detectedLicense   ) )
                        {
                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Auto detected license: " + detectedLicense );

                            TModuleInfoEntryPtrVector::iterator i = moduleInfoEntries->begin();
                            while ( i != moduleInfoEntries->end() )
                            {
                                CModuleInfoEntryPtr& moduleInfoEntry = (*i);
                                moduleInfoEntry->SetLicense( detectedLicense, AllPlatforms );
                                ++i;
                            }
                        }
                    }
                    else
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load detected license file: " + licenceFilePath );
                    }
                }
            }

            if ( !IsAnySemVerDefined( *moduleInfoEntries.GetPointer() ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "No semver information is available in the ModuleInfo" );

                CORE::CString semverFilePath;
                if ( DirHasSemVerFile( rootDir, semverFilePath ) )
                {
                    CORE::CString semverFileContent;
                    if ( CORE::LoadTextFileAsString( semverFilePath, semverFileContent, false ) )
                    {
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully loaded semver file: " + semverFilePath );

                        CORE::CVersion detectedSemVer;
                        if ( TryAutoSemVerDetection( semverFileContent ,
                                                     detectedSemVer    ) )
                        {
                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Auto detected semver: " + detectedSemVer.ToString() );

                            TModuleInfoEntryPtrVector::iterator i = moduleInfoEntries->begin();
                            while ( i != moduleInfoEntries->end() )
                            {
                                CModuleInfoEntryPtr& moduleInfoEntry = (*i);
                                moduleInfoEntry->SetSemVer( detectedSemVer, AllPlatforms );
                                ++i;
                            }
                        }
                    }
                    else
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load detected semver file: " + semverFilePath );
                    }
                }
            }

            TModuleInfoEntryPtrVector::iterator i = moduleInfoEntries->begin();
            while ( i != moduleInfoEntries->end() )
            {
                CModuleInfoEntryPtr& moduleInfoEntry = (*i);

                // If there is any module info specified for 'AllPlatforms' but it does not have a
                // module name set then we shall determine a default which is the based on the directory the
                // project is in. Note that platform specific info can overwrite this of course but at least
                // this gives us a default if they don't have a name specified either.
                CModuleInfoPtr moduleInfo = moduleInfoEntry->FindOrCreateModuleInfoForPlatform( AllPlatforms, false );
                if ( !moduleInfo.IsNULL() && moduleInfo->name.IsNULLOrEmpty() )
                {
                    // Set a project name based off the module sub-dir name
                    moduleInfo->name = CORE::LastSubDir( moduleInfoEntry->rootDir );
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Setting module name to " + moduleInfo->name + " based on the sub-dir name because no other name is available" );
                }
                ++i;
            }
        }
    }
    else
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Attempting to process legacy cmake suffix file" );

        CModuleInfoEntryPtr moduleInfoEntry = CModuleInfoEntry::CreateSharedObj();
        moduleInfoEntry->rootDir = rootDir;
        LegacyCMakeProcessProjectDir( *projectInfo.GetPointer(), moduleInfoEntry );
        moduleInfoEntries->push_back( moduleInfoEntry );
    }

    TModuleInfoEntryPtrVector::iterator i = moduleInfoEntries->begin();
    while ( i != moduleInfoEntries->end() )
    {
        CModuleInfoEntryPtr& moduleInfoEntry = (*i);

        // Assign the rootdir to the entry, we don't save this inside the files
        moduleInfoEntry->rootDir = rootDir;

        FindSubDirsWithHeaders( *projectInfo.GetPointer(), moduleInfoEntry, *newProcessingInstructions.GetPointer() );
        FindSubDirsWithSource( *projectInfo.GetPointer(), moduleInfoEntry, *newProcessingInstructions.GetPointer() );

        // If we have a module name then use it for our logging output
        // we want to be able to see in the log which modules where successfully processed
        CORE::CString consensusModuleName = moduleInfoEntry->GetConsensusName();

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Processed module " + consensusModuleName + " from project dir: " + moduleInfoEntry->rootDir );

        ++i;
    }

    return true;
}

/*---------------------------------------------------------------------------*/

void
PreprocessDir( const CORE::CString& path )
{GUCEF_TRACE;

    CDirPreprocessorManager& dirPreprocessorManager = CProjectGenGlobal::Instance()->GetDirPreprocessorManager();
    const CDirPreprocessorManager::TDirPreprocessorsList& dirPreprocessorsList = dirPreprocessorManager.GetDirPreprocessors();

    if ( dirPreprocessorsList.empty() )
        return;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "There are " + CORE::ToString( dirPreprocessorsList.size() ) + " preprocessors registered" );

    CDirPreprocessorManager::TDirPreprocessorsList::const_iterator i = dirPreprocessorsList.begin();
    while ( i != dirPreprocessorsList.end() )
    {
        if ( (*i)->ProccessDir( path ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Preprocessed directory " + path );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Failed to preprocess directory " + path );
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
LocateModuleDirsRecursively( const CProjectInfo& projectInfo                          ,
                             const CORE::CString& topLevelDir                         ,
                             CORE::CStringSet& allProjectDirs                         ,
                             TDirProcessingInstructionsMap& newProcessingInstructions )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_EVERYTHING, "Recursively processing directory for module info: " + topLevelDir );

    // Run any custom preprocessing logic that's registered
    PreprocessDir( topLevelDir );

    // Is this a project dir or some other dir?
    if ( IsDirAProjectDir( topLevelDir ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Determined that the following directory is a project directory: " + topLevelDir );
        allProjectDirs.insert( topLevelDir );
    }

    // Get all subdir's
    TStringSet dirList;
    PopulateDirListFromDir( projectInfo, topLevelDir, dirList, AllPlatforms, false );

    // Add/subtract dirs from the list based on generator instructions
    // This early application (before module definition) of processing instructions allows us
    // to limit the number of directories that need to be processed and thus speed things up a bit
    ExcludeOrIncludeDirEntriesAsSpecifiedForDir( projectInfo, topLevelDir, dirList, newProcessingInstructions );

    // Process all sub-dirs
    TStringSet::iterator i = dirList.begin();
    while ( i != dirList.end() )
    {
        CORE::CString subDir = topLevelDir;
        CORE::AppendToPath( subDir, (*i) );

        LocateModuleDirsRecursively( projectInfo, subDir, allProjectDirs, newProcessingInstructions );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

TModuleInfoEntryPtrSet
FindModulesInfoEntryWhichDependOnModule( CProjectInfo& projectInfo           ,
                                         const CORE::CString& dependencyName )
{GUCEF_TRACE;

    // Loop trough all modules and check if they depend on the given module
    TModuleInfoEntryPtrSet resultList;
    TModuleInfoEntryPtrVector::iterator i = projectInfo.modules.begin();
    while ( i != projectInfo.modules.end() )
    {
        CModuleInfoEntryPtr& moduleInfoEntry = (*i);
        TModuleInfoPtrMap::const_iterator n = moduleInfoEntry->GetModulesPerPlatform().begin();
        while ( n != moduleInfoEntry->GetModulesPerPlatform().end() )
        {
            // Check if this module depends on the module we are looking for regardless of platform
            CModuleInfoPtr moduleInfo = (*n).second;
            if ( IsStringInList( moduleInfo->GetNamesOfDependencies(), false, dependencyName ) )
            {
                resultList.insert( moduleInfoEntry );
            }
            ++n;
        }

        ++i;
    }

    return resultList;
}

/*-------------------------------------------------------------------------*/

void
RemoveDependencyToModule( CProjectInfo& projectInfo       ,
                          const CORE::CString& moduleName )
{GUCEF_TRACE;

    // Loop trough all modules and process each as we go
    TModuleInfoEntryPtrVector::iterator i = projectInfo.modules.begin();
    while ( i != projectInfo.modules.end() )
    {
        CModuleInfoEntryPtr& moduleInfoEntry = (*i);
        TModuleInfoPtrMap::const_iterator n = moduleInfoEntry->GetModulesPerPlatform().begin();
        while ( n != moduleInfoEntry->GetModulesPerPlatform().end() )
        {
            CModuleInfoPtr moduleInfo = (*n).second;
            moduleInfo->RemoveNameOfDependency( moduleName );
            ++n;
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
RemoveDependenciesOnIntegrationLocations( CProjectInfo& projectInfo )
{GUCEF_TRACE;

    // Loop trough all modules and process each code include as we go
    TModuleInfoEntryPtrVector::iterator i = projectInfo.modules.begin();
    while ( i != projectInfo.modules.end() )
    {
        CModuleInfoEntryPtr& moduleInfoEntry = (*i);
        TModuleInfoPtrMap::const_iterator n = moduleInfoEntry->GetModulesPerPlatform().begin();
        while ( n != moduleInfoEntry->GetModulesPerPlatform().end() )
        {
            const CModuleInfoPtr& moduleInfo = (*n).second;
            if ( ( MODULETYPE_CODE_INTEGRATE_LOCATION == moduleInfo->moduleType ) ||
                 ( MODULETYPE_HEADER_INTEGRATE_LOCATION == moduleInfo->moduleType ) )
            {
                // We found a code include location, now process it for all modules which proclaim to have a dependency on it
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Removing dependency labeled as \"" + moduleInfo->name + "\" for platform " + (*n).first + " because it is an integration location and it has been processed" );
                RemoveDependencyToModule( projectInfo      ,
                                          moduleInfo->name );
            }
            ++n;
        }

        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
ProcessModuleInformation_MergeASyncTaskResult( CORE::CTaskPtr processModuleDirTask                        ,
                                               bool processModuleDirResult                                ,
                                               CProjectInfoPtr projectInfo                                ,
                                               CORE::CString rootDir                                      ,
                                               TModuleInfoEntryPtrVectorPtr newModuleInfoEntries          ,
                                               TDirProcessingInstructionsMapPtr newProcessingInstructions )
{GUCEF_TRACE;

    bool totalSuccess = true;

    MT::CObjectScopeLock projectInfoWriterLock( projectInfo.GetPointerAlways() );

    if ( !newProcessingInstructions.IsNULL() )
        totalSuccess = MergeDirProcessingInstructionsMap( projectInfo->dirProcessingInstructions, (*newProcessingInstructions), true ) && totalSuccess;

    if ( !newModuleInfoEntries.IsNULL() )
        totalSuccess = MergeModuleInfoEntries( (*newModuleInfoEntries), projectInfo->modules ) && totalSuccess;

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

void
ProcessModuleInformation( CORE::ThreadPoolPtr threadPool        ,
                          CProjectInfoPtr projectInfo           ,
                          const CORE::CStringSet& allModuleDirs )
{GUCEF_TRACE;

    bool totalSuccess = true;

    CORE::CASync asyncTasks( threadPool );
    CORE::CStringSet::const_iterator p = allModuleDirs.begin();
    while ( p != allModuleDirs.end() )
    {
        CORE::CString moduleDir = (*p);
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Queueing processing task for module dir: " + moduleDir );

        // queue task chain for this module dir
        TModuleInfoEntryPtrVectorPtr newModuleInfoEntries = TModuleInfoEntryPtrVectorPtr( GUCEF_NEW TModuleInfoEntryPtrVector() );
        TDirProcessingInstructionsMapPtr newProcessingInstructions = TDirProcessingInstructionsMapPtr( GUCEF_NEW TDirProcessingInstructionsMap() );
        
        CORE::CFutureResult future = asyncTasks.QueueCallback( &ProcessProjectDir        ,
                                                               projectInfo               ,
                                                               moduleDir                 ,
                                                               newModuleInfoEntries      , 
                                                               newProcessingInstructions ).ThenPassToCallback( &ProcessModuleInformation_MergeASyncTaskResult );
        if ( future.HasNoFuture() )
        {
            totalSuccess = false;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Encountered an error queueing processing task for module dir: " + moduleDir + " - TaskStatus: " + future.GetResult()->GetTaskStatusString() );
            asyncTasks.ClearChain();
        }

        ++p;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Waiting for all processing tasks to finish for discovered modules" );
    while ( !threadPool->WaitForAllTasksToFinish( 5000 ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Waiting for processing tasks to finish for discovered modules" );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Processing tasks all finished for the discovered modules" );
}

/*-------------------------------------------------------------------------*/

CDirCrawlingProjectInfoGatherer::CDirCrawlingProjectInfoGatherer( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDirCrawlingProjectInfoGatherer::~CDirCrawlingProjectInfoGatherer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDirCrawlingProjectInfoGatherer::GatherInfo( const TStringVector& rootDirs        ,
                                             CProjectInfoPtr projectInfo          ,
                                             const CORE::CValueList& params       ,
                                             const CORE::CString& threadPoolToUse )
{GUCEF_TRACE;

    m_threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( threadPoolToUse );
    //m_threadPool->SetNrOfWorkerThreadsToLogicalCPUs( 1 );
    m_threadPool->SetDesiredMinNrOfWorkerThreads( 1 );

    // Gather all processing instructions
    TDirProcessingInstructionsMap newProcessingInstructions;
    TStringVector::const_iterator i = rootDirs.begin();
    while ( i != rootDirs.end() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Recursively loading all processing instructions for root directory \"" + (*i) + "\"" );
        LoadAllProcessingInstructions( *projectInfo.GetPointerAlways(), CORE::RelativePath( (*i) ), newProcessingInstructions );
        ++i;
    }
    MergeDirProcessingInstructionsMap( projectInfo->dirProcessingInstructions, newProcessingInstructions, true );
    newProcessingInstructions.clear();

    // Gather all module information
    CORE::CStringSet allModuleDirs;
    i = rootDirs.begin();
    while ( i != rootDirs.end() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Identifying all modules for root directory \"" + (*i) + "\"" );

        CORE::CString resolvedPath = CORE::RelativePath( (*i), true );
        projectInfo->rootDirs.push_back( resolvedPath );

        LocateModuleDirsRecursively( *projectInfo.GetPointerAlways(), resolvedPath, allModuleDirs, newProcessingInstructions );

        ++i;
    }
    MergeDirProcessingInstructionsMap( projectInfo->dirProcessingInstructions, newProcessingInstructions, true );

    ProcessModuleInformation( m_threadPool, projectInfo, allModuleDirs );

    return projectInfo->BulkPostProcessAllModuleInfo();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
