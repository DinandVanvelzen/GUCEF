/*
 *  CMakeListGenerator: Tool to generate CMakeList.txt with headers and sources
 *  Copyright (C) 2002 - 2009.  Dinand Vanvelzen
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

#include <stdio.h>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

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

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifdef GUCEF_MSWIN_BUILD
#include <windows.h>
#endif /* GUCEF_MSWIN_BUILD ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*---------------------------------------------------------------------------*/

typedef CORE::CString::StringSet                    TStringSet;
typedef std::map< CORE::CString, CORE::CString >    TStringMap;
typedef std::map< CORE::CString, TStringSet >       TStringSetMap;
typedef CORE::CString::StringVector                 TStringVector;
typedef std::map< CORE::CString, TStringVector >    TStringVectorMap;
typedef std::map< CORE::CString, TStringVectorMap > TStringVectorMapMap;

/*---------------------------------------------------------------------------*/

struct SModuleInfo
{
    TStringVector dependencies;                  // list of module names of all modules this module depends on
    TStringVector linkedLibraries;               // list of all libraries the module links against

    TStringVectorMap includeDirs;                // include directories of this module's own headers
    TStringVectorMap sourceDirs;                 // source directories of this module's own source

    TStringSet dependencyIncludeDirs;            // include directories needed for the headers of the dependencies, paths only no files
    TStringSetMap dependencyPlatformIncludeDirs; // include directories needed for the headers of the dependencies which are platform specific, paths only no files

    TStringVectorMapMap platformHeaderFiles;     // platform specific headers files of this module
    TStringVectorMapMap platformSourceFiles;     // platform specific source files of this module

    CORE::CString name;                          // the name of the module
    CORE::CString rootDir;                       // the absolute path to the root of this module's directory tree
    bool isExecutable;                           // Whether the module is a library or an executable

    CORE::CString cmakeListFileContentPreSuffix;  // autogenerated content which prefixes the suffix file content
    CORE::CString cmakeListSuffixFileContent;     // manually written suffix file which will be merged into the final file
    CORE::CString cmakeListFileContentPostSuffix; // autogenerated content which postfixes the suffix file content

    int buildOrder;                               // order number of this module in the build queue
};
typedef struct SModuleInfo TModuleInfo;

/*---------------------------------------------------------------------------*/

typedef std::vector< TModuleInfo > TModuleInfoVector;
typedef std::vector< TModuleInfo* > TModuleInfoPtrVector;
typedef std::map< int, TModuleInfo* > TModulePrioMap;

/*---------------------------------------------------------------------------*/

struct SDirProcessingInstructions
{
    TStringVector dirExcludeList;              // list of directories that are to be excluded at all times
    TStringVectorMap dirPlatformExcludeList;   // list of directories that are to be excluded for the given platform
    TStringVector dirIncludeList;              // list of directories that are to be included at all times
    TStringVectorMap dirPlatformIncludeList;   // list of directories that are to be included for the given platform

    TStringVector fileExcludeList;              // list of files that are to be excluded at all times
    TStringVectorMap filePlatformExcludeList;   // list of files that are to be excluded for the given platform
    TStringVector fileIncludeList;              // list of files that are to be included at all times
    TStringVectorMap filePlatformIncludeList;   // list of files that are to be included for the given platform

    CORE::CDataNode processingInstructions; // All unparsed processing intruction data
};
typedef struct SDirProcessingInstructions TDirProcessingInstructions;

/*---------------------------------------------------------------------------*/

typedef std::map< CORE::CString, TDirProcessingInstructions > TDirProcessingInstructionsMap;

/*---------------------------------------------------------------------------*/

struct SProjectInfo
{
    TModuleInfoVector modules;                               // All generated module information
    TDirProcessingInstructionsMap dirProcessingInstructions; // All loaded processing instructions mapped per path
    TStringVector globalDirExcludeList;                      // Dirs that should never be included in processing regardless of path
};
typedef struct SProjectInfo TProjectInfo;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

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
    }
    return fileTypes;
}

/*---------------------------------------------------------------------------*/

const TStringVector&
GetHeaderFileExtensions( void )
{GUCEF_TRACE;

    static TStringVector fileTypes;

    if ( fileTypes.empty() )
    {
        fileTypes.push_back( "h" );
        fileTypes.push_back( "hpp" );
    }
    return fileTypes;
}

/*---------------------------------------------------------------------------*/

bool
RemoveString( CORE::CString::StringVector& list ,
              const CORE::CString& searchStr    )
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

/*---------------------------------------------------------------------------*/

bool
IsStringInList( const TStringVector& list       ,
                bool caseSensitive              ,
                const CORE::CString& testString )
{GUCEF_TRACE;

    TStringVector::const_iterator i = list.begin();
    while ( i != list.end() )
    {
        if ( (*i).Equals( testString, caseSensitive ) )
        {
            return true;
        }
        ++i;
    }
    return false;
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
IsDirAProjectDir( CORE::CString dir )
{GUCEF_TRACE;

    // The dir is a module dir if it has a suffix file in it
    CORE::CString suffixFilePath = dir;
    CORE::AppendToPath( suffixFilePath, "CMakeListsSuffix.txt" );

    return CORE::FileExists( suffixFilePath );
}

/*-------------------------------------------------------------------------*/

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

CORE::CString
GetProcessingInstructionsPath( const CORE::CString& dir )
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

    if ( CORE::FileExists( instructionsFile ) )
    {
        CORE::CDStoreCodecRegistry::TDStoreCodecPtr codecPtr = GetXmlDStoreCodec();
        if ( !codecPtr.IsNULL() )
        {
            if ( codecPtr->BuildDataTree( &instructions    ,
                                          instructionsFile ) )
            {
                return true;
            }
        }

    }
    return false;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GetExcludeListPath( const CORE::CString& dir )
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

    if ( CORE::FileExists( excludeFile ) )
    {
        CORE::CString excludeFileContent;
        if ( CORE::LoadTextFileAsString( excludeFile        ,
                                         excludeFileContent ) )
        {
            excludeFileContent = excludeFileContent.RemoveChar( '\r' );

            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Loaded simple exclude list for directory \"" + dir + "\"" );
            return excludeFileContent.ParseElements( '\n' );
        }
    }

    return TStringVector();
}

/*---------------------------------------------------------------------------*/

void
ExcludeOrIncludeEntriesAsSpecifiedForDir( const CORE::CString& dir                          ,
                                          const TDirProcessingInstructions& allInstructions ,
                                          const CORE::CString& platform                     ,
                                          bool checkDirs                                    ,
                                          bool applyPlatformChangesOnly                     ,
                                          TStringVector& allEntries                         )
{GUCEF_TRACE;

    if ( checkDirs )
    {
        if ( !applyPlatformChangesOnly )
        {
            // First we exclude based off of the simple exclude list
            const TStringVector& excludeList = allInstructions.dirExcludeList;
            TStringVector::const_iterator n = excludeList.begin();
            while ( n != excludeList.end() )
            {
                if ( RemoveString( allEntries, (*n) ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Excluded the directory \"" + (*n) + "\" based on the exclude list for this dir" );
                }
                ++n;
            }

            const TStringVector& includeList = allInstructions.dirIncludeList;
            n = includeList.begin();
            while ( n != includeList.end() )
            {
                allEntries.push_back( (*n) );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Included the directory \"" + (*n) + "\" based on the include list for this dir" );
                ++n;
            }
        }

        if ( !platform.IsNULLOrEmpty() )
        {
            TStringVectorMap::const_iterator i = allInstructions.dirPlatformExcludeList.find( platform );
            if ( i != allInstructions.dirPlatformExcludeList.end() )
            {
                const TStringVector& excludeList = (*i).second;
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

            i = allInstructions.dirPlatformIncludeList.find( platform );
            if ( i != allInstructions.dirPlatformIncludeList.end() )
            {
                const TStringVector& includeList = (*i).second;
                TStringVector::const_iterator n = includeList.begin();
                while ( n != includeList.end() )
                {
                    allEntries.push_back( (*n) );
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Included the directory \"" + (*n) + "\" based on the include list for this dir" );
                    ++n;
                }
            }
        }
    }
    else
    {
        if ( !applyPlatformChangesOnly )
        {
            // First we exclude based off of the simple exclude list
            const TStringVector& excludeList = allInstructions.fileExcludeList;
            TStringVector::const_iterator n = excludeList.begin();
            while ( n != excludeList.end() )
            {
                if ( RemoveString( allEntries, (*n) ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Excluded the file \"" + (*n) + "\" based on the exclude list for this dir" );
                }
                ++n;
            }

            const TStringVector& includeList = allInstructions.fileIncludeList;
            n = includeList.begin();
            while ( n != includeList.end() )
            {
                allEntries.push_back( (*n) );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Included the file \"" + (*n) + "\" based on the include list for this dir" );
                ++n;
            }
        }

        if ( !platform.IsNULLOrEmpty() )
        {
            TStringVectorMap::const_iterator i = allInstructions.filePlatformExcludeList.find( platform );
            if ( i != allInstructions.filePlatformExcludeList.end() )
            {
                const TStringVector& excludeList = (*i).second;
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

            i = allInstructions.filePlatformIncludeList.find( platform );
            if ( i != allInstructions.filePlatformIncludeList.end() )
            {
                const TStringVector& includeList = (*i).second;
                TStringVector::const_iterator n = includeList.begin();
                while ( n != includeList.end() )
                {
                    allEntries.push_back( (*n) );
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Included the file \"" + (*n) + "\" based on the include list for this dir" );
                    ++n;
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

void
ExcludeOrIncludeDirEntriesAsSpecifiedForDir( const CORE::CString& dir                          ,
                                             const TDirProcessingInstructions& allInstructions ,
                                             const CORE::CString& platform                     ,
                                             bool applyPlatformChangesOnly                     ,
                                             TStringVector& allEntries                         )
{GUCEF_TRACE;

    ExcludeOrIncludeEntriesAsSpecifiedForDir( dir                      ,
                                              allInstructions          ,
                                              platform                 ,
                                              true                     ,
                                              applyPlatformChangesOnly ,
                                              allEntries               );
}

/*---------------------------------------------------------------------------*/

void
ExcludeOrIncludeFileEntriesAsSpecifiedForDir( const CORE::CString& dir                          ,
                                              const TDirProcessingInstructions& allInstructions ,
                                              const CORE::CString& platform                     ,
                                              bool applyPlatformChangesOnly                     ,
                                              TStringVector& allEntries                         )
{GUCEF_TRACE;

    ExcludeOrIncludeEntriesAsSpecifiedForDir( dir                      ,
                                              allInstructions          ,
                                              platform                 ,
                                              false                    ,
                                              applyPlatformChangesOnly ,
                                              allEntries               );
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
ParseProcessingInstructions( const CORE::CString& instructionsDir           ,
                             TDirProcessingInstructions& instructionStorage )
{GUCEF_TRACE;

    // Now we exclude based on the processing instructions for this dir
    // if any exist
    const CORE::CDataNode& instructions = instructionStorage.processingInstructions;
    if ( instructions.HasChildren() )
    {
        // write down the tags we will parse for
        static const CORE::CString rootNodeName = "CMAKELISTGENERATOR";
        static const CORE::CString excludesNodeName = "EXCLUDES";
        static const CORE::CString includesNodeName = "INCLUDES";
        static const CORE::CString platformNodeName = "PLATFORM";
        static const CORE::CString itemNodeName = "ITEM";
        static const CORE::CString nameAttribName = "NAME";
        static const CORE::CString typeAttribName = "TYPE";
        static const CORE::CString allPlatformsValue = "ALL";

        // Parse the instructions for the information we are looking for
        CORE::CString leftOver;
        const CORE::CDataNode* rootNode = instructions.Find( rootNodeName );
        if ( 0 != rootNode )
        {
            CORE::CDataNode::const_iterator i = rootNode->ConstBegin();
            while ( i != rootNode->End() )
            {
                const CORE::CDataNode* curNode = (*i);
                if ( curNode->GetName() == excludesNodeName )
                {
                    // Store whether the instructions apply to a particular platform
                    CORE::CString platformName = curNode->GetAttributeValue( platformNodeName );

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
                                        instructionStorage.dirExcludeList.push_back( itemName );
                                        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added wildcard directory exclude entry \"" + itemName + "\" based on the processing instructions for this dir" );
                                        instructionStorage.fileExcludeList.push_back( itemName );
                                        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added wildcard file exclude entry \"" + itemName + "\" based on the processing instructions for this dir" );
                                    }
                                    else
                                    {
                                        if ( itemIsADir )
                                        {
                                            instructionStorage.dirExcludeList.push_back( itemName );
                                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added directory exclude entry \"" + itemName + "\" based on the processing instructions for this dir" );
                                        }
                                        else
                                        {
                                            instructionStorage.fileExcludeList.push_back( itemName );
                                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added file exclude entry \"" + itemName + "\" based on the processing instructions for this dir" );
                                        }
                                    }
                                }
                                else
                                {
                                    // Add for the given platform
                                    if ( itemName.HasChar( '*' ) != -1 && itemType.IsNULLOrEmpty() )
                                    {
                                        instructionStorage.dirPlatformExcludeList[ platformName ].push_back( itemName );
                                        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added wildcard directory exclude entry \"" + itemName + "\" for platform " + platformName + " based on the processing instructions for this dir" );
                                        instructionStorage.filePlatformExcludeList[ platformName ].push_back( itemName );
                                        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added wildcard file exclude entry \"" + itemName + "\" for platform " + platformName + " based on the processing instructions for this dir" );
                                    }
                                    else
                                    {
                                        if ( itemIsADir )
                                        {
                                            instructionStorage.dirPlatformExcludeList[ platformName ].push_back( itemName );
                                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added directory exclude entry \"" + itemName + "\" for platform " + platformName + " based on the processing instructions for this dir" );
                                        }
                                        else
                                        {
                                            instructionStorage.filePlatformExcludeList[ platformName ].push_back( itemName );
                                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added file exclude entry \"" + itemName + "\" for platform " + platformName + " based on the processing instructions for this dir" );
                                        }
                                    }
                                }
                            }
                        }
                        ++n;
                    }

                }
                else
                if ( curNode->GetName() == includesNodeName )
                {
                    // Store whether the instructions apply to a particular platform
                    CORE::CString platformName = curNode->GetAttributeValue( platformNodeName );

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
                                        instructionStorage.dirIncludeList.push_back( itemName );
                                        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added wildcard directory include entry \"" + itemName + "\" based on the processing instructions for this dir" );
                                        instructionStorage.fileIncludeList.push_back( itemName );
                                        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added wildcard file include entry \"" + itemName + "\" based on the processing instructions for this dir" );
                                    }
                                    else
                                    if ( itemIsADir )
                                    {
                                        instructionStorage.dirIncludeList.push_back( itemName );
                                        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added directory include entry \"" + itemName + "\" based on the processing instructions for this dir" );
                                    }
                                    else
                                    {
                                        instructionStorage.fileIncludeList.push_back( itemName );
                                        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added file include entry \"" + itemName + "\" based on the processing instructions for this dir" );
                                    }
                                }
                                else
                                {
                                    // Add for the given platform
                                    if ( itemName.HasChar( '*' ) != -1 && itemType.IsNULLOrEmpty() )
                                    {
                                        instructionStorage.dirPlatformIncludeList[ platformName ].push_back( itemName );
                                        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added wildcard directory include entry \"" + itemName + "\" for platform " + platformName + " based on the processing instructions for this dir" );
                                        instructionStorage.filePlatformIncludeList[ platformName ].push_back( itemName );
                                        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added wildcard file include entry \"" + itemName + "\" for platform " + platformName + " based on the processing instructions for this dir" );
                                    }
                                    else
                                    {
                                        if ( itemIsADir )
                                        {
                                            instructionStorage.dirPlatformIncludeList[ platformName ].push_back( itemName );
                                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added directory include entry \"" + itemName + "\" for platform " + platformName + " based on the processing instructions for this dir" );
                                        }
                                        else
                                        {
                                            instructionStorage.filePlatformIncludeList[ platformName ].push_back( itemName );
                                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Added file include entry \"" + itemName + "\" for platform " + platformName + " based on the processing instructions for this dir" );
                                        }
                                    }
                                }
                            }
                        }
                        ++n;
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

    return CORE::FileExists( GetExcludeListPath( dir ) )         ||
           CORE::FileExists( GetProcessingInstructionsPath( dir ) );
}

/*---------------------------------------------------------------------------*/

TDirProcessingInstructions*
GetProcessingInstructions( TProjectInfo& projectInfo ,
                           const CORE::CString& dir  )
{GUCEF_TRACE;

    // See if we have already stored instructions for this directory
    TDirProcessingInstructionsMap::iterator i = projectInfo.dirProcessingInstructions.find( dir );
    if ( i != projectInfo.dirProcessingInstructions.end() )
    {
        return &( (*i).second );
    }

    if ( AreProcessingInstructionsOnDisk( dir ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Located processing instructions for directory \"" + dir + "\"" );

        // See if we can load instructions for this directory
        TDirProcessingInstructions& instructions = projectInfo.dirProcessingInstructions[ dir ];

        // Load the simple exclude list and put the excludes in the correct list
        TStringVector simpleExcludeList = GetExcludeList( dir );
        TStringVector::iterator n = simpleExcludeList.begin();
        while ( n != simpleExcludeList.end() )
        {
            if ( (*n).HasChar( '*' ) != -1 )
            {
                GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Determined that the simple exclude list wildcard item \"" + (*n) + "\" is a directory" );
                instructions.dirExcludeList.push_back( (*n) );
                GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Determined that the simple exclude list wildcard item \"" + (*n) + "\" is a file" );
                instructions.fileExcludeList.push_back( (*n) );
            }
            else
            if ( IsProcessingInstructionsItemADir( dir, (*n) ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Determined that the simple exclude list item \"" + (*n) + "\" is a directory" );
                instructions.dirExcludeList.push_back( (*n) );
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Determined that the simple exclude list item \"" + (*n) + "\" is a file" );
                instructions.fileExcludeList.push_back( (*n) );
            }
            ++n;
        }

        // Now process the more fancy processing instructions
        if ( GetProcessingInstructions( dir, instructions.processingInstructions ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Parsing advanced processing instructions for directory \"" + dir + "\"" );
            ParseProcessingInstructions( dir, instructions );
        }
        return &instructions;
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

void
LoadAllProcessingInstructions( TProjectInfo& projectInfo    ,
                               const CORE::CString& rootDir )
{GUCEF_TRACE;

    // Load instructions for the root dir itself
    GetProcessingInstructions( projectInfo, rootDir );

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

                    // Recurse into sub-dir
                    LoadAllProcessingInstructions( projectInfo, subRoot );
                }
            }
        }
        while ( 0 != DI_Next_Dir_Entry( sdiData ) );
        DI_Cleanup( sdiData );
    }
}

/*---------------------------------------------------------------------------*/

void
ExcludeOrIncludeDirEntriesAsSpecifiedForDir( TProjectInfo& projectInfo     ,
                                             const CORE::CString& dir      ,
                                             const CORE::CString& platform ,
                                             bool applyPlatformChangesOnly ,
                                             TStringVector& allEntries     )
{GUCEF_TRACE;

    // Fetch processing instructions from directory
    TDirProcessingInstructions* instructionStorage = GetProcessingInstructions( projectInfo, dir );

    // Perform processing of global dir excludes
    TStringVector::iterator n = projectInfo.globalDirExcludeList.begin();
    while ( n != projectInfo.globalDirExcludeList.end() )
    {
        if ( RemoveString( allEntries, (*n) ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Excluded the dir \"" + (*n) + "\" based on the global dir exclude list" );
        }
        ++n;
    }

    if ( NULL != instructionStorage )
    {
        // Carry out the process using the fetched instructions
        ExcludeOrIncludeDirEntriesAsSpecifiedForDir( dir, *instructionStorage, platform, applyPlatformChangesOnly, allEntries );
    }
}

/*---------------------------------------------------------------------------*/

void
ExcludeOrIncludeFileEntriesAsSpecifiedForDir( TProjectInfo& projectInfo     ,
                                              const CORE::CString& dir      ,
                                              const CORE::CString& platform ,
                                              bool applyPlatformChangesOnly ,
                                              TStringVector& allEntries     )
{GUCEF_TRACE;

    // Fetch processing instructions from directory
    TDirProcessingInstructions* instructionStorage = GetProcessingInstructions( projectInfo, dir );

    if ( NULL != instructionStorage )
    {
        // Carry out the process using the fetched instructions
        ExcludeOrIncludeFileEntriesAsSpecifiedForDir( dir, *instructionStorage, platform, applyPlatformChangesOnly, allEntries );
    }
}

/*---------------------------------------------------------------------------*/

void
ExcludeOrIncludeDirEntriesAsSpecifiedForDir( TProjectInfo& projectInfo ,
                                             const CORE::CString& dir  ,
                                             TStringVector& allEntries )
{GUCEF_TRACE;

    ExcludeOrIncludeDirEntriesAsSpecifiedForDir( projectInfo,
                                                 dir,
                                                 CORE::CString(),
                                                 false,
                                                 allEntries );
}

/*---------------------------------------------------------------------------*/

void
ExcludeOrIncludeFileEntriesAsSpecifiedForDir( TProjectInfo& projectInfo ,
                                              const CORE::CString& dir  ,
                                              TStringVector& allEntries )
{GUCEF_TRACE;

    ExcludeOrIncludeFileEntriesAsSpecifiedForDir( projectInfo,
                                                  dir,
                                                  CORE::CString(),
                                                  false,
                                                  allEntries );
}

/*-------------------------------------------------------------------------*/

void
PopulateFileListFromDir( const CORE::CString& path      ,
                         const TStringVector& fileTypes ,
                         TStringVector& fileList        )
{GUCEF_TRACE;

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
                    fileList.push_back( filename );
                }
            }
        }
        while ( 0 != DI_Next_Dir_Entry( sdiData ) );
        DI_Cleanup( sdiData );
    }
}

/*---------------------------------------------------------------------------*/

void
PopulateDirListFromDir( const CORE::CString& path ,
                        TStringVector& dirList    )
{GUCEF_TRACE;

    CORE::SDI_Data* sdiData = CORE::DI_First_Dir_Entry( path.C_String() );
    if ( NULL != sdiData )
    {
        do
        {
            if ( 0 == DI_Is_It_A_File( sdiData ) )
            {
                CORE::CString dirName = DI_Name( sdiData );
                if ( ( dirName != "." ) && ( dirName != ".." ) )
                {
                    dirList.push_back( dirName );
                }
            }
        }
        while ( 0 != DI_Next_Dir_Entry( sdiData ) );
        DI_Cleanup( sdiData );
    }
}

/*---------------------------------------------------------------------------*/

const CORE::CString&
GetCMakeListsFileHeader( bool addCompileDate = false )
{GUCEF_TRACE;

    static bool initialized = false;
    static CORE::CString fileHeader;
    static CORE::CString fileHeaderWithCompileDate;

    if ( !initialized )
    {
        fileHeader =
        "#-------------------------------------------------------------------\n"
        "# This file is part of a CMake build system designed for GUCEF\n"
        "#     (Galaxy Unlimited Framework)\n"
        "# For the latest info, see http://www.VanvelzenSoftware.com/\n"
        "#\n"
        "# The contents of this file are placed in the public domain. Feel\n"
        "# free to make use of it in any way you like.\n"
        "#-------------------------------------------------------------------\n"
        "#\n";

        fileHeaderWithCompileDate = fileHeader +
        "# THIS FILE IS AUTOMATICALLY GENERATED BY CMAKELISTGENERATOR\n"
        "#\n"
        "# CMAKELISTGENERATOR was compiled on: " __DATE__ " @ " __TIME__ "\n"
        "#\n"
        "#\n"
        "#\n";

        initialized = true;
    }

    if ( addCompileDate )
    {
        return fileHeaderWithCompileDate;
    }
    return fileHeader;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GenerateCMakeListsFileSection( const CORE::CString& sectionContent ,
                               const TStringVectorMap& fileMap     )
{GUCEF_TRACE;

    CORE::CString newSectionContent = sectionContent;
    TStringVectorMap::const_iterator i = fileMap.begin();
    while ( i != fileMap.end() )
    {
        TStringVector::const_iterator n = (*i).second.begin();
        while ( n != (*i).second.end() )
        {
            CORE::CString path = (*i).first;
            CORE::AppendToPath( path, (*n) );
            path = path.ReplaceChar( '\\', '/' );

            newSectionContent += "  " + path + "\n";
            ++n;
        }
        ++i;
    }
    newSectionContent += ")\n\n";
    return newSectionContent;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GenerateCMakeListsFileIncludeSection( const TStringVectorMap& includeFiles )
{GUCEF_TRACE;

    CORE::CString sectionContent = "set(HEADER_FILES \n";
    return GenerateCMakeListsFileSection( sectionContent, includeFiles );
}

/*---------------------------------------------------------------------------*/

CORE::CString
GenerateCMakeListsFileSrcSection( const TStringVectorMap& srcFiles )
{GUCEF_TRACE;

    CORE::CString sectionContent = "set(SOURCE_FILES \n";
    return GenerateCMakeListsFileSection( sectionContent, srcFiles );
}

/*---------------------------------------------------------------------------*/

void
GetAllPlatformFilesInPlatformDirs( TModuleInfo& moduleInfo           ,
                                   const CORE::CString& platformName ,
                                   const CORE::CString& platformDir  )
{GUCEF_TRACE;

    // First we check each of the project include dirs for a platform sub-sir
    TStringVectorMap::iterator i = moduleInfo.includeDirs.begin();
    while ( i != moduleInfo.includeDirs.end() )
    {
        // Build the include dir to check for a platform sub-dir
        CORE::CString includeDir = (*i).first;
        CORE::AppendToPath( includeDir, platformDir );
        CORE::CString fullIncludeDir = moduleInfo.rootDir;
        CORE::AppendToPath( fullIncludeDir, includeDir );

        // Try and get a list of include files from the platform dir
        TStringVector platformFiles;
        PopulateFileListFromDir( fullIncludeDir, GetHeaderFileExtensions(), platformFiles );

        if ( !platformFiles.empty() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Discovered valid platform sub-dir which has header files" + includeDir );

            // Add the platform files to the list of platform files from all include dirs
            TStringVectorMap& allFilesForThisPlatform = moduleInfo.platformHeaderFiles[ platformName ];
            CORE::CString includeDirIndex = includeDir.ReplaceChar( '\\', '/' );
            TStringVector& allFilesForThisPlatformDir = allFilesForThisPlatform[ includeDirIndex ];

            TStringVector::iterator n = platformFiles.begin();
            while ( n != platformFiles.end() )
            {
                allFilesForThisPlatformDir.push_back( (*n) );
                ++n;
            }
        }

        // Try and get a list of source files from the platform dir
        platformFiles.clear();
        PopulateFileListFromDir( fullIncludeDir, GetSourceFileExtensions(), platformFiles );

        if ( !platformFiles.empty() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Discovered valid platform sub-dir which has source files" + includeDir );

            // Add the platform files to the list of platform files from all include dirs
            TStringVectorMap& allFilesForThisPlatform = moduleInfo.platformSourceFiles[ platformName ];
            CORE::CString includeDirIndex = includeDir.ReplaceChar( '\\', '/' );
            TStringVector& allFilesForThisPlatformDir = allFilesForThisPlatform[ includeDirIndex ];

            TStringVector::iterator n = platformFiles.begin();
            while ( n != platformFiles.end() )
            {
                allFilesForThisPlatformDir.push_back( (*n) );
                ++n;
            }
        }

        ++i;
    }

    // Now we check each of the project source dirs for a platform sub-sir
    i = moduleInfo.sourceDirs.begin();
    while ( i != moduleInfo.sourceDirs.end() )
    {
        // Build the source dir to check for a platform sub-dir
        CORE::CString sourceDir = (*i).first;
        CORE::AppendToPath( sourceDir, platformDir );
        CORE::CString fullSourceDir = moduleInfo.rootDir;
        CORE::AppendToPath( fullSourceDir, sourceDir );

        // Try and get a list of header files from the platform dir
        TStringVector platformFiles;
        PopulateFileListFromDir( fullSourceDir, GetHeaderFileExtensions(), platformFiles );

        if ( !platformFiles.empty() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Discovered valid platform sub-dir which has header files " + sourceDir );

            // Add the platform files to the list of platform files from all source dirs
            TStringVectorMap& allFilesForThisPlatform = moduleInfo.platformHeaderFiles[ platformName ];
            CORE::CString sourceDirIndex = sourceDir.ReplaceChar( '\\', '/' );
            TStringVector& allFilesForThisPlatformDir = allFilesForThisPlatform[ sourceDirIndex ];

            TStringVector::iterator n = platformFiles.begin();
            while ( n != platformFiles.end() )
            {
                allFilesForThisPlatformDir.push_back( (*n) );
                ++n;
            }
        }

        // Try and get a list of files from the platform dir
        platformFiles.clear();
        PopulateFileListFromDir( fullSourceDir, GetSourceFileExtensions(), platformFiles );

        if ( !platformFiles.empty() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Discovered valid platform sub-dir which has source files " + sourceDir );

            // Add the platform files to the list of platform files from all source dirs
            TStringVectorMap& allFilesForThisPlatform = moduleInfo.platformSourceFiles[ platformName ];
            CORE::CString sourceDirIndex = sourceDir.ReplaceChar( '\\', '/' );
            TStringVector& allFilesForThisPlatformDir = allFilesForThisPlatform[ sourceDirIndex ];

            TStringVector::iterator n = platformFiles.begin();
            while ( n != platformFiles.end() )
            {
                allFilesForThisPlatformDir.push_back( (*n) );
                ++n;
            }
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

const TStringSetMap&
GetSupportedPlatformDirMap( void )

{GUCEF_TRACE;

    static TStringSetMap platformMap;
    if ( platformMap.empty() )
    {
        platformMap[ "WIN32" ].insert( "mswin" );
        platformMap[ "WIN32" ].insert( "win32" );
        platformMap[ "WIN64" ].insert( "mswin" );
        platformMap[ "WIN64" ].insert( "win64" );
        platformMap[ "UNIX" ].insert( "linux" );
        platformMap[ "UNIX" ].insert( "unix" );
        platformMap[ "IPHONEOS" ].insert( "iphone" );
        platformMap[ "SYMBIAN" ].insert( "symbian" );
        platformMap[ "OSX" ].insert( "osx" );
        platformMap[ "OSX" ].insert( "mac" );
        platformMap[ "ANDROID" ].insert( "android" );
        platformMap[ "GLX" ].insert( "glx" );
        platformMap[ "GTK" ].insert( "gtk" );
        platformMap[ "SDL" ].insert( "sdl" );
    }
    return platformMap;
}

/*---------------------------------------------------------------------------*/

const TStringSet&
GetSupportedPlatforms( void )
{GUCEF_TRACE;

    static TStringSet platforms;
    if ( platforms.empty() )
    {
        const TStringSetMap& dirMap = GetSupportedPlatformDirMap();
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
GetSupportedPlatformDirs()
{GUCEF_TRACE;

    static TStringSet platformDirs;
    if ( platformDirs.empty() )
    {
        const TStringSetMap& dirMap = GetSupportedPlatformDirMap();
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
IsDirAPlatformDir( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString lastSubDir = CORE::LastSubDir( path ).Lowercase();
    const TStringSet& supportedPlatformDirs = GetSupportedPlatformDirs();
    return supportedPlatformDirs.end() != supportedPlatformDirs.find( lastSubDir );
}

/*---------------------------------------------------------------------------*/

void
GetListOfAllModuleDirs( TModuleInfo& moduleInfo   ,
                        TStringSet& moduleDirs    ,
                        bool relativePaths        )
{GUCEF_TRACE;

    TStringVectorMap::iterator i = moduleInfo.includeDirs.begin();
    while ( i != moduleInfo.includeDirs.end() )
    {
        if ( relativePaths )
        {
            moduleDirs.insert( (*i).first );
        }
        else
        {
            CORE::CString fullPath = moduleInfo.rootDir;
            CORE::AppendToPath( fullPath, (*i).first );

            moduleDirs.insert( fullPath );
        }
        ++i;
    }
    i = moduleInfo.sourceDirs.begin();
    while ( i != moduleInfo.sourceDirs.end() )
    {
        if ( relativePaths )
        {
            moduleDirs.insert( (*i).first );
        }
        else
        {
            CORE::CString fullPath = moduleInfo.rootDir;
            CORE::AppendToPath( fullPath, (*i).first );

            moduleDirs.insert( fullPath );
        }
        ++i;
    }
    TStringVectorMapMap::iterator n = moduleInfo.platformHeaderFiles.begin();
    while ( n != moduleInfo.platformHeaderFiles.end() )
    {
        i = (*n).second.begin();
        while ( i != (*n).second.end() )
        {
            if ( relativePaths )
            {
                moduleDirs.insert( (*i).first );
            }
            else
            {
                CORE::CString fullPath = moduleInfo.rootDir;
                CORE::AppendToPath( fullPath, (*i).first );

                moduleDirs.insert( fullPath );
            }
            ++i;
        }
        ++n;
    }
    n = moduleInfo.platformSourceFiles.begin();
    while ( n != moduleInfo.platformSourceFiles.end() )
    {
        i = (*n).second.begin();
        while ( i != (*n).second.end() )
        {
            if ( relativePaths )
            {
                moduleDirs.insert( (*i).first );
            }
            else
            {
                CORE::CString fullPath = moduleInfo.rootDir;
                CORE::AppendToPath( fullPath, (*i).first );

                moduleDirs.insert( fullPath );
            }
            ++i;
        }
        ++n;
    }
}

/*---------------------------------------------------------------------------*/

void
ApplyDirProcessingInstructionsToModule( TProjectInfo& projectInfo     ,
                                        TModuleInfo& moduleInfo       ,
                                        TStringVectorMap::iterator& i ,
                                        const CORE::CString& platform ,
                                        bool applyPlatformChangesOnly )
{GUCEF_TRACE;

    // Create the full path in order to locate the processing instructions
    CORE::CString fullDir = moduleInfo.rootDir;
    CORE::AppendToPath( fullDir, (*i).first );

    // Get the processing instructions for this dir which we know is a module dir
    TDirProcessingInstructions* instructions = GetProcessingInstructions( projectInfo, fullDir );
    if ( NULL != instructions )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Applying processing instructions to module \"" + moduleInfo.name + "\" for module directory \"" + (*i).first + "\"" );

        // Now we add/substract based on generator instructions
        ExcludeOrIncludeFileEntriesAsSpecifiedForDir( fullDir                  ,
                                                      *instructions            ,
                                                      platform                 ,
                                                      applyPlatformChangesOnly ,
                                                      (*i).second              );
    }
}

/*---------------------------------------------------------------------------*/

void
ApplyDirProcessingInstructionsToModule( TProjectInfo& projectInfo ,
                                        TModuleInfo& moduleInfo   )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Applying processing instructions to module \"" + moduleInfo.name + "\" if any exist" );

    // First we process all the instructions that apply to all platforms
    // In order to do this we will iterate all the module dirs

    // Apply instructions to files in the include directories
    TStringVectorMap::iterator i = moduleInfo.includeDirs.begin();
    while ( i != moduleInfo.includeDirs.end() )
    {
        ApplyDirProcessingInstructionsToModule( projectInfo     ,
                                                moduleInfo      ,
                                                i               ,
                                                CORE::CString() ,
                                                false           );
        ++i;
    }

    // Apply instructions to files in the source directories
    i = moduleInfo.sourceDirs.begin();
    while ( i != moduleInfo.sourceDirs.end() )
    {
        ApplyDirProcessingInstructionsToModule( projectInfo     ,
                                                moduleInfo      ,
                                                i               ,
                                                CORE::CString() ,
                                                false           );
        ++i;
    }

    // Apply instructions to files in the platform include directories
    TStringVectorMapMap::iterator n = moduleInfo.platformHeaderFiles.begin();
    while ( n != moduleInfo.platformHeaderFiles.end() )
    {
        TStringVectorMap& dirsForPlatform = (*n).second;
        TStringVectorMap::iterator m = dirsForPlatform.begin();
        while ( m != dirsForPlatform.end() )
        {
            ApplyDirProcessingInstructionsToModule( projectInfo     ,
                                                    moduleInfo      ,
                                                    m               ,
                                                    CORE::CString() ,
                                                    false           );

            ++m;
        }
        ++n;
    }

    // Apply instructions to files in the platform source directories
    n = moduleInfo.platformSourceFiles.begin();
    while ( n != moduleInfo.platformSourceFiles.end() )
    {
        TStringVectorMap& dirsForPlatform = (*n).second;
        TStringVectorMap::iterator m = dirsForPlatform.begin();
        while ( m != dirsForPlatform.end() )
        {
            ApplyDirProcessingInstructionsToModule( projectInfo     ,
                                                    moduleInfo      ,
                                                    m               ,
                                                    CORE::CString() ,
                                                    false           );

            ++m;
        }
        ++n;
    }

    // Now for platform support we need to do something different.
    // For every directory that belongs to this module we have to
    // check if we have instructions for specific platforms

    TStringSet allModuleDirs;
    GetListOfAllModuleDirs( moduleInfo, allModuleDirs, false );

    // For each supported platform check if there is a dir with relevant instructions
    // for that platform
    const TStringSet& supportedPlatforms = GetSupportedPlatforms();
    TStringSet::const_iterator r = supportedPlatforms.begin();
    while ( r != supportedPlatforms.end() )
    {
        TStringSet::iterator p = allModuleDirs.begin();
        while ( p != allModuleDirs.end() )
        {
            // Get the processing instructions for this dir which we know is a module dir
            TDirProcessingInstructions* instructions = GetProcessingInstructions( projectInfo, (*p) );
            if ( NULL != instructions )
            {
                // See if we have any instructions for this specific platform
                i = instructions->filePlatformExcludeList.find( (*r) );
                if ( i != instructions->filePlatformExcludeList.end() )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Applying platfrom specific exclude instructions for platform \"" + (*r) + "\" to module \"" + moduleInfo.name + "\" for module directory \"" + (*i).first + "\"" );

                    // Get all excludes
                    TStringVector& excludeList = (*i).second;

                    // Separate them into headers and sources
                    TStringVector headerExcludeList;
                    FilterStringVectorForFilesWithExtensions( headerExcludeList, GetHeaderFileExtensions(), excludeList );
                    TStringVector sourceExcludeList;
                    FilterStringVectorForFilesWithExtensions( sourceExcludeList, GetSourceFileExtensions(), excludeList );

                    CORE::CString relativePathToSubDir = (*p).CutChars( moduleInfo.rootDir.Length(), true );

                    TStringVectorMap& platformHeaderFiles = moduleInfo.platformHeaderFiles[ (*r) ];
                    TStringVector::iterator q = headerExcludeList.begin();
                    while ( q != headerExcludeList.end() )
                    {
                        TStringVector& platformDirFiles = platformHeaderFiles[ relativePathToSubDir ];
                        if ( RemoveString( platformDirFiles, (*q) ) )
                        {
                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Excluded the file \"" + (*q) + "\" for platform \"" + (*r) + "\" based on the exclude list for this dir" );
                        }
                        ++q;
                    }

                    TStringVectorMap& platformSourceFiles = moduleInfo.platformSourceFiles[ (*r) ];
                    q = sourceExcludeList.begin();
                    while ( q != sourceExcludeList.end() )
                    {
                        TStringVector& platformDirFiles = platformSourceFiles[ relativePathToSubDir ];
                        if ( RemoveString( platformDirFiles, (*q) ) )
                        {
                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Excluded the file \"" + (*q) + "\" for platform \"" + (*r) + "\" based on the exclude list for this dir" );
                        }
                        ++q;
                    }
                }

                i = instructions->filePlatformIncludeList.find( (*r) );
                if ( i != instructions->filePlatformIncludeList.end() )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Applying platform specific include instructions for platform \"" + (*r) + "\" to module \"" + moduleInfo.name + "\" for module directory \"" + (*i).first + "\"" );

                    // Get all includes
                    TStringVector& includeList = (*i).second;

                    // Separate them into headers and sources
                    TStringVector headerIncludeList;
                    FilterStringVectorForFilesWithExtensions( headerIncludeList, GetHeaderFileExtensions(), includeList );
                    TStringVector sourceIncludeList;
                    FilterStringVectorForFilesWithExtensions( sourceIncludeList, GetSourceFileExtensions(), includeList );

                    CORE::CString relativePathToSubDir = (*p).CutChars( moduleInfo.rootDir.Length()+1, true );

                    TStringVectorMap& platformHeaderFiles = moduleInfo.platformHeaderFiles[ (*r) ];
                    TStringVector::iterator q = headerIncludeList.begin();
                    while ( q != headerIncludeList.end() )
                    {
                        TStringVector& platformDirFiles = platformHeaderFiles[ relativePathToSubDir ];
                        platformDirFiles.push_back( (*q) );
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Included the file \"" + (*q) + "\" for platform \"" + (*r) + "\" based on the include list for this dir" );
                        ++q;
                    }

                    TStringVectorMap& platformSourceFiles = moduleInfo.platformSourceFiles[ (*r) ];
                    q = sourceIncludeList.begin();
                    while ( q != sourceIncludeList.end() )
                    {
                        TStringVector& platformDirFiles = platformSourceFiles[ relativePathToSubDir ];
                        platformDirFiles.push_back( (*q) );
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Included the file \"" + (*q) + "\" for platform \"" + (*r) + "\" based on the include list for this dir" );
                        ++q;
                    }
                }
            }
            ++p;
        }

        ++r;
    }
}

/*---------------------------------------------------------------------------*/

void
GetAllPlatformFilesInPlatformDirs( const TProjectInfo& projectInfo ,
                                   TModuleInfo& moduleInfo         )
{GUCEF_TRACE;

    // Look for supported platform dirs to find platform files
    const TStringSetMap& dirMap = GetSupportedPlatformDirMap();
    TStringSetMap::const_iterator i = dirMap.begin();
    while ( i != dirMap.end() )
    {
        const TStringSet& dirs = (*i).second;
        TStringSet::const_iterator n = dirs.begin();
        while ( n != dirs.end() )
        {
            GetAllPlatformFilesInPlatformDirs( moduleInfo, (*i).first, (*n) );
            ++n;
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
GenerateCMakeListsFilePlatformFilesSection( TModuleInfo& moduleInfo           ,
                                            const CORE::CString& platformName ,
                                            CORE::CString& headerSection      ,
                                            CORE::CString& sourceSection      ,
                                            bool& hasPlatformHeaderFiles      ,
                                            bool& hasPlatformSourceFiles      )
{GUCEF_TRACE;



    TStringVectorMapMap::iterator m = moduleInfo.platformHeaderFiles.find( platformName );
    if ( m != moduleInfo.platformHeaderFiles.end() )
    {
        const TStringVectorMap& platformHeaderFiles = (*m).second;
        if ( !platformHeaderFiles.empty() )
        {
            hasPlatformHeaderFiles = true;
            headerSection = "  set( PLATFORM_HEADER_FILES \n";

            TStringVectorMap::const_iterator n = platformHeaderFiles.begin();
            while ( n != platformHeaderFiles.end() )
            {
                const TStringVector& platformHeaderFilesDir = (*n).second;
                TStringVector::const_iterator i = platformHeaderFilesDir.begin();
                while ( i != platformHeaderFilesDir.end() )
                {
                    CORE::CString path = (*n).first;
                    CORE::AppendToPath( path, (*i) );
                    path = path.ReplaceChar( '\\', '/' );

                    headerSection += "    " + path + "\n";

                    ++i;
                }
                ++n;
            }
            headerSection += "  )\n\n";

            // Add additional platform specific includes
            //headerSection += "  include_directories( ";
            //n = platformHeaderFiles.begin();
            //while ( n != platformHeaderFiles.end() )
            //{
            //    headerSection += (*n).first + " ";
            //    ++n;
            //}
            //headerSection += ")\n";

            headerSection += "  set( PLATFORM_HEADER_INSTALL \"" + platformName + "\" )\n";
            headerSection += "  source_group( \"Platform Header Files\" FILES ${PLATFORM_HEADER_FILES} )\n\n";
        }
    }

    m = moduleInfo.platformSourceFiles.find( platformName );
    if ( m != moduleInfo.platformSourceFiles.end() )
    {
        const TStringVectorMap& platformSourceFiles = (*m).second;
        if ( !platformSourceFiles.empty() )
        {
            hasPlatformSourceFiles = true;
            sourceSection = "  set( PLATFORM_SOURCE_FILES \n";

            TStringVectorMap::const_iterator n = platformSourceFiles.begin();
            while ( n != platformSourceFiles.end() )
            {
                const TStringVector& platformSourceFilesDir = (*n).second;
                TStringVector::const_iterator i = platformSourceFilesDir.begin();
                while ( i != platformSourceFilesDir.end() )
                {
                    CORE::CString path = (*n).first;
                    CORE::AppendToPath( path, (*i) );
                    path = path.ReplaceChar( '\\', '/' );

                    sourceSection += "    " + path + "\n";
                    ++i;
                }
                ++n;
            }
            sourceSection += "  )\n\n";

            sourceSection += "  set( PLATFORM_SOURCE_INSTALL \"" + platformName + "\" )\n";
            sourceSection += "  source_group( \"Platform Source Files\" FILES ${PLATFORM_SOURCE_FILES} )\n\n";
        }
    }
}

/*---------------------------------------------------------------------------*/

CORE::CString
GenerateCMakeListsFilePlatformFilesSection( TModuleInfo& moduleInfo )
{GUCEF_TRACE;

    bool hasPlatformHeaderFiles = false;
    bool hasPlatformSourceFiles = false;

    CORE::CString sectionContent;
    bool firstPlatform = true;
    TStringVectorMapMap::iterator i = moduleInfo.platformHeaderFiles.begin();
    while ( i != moduleInfo.platformHeaderFiles.end() )
    {
        CORE::CString headerSection;
        CORE::CString sourceSection;
        const CORE::CString& platformName = (*i).first;
        GenerateCMakeListsFilePlatformFilesSection( moduleInfo             ,
                                                    platformName           ,
                                                    headerSection          ,
                                                    sourceSection          ,
                                                    hasPlatformHeaderFiles ,
                                                    hasPlatformSourceFiles );

        if ( firstPlatform )
        {
            sectionContent = "\n\nif (" + platformName + ")\n" + headerSection + sourceSection;
            firstPlatform = false;
        }
        else
        {
            sectionContent += "elseif (" + platformName + ")\n" + headerSection + sourceSection;
        }

        ++i;
    }

    if ( hasPlatformHeaderFiles || hasPlatformSourceFiles )
    {
        // since we added data we have to close the section
        sectionContent += "endif ()\n\n";
    }

    // Make sure the variable is defined even if we have no platform files
    // This allows people to always add the variable in the suffix file without knowing
    // if there are platform files
    if ( !hasPlatformHeaderFiles )
    {
        sectionContent += "# Make sure the PLATFORM_HEADER_FILES variable is always defined\n set( PLATFORM_HEADER_FILES \"\" )\n\n";
    }
    if ( !hasPlatformSourceFiles )
    {
        sectionContent += "# Make sure the PLATFORM_SOURCE_FILES variable is always defined\n set( PLATFORM_SOURCE_FILES \"\" )\n\n";
    }

    return sectionContent;
}

/*---------------------------------------------------------------------------*/

TStringVector
ParseFileLines( const CORE::CString& fileSuffix )
{GUCEF_TRACE;

    CORE::CString testStr = fileSuffix.ReplaceChar( '\r', '\n' );
    testStr = testStr.CompactRepeatingChar( '\n' );
    return testStr.ParseElements( '\n', false );
}

/*---------------------------------------------------------------------------*/

TStringSet
ParseIncludeDirs( const CORE::CString& fileSuffix )
{GUCEF_TRACE;

    TStringSet includeDirs;

    TStringVector suffixFileLines = ParseFileLines( fileSuffix );
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
ParseSuffixFile( TModuleInfo& moduleInfo )
{GUCEF_TRACE;

    TStringVector suffixFileLines = ParseFileLines( moduleInfo.cmakeListSuffixFileContent );
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
                moduleInfo.linkedLibraries.push_back( (*n) );
                ++n;
            }
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

TStringVector
ParseDependencies( const CORE::CString& fileSuffix ,
                   CORE::CString& moduleName       )
{GUCEF_TRACE;

    TStringVector dependencies;

    TStringVector suffixFileLines = ParseFileLines( fileSuffix );
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
                dependencies.push_back( (*n) );
                ++n;
            }
        }
        ++i;
    }
    return dependencies;
}

/*---------------------------------------------------------------------------*/

bool
ParseModuleProperties( const CORE::CString& fileSuffix ,
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

const TModuleInfo*
GetModuleInfo( const TProjectInfo& projectInfo ,
               const CORE::CString& moduleName )
{GUCEF_TRACE;

    TModuleInfoVector::const_iterator i = projectInfo.modules.begin();
    while ( i != projectInfo.modules.end() )
    {
        if ( (*i).name == moduleName )
        {
            return &(*i);
        }
        ++i;
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GenerateCMakeModuleIncludesSectionForPlatform( const TProjectInfo& projectInfo   ,
                                               const TModuleInfo& moduleInfo     ,
                                               const CORE::CString& platformName )
{GUCEF_TRACE;

    CORE::CString allRelDependencyPaths;

    // Add all the include dirs inherited from dependency modules
    TStringSetMap::const_iterator m = moduleInfo.dependencyPlatformIncludeDirs.find( platformName );
    if ( m != moduleInfo.dependencyPlatformIncludeDirs.end() )
    {
        const TStringSet& platformIncludes = (*m).second;
        TStringSet::const_iterator i = platformIncludes.begin();
        while ( i != platformIncludes.end() )
        {
            allRelDependencyPaths += (*i) + " ";
            ++i;
        }
    }

    // Add all the regular platform include dirs for this module itself
    TStringVectorMapMap::const_iterator t = moduleInfo.platformHeaderFiles.find( platformName );
    if ( t != moduleInfo.platformHeaderFiles.end() )
    {
        const TStringVectorMap& platformHeaderFiles = (*t).second;
        TStringVectorMap::const_iterator n = platformHeaderFiles.begin();
        while ( n != platformHeaderFiles.end() )
        {
            CORE::CString includeDir = (*n).first;
            includeDir = CORE::RelativePath( includeDir ).ReplaceChar( '\\', '/' );
            allRelDependencyPaths += includeDir + " ";
            ++n;
        }
    }

    CORE::CString sectionContent;
    if ( allRelDependencyPaths.Length() > 0 )
    {
        sectionContent = "\nif ( "+ platformName + " )\n  include_directories( " + allRelDependencyPaths + ")\nendif( "+ platformName + " )\n";
    }

    return sectionContent;
}

/*---------------------------------------------------------------------------*/

void
GenerateModuleIncludesForPlatform( const TProjectInfo& projectInfo   ,
                                   TModuleInfo& moduleInfo           ,
                                   const CORE::CString& platformName )
{GUCEF_TRACE;

    // Add include dirs for each dependency we know about
    TStringVector& dependencies = moduleInfo.dependencies;
    TStringVector::iterator i = dependencies.begin();
    while ( i != dependencies.end() )
    {
        // Get a dependency module which is already fully processed
        const TModuleInfo* dependencyModule = GetModuleInfo( projectInfo, (*i) );
        if ( NULL != dependencyModule )
        {
            // Determine the relative path to this other module
            CORE::CString relativePath = CORE::GetRelativePathToOtherPathRoot( moduleInfo.rootDir        ,
                                                                               dependencyModule->rootDir );
            relativePath = relativePath.ReplaceChar( '\\', '/' );

            // Now construct the relative path to each of the dependency module's include dirs
            // These dir will all become include dirs for this module
            TStringVectorMapMap::const_iterator p = dependencyModule->platformHeaderFiles.find( platformName );
            if ( p != dependencyModule->platformHeaderFiles.end() )
            {
                const TStringVectorMap& platformHeaderFiles = (*p).second;
                TStringVectorMap::const_iterator n = platformHeaderFiles.begin();
                while ( n != platformHeaderFiles.end() )
                {
                    CORE::CString dependencyInclDir = relativePath;
                    CORE::AppendToPath( dependencyInclDir, (*n).first );
                    dependencyInclDir = CORE::RelativePath( dependencyInclDir );
                    dependencyInclDir = dependencyInclDir.ReplaceChar( '\\', '/' );

                    // Add the contructed include directory to the list of dependency directories
                    // for the current module. This can later be used again by other modules which
                    // include this one.
                    moduleInfo.dependencyPlatformIncludeDirs[ platformName ].insert( dependencyInclDir );
                    ++n;
                }
            }

            // On top of that we have to include all the include dirs that the dependency module
            // was including itself since it's headers might be referring to those files.
            TStringSetMap::const_iterator r = dependencyModule->dependencyPlatformIncludeDirs.find( platformName );
            if ( r != dependencyModule->dependencyPlatformIncludeDirs.end() )
            {
                const TStringSet& dependencyPlatformIncludeDirs = (*r).second;
                TStringSet::const_iterator m = dependencyPlatformIncludeDirs.begin();
                while ( m != dependencyPlatformIncludeDirs.end() )
                {
                    CORE::CString dependencyInclDir = relativePath;
                    CORE::AppendToPath( dependencyInclDir, (*m) );
                    dependencyInclDir = CORE::RelativePath( dependencyInclDir );
                    dependencyInclDir = dependencyInclDir.ReplaceChar( '\\', '/' );

                    // Add the contructed include directory to the list of dependency directories
                    // for the current module. This can later be used again by other modules which
                    // include this one.
                    moduleInfo.dependencyPlatformIncludeDirs[ platformName ].insert( dependencyInclDir );
                    ++m;
                }
            }
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

TStringSet
GetListOfRelevantPlatformsForModule( TModuleInfo& moduleInfo )
{GUCEF_TRACE;

    TStringSet platformList;
    TStringVectorMapMap::iterator i = moduleInfo.platformHeaderFiles.begin();
    while ( i != moduleInfo.platformHeaderFiles.end() )
    {
        platformList.insert( (*i).first );
        ++i;
    }
    i = moduleInfo.platformSourceFiles.begin();
    while ( i != moduleInfo.platformSourceFiles.end() )
    {
        platformList.insert( (*i).first );
        ++i;
    }
    return platformList;
}

/*---------------------------------------------------------------------------*/

void
GenerateModuleIncludesForAllPlatforms( const TProjectInfo& projectInfo ,
                                       TModuleInfo& moduleInfo         )
{GUCEF_TRACE;

    TStringSet relevantPlatformDirs = GetSupportedPlatforms();
    TStringSet::iterator i = relevantPlatformDirs.begin();
    while ( i != relevantPlatformDirs.end() )
    {
        GenerateModuleIncludesForPlatform( projectInfo, moduleInfo, (*i) );
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

CORE::CString
GenerateCMakeModuleIncludesSectionForAllPlatforms( const TProjectInfo& projectInfo ,
                                                   const TModuleInfo& moduleInfo   )
{GUCEF_TRACE;

    CORE::CString sectionContent;

    TStringSet relevantPlatformDirs = GetSupportedPlatforms();
    TStringSet::iterator i = relevantPlatformDirs.begin();
    while ( i != relevantPlatformDirs.end() )
    {
        sectionContent += GenerateCMakeModuleIncludesSectionForPlatform( projectInfo, moduleInfo, (*i) );
        ++i;
    }
    return sectionContent;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GenerateCMakeModuleIncludesSection( const TProjectInfo& projectInfo ,
                                    const TModuleInfo& moduleInfo   )
{GUCEF_TRACE;

    // Add include dirs for each dependency we know about
    CORE::CString allRelDependencyPaths;
    const TStringSet& includeDirs = moduleInfo.dependencyIncludeDirs;
    TStringSet::const_iterator i = includeDirs.begin();
    while ( i != includeDirs.end() )
    {
        allRelDependencyPaths += (*i) + " ";
        ++i;
    }

    // Add all the regular include dirs for this module
    TStringVectorMap::const_iterator n = moduleInfo.includeDirs.begin();
    while ( n != moduleInfo.includeDirs.end() )
    {
        CORE::CString includeDir = (*n).first.ReplaceChar( '\\', '/' );
        if ( 0 != includeDir.Length() )
        {
            allRelDependencyPaths += includeDir + " ";
        }
        else
        {
            // Check if there is more then one include dir
            // If so we have create an include for an empty include dir
            // to ensure files in subdirs can include the file with the zero length
            // subdir.
            if ( 1 < moduleInfo.includeDirs.size() )
            {
                allRelDependencyPaths += "../" + CORE::LastSubDir( moduleInfo.rootDir ) + " ";
            }
        }
        ++n;
    }

    CORE::CString sectionContent;
    if ( allRelDependencyPaths.Length() > 0 )
    {
        sectionContent = "\ninclude_directories( " + allRelDependencyPaths + ")\n";
    }

    sectionContent += GenerateCMakeModuleIncludesSectionForAllPlatforms( projectInfo, moduleInfo );

    return sectionContent;
}

/*---------------------------------------------------------------------------*/

void
GenerateModuleIncludes( const TProjectInfo& projectInfo ,
                        TModuleInfo& moduleInfo         )
{GUCEF_TRACE;

    // Add include dirs for each dependency we know about
    TStringVector& dependencies = moduleInfo.dependencies;
    TStringVector::iterator i = dependencies.begin();
    while ( i != dependencies.end() )
    {
        // Get a dependency module which is already fully processed
        const TModuleInfo* dependencyModule = GetModuleInfo( projectInfo, (*i) );
        if ( NULL != dependencyModule )
        {
            // Determine the relative path to this other module
            CORE::CString relativePath = GetRelativePathToOtherPathRoot( moduleInfo.rootDir        ,
                                                                         dependencyModule->rootDir );

            // Now construct the relative path to each of the dependency module's include dirs
            // These dir will all become include dirs for this module
            TStringVectorMap::const_iterator n = dependencyModule->includeDirs.begin();
            while ( n != dependencyModule->includeDirs.end() )
            {
                CORE::CString dependencyInclDir = relativePath;
                CORE::AppendToPath( dependencyInclDir, (*n).first );
                dependencyInclDir = CORE::RelativePath( dependencyInclDir );
                dependencyInclDir = dependencyInclDir.ReplaceChar( '\\', '/' );

                // Add the contructed include directory to the list of dependency directories
                // for the current module. This can later be used again by other modules which
                // include this one.
                moduleInfo.dependencyIncludeDirs.insert( dependencyInclDir );
                ++n;
            }

            // On top of that we have to include all the include dirs that the dependency module
            // was including itself since it's headers might be referring to those files.
            TStringSet::const_iterator m = dependencyModule->dependencyIncludeDirs.begin();
            while ( m != dependencyModule->dependencyIncludeDirs.end() )
            {
                CORE::CString dependencyInclDir = relativePath;
                CORE::AppendToPath( dependencyInclDir, (*m) );
                dependencyInclDir = CORE::RelativePath( dependencyInclDir );
                dependencyInclDir = dependencyInclDir.ReplaceChar( '\\', '/' );

                // Add the contructed include directory to the list of dependency directories
                // for the current module. This can later be used again by other modules which
                // include this one.
                moduleInfo.dependencyIncludeDirs.insert( dependencyInclDir );
                ++m;
            }
        }
        ++i;
    }

    GenerateModuleIncludesForAllPlatforms( projectInfo, moduleInfo );
}

/*---------------------------------------------------------------------------*/

void
GenerateModuleIncludes( TProjectInfo& projectInfo )
{GUCEF_TRACE;

    TModuleInfoVector::iterator i = projectInfo.modules.begin();
    while ( i != projectInfo.modules.end() )
    {
        GenerateModuleIncludes( projectInfo, (*i) );
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

CORE::CString
GenerateAutoGenertedSeperator( bool end )
{GUCEF_TRACE;

    CORE::CString seperator = "\n\n\n#********";
    if ( end )
    {
        seperator += " END OF AUTOGENERATED CONTENT ";
    }
    else
    {
        seperator += " START OF AUTOGENERATED CONTENT ";
    }
    seperator += "********\n\n\n";
    return seperator;
}

/*---------------------------------------------------------------------------*/

void
GenerateCMakeListsFile( const TProjectInfo& projectInfo ,
                        TModuleInfo& moduleInfo         ,
                        bool addCompileDate = false     )
{GUCEF_TRACE;

    // Set file header comment section
    CORE::CString fileContent = GetCMakeListsFileHeader( addCompileDate );

    // Set project name comment section
    fileContent += "\n# Configure " + moduleInfo.name + "\n\n";

    // Add all the include files
    fileContent += GenerateCMakeListsFileIncludeSection( moduleInfo.includeDirs );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Processed " + CORE::ToString( moduleInfo.includeDirs.size() ) + " include dirs for project " + moduleInfo.name );

    // Add all the source files
    fileContent += GenerateCMakeListsFileSrcSection( moduleInfo.sourceDirs );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Processed " + CORE::ToString( moduleInfo.sourceDirs.size() ) + " source dirs for project " + moduleInfo.name );

    // Add all platform files, headers and source
    fileContent += GenerateCMakeListsFilePlatformFilesSection( moduleInfo );

    fileContent += "# For ease of use make a variable that has all files for this module\nset( ALL_FILES ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADER_FILES} ${PLATFORM_SOURCE_FILES} )\n\n";

    fileContent += GenerateAutoGenertedSeperator( true );

    // We have completed generating the file content and gathering info
    moduleInfo.cmakeListFileContentPreSuffix = fileContent;

    fileContent = GenerateAutoGenertedSeperator( false );

    // Add all the include directories for this module
    fileContent += GenerateCMakeModuleIncludesSection( projectInfo, moduleInfo );

    fileContent += GenerateAutoGenertedSeperator( true );

    moduleInfo.cmakeListFileContentPostSuffix = fileContent;
}

/*---------------------------------------------------------------------------*/

void
FindSubDirsWithFileTypes( TProjectInfo& projectInfo          ,
                          TStringVectorMap& fileMap          ,
                          const TStringVector& fileTypes     ,
                          const CORE::CString& platform      ,
                          bool applyOnlyPlatformInstructions ,
                          const CORE::CString& curRootDir    ,
                          const CORE::CString& curRootDirSeg )
{GUCEF_TRACE;

    // First we build a list of all files for the directory for ease of handling
    TStringVector fileList;
    PopulateFileListFromDir( curRootDir, fileTypes, fileList );

    if ( fileList.size() > 0 )
    {
        // found files in the current root
        if ( !fileMap.insert( std::pair< CORE::CString, TStringVector >( curRootDirSeg, fileList ) ).second )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to add files to the file map for subdir \"" + curRootDirSeg + "\"" );
        }
    }

    // Get a list of sub-dirs
    TStringVector dirList;
    PopulateDirListFromDir( curRootDir, dirList );

    // Now we add/substract from that list based on generator instructions
    ExcludeOrIncludeDirEntriesAsSpecifiedForDir( projectInfo                   ,
                                                 curRootDir                    ,
                                                 platform                      ,
                                                 applyOnlyPlatformInstructions ,
                                                 dirList                       );

    TStringVector::iterator i = dirList.begin();
    while ( i != dirList.end() )
    {
        CORE::CString subDir = curRootDir;
        CORE::AppendToPath( subDir, (*i) );

        // Do not recurse into other project dirs or platform dirs
        if ( !IsDirAProjectDir( subDir ) && !IsDirAPlatformDir( subDir ) )
        {
            CORE::CString subDirSeg = curRootDirSeg;
            CORE::AppendToPath( subDirSeg, (*i) );
            subDirSeg.ReplaceChar( '\\', '/' );

            FindSubDirsWithFileTypes( projectInfo                   ,
                                      fileMap                       ,
                                      fileTypes                     ,
                                      platform                      ,
                                      applyOnlyPlatformInstructions ,
                                      subDir                        ,
                                      subDirSeg                     );
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
FindSubDirsWithFileTypes( TProjectInfo& projectInfo          ,
                          TStringVectorMap& fileMap          ,
                          const TStringVector& fileTypes     ,
                          const CORE::CString& curRootDir    ,
                          const CORE::CString& curRootDirSeg )
{GUCEF_TRACE;

    FindSubDirsWithFileTypes( projectInfo     ,
                              fileMap         ,
                              fileTypes       ,
                              CORE::CString() ,
                              false           ,
                              curRootDir      ,
                              curRootDirSeg   );
}

/*---------------------------------------------------------------------------*/

void
FindSubDirsWithHeaders( TProjectInfo& projectInfo ,
                        TModuleInfo& moduleInfo   )
{GUCEF_TRACE;

    // Look into the root itself and recuse downwards
    FindSubDirsWithFileTypes( projectInfo, moduleInfo.includeDirs, GetHeaderFileExtensions(), moduleInfo.rootDir, "" );
}

/*---------------------------------------------------------------------------*/

void
FindSubDirsWithSource( TProjectInfo& projectInfo ,
                       TModuleInfo& moduleInfo   )
{GUCEF_TRACE;

    // Look into the root itself and recuse downwards
    FindSubDirsWithFileTypes( projectInfo, moduleInfo.sourceDirs, GetSourceFileExtensions(), moduleInfo.rootDir, "" );
}

/*---------------------------------------------------------------------------*/

void
ProcessProjectDir( TProjectInfo& projectInfo ,
                   TModuleInfo& moduleInfo   )
{GUCEF_TRACE;

    // Set a project name based off the module sub-dir name
    // Best we can do unless we can get it from the suffix file later
    moduleInfo.name = CORE::LastSubDir( moduleInfo.rootDir );

    CORE::CString pathToSuffixFile = moduleInfo.rootDir;
    CORE::AppendToPath( pathToSuffixFile, "CMakeListsSuffix.txt" );

    if ( CORE::LoadTextFileAsString( pathToSuffixFile, moduleInfo.cmakeListSuffixFileContent ) )
    {
        // Fill in the dependencies as specified in the suffix file
        CORE::CString actualModuleName;
        moduleInfo.dependencies = ParseDependencies( moduleInfo.cmakeListSuffixFileContent, actualModuleName );
        moduleInfo.dependencyIncludeDirs = ParseIncludeDirs( moduleInfo.cmakeListSuffixFileContent );
        moduleInfo.isExecutable = false;
        ParseModuleProperties( moduleInfo.cmakeListSuffixFileContent, actualModuleName, moduleInfo.isExecutable );
        ParseSuffixFile( moduleInfo );
        if ( actualModuleName != moduleInfo.name )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Detected module name that differs from module sub-dir name, correcting module name from \"" + moduleInfo.name + "\" to \"" + actualModuleName + "\"" );
            moduleInfo.name = actualModuleName;
        }

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Processed suffix file for project " + moduleInfo.name );
    }
    else
    {
        // add suffix example section instead
        moduleInfo.cmakeListSuffixFileContent  = "## TODO: the following is an example suffix section, you have to complete it\n";
        moduleInfo.cmakeListSuffixFileContent += "#add_definitions(-DTIXML_USE_STL)\n";
        moduleInfo.cmakeListSuffixFileContent += "#add_executable(" + moduleInfo.name + " ${HEADER_FILES} ${SOURCE_FILES})\n";
        moduleInfo.cmakeListSuffixFileContent += "#target_link_libraries(" + moduleInfo.name + " ${GUCEF_LIBRARIES})\n";
        moduleInfo.cmakeListSuffixFileContent += "#gucef_config_tool(" + moduleInfo.name + ")\n";
    }

    FindSubDirsWithHeaders( projectInfo, moduleInfo );
    FindSubDirsWithSource( projectInfo, moduleInfo );
    GetAllPlatformFilesInPlatformDirs( projectInfo, moduleInfo );
    ApplyDirProcessingInstructionsToModule( projectInfo, moduleInfo );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Generated CMake List file content for project dir: " + moduleInfo.rootDir );
}

/*---------------------------------------------------------------------------*/

void
LocateAndProcessProjectDirsRecusively( TProjectInfo& projectInfo        ,
                                       const CORE::CString& topLevelDir )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_EVERYTHING, "Recursively processing directory for module info: " + topLevelDir );

    // Is this a project dir or some other dir?
    if ( IsDirAProjectDir( topLevelDir ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Determined that the following directory is a project directory: " + topLevelDir );

        // Process this dir
        TModuleInfo moduleInfo;
        moduleInfo.rootDir = topLevelDir;
        moduleInfo.buildOrder = 0;
        ProcessProjectDir( projectInfo, moduleInfo );
        projectInfo.modules.push_back( moduleInfo );
    }

    // Get all subdir's
    CORE::CString::StringVector dirList;
    PopulateDirListFromDir( topLevelDir, dirList );

    // Add/subtract dirs from the list based on generator instructions
    // This early application (before module definition) of processing instructions allows us
    // to limit the number of directories that need to be processed and thus speed things up a bit
    ExcludeOrIncludeDirEntriesAsSpecifiedForDir( projectInfo, topLevelDir, dirList );

    // Process all sub-dirs
    CORE::CString::StringVector::iterator i = dirList.begin();
    while ( i != dirList.end() )
    {
        CORE::CString subDir = topLevelDir;
        CORE::AppendToPath( subDir, (*i) );

        LocateAndProcessProjectDirsRecusively( projectInfo, subDir );
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
WriteCMakeListsFilesToDisk( TProjectInfo& projectInfo        ,
                            const CORE::CString& logFilename )
{GUCEF_TRACE;

    // Write all the CMakeLists.txt files
    std::vector< TModuleInfo >::iterator i = projectInfo.modules.begin();
    while ( i != projectInfo.modules.end() )
    {
        TModuleInfo& moduleInfo = (*i);

        CORE::CString pathToCMakeListsFile = moduleInfo.rootDir;
        CORE::AppendToPath( pathToCMakeListsFile, "CMakeLists.txt" );

        CORE::CString fileContent = moduleInfo.cmakeListFileContentPreSuffix +
                                    moduleInfo.cmakeListSuffixFileContent +
                                    moduleInfo.cmakeListFileContentPostSuffix;

        if ( logFilename.Length() > 0 )
        {
            fileContent += "# Generator logfile can be found at: " + logFilename;
        }

        CORE::WriteStringAsTextFile( pathToCMakeListsFile, fileContent );

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Created CMakeLists.txt file for project dir: " + moduleInfo.rootDir );
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
WriteCMakeListsFilesToDisk( TProjectInfo& projectInfo )
{GUCEF_TRACE;

    WriteCMakeListsFilesToDisk( projectInfo, CORE::CString() );
}

/*---------------------------------------------------------------------------*/

void
GenerateCMakeListsContentForModules( TProjectInfo& projectInfo   ,
                                     bool addCompileDate = false )
{GUCEF_TRACE;

    // Generate all the CMakeLists.txt file content
    std::vector< TModuleInfo >::iterator i = projectInfo.modules.begin();
    while ( i != projectInfo.modules.end() )
    {
        TModuleInfo& moduleInfo = (*i);
        GenerateCMakeListsFile( projectInfo, moduleInfo, addCompileDate );

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Generated CMakeLists.txt file contents for project dir: " + moduleInfo.rootDir );
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

int
GetModulePrio( TModulePrioMap& prioMap         ,
               const CORE::CString& moduleName )
{GUCEF_TRACE;

    TModulePrioMap::iterator i = prioMap.begin();
    while ( i != prioMap.end() )
    {
        if ( (*i).second->name == moduleName )
        {
            return (*i).first;
        }
        ++i;
    }
    return -1;
}

/*---------------------------------------------------------------------------*/

TModuleInfoPtrVector
GetModulesWithDependencyCounfOf( TModuleInfoVector& modules   ,
                                 CORE::UInt32 dependencyCount )
{GUCEF_TRACE;

    TModuleInfoPtrVector resultSet;
    TModuleInfoVector::iterator i = modules.begin();
    while ( i != modules.end() )
    {
        if ( (*i).dependencies.size() == dependencyCount )
        {
            resultSet.push_back( &( (*i) ) );
        }
        ++i;
    }
    return resultSet;
}

/*---------------------------------------------------------------------------*/

CORE::UInt32
GetHighestDependencyCount( TModuleInfoVector& modules )
{GUCEF_TRACE;

    CORE::UInt32 greatestDependencyCount = 0;
    TModuleInfoVector::iterator i = modules.begin();
    while ( i != modules.end() )
    {
        if ( (*i).dependencies.size() > greatestDependencyCount )
        {
            greatestDependencyCount = (*i).dependencies.size();
        }
        ++i;
    }
    return greatestDependencyCount;
}

/*---------------------------------------------------------------------------*/

void
SortModulesInDependencyOrder( TProjectInfo& projectInfo )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Sorting all modules based on build priority,.." );

    TModulePrioMap prioMap;

    // First we make sorting easier by putting all modules in the priority list
    // in such a way that they are already sorted somewhat based on their dependency count
    int prioInc=0;
    CORE::UInt32 highestDependencyCount = GetHighestDependencyCount( projectInfo.modules );
    for ( CORE::UInt32 i=0; i<=highestDependencyCount; ++i )
    {
        // Grab a list of modules with *i* dependencies
        TModuleInfoPtrVector modules = GetModulesWithDependencyCounfOf( projectInfo.modules, i );
        TModuleInfoPtrVector::iterator n = modules.begin();
        while ( n != modules.end() )
        {
            prioMap[ prioInc ] = (*n);
            ++n;
            ++prioInc;
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Performed initial sorting based on number of dependencies" );

    // Now we can bubble sort the priority map, because of the initial sorting done above
    // the number of iterations should be greatly reduced.
    bool changes = true;
    while ( changes )
    {
        changes = false;

        TModulePrioMap::iterator n = prioMap.begin();
        while ( n != prioMap.end() )
        {
            int modulePrio = (*n).first;
            TModuleInfo* moduleInfo = (*n).second;

            TStringVector::iterator m = moduleInfo->dependencies.begin();
            while ( m != moduleInfo->dependencies.end() )
            {
                // Logically we cannot have a prio higher then the dependency
                // so we will ensure it is lower
                int dependencyPrio = GetModulePrio( prioMap, (*m) );
                if ( dependencyPrio >= modulePrio )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Module " + moduleInfo->name + " with build priority " + CORE::Int32ToString( modulePrio ) +
                                " has dependency " + (*m) + " which has build priority " + CORE::Int32ToString( dependencyPrio ) + ", the dependency should have a lower priority (thus build earlier) then the module that requires it!"  );

                    TModulePrioMap newPrioMap;

                    // Set the new priority, the priority should be higher then the dependency
                    // causing it to be build after the dependency (lower prio = builder earlier)
                    modulePrio = dependencyPrio+1;

                    // Now insert our reprioritized item at this location
                    newPrioMap[ modulePrio ] = moduleInfo;

                    // Now add everything around the reprioritized item to our
                    // new prio map
                    TModulePrioMap::iterator p = prioMap.begin();
                    while ( p != prioMap.end() )
                    {
                        if ( (*p).first < modulePrio )
                        {
                            if ( (*p).second->name != moduleInfo->name )
                            {
                                newPrioMap[ (*p).first ] = (*p).second;
                            }
                        }
                        else
                        if ( (*p).first >= modulePrio )
                        {
                            if ( (*p).second->name != moduleInfo->name )
                            {
                                newPrioMap[ (*p).first + 1 ] = (*p).second;
                                GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Changed build priority for module: " + (*p).second->name +
                                            " from " + CORE::Int32ToString( (*p).first ) + " to " + CORE::Int32ToString( (*p).first+1 ) );
                            }
                        }
                        ++p;
                    }

                    // Reindex list to close gap
                    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Reindexing the build priority list" );
                    CORE::Int32 i=0;
                    TModulePrioMap newestPrioMap;
                    p = newPrioMap.begin();
                    while ( p != newPrioMap.end() )
                    {
                        newestPrioMap[ i ] = (*p).second;

                        if ( i != (*p).first )
                        {
                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Changed build priority for module: " + (*p).second->name +
                                        " from " + CORE::Int32ToString( (*p).first ) + " to " + CORE::Int32ToString( i ) );
                        }
                        ++i; ++p;
                    }

                    #ifdef GUCEF_CORE_DEBUG_MODE

                    // For debug: output final differeces between the altered list and the original
                    TModulePrioMap::iterator q = prioMap.begin();
                    p = newestPrioMap.begin();
                    while ( p != newestPrioMap.end() )
                    {
                        if ( (*p).second != (*q).second )
                        {
                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Difference with original build order: module " + (*p).second->name +
                                        " is now at index " + CORE::Int32ToString( (*p).first ) + " where module " + (*q).second->name + " used to be" );
                        }
                        ++q; ++p;
                    }

                    #endif

                    // Replace the old map with the new one and start the next bubbling iteration
                    prioMap = newestPrioMap;
                    changes = true;

                    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Completed changing the build priority for module: " + moduleInfo->name );
                    break;
                }
                ++m;
            }

            // Restart the process if something had to be changed
            if ( changes )
            {
                break;
            }

            ++n;
        }
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Finished determining the correct build order (priority) for all modules, assigning priorities and reordering modules to refect this" );

    // First assign the determined build order index to the module
    TModuleInfoVector newModuleInfoVector;
    TModulePrioMap::iterator n = prioMap.begin();
    while ( n != prioMap.end() )
    {
        TModuleInfo* moduleInfo = (*n).second;
        moduleInfo->buildOrder = (*n).first;

        newModuleInfoVector.push_back( *moduleInfo );
        ++n;
    }
    projectInfo.modules = newModuleInfoVector;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Finished assigning the correct build order for all modules and sorted them accordingly" );
}

/*---------------------------------------------------------------------------*/

bool
GenerateProjectInfoDataTree( const TProjectInfo& projectInfo ,
                             CORE::CDataNode& outputInfo     )
{GUCEF_TRACE;

    // start from a clean slate
    outputInfo.Clear();

    // Add project info
    outputInfo.SetName( "Project" );
    outputInfo.SetAttribute( "ModuleCount", (UInt64) projectInfo.modules.size() );

    // Add info for each module
    TModuleInfoVector::const_iterator i = projectInfo.modules.begin();
    while ( i != projectInfo.modules.end() )
    {
        const TModuleInfo& moduleInfo = (*i);

        // Add basic module info
        CORE::CDataNode moduleInfoNode;
        moduleInfoNode.SetName( "Module" );
        moduleInfoNode.SetAttribute( "Name", moduleInfo.name );
        moduleInfoNode.SetAttribute( "BuildOrder", CORE::Int32ToString( moduleInfo.buildOrder ) );
        moduleInfoNode.SetAttribute( "RootDir", moduleInfo.rootDir );
        moduleInfoNode.SetAttribute( "IsExecutable", CORE::BoolToString( moduleInfo.isExecutable ) );

        // Add headers for all platforms
        CORE::CDataNode headersInfoNode;
        headersInfoNode.SetName( "Files" );
        headersInfoNode.SetAttribute( "Type", "Headers" );
        headersInfoNode.SetAttribute( "Platform", "All" );
        headersInfoNode.SetAttribute( "DirCount", (UInt64) moduleInfo.includeDirs.size() );
        TStringVectorMap::const_iterator n = moduleInfo.includeDirs.begin();
        while ( n != moduleInfo.includeDirs.end() )
        {
            CORE::CDataNode pathNode;
            pathNode.SetName( "Dir" );
            pathNode.SetAttribute( "Path", (*n).first );

            CORE::CDataNode fileNode;
            fileNode.SetName( "File" );

            const TStringVector& fileVector = (*n).second;
            pathNode.SetAttribute( "FileCount", (UInt64) fileVector.size() );
            TStringVector::const_iterator m = fileVector.begin();
            while ( m != fileVector.end() )
            {
                fileNode.SetAttribute( "Name", (*m) );
                pathNode.AddChild( fileNode );
                ++m;
            }

            headersInfoNode.AddChild( pathNode );
            ++n;
        }
        moduleInfoNode.AddChild( headersInfoNode );
        headersInfoNode.DelSubTree();

        // Add headers for specific platforms
        TStringVectorMapMap::const_iterator x = moduleInfo.platformHeaderFiles.begin();
        while ( x != moduleInfo.platformHeaderFiles.end() )
        {
            const TStringVectorMap& platformHeaders = (*x).second;
            headersInfoNode.SetAttribute( "Platform", (*x).first );
            headersInfoNode.SetAttribute( "DirCount", (UInt64) platformHeaders.size() );

            n = platformHeaders.begin();
            while ( n != platformHeaders.end() )
            {
                CORE::CDataNode pathNode;
                pathNode.SetName( "Dir" );
                pathNode.SetAttribute( "Path", (*n).first );

                CORE::CDataNode fileNode;
                fileNode.SetName( "File" );

                const TStringVector& fileVector = (*n).second;
                pathNode.SetAttribute( "FileCount", (UInt64) fileVector.size() );
                TStringVector::const_iterator m = fileVector.begin();
                while ( m != fileVector.end() )
                {
                    fileNode.SetAttribute( "Name", (*m) );
                    pathNode.AddChild( fileNode );
                    ++m;
                }

                headersInfoNode.AddChild( pathNode );
                ++n;
            }
            moduleInfoNode.AddChild( headersInfoNode );
            headersInfoNode.DelSubTree();
            ++x;
        }

        // Add sources for all platforms
        CORE::CDataNode sourceInfoNode;
        sourceInfoNode.SetName( "Files" );
        sourceInfoNode.SetAttribute( "Type", "Source" );
        sourceInfoNode.SetAttribute( "Platform", "All" );
        sourceInfoNode.SetAttribute( "DirCount", (UInt64) moduleInfo.sourceDirs.size() );
        n = moduleInfo.sourceDirs.begin();
        while ( n != moduleInfo.sourceDirs.end() )
        {
            CORE::CDataNode pathNode;
            pathNode.SetName( "Dir" );
            pathNode.SetAttribute( "Path", (*n).first );

            CORE::CDataNode fileNode;
            fileNode.SetName( "File" );

            const TStringVector& fileVector = (*n).second;
            pathNode.SetAttribute( "FileCount", (UInt64) fileVector.size() );
            TStringVector::const_iterator m = fileVector.begin();
            while ( m != fileVector.end() )
            {
                fileNode.SetAttribute( "Name", (*m) );
                pathNode.AddChild( fileNode );
                ++m;
            }

            sourceInfoNode.AddChild( pathNode );
            ++n;
        }
        moduleInfoNode.AddChild( sourceInfoNode );
        sourceInfoNode.DelSubTree();

        // Add sources for specific platforms
        x = moduleInfo.platformSourceFiles.begin();
        while ( x != moduleInfo.platformSourceFiles.end() )
        {
            const TStringVectorMap& platformSources = (*x).second;
            sourceInfoNode.SetAttribute( "Platform", (*x).first );
            sourceInfoNode.SetAttribute( "DirCount", (UInt64) moduleInfo.sourceDirs.size() );

            n = platformSources.begin();
            while ( n != platformSources.end() )
            {
                CORE::CDataNode pathNode;
                pathNode.SetName( "Dir" );
                pathNode.SetAttribute( "Path", (*n).first );

                CORE::CDataNode fileNode;
                fileNode.SetName( "File" );

                const TStringVector& fileVector = (*n).second;
                pathNode.SetAttribute( "FileCount", (UInt64) fileVector.size() );
                TStringVector::const_iterator m = fileVector.begin();
                while ( m != fileVector.end() )
                {
                    fileNode.SetAttribute( "Name", (*m) );
                    pathNode.AddChild( fileNode );
                    ++m;
                }

                sourceInfoNode.AddChild( pathNode );
                ++n;
            }
            moduleInfoNode.AddChild( sourceInfoNode );
            sourceInfoNode.DelSubTree();
            ++x;
        }

        // Add includes for all platforms
        CORE::CDataNode includesInfoNode;
        includesInfoNode.SetName( "Includes" );
        includesInfoNode.SetAttribute( "Platform", "All" );
        includesInfoNode.SetAttribute( "Count", (UInt64) moduleInfo.dependencyIncludeDirs.size() );
        includesInfoNode.SetAttribute( "Source", "Dependency" );
        TStringSet::const_iterator h = moduleInfo.dependencyIncludeDirs.begin();
        while ( h !=  moduleInfo.dependencyIncludeDirs.end() )
        {
            CORE::CDataNode includeNode;
            includeNode.SetName( "Include" );
            includeNode.SetAttribute( "Path", (*h) );

            includesInfoNode.AddChild( includeNode );
            ++h;
        }
        moduleInfoNode.AddChild( includesInfoNode );
        includesInfoNode.DelSubTree();

        // Add includes for specific platforms
        TStringSetMap::const_iterator q = moduleInfo.dependencyPlatformIncludeDirs.begin();
        while ( q != moduleInfo.dependencyPlatformIncludeDirs.end() )
        {
            const TStringSet& platformIncludes = (*q).second;
            includesInfoNode.SetAttribute( "Platform", (*q).first );
            includesInfoNode.SetAttribute( "Count", (UInt64) platformIncludes.size() );
            includesInfoNode.SetAttribute( "Source", "Dependency" );

            h = platformIncludes.begin();
            while ( h != platformIncludes.end() )
            {
                CORE::CDataNode includeNode;
                includeNode.SetName( "Include" );
                includeNode.SetAttribute( "Path", (*h) );

                includesInfoNode.AddChild( includeNode );
                ++h;
            }
            moduleInfoNode.AddChild( includesInfoNode );
            includesInfoNode.DelSubTree();
            ++q;
        }

        // Add all the regular include dirs for this module
        // These are already represented in the path attribute of the files section
        // but for ease of processing and clarity they are provided again in the includes section
        includesInfoNode.SetAttribute( "Platform", "All" );
        includesInfoNode.SetAttribute( "Count", (UInt64) moduleInfo.includeDirs.size() );
        includesInfoNode.SetAttribute( "Source", "Self" );
        n = moduleInfo.includeDirs.begin();
        while ( n != moduleInfo.includeDirs.end() )
        {
            CORE::CString includeDir = (*n).first.ReplaceChar( '\\', '/' );
            if ( 0 != includeDir.Length() )
            {
                CORE::CDataNode includeNode;
                includeNode.SetName( "Include" );
                includeNode.SetAttribute( "Path", includeDir );
                includesInfoNode.AddChild( includeNode );
            }
            else
            {
                // Check if there is more then one include dir
                // If so we have create an include for an empty include dir
                // to ensure files in subdirs can include the file with the zero length
                // subdir.
                if ( 1 < moduleInfo.includeDirs.size() )
                {
                    CORE::CString includeDir = "../" + CORE::LastSubDir( moduleInfo.rootDir ) + " ";
                    CORE::CDataNode includeNode;
                    includeNode.SetName( "Include" );
                    includeNode.SetAttribute( "Path", includeDir );
                    includesInfoNode.AddChild( includeNode );
                }
            }
            ++n;
        }
        moduleInfoNode.AddChild( includesInfoNode );
        includesInfoNode.DelSubTree();

        //includesInfoNode.SetAttribute( "Count", CORE::UInt32ToString( moduleInfo.platformHeaderFiles.size() ) );
        //n = moduleInfo.includeDirs.begin();
        //while ( n != moduleInfo.includeDirs.end() )
        //{
        //    CORE::CString includeDir = (*n).first.ReplaceChar( '\\', '/' );
        //    if ( 0 != includeDir.Length() )
        //    {
        //        CORE::CDataNode includeNode;
        //        includeNode.SetName( "Include" );
        //        includeNode.SetAttribute( "Path", includeDir );
        //        includesInfoNode.AddChild( includeNode );
        //    }
        //    else
        //    {
        //        // Check if there is more then one include dir
        //        // If so we have create an include for an empty include dir
        //        // to ensure files in subdirs can include the file with the zero length
        //        // subdir.
        //        if ( 1 < moduleInfo.includeDirs.size() )
        //        {
        //            CORE::CString includeDir = "../" + CORE::LastSubDir( moduleInfo.rootDir ) + " ";
        //            CORE::CDataNode includeNode;
        //            includeNode.SetName( "Include" );
        //            includeNode.SetAttribute( "Path", includeDir );
        //            includesInfoNode.AddChild( includeNode );
        //        }
        //    }
        //    ++n;
        //}
        //moduleInfoNode.AddChild( includesInfoNode );
        //includesInfoNode.DelSubTree();

        // Add all the module dependencies
        CORE::CDataNode dependenciesNode;
        dependenciesNode.SetName( "Dependencies" );
        dependenciesNode.SetAttribute( "Count", (UInt64) moduleInfo.dependencies.size() );
        TStringVector::const_iterator m = moduleInfo.dependencies.begin();
        while ( m != moduleInfo.dependencies.end() )
        {
            CORE::CDataNode dependencyNode;
            dependencyNode.SetName( "Dependency" );
            dependencyNode.SetAttribute( "Name", (*m) );
            dependenciesNode.AddChild( dependencyNode );
            ++m;
        }
        moduleInfoNode.AddChild( dependenciesNode );

        // Add all the libraries that are linked but not part of the overall project
        CORE::CDataNode linkedLibrariesNode;
        linkedLibrariesNode.SetName( "LinkedLibraries" );
        linkedLibrariesNode.SetAttribute( "Count", (UInt64) moduleInfo.linkedLibraries.size() );
        m = moduleInfo.linkedLibraries.begin();
        while ( m != moduleInfo.linkedLibraries.end() )
        {
            CORE::CDataNode libraryNode;
            libraryNode.SetName( "LinkedLibrary" );
            libraryNode.SetAttribute( "Name", (*m) );
            linkedLibrariesNode.AddChild( libraryNode );
            ++m;
        }
        moduleInfoNode.AddChild( linkedLibrariesNode );

        // Add all the info for this module to the overall project
        outputInfo.AddChild( moduleInfoNode );
        ++i;
    }

    return true;
}

/*---------------------------------------------------------------------------*/

bool
WriteProjectInfoDataTreeToDisk( const TProjectInfo& projectInfo         ,
                                const CORE::CString& outputInfoFilename )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Preparing to write all project information to disk file \"" + outputInfoFilename + "\"" );

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec = GetXmlDStoreCodec();
    if ( !codec.IsNULL() )
    {
        CORE::CDataNode info;
        if ( GenerateProjectInfoDataTree( projectInfo, info ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully generated a data tree with all project information" );

            if ( codec->StoreDataTree( &info, outputInfoFilename ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully wrote all project information to disk file \"" + outputInfoFilename + "\"" );
                return true;
            }
        }
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to write all project information to disk file \"" + outputInfoFilename + "\"" );
    return false;
}

/*---------------------------------------------------------------------------*/

void
ParseParams( const int argc                 ,
             char* argv[]                   ,
             CORE::CValueList& keyValueList )
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
            argString += ' ' + CORE::CString( argv[ i ] );
        }

        // Parse the param list based on the ' symbol
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Application parameters: " + argString );
        keyValueList.SetMultiple( argString, '*' );
    }
}

/*---------------------------------------------------------------------------*/

int
main( int argc , char* argv[] )
{GUCEF_TRACE;

    CORE::CString logFilename = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
    CORE::AppendToPath( logFilename, "CMakeListsGenerator_Log.txt" );
    CORE::CFileAccess logFileAccess( logFilename, "w" );

    CORE::CStdLogger logger( logFileAccess );
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    CORE::CMSWinConsoleLogger consoleOut;
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &consoleOut );
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    CORE::CXTermConsoleLogger consoleOut;
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &consoleOut );
    #endif

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "This tool was compiled on: " __DATE__ " @ " __TIME__ );

    CORE::CValueList keyValueList;
    ParseParams( argc, argv, keyValueList );

    TStringVector rootDirs;
    try
    {
        rootDirs = keyValueList.GetValueStringVector( "rootDir" );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Number of rootDir arguments passed from command line: " + CORE::ToString( rootDirs.size() ) );
    }
    catch ( CORE::CValueList::EUnknownKey& )
    {
        rootDirs.push_back( CORE::RelativePath( "$CURWORKDIR$" ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Using current working directory since no rootDir arguments where passed from the command line" );
    }

    // Set any global dir excludes that where passed as cmd parameters
    TProjectInfo projectInfo;
    projectInfo.globalDirExcludeList = keyValueList.GetValueAlways( "dirsToIgnore" ).AsString().ParseElements( ';', false );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "There are " + CORE::ToString( projectInfo.globalDirExcludeList.size() ) + " dirs in the global dir ignore list" );

    // Gather all processing instructions
    TStringVector::iterator i = rootDirs.begin();
    while ( i != rootDirs.end() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Recursively loading all processing instructions for root directory \"" + (*i) + "\"" );
        LoadAllProcessingInstructions( projectInfo, (*i) );
        ++i;
    }

    // Gather all module information
    i = rootDirs.begin();
    while ( i != rootDirs.end() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Identifying all modules for root directory \"" + (*i) + "\"" );
        LocateAndProcessProjectDirsRecusively( projectInfo, (*i) );
        ++i;
    }

    // Order the modules in the list such so that they are placed in the order they need to be build
    SortModulesInDependencyOrder( projectInfo );

    // Now that everything is sorted and we have all the info we can generate all the include paths
    GenerateModuleIncludes( projectInfo );

    // Write all the project information we gathered to disk before doing any CMake specific output
    CORE::CString projectinfoFilename = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
    CORE::AppendToPath( projectinfoFilename, "Project.xml" );
    WriteProjectInfoDataTreeToDisk( projectInfo, projectinfoFilename );

    bool addToolCompileTimeToOutput = CORE::StringToBool( keyValueList.GetValueAlways( "addToolCompileTimeToOutput" ) );

    // Generate the contents of the CMakeLists files
    GenerateCMakeListsContentForModules( projectInfo, addToolCompileTimeToOutput );

    bool writeLogLocationToOutput = CORE::StringToBool( keyValueList.GetValueAlways( "writeLogLocationToOutput" ) );

    // Now write what we created to disk
    if ( writeLogLocationToOutput )
    {
        WriteCMakeListsFilesToDisk( projectInfo, logFilename );
    }
    else
    {
        WriteCMakeListsFilesToDisk( projectInfo );
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Wrote log file to: " + logFilename );
}

/*---------------------------------------------------------------------------*/

#ifdef GUCEF_MSWIN_BUILD

int __stdcall
WinMain( HINSTANCE hinstance     ,
         HINSTANCE hprevinstance ,
         LPSTR lpcmdline         ,
         int ncmdshow            )
{GUCEF_TRACE;

    int argc = 0;
    char** argv = &lpcmdline;
    if ( lpcmdline != NULL )
    {
        if ( *lpcmdline != '\0' )
        {
            argc = 1;
        }
    }

    return main( argc, argv );
}

#endif

/*---------------------------------------------------------------------------*/
