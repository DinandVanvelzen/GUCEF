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

#ifndef GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H
#include "CDStoreCodecPluginManager.h"
#define GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H
#endif /* GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#include "gucefProjectGen_DataTypes.h"

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

const CORE::CString AllPlatforms = "all";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
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

static bool
IsStringInList( const TStringSet& list          ,
                bool caseSensitive              ,
                const CORE::CString& testString )
{GUCEF_TRACE;

    TStringSet::const_iterator i = list.begin();
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

static bool
IsStringInKeyList( const TStringVectorMap& list    ,
                   bool caseSensitive              ,
                   const CORE::CString& testString )
{GUCEF_TRACE;

    TStringVectorMap::const_iterator i = list.begin();
    while ( i != list.end() )
    {
        if ( (*i).first.Equals( testString, caseSensitive ) )
        {
            return true;
        }
        ++i;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void
MergeStringVector( TStringVector& targetList          ,
                   const TStringVector& listToMergeIn ,
                   bool caseSensitive                 )
{GUCEF_TRACE;

    TStringVector::const_iterator i = listToMergeIn.begin();
    while ( i != listToMergeIn.end() )
    {
        if ( !IsStringInList( targetList, caseSensitive, (*i) ) )
        {
            targetList.push_back( (*i) );
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

static bool
IsKeyStringInMap( const CORE::CStringMap& testMap ,
                  bool caseSensitive              ,
                  const CORE::CString& testString )
{
    if ( !caseSensitive )
    {
        CORE::CStringMap::const_iterator i = testMap.find( testString );
        return i != testMap.end();
    }
    
    CORE::CStringMap::const_iterator i = testMap.begin();
    while ( i != testMap.end() )
    {
        if ( (*i).first.Equals( testString, caseSensitive )  )
        {
            return true;
        }
        ++i;
    }  
    return false;
}

/*---------------------------------------------------------------------------*/

static void
MergeStringMap( CORE::CStringMap& targetMap          ,
                const CORE::CStringMap& mapToMergeIn ,
                bool caseSensitive                   ,
                bool concatExistingEntries           )
{GUCEF_TRACE;

    CORE::CStringMap::const_iterator i = mapToMergeIn.begin();
    while ( i != mapToMergeIn.end() )
    {
        if ( !IsKeyStringInMap( targetMap, caseSensitive, (*i).first ) )
        {
            targetMap.insert( (*i) );
        }
        else
        {
            if ( concatExistingEntries )
            {
                targetMap[ (*i).first ] += (*i).second;
            }
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
MergeStringSet( TStringSet& targetList          ,
                const TStringSet& listToMergeIn ,
                bool caseSensitive              )
{GUCEF_TRACE;

    TStringSet::const_iterator i = listToMergeIn.begin();
    while ( i != listToMergeIn.end() )
    {
        if ( caseSensitive )
        {
            TStringSet::iterator n = targetList.find( (*i) );
            if ( n == targetList.end() )
            {
                targetList.insert( (*i) );
            }
        }
        else
        {
            if ( !IsStringInList( targetList, caseSensitive, (*i) ) )
            {
                targetList.insert( (*i) );
            }
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

TStringSet
StringVectorToStringSet( const TStringVector& stringVector )
{GUCEF_TRACE;

    TStringSet stringSet;
    TStringVector::const_iterator i = stringVector.begin();
    while ( i != stringVector.end() )
    {
        stringSet.insert( (*i) );
        ++i;
    }
    return stringSet;
}

/*---------------------------------------------------------------------------*/

void
MergeStringVectorMap( TStringVectorMap& targetMap          ,
                      const TStringVectorMap& mapToMergeIn ,
                      bool caseSensitive                   )
{GUCEF_TRACE;

    TStringVectorMap::const_iterator i = mapToMergeIn.begin();
    while ( i != mapToMergeIn.end() )
    {
        MergeStringVector( targetMap[ (*i).first ], (*i).second, caseSensitive );
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
MergeStringSetMap( TStringSetMap& targetMap          ,
                   const TStringSetMap& mapToMergeIn ,
                   bool caseSensitive                )
{GUCEF_TRACE;

    TStringSetMap::const_iterator i = mapToMergeIn.begin();
    while ( i != mapToMergeIn.end() )
    {
        MergeStringSet( targetMap[ (*i).first ], (*i).second, caseSensitive );
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

CORE::CString
ModuleTypeToString( const TModuleType moduleType )
{GUCEF_TRACE;

    switch ( moduleType )
    {
        case MODULETYPE_UNDEFINED: return "";
        case MODULETYPE_EXECUTABLE: return "Executable";
        case MODULETYPE_SHARED_LIBRARY: return "SharedLibrary";
        case MODULETYPE_STATIC_LIBRARY: return "StaticLibrary";
        case MODULETYPE_HEADER_INCLUDE_LOCATION: return "HeaderIncludeLocation";
        case MODULETYPE_HEADER_INTEGRATE_LOCATION: return "HeaderIntegrateLocation";
        case MODULETYPE_CODE_INTEGRATE_LOCATION: return "CodeIntegrateLocation";
        case MODULETYPE_REFERENCE_LIBRARY: return "ReferenceLibrary";
        case MODULETYPE_BINARY_PACKAGE : return "BinaryPackage";
        case MODULETYPE_UNKNOWN: return "Unknown";
        default: return "";
    }
}

/*-------------------------------------------------------------------------*/

TModuleType
StringToModuleType( const CORE::CString& moduleTypeStr )
{
    CORE::CString moduleTypeString = moduleTypeStr.Lowercase();
    if ( moduleTypeString == "" ) return MODULETYPE_UNDEFINED;
    if ( moduleTypeString == "executable" ) return MODULETYPE_EXECUTABLE;
    if ( moduleTypeString == "sharedlibrary" ) return MODULETYPE_SHARED_LIBRARY;
    if ( moduleTypeString == "staticlibrary" ) return MODULETYPE_STATIC_LIBRARY;
    if ( moduleTypeString == "headerincludelocation" ) return MODULETYPE_HEADER_INCLUDE_LOCATION;
    if ( moduleTypeString == "headerintegratelocation" ) return MODULETYPE_HEADER_INTEGRATE_LOCATION;
    if ( moduleTypeString == "codeintegratelocation" ) return MODULETYPE_CODE_INTEGRATE_LOCATION;
    if ( moduleTypeString == "referencelibrary" ) return MODULETYPE_REFERENCE_LIBRARY;
    if ( moduleTypeString == "binarypackage" ) return MODULETYPE_BINARY_PACKAGE;
    if ( moduleTypeString == "unknown" ) return MODULETYPE_UNKNOWN;
    return MODULETYPE_UNDEFINED;
}

/*-------------------------------------------------------------------------*/

CORE::CDStoreCodecRegistry::TDStoreCodecPtr
GetXmlDStoreCodec( void )
{GUCEF_TRACE;

    static CORE::CDStoreCodecRegistry::TDStoreCodecPtr codecPtr;
    if ( codecPtr.IsNULL() )
    {
        CORE::CDStoreCodecRegistry* registry = &CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry();
        if ( !registry->TryGetXmlCodec( codecPtr ) )
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

bool
ContainsFileWithFileExtension( const TStringSetMap& files    ,
                               const CORE::CString& fileExt  )
{GUCEF_TRACE;

    TStringSetMap::const_iterator i = files.begin();
    while ( i != files.end() )
    {
        TStringSet::const_iterator n = (*i).second.begin();
        while ( n != (*i).second.end() )
        {
            if ( fileExt.Equals( CORE::ExtractFileExtention( (*n) ), false ) )
                return true;
            ++n;
        }
        ++i;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void
ApplyConfigToProject( const CORE::CDataNode& loadedConfig , 
                      CProjectInfo& projectInfo           )
{GUCEF_TRACE;
    
    CORE::CDataNode::TConstDataNodeSet platformDefs = loadedConfig.FindChildrenOfType( "PlatformDefinitions", true );
    CORE::CDataNode::TConstDataNodeSet::const_iterator i = platformDefs.begin();
    while ( i != platformDefs.end() )
    {
        const CORE::CDataNode* definitionGroup = (*i);
        CORE::CDataNode::const_iterator n = definitionGroup->ConstBegin();
        while ( n != definitionGroup->ConstEnd() )
        {
            const CORE::CDataNode* platform = (*n);
            const CORE::CString platformName = platform->GetName();
            CORE::CString aliases = platform->GetAttributeValueOrChildValueByName( "Aliases" );
            CORE::CString platformDirs = platform->GetAttributeValueOrChildValueByName( "PlatformDirs" );

            TPlatformDefinition& platformDef = (projectInfo.platforms)[ platformName ];
            platformDef.aliases = aliases.ParseUniqueElements( ';', false );
            platformDef.platformDirs = platformDirs.ParseUniqueElements( ';', false );

            ++n;
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

CORE::CString
GetLanguageForModule( const CModuleInfoPtr& moduleInfo )
{GUCEF_TRACE;

    const TStringSet& languageSet = moduleInfo->compilerSettings.GetLanguagesUsed();
    if ( languageSet.empty() )
    {
        // No language was specified but premake requires one
        // We will determine the language based on the files in the project
        if ( ContainsFileWithFileExtension( moduleInfo->GetSourceDirs(), "CS" ) )
        {
            return "C#";
        }
        else
        if ( ( ContainsFileWithFileExtension( moduleInfo->GetSourceDirs(), "CPP" ) ) ||
             ( ContainsFileWithFileExtension( moduleInfo->GetSourceDirs(), "CXX" ) ) ||
             ( ContainsFileWithFileExtension( moduleInfo->GetSourceDirs(), "CHH" ) ) )
        {
            return "C++";
        }
        else
        {
            return "C";
        }
    }
    else
    {
        // Premake supports only 1 language per module so we list the first one
        return (*languageSet.begin()).Uppercase();
    }
}

/*-------------------------------------------------------------------------*/

TStringSet
ResolveMultiPlatformName( const CORE::CString& platformName          ,
                          const TPlatformDefinitionMap* platformDefs )
{GUCEF_TRACE;

    TStringSet resultSet;

    // Save some effort for "All Platforms"...
    if ( platformName == AllPlatforms )
    {
        resultSet.insert( platformName );
        return resultSet;
    }
        
    TStringVector elements = platformName.Lowercase().ParseElements( ';', false );
    TStringVector::iterator i = elements.begin();
    while ( i != elements.end() )
    {        
        const CORE::CString& element = (*i);
        
        if ( GUCEF_NULL != platformDefs && !platformDefs->empty() )
        {
            TPlatformDefinitionMap::const_iterator n = platformDefs->begin();
            while ( n != platformDefs->end() )
            {
                // Check to see if this is a regular platform name in its own right
                if ( (*n).first == element )
                {
                    resultSet.insert( element );
                    ++n;
                    continue;
                }
                
                // Check to see if this is an alias for the current platform
                const TPlatformDefinition& platformDef = (*n).second;
                const TStringSet& aliases = platformDef.aliases;
                TStringSet::const_iterator m = aliases.begin();
                while ( m != aliases.end() )
                {
                    if ( (*m) == element )
                    {
                        // Add the regular platform name as a result of matching the alias
                        resultSet.insert( (*n).first );
                        break;
                    }
                    ++m;
                }
                ++n;
            }
        }
        else
        {
            // Use the legacy hardcoded aliases

            if ( (*i).Equals( "mswin", false ) )
            {
                resultSet.insert( "win32" );
                resultSet.insert( "win64" );
            }
            else
            if ( (*i).Equals( "linux", false ) )
            {
                resultSet.insert( "linux32" );
                resultSet.insert( "linux64" );
            }
            else
            {
                resultSet.insert( (*i).Lowercase() );
            }
        }
        ++i;
    }

    if ( resultSet.empty() )
        resultSet.insert( platformName );
    return resultSet;
}

/*-------------------------------------------------------------------------*/

// Serialize a specific moduleInfo which is derived off of
// the given entry. The given moduleInfo can be an independent platform
// moduleInfo or a merged moduleInfo
bool
SerializeModuleInfo( const CModuleInfoEntryPtr& moduleEntry ,
                     const CModuleInfoPtr& moduleInfo       ,
                     const CORE::CString& platform          ,
                     CORE::CDataNode& parentNode            )
{GUCEF_TRACE;

    bool totalSuccess = true;

    // Add basic module info
    CORE::CDataNode moduleInfoNode;
    moduleInfoNode.SetName( "Module" );
    
    if ( !moduleInfo->name.IsNULLOrEmpty() )
    {
        moduleInfoNode.SetAttribute( "Name", moduleInfo->name );
    }

    if ( moduleInfo->metadata.HasAnyMetaData() )
    {
        CORE::CDataNode* metaDataNode = moduleInfoNode.AddChild( "ModuleMetaData" );
        if ( GUCEF_NULL != metaDataNode )
        {
            CORE::CDataNodeSerializableSettings defaultSettings;
            totalSuccess = moduleInfo->metadata.Serialize( *metaDataNode, defaultSettings ) && totalSuccess;
        }
    }
    //if ( moduleInfo->metadata.HasSemVer() )
    //{
    //    moduleInfoNode.SetAttribute( "SemVer", moduleInfo->metadata.GetSemVer().ToString() );
    //    moduleInfoNode.SetAttribute( "HasSemVer", true );
    //}

    if ( moduleInfo->buildOrder != -1 )
    {
        moduleInfoNode.SetAttribute( "BuildOrder", moduleInfo->buildOrder );
    }
    //if ( moduleInfo->buildChain != -1 )
    //{
    //    moduleInfoNode.SetAttribute( "BuildChain", CORE::Int32ToString( moduleInfo->buildChain ) );
    //}
    //if ( !moduleInfo->buildChainDependencies.empty() )
    //{
    //    TInt32Set::iterator n = moduleInfo->buildChainDependencies.begin();
    //    CORE::CString buildChainDepsValue;
    //    for ( UInt32 i=0; i+1 < moduleInfo->buildChainDependencies.size(); ++i )
    //    {                     
    //        buildChainDepsValue += (*n) + ';';
    //        ++n;
    //    }
    //    buildChainDepsValue += (*n);
    //    moduleInfoNode.SetAttribute( "BuildChainDeps", buildChainDepsValue );
    //}
    if ( moduleInfo->moduleType != MODULETYPE_UNDEFINED )
    {
        moduleInfoNode.SetAttribute( "Type", ModuleTypeToString( moduleInfo->moduleType ) );
    }
    if ( !moduleInfo->tags.empty() )
    {
        UInt32 i=0;
        CORE::CString tagValue;
        TStringSet::iterator s = moduleInfo->tags.begin();
        while ( s != moduleInfo->tags.end() )
        {                     
            tagValue += (*s) + ';';
            ++s;
        }
        if ( !tagValue.IsNULLOrEmpty() )
            tagValue = tagValue.CutChars( 1, false, 0 );
        moduleInfoNode.SetAttribute( "Tags", tagValue );
    }

    moduleInfoNode.SetAttribute( "Platform", platform );

    // Add headers
    if ( moduleInfo->GetIncludeDirs().size() > 0 )
    {
        CORE::CDataNode headersInfoNode;
        headersInfoNode.SetName( "Files" );
        headersInfoNode.SetAttribute( "Type", "Headers" );
        headersInfoNode.SetAttribute( "DirCount", CORE::ToString( moduleInfo->GetIncludeDirs().size() ) );
        TStringSetMap::const_iterator n = moduleInfo->GetIncludeDirs().begin();
        while ( n != moduleInfo->GetIncludeDirs().end() )
        {
            CORE::CDataNode pathNode;
            pathNode.SetName( "Dir" );
            pathNode.SetAttribute( "Path", (*n).first );

            CORE::CDataNode fileNode;
            fileNode.SetName( "File" );

            const TStringSet& fileSet = (*n).second;
            pathNode.SetAttribute( "FileCount", CORE::ToString( fileSet.size() ) );
            TStringSet::const_iterator m = fileSet.begin();
            while ( m != fileSet.end() )
            {
                fileNode.SetAttribute( "Name", (*m) );
                pathNode.AddChild( fileNode );
                ++m;
            }

            // Don't add dirs that have no files in them
            // These should not be present in our data in the first place. But just in case,...
            if ( fileSet.size() > 0 )
            {
                headersInfoNode.AddChild( pathNode );
            }
            ++n;
        }

        moduleInfoNode.AddChild( headersInfoNode );
        headersInfoNode.DelSubTree();
    }

    // Add sources
    if ( moduleInfo->GetSourceDirs().size() > 0 )
    {
        CORE::CDataNode sourceInfoNode;
        sourceInfoNode.SetName( "Files" );
        sourceInfoNode.SetAttribute( "Type", "Source" );
        sourceInfoNode.SetAttribute( "DirCount", CORE::ToString( moduleInfo->GetSourceDirs().size() ) );
        TStringSetMap::const_iterator n = moduleInfo->GetSourceDirs().begin();
        while ( n != moduleInfo->GetSourceDirs().end() )
        {
            CORE::CDataNode pathNode;
            pathNode.SetName( "Dir" );
            pathNode.SetAttribute( "Path", (*n).first );

            CORE::CDataNode fileNode;
            fileNode.SetName( "File" );

            const TStringSet& fileSet = (*n).second;
            pathNode.SetAttribute( "FileCount", CORE::ToString( fileSet.size() ) );
            TStringSet::const_iterator m = fileSet.begin();
            while ( m != fileSet.end() )
            {
                fileNode.SetAttribute( "Name", (*m) );
                pathNode.AddChild( fileNode );
                ++m;
            }

            // Don't add dirs that have no files in them
            // These should not be present in our data in the first place. But just in case,...
            if ( fileSet.size() > 0 )
            {
                sourceInfoNode.AddChild( pathNode );
            }
            ++n;
        }

        moduleInfoNode.AddChild( sourceInfoNode );
        sourceInfoNode.DelSubTree();
    }

    // Add include paths inherited from dependencies
    if ( moduleInfo->dependencyIncludeDirs.size() > 0 )
    {
        CORE::CDataNode includesInfoNode( "Includes" );
        includesInfoNode.SetAttribute( "Count", CORE::ToString( moduleInfo->dependencyIncludeDirs.size() ) );
        includesInfoNode.SetAttribute( "Source", "Dependency" );
        TStringSet::const_iterator q = moduleInfo->dependencyIncludeDirs.begin();
        while ( q != moduleInfo->dependencyIncludeDirs.end() )
        {
            CORE::CDataNode includeNode;
            includeNode.SetName( "Include" );
            includeNode.SetAttribute( "Path", (*q) );

            includesInfoNode.AddChild( includeNode );
            ++q;
        }

        moduleInfoNode.AddChild( includesInfoNode );
        includesInfoNode.DelSubTree();
    }

    // Add all the regular include dirs for this module
    // These are already represented in the path attribute of the files section
    // but for ease of processing and clarity they are provided again in the includes section
    if ( moduleInfo->GetIncludeDirs().size() > 0 )
    {
       CORE::CDataNode includesInfoNode( "Includes" );
        includesInfoNode.SetAttribute( "Count", CORE::ToString( moduleInfo->GetIncludeDirs().size() ) );
        includesInfoNode.SetAttribute( "Source", "Self" );
        TStringSetMap::const_iterator n = moduleInfo->GetIncludeDirs().begin();
        while ( n != moduleInfo->GetIncludeDirs().end() )
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
                if ( 1 < moduleInfo->GetIncludeDirs().size() )
                {
                    CORE::CString includeDir = "../" + CORE::LastSubDir( moduleEntry->rootDir ) + " ";
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
    }

    // Add all the module dependencies
    if ( moduleInfo->GetNamesOfDependencies().size() > 0 )
    {
        CORE::CDataNode dependenciesNode;
        dependenciesNode.SetName( "Dependencies" );
        dependenciesNode.SetAttribute( "Count", CORE::ToString( moduleInfo->GetNamesOfDependencies().size() ) );
        TStringSet::const_iterator m = moduleInfo->GetNamesOfDependencies().begin();
        while ( m != moduleInfo->GetNamesOfDependencies().end() )
        {
            CORE::CDataNode dependencyNode;
            dependencyNode.SetName( "Dependency" );
            dependencyNode.SetAttribute( "Name", (*m) );
            dependenciesNode.AddChild( dependencyNode );
            ++m;
        }
        moduleInfoNode.AddChild( dependenciesNode );
    }

    // Add all the logical module dependencies
    if ( moduleInfo->GetNamesOfLogicalDependencies().size() > 0 )
    {
        CORE::CDataNode dependenciesNode;
        dependenciesNode.SetName( "LogicalDependencies" );
        dependenciesNode.SetAttribute( "Count", CORE::ToString( moduleInfo->GetNamesOfLogicalDependencies().size() ) );
        TStringSet::const_iterator m = moduleInfo->GetNamesOfLogicalDependencies().begin();
        while ( m != moduleInfo->GetNamesOfLogicalDependencies().end() )
        {
            CORE::CDataNode dependencyNode;
            dependencyNode.SetName( "LogicalDependency" );
            dependencyNode.SetAttribute( "Name", (*m) );
            dependenciesNode.AddChild( dependencyNode );
            ++m;
        }
        moduleInfoNode.AddChild( dependenciesNode );
    }

    // Add all the runtime module dependencies
    if ( moduleInfo->runtimeDependencies.size() > 0 )
    {
        CORE::CDataNode dependenciesNode;
        dependenciesNode.SetName( "RuntimeDependencies" );
        dependenciesNode.SetAttribute( "Count", CORE::ToString( moduleInfo->runtimeDependencies.size() ) );
        TStringSet::const_iterator m = moduleInfo->runtimeDependencies.begin();
        while ( m != moduleInfo->runtimeDependencies.end() )
        {
            CORE::CDataNode dependencyNode;
            dependencyNode.SetName( "RuntimeDependency" );
            dependencyNode.SetAttribute( "Name", (*m) );
            dependenciesNode.AddChild( dependencyNode );
            ++m;
        }
        moduleInfoNode.AddChild( dependenciesNode );
    }

    // Add all the module preprocessor instructions
    if ( moduleInfo->preprocessorSettings.GetDefines().size() > 0 )
    {
        CORE::CDataNode preprocessorNode;
        preprocessorNode.SetName( "Preprocessor" );
        TStringSet::const_iterator m = moduleInfo->preprocessorSettings.GetDefines().begin();
        while ( m != moduleInfo->preprocessorSettings.GetDefines().end() )
        {
            CORE::CDataNode defineNode;
            defineNode.SetName( "Define" );
            defineNode.SetAttribute( "String", (*m) );
            preprocessorNode.AddChild( defineNode );
            ++m;
        }
        moduleInfoNode.AddChild( preprocessorNode );
    }

    CORE::CDataNode linkerNode;
    linkerNode.SetName( "Linker" );
    bool addedLinkedSettings = false;

    // Now Serialize all linker related info
    // First add all the libraries that are linked but not part of the overall project
    if ( moduleInfo->linkerSettings.GetLinkedLibraries().size() > 0 )
    {
        addedLinkedSettings = true;
        TLinkedLibrarySettingsPtrMap::const_iterator m = moduleInfo->linkerSettings.GetLinkedLibraries().begin();
        while ( m != moduleInfo->linkerSettings.GetLinkedLibraries().end() )
        {
            CORE::CDataNode libraryNode;
            libraryNode.SetName( "Dependency" );
            libraryNode.SetAttribute( "Name", (*m).first );

            TModuleType linkedLibType = (*m).second->GetModuleType();
            if ( ( MODULETYPE_UNDEFINED == linkedLibType ) ||
                 ( MODULETYPE_UNKNOWN == linkedLibType )    )
            {
                libraryNode.SetAttribute( "Type", ModuleTypeToString( linkedLibType ) );
            }
            if ( !(*m).second->GetLibraryPath().IsNULLOrEmpty() )
            {
                libraryNode.SetAttribute( "Path", (*m).second->GetLibraryPath() );
            }
            linkerNode.AddChild( libraryNode );
            ++m;
        }

        m = moduleInfo->linkerSettings.GetLinkedLogicalLibraries().begin();
        while ( m != moduleInfo->linkerSettings.GetLinkedLogicalLibraries().end() )
        {
            CORE::CDataNode libraryNode;
            libraryNode.SetName( "LogicalDependency" );
            libraryNode.SetAttribute( "Name", (*m).first );

            TModuleType linkedLibType = (*m).second->GetModuleType();
            if ( ( MODULETYPE_UNDEFINED == linkedLibType ) ||
                 ( MODULETYPE_UNKNOWN == linkedLibType )    )
            {
                libraryNode.SetAttribute( "Type", ModuleTypeToString( linkedLibType ) );
            }
            if ( !(*m).second->GetLibraryPath().IsNULLOrEmpty() )
            {
                libraryNode.SetAttribute( "Path", (*m).second->GetLibraryPath() );
            }
            linkerNode.AddChild( libraryNode );
            ++m;
        }
    }
    if ( !moduleInfo->linkerSettings.GetTargetName().IsNULLOrEmpty() )
    {
        addedLinkedSettings = true;
        linkerNode.SetAttribute( "TargetName", moduleInfo->linkerSettings.GetTargetName() ); 
    }

    if ( !moduleInfo->linkerSettings.GetLibraryPaths().empty() )
    {
        CORE::CString libPaths;
        TStringSet::const_iterator r = moduleInfo->linkerSettings.GetLibraryPaths().begin();
        while ( r != moduleInfo->linkerSettings.GetLibraryPaths().end() )
        {
            if ( libPaths.IsNULLOrEmpty() )
            {
                libPaths = (*r);
            }
            else
            {
                libPaths += ';' + (*r);
            }
            ++r;
        }
        linkerNode.SetAttribute( "LibPaths", libPaths );
        addedLinkedSettings = true;
    }

    if ( addedLinkedSettings )
    {
        moduleInfoNode.AddChild( linkerNode );
    }

    // Add all the info for this module to the overall project
    parentNode.AddChild( moduleInfoNode );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
SerializeModuleInfo( const CModuleInfoEntryPtr& moduleEntry ,
                     CORE::CDataNode& parentNode            )
{GUCEF_TRACE;

    CORE::CDataNode* node = parentNode.AddChild( "ModuleInfoEntry" );
    if ( GUCEF_NULL != node )
    {       
        CORE::CDataNodeSerializableSettings defaultSerializableSettings;
        if ( moduleEntry->Serialize( *node, defaultSerializableSettings ) )
        {
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
SerializeModuleInfo( const CModuleInfoEntryPtr& moduleEntry ,
                     const CORE::CString& outputFilepath    )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec = GetXmlDStoreCodec();
    if ( 0 != codec )
    {
        CORE::CDataNode info;
        if ( SerializeModuleInfo( moduleEntry, info ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SerializeModuleInfo: Successfully generated a data tree with all module information" );

            if ( codec->StoreDataTree( &info, outputFilepath ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SerializeModuleInfo: Successfully wrote all module information to disk file \"" + outputFilepath + "\"" );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SerializeModuleInfo: Failed to store the serialized module information to disk at " + outputFilepath );
                return false;
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SerializeModuleInfo: Failed to serialize the given module information" );
            return false;
        }
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SerializeModuleInfo: Cannot serialize since no codec is registered that can be used for serialization" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
SerializeProjectInfo( const CProjectInfo& projectInfo     ,
                      const CORE::CString& outputFilepath )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec = GetXmlDStoreCodec();
    if ( 0 != codec )
    {
        CORE::CDataNode info;
        CORE::CDataNodeSerializableSettings defaultSettings;
        if ( projectInfo.Serialize( info, defaultSettings ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully generated a data tree with all project information" );

            if ( codec->StoreDataTree( &info, outputFilepath ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully wrote all project information to disk file \"" + outputFilepath + "\"" );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SerializeProjectInfo: Failed to store the serialized project information to disk at " + outputFilepath );
                return false;
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SerializeProjectInfo: Failed to serialize the given project information" );
            return false;
        }
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SerializeProjectInfo: Cannot serialize since no codec is registered that can be used for serialization" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
DeserializeProjectInfo( CProjectInfo& projectInfo            ,
                        const CORE::CString& projectInfoPath )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec = GetXmlDStoreCodec();
    if ( 0 != codec )
    {
        CORE::CDataNode info;
        if ( codec->BuildDataTree( &info, projectInfoPath ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully generated a data tree with all project information" );

            CORE::CDataNodeSerializableSettings defaultSettings;
            if ( projectInfo.Deserialize( info, defaultSettings ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully loaded all project information from disk file \"" + projectInfoPath + "\"" );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeProjectInfo: Failed to load project information from deserialized info loaded from " + projectInfoPath );
                return false;
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeProjectInfo: Failed to deserialize project information from disk at " + projectInfoPath );
            return false;
        }
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeProjectInfo: Cannot deserialize since no codec is registered that can be used for deserialization" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
DeserializeModuleInfo( CModuleInfoPtr& moduleInfo        ,
                       const CORE::CDataNode& parentNode )
{GUCEF_TRACE;

    const CORE::CDataNode* moduleInfoNode = parentNode.Find( "Module" );
    if ( GUCEF_NULL == moduleInfoNode ) 
        return false;

    // Check to see if a name was defined
    CORE::CString tmpStr = moduleInfoNode->GetAttributeValue( "Name" );
    if ( !tmpStr.IsNULLOrEmpty() )
    {
        moduleInfo->name = tmpStr;
    }

    // Find the overall module properties
    tmpStr = moduleInfoNode->GetAttributeValue( "BuildOrder", "-1" );
    moduleInfo->buildOrder = CORE::StringToInt64( tmpStr );
    //tmpStr = moduleInfoNode->GetAttributeValue( "BuildChain", "-1" );
    //moduleInfo->buildChain = CORE::StringToInt32( tmpStr );
    //tmpStr = moduleInfoNode->GetAttributeValue( "BuildChainDeps" );
    //if ( !tmpStr.IsNULLOrEmpty() )
    //{
    //    TStringVector tmpStrVec = tmpStr.ParseElements( ';', false );
    //    if ( tmpStrVec.empty() )
    //    {
    //        for ( UInt32 i=0; i<tmpStrVec.size(); ++i )
    //        {
    //            Int32 chainId = CORE::StringToInt32( tmpStrVec[ i ] );
    //            moduleInfo->buildChainDependencies.insert( chainId );
    //        }
    //    }
    //}
    moduleInfo->tags = StringVectorToStringSet( moduleInfoNode->GetAttributeValue( "Tags" ).AsString().ParseElements( ';', false ) );
    moduleInfo->moduleType = StringToModuleType( moduleInfoNode->GetAttributeValue( "Type" ) );
    
    if ( moduleInfoNode->IsAttribute( "ConsiderSubDirs" ) )
    {
        moduleInfo->considerSubDirs = moduleInfoNode->GetAttributeValue( "ConsiderSubDirs", moduleInfo->considerSubDirs, false ).AsBool( moduleInfo->considerSubDirs, true );
        moduleInfo->hasConsiderSubDirs = true;
    }
    else
        moduleInfo->hasConsiderSubDirs = false;

    if ( moduleInfoNode->IsAttribute( "IgnoreModule" ) )
    {
        moduleInfo->ignoreModule = moduleInfoNode->GetAttributeValue( "IgnoreModule", moduleInfo->ignoreModule, false ).AsBool( moduleInfo->ignoreModule, true );
        moduleInfo->hasIgnoreModule = true;
    }
    else
        moduleInfo->hasIgnoreModule = false;

    // Find any/all files for which are part of this module
    CORE::CDataNode::TConstDataNodeSet fileNodes = moduleInfoNode->FindChildrenOfType( "Files" );
    CORE::CDataNode::TConstDataNodeSet::iterator i = fileNodes.begin();
    while ( i != fileNodes.end() )
    {
        const CORE::CDataNode* filesDirsNode = (*i);
        CORE::CString filesType = filesDirsNode->GetAttributeValue( "Type" );

        CORE::CDataNode::TConstDataNodeSet dirs = filesDirsNode->FindChildrenOfType( "Dir" );
        CORE::CDataNode::TConstDataNodeSet::iterator n = dirs.begin();
        while ( n != dirs.end() )
        {
            const CORE::CDataNode* dirsNode = (*n);
            CORE::CString path = dirsNode->GetAttributeValue( "Path" );

            CORE::CDataNode::TConstDataNodeSet files = dirsNode->FindChildrenOfType( "File" );
            CORE::CDataNode::TConstDataNodeSet::iterator m = files.begin();
            while ( m != files.end() )
            {
                CORE::CString filename = (*m)->GetAttributeValue( "Name" );

                if ( filesType == "Headers" )
                {
                    // We have a list of header files
                    moduleInfo->AddIncludeFile( path, filename );
                }
                else
                if ( filesType == "Source" )
                {
                    // We have a list of source files
                    moduleInfo->AddSourceFile( path, filename );
                }
                ++m;
            }
            ++n;
        }
        ++i;
    }

    // Find dependency includes
    CORE::CDataNode::TConstDataNodeSet includesNodes = moduleInfoNode->FindChildrenOfType( "Includes" );
    i = includesNodes.begin();
    while ( i != includesNodes.end() )
    {
        const CORE::CDataNode* includesNode = (*i);
        CORE::CString source = includesNode->GetAttributeValue( "Source" ).AsString().Lowercase();
        if ( source == "dependency" )
        {
            CORE::CDataNode::TConstDataNodeSet includes = includesNode->FindChildrenOfType( "Include" );
            CORE::CDataNode::TConstDataNodeSet::iterator n = includes.begin();
            while ( n != includes.end() )
            {
                const CORE::CDataNode* includeNode = (*n);
                CORE::CString path = includeNode->GetAttributeValue( "Path" );

                moduleInfo->dependencyIncludeDirs.insert( path );
                ++n;
            }
        }
        else
        if ( source == "self" )
        {
            // If the "self" includes came from actual include files then they would be automatically
            // added when the files are added. However,... some importers might place additional includes
            // under self which would not be auto-detected via include files. As such we have to add them 
            // here as well to make sure. 
            // Importers might not have any way of knowing whether a path was added due to dependency needs

            CORE::CDataNode::TConstDataNodeSet includes = includesNode->FindChildrenOfType( "Include" );
            CORE::CDataNode::TConstDataNodeSet::iterator n = includes.begin();
            while ( n != includes.end() )
            {
                const CORE::CDataNode* includeNode = (*n);
                CORE::CString path = includeNode->GetAttributeValue( "Path" );

                if ( moduleInfo->GetIncludeFiles( path ).empty() )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeModuleInfo: Adding include dir of source \"self\" which does not currently have a reference to include files: " + path );
                    moduleInfo->AddIncludeDir( path );
                }
                ++n;
            }
        }
        ++i;
    }

    // Find all the module dependencies
    const CORE::CDataNode* dependenciesNode = moduleInfoNode->Find( "Dependencies" );
    if ( NULL != dependenciesNode )
    {
        CORE::CDataNode::TConstDataNodeSet dependencies = dependenciesNode->FindChildrenOfType( "Dependency" );
        i = dependencies.begin();
        while ( i != dependencies.end() )
        {
            const CORE::CDataNode* dependencyNode = (*i);
            CORE::CString dependencyName = dependencyNode->GetAttributeValue( "Name" );
            if ( !dependencyName.IsNULLOrEmpty() )
            {
                moduleInfo->AddNameOfDependency( dependencyName );
            }
            ++i;
        }
    }

    // Get all the runtime module dependencies if any
    const CORE::CDataNode* rdependenciesNode = moduleInfoNode->Find( "RuntimeDependencies" );
    if ( NULL != rdependenciesNode )
    {
        CORE::CDataNode::TConstDataNodeSet dependencies = rdependenciesNode->FindChildrenOfType( "RuntimeDependency" );
        i = dependencies.begin();
        while ( i != dependencies.end() )
        {
            const CORE::CDataNode* dependencyNode = (*i);
            CORE::CString dependencyName = dependencyNode->GetAttributeValue( "Name" );
            if ( !dependencyName.IsNULLOrEmpty() )
            {
                moduleInfo->runtimeDependencies.insert( dependencyName );
            }
            ++i;
        }
    }

    CORE::CDataNodeSerializableSettings serializableSettings;

    // Go through all linker related settings, if any exist    
    moduleInfo->linkerSettings.Deserialize( *moduleInfoNode, serializableSettings );

    // Go through all preprocessor related settings, if any exist
    moduleInfo->preprocessorSettings.Deserialize( *moduleInfoNode, serializableSettings );

    // Go through all compiler related settings, if any exist
    moduleInfo->compilerSettings.Deserialize( *moduleInfoNode, serializableSettings );

    return true;
}

/*-------------------------------------------------------------------------*/

const CModuleInfoPtr
FindModuleByName( const TModuleInfoEntryPairVector& mergeLinks ,
                  const CORE::CString& moduleName              )
{GUCEF_TRACE;

    TModuleInfoEntryPairVector::const_iterator i = mergeLinks.begin();
    while ( i != mergeLinks.end() )
    {
        const CModuleInfo& moduleInfo = (*(*i).second);
        if ( moduleInfo.name == moduleName )
        {
            return (*i).second;
        }
        ++i;
    }
    return CModuleInfoPtr();
}

/*-------------------------------------------------------------------------*/

bool
DeserializeModuleInfo( const CProjectInfo& projectInfo      ,
                       CModuleInfoEntryPtr& moduleInfoEntry ,
                       const CORE::CDataNode& parentNode    )
{GUCEF_TRACE;

    const CORE::CDataNode* moduleEntryNode = parentNode.Find( "ModuleInfoEntry" );
    if ( GUCEF_NULL == moduleEntryNode ) 
        return false;

    CORE::CDataNodeSerializableSettings defaultSerializableSettings;
    
    // @TODO: temp hack to get ProjectInfo into the deserializer which is needed for normalization
    defaultSerializableSettings.SetUserData( const_cast< CProjectInfo* >( &projectInfo ) );

    return moduleInfoEntry->Deserialize( *moduleEntryNode, defaultSerializableSettings );
}

/*-------------------------------------------------------------------------*/

void
MergeModuleTypeMap( TModuleTypeMap& baseMap           ,
                    const TModuleTypeMap& incomingMap )
{GUCEF_TRACE;

    TModuleTypeMap::const_iterator i = incomingMap.begin();
    while ( i != incomingMap.end() )
    {
        TModuleTypeMap::iterator n = baseMap.find( (*i).first );
        if ( n != baseMap.end() )
        {
            TModuleType moduleType = (*i).second;
            if ( ( MODULETYPE_UNDEFINED != moduleType ) &&
                 ( MODULETYPE_UNKNOWN != moduleType )    )
            {
                baseMap[ (*i).first ] = moduleType;
            }
        }
        else
        {
            baseMap[ (*i).first ] = (*i).second;
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
MergeModuleMetaData( const CModuleMetaData& priorityA ,
                     const CModuleMetaData& priorityB ,
                     CModuleMetaData& target          )
{GUCEF_TRACE;

    MergeStringSet( target.authors, priorityA.authors, false );
    MergeStringSet( target.authors, priorityB.authors, false );
    MergeStringSet( target.maintainers, priorityA.maintainers, false );
    MergeStringSet( target.maintainers, priorityB.maintainers, false );

    target.descriptionDetails = priorityA.descriptionDetails.IsNULLOrEmpty() ? priorityB.descriptionDetails : priorityA.descriptionDetails;
    target.descriptionHeadline = priorityA.descriptionHeadline.IsNULLOrEmpty() ? priorityB.descriptionHeadline : priorityA.descriptionHeadline;
    target.license = priorityA.license.IsNULLOrEmpty() ? priorityB.license : priorityA.license;
    target.lastEditBy = priorityA.lastEditBy.IsNULLOrEmpty() ? priorityB.lastEditBy : priorityA.lastEditBy;
    target.SetSemVer( priorityA.HasSemVer() ? priorityB.GetSemVer() : priorityA.GetSemVer() );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
MergeModuleInfo( const CModuleInfoEntryPtr& moduleInfoEntry ,
                 const CORE::CString& targetPlatform        ,
                 CModuleInfoPtr& mergedModuleInfo           )
{GUCEF_TRACE;

    if ( !mergedModuleInfo.IsNULL() )
        mergedModuleInfo->Clear();

    const CModuleInfoPtr allPlatformsInfo = moduleInfoEntry->FindModuleInfoForPlatform( AllPlatforms );
    const CModuleInfoPtr targetPlatformInfo = moduleInfoEntry->FindModuleInfoForPlatform( targetPlatform );
    if ( ( !allPlatformsInfo.IsNULL() ) || ( !targetPlatformInfo.IsNULL() ) )
    {
        // Check if we have both
        if ( !allPlatformsInfo.IsNULL() && !targetPlatformInfo.IsNULL() )
        {
            // Check if at least one of them has a build order set
            if ( allPlatformsInfo->buildOrder > -1 || targetPlatformInfo->buildOrder > -1 )
            {
                // Check if at least one of them has a module type set
                if ( allPlatformsInfo->moduleType != MODULETYPE_UNDEFINED || targetPlatformInfo->moduleType != MODULETYPE_UNDEFINED )
                {
                    // Use the 'all' platform as a base to work from
                    mergedModuleInfo = CModuleInfo::CreateSharedObjWithParam( *allPlatformsInfo );

                    // Now merge in the platform specific info
                    mergedModuleInfo->Merge( targetPlatformInfo );

                    //MergeModuleMetaData( moduleInfoEntry, targetPlatform, mergedModuleInfo );
                    return true;
                }
            }

            // Even though some info is specified for both platforms neither counts as independent
            // definition for which a build order must be set
            return false;
        }
        //else
        //if ( !allPlatformsInfo.IsNULL() && allPlatformsInfo->buildOrder > -1 )
        //{
        //    // We only have the 'all' platform which is fine, we will just use that
        //    mergedModuleInfo = CModuleInfo::CreateSharedObjWithParam( *allPlatformsInfo );
        //    
        //    MergeModuleMetaData( moduleInfoEntry, targetPlatform, mergedModuleInfo );
        //    return true;
        //}
        //else
        //if ( !targetPlatformInfo.IsNULL() && targetPlatformInfo->buildOrder > -1 )
        //{
        //    // We only have the target platform which is fine, we will just use that
        //    // this module apparently is not available for all platforms even in altered form
        //    mergedModuleInfo = CModuleInfo::CreateSharedObjWithParam( *targetPlatformInfo );
        //    
        //    MergeModuleMetaData( moduleInfoEntry, targetPlatform, mergedModuleInfo );
        //    return true;
        //}
    }

    // This module should not be used since it doesn't have platform specific info
    // nor info which applies to all platforms.
    return false;
}

/*-------------------------------------------------------------------------*/

bool
MergeModuleInfoEntries( const TModuleInfoEntryPtrVector& moduleInfoEntriesToMergeIn ,
                        TModuleInfoEntryPtrVector& moduleInfoEntries                )
{GUCEF_TRACE;

    bool totalSuccess = true;
    TModuleInfoEntryPtrVector::const_iterator i = moduleInfoEntriesToMergeIn.begin();
    while ( i != moduleInfoEntriesToMergeIn.end() )
    {
        const CModuleInfoEntryPtr& entryToMergeIn = (*i);
        CString consensusName = entryToMergeIn->GetConsensusName();

        bool foundMatch = false;
        TModuleInfoEntryPtrVector::iterator e = moduleInfoEntries.begin();
        while ( e != moduleInfoEntries.end() )
        {
            CModuleInfoEntryPtr& existingEntry = (*e);
            CString existingEntryConsensusName = existingEntry->GetConsensusName();

            if ( existingEntryConsensusName == consensusName )
            {
                // We already have such a module entry, we need to merge
                foundMatch = true;
                totalSuccess = existingEntry->Merge( entryToMergeIn ) && totalSuccess;
            }

            ++e;
        }

        if ( !foundMatch )
        {
            // No such match so the 'merge' is a straightforward insert
            moduleInfoEntries.push_back( entryToMergeIn );
        }
        ++i;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
MergeAllModuleInfoForPlatform( const TModuleInfoEntryPtrSet& allInfo  ,
                               const CORE::CString& platform          ,
                               TModuleInfoPtrVector& allMergedInfo    ,
                               TModuleInfoEntryPairVector& mergeLinks )
{GUCEF_TRACE;

    allMergedInfo.clear();

    TModuleInfoEntryPtrVector indexMap;
    TModuleInfoEntryPtrSet::const_iterator i = allInfo.begin();
    while ( i != allInfo.end() )
    {
        // For each module we create a merged platform specific module
        // description which is easy to process if you only care about that platform
        CModuleInfoPtr mergedInfo;
        if ( MergeModuleInfo( (*i), platform, mergedInfo ) )
        {
            // Store the merged info
            allMergedInfo.push_back( mergedInfo );

            // Store a link between the merged info and the original info
            // at the same index as the merged info
            indexMap.push_back( (*i) );
        }
        ++i;
    }

    // Now that alterations to the storage are completed we can map the index
    // of each entry to the actual data storage
    for ( CORE::UInt32 m=0; m<allMergedInfo.size(); ++m )
    {
        TModuleInfoEntryPair mergeLink( indexMap[ m ], (allMergedInfo[ m ]) );
        mergeLinks.push_back( mergeLink );
    }
    return true;
}

/*---------------------------------------------------------------------------*/

bool
MergeAllModuleInfoForPlatform( const TModuleInfoEntryPtrVector& allInfo ,
                               const CORE::CString& platform            ,
                               TModuleInfoPtrVector& allMergedInfo      ,
                               TModuleInfoEntryPairVector& mergeLinks   )
{GUCEF_TRACE;

    TModuleInfoEntryPtrSet infoPtrs;
    TModuleInfoEntryPtrVector::const_iterator i = allInfo.begin();
    while ( i != allInfo.end() )
    {
        infoPtrs.insert( (*i) );
        ++i;
    }

    return MergeAllModuleInfoForPlatform( infoPtrs      ,
                                          platform      ,
                                          allMergedInfo ,
                                          mergeLinks    );
}

/*---------------------------------------------------------------------------*/

bool
MergePlatformDefinition( TPlatformDefinition& targetPlatform          ,
                         const TPlatformDefinition& platformToMergeIn ,
                         bool caseSensitive                           )
{GUCEF_TRACE;

    MergeStringSet( targetPlatform.aliases, platformToMergeIn.aliases, caseSensitive );
    MergeStringSet( targetPlatform.platformDirs, platformToMergeIn.platformDirs, caseSensitive );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
MergePlatformDefinitionMap( TPlatformDefinitionMap& targetPlatforms          ,
                            const TPlatformDefinitionMap& platformsToMergeIn ,
                            bool caseSensitive                               )
{GUCEF_TRACE;

    bool totalSuccess = true;
    TPlatformDefinitionMap::const_iterator i = platformsToMergeIn.begin();
    while ( i != platformsToMergeIn.end() )
    {
        TPlatformDefinitionMap::iterator n = targetPlatforms.find( (*i).first );
        if ( n != targetPlatforms.end() )
        {
            // We already have a platform definition with this name, merge them
            totalSuccess = MergePlatformDefinition( (*n).second, (*i).second, caseSensitive ) && totalSuccess;
        }
        else
        {
            // We do not have instructions for this dir yet, just add them
            targetPlatforms.insert( *i );
        }
        ++i;
    }
    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
MergeDirProcessingInstructions( TDirProcessingInstructions& mergedInstructions    ,
                                const TDirProcessingInstructions& newInstructions ,
                                bool caseSensitive                                )
{GUCEF_TRACE;

    bool totalSuccess = true;

    MergeStringVectorMap( mergedInstructions.dirExcludeList, newInstructions.dirExcludeList, caseSensitive );
    MergeStringVectorMap( mergedInstructions.dirIncludeList, newInstructions.dirIncludeList, caseSensitive );
    MergeStringVectorMap( mergedInstructions.fileExcludeList, newInstructions.fileExcludeList, caseSensitive );
    MergeStringVectorMap( mergedInstructions.fileIncludeList, newInstructions.fileIncludeList, caseSensitive );
    MergeStringVectorMap( mergedInstructions.fileIncludeList, newInstructions.fileIncludeList, caseSensitive );
    totalSuccess = MergePlatformDefinitionMap( mergedInstructions.platforms, newInstructions.platforms, caseSensitive ) && totalSuccess;

    // @TODO: We need some kind of proper tree merge for the opaque processing instructions
    // For now we just copy the new instructions into the merged instructions as this is unlikely to be actually used
    mergedInstructions.processingInstructions.CopySubTree( newInstructions.processingInstructions );

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
MergeDirProcessingInstructionsMap( TDirProcessingInstructionsMap& mergedInstructions    ,
                                   const TDirProcessingInstructionsMap& newInstructions ,
                                   bool caseSensitive                                   )
{GUCEF_TRACE;

    bool totalSuccess = true;

    TDirProcessingInstructionsMap::const_iterator i = newInstructions.begin();
    while ( i != newInstructions.end() )
    {
        TDirProcessingInstructionsMap::iterator n = mergedInstructions.find( (*i).first );
        if ( n != mergedInstructions.end() )
        {
            // We already have instructions for this dir, merge them
            totalSuccess = MergeDirProcessingInstructions( (*n).second, (*i).second, caseSensitive ) && totalSuccess;
        }
        else
        {
            // We do not have instructions for this dir yet, just add them
            mergedInstructions.insert( *i );
        }
        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

const CORE::CString*
GetModuleName( const TProjectTargetInfoMap& targetPlatforms ,
               const CORE::CString& targetPlatform          ,
               CModuleInfoPtr* outModuleInfo                )
{GUCEF_TRACE;


    TProjectTargetInfoMap::const_iterator i = targetPlatforms.find( targetPlatform );
    if ( i != targetPlatforms.end() )
    {
        const CModuleInfoEntryPtr mainModule = (*i).second.mainModule;
        if ( !mainModule.IsNULL() )
        {
            return mainModule->GetModuleName( targetPlatform, outModuleInfo );
        }
    }

    // If no target is specified for a specific platform then there might still be a
    // default for all platforms
    if ( targetPlatform != AllPlatforms )
    {
        i = targetPlatforms.find( AllPlatforms );
        if ( i != targetPlatforms.end() )
        {
            const CModuleInfoEntryPtr mainModule = (*i).second.mainModule;
            if ( !mainModule.IsNULL() )
            {
                return mainModule->GetModuleName( AllPlatforms, outModuleInfo );
            }
        }
    }

    return GUCEF_NULL;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GetConsensusTargetName( const TProjectTargetInfoMap& targetPlatforms ,
                        const CORE::CString& targetPlatform          )
{GUCEF_TRACE;

    TProjectTargetInfoMap::const_iterator n = targetPlatforms.find( targetPlatform );
    if ( n != targetPlatforms.end() )
    {
        // A target was specified for this platform which makes our job easy
        const TProjectTargetInfo& target = (*n).second;
        if ( GUCEF_NULL != target.mainModule )
        {
            return GetModuleTargetName( target.mainModule, targetPlatform, true );
        }
    }

    // If no name is specified for all platforms then we will have to
    // determine the best name to use. We do this by getting the name
    // for all platforms and counting how often each is used. The most used
    // name is considered the general consensus name. If the same count applies
    // to multiple we will try to use a popular platform to improve our 'guess'

    typedef std::map< CORE::CString, CORE::UInt32 > TStringCountMap;

    TStringCountMap countMap;
    n = targetPlatforms.begin();
    while ( n != targetPlatforms.end() )
    {
        const TProjectTargetInfo& target = (*n).second;
        if ( GUCEF_NULL != target.mainModule )
        {
            CORE::CString targetName = GetModuleTargetName( target.mainModule, targetPlatform, true );
            TStringCountMap::iterator m = countMap.find( targetName );
            if ( m != countMap.end() )
            {
                ++((*m).second);
            }
            else
            {
                countMap[ targetName ] = 1;
            }
        }
        ++n;
    }

    // Not all target origins cause a main module to be defined.
    // As such its perfectly possible be unable to define a consensus target name because the concept does not apply
    // to the collection of targets due to the origins of the collection
    if ( countMap.empty() )
        return CORE::CString();

    // Now that we have the popularity count of each name get the highest count
    CORE::UInt32 highestCount = 0;
    TStringCountMap::iterator i = countMap.begin();
    while ( i != countMap.end() )
    {
        if ( highestCount < (*i).second )
        {
            highestCount = (*i).second;
        }
        ++i;
    }

    // Make the list of most popular names
    TStringSet topNames;
    i = countMap.begin();
    while ( i != countMap.end() )
    {
        if ( highestCount == (*i).second )
        {
            topNames.insert( (*i).first );
        }
        ++i;
    }

    // If we have multiple use a popular platform if
    // possible, otherwise just grab one

    //@TODO: popular platform check

    CString consensusName = (*topNames.begin());
    return consensusName;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GetConsensusTargetName( const TProjectTargetInfoMap& targetPlatforms )
{GUCEF_TRACE;

    // an "all platforms" name always counts as the general consensus name
    return GetConsensusTargetName( targetPlatforms, AllPlatforms );
}

/*---------------------------------------------------------------------------*/

CORE::CString
GetModuleTargetName( const CModuleInfoEntryPtr& moduleInfoEntry ,
                     const CORE::CString& targetPlatform        ,
                     bool useModuleNameIfNoTargetName           )
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = moduleInfoEntry->FindModuleInfoForPlatform( targetPlatform );
    if ( !moduleInfo.IsNULL() )
    {
        if ( !moduleInfo->linkerSettings.GetTargetName().IsNULLOrEmpty() )
        {
            return moduleInfo->linkerSettings.GetTargetName();
        }
    }
    if ( targetPlatform != AllPlatforms && !targetPlatform.IsNULLOrEmpty() )
    {
        moduleInfo = moduleInfoEntry->FindModuleInfoForPlatform( AllPlatforms );
        if ( !moduleInfo.IsNULL() )
        {
            if ( !moduleInfo->linkerSettings.GetTargetName().IsNULLOrEmpty() )
            {
                return moduleInfo->linkerSettings.GetTargetName();
            }
        }
    }

    // If we got here no target name was defined so the only option left is the 
    // module name. For most projects this is the same thing.
    if ( useModuleNameIfNoTargetName )
    {
        return moduleInfoEntry->GetModuleNameAlways( targetPlatform );
    }

    return CORE::CString(); 
}

/*---------------------------------------------------------------------------*/

TModuleType
GetModuleType( const CModuleInfoEntryPtr& moduleInfoEntry ,
               const CORE::CString& targetPlatform        )
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = moduleInfoEntry->FindModuleInfoForPlatform( targetPlatform );
    if ( !moduleInfo.IsNULL() )
    {
        if ( MODULETYPE_UNDEFINED != moduleInfo->moduleType )
        {
            return moduleInfo->moduleType;
        }
    }
    if ( targetPlatform != AllPlatforms && !targetPlatform.IsNULLOrEmpty() )
    {
        moduleInfo = moduleInfoEntry->FindModuleInfoForPlatform( AllPlatforms );
        if ( !moduleInfo.IsNULL() )
        {
            return moduleInfo->moduleType;
        }
    }

    // Since there is no specific info for the given platform and no AllPlatform info
    // we will see if we can derive from another if there is consensus
    std::set< TModuleType > typeSet;
    TModuleInfoPtrMap::const_iterator i = moduleInfoEntry->GetModulesPerPlatform().begin();
    while ( i != moduleInfoEntry->GetModulesPerPlatform().end() )
    {
        const CModuleInfoPtr& platformModuleInfo = (*i).second;
        if ( MODULETYPE_UNDEFINED != platformModuleInfo->moduleType )
            typeSet.insert( platformModuleInfo->moduleType );
        ++i;
    }
    if ( 1 == typeSet.size() )
    {
        return *typeSet.begin();
    }

    return MODULETYPE_UNDEFINED;
}

/*---------------------------------------------------------------------------*/

void
GetModuleInfoWithUniqueModulesTypes( const CModuleInfoEntryPtr& moduleInfoEntry ,
                                     TModuleInfoPtrMap& moduleMap               )
{GUCEF_TRACE;

    // First try and get a 'AllPlatforms' definition which makes all the difference
    const CModuleInfoPtr moduleInfo = moduleInfoEntry->FindModuleInfoForPlatform( AllPlatforms );
    if ( NULL != moduleInfo && ( MODULETYPE_UNDEFINED != moduleInfo->moduleType ) )
    {
        // Since a 'AllPlatforms' definition is available we have a baseline to compare against
        // We will only add platform specific entries if they differ from our baseline
        TModuleInfoPtrMap::const_iterator i = moduleInfoEntry->GetModulesPerPlatform().begin();
        while ( i != moduleInfoEntry->GetModulesPerPlatform().end() )
        {
            const CORE::CString& platformName = (*i).first;
            const CModuleInfoPtr& platformModuleInfo = (*i).second;

            if ( ( MODULETYPE_UNDEFINED != platformModuleInfo->moduleType )  &&
                 ( moduleInfo->moduleType != platformModuleInfo->moduleType ) )
            {
                moduleMap[ platformName ] = ((*i).second);
            }
            ++i;
        }
        moduleMap[ AllPlatforms ] = moduleInfo;
    }
    else
    {
        // If we get here: We cannot filter in this case,.. just add all platforms
        TModuleInfoPtrMap::const_iterator i = moduleInfoEntry->GetModulesPerPlatform().begin();
        while ( i != moduleInfoEntry->GetModulesPerPlatform().end() )
        {
            const CORE::CString& platformName = (*i).first;
            const CModuleInfoPtr& platformModuleInfo = (*i).second;

            if ( MODULETYPE_UNDEFINED != platformModuleInfo->moduleType )
            {
                moduleMap[ platformName ] = ((*i).second);
            }
            ++i;
        }
    }
}

/*---------------------------------------------------------------------------*/

void
GetModuleInfoWithUniqueModuleNames( const CModuleInfoEntryPtr& moduleInfoEntry ,
                                    TModuleInfoPtrMap& moduleMap               )
{GUCEF_TRACE;

    // First try and get a 'AllPlatforms' definition which makes all the difference
    const CModuleInfoPtr moduleInfo = moduleInfoEntry->FindModuleInfoForPlatform( AllPlatforms );
    if ( NULL != moduleInfo && !moduleInfo->name.IsNULLOrEmpty() )
    {
        // Since a 'AllPlatforms' definition is available we have a baseline to compare against
        // We will only add platform specific entries if they differ from our baseline
        TModuleInfoPtrMap::const_iterator i = moduleInfoEntry->GetModulesPerPlatform().begin();
        while ( i != moduleInfoEntry->GetModulesPerPlatform().end() )
        {
            const CORE::CString& platformName = (*i).first;
            const CModuleInfoPtr& platformModuleInfo = (*i).second;

            if ( ( !platformModuleInfo->name.IsNULLOrEmpty() )   &&
                 ( moduleInfo->name != platformModuleInfo->name ) )
            {
                moduleMap[ platformName ] = ((*i).second);
            }
            ++i;
        }
        moduleMap[ AllPlatforms ] = moduleInfo;
    }
    else
    {
        // If we get here: We cannot filter in this case,.. just add all platforms
        TModuleInfoPtrMap::const_iterator i = moduleInfoEntry->GetModulesPerPlatform().begin();
        while ( i != moduleInfoEntry->GetModulesPerPlatform().end() )
        {
            const CORE::CString& platformName = (*i).first;
            const CModuleInfoPtr& platformModuleInfo = (*i).second;

            if ( MODULETYPE_UNDEFINED != platformModuleInfo->moduleType )
            {
                moduleMap[ platformName ] = ((*i).second);
            }
            ++i;
        }
    }
}

/*---------------------------------------------------------------------------*/

CORE::CString
LocalizeDirSepCharForPlatform( const CORE::CString& path     ,
                               const CORE::CString& platform )
{GUCEF_TRACE;

    if ( "win32" == platform || "win64" == platform )
    {
        return path.ReplaceChar( '/', '\\' );
    }
    if ( "linux32" == platform || "linux64" == platform || "android32" == platform || "android64" == platform  )
    {
        return path.ReplaceChar( '\\', '/' );
    }

    // Not a platform for which we perform a hardcoded conversion
    // Let's just pick '/' as the default standard
    return path.ReplaceChar( '\\', '/' );
}

/*---------------------------------------------------------------------------*/

void
GetAllModuleInfoPaths( const CModuleInfoEntryPtr& moduleInfoEntry ,
                       const CORE::CString& platform              ,
                       CORE::CString::StringSet& allPaths         ,
                       bool includeModuleRootPath                 ,
                       bool includeDepencencyIncludePaths         )
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator i = moduleInfoEntry->GetModulesPerPlatform().find( platform );
    if ( i != moduleInfoEntry->GetModulesPerPlatform().end() )
    {
        if ( includeModuleRootPath )
        {
            allPaths.insert( moduleInfoEntry->rootDir );
        }
        
        const TStringSetMap& includeDirs = (*i).second->GetIncludeDirs(); 
        TStringSetMap::const_iterator n = includeDirs.begin();
        while ( n != includeDirs.end() )
        {
            const CORE::CString& includeDir = (*n).first;
            if ( includeModuleRootPath )
            {
                CORE::CString path = CORE::CombinePath( moduleInfoEntry->rootDir, includeDir );
                path = CORE::RelativePath( path, true );
                path = LocalizeDirSepCharForPlatform( path, platform );
                allPaths.insert( path );
            }
            else
            {
                CORE::CString path = CORE::RelativePath( includeDir, true );
                path = LocalizeDirSepCharForPlatform( path, platform );
                allPaths.insert( path );
            }
            ++n;
        }        

        const TStringSetMap& sourceDirs = (*i).second->GetSourceDirs(); 
        n = sourceDirs.begin();
        while ( n != sourceDirs.end() )
        {
            const CORE::CString& sourceDir = (*n).first;

            if ( includeModuleRootPath )
            {
                CORE::CString path = CORE::CombinePath( moduleInfoEntry->rootDir, sourceDir );
                path = CORE::RelativePath( path, true );
                path = LocalizeDirSepCharForPlatform( path, platform );
                allPaths.insert( path );
            }
            else
            {
                CORE::CString path = CORE::RelativePath( sourceDir, true );
                path = LocalizeDirSepCharForPlatform( path, platform );
                allPaths.insert( path );
            }
            ++n;
        } 

        if ( includeDepencencyIncludePaths )
        {
            const TStringSet& dependencyIncludeDirs = (*i).second->dependencyIncludeDirs;
            TStringSet::const_iterator m = dependencyIncludeDirs.begin();
            while ( m != dependencyIncludeDirs.end() )
            {
                if ( includeModuleRootPath )
                {
                    CORE::CString path = CORE::CombinePath( moduleInfoEntry->rootDir, (*m) );
                    path = CORE::RelativePath( path, true );
                    path = LocalizeDirSepCharForPlatform( path, platform );
                    allPaths.insert( path );
                }
                else
                {
                    CORE::CString path = CORE::RelativePath( (*m), true );
                    path = LocalizeDirSepCharForPlatform( path, platform );
                    allPaths.insert( path );
                }
                ++m;
            }
        }

        ++i;
    }

    // We handled all the platform specific stuff, now also cover everything that applies to any platform
    if ( platform != AllPlatforms )
    {
        GetAllModuleInfoPaths( moduleInfoEntry, AllPlatforms, allPaths, includeModuleRootPath, includeDepencencyIncludePaths );
    }
}

/*---------------------------------------------------------------------------*/

void
GetAllModuleInfoFilePaths( const CModuleInfoEntryPtr& moduleInfoEntry ,
                           const CORE::CString& platform              ,
                           CORE::CString::StringSet& allPaths         ,
                           bool includeModuleRootPath                 )
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator i = moduleInfoEntry->GetModulesPerPlatform().find( platform );
    if ( i != moduleInfoEntry->GetModulesPerPlatform().end() )
    {
        const TStringSetMap& includeDirs = (*i).second->GetIncludeDirs(); 
        TStringSetMap::const_iterator n = includeDirs.begin();
        while ( n != includeDirs.end() )
        {
            const CORE::CString& includeDir = (*n).first;
            const TStringSet& includeDirContent = (*n).second;

            TStringSet::const_iterator m = includeDirContent.begin();
            while ( m != includeDirContent.end() )
            {
                CORE::CString path = CORE::CombinePath( includeDir, (*m) );
                if ( includeModuleRootPath )
                {
                    path = CORE::CombinePath( moduleInfoEntry->rootDir, path );
                }

                path = CORE::RelativePath( path, true );
                path = LocalizeDirSepCharForPlatform( path, platform );
                allPaths.insert( path );
                ++m;
            }
            ++n;
        }        

        const TStringSetMap& sourceDirs = (*i).second->GetSourceDirs(); 
        n = sourceDirs.begin();
        while ( n != sourceDirs.end() )
        {
            const CORE::CString& sourceDir = (*n).first;
            const TStringSet& sourceDirContent = (*n).second;

            TStringSet::const_iterator m = sourceDirContent.begin();
            while ( m != sourceDirContent.end() )
            {
                CORE::CString path = CORE::CombinePath( sourceDir, (*m) );
                if ( includeModuleRootPath )
                {
                    path = CORE::CombinePath( moduleInfoEntry->rootDir, path );
                }

                path = CORE::RelativePath( path, true );
                path = LocalizeDirSepCharForPlatform( path, platform );
                allPaths.insert( path );
                ++m;
            }
            ++n;
        } 

        ++i;
    }

    // We handled all the platform specific stuff, now also cover everything that applies to any platform
    if ( platform != AllPlatforms )
    {
        GetAllModuleInfoFilePaths( moduleInfoEntry, AllPlatforms, allPaths, includeModuleRootPath );
    }
}

/*---------------------------------------------------------------------------*/

CORE::CString
GetShortestRelativePathFromAbsPathToProjectRoot( const CProjectInfo& projectInfo ,
                                                 const CORE::CString& absPath    )
{GUCEF_TRACE;

    typedef std::map< CORE::UInt32, CORE::CString > TUInt32ToStringMap;

    TUInt32ToStringMap lengthMap;

    TStringVector::const_iterator i = projectInfo.rootDirs.begin();
    while ( i != projectInfo.rootDirs.end() )
    {
        CORE::CString relPath = CORE::GetRelativePathToOtherPathRoot( (*i), absPath ); 
        lengthMap[ relPath.Length() ] = relPath;
        ++i;
    }

    if ( !lengthMap.empty() )
    {
        return (*lengthMap.begin()).second;
    }
    return CORE::CString::Empty;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GetShortestRelativePathFromModuleToProjectRoot( const CProjectInfo& projectInfo            ,
                                                const CModuleInfoEntryPtr& moduleInfoEntry )
{GUCEF_TRACE;

    return GetShortestRelativePathFromAbsPathToProjectRoot( projectInfo, moduleInfoEntry->rootDir );
}

/*---------------------------------------------------------------------------*/

void
GetAllModuleInfoFilePaths( const CProjectInfo& projectInfo            ,
                           const CModuleInfoEntryPtr& moduleInfoEntry ,
                           const CORE::CString& platform              ,
                           CORE::CString::StringSet& allPaths         ,
                           bool includeModuleRootPath                 ,
                           bool relativeToProjectRoot                 )
{GUCEF_TRACE;

    if ( includeModuleRootPath && relativeToProjectRoot )
    {
        CORE::CString::StringSet modulePaths;
        GetAllModuleInfoFilePaths( moduleInfoEntry, platform, modulePaths, false );
        
        CORE::CString projectRootPath = GetShortestRelativePathFromModuleToProjectRoot( projectInfo, moduleInfoEntry );
        projectRootPath = LocalizeDirSepCharForPlatform( projectRootPath, platform );

        CORE::CString::StringSet::iterator i = modulePaths.begin();
        while ( i != modulePaths.end() )
        {
            CORE::CString path = CORE::CombinePath( projectRootPath, (*i) );
            path = CORE::RelativePath( path, true );
            path = LocalizeDirSepCharForPlatform( path, platform );
            allPaths.insert( path );
            ++i;
        }
    }
    else
    {
        GetAllModuleInfoFilePaths( moduleInfoEntry, platform, allPaths, includeModuleRootPath );
    }
}

/*---------------------------------------------------------------------------*/

void
GetAllModuleInfoPaths( const CProjectInfo& projectInfo            ,
                       const CModuleInfoEntryPtr& moduleInfoEntry ,
                       const CORE::CString& platform              ,
                       CORE::CString::StringSet& allPaths         ,
                       bool includeModuleRootPath                 ,
                       bool relativeToProjectRoot                 ,
                       bool includeDepencencyIncludePaths         )
{GUCEF_TRACE;

    if ( includeModuleRootPath && relativeToProjectRoot )
    {
        CORE::CString::StringSet modulePaths;
        GetAllModuleInfoPaths( moduleInfoEntry, platform, modulePaths, false, includeDepencencyIncludePaths );
        
        CORE::CString projectRootPath = GetShortestRelativePathFromModuleToProjectRoot( projectInfo, moduleInfoEntry );
        projectRootPath = LocalizeDirSepCharForPlatform( projectRootPath, platform );
        
        allPaths.insert( projectRootPath );
        CORE::CString::StringSet::iterator i = modulePaths.begin();
        while ( i != modulePaths.end() )
        {
            CORE::CString path = CORE::CombinePath( projectRootPath, (*i) );
            path = CORE::RelativePath( path, true );
            path = LocalizeDirSepCharForPlatform( path, platform );
            allPaths.insert( path );
            ++i;
        }
    }
    else
    {
        GetAllModuleInfoPaths( moduleInfoEntry, platform, allPaths, includeModuleRootPath, includeDepencencyIncludePaths );
    }
}

/*---------------------------------------------------------------------------*/

const CModuleInfoEntryPtr
GetModuleInfoEntry( const TModuleInfoEntryPtrVector& moduleInfoEntries ,
                    const CORE::CString& moduleName                    ,
                    const CORE::CString& platform                      ,
                    CModuleInfoPtr* moduleInfo                         )
{GUCEF_TRACE;

    TModuleInfoEntryPtrVector::const_iterator i = moduleInfoEntries.begin();
    while ( i != moduleInfoEntries.end() )
    {
        CORE::CString nameOfCurrentModule = (*i)->GetModuleNameAlways( platform, moduleInfo );
        if ( nameOfCurrentModule == moduleName )
        {
            return (*i);
        }
        ++i;
    }
    return CModuleInfoEntryPtr();
}

/*---------------------------------------------------------------------------*/

void
GetAllTagsUsed( const CProjectInfo& projectInfo ,
                TStringSet& tagsUsed            )
{GUCEF_TRACE;

    TModuleInfoEntryPtrVector::const_iterator i = projectInfo.modules.begin();
    while ( i != projectInfo.modules.end() )
    {
        const TModuleInfoPtrMap& modulesPerPlatform = (*i)->GetModulesPerPlatform();
        TModuleInfoPtrMap::const_iterator n = modulesPerPlatform.begin();
        while ( n != modulesPerPlatform.end() )
        {
            MergeStringSet( tagsUsed, (*n).second->tags, true );
            ++n;
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

bool
IsModuleTagged( const CModuleInfoEntryPtr& module ,
                const CORE::CString& tag          ,
                const CORE::CString& platform     )
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator i = module->GetModulesPerPlatform().find( AllPlatforms );
    if ( i != module->GetModulesPerPlatform().end() )
    {
        if ( (*i).second->tags.find( tag ) != (*i).second->tags.end() )
            return true;
    }
    i = module->GetModulesPerPlatform().find( platform );
    if ( i != module->GetModulesPerPlatform().end() )
    {
        if ( (*i).second->tags.find( tag ) != (*i).second->tags.end() )
            return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
IsModuleTagged( const CModuleInfoEntryPtr& module    ,
                const CORE::CString::StringSet& tags ,
                const CORE::CString& platform        )
{GUCEF_TRACE;

    CORE::CString::StringSet::const_iterator i = tags.begin();
    while ( i != tags.end() )
    {
        if ( IsModuleTagged( module, (*i), platform ) )
            return true;
        ++i;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void
GetTaggedModules( const CProjectInfo& projectInfo       ,
                  const CORE::CString& tag              ,
                  TModuleInfoEntryPtrSet& taggedModules ,
                  const CORE::CString& platform         )
{GUCEF_TRACE;

    TModuleInfoEntryPtrVector::const_iterator i = projectInfo.modules.begin();
    while ( i != projectInfo.modules.end() )
    {
        if ( IsModuleTagged( (*i), tag, platform ) )
            taggedModules.insert( (*i) );
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

bool
ShouldModuleBeIgnored( const CModuleInfoEntryPtr& moduleInfo ,
                       const CORE::CString& platformName     )
{GUCEF_TRACE;

    const TModuleInfoPtrMap& modulesPerPlatform = moduleInfo->GetModulesPerPlatform();
    TModuleInfoPtrMap::const_iterator i = modulesPerPlatform.find( platformName );
    if ( i != modulesPerPlatform.end() )
    {
        const CModuleInfoPtr& moduleInfo = (*i).second;
        if ( moduleInfo->hasIgnoreModule )
            return moduleInfo->ignoreModule;
    }
    i = modulesPerPlatform.find( AllPlatforms );
    if ( i != modulesPerPlatform.end() )
    {
        const CModuleInfoPtr& moduleInfo = (*i).second;
        if ( moduleInfo->hasIgnoreModule )
            return moduleInfo->ignoreModule;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
IsModuleTaggedWith( const CModuleInfoEntryPtr& moduleInfo ,
                    const CORE::CString& platformName     ,
                    const CORE::CString& tag              )
{GUCEF_TRACE;

    const TModuleInfoPtrMap& modulesPerPlatform = moduleInfo->GetModulesPerPlatform();
    TModuleInfoPtrMap::const_iterator i = modulesPerPlatform.find( platformName );
    if ( i != modulesPerPlatform.end() )
    {
        if ( IsStringInList( (*i).second->tags, false, tag ) )
            return true;
    }
    i = modulesPerPlatform.find( AllPlatforms );
    if ( i != modulesPerPlatform.end() )
    {
        if ( IsStringInList( (*i).second->tags, false, tag ) )
            return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
HasIndependentModuleType( const TModuleInfoPtrMap& moduleDefs )
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator i = moduleDefs.begin();
    while ( i != moduleDefs.end() )
    {
        TModuleType moduleType = (*i).second->moduleType;

        if ( ( moduleType != MODULETYPE_HEADER_INCLUDE_LOCATION )   &&
             ( moduleType != MODULETYPE_HEADER_INTEGRATE_LOCATION ) &&
             ( moduleType != MODULETYPE_CODE_INTEGRATE_LOCATION )   &&
             ( moduleType != MODULETYPE_BINARY_PACKAGE )            &&
             ( moduleType != MODULETYPE_UNDEFINED )                 &&
             ( moduleType != MODULETYPE_UNKNOWN )                    )
        {
            return true;
        }
        ++i;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void
GetExecutables( const CProjectInfo& projectInfo           ,
                TModuleInfoEntryPtrSet& executableTargets ,
                const CORE::CString& platform             )
{GUCEF_TRACE;

    TModuleInfoEntryPtrVector::const_iterator i = projectInfo.modules.begin();
    while ( i != projectInfo.modules.end() )
    {
        if ( MODULETYPE_EXECUTABLE == GetModuleType( (*i), platform ) )
            executableTargets.insert( (*i) );
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
SplitProjectPerTarget( const CProjectInfo& projectInfo    ,
                       TProjectTargetInfoMapMap& targets  ,
                       bool tagsAsTargets                 ,
                       bool collapseRedundantPlatforms    ,
                       const TStringSet& platformsUsed    )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SplitProjectPerTarget: Commencing splitting of the gathered project info into derived targets. Will split among the " + 
        CORE::ToString( platformsUsed.size() ) + " platforms defined" );

    TStringSet::iterator p = platformsUsed.begin();
    while ( p != platformsUsed.end() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SplitProjectPerTarget: Locating executables (if any) for platform " + (*p) );

        TModuleInfoEntryPtrSet executables;
        GetExecutables( projectInfo, executables, (*p) );

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SplitProjectPerTarget: Located " + CORE::ToString( executables.size() ) + " executable target candidates for platform " + (*p) );

        TModuleInfoEntryPtrSet::iterator i = executables.begin();
        while ( i != executables.end() )
        {
            CModuleInfoEntryPtr executable = (*i);
            CORE::CString targetName = executable->GetModuleNameAlways( (*p) );

            // Don't bother if the executable itself doesnt have a platform definition for the current platform            
            if ( executable->GetModulesPerPlatform().find( (*p) ) != executable->GetModulesPerPlatform().end() )
            {            
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SplitProjectPerTarget: Locating dependencies for target candidate \"" + 
                    targetName + "\" for platform " + (*p) );

                TModuleInfoEntryPtrSet foundDependencies;
                if ( projectInfo.GetModuleDependencies( executable, (*p), foundDependencies, true, true ) )
                {
                    // if we made it here we found the executable and were able to satisfy all dependencies
                    // for the current platform

                    CORE::CString projectName = projectInfo.projectName + "_exe_" + targetName; 
                    TProjectTargetInfoMap& targetPerPlatform = targets[ projectName ];
                    TProjectTargetInfo& target = targetPerPlatform[ (*p) ];

                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SplitProjectPerTarget: Located " + CORE::ToString( foundDependencies.size() ) + 
                        " dependencies for executable target \"" + targetName + "\" for platform " + (*p) );

                    target.projectName = projectName;
                    target.mainModule = executable;
                    target.modules.insert( executable );
                    TModuleInfoEntryPtrSet::iterator j = foundDependencies.begin();
                    while ( j != foundDependencies.end() )
                    {
                        target.modules.insert( (*j) );
                        ++j;
                    } 

                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SplitProjectPerTarget: Executable Target \"" + targetName + "\" has been defined for platform " + (*p) );                        
                }
                else
                {
                     GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SplitProjectPerTarget: We cannot satisfy the full dependency chain for executable \"" + targetName + 
                        "\" for the given platform \"" + (*p) + "\", it will not be available as a target specific to this platform" );
                }
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SplitProjectPerTarget: The executable \"" + targetName + "\" has no definition specific to the given platform \"" 
                    + (*p) + "\" and thus will not be available as a target specific to this platform" ); 
            }
            ++i;
        }
        ++p;
    }

    if ( tagsAsTargets )
    {
        TStringSet tagsUsed;
        GetAllTagsUsed( projectInfo, tagsUsed );

        TStringSet::iterator p = platformsUsed.begin();
        while ( p != platformsUsed.end() )
        {
            TStringSet::iterator i = tagsUsed.begin();
            while ( i != tagsUsed.end() )
            {
                TModuleInfoEntryPtrSet taggedModules;
                GetTaggedModules( projectInfo, (*i), taggedModules, (*p) );
                CORE::CString projectName = projectInfo.projectName + "_tag_" + (*i);

                TModuleInfoEntryPtrSet::iterator m = taggedModules.begin();
                while ( m != taggedModules.end() )
                {
                    const CModuleInfoEntryPtr& taggedModule = (*m);
                                            
                    // Don't include this module if it doesnt have a definition for the current platform
                    if ( taggedModule->GetModulesPerPlatform().find( (*p) ) != taggedModule->GetModulesPerPlatform().end() )
                    {                    
                        // Tagged or not we need to include the dependencies of tagged modules as well
                        // We don't want to make projects that cannot compile
                        TModuleInfoEntryPtrSet foundDependencies;
                        if ( projectInfo.GetModuleDependencies( taggedModule, (*p), foundDependencies, true, true ) )
                        {
                            TProjectTargetInfoMap& targetPerPlatform = targets[ projectName ];
                            TProjectTargetInfo& target = targetPerPlatform[ (*p) ];

                            target.projectName = projectName;
                            target.modules.insert( taggedModule );

                            // Since many modules can have the same tag there really is no such thing as a 'main' tagged module.
                            // It may be that there is only 1 in a repo but that is a coincidence and not relevant
                            target.mainModule.Unlink();

                            TModuleInfoEntryPtrSet::iterator j = foundDependencies.begin();
                            while ( j != foundDependencies.end() )
                            {
                                target.modules.insert( (*j) );
                                ++j;
                            }
                        }
                    }
                    ++m;
                }
                ++i;
            }
            ++p;
        }
    }

    if ( collapseRedundantPlatforms )
    {
        TProjectTargetInfoMapMap::iterator t = targets.begin();
        while ( t != targets.end() )
        {
            // First check to see if this module has a 'AllPlatforms' definition
            // Without one we cannot collapse since there is no unifying target to collapse to
            TProjectTargetInfoMap& targetByPlatform = (*t).second;
            TProjectTargetInfoMap::iterator a = targetByPlatform.find( AllPlatforms );
            if ( a != targetByPlatform.end() )
            {
                TProjectTargetInfo& allPlatformsTarget = (*a).second;
                
                // We now check to see if the modules match across platforms which is all that is needed here
                // The modules themselves will deal with platform specifics at an intra-module level
                // The use-case we look for are cases where some platforms have different modules then others
                // in which case we need to keep them as distinct targets
                TStringSet redundantPlatforms; 
                TProjectTargetInfoMap::iterator m = targetByPlatform.begin();
                while (  m != targetByPlatform.end() )
                {
                     const CORE::CString& currentPlatform = (*m).first;
                     if ( currentPlatform != AllPlatforms )
                     {
                         TProjectTargetInfo& somePlatformTarget = (*m).second;
                         if ( somePlatformTarget.modules == allPlatformsTarget.modules )
                            redundantPlatforms.insert( (*m).first );
                    }
                    ++m;
                }
                TStringSet::iterator r = redundantPlatforms.begin();
                while ( r != redundantPlatforms.end() )
                {
                    targetByPlatform.erase( (*r) );
                    ++r;
                }
            }
            // else: targets that don't have a 'AllPlatforms' target cannot be collapsed
            ++t;
        }
    }

    // In order to facilitate uniform processing we also include the complete project as its own target
    // This ensures that backend code doesnt need different code to process the complete project vs some
    // target based subset
    // Note that the full project is by definition "all" platforms because there is no target differntiation
    // It relies soley on module level per-platform differenes to be processed

    TProjectTargetInfoMap& fullProjectTargets = targets[ projectInfo.projectName ];
    TProjectTargetInfo& fullProjectTarget = fullProjectTargets[ AllPlatforms ];
    fullProjectTarget.projectName = projectInfo.projectName;
    fullProjectTarget.mainModule.Unlink();
    TModuleInfoEntryPtrVector::const_iterator w = projectInfo.modules.begin();
    while ( w != projectInfo.modules.end() )
    {
        fullProjectTarget.modules.insert( (*w) );
        ++w;
    }
}

/*---------------------------------------------------------------------------*/

void
SplitProjectPerTarget( const CProjectInfo& projectInfo    ,
                       TProjectTargetInfoMapMap& targets  ,
                       bool tagsAsTargets                 ,
                       bool collapseRedundantPlatforms    )
{GUCEF_TRACE;

    TStringSet platformsUsed;
    projectInfo.GetAllPlatformsUsed( platformsUsed );

    SplitProjectPerTarget( projectInfo                , 
                           targets                    ,
                           tagsAsTargets              ,
                           collapseRedundantPlatforms ,
                           platformsUsed              );
}

/*-------------------------------------------------------------------------*/

const TProjectTargetInfo*
GetPlatformProjectTarget( const TProjectTargetInfoMap& platformTargets ,
                          const CORE::CString& platformName            )
{GUCEF_TRACE;

    TProjectTargetInfoMap::const_iterator i = platformTargets.find( platformName );
    if ( i != platformTargets.end() )
    {
        return &(*i).second;
    }
    if ( AllPlatforms != platformName )
    {
        i = platformTargets.find( AllPlatforms );
        if ( i != platformTargets.end() )
        {
            return &(*i).second;
        }
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
HasPlatformDefinition( const CModuleInfoEntryPtr& moduleInfoEntry ,
                       const CORE::CString& platform              )
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator i = moduleInfoEntry->GetModulesPerPlatform().begin();
    while ( i != moduleInfoEntry->GetModulesPerPlatform().end() )
    {
        if ( (*i).first.Equals( platform, false ) )
            return true;
        ++i;
    }
    return false;
}

/*--------------------------------------------------------------------------*/

const CORE::CString::StringSet&
GetKnownLicenseFiles( void )
{GUCEF_TRACE;

    static CORE::CString::StringSet licenseFiles;

    if ( licenseFiles.empty() )
    {
        licenseFiles.insert( "license" );
        licenseFiles.insert( "license.txt" );
        licenseFiles.insert( "copying" );
        licenseFiles.insert( "copying.lesser" );
    }
    return licenseFiles;
}

/*---------------------------------------------------------------------------*/

bool
DirHasLicenseFile( const CORE::CString& path      ,
                   CORE::CString& licenceFilePath )
{GUCEF_TRACE;

    if ( path.IsNULLOrEmpty() )
        return false;
    
    licenceFilePath.Clear();
    CORE::CString::StringSet licenseFiles = GetKnownLicenseFiles();

    CORE::SDI_Data* sdiData = CORE::DI_First_Dir_Entry( path.C_String() );
    if ( GUCEF_NULL != sdiData )
    {
        do
        {
            // make sure we are dealing with a file
            if ( 0 != DI_Is_It_A_File( sdiData ) )
            {
                CORE::CString filename = DI_Name( sdiData );
                if ( ( filename != "." ) && ( filename != ".." ) )
                {
                    CORE::CString lcFilename = filename.Lowercase();

                    if ( licenseFiles.find( lcFilename ) != licenseFiles.end() )
                    {
                        // we found one
                        licenceFilePath = CORE::CombinePath( path, filename );
                        break;
                    }
                }
            }
        }
        while ( 0 != DI_Next_Dir_Entry( sdiData ) );
        DI_Cleanup( sdiData );
    }

    return !licenceFilePath.IsNULLOrEmpty();
}

/*---------------------------------------------------------------------------*/

bool 
IsLicenseBsd( const CORE::CString& fileContentUpperCase )
{GUCEF_TRACE;

    static const char* bsdFragment =
    "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND " 
    "ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY "
    "AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE "
    "LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT "
    "LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) "
    "HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING "
    "NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE";

    static const char* bsdFragment2 =
    "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR "
    "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, "
    "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE "
    "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER "
    "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, "
    "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE "
    "SOFTWARE.";

    static const char* bsdFragment3 = 
    "THE \"BSD\" LICENCE";

    Int32 fragmentIndex = fileContentUpperCase.HasSubstr( bsdFragment, true );
    Int32 fragment2Index = fileContentUpperCase.HasSubstr( bsdFragment2, true );
    Int32 fragment3Index = fileContentUpperCase.HasSubstr( bsdFragment3, true );
    return fragmentIndex >= 0 || fragment2Index >= 0 || fragment3Index >= 0;
}

/*---------------------------------------------------------------------------*/

bool 
IsLicenseLGPLv3( const CORE::CString& fileContentUpperCase )
{GUCEF_TRACE;

    static const char* gnuFragment = "GNU LESSER GENERAL PUBLIC LICENSE";
    static const char* gnuFragment2 = "VERSION 3";

    Int32 fragmentIndex = fileContentUpperCase.HasSubstr( gnuFragment, true );
    Int32 fragment2Index = fileContentUpperCase.HasSubstr( gnuFragment2, true );
    return fragmentIndex >= 0 && fragment2Index >= 0;
}

/*---------------------------------------------------------------------------*/

bool 
IsLicenseLGPLv2( const CORE::CString& fileContentUpperCase )
{GUCEF_TRACE;

    static const char* gnuFragment = "GNU LESSER GENERAL PUBLIC LICENSE";
    static const char* gnuFragment2 = "VERSION 2";

    Int32 fragmentIndex = fileContentUpperCase.HasSubstr( gnuFragment, true );
    Int32 fragment2Index = fileContentUpperCase.HasSubstr( gnuFragment2, true );
    return fragmentIndex >= 0 && fragment2Index >= 0;
}

/*---------------------------------------------------------------------------*/

bool
IsLicenseApachev2( const CORE::CString& fileContentUpperCase )

{GUCEF_TRACE;

    static const char* gnuFragment = "APACHE LICENSE";
    static const char* gnuFragment2 = "VERSION 2";

    Int32 fragmentIndex = fileContentUpperCase.HasSubstr( gnuFragment, true );
    Int32 fragment2Index = fileContentUpperCase.HasSubstr( gnuFragment2, true );
    return fragmentIndex >= 0 && fragment2Index >= 0;
}

/*---------------------------------------------------------------------------*/

bool
IsLicenseApachev3( const CORE::CString& fileContentUpperCase )

{GUCEF_TRACE;

    static const char* gnuFragment = "APACHE LICENSE";
    static const char* gnuFragment2 = "VERSION 3";

    Int32 fragmentIndex = fileContentUpperCase.HasSubstr( gnuFragment, true );
    Int32 fragment2Index = fileContentUpperCase.HasSubstr( gnuFragment2, true );
    return fragmentIndex >= 0 && fragment2Index >= 0;
}

/*---------------------------------------------------------------------------*/

bool
IsLicensePublicDomain( const CORE::CString& fileContentUpperCase )
{GUCEF_TRACE;

    static const char* publicDomainFragment = "RELEASED TO THE PUBLIC DOMAIN";

    Int32 fragmentIndex = fileContentUpperCase.HasSubstr( publicDomainFragment, true );
    return fragmentIndex >= 0;
}

/*---------------------------------------------------------------------------*/

bool
IsLicenseZLib( const CORE::CString& fileContentUpperCase )
{GUCEF_TRACE;

    static const char* zlibFragment = "ZLIB LICENSE";

    Int32 fragmentIndex = fileContentUpperCase.HasSubstr( zlibFragment, true );
    return fragmentIndex >= 0;
}

/*---------------------------------------------------------------------------*/

bool
IsLicenseMIT( const CORE::CString& fileContentUpperCase )
{GUCEF_TRACE;

    static const char* mitFragment = "MIT LICENSE";

    Int32 fragmentIndex = fileContentUpperCase.HasSubstr( mitFragment, true );
    return fragmentIndex >= 0;
}

/*---------------------------------------------------------------------------*/

bool
TryAutoLicenceDetection( const CORE::CString& fileContent ,
                         CORE::CString& detectedLicense   )
{GUCEF_TRACE;

    detectedLicense.Clear();
    CORE::CString uppercaseContent = fileContent.Uppercase().RemoveChar( '\n' ).RemoveChar( '\r' );

    if ( IsLicenseMIT( uppercaseContent ) )
    {
        detectedLicense = "MIT";
        return true;
    }
    if ( IsLicenseZLib( uppercaseContent ) )
    {
        detectedLicense = "ZLIB";
        return true;
    }
    if ( IsLicensePublicDomain( uppercaseContent ) )
    {
        detectedLicense = "PUBLIC DOMAIN";
        return true;
    }
    if ( IsLicenseApachev3( uppercaseContent ) )
    {
        detectedLicense = "APACHEv3";
        return true;
    }
    if ( IsLicenseApachev2( uppercaseContent ) )
    {
        detectedLicense = "APACHEv2";
        return true;
    }
    if ( IsLicenseLGPLv2( uppercaseContent ) )
    {
        detectedLicense = "LGPLv2";
        return true;
    }
    if ( IsLicenseLGPLv3( uppercaseContent ) )
    {
        detectedLicense = "LGPLv3";
        return true;
    }    
    if ( IsLicenseBsd( uppercaseContent ) )
    {
        detectedLicense = "BSD";
        return true;
    }    

    return false;
}

/*---------------------------------------------------------------------------*/

bool
IsAnyLicenseDefined( const TModuleInfoEntryPtrVector& moduleInfoEntries )
{GUCEF_TRACE;

    TModuleInfoEntryPtrVector::const_iterator i = moduleInfoEntries.begin();
    while ( i != moduleInfoEntries.end() )
    {
        const CModuleInfoEntryPtr& entry = (*i);
        if ( entry->IsAnyLicenseDefined() )
        {
            return true;
        }
        ++i;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
IsAnySemVerDefined( const TModuleInfoEntryPtrVector& moduleInfoEntries )
{GUCEF_TRACE;

    TModuleInfoEntryPtrVector::const_iterator i = moduleInfoEntries.begin();
    while ( i != moduleInfoEntries.end() )
    {
        const CModuleInfoEntryPtr& entry = (*i);
        if ( entry->HasAnySemVer() )
        {
            return true;
        }
        ++i;
    }
    return false;
}

/*--------------------------------------------------------------------------*/

const CORE::CString::StringSet&
GetKnownSemVerFiles( void )
{GUCEF_TRACE;

    static CORE::CString::StringSet semverFiles;

    if ( semverFiles.empty() )
    {
        semverFiles.insert( "semver" );
        semverFiles.insert( "semver.txt" );
        semverFiles.insert( "version" );
        semverFiles.insert( "version.txt" );
        semverFiles.insert( "version-semver" );
        semverFiles.insert( "version-semver.txt" );
    }
    return semverFiles;
}

/*---------------------------------------------------------------------------*/

bool
DirHasSemVerFile( const CORE::CString& path     ,
                  CORE::CString& semverFilePath )
{GUCEF_TRACE;

    if ( path.IsNULLOrEmpty() )
        return false;
    
    semverFilePath.Clear();
    CORE::CString::StringSet semverFiles = GetKnownSemVerFiles();

    CORE::SDI_Data* sdiData = CORE::DI_First_Dir_Entry( path.C_String() );
    if ( GUCEF_NULL != sdiData )
    {
        do
        {
            // make sure we are dealing with a file
            if ( 0 != DI_Is_It_A_File( sdiData ) )
            {
                CORE::CString filename = DI_Name( sdiData );
                if ( ( filename != "." ) && ( filename != ".." ) )
                {
                    CORE::CString lcFilename = filename.Lowercase();

                    if ( semverFiles.find( lcFilename ) != semverFiles.end() )
                    {
                        // we found one
                        semverFilePath = CORE::CombinePath( path, filename );
                        break;
                    }
                }
            }
        }
        while ( 0 != DI_Next_Dir_Entry( sdiData ) );
        DI_Cleanup( sdiData );
    }

    return !semverFilePath.IsNULLOrEmpty();
}

/*---------------------------------------------------------------------------*/

bool
TryAutoSemVerDetection( const CORE::CString& fileContent ,
                        CORE::CVersion& detectedSemVer   )
{GUCEF_TRACE;

    // @TODO: Support more variation
    return detectedSemVer.FromString( fileContent );
}

/*---------------------------------------------------------------------------*/

CLinkedLibrarySettings::CLinkedLibrarySettings( void )
    : CORE::CTSharedObjCreator< CLinkedLibrarySettings, MT::CMutex >( this )
    , m_moduleType( TModuleType::MODULETYPE_UNDEFINED )
    , m_libPath()
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CLinkedLibrarySettings::CLinkedLibrarySettings( const CLinkedLibrarySettings& src )
    : CORE::CTSharedObjCreator< CLinkedLibrarySettings, MT::CMutex >( this )
    , m_moduleType( src.m_moduleType )
    , m_libPath( src.m_libPath )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CLinkedLibrarySettings::~CLinkedLibrarySettings()
{GUCEF_TRACE;

    Clear();
}

/*---------------------------------------------------------------------------*/

void
CLinkedLibrarySettings::Clear( void )
{GUCEF_TRACE;

    m_moduleType = TModuleType::MODULETYPE_UNDEFINED;
    m_libPath.Clear();
}

/*---------------------------------------------------------------------------*/

void
CLinkedLibrarySettings::SetModuleType( TModuleType moduleType )
{GUCEF_TRACE;

    m_moduleType = moduleType;
}

/*---------------------------------------------------------------------------*/

TModuleType
CLinkedLibrarySettings::GetModuleType( void ) const
{GUCEF_TRACE;

    return m_moduleType;
}

/*---------------------------------------------------------------------------*/

void
CLinkedLibrarySettings::SetLibraryPath( const CORE::CString& libPath )
{GUCEF_TRACE;

    m_libPath = libPath;
}

/*---------------------------------------------------------------------------*/

const CORE::CString&
CLinkedLibrarySettings::GetLibraryPath( void ) const
{GUCEF_TRACE;

    return m_libPath;
}

/*---------------------------------------------------------------------------*/

bool
CLinkedLibrarySettings::Merge( const CLinkedLibrarySettings& linkedLibrarySettingsToMergeIn ,
                               bool onConflictOriginalInfoStays                             )
{GUCEF_TRACE;

    if ( m_libPath.IsNULLOrEmpty() )
        m_libPath = linkedLibrarySettingsToMergeIn.m_libPath;
    else
    if ( !onConflictOriginalInfoStays && !linkedLibrarySettingsToMergeIn.m_libPath.IsNULLOrEmpty() )
        m_libPath = linkedLibrarySettingsToMergeIn.m_libPath;

    if ( TModuleType::MODULETYPE_UNDEFINED == m_moduleType || ( TModuleType::MODULETYPE_UNKNOWN == m_moduleType && TModuleType::MODULETYPE_UNDEFINED != linkedLibrarySettingsToMergeIn.m_moduleType ) )
        m_moduleType = linkedLibrarySettingsToMergeIn.m_moduleType;
    else
    if ( !onConflictOriginalInfoStays && !( TModuleType::MODULETYPE_UNDEFINED == linkedLibrarySettingsToMergeIn.m_moduleType || TModuleType::MODULETYPE_UNKNOWN == linkedLibrarySettingsToMergeIn.m_moduleType ) )
        m_moduleType = linkedLibrarySettingsToMergeIn.m_moduleType;

    return true;
}

/*---------------------------------------------------------------------------*/

CLinkerSettings::CLinkerSettings( void )
    : CORE::CTSharedObjCreator< CLinkerSettings, MT::CMutex >( this )
    , m_linkedLibraries()
    , m_linkedLogicalLibraries()
    , m_libPaths()
    , m_targetName()
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CLinkerSettings::CLinkerSettings( const CLinkerSettings& src )
    : CORE::CTSharedObjCreator< CLinkerSettings, MT::CMutex >( this )
    , m_linkedLibraries( src.m_linkedLibraries )
    , m_linkedLogicalLibraries( src.m_linkedLogicalLibraries )
    , m_libPaths( src.m_libPaths )
    , m_targetName( src.m_targetName )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CLinkerSettings::~CLinkerSettings()
{GUCEF_TRACE;

    Clear();
}

/*---------------------------------------------------------------------------*/

void
CLinkerSettings::Clear( void )
{GUCEF_TRACE;

    m_linkedLibraries.clear();
    m_linkedLogicalLibraries.clear();
    m_libPaths.clear();
    m_targetName.Clear();
}

/*---------------------------------------------------------------------------*/

const TLinkedLibrarySettingsPtrMap&
CLinkerSettings::GetLinkedLibraries( void ) const
{GUCEF_TRACE;

    return m_linkedLibraries;
}

/*---------------------------------------------------------------------------*/

const TLinkedLibrarySettingsPtrMap&
CLinkerSettings::GetLinkedLogicalLibraries( void ) const
{GUCEF_TRACE;

    return m_linkedLogicalLibraries;
}

/*---------------------------------------------------------------------------*/

bool
CLinkerSettings::HasLinkerDependency( const CORE::CString& dependencyName ) const
{GUCEF_TRACE;

    return m_linkedLibraries.find( dependencyName ) != m_linkedLibraries.end();
}

/*---------------------------------------------------------------------------*/

const TStringSet&
CLinkerSettings::GetLibraryPaths( void ) const
{GUCEF_TRACE;

    return m_libPaths;
}

/*---------------------------------------------------------------------------*/

void
CLinkerSettings::SetTargetName( const CORE::CString& targetName )
{GUCEF_TRACE;

    m_targetName = targetName;
}

/*---------------------------------------------------------------------------*/

const CORE::CString&
CLinkerSettings::GetTargetName( void ) const
{GUCEF_TRACE;

    return m_targetName;
}

/*---------------------------------------------------------------------------*/

void
CLinkerSettings::GetListOfLinkedLibraries( CORE::CStringSet& linkedLibraries ) const
{GUCEF_TRACE;

    TLinkedLibrarySettingsPtrMap::const_iterator i = m_linkedLibraries.begin();
    while ( i != m_linkedLibraries.end() )
    {
        linkedLibraries.insert( (*i).first );
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

bool
CLinkerSettings::MergeLibraryPaths( const TStringSet& libPathsToMergeIn )
{GUCEF_TRACE;

    MergeStringSet( m_libPaths, libPathsToMergeIn, true );
    return true;
}

/*---------------------------------------------------------------------------*/

bool
CLinkerSettings::TryGetLinkedLibrary( const CORE::CString& libraryName         ,
                                      CLinkedLibrarySettingsPtr& linkedLibrary ,
                                      bool createDefaultIfNotExist             ) 
{GUCEF_TRACE;

    TLinkedLibrarySettingsPtrMap::iterator i = m_linkedLibraries.find( libraryName );
    if ( i != m_linkedLibraries.end() )
    {
       linkedLibrary = (*i).second;
       return true;
    }
    else
    {
        if ( createDefaultIfNotExist )
        {
            linkedLibrary = CLinkedLibrarySettings::CreateSharedObj();
            if ( !linkedLibrary.IsNULL() )
            {
                m_linkedLibraries[ libraryName ] = linkedLibrary;
                return true;
            }
        }
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
CLinkerSettings::TryGetLinkedLibrary( const CORE::CString& libraryName         ,
                                      CLinkedLibrarySettingsPtr& linkedLibrary ) const
{GUCEF_TRACE;

    TLinkedLibrarySettingsPtrMap::const_iterator i = m_linkedLibraries.find( libraryName );
    if ( i != m_linkedLibraries.end() )
    {
       linkedLibrary = (*i).second;
       return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void
CLinkerSettings::DeleteLinkedLibrary( const CORE::CString& libraryName )
{GUCEF_TRACE;

    m_linkedLibraries.erase( libraryName );
}

/*---------------------------------------------------------------------------*/

bool
CLinkerSettings::MoveLinkedLibraryToLogicalLibraries( const CORE::CString& libraryName )
{GUCEF_TRACE;

    TLinkedLibrarySettingsPtrMap::iterator i = m_linkedLibraries.find( libraryName );
    if ( i != m_linkedLibraries.end() )
    {
       CLinkedLibrarySettingsPtr linkedLibrary = (*i).second;
       m_linkedLibraries.erase( libraryName );
       m_linkedLogicalLibraries[ libraryName ] = linkedLibrary;
       return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
CLinkerSettings::Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    // Go through all linker related settings, if any exist
    const CORE::CDataNode* linkerNode = domRootNode.Find( "Linker" );
    if ( GUCEF_NULL != linkerNode )
    {
        m_targetName = linkerNode->GetAttributeValueOrChildValueByName( "TargetName" );
        
        // If general paths were defined at the linker level add them to each dependency
        // They should be added after the dependency paths this could have been added above because
        // those should take priority in a linker search for paths        
        m_libPaths = StringVectorToStringSet( linkerNode->GetAttributeValueOrChildValueByName( "LibPaths" ).AsString().ParseElements( ';' ) );
        
        // Find all the libraries that are linked but not part of the overall project
        CORE::CDataNode::TConstDataNodeSet linkedLibs = linkerNode->FindChildrenOfType( "Dependency" );
        CORE::CDataNode::TConstDataNodeSet::const_iterator i = linkedLibs.begin();
        while ( i != linkedLibs.end() )
        {
            const CORE::CDataNode* linkedLibNode = (*i);
            CORE::CString linkedLibName = linkedLibNode->GetAttributeValue( "Name" );
            if ( !linkedLibName.IsNULLOrEmpty() )
            {
                CLinkedLibrarySettingsPtr linkedLibrary;
                if ( TryGetLinkedLibrary( linkedLibName, linkedLibrary, true ) && !linkedLibrary.IsNULL() )
                {
                    CORE::CString linkedLibType = linkedLibNode->GetAttributeValue( "Type" );
                    if ( !linkedLibType.IsNULLOrEmpty() )
                    {
                        linkedLibrary->SetModuleType( StringToModuleType( linkedLibType ) );
                    }
                    else
                    {
                        linkedLibrary->SetModuleType( MODULETYPE_UNDEFINED );
                    }

                    linkedLibrary->SetLibraryPath( linkedLibNode->GetAttributeValue( "Path" ) );
                }
            }
            ++i;
        }
    }
    return true;
}

/*---------------------------------------------------------------------------*/

bool
CLinkerSettings::MergeLinkedLibraries( const CLinkerSettings& linkerSettingsToMergeIn ,
                                       bool onConflictOriginalInfoStays               )
{GUCEF_TRACE;

    bool totalSuccess = true;

    TLinkedLibrarySettingsPtrMap::const_iterator i = linkerSettingsToMergeIn.m_linkedLibraries.begin();
    while ( i != linkerSettingsToMergeIn.m_linkedLibraries.end() )
    {
        const CORE::CString& libName = (*i).first;
        const CLinkedLibrarySettingsPtr& libSettingsToMergeIn = (*i).second;

        if ( !libSettingsToMergeIn.IsNULL() )
        {
            TLinkedLibrarySettingsPtrMap::iterator n = m_linkedLibraries.find( libName );
            if ( n != m_linkedLibraries.end() )
            {
                // We already have such a library specified and as such we will merge
                CLinkedLibrarySettingsPtr& targetLibSettings = (*n).second;
                totalSuccess = targetLibSettings->Merge( *libSettingsToMergeIn, onConflictOriginalInfoStays ) && totalSuccess;
            }
            else
            {
                // No such library specified, add it
                m_linkedLibraries[ libName ] = CLinkedLibrarySettings::CreateSharedObjWithParam( *libSettingsToMergeIn );
            }
        }
        ++i;
    }

    MergeStringSet( m_libPaths, linkerSettingsToMergeIn.m_libPaths, true );

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CLinkerSettings::Merge( const CLinkerSettings& linkerSettingsToMergeIn ,
                        bool onConflictOriginalInfoStays               )
{GUCEF_TRACE;

    bool totalSuccess = true;

    totalSuccess = MergeLinkedLibraries( linkerSettingsToMergeIn, onConflictOriginalInfoStays ) && totalSuccess;

    if ( m_targetName.IsNULLOrEmpty() )
        m_targetName = linkerSettingsToMergeIn.m_targetName;
    else
    if ( !onConflictOriginalInfoStays && !linkerSettingsToMergeIn.m_targetName.IsNULLOrEmpty() )
        m_targetName = linkerSettingsToMergeIn.m_targetName;

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

CCompilerSettings::CCompilerSettings( void )
    : CORE::CTSharedObjCreator< CCompilerSettings, MT::CMutex >( this )
    , m_languagesUsed()
    , m_compilerFlags()
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CCompilerSettings::CCompilerSettings( const CCompilerSettings& src )
    : CORE::CTSharedObjCreator< CCompilerSettings, MT::CMutex >( this )
    , m_languagesUsed( src.m_languagesUsed )
    , m_compilerFlags( src.m_compilerFlags )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CCompilerSettings::~CCompilerSettings()
{GUCEF_TRACE;

    Clear();
}

/*---------------------------------------------------------------------------*/

void
CCompilerSettings::AddUsedLanguage( const CORE::CString& languageUsed )
{GUCEF_TRACE;

    m_languagesUsed.insert( languageUsed );
}

/*---------------------------------------------------------------------------*/

const CORE::CStringSet&
CCompilerSettings::GetLanguagesUsed( void ) const
{GUCEF_TRACE;

    return m_languagesUsed;
}

/*---------------------------------------------------------------------------*/

const CORE::CStringMap&
CCompilerSettings::GetCompilerFlags( void ) const
{GUCEF_TRACE;

    return m_compilerFlags;
}

/*---------------------------------------------------------------------------*/

bool
CCompilerSettings::Merge( const CCompilerSettings& compilerSettingsToMergeIn ,
                          bool onConflictOriginalInfoStays                   )
{GUCEF_TRACE;

    MergeStringSet( m_languagesUsed, compilerSettingsToMergeIn.m_languagesUsed, false );
    MergeStringMap( m_compilerFlags, compilerSettingsToMergeIn.m_compilerFlags, false, true );
    return true;
}

/*---------------------------------------------------------------------------*/

bool
CCompilerSettings::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

   // Go through all preprocessor related settings, if any exist
    const CORE::CDataNode* compilerNode = domRootNode.Find( "Compiler" );
    if ( GUCEF_NULL != compilerNode )
    {
        // Find all the code languages defined for the compiler
        CORE::CDataNode::TConstDataNodeSet languages = compilerNode->FindChildrenOfType( "Language" );
        CORE::CDataNode::TConstDataNodeSet::const_iterator i = languages.begin();
        while ( i != languages.end() )
        {
            const CORE::CDataNode* languageNode = (*i);
            CORE::CString name = languageNode->GetAttributeValue( "Name" );
            if ( !name.IsNULLOrEmpty() )
            {
                m_languagesUsed.insert( name );
            }
            ++i;
        }

        // Find all the compiler flags defined per compiler
        CORE::CDataNode::TConstDataNodeSet flagNodes = compilerNode->FindChildrenOfType( "CompilerFlags" );
        i = flagNodes.begin();
        while ( i != flagNodes.end() )
        {
            const CORE::CDataNode* flagsNode = (*i);
            CORE::CString compilerName = flagsNode->GetAttributeValue( "CompilerName" );
            CORE::CString flags = flagsNode->GetAttributeValue( "Flags" );
            if ( !compilerName.IsNULLOrEmpty() && !flags.IsNULLOrEmpty() )
            {
                m_compilerFlags[ compilerName ] = flags;
            }
            ++i;
        }
    }
    return true;
}

/*---------------------------------------------------------------------------*/

void
CCompilerSettings::Clear( void )
{GUCEF_TRACE;

    m_languagesUsed.clear();
    m_compilerFlags.clear();
}

/*---------------------------------------------------------------------------*/

CPreprocessorSettings::CPreprocessorSettings( void )
    : CORE::CTSharedObjCreator< CPreprocessorSettings, MT::CMutex >( this )
    , m_defines()
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CPreprocessorSettings::CPreprocessorSettings( const CPreprocessorSettings& src )
    : CORE::CTSharedObjCreator< CPreprocessorSettings, MT::CMutex >( this )
    , m_defines( src.m_defines )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CPreprocessorSettings::~CPreprocessorSettings()
{GUCEF_TRACE;

    Clear();
}

/*---------------------------------------------------------------------------*/

void
CPreprocessorSettings::AddDefine( const CORE::CString& define )
{GUCEF_TRACE;

    m_defines.insert( define );
}

/*---------------------------------------------------------------------------*/

const CORE::CStringSet&
CPreprocessorSettings::GetDefines( void ) const
{GUCEF_TRACE;

    return m_defines;
}

/*---------------------------------------------------------------------------*/

bool
CPreprocessorSettings::Merge( const CPreprocessorSettings& preprocessorSettingsToMergeIn ,
                              bool onConflictOriginalInfoStays                           )
{GUCEF_TRACE;

    MergeStringSet( m_defines, preprocessorSettingsToMergeIn.m_defines, false );
    return true;
}

/*---------------------------------------------------------------------------*/

bool
CPreprocessorSettings::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                    const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    const CORE::CDataNode* preprocessorNode = domRootNode.Find( "Preprocessor" );
    if ( GUCEF_NULL != preprocessorNode )
    {
        // Find all the preprocessor definitions
        CORE::CDataNode::TConstDataNodeSet defines = preprocessorNode->FindChildrenOfType( "Define" );
        CORE::CDataNode::TConstDataNodeSet::const_iterator i = defines.begin();
        while ( i != defines.end() )
        {
            const CORE::CDataNode* defineNode = (*i);
            CORE::CString defineValue = defineNode->GetAttributeValue( "String" );
            if ( !defineValue.IsNULLOrEmpty() )
            {
                AddDefine( defineValue );
            }
            ++i;
        }
    }
    return true;
}

/*---------------------------------------------------------------------------*/

void
CPreprocessorSettings::Clear( void )
{GUCEF_TRACE;

    m_defines.clear();
}

/*---------------------------------------------------------------------------*/

const CORE::CString CModuleMetaData::ClassTypeName = "GUCEF::PROJECTGEN::CModuleMetaData";

/*---------------------------------------------------------------------------*/

CModuleMetaData::CModuleMetaData( void ) 
    : CORE::CIDataNodeSerializable()
    , lastEditBy()
    , authors()
    , maintainers()
    , descriptionHeadline()
    , descriptionDetails()
    , license()
    , m_semver()
    , m_hasSemVer( false )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CModuleMetaData::CModuleMetaData( const CModuleMetaData& src ) 
    : CORE::CIDataNodeSerializable( src )
    , lastEditBy( src.lastEditBy )
    , authors( src.authors )
    , maintainers( src.maintainers )
    , descriptionHeadline( src.descriptionHeadline )
    , descriptionDetails( src.descriptionDetails )
    , license( src.license )
    , m_semver( src.m_semver )
    , m_hasSemVer( src.m_hasSemVer )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CModuleMetaData::~CModuleMetaData() 
{GUCEF_TRACE;

    Clear();
}

/*---------------------------------------------------------------------------*/

void
CModuleMetaData::Clear( void ) 
{GUCEF_TRACE;

    lastEditBy.Clear();
    authors.clear();
    maintainers.clear();    
    descriptionHeadline.Clear();
    descriptionDetails.Clear();
    license.Clear();
    m_semver.Clear();
    m_hasSemVer = false;
}

/*---------------------------------------------------------------------------*/

void
CModuleMetaData::SetSemVer( const CORE::CVersion& semVer )
{GUCEF_TRACE;

    m_semver = semVer;
    m_hasSemVer = true;
}

/*---------------------------------------------------------------------------*/

const CORE::CVersion&
CModuleMetaData::GetSemVer( void ) const
{GUCEF_TRACE;

    return m_semver;
}

/*---------------------------------------------------------------------------*/

bool
CModuleMetaData::HasSemVer( void ) const
{GUCEF_TRACE;

    return m_hasSemVer;
}

/*---------------------------------------------------------------------------*/

bool
CModuleMetaData::HasAnyMetaData( void ) const
{GUCEF_TRACE;

    return m_hasSemVer ||
           !lastEditBy.IsNULLOrEmpty() ||
           !authors.empty() ||
           !maintainers.empty() ||
           !descriptionHeadline.IsNULLOrEmpty() ||
           !descriptionDetails.IsNULLOrEmpty() ||
           !license.IsNULLOrEmpty();
}

/*---------------------------------------------------------------------------*/

bool
CModuleMetaData::Merge( const CModuleMetaData& moduleMetaDataToMergeIn ,
                        bool onConflictOriginalInfoStays               )
{GUCEF_TRACE;

    if ( !moduleMetaDataToMergeIn.lastEditBy.IsNULLOrEmpty() )
        lastEditBy = moduleMetaDataToMergeIn.lastEditBy;
    
    MergeStringSet( authors, moduleMetaDataToMergeIn.authors, false );

    MergeStringSet( maintainers, moduleMetaDataToMergeIn.maintainers, false );

    if ( descriptionHeadline.IsNULLOrEmpty() )
        descriptionHeadline = moduleMetaDataToMergeIn.descriptionHeadline;
    if ( descriptionDetails.IsNULLOrEmpty() )
        descriptionDetails = moduleMetaDataToMergeIn.descriptionDetails;
    if ( license.IsNULLOrEmpty() )
        license = moduleMetaDataToMergeIn.license;

    if ( ( !m_hasSemVer || !onConflictOriginalInfoStays ) && moduleMetaDataToMergeIn.m_hasSemVer )
    {
        m_semver = moduleMetaDataToMergeIn.m_semver;
        m_hasSemVer = true;
    }

    return true;
}

/*---------------------------------------------------------------------------*/

CModuleMetaData&
CModuleMetaData::operator=( const CModuleMetaData& src ) 
{GUCEF_TRACE;

    if ( &src != this )
    {
        lastEditBy = src.lastEditBy;
        authors = src.authors;
        maintainers = src.maintainers;        
        descriptionHeadline = src.descriptionHeadline;
        descriptionDetails = src.descriptionDetails;
        license = src.license;
        m_semver = src.m_semver;
        m_hasSemVer = src.m_hasSemVer;
    }
    return *this;
}

/*---------------------------------------------------------------------------*/

bool 
CModuleMetaData::Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    if ( !lastEditBy.IsNULLOrEmpty() )
    {
        domRootNode.AddChildWithValue( "LastEditBy", lastEditBy, GUCEF_DATATYPE_STRING );
    }
    if ( !descriptionHeadline.IsNULLOrEmpty() )
    {
        domRootNode.AddChildWithValue( "DescriptionHeadline", descriptionHeadline, GUCEF_DATATYPE_STRING );
    }
    if ( !descriptionDetails.IsNULLOrEmpty() )
    {
        domRootNode.AddChildWithValue( "DescriptionDetails", descriptionDetails, GUCEF_DATATYPE_STRING );
    }
    if ( !license.IsNULLOrEmpty() )
    {
        domRootNode.AddChildWithValue( "License", license, GUCEF_DATATYPE_STRING );
    }
    if ( m_hasSemVer )
    {
        domRootNode.AddChildWithValue( "SemVer", m_semver.ToString(), GUCEF_DATATYPE_STRING );
    }

    domRootNode.AddAllValuesAsChildrenOfChild< CORE::CString::StringSet >( "Authors", authors, GUCEF_DATATYPE_ARRAY, "Author" );
    domRootNode.AddAllValuesAsChildrenOfChild< CORE::CString::StringSet >( "Maintainers", maintainers, GUCEF_DATATYPE_ARRAY, "Maintainer" );

    return true;
}

/*---------------------------------------------------------------------------*/

bool
CModuleMetaData::Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    license = domRootNode.GetAttributeValueOrChildValueByName( "License", license, false ).AsString( license, true );
    descriptionHeadline = domRootNode.GetAttributeValueOrChildValueByName( "DescriptionHeadline", descriptionHeadline, false ).AsString( descriptionHeadline, true );
    descriptionDetails = domRootNode.GetAttributeValueOrChildValueByName( "DescriptionDetails", descriptionDetails, false ).AsString( descriptionDetails, true );
    m_semver.FromString( domRootNode.GetAttributeValueOrChildValueByName( "SemVer", m_semver.ToString(), false ).AsString( m_semver.ToString(), true ) );
    m_hasSemVer = domRootNode.GetAttributeValueOrChildValueByName( "HasSemVer", m_hasSemVer ).AsBool( m_hasSemVer, true );

    domRootNode.GetValuesOfChildByName( "Authors", authors );
    domRootNode.GetValuesOfChildByName( "Maintainers", maintainers );

    return true;
}

/*---------------------------------------------------------------------------*/

CORE::CICloneable* 
CModuleMetaData::Clone( void ) const 
{GUCEF_TRACE;

    return new CModuleMetaData( *this );
}

/*---------------------------------------------------------------------------*/

const CORE::CString& 
CModuleMetaData::GetClassTypeName( void ) const 
{GUCEF_TRACE;

    return ClassTypeName;
}

/*---------------------------------------------------------------------------*/

CModuleInfo::CModuleInfo( void ) 
    : CORE::CTSharedObjCreator< CModuleInfo, MT::CMutex >( this )
    , name()    
    , moduleType( MODULETYPE_UNDEFINED )
    , tags()
    , dependencyIncludeDirs()
    , runtimeDependencies()
    , buildOrder( -1 )
    , considerSubDirs( true )
    , hasConsiderSubDirs( false )
    , linkerSettings()
    , compilerSettings()
    , preprocessorSettings()
    , ignoreModule( false )
    , hasIgnoreModule( false )
    , metadata()
    , m_platformName()
    , m_namesOfDependencies()
    , m_namesOfLogicalDependencies()
    , m_includeDirs()
    , m_sourceDirs()
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CModuleInfo::CModuleInfo( const CModuleInfo& src ) 
    : CORE::CTSharedObjCreator< CModuleInfo, MT::CMutex >( this )
    , name( src.name )
    , moduleType( src.moduleType )
    , tags( src.tags )
    , dependencyIncludeDirs( src.dependencyIncludeDirs )
    , runtimeDependencies( src.runtimeDependencies )
    , buildOrder( src.buildOrder )
    , considerSubDirs( src.considerSubDirs )
    , hasConsiderSubDirs( src.hasConsiderSubDirs )
    , linkerSettings( src.linkerSettings )
    , compilerSettings( src.compilerSettings )
    , preprocessorSettings( src.preprocessorSettings )
    , ignoreModule( src.ignoreModule )
    , hasIgnoreModule( src.hasIgnoreModule )
    , metadata( src.metadata )
    , m_platformName( src.m_platformName )
    , m_namesOfDependencies( src.m_namesOfDependencies )
    , m_namesOfLogicalDependencies( src.m_namesOfLogicalDependencies )
    , m_includeDirs( src.m_includeDirs )
    , m_sourceDirs( src.m_sourceDirs )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CModuleInfo::~CModuleInfo() 
{GUCEF_TRACE;

    Clear();
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::Clear( void ) 
{GUCEF_TRACE;

    name.Clear();
    moduleType = MODULETYPE_UNDEFINED;
    tags.clear();
    dependencyIncludeDirs.clear();
    runtimeDependencies.clear();
    buildOrder = -1;
    considerSubDirs = true;
    hasConsiderSubDirs = false;
    linkerSettings.Clear();
    compilerSettings.Clear();
    preprocessorSettings.Clear();
    ignoreModule = false;
    hasIgnoreModule = false;
    metadata.Clear();

    m_platformName.Clear();
    m_namesOfDependencies.clear();
    m_namesOfLogicalDependencies.clear();
    m_includeDirs.clear();
    m_sourceDirs.clear();
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfo::Merge( CModuleInfoPtr moduleInfoToMergeIn ,
                    bool onConflictOriginalInfoStays   )
{GUCEF_TRACE;

    bool totalSuccess = true;

    if ( m_platformName.IsNULLOrEmpty() )
        m_platformName = moduleInfoToMergeIn->m_platformName;
    else
    if ( !onConflictOriginalInfoStays && !moduleInfoToMergeIn->m_platformName.IsNULLOrEmpty() )
        m_platformName = moduleInfoToMergeIn->m_platformName;

    if ( name.IsNULLOrEmpty() )
        name = moduleInfoToMergeIn->name;
    else
    if ( !onConflictOriginalInfoStays && !moduleInfoToMergeIn->name.IsNULLOrEmpty() )
        name = moduleInfoToMergeIn->name;

    if ( MODULETYPE_UNDEFINED == moduleType )
        moduleType = moduleInfoToMergeIn->moduleType;
    else
    if ( !onConflictOriginalInfoStays && MODULETYPE_UNDEFINED != moduleInfoToMergeIn->moduleType )
        moduleType = moduleInfoToMergeIn->moduleType;

    MergeStringSetMap( m_includeDirs, moduleInfoToMergeIn->m_includeDirs, true );
    MergeStringSetMap( m_sourceDirs, moduleInfoToMergeIn->m_sourceDirs, true );

    MergeStringSet( tags, moduleInfoToMergeIn->tags, true );
    MergeStringSet( m_namesOfDependencies, moduleInfoToMergeIn->m_namesOfDependencies, true );
    MergeStringSet( m_namesOfLogicalDependencies, moduleInfoToMergeIn->m_namesOfDependencies, true );
    MergeStringSet( dependencyIncludeDirs, moduleInfoToMergeIn->dependencyIncludeDirs, true );
    MergeStringSet( runtimeDependencies, moduleInfoToMergeIn->runtimeDependencies, true );

    totalSuccess = metadata.Merge( moduleInfoToMergeIn->metadata, onConflictOriginalInfoStays ) && totalSuccess;
    totalSuccess = linkerSettings.Merge( moduleInfoToMergeIn->linkerSettings, onConflictOriginalInfoStays ) && totalSuccess;
    totalSuccess = compilerSettings.Merge( moduleInfoToMergeIn->compilerSettings, onConflictOriginalInfoStays ) && totalSuccess;
    totalSuccess = preprocessorSettings.Merge( moduleInfoToMergeIn->preprocessorSettings, onConflictOriginalInfoStays ) && totalSuccess;

    if ( -1 == buildOrder )
        buildOrder = moduleInfoToMergeIn->buildOrder;
    else
    if ( !onConflictOriginalInfoStays && -1 != moduleInfoToMergeIn->buildOrder )
        buildOrder = moduleInfoToMergeIn->buildOrder;

    //if ( -1 == buildChain )
    //    buildChain = moduleInfoToMergeIn->buildChain;
    //else
    //if ( !onConflictOriginalInfoStays && -1 != moduleInfoToMergeIn->buildChain )
    //    buildChain = moduleInfoToMergeIn->buildChain;

    //buildChainDependencies.insert( moduleInfoToMergeIn->buildChainDependencies.begin(), moduleInfoToMergeIn->buildChainDependencies.end() );

    if ( !hasConsiderSubDirs )
    {
        considerSubDirs = moduleInfoToMergeIn->considerSubDirs;
        hasConsiderSubDirs = moduleInfoToMergeIn->hasConsiderSubDirs;
    }
    else
    if ( !onConflictOriginalInfoStays && moduleInfoToMergeIn->hasConsiderSubDirs )
    {
        considerSubDirs = moduleInfoToMergeIn->considerSubDirs;
        hasConsiderSubDirs = moduleInfoToMergeIn->hasConsiderSubDirs;
    }

    if ( !hasIgnoreModule )
    {
        ignoreModule = moduleInfoToMergeIn->ignoreModule;
        hasIgnoreModule = moduleInfoToMergeIn->hasIgnoreModule;
    }
    else
    if ( !onConflictOriginalInfoStays && moduleInfoToMergeIn->hasIgnoreModule )
    {
        ignoreModule = moduleInfoToMergeIn->ignoreModule;
        hasIgnoreModule = moduleInfoToMergeIn->hasIgnoreModule;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::SetPlatformName( const CORE::CString& platformName )
{GUCEF_TRACE;

    m_platformName = platformName;
}

/*---------------------------------------------------------------------------*/

const CORE::CString&
CModuleInfo::GetPlatformName( void ) const
{GUCEF_TRACE;

    return m_platformName;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddIncludeDir( const CORE::CString& pathToIncludeDir )
{GUCEF_TRACE;

    m_includeDirs[ pathToIncludeDir ];
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddIncludeDirs( const CORE::CStringSet& pathsToIncludeDirs )
{GUCEF_TRACE;

    CORE::CStringSet::const_iterator i = pathsToIncludeDirs.begin();
    while ( i != pathsToIncludeDirs.end() )
    {
        const CORE::CString& pathToIncludeDir = (*i);
        m_includeDirs[ pathToIncludeDir ];
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfo::RemoveIncludeDir( const CORE::CString& pathToIncludeDir ,
                               bool mustBeEmpty                      )
{GUCEF_TRACE;

    if ( mustBeEmpty )
    {
        TStringSetMap::iterator i = m_includeDirs.find( pathToIncludeDir );
        if ( i != m_includeDirs.end() )
        {
            // it exists thus it must be empty
            if ( (*i).second.empty() )
            {
                m_includeDirs.erase( i );
                return true;
            }
            return false;
        }
        return true;
    }
    else
    {
        m_includeDirs.erase( pathToIncludeDir );
        return true;
    }
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::SetIncludeFiles( const TStringSetMap& files )
{GUCEF_TRACE;

    m_includeDirs = files;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::SetIncludeFiles( const CORE::CString& pathToIncludeFiles ,
                              const TStringSet& files                 )
{GUCEF_TRACE;

    m_includeDirs[ pathToIncludeFiles ] = files;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddIncludeFiles( const CORE::CString& pathToIncludeFiles ,
                              const TStringSet& files                 )
{GUCEF_TRACE;

    TStringSet& preExisting = m_includeDirs[ pathToIncludeFiles ];
    MergeStringSet( preExisting, files, true );
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddIncludeFiles( const TStringSetMap& files )
{GUCEF_TRACE;

    MergeStringSetMap( m_includeDirs, files, false );
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddIncludeFile( const CORE::CString& pathToFiles ,
                             const CORE::CString& filename    )
{GUCEF_TRACE;

    m_includeDirs[ pathToFiles ].insert( filename );
}

/*---------------------------------------------------------------------------*/

const TStringSetMap&
CModuleInfo::GetIncludeDirs( void ) const
{GUCEF_TRACE;

    return m_includeDirs;
}

/*---------------------------------------------------------------------------*/

const TStringSet&
CModuleInfo::GetIncludeFiles( const CORE::CString& pathToFiles ) const
{GUCEF_TRACE;

    TStringSetMap::const_iterator i = m_includeDirs.find( pathToFiles );
    if ( i != m_includeDirs.end() )
    {
        return (*i).second;
    }
    return CORE::CString::EmptyStringSet;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::SetSourceFiles( const TStringSetMap& files )
{GUCEF_TRACE;

    m_sourceDirs = files;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::SetSourceFiles( const CORE::CString& pathToFiles ,
                             const TStringSet& files          )
{GUCEF_TRACE;

    m_sourceDirs[ pathToFiles ] = files;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddSourceFiles( const CORE::CString& pathToFiles ,
                             const TStringSet& files          )
{GUCEF_TRACE;

    TStringSet& preExisting = m_sourceDirs[ pathToFiles ];
    MergeStringSet( preExisting, files, false );
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddSourceFiles( const TStringSetMap& files )
{GUCEF_TRACE;

    MergeStringSetMap( m_sourceDirs, files, false );
}

/*---------------------------------------------------------------------------*/

const TStringSetMap&
CModuleInfo::GetSourceDirs( void ) const
{GUCEF_TRACE;

    return m_sourceDirs;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddSourceFile( const CORE::CString& pathToFiles ,
                            const CORE::CString& filename    )
{GUCEF_TRACE;

    m_sourceDirs[ pathToFiles ].insert( filename );
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::SetNamesOfDependencies( const TStringSet& dependencies )
{GUCEF_TRACE;

    m_namesOfDependencies = dependencies;
}

/*---------------------------------------------------------------------------*/

const TStringSet&
CModuleInfo::GetNamesOfDependencies( void ) const
{GUCEF_TRACE;

    return m_namesOfDependencies;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddNameOfDependency( const CORE::CString& dependency )
{GUCEF_TRACE;

    m_namesOfDependencies.insert( dependency );
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::RemoveNameOfDependency( const CORE::CString& dependency )
{GUCEF_TRACE;

    m_namesOfDependencies.erase( dependency );
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfo::HasDependencyWithName( const CORE::CString& dependency ) const
{GUCEF_TRACE;

    return m_namesOfDependencies.find( dependency ) != m_namesOfDependencies.end();
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::MergeNamesOfDependencies( const TStringSet& dependenciesToMergeIn )
{GUCEF_TRACE;

    MergeStringSet( m_namesOfDependencies, dependenciesToMergeIn, false );
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfo::MoveDependencyToLogicalDependencies( const CORE::CString& dependency )
{GUCEF_TRACE;

    TStringSet::iterator i = m_namesOfDependencies.find( dependency );
    if ( i != m_namesOfDependencies.end() )
    {
        m_namesOfDependencies.erase( dependency );
        m_namesOfLogicalDependencies.insert( dependency );
        return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

const TStringSet&
CModuleInfo::GetNamesOfLogicalDependencies( void ) const
{GUCEF_TRACE;

    return m_namesOfLogicalDependencies;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddNameOfRuntimeDependency( const CORE::CString& dependency )
{GUCEF_TRACE;

    runtimeDependencies.insert( dependency );
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::RemoveNameOfRuntimeDependency( const CORE::CString& dependency )
{GUCEF_TRACE;

    runtimeDependencies.erase( dependency );
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddDependencyIncludeDirs( const CORE::CStringSet& pathsToIncludeDirs )
{GUCEF_TRACE;

    dependencyIncludeDirs.insert( pathsToIncludeDirs.begin(), pathsToIncludeDirs.end() );
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfo::HasIndependentModuleType( void ) const
{GUCEF_TRACE;

    switch ( moduleType )
    {
        case MODULETYPE_HEADER_INCLUDE_LOCATION:
        case MODULETYPE_HEADER_INTEGRATE_LOCATION:
        case MODULETYPE_CODE_INTEGRATE_LOCATION:
        //case MODULETYPE_BINARY_PACKAGE:
        case MODULETYPE_UNDEFINED:
        case MODULETYPE_UNKNOWN:
        {
            return false;
        }
        default:
        {
            return true;
        }
    }
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfo::HasValidModuleType( void ) const
{GUCEF_TRACE;

    switch ( moduleType )
    {
        case MODULETYPE_UNDEFINED:
        case MODULETYPE_UNKNOWN:
        {
            return false;
        }
        default:
        {
            return true;
        }
    }
}

/*---------------------------------------------------------------------------*/

const CORE::CString CModuleInfoEntry::ClassTypeName = "GUCEF::PROJECTGEN::CModuleInfoEntry";

/*---------------------------------------------------------------------------*/

CModuleInfoEntry::CModuleInfoEntry( void ) 
    : CORE::CIDataNodeSerializable()
    , CORE::CTSharedObjCreator< CModuleInfoEntry, MT::CMutex >( this )
    , rootDir()
    , m_consensusName()
    , m_modulesPerPlatform()
    , m_flattenedInfoPerPlatform()
    , m_isBroken( false )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CModuleInfoEntry::CModuleInfoEntry( const CModuleInfoEntry& src ) 
    : CORE::CIDataNodeSerializable( src )
    , CORE::CTSharedObjCreator< CModuleInfoEntry, MT::CMutex >( this )
    , rootDir( src.rootDir )
    , m_consensusName( src.m_consensusName )
    , m_modulesPerPlatform( src.m_modulesPerPlatform )
    , m_flattenedInfoPerPlatform( src.m_flattenedInfoPerPlatform )
    , m_isBroken( src.m_isBroken )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CModuleInfoEntry::~CModuleInfoEntry() 
{GUCEF_TRACE;

    Clear();
}

/*---------------------------------------------------------------------------*/

void
CModuleInfoEntry::Clear( void ) 
{GUCEF_TRACE;
    
    rootDir.Clear();

    m_consensusName.Clear();
    m_modulesPerPlatform.clear();
    m_flattenedInfoPerPlatform.clear();

    m_isBroken = false;
}

/*---------------------------------------------------------------------------*/

const CORE::CString&
CModuleInfoEntry::GetConsensusName( CModuleInfoPtr* moduleInfo ,
                                    bool dontUseCached         ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != moduleInfo )
        *moduleInfo = GUCEF_NULL;

    if ( !dontUseCached && !m_consensusName.IsNULLOrEmpty() )
        return m_consensusName;

    TModuleInfoPtrMap::const_iterator n = m_modulesPerPlatform.find( AllPlatforms );
    if ( n != m_modulesPerPlatform.end() )
    {
        // A name was specified for all platforms which makes our job easy
        // an all platform name always counts as the general consensus name
        if ( !(*n).second->name.IsNULLOrEmpty() )
        {
            if ( GUCEF_NULL != moduleInfo )
                *moduleInfo = (*n).second;
            return m_consensusName = (*n).second->name;
        }
    }

    // If no name is specified for all platforms then we will have to
    // determine the best name to use. We do this by getting the name
    // for all platforms and counting how often each is used. The most used
    // name is considered the general consensus name. If the same count applies
    // to multiple we will try to use a popular platform to improve our 'guess'

    typedef std::map< CORE::CString, CORE::UInt32 > TStringCountMap;

    TStringCountMap countMap;
    n = m_modulesPerPlatform.begin();
    while ( n != m_modulesPerPlatform.end() )
    {
        const CModuleInfoPtr& mInfo = (*n).second;
        if ( !mInfo->name.IsNULLOrEmpty() )
        {
            TStringCountMap::iterator m = countMap.find( mInfo->name );
            if ( m != countMap.end() )
            {
                ++((*m).second);
            }
            else
            {
                countMap[ mInfo->name ] = 1;
            }
        }
        ++n;
    }

    if ( countMap.empty() )
    {
        // This should not happen:
        // Given that we have no module definitions for ANY platform and yet have a higher level concept of it...
        // We promised we would always return a module name. The only thing we have at this point is the directory path
        // As such we will assume the dir is named after the module it houses as our last ditch guess
        return m_consensusName = CORE::LastSubDir( rootDir );
    }

    // Now that we have the popularity count of each name get the highest count
    CORE::UInt32 highestCount = 0;
    TStringCountMap::iterator i = countMap.begin();
    while ( i != countMap.end() )
    {
        if ( highestCount < (*i).second )
        {
            highestCount = (*i).second;
        }
        ++i;
    }

    // Make the list of most popular names
    TStringSet topNames;
    i = countMap.begin();
    while ( i != countMap.end() )
    {
        if ( highestCount == (*i).second )
        {
            topNames.insert( (*i).first );
        }
        ++i;
    }

    // If we have multiple use a popular platform if
    // possible, otherwise just grab one

    //@TODO: popular platform check

    CString consensusName = (*topNames.begin());
    if ( NULL != moduleInfo )
    {
        // Now turn the consensus name back into a module pointer
        n = m_modulesPerPlatform.begin();
        while ( n != m_modulesPerPlatform.end() )
        {
            if ( (*n).second->name == consensusName )
            {
                *moduleInfo = (*n).second;
                break;
            }
            ++n;
        }
    }
    return m_consensusName = consensusName;
}

/*---------------------------------------------------------------------------*/

const CORE::CString*
CModuleInfoEntry::GetModuleName( const CORE::CString& targetPlatform ,
                                 CModuleInfoPtr* outModuleInfo       ) const
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator n = m_modulesPerPlatform.find( targetPlatform );
    if ( n != m_modulesPerPlatform.end() )
    {
        // A module was specified for this platform
        // Just because we have a module definition doens't mean we have a name
        if ( !(*n).second->name.IsNULLOrEmpty() )
        {
            // We have a name for this specific platform
            if ( GUCEF_NULL != outModuleInfo )
            {
                *outModuleInfo = (*n).second;
            }
            return &( (*n).second->name );
        }
    }

    // If no name is specified for a specific platform then there might still be a
    // default for all platforms
    if ( targetPlatform != AllPlatforms )
    {
        n = m_modulesPerPlatform.find( AllPlatforms );
        if ( n != m_modulesPerPlatform.end() )
        {
            // An 'AllPlatforms' definition is available for this module
            // Just because we have a module definition doesn't mean we have a name
            if ( !(*n).second->name.IsNULLOrEmpty() )
            {
                // We have a name for this specific platform
                if ( GUCEF_NULL != outModuleInfo )
                {
                    *outModuleInfo = (*n).second;
                }
                return &( (*n).second->name );
            }
        }
    }
    return GUCEF_NULL;
}

/*---------------------------------------------------------------------------*/

CORE::CString
CModuleInfoEntry::GetModuleNameAlways( const CORE::CString& targetPlatform ,
                                       CModuleInfoPtr* moduleInfo          ) const
{GUCEF_TRACE;

    const CORE::CString* strPtr = GetModuleName( targetPlatform, moduleInfo );
    if ( GUCEF_NULL == strPtr )
    {
        return GetConsensusName( moduleInfo );
    }
    return *strPtr;
}

/*---------------------------------------------------------------------------*/

CModuleInfoEntry&
CModuleInfoEntry::operator=( const CModuleInfoEntry& src ) 
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_modulesPerPlatform = src.m_modulesPerPlatform;
        rootDir = src.rootDir;

        m_consensusName = src.m_consensusName;
    }
    return *this;
}

/*---------------------------------------------------------------------------*/
               
bool 
CModuleInfoEntry::Serialize( CORE::CDataNode& domRootNode                        ,
                             const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    
    domRootNode.SetAttribute( "RootDir", rootDir );

    TModuleInfoPtrMap::const_iterator i = m_modulesPerPlatform.begin();
    while ( i != m_modulesPerPlatform.end() )
    {
        const CORE::CString& platform = (*i).first;
        CModuleInfoPtr moduleInfo = (*i).second;

        if ( !SerializeModuleInfo( CreateBasicSharedPtr() ,
                                   moduleInfo             ,
                                   platform               ,
                                   domRootNode            ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "SerializeModuleInfo: Failed to serialize moduleInfo of module " + moduleInfo->name + " for platform " + platform );
            totalSuccess = false;
        }

        ++i;
    }
    
    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfoEntry::SetModuleInfo( CModuleInfoPtr moduleInfo     ,
                                 const CORE::CString& platform )
{GUCEF_TRACE;

    // First check if we already have a entry for this platform
    TModuleInfoPtrMap::iterator i = m_modulesPerPlatform.find( platform );
    if ( i != m_modulesPerPlatform.end() )
    {
        // Since we already have an entry for this platform we will merge the two
        CModuleInfoPtr preExistingModuleInfo = (*i).second;
        preExistingModuleInfo->Merge( moduleInfo );
    }
    else
    {
        m_modulesPerPlatform[ platform ] = moduleInfo;
        moduleInfo->SetPlatformName( platform );
    }
}

/*-------------------------------------------------------------------------*/

CModuleInfoPtr
CModuleInfoEntry::FindOrCreateModuleInfoForPlatform( const CORE::CString& platform ,
                                                     bool createNewIfNoneExists    )
{GUCEF_TRACE;

    TModuleInfoPtrMap::iterator i = m_modulesPerPlatform.find( platform.Lowercase() );
    if ( i != m_modulesPerPlatform.end() )
    {
        return (*i).second;
    }
    if ( createNewIfNoneExists )
    {
        CModuleInfoPtr moduleInfo = CModuleInfo::CreateSharedObj();
        moduleInfo->SetPlatformName( platform );
        m_modulesPerPlatform[ platform ] = moduleInfo;
        return moduleInfo;
    }
    return CModuleInfoPtr();
}

/*-------------------------------------------------------------------------*/

const CModuleInfoPtr
CModuleInfoEntry::FindModuleInfoForPlatform( const CORE::CString& platform      ,
                                             bool considerAllPlatformAsFallback ) const
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator i = m_modulesPerPlatform.find( platform.Lowercase() );
    if ( i != m_modulesPerPlatform.end() )
    {
        return (*i).second;
    }

    if ( considerAllPlatformAsFallback && ( platform != AllPlatforms && !platform.IsNULLOrEmpty() ) )
    {
        TModuleInfoPtrMap::const_iterator i = m_modulesPerPlatform.find( AllPlatforms );
        if ( i != m_modulesPerPlatform.end() )
        {
            return (*i).second;
        }
    }
    return CModuleInfoPtr();
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::FindAllModuleInfoWithModuleType( TModuleType moduleType       ,
                                                   TModuleInfoPtrSet& foundInfo ) const
{GUCEF_TRACE;

    // We may not have a platform specific module type defined (fairly typical)
    CModuleInfoPtr allPlatformsDef;
    TModuleInfoPtrMap::const_iterator i = m_modulesPerPlatform.find( AllPlatforms );
    if ( i != m_modulesPerPlatform.end() )
    {
        const CModuleInfoPtr& info = (*i).second;
        if GUCEF_PREDICT_TRUE( !info.IsNULL() )
        {
            if ( info->moduleType == moduleType )
            {
                allPlatformsDef = info;
            }
        }
    }

    // no luck, try the other platforms, if any
    i = m_modulesPerPlatform.begin();
    while ( i != m_modulesPerPlatform.end() )
    {
        const CORE::CString& targetPlatform = (*i).first;
        if ( AllPlatforms != targetPlatform )
        {
            const CModuleInfoPtr& info = (*i).second;
            if GUCEF_PREDICT_TRUE( !info.IsNULL() )
            {
                TModuleType platformModuleType = info->moduleType;
                if ( MODULETYPE_UNDEFINED == platformModuleType )
                {
                    // When undefined the 'all' platforms value overules
                    if ( !allPlatformsDef.IsNULL() )
                    {
                        // we count this one because the 'all' platform has the module type desired and
                        // we have a platform specific definition but it doesnt override the module type as defined by
                        // the 'all' platform definition hence it logically has the type we are looking for
                        foundInfo.insert( info );
                    }
                }
                else
                if ( platformModuleType == moduleType )
                {
                    foundInfo.insert( info );
                }
            }
        }
        ++i;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::HasAnyModuleInfoWithModuleType( TModuleType moduleType ) const
{GUCEF_TRACE;

    TModuleInfoPtrSet foundInfo;
    FindAllModuleInfoWithModuleType( moduleType, foundInfo );
    return !foundInfo.empty();
}

/*-------------------------------------------------------------------------*/

CModuleInfoPtr
CModuleInfoEntry::FindModuleInfoWithModuleType( TModuleType moduleType             ,
                                                const CORE::CString& platform      ,
                                                bool considerAllPlatformAsFallback ) const
{GUCEF_TRACE;

    // Considering a specific platform is given, try that one first
    TModuleInfoPtrMap::const_iterator i = m_modulesPerPlatform.find( platform );
    if ( i != m_modulesPerPlatform.end() )
    {
        const CModuleInfoPtr& info = (*i).second;
        if GUCEF_PREDICT_TRUE( !info.IsNULL() )
        {
            if ( info->moduleType == moduleType )
            {
                return info;
            }
        }
    }

    // no luck, check if we can try the 'all' platform
    if ( considerAllPlatformAsFallback && ( platform != AllPlatforms && !platform.IsNULLOrEmpty() ) )
    {
        i = m_modulesPerPlatform.find( AllPlatforms );
        if ( i != m_modulesPerPlatform.end() )
        {
            const CModuleInfoPtr& info = (*i).second;
            if GUCEF_PREDICT_TRUE( !info.IsNULL() )
            {
                if ( info->moduleType == moduleType )
                {
                    return info;
                }
            }
        }
    }
    return CModuleInfoPtr();
}

/*-------------------------------------------------------------------------*/

const TModuleInfoPtrMap&
CModuleInfoEntry::GetModulesPerPlatform( void ) const
{GUCEF_TRACE;

    return m_modulesPerPlatform;
}

/*-------------------------------------------------------------------------*/

const TModuleInfoPtrMap&
CModuleInfoEntry::GetFlattenedModulesPerPlatform( void ) const
{GUCEF_TRACE;

    return m_flattenedInfoPerPlatform;
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::HasDependency( const CORE::CString& platform       ,
                                 const CORE::CString& dependencyName ,
                                 bool platformSpecificOnly           ) const
{GUCEF_TRACE;

    // Check for a platform specific dependency
    TModuleInfoPtrMap::const_iterator n = m_modulesPerPlatform.find( platform );
    if ( n != m_modulesPerPlatform.end() )
    {
        // This module has info for for this module, check its type
        const CModuleInfoPtr& moduleInfo = (*n).second;
        if ( IsStringInList( moduleInfo->GetNamesOfDependencies(), false, dependencyName ) )
        {
            return true;
        }
    }

    // Alternatively check the 'all platforms' definition
    if ( !platformSpecificOnly && platform != AllPlatforms && !platform.IsNULLOrEmpty() )
    {
        n = m_modulesPerPlatform.find( AllPlatforms );
        if ( n != m_modulesPerPlatform.end() )
        {
            // This module has info for for this module, check its type
            const CModuleInfoPtr& moduleInfo = (*n).second;
            if ( IsStringInList( moduleInfo->GetNamesOfDependencies(), false, dependencyName ) )
            {
                return true;
            }
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::HasLinkerDependency( const CORE::CString& platform       ,
                                       const CORE::CString& dependencyName ,
                                       bool platformSpecificOnly           ) const
{GUCEF_TRACE;

    // Check for a platform specific dependency
    TModuleInfoPtrMap::const_iterator n = m_modulesPerPlatform.find( platform );
    if ( n != m_modulesPerPlatform.end() )
    {
        // This module has info for for this module, check its type
        const CModuleInfoPtr& moduleInfo = (*n).second;
        if ( moduleInfo->linkerSettings.HasLinkerDependency( dependencyName ) )
        {
            return true;
        }
    }

    // Alternatively check the 'all platforms' definition
    if ( !platformSpecificOnly && platform != AllPlatforms && !platform.IsNULLOrEmpty() )
    {
        n = m_modulesPerPlatform.find( AllPlatforms );
        if ( n != m_modulesPerPlatform.end() )
        {
            // This module has info for for this module, check its type
            const CModuleInfoPtr& moduleInfo = (*n).second;
            if ( moduleInfo->linkerSettings.HasLinkerDependency( dependencyName ) )
            {
                return true;
            }
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::IsApplicableForPlatform( const CORE::CString& platform ) const
{GUCEF_TRACE;

    // Check for a platform specific definition
    TModuleInfoPtrMap::const_iterator n = m_modulesPerPlatform.find( platform );
    if ( n != m_modulesPerPlatform.end() )
    {
        // This module has info for for this module, check its type
        const CModuleInfoPtr& moduleInfo = (*n).second;
        if GUCEF_PREDICT_TRUE( !moduleInfo.IsNULL() )
        {
            if ( moduleInfo->HasValidModuleType() )
            {
                return true;
            }
        }
    }

    // Alternatively check the 'all platforms' definition
    if ( platform != AllPlatforms && !platform.IsNULLOrEmpty() )
    {
        n = m_modulesPerPlatform.find( AllPlatforms );
        if ( n != m_modulesPerPlatform.end() )
        {
            // This module has info for for this module, check its type
            const CModuleInfoPtr& moduleInfo = (*n).second;
            if GUCEF_PREDICT_TRUE( !moduleInfo.IsNULL() )
            {
                if ( moduleInfo->HasValidModuleType() )
                {
                    return true;
                }
            }
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::HasIndependentModuleTypeForPlatform( const CORE::CString& platform ) const
{GUCEF_TRACE;

    // Check for a platform specific definition
    TModuleInfoPtrMap::const_iterator n = m_modulesPerPlatform.find( platform );
    if ( n != m_modulesPerPlatform.end() )
    {
        // This module has info for for this module, check its type
        const CModuleInfoPtr& moduleInfo = (*n).second;
        if GUCEF_PREDICT_TRUE( !moduleInfo.IsNULL() )
        {
            if ( moduleInfo->HasIndependentModuleType() )
            {
                return true;
            }
        }
    }

    // Alternatively check the 'all platforms' definition
    if ( platform != AllPlatforms && !platform.IsNULLOrEmpty() )
    {
        n = m_modulesPerPlatform.find( AllPlatforms );
        if ( n != m_modulesPerPlatform.end() )
        {
            // This module has info for for this module, check its type
            const CModuleInfoPtr& moduleInfo = (*n).second;
            if GUCEF_PREDICT_TRUE( !moduleInfo.IsNULL() )
            {
                if ( moduleInfo->HasIndependentModuleType() )
                {
                    return true;
                }
            }
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::HasAllPlatformsDefinition( void ) const
{GUCEF_TRACE;

    // Check for the AllPlatforms definition
    TModuleInfoPtrMap::const_iterator n = m_modulesPerPlatform.find( AllPlatforms );
    if ( n != m_modulesPerPlatform.end() )
    {
        // Check if its not just a placeholder, it has to be valid
        const CModuleInfoPtr& allPlatformsDefinition = (*n).second;
        if ( !allPlatformsDefinition.IsNULL() && allPlatformsDefinition->HasValidModuleType() )
        {
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CModuleInfoEntry::SetIsBroken( bool isBroken )
{GUCEF_TRACE;

    m_isBroken = isBroken;
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::IsBroken( void ) const
{GUCEF_TRACE;

    return m_isBroken;
}

/*-------------------------------------------------------------------------*/

void
CModuleInfoEntry::GetModuleDependencyNames( const CORE::CString& targetPlatform ,
                                            TStringSet& dependencies            ) const
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindModuleInfoForPlatform( targetPlatform );
    if ( !moduleInfo.IsNULL() )
    {
        MergeStringSet( dependencies, moduleInfo->GetNamesOfDependencies(), false );
    }
    if ( targetPlatform != AllPlatforms && !targetPlatform.IsNULLOrEmpty() )
    {
        moduleInfo = FindModuleInfoForPlatform( AllPlatforms );
        if ( !moduleInfo.IsNULL() )
        {
            MergeStringSet( dependencies, moduleInfo->GetNamesOfDependencies(), false );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CModuleInfoEntry::GetModuleLinkerDependencyNames( const CORE::CString& targetPlatform ,
                                                  TStringSet& dependencies            ) const
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindModuleInfoForPlatform( targetPlatform );
    if ( !moduleInfo.IsNULL() )
    {
        moduleInfo->linkerSettings.GetListOfLinkedLibraries( dependencies );
    }
    if ( targetPlatform != AllPlatforms && !targetPlatform.IsNULLOrEmpty() )
    {
        moduleInfo = FindModuleInfoForPlatform( AllPlatforms );
        if ( !moduleInfo.IsNULL() )
        {
            moduleInfo->linkerSettings.GetListOfLinkedLibraries( dependencies );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CModuleInfoEntry::GetModuleRuntimeDependencyNames( const CORE::CString& targetPlatform ,
                                                   TStringSet& dependencies            ) const
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindModuleInfoForPlatform( targetPlatform );
    if ( !moduleInfo.IsNULL() )
    {
        MergeStringSet( dependencies, moduleInfo->runtimeDependencies, false );
    }
    if ( targetPlatform != AllPlatforms && !targetPlatform.IsNULLOrEmpty() )
    {
        moduleInfo = FindModuleInfoForPlatform( AllPlatforms );
        if ( !moduleInfo.IsNULL() )
        {
            MergeStringSet( dependencies, moduleInfo->runtimeDependencies, false );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CModuleInfoEntry::RemoveRuntimeDependency( const CORE::CString& dependencyName ,
                                           const CORE::CString& platformName   )
{GUCEF_TRACE;

    // Check for platform specific definitions
    TModuleInfoPtrMap::iterator n = m_modulesPerPlatform.find( platformName );
    if ( n != m_modulesPerPlatform.end() )
    {
        // This module has info for for this module, check its type
        CModuleInfoPtr& platformModuleDefinition = (*n).second;
        if ( !platformModuleDefinition.IsNULL() )
        {
            platformModuleDefinition->RemoveNameOfRuntimeDependency( dependencyName );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CModuleInfoEntry::RemoveRuntimeDependency( const CORE::CString& dependencyName )
{GUCEF_TRACE;

    // Check for platform specific definitions
    TModuleInfoPtrMap::iterator n = m_modulesPerPlatform.begin();
    while ( n != m_modulesPerPlatform.end() )
    {
        // This module has info for for this module, check its type
        const CORE::CString& platform = (*n).first;
        CModuleInfoPtr& platformModuleDefinition = (*n).second;
        if ( !platformModuleDefinition.IsNULL() )
        {
            platformModuleDefinition->RemoveNameOfRuntimeDependency( dependencyName );
        }
        
        ++n;
    }
}

/*-------------------------------------------------------------------------*/

void
CModuleInfoEntry::GetReferencedPlatforms( TStringSet& platforms ) const
{GUCEF_TRACE;

    // Check for platform specific definitions
    TModuleInfoPtrMap::const_iterator n = m_modulesPerPlatform.begin();
    while ( n != m_modulesPerPlatform.end() )
    {
        // This module has info for for this module, check its type
        const CORE::CString& platform = (*n).first; 
        platforms.insert( platform );
        ++n;
    }
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfoEntry::Deserialize( const CORE::CDataNode& domRootNode                  ,
                               const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    if ( rootDir.IsNULLOrEmpty() )
    {
        rootDir = domRootNode.GetAttributeValue( "RootDir" );
    }

    const CORE::CDataNode::TConstDataNodeSet moduleInfoNodes = domRootNode.FindChildrenOfType( "Module" );
    if ( moduleInfoNodes.size() == 0 ) 
        return false;

    CORE::CDataNode::TConstDataNodeSet::const_iterator n = moduleInfoNodes.begin();
    while ( n != moduleInfoNodes.end() )
    {
        CModuleInfoPtr moduleInfoForPlatform = CModuleInfo::CreateSharedObj();
        const CORE::CDataNode* moduleNode = (*n);

        if ( DeserializeModuleInfo( moduleInfoForPlatform ,
                                    *moduleNode           ) )
        {
            if ( moduleInfoForPlatform->name.IsNULLOrEmpty() )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully deserialized module definition for module with no name specified for the applicable platforms" );
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully deserialized module definition for module with name " + moduleInfoForPlatform->name );
            }

            // Get all platforms for which this info applies.
            // Keep in mind that multiple platforms can be specified for ease of use.
            // This feature requires platform entries to be separated by a ';'
            TStringVector platforms = moduleNode->GetAttributeValue( "Platform" ).AsString().Lowercase().ParseElements( ';', false);

            if ( platforms.empty() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Failed to locate a Platform value for a module, will default to all platforms but this may not be correct" );
                platforms.push_back( AllPlatforms );
            }

            TStringVector::iterator i = platforms.begin();
            while ( i != platforms.end() )
            {
                // @TODO: find a cleaner way to resolve this vs userdata
                CProjectInfo* projectInfo = static_cast<CProjectInfo*>( settings.GetUserData() );  
                if ( GUCEF_NULL == projectInfo )
                    return false;

                const CORE::CString& multiPlatformName = (*i); 

                TStringSet actualPlatforms = ResolveMultiPlatformName( multiPlatformName, &projectInfo->platforms );
                TStringSet::iterator n = actualPlatforms.begin();
                while ( n != actualPlatforms.end() )
                {
                    const CORE::CString& actualPlatformName = (*n);

                    if ( multiPlatformName != actualPlatformName )
                    {
                        CModuleInfoPtr actualModuleInfoForPlatform = CModuleInfo::CreateSharedObjWithParam( *moduleInfoForPlatform );
                        actualModuleInfoForPlatform->SetPlatformName( actualPlatformName );
                        SetModuleInfo( actualModuleInfoForPlatform, actualPlatformName );
                    }
                    else
                    {
                        SetModuleInfo( moduleInfoForPlatform, actualPlatformName );
                    }
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Adding module definition for platform " + actualPlatformName );
                    
                    ++n;
                }                

                ++i;
            }
        }
        else
        {
            return false;
        }
        ++n;
    }

    return true;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfoEntry::Merge( const CModuleInfoEntryPtr& infoToMergeIn ,
                         bool onConflictOriginalInfoStays         ) 
{GUCEF_TRACE;

    bool totalSuccess = true;

    if ( rootDir.IsNULLOrEmpty() )
        rootDir = infoToMergeIn->rootDir;
    else
    if ( !onConflictOriginalInfoStays && !infoToMergeIn->rootDir.IsNULLOrEmpty() )
        rootDir = infoToMergeIn->rootDir;

    TModuleInfoPtrMap::const_iterator i = infoToMergeIn->m_modulesPerPlatform.begin();
    while ( i != infoToMergeIn->m_modulesPerPlatform.end() )
    {
        const CString& platformToMergeIn = (*i).first;
        const CModuleInfoPtr& moduleInfoToMergeIn = (*i).second;

        TModuleInfoPtrMap::iterator e = m_modulesPerPlatform.find( platformToMergeIn );
        if ( e != m_modulesPerPlatform.end() )
        {
            // We already have info for this platform
            // we need to merge
            CModuleInfoPtr& moduleInfo = (*e).second;
            totalSuccess = moduleInfo->Merge( moduleInfoToMergeIn ) && totalSuccess;

            ++e;
        }
        else
        {
            // No entry exists yet for this platform so we can just do a simple insert
            m_modulesPerPlatform[ platformToMergeIn ] = moduleInfoToMergeIn;
        }

        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfoEntry::GetIncludeFilesForPlatform( const CORE::CString& platformName ,
                                              TStringSetMap& files              ,
                                              bool autoConsiderAllPlatforms     ) const
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindModuleInfoForPlatform( platformName );
    if ( !moduleInfo.IsNULL() )
    {
        MergeStringSetMap( files, moduleInfo->GetIncludeDirs(), false );
    }
    if ( autoConsiderAllPlatforms && ( platformName != AllPlatforms && !platformName.IsNULLOrEmpty() ) )
    {
        moduleInfo = FindModuleInfoForPlatform( AllPlatforms );
        if ( !moduleInfo.IsNULL() )
        {
            MergeStringSetMap( files, moduleInfo->GetIncludeDirs(), false );
        }
    }
}

/*---------------------------------------------------------------------------*/

void
CModuleInfoEntry::GetIncludeDirsForPlatform( const CORE::CString& platformName ,
                                             TStringSet& subDirPaths           ,
                                             bool autoConsiderAllPlatforms     ) const
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindModuleInfoForPlatform( platformName );
    if ( !moduleInfo.IsNULL() )
    {
        const TStringSetMap& includes = moduleInfo->GetIncludeDirs();
        TStringSetMap::const_iterator i = includes.begin();
        while ( i != includes.end() )
        {
            const CORE::CString& subPath = (*i).first;
            subDirPaths.insert( subPath );
            ++i;
        }
    }
    if ( autoConsiderAllPlatforms && ( platformName != AllPlatforms && !platformName.IsNULLOrEmpty() ) )
    {
        moduleInfo = FindModuleInfoForPlatform( AllPlatforms );
        if ( !moduleInfo.IsNULL() )
        {
            const TStringSetMap& includes = moduleInfo->GetIncludeDirs();
            TStringSetMap::const_iterator i = includes.begin();
            while ( i != includes.end() )
            {
                const CORE::CString& subPath = (*i).first;
                subDirPaths.insert( subPath );
                ++i;
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

void
CModuleInfoEntry::GetPathsToIncludeDirsForPlatform( const CORE::CString& platformName  ,
                                                    const CORE::CString& otherFromPath ,
                                                    TStringSet& pathsToSubDirPaths     ,
                                                    bool autoConsiderAllPlatforms      ) const
{GUCEF_TRACE;

    TStringSet localSubDirPaths;
    GetIncludeDirsForPlatform( platformName, localSubDirPaths, autoConsiderAllPlatforms );

    TStringSet::iterator i = localSubDirPaths.begin();
    while ( i != localSubDirPaths.end() )
    {
        const CORE::CString& subPath = (*i);
        CORE::CString fullLocalPath = CORE::CombinePath( rootDir, subPath );
        CORE::CString relativePath = CORE::GetRelativePathToOtherPathRoot( otherFromPath, fullLocalPath, true ); 

        pathsToSubDirPaths.insert( relativePath );

        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
CModuleInfoEntry::CleanupIncludeDirs( void )
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator i = m_modulesPerPlatform.begin();
    while ( i != m_modulesPerPlatform.end() )
    {
        CModuleInfoPtr moduleInfo = (*i).second;
        
        // Check for empty include dirs
        // If the include dir does not have include files as part of this module then 
        // the dir should have been a dependency include dir
        TStringSet dirs;
        TStringSetMap::const_iterator n = moduleInfo->GetIncludeDirs().begin();
        while ( n != moduleInfo->GetIncludeDirs().end() )
        {
            const TStringSet& filesInDirList = (*n).second;
            if ( filesInDirList.empty() )
            {
                dirs.insert( (*n).first );
            }
            ++n;
        }

        TStringSet::iterator m = dirs.begin();
        while ( m != dirs.end() )
        {
            moduleInfo->RemoveIncludeDir( (*m), true ); 
            moduleInfo->dependencyIncludeDirs.insert( (*m) );

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ModuleInfoEntry:CleanupIncludeDirs: Moved dir from module header dir to dependency include dir as it had no files: " + (*m) );

            ++m;
        }

        // Check for duplicates,.. dirs that are listed as 
        // dependency includes but which also have headers part of this module
        // as such its not a dir coming from a dependency
        n = moduleInfo->GetIncludeDirs().begin();
        while ( n != moduleInfo->GetIncludeDirs().end() )
        {
            TStringSet::iterator p = moduleInfo->dependencyIncludeDirs.find( (*n).first );
            if ( p != moduleInfo->dependencyIncludeDirs.end() )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ModuleInfoEntry:CleanupIncludeDirs: Deleting dependency include dir since it already exists as a module header dir: " + (*p) );                
                moduleInfo->dependencyIncludeDirs.erase( p );
            }            
            ++n;
        }
                
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

CORE::CICloneable* 
CModuleInfoEntry::Clone( void ) const 
{GUCEF_TRACE;

    return new CModuleInfoEntry( *this );
}

/*---------------------------------------------------------------------------*/

const CORE::CString& 
CModuleInfoEntry::GetClassTypeName( void ) const 
{GUCEF_TRACE;

    return ClassTypeName;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfoEntry::SetLicense( const CORE::CString& license  ,
                              const CORE::CString& platform )
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindOrCreateModuleInfoForPlatform( platform, true );
    if ( !moduleInfo.IsNULL() )
    {
        moduleInfo->metadata.license = license;
        return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfoEntry::IsAnyLicenseDefined( void ) const
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator i = m_modulesPerPlatform.begin();
    while ( i != m_modulesPerPlatform.end() )
    {
        const CModuleInfoPtr& moduleInfo = (*i).second;
        if ( !moduleInfo.IsNULL() )
        {
            if ( !moduleInfo->metadata.license.IsNULLOrEmpty() )
                return true;
        }

        ++i;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfoEntry::SetSemVer( const CORE::CVersion& semver  ,
                             const CORE::CString& platform )
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindOrCreateModuleInfoForPlatform( platform, true );
    if ( !moduleInfo.IsNULL() )
    {
        moduleInfo->metadata.SetSemVer( semver );
        return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfoEntry::HasAnySemVer( void ) const
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator i = m_modulesPerPlatform.begin();
    while ( i != m_modulesPerPlatform.end() )
    {
        const CModuleInfoPtr& moduleInfo = (*i).second;
        if ( !moduleInfo.IsNULL() )
        {
            if ( moduleInfo->metadata.HasSemVer() )
                return true;
        }

        ++i;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfoEntry::SetLastEditBy( const CORE::CString& lastEditBy ,
                                 const CORE::CString& platform   )
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindOrCreateModuleInfoForPlatform( platform, true );
    if ( !moduleInfo.IsNULL() )
    {
        moduleInfo->metadata.lastEditBy = lastEditBy;
        return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

CModuleDependencyNode::CModuleDependencyNode( void )
    : CORE::CTSharedObjCreator< CModuleDependencyNode, MT::CMutex >( this )
    , m_module()
    , m_dependencies()
    , m_linkerDependencies()
    , m_runtimeDependencies()
    , m_dependents()
    , m_linkerDependents()
    , m_runtimeDependents()
    , m_targetPlatform()
    , m_hasMissingDependencies( false )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CModuleDependencyNode::~CModuleDependencyNode()
{GUCEF_TRACE;

    // By the time we get here clear should have been called already or you might get
    // memory leaks due to shared ptr chains
    Clear();
}

/*---------------------------------------------------------------------------*/

void
CModuleDependencyNode::Clear( void )
{GUCEF_TRACE;

    m_module.Unlink();
    m_dependencies.clear();
    m_linkerDependencies.clear();
    m_runtimeDependencies.clear();
    m_dependents.clear();
    m_linkerDependents.clear();
    m_runtimeDependents.clear();
    m_targetPlatform.Clear();
    m_hasMissingDependencies = false;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::HasMissingDependencies( void ) const
{GUCEF_TRACE;

    return m_hasMissingDependencies;
}

/*---------------------------------------------------------------------------*/

void
CModuleDependencyNode::SetHasMissingDependencies( bool hasMissing )
{GUCEF_TRACE;

    m_hasMissingDependencies = hasMissing;
}

/*---------------------------------------------------------------------------*/

void
CModuleDependencyNode::SetTargetPlatform( const CORE::CString& targetPlatform )
{GUCEF_TRACE;

    m_targetPlatform = targetPlatform;
}

/*---------------------------------------------------------------------------*/

const CORE::CString&
CModuleDependencyNode::GetTargetPlatform( void ) const
{GUCEF_TRACE;

    return m_targetPlatform;
}

/*---------------------------------------------------------------------------*/

void
CModuleDependencyNode::SetModule( const CModuleInfoEntryPtr& module )
{GUCEF_TRACE;

    m_module = module;
}

/*---------------------------------------------------------------------------*/

CModuleInfoEntryPtr
CModuleDependencyNode::GetModule( void ) const
{GUCEF_TRACE;

    return m_module;
}

/*---------------------------------------------------------------------------*/

const CORE::CString&
CModuleDependencyNode::GetConsensusName( void ) const
{GUCEF_TRACE;

    if ( !m_module.IsNULL() )
    {
        return m_module->GetConsensusName();
    }
    return CORE::CString::Empty;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::SetDependency( CModuleDependencyNodePtr dependency )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( !dependency.IsNULL() )
    {
        CModuleInfoEntryPtr dependencyModule = dependency->GetModule();
        if GUCEF_PREDICT_TRUE( !dependencyModule.IsNULL() )
        {
            const CORE::CString& consensusName = dependencyModule->GetConsensusName();
            if GUCEF_PREDICT_TRUE( !consensusName.IsNULLOrEmpty() )
            {
                m_dependencies[ consensusName ] = dependency;
                return true;
            }
        }
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void 
CModuleDependencyNode::SetDependencies( const TModuleDependencyNodePtrMap& dependencies )
{GUCEF_TRACE;

    m_dependencies = dependencies;
}

/*---------------------------------------------------------------------------*/

const TModuleDependencyNodePtrMap&
CModuleDependencyNode::GetDependencies( void ) const
{GUCEF_TRACE;

    return m_dependencies;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GatherDependencyModules( TModuleInfoEntryPtrSet& dependencies   ,
                                                bool includeDependenciesOfDependencies ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    TModuleDependencyNodePtrMap::const_iterator i = m_dependencies.begin();
    while ( i != m_dependencies.end() )
    {
        const CModuleDependencyNodePtr& dependencyNode = (*i).second;
        if GUCEF_PREDICT_TRUE( !dependencyNode.IsNULL() )
        {
            dependencies.insert( dependencyNode->GetModule() );
            if ( includeDependenciesOfDependencies )
            {
                totalSuccess = dependencyNode->GatherDependencyModules( dependencies, includeDependenciesOfDependencies ) && totalSuccess;
            }
        }
        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::SetLinkerDependency( CModuleDependencyNodePtr dependency )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( !dependency.IsNULL() )
    {
        CModuleInfoEntryPtr dependencyModule = dependency->GetModule();
        if GUCEF_PREDICT_TRUE( !dependencyModule.IsNULL() )
        {
            const CORE::CString& consensusName = dependencyModule->GetConsensusName();
            if GUCEF_PREDICT_TRUE( !consensusName.IsNULLOrEmpty() )
            {
                m_linkerDependencies[ consensusName ] = dependency;
                return true;
            }
        }
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void 
CModuleDependencyNode::SetLinkerDependencies( const TModuleDependencyNodePtrMap& dependencies )
{GUCEF_TRACE;

    m_linkerDependencies = dependencies;
}

/*---------------------------------------------------------------------------*/

const TModuleDependencyNodePtrMap&
CModuleDependencyNode::GetLinkerDependencies( void ) const
{GUCEF_TRACE;

    return m_linkerDependencies;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GatherLinkerDependencyModules( TModuleInfoEntryPtrSet& dependencies   ,
                                                      bool includeDependenciesOfDependencies ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    TModuleDependencyNodePtrMap::const_iterator i = m_linkerDependencies.begin();
    while ( i != m_linkerDependencies.end() )
    {
        const CModuleDependencyNodePtr& dependencyNode = (*i).second;
        if GUCEF_PREDICT_TRUE( !dependencyNode.IsNULL() )
        {
            dependencies.insert( dependencyNode->GetModule() );
            if ( includeDependenciesOfDependencies )
            {
                totalSuccess = dependencyNode->GatherLinkerDependencyModules( dependencies, includeDependenciesOfDependencies ) && totalSuccess;
            }
        }
        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::SetRuntimeDependency( CModuleDependencyNodePtr dependency )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( !dependency.IsNULL() )
    {
        CModuleInfoEntryPtr dependencyModule = dependency->GetModule();
        if GUCEF_PREDICT_TRUE( !dependencyModule.IsNULL() )
        {
            const CORE::CString& consensusName = dependencyModule->GetConsensusName();
            if GUCEF_PREDICT_TRUE( !consensusName.IsNULLOrEmpty() )
            {
                m_runtimeDependencies[ consensusName ] = dependency;
                return true;
            }
        }
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void 
CModuleDependencyNode::SetRuntimeDependencies( const TModuleDependencyNodePtrMap& dependencies )
{GUCEF_TRACE;

    m_runtimeDependencies = dependencies;
}

/*---------------------------------------------------------------------------*/

const TModuleDependencyNodePtrMap&
CModuleDependencyNode::GetRuntimeDependencies( void ) const
{GUCEF_TRACE;

    return m_runtimeDependencies;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GatherRuntimeDependencyModules( TModuleInfoEntryPtrSet& dependencies   ,
                                                       bool includeDependenciesOfDependencies ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    TModuleDependencyNodePtrMap::const_iterator i = m_runtimeDependencies.begin();
    while ( i != m_runtimeDependencies.end() )
    {
        const CModuleDependencyNodePtr& dependencyNode = (*i).second;
        if GUCEF_PREDICT_TRUE( !dependencyNode.IsNULL() )
        {
            dependencies.insert( dependencyNode->GetModule() );
            if ( includeDependenciesOfDependencies )
            {
                totalSuccess = dependencyNode->GatherRuntimeDependencyModules( dependencies, includeDependenciesOfDependencies ) && totalSuccess;
            }
        }
        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::SetDependent( CModuleDependencyNodePtr dependent )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( !dependent.IsNULL() )
    {
        CModuleInfoEntryPtr dependentModule = dependent->GetModule();
        if GUCEF_PREDICT_TRUE( !dependentModule.IsNULL() )
        {
            const CORE::CString& consensusName = dependentModule->GetConsensusName();
            if GUCEF_PREDICT_TRUE( !consensusName.IsNULLOrEmpty() )
            {
                m_dependents[ consensusName ] = dependent;
                return true;
            }
        }
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void 
CModuleDependencyNode::SetDependents( const TModuleDependencyNodePtrMap& dependents )
{GUCEF_TRACE;

    m_dependents = dependents;
}

/*---------------------------------------------------------------------------*/

const TModuleDependencyNodePtrMap&
CModuleDependencyNode::GetDependents( void ) const
{GUCEF_TRACE;

    return m_dependents;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GatherDependentModules( TModuleInfoEntryPtrSet& dependents ,
                                               bool includeDependentsOfDependents ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    TModuleDependencyNodePtrMap::const_iterator i = m_dependents.begin();
    while ( i != m_dependents.end() )
    {
        const CModuleDependencyNodePtr& dependentNode = (*i).second;
        if GUCEF_PREDICT_TRUE( !dependentNode.IsNULL() )
        {
            dependents.insert( dependentNode->GetModule() );
            if ( includeDependentsOfDependents )
            {
                totalSuccess = dependentNode->GatherDependentModules( dependents, includeDependentsOfDependents ) && totalSuccess;
            }
        }
        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::SetLinkerDependent( CModuleDependencyNodePtr dependent )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( !dependent.IsNULL() )
    {
        CModuleInfoEntryPtr dependentModule = dependent->GetModule();
        if GUCEF_PREDICT_TRUE( !dependentModule.IsNULL() )
        {
            const CORE::CString& consensusName = dependentModule->GetConsensusName();
            if GUCEF_PREDICT_TRUE( !consensusName.IsNULLOrEmpty() )
            {
                m_linkerDependents[ consensusName ] = dependent;
                return true;
            }
        }
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void 
CModuleDependencyNode::SetLinkerDependents( const TModuleDependencyNodePtrMap& dependents )
{GUCEF_TRACE;

    m_linkerDependents = dependents;
}

/*---------------------------------------------------------------------------*/

const TModuleDependencyNodePtrMap&
CModuleDependencyNode::GetLinkerDependents( void ) const
{GUCEF_TRACE;

    return m_linkerDependents;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GatherLinkerDependentModules( TModuleInfoEntryPtrSet& dependents ,
                                                     bool includeDependentsOfDependents ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    TModuleDependencyNodePtrMap::const_iterator i = m_linkerDependents.begin();
    while ( i != m_linkerDependents.end() )
    {
        const CModuleDependencyNodePtr& dependentNode = (*i).second;
        if GUCEF_PREDICT_TRUE( !dependentNode.IsNULL() )
        {
            dependents.insert( dependentNode->GetModule() );
            if ( includeDependentsOfDependents )
            {
                totalSuccess = dependentNode->GatherLinkerDependentModules( dependents, includeDependentsOfDependents ) && totalSuccess;
            }
        }
        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::SetRuntimeDependent( CModuleDependencyNodePtr dependent )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( !dependent.IsNULL() )
    {
        CModuleInfoEntryPtr dependentModule = dependent->GetModule();
        if GUCEF_PREDICT_TRUE( !dependentModule.IsNULL() )
        {
            const CORE::CString& consensusName = dependentModule->GetConsensusName();
            if GUCEF_PREDICT_TRUE( !consensusName.IsNULLOrEmpty() )
            {
                m_runtimeDependents[ consensusName ] = dependent;
                return true;
            }
        }
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void 
CModuleDependencyNode::SetRuntimeDependents( const TModuleDependencyNodePtrMap& dependents )
{GUCEF_TRACE;

    m_runtimeDependents = dependents;
}

/*---------------------------------------------------------------------------*/

const TModuleDependencyNodePtrMap&
CModuleDependencyNode::GetRuntimeDependents( void ) const
{GUCEF_TRACE;

    return m_runtimeDependents;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GatherRuntimeDependentModules( TModuleInfoEntryPtrSet& dependents ,
                                                      bool includeDependentsOfDependents ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    TModuleDependencyNodePtrMap::const_iterator i = m_runtimeDependents.begin();
    while ( i != m_runtimeDependents.end() )
    {
        const CModuleDependencyNodePtr& dependentNode = (*i).second;
        if GUCEF_PREDICT_TRUE( !dependentNode.IsNULL() )
        {
            dependents.insert( dependentNode->GetModule() );
            if ( includeDependentsOfDependents )
            {
                totalSuccess = dependentNode->GatherRuntimeDependentModules( dependents, includeDependentsOfDependents ) && totalSuccess;
            }
        }
        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

const CORE::CString CProjectInfo::ClassTypeName = "GUCEF::PROJECTGEN::CProjectInfo";

/*---------------------------------------------------------------------------*/

CProjectInfo::CProjectInfo( void ) 
    : CORE::CTSharedObjCreator< CProjectInfo, MT::CMutex >( this )
    , m_rwLock( true )
    , projectName()
    , rootDirs()
    , modules()
    , dirProcessingInstructions()
    , globalDirExcludeList()
    , platforms()
    , m_moduleDependencyChains()
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CProjectInfo::CProjectInfo( const CProjectInfo& src ) 
    : CORE::CTSharedObjCreator< CProjectInfo, MT::CMutex >( this )
    , m_rwLock( true )
    , projectName( src.projectName )
    , rootDirs( src.rootDirs )
    , modules( src.modules )
    , dirProcessingInstructions( src.dirProcessingInstructions )
    , globalDirExcludeList( src.globalDirExcludeList )
    , platforms( src.platforms )
    , m_moduleDependencyChains( src.m_moduleDependencyChains )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CProjectInfo::~CProjectInfo() 
{GUCEF_TRACE;

    Clear();
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::GetAllPlatformsUsed( TStringSet& platformList ,
                                   bool okToUseCachedValue  ) const
{GUCEF_TRACE;

    platformList.clear();

    if ( !m_actualPlatformsUsed.empty() && okToUseCachedValue )
    {
        // We have cached the platforms used so we can just return that
        platformList = m_actualPlatformsUsed;
        return;
    }

    // We have no cached value or not allowed to use it so we will have to (re)determine the platforms used
    TModuleInfoEntryPtrVector::const_iterator i = modules.begin();
    while ( i != modules.end() )
    {
        const TModuleInfoPtrMap& modulesPerPlatform = (*i)->GetModulesPerPlatform();
        TModuleInfoPtrMap::const_iterator n = modulesPerPlatform.begin();
        while ( n != modulesPerPlatform.end() )
        {
            platformList.insert( (*n).first );
            ++n;
        }
        ++i;
    }

    m_actualPlatformsUsed = platformList;
}

/*---------------------------------------------------------------------------*/

CModuleInfoEntryPtr
CProjectInfo::GetModuleInfoEntry( const CORE::CString& consensusModuleName ) const
{GUCEF_TRACE;

    TModuleInfoEntryPtrVector::const_iterator m = modules.begin();
    while ( m != modules.end() )
    {
        const CModuleInfoEntryPtr& moduleInfoEntry = (*m);
        if ( !moduleInfoEntry.IsNULL() )
        {
            // We will use the consensus name as the key 
            const CORE::CString& thisConsensusModuleName = moduleInfoEntry->GetConsensusName();
            if ( consensusModuleName == thisConsensusModuleName )
            {
                return moduleInfoEntry;
            }
        }
        ++m;
    }
    return CModuleInfoEntryPtr();
}

/*---------------------------------------------------------------------------*/

CModuleInfoEntryPtr
CProjectInfo::GetModuleInfoEntry( const CORE::CString& moduleName    ,
                                  const CORE::CString& platform      ,
                                  CModuleInfoPtr* platformModuleInfo ) const
{GUCEF_TRACE;

    // Check the dependency chains first since its conveniently mapped by platform and module name
    TStringToModuleDependencyNodePtrMap::const_iterator i = m_moduleDependencyChains.find( platform );
    if ( i != m_moduleDependencyChains.end() )
    {
        // we have chains for this platform, now look for the specific module
        const TModuleDependencyNodePtrMap& chainsForPlatform = (*i).second;
        TModuleDependencyNodePtrMap::const_iterator n = chainsForPlatform.find( moduleName );
        if ( n != chainsForPlatform.end() )
        {            
            const CModuleDependencyNodePtr dependencyChain = (*n).second;
            CModuleInfoEntryPtr moduleInfoEntry = dependencyChain->GetModule();
            if ( !moduleInfoEntry.IsNULL() && GUCEF_NULL != platformModuleInfo )
            {
                *platformModuleInfo = moduleInfoEntry->FindModuleInfoForPlatform( platform );
            }
            return moduleInfoEntry;
        }
    }
    if ( platform != AllPlatforms && !platform.IsNULLOrEmpty() )
    {
        // Also check for 'all' platforms
        i = m_moduleDependencyChains.find( AllPlatforms );
        if ( i != m_moduleDependencyChains.end() )
        {
            // we have chains for this platform, now look for the specific module
            const TModuleDependencyNodePtrMap& chainsForPlatform = (*i).second;
            TModuleDependencyNodePtrMap::const_iterator n = chainsForPlatform.find( moduleName );
            if ( n != chainsForPlatform.end() )
            {            
                const CModuleDependencyNodePtr dependencyChain = (*n).second;
                CModuleInfoEntryPtr moduleInfoEntry = dependencyChain->GetModule();
                if ( !moduleInfoEntry.IsNULL() && GUCEF_NULL != platformModuleInfo )
                {
                    *platformModuleInfo = moduleInfoEntry->FindModuleInfoForPlatform( platform );
                }
                return moduleInfoEntry;
            }
        }
    }

    // No such reference in the dependency chains
    // fall back to the flat list of modules
    CModuleInfoEntryPtr moduleInfoEntry = GetModuleInfoEntry( moduleName );
    if ( !moduleInfoEntry.IsNULL() )
    {
        if ( moduleInfoEntry->IsApplicableForPlatform( platform ) )
        {
            if ( GUCEF_NULL != platformModuleInfo )
                *platformModuleInfo = moduleInfoEntry->FindModuleInfoForPlatform( platform );
                
            return moduleInfoEntry;
        }
    }

    return CModuleInfoEntryPtr();    
}

/*---------------------------------------------------------------------------*/

CModuleInfoEntryPtr
CProjectInfo::GetModuleInfoEntry( const CORE::CString& moduleName    ,
                                  const CORE::CString& platform      ,
                                  CModuleInfoPtr* platformModuleInfo ,
                                  bool createNewIfNoneExists         )
{GUCEF_TRACE;

    // Check the dependency chains first since its conveniently mapped by platform and module name
    TStringToModuleDependencyNodePtrMap::iterator i = m_moduleDependencyChains.find( platform );
    if ( i != m_moduleDependencyChains.end() )
    {
        // we have chains for this platform, now look for the specific module
        const TModuleDependencyNodePtrMap& chainsForPlatform = (*i).second;
        TModuleDependencyNodePtrMap::const_iterator n = chainsForPlatform.find( moduleName );
        if ( n != chainsForPlatform.end() )
        {            
            const CModuleDependencyNodePtr dependencyChain = (*n).second;
            CModuleInfoEntryPtr moduleInfoEntry = dependencyChain->GetModule();
            if ( !moduleInfoEntry.IsNULL() && GUCEF_NULL != platformModuleInfo )
            {
                *platformModuleInfo = moduleInfoEntry->FindOrCreateModuleInfoForPlatform( platform, createNewIfNoneExists );
            }
            return moduleInfoEntry;
        }
    }
    if ( platform != AllPlatforms && !platform.IsNULLOrEmpty() )
    {
        // Also check for 'all' platforms
        i = m_moduleDependencyChains.find( AllPlatforms );
        if ( i != m_moduleDependencyChains.end() )
        {
            // we have chains for this platform, now look for the specific module
            const TModuleDependencyNodePtrMap& chainsForPlatform = (*i).second;
            TModuleDependencyNodePtrMap::const_iterator n = chainsForPlatform.find( moduleName );
            if ( n != chainsForPlatform.end() )
            {            
                const CModuleDependencyNodePtr dependencyChain = (*n).second;
                CModuleInfoEntryPtr moduleInfoEntry = dependencyChain->GetModule();
                if ( !moduleInfoEntry.IsNULL() && GUCEF_NULL != platformModuleInfo )
                {
                    *platformModuleInfo = moduleInfoEntry->FindOrCreateModuleInfoForPlatform( platform, createNewIfNoneExists );
                }
                return moduleInfoEntry;
            }
        }
    }

    // No such reference in the dependency chains
    // fall back to the flat list of modules
    CModuleInfoEntryPtr moduleInfoEntry = GetModuleInfoEntry( moduleName );
    if ( !moduleInfoEntry.IsNULL() )
    {
        if ( moduleInfoEntry->IsApplicableForPlatform( platform ) )
        {
            if ( GUCEF_NULL != platformModuleInfo )
                *platformModuleInfo = moduleInfoEntry->FindOrCreateModuleInfoForPlatform( platform, createNewIfNoneExists );
                
            return moduleInfoEntry;
        }
    }

    return CModuleInfoEntryPtr();    
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::ReduceAllPlatformsDefinitionToSustainablePlatforms( CModuleInfoEntryPtr moduleInfoEntry )
{GUCEF_TRACE;

    
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::SanitizeRuntimeDependenciesForPlatform( CModuleInfoEntryPtr moduleInfoEntry ,
                                                      const CORE::CString& targetPlatform )
{GUCEF_TRACE;

    if ( moduleInfoEntry.IsNULL() )
        return;

    const CORE::CString& moduleName = moduleInfoEntry->GetConsensusName();

    CORE::CStringSet runtimeDependencyModuleNames;
    moduleInfoEntry->GetModuleRuntimeDependencyNames( targetPlatform, runtimeDependencyModuleNames );

    CORE::CStringSet::iterator i = runtimeDependencyModuleNames.begin();
    while ( i != runtimeDependencyModuleNames.end() )
    {
        const CORE::CString& dependencyName = (*i);

        // Getting the module for the given entry checks if it exists and if its applicable to the given platform
        CModuleInfoPtr dependencyModulePlatformSpecifics;
        CModuleInfoEntryPtr dependencyModule = GetModuleInfoEntry( dependencyName, targetPlatform, &dependencyModulePlatformSpecifics );
        if ( !dependencyModule.IsNULL() )
        {
            if ( !dependencyModule->IsBroken() )
            {

            }
            else
            {
                // We found a problematic entry
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:SanitizeRuntimeDependencies: Found that module \"" + moduleName +
                    "\" has broken runtime dependency module definition \"" + dependencyName + "\". Will remove the dependency"  );

                moduleInfoEntry->RemoveRuntimeDependency( dependencyName, targetPlatform );
            }
        }
        else
        {
            // We found a problematic entry
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:SanitizeRuntimeDependencies: Found that module \"" + moduleName +
                "\" has missing runtime dependency module definition \"" + dependencyName + "\" for platform " + targetPlatform + ". Will remove the dependency"  );

            moduleInfoEntry->RemoveRuntimeDependency( dependencyName, targetPlatform );

            if ( targetPlatform == AllPlatforms )
            {
                // Just because we cannot satisfy the dependency for 'all' platforms we can still perhaps make it available as an
                // extention for the specific platforms that the runtime dependency supports
                dependencyModule = GetModuleInfoEntry( dependencyName );
                if ( !dependencyModule.IsNULL() && !dependencyModule->IsBroken() )
                {
                    CORE::CStringSet supportedPlatforms;
                    dependencyModule->GetReferencedPlatforms( supportedPlatforms );
                    supportedPlatforms.erase( AllPlatforms );

                    CORE::CStringSet::iterator s = supportedPlatforms.begin();
                    while ( s != supportedPlatforms.end() )
                    {
                        const CORE::CString& supportedPlatform = (*s);
                        CModuleInfoPtr platformSpecificDefinition = moduleInfoEntry->FindOrCreateModuleInfoForPlatform( supportedPlatform, true );
                        if ( !platformSpecificDefinition.IsNULL() )
                        {
                            platformSpecificDefinition->AddNameOfRuntimeDependency( dependencyName );

                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:SanitizeRuntimeDependencies: Added runtime dependency for module \"" + moduleName +
                                "\" to module \"" + dependencyName + "\" for platform " + supportedPlatform + " as substitute for All platforms reference to the same"  );
                        }
                        ++s;
                    }
                }
            }
        }
        ++i;
    }  
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::SanitizeRuntimeDependencies( CModuleInfoEntryPtr moduleInfoEntry )
{GUCEF_TRACE;

    if ( moduleInfoEntry.IsNULL() )
        return;

    // Take care of 'All' platform first since that one does allow reducing down
    // to platform specific definitions
    SanitizeRuntimeDependenciesForPlatform( moduleInfoEntry, AllPlatforms );

    CORE::CStringSet referencedPlatforms;
    moduleInfoEntry->GetReferencedPlatforms( referencedPlatforms );
    referencedPlatforms.erase( AllPlatforms );

    CORE::CStringSet::iterator p = referencedPlatforms.begin();
    while ( p != referencedPlatforms.end() )
    {
        const CORE::CString& targetPlatform = (*p);
        SanitizeRuntimeDependenciesForPlatform( moduleInfoEntry, targetPlatform );
        ++p;
    }
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::SanitizeRuntimeDependencies( void )
{GUCEF_TRACE;

    TModuleInfoEntryPtrVector::iterator m = modules.begin();
    while ( m != modules.end() )
    {
        CModuleInfoEntryPtr& moduleInfoEntry = (*m);
        if GUCEF_PREDICT_TRUE( !moduleInfoEntry.IsNULL() )
        {
            SanitizeRuntimeDependencies( moduleInfoEntry );
        }
        ++m;
    }
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::SanitizeAllPlatformsUsage( void )
{GUCEF_TRACE;

    TModuleInfoEntryPtrVector::iterator m = modules.begin();
    while ( m != modules.end() )
    {
        CModuleInfoEntryPtr& moduleInfoEntry = (*m);
        if GUCEF_PREDICT_TRUE( !moduleInfoEntry.IsNULL() )
        {
            if ( moduleInfoEntry->HasAllPlatformsDefinition() )
            {
                TModuleInfoEntryPtrSet problemModules;
                CORE::CStringSet missingModules;
                if ( !CanModuleSustainAllPlatformsDefinition( moduleInfoEntry, problemModules, missingModules ) )
                {
                    const CORE::CString& moduleName = moduleInfoEntry->GetConsensusName();

                    CORE::CStringSet problemModuleNames; 
                    TModuleInfoEntryPtrSet::iterator p = problemModules.begin();
                    while ( p != problemModules.end() )
                    {
                        const CModuleInfoEntryPtr& problemModule = (*p);
                        if GUCEF_PREDICT_TRUE( !problemModule.IsNULL() )
                        {
                            problemModuleNames.insert( (*p)->GetConsensusName() );
                        }
                        ++p;
                    }

                    // We found a problematic entry
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:SanitizeAllPlatformsUsage: Found that module \"" + moduleName +
                        "\" cannot sustain an 'all' platforms definition due to its dependencies. There are " +
                        CORE::ToString( problemModuleNames.size() ) + " problematic dependencies found \"" + CORE::ToString( problemModuleNames ) +
                        "\" and " + CORE::ToString( missingModules.size() ) + " missing dependencies found \"" + CORE::ToString( missingModules ) + "\""  );

                    if ( missingModules.empty() )
                    {
                        ReduceAllPlatformsDefinitionToSustainablePlatforms( moduleInfoEntry );
                    }
                    else
                    {
                        // we cannot fix this. The user needs to fix the dependencies or definition
                        moduleInfoEntry->SetIsBroken( true );
                    }
                }
            }
        }
        ++m;
    }
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::GenerateDependencyChainNodes( const CORE::CString& targetPlatform     ,
                                            bool okToUseCachedValuesWhereApplicable )
{GUCEF_TRACE;

    bool totalSuccess = true;
    TModuleDependencyNodePtrMap& chainsForPlatform = m_moduleDependencyChains[ targetPlatform ];

    // We should not have an empty chains map given that we are iterating over the platforms used by the modules
    // This tells us its not initialized yet, which is fine, we will just create it now
    if ( chainsForPlatform.empty() || !okToUseCachedValuesWhereApplicable )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChains: (re)Building dependency chains for platform " + targetPlatform +
            ". It currently has " + CORE::ToString( chainsForPlatform.size() ) + " dependency chain nodes" );

        TModuleInfoEntryPtrVector::const_iterator m = modules.begin();
        while ( m != modules.end() )
        {
            const CModuleInfoEntryPtr& moduleInfoEntry = (*m);
            if GUCEF_PREDICT_TRUE( !moduleInfoEntry.IsNULL() )
            {
                // Don't include logical modules in the dependency chains
                // Only 'real' modules
                if ( moduleInfoEntry->HasIndependentModuleTypeForPlatform( targetPlatform ) )
                {
                    // This dependency chain map is specific to the target platform
                    // don't include modules that have no relevance to said platform
                    // Also don't include broken module definitions
                    if ( moduleInfoEntry->IsApplicableForPlatform( targetPlatform ) && !moduleInfoEntry->IsBroken() )
                    {
                        // We will use the consensus name as the key for the chain
                        const CORE::CString& consensusModuleName = moduleInfoEntry->GetConsensusName();

                        // Check for a pre-existing entry
                        // With these chain nodes we have to be careful not to create memory leaks due to the bi-directional
                        // shared ptr linkage
                        CModuleDependencyNodePtr& dependencyChain = chainsForPlatform[ consensusModuleName ];
                        if ( !dependencyChain.IsNULL() )
                        {
                            dependencyChain->Clear();
                        }

                        // Now create the new entry
                        dependencyChain = chainsForPlatform[ consensusModuleName ] = CModuleDependencyNode::CreateSharedObj();
                        if ( !dependencyChain.IsNULL() )
                        {
                            dependencyChain->SetModule( moduleInfoEntry );
                            dependencyChain->SetTargetPlatform( targetPlatform );
                        }
                        else
                        {
                            totalSuccess = false;
                        }
                    }
                }
            }
            ++m;
        }

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChains: Platform " + targetPlatform + " has " + CORE::ToString( chainsForPlatform.size() ) + " dependency chain nodes" );
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::UpdateDependencyChains( bool okToUseCachedValuesWhereApplicable )
{GUCEF_TRACE;

    bool totalSuccess = true;

    // we specifically use a platforms list which is sourced from our flat list of modules
    // so that we can ensure that we have all platforms used by the modules
    TStringSet platformList;
    GetAllPlatformsUsed( platformList, okToUseCachedValuesWhereApplicable );

    // we now access and/or create the map entries for each platform
    TStringSet::const_iterator i = platformList.begin();
    while ( i != platformList.end() )
    {
        const CORE::CString& targetPlatform = (*i);
        totalSuccess = GenerateDependencyChainNodes( targetPlatform, okToUseCachedValuesWhereApplicable ) && totalSuccess;
        ++i;
    }

    // Now that we know for sure that we have a dependency node entry for every relevant
    // module we can use the UpdateDependencyChain() function to do the rest of the work
    i = platformList.begin();
    while ( i != platformList.end() )
    {
        const CORE::CString& targetPlatform = (*i);

        // We do the 'AllPlatforms' platform last so that we can ensure that the chains for the specific platforms
        // are updated first, this way we can ensure that the 'AllPlatforms' chains will have platform specific chains
        // available to them when they are updated.
        if ( targetPlatform != AllPlatforms && !targetPlatform.IsNULLOrEmpty() )
        {
            TModuleDependencyNodePtrMap& chainsForPlatform = m_moduleDependencyChains[ targetPlatform ];

            TModuleDependencyNodePtrMap::iterator n = chainsForPlatform.begin();
            while ( n != chainsForPlatform.end() )
            {            
                const CORE::CString& consensusModuleName = (*n).first;
                CModuleDependencyNodePtr dependencyChain = (*n).second;

                bool chainSuccess = UpdateDependencyChain( dependencyChain );
                if ( !chainSuccess )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:UpdateDependencyChains: Failed to update dependency chain for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                    totalSuccess = false;
                }
                ++n;
            }
        }
        ++i;
    }

    // Now do the 'AllPlatforms' platform
    TModuleDependencyNodePtrMap& chainsForPlatform = m_moduleDependencyChains[ AllPlatforms ];

    TModuleDependencyNodePtrMap::iterator n = chainsForPlatform.begin();
    while ( n != chainsForPlatform.end() )
    {            
        const CORE::CString& consensusModuleName = (*n).first;
        CModuleDependencyNodePtr dependencyChain = (*n).second;

        bool chainSuccess = UpdateDependencyChain( dependencyChain );
        if ( !chainSuccess )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:UpdateDependencyChains: Failed to update dependency chain for module \"" + consensusModuleName + "\" and platform " + AllPlatforms );
            totalSuccess = false;
        }
        ++n;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::CanModuleSustainAllPlatformsDefinition( const CORE::CStringSet& dependencies   ,
                                                      bool okIfNoSuchDependencyDefined       ,
                                                      TModuleInfoEntryPtrSet& problemModules ,
                                                      CORE::CStringSet& missingModules       ) const
{GUCEF_TRACE;

    bool canSustainAllPlatforms = true;

    CORE::CStringSet::const_iterator i = dependencies.begin();
    while ( i != dependencies.end() )
    {
        const CORE::CString& dependencyName = (*i);
        CModuleInfoEntryPtr dependencyModuleEntry = GetModuleInfoEntry( dependencyName );
        if ( !dependencyModuleEntry.IsNULL() )
        {
            if ( !CanModuleSustainAllPlatformsDefinition( dependencyModuleEntry, problemModules, missingModules ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:CanModuleSustainAllPlatformsDefinition: Dependency module \"" + dependencyName + "\" cannot sustain an 'all' platforms definition" );
                canSustainAllPlatforms = false;
            }
        }
        else
        {
            if ( !okIfNoSuchDependencyDefined )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:CanModuleSustainAllPlatformsDefinition: Dependency module \"" + dependencyName + "\" cannot be found, no module by that name for ANY platform" );
                canSustainAllPlatforms = false;
                missingModules.insert( dependencyName );
            }
        }
        ++i;
    }

    return canSustainAllPlatforms;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::CanModuleSustainAllPlatformsDefinition( CModuleInfoEntryPtr moduleInfoEntry    ,
                                                      TModuleInfoEntryPtrSet& problemModules ,
                                                      CORE::CStringSet& missingModules       ) const
{GUCEF_TRACE;

    if ( moduleInfoEntry.IsNULL() )
        return false;
    bool canSustainAllPlatforms = true;

    const TModuleInfoPtrMap& modulesPerPlatform = moduleInfoEntry->GetModulesPerPlatform();
    TModuleInfoPtrMap::const_iterator i = modulesPerPlatform.find( AllPlatforms );
    if ( i != modulesPerPlatform.end() )
    {
        const CModuleInfoPtr& allPlatformsDefinition = (*i).second;
        if ( !allPlatformsDefinition.IsNULL() )
        {              
            // Check the build dependencies, used for header include paths and such
            const CORE::CStringSet& dependencies = allPlatformsDefinition->GetNamesOfDependencies();
            if ( !CanModuleSustainAllPlatformsDefinition( dependencies, false, problemModules, missingModules ) )
            {
                canSustainAllPlatforms = false;
            }

            // Check the linker dependencies, used for static linking to the given libraries
            // Keep in mind that for linking you may depend on external binary libraries for which we have no
            // definition ourselves. We won't consider it an error in the linker case if there is no such module definition
            CORE::CStringSet linkerDependencies;
            allPlatformsDefinition->linkerSettings.GetListOfLinkedLibraries( linkerDependencies );
            if ( !CanModuleSustainAllPlatformsDefinition( linkerDependencies, true, problemModules, missingModules ) )
            {
                canSustainAllPlatforms = false;
            }
        }
        ++i;
    }
    return canSustainAllPlatforms;
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::GetSupportedPlatformsBasedOnDependencies( CModuleInfoEntryPtr moduleInfoEntry ,
                                                        CORE::CStringSet& platforms         ) const
{GUCEF_TRACE;

    if ( moduleInfoEntry.IsNULL() )
        return;

    const TModuleInfoPtrMap& modulesPerPlatform = moduleInfoEntry->GetModulesPerPlatform();
    TModuleInfoPtrMap::const_iterator i = modulesPerPlatform.begin();
    while ( i != modulesPerPlatform.end() )
    {
        const CORE::CString& platformName = (*i).first;
        const CModuleInfoPtr& platformDefinition = (*i).second;

        platformDefinition->GetNamesOfDependencies();

        if ( platformName == AllPlatforms || platformName.IsNULLOrEmpty() )
        {
        }
        else
        {
        }

        ++i;
    }

}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::SanitizeModulePlatformUsage( CModuleInfoEntryPtr moduleInfoEntry ,
                                           const CORE::CString& platform       ,
                                           const TStringSet& dependencies      )
{GUCEF_TRACE;

    if ( dependencies.empty() )
        return true;

    bool passedSanityCheck = true;

    // We have dependencies, we need to ensure that they are also defined for all platforms
    TStringSet::iterator d = dependencies.begin();
    while ( d != dependencies.end() )
    {
        const CORE::CString& dependencyName = (*d);
        CModuleInfoPtr dependencyPlatformModule;
        CModuleInfoEntryPtr dependencyModule = GetModuleInfoEntry( dependencyName, platform, &dependencyPlatformModule );
        if ( dependencyModule.IsNULL() )
        {
            // Dependency does not exist at all, let alone for the specific platform
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:SanitizeModulePlatformUsage: Module \"" + moduleInfoEntry->GetConsensusName() +
                "\" has a dependency on \"" + dependencyName + "\". No such module exists, this is not auto fixable and the module will not be usable. Please change the module specification or supply the dependency" );

            // We cannot fix this
            CModuleDependencyNodePtr dependencyChain;
            if ( TryGetModuleDependencyChain( dependencyChain, moduleInfoEntry->GetConsensusName(), platform, true ) )
            {
                dependencyChain->SetHasMissingDependencies( true );
            }
            passedSanityCheck = false;
        }
        else
        if ( dependencyPlatformModule.IsNULL() )
        {
            if ( platform == AllPlatforms || platform.IsNULLOrEmpty()  )
            {
                // The dependency is not defined for the platform so we will have to downgrade the dependent module to the platform for which the dependency is defined
                CORE::CStringSet supportedPlatforms;
                dependencyModule->GetReferencedPlatforms( supportedPlatforms );
            }

            ++d;
        }
        else
        {
            // we have what we need: No issues
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:SanitizeModulePlatformUsage: Module \"" + moduleInfoEntry->GetConsensusName() +
                "\" has a dependency on \"" + dependencyName + "\". No such module exists" );
        }
    }

    return passedSanityCheck;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::SanitizeModulePlatformUsage( CModuleInfoEntryPtr moduleInfoEntry ,
                                           const CORE::CString& platform       )
{GUCEF_TRACE;

    bool passedSanityCheck = true;
    TStringSet dependencies;

    moduleInfoEntry->GetModuleDependencyNames( platform, dependencies );
    passedSanityCheck = SanitizeModulePlatformUsage( moduleInfoEntry, platform, dependencies ) && passedSanityCheck;
    dependencies.clear();

    moduleInfoEntry->GetModuleLinkerDependencyNames( platform, dependencies );
    passedSanityCheck = SanitizeModulePlatformUsage( moduleInfoEntry, platform, dependencies ) && passedSanityCheck;
    dependencies.clear();

    return passedSanityCheck;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::SanitizeModulePlatformUsage( CModuleInfoEntryPtr moduleInfoEntry )
{GUCEF_TRACE;

    // Things to check:
    //      For AllPlatforms: fixable:
    //          - If present ensure that all the relevant dependencies are also present for all platforms
    //            If not we need to downgrade the module to the platforms for which the dependencies are present
    //      For platform specific: not-fixable (thus error and delete):
    //          - If present ensure that all the relevant dependencies are also present for the platform
    //                  ie win32->win32 or win32->allPlatforms
    //                  not allowed would be: win32->win64 or win32->linux

    CORE::CStringSet platforms;
    moduleInfoEntry->GetReferencedPlatforms( platforms );

    bool passedSanityCheck = true;

    //CORE::CStringSet::const_iterator i = platforms.begin();
    //while ( i != platforms.end() )
    //{
    //    const CORE::CString& platform = (*i);
    //    passedSanityCheck = SanitizeModulePlatformUsage( moduleInfoEntry, platform ) && passedSanityCheck;
    //    ++i;
    //}

    //

    //TStringSet dependencies;

    //moduleInfoEntry->GetModuleDependencyNames( AllPlatforms, dependencies );
    //SanitizeModulePlatformUsage( moduleInfoEntry, dependencies );
    //dependencies.clear();

    //moduleInfoEntry->GetModuleLinkerDependencyNames( AllPlatforms, dependencies );
    //SanitizeModulePlatformUsage( moduleInfoEntry, dependencies );
    //dependencies.clear();


    //const TModuleInfoPtrMap& platformModules = moduleInfoEntry->GetModulesPerPlatform();
    //TModuleInfoPtrMap::const_iterator i = platformModules.find( AllPlatforms );
    //if ( i != platformModules.end() )
    //{
    //    // the module is defined for all platforms, we need to ensure that all dependencies are also defined for all platforms
    //    CModuleInfoPtr module = (*i).second;
    //    if ( module->HasIndependentModuleType() )
    //    {
    //        CORE::CStringMap platformSpecificDependencies;

    //        const TStringSet& dependencies = module->GetNamesOfDependencies();
    //        SanitizeModulePlatformUsage( moduleInfoEntry, dependencies );

    //        dependencies = module->GetNamesOfLinkerDependencies();
    //        SanitizeModulePlatformUsage( moduleInfoEntry, dependencies );
    //    }
    //}

    return passedSanityCheck;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::UpdateDependencyChain( CModuleDependencyNodePtr dependencyChain )
{GUCEF_TRACE;

    bool totalSuccess = true;

    if ( !dependencyChain.IsNULL() )
    {
        const CORE::CString& targetPlatform = dependencyChain->GetTargetPlatform();
        const CModuleInfoEntryPtr& moduleInfoEntry = dependencyChain->GetModule();

        if ( !moduleInfoEntry.IsNULL() )
        {
            const CORE::CString& consensusModuleName = moduleInfoEntry->GetConsensusName();

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChain: Updating chain node for module \"" + consensusModuleName + "\" for target platform " + targetPlatform );

            TStringSet dependencies;
            moduleInfoEntry->GetModuleDependencyNames( targetPlatform, dependencies );

            TStringSet::iterator d = dependencies.begin();
            while ( d != dependencies.end() )
            {
                const CORE::CString& dependencyName = (*d);
                CModuleDependencyNodePtr dependencyModule;
                if ( TryGetModuleDependencyChain( dependencyModule, dependencyName, targetPlatform, false ) && !dependencyModule.IsNULL() )
                {
                    totalSuccess = dependencyChain->SetDependency( dependencyModule ) && totalSuccess;
                }
                else
                {
                    CModuleInfoEntryPtr dependencyModuleInfoEntry = GetModuleInfoEntry( dependencyName );
                    if ( !dependencyModuleInfoEntry.IsNULL() )
                    {
                        if ( dependencyModuleInfoEntry->HasIndependentModuleTypeForPlatform( targetPlatform ) )
                        {
                            if ( !dependencyModuleInfoEntry->IsBroken() )
                            {
                                bool issueWasFixed = false;
                                if ( SanitizeModulePlatformUsage( moduleInfoEntry ) )
                                {
                                    // try again now that we have sanitized the module
                                    if ( TryGetModuleDependencyChain( dependencyModule, dependencyName, targetPlatform, false ) && !dependencyModule.IsNULL() )
                                    {
                                        issueWasFixed = dependencyChain->SetDependency( dependencyModule );
                                        totalSuccess = issueWasFixed && totalSuccess;
                                    }
                                }
                                if ( !issueWasFixed )
                                {
                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:UpdateDependencyChain: Failed to obtain actual module for dependency name: \"" +
                                        dependencyName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                                    totalSuccess = false;
                                }
                            }
                            else
                            {
                                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChain: Ignoring broken dependency with name: \"" +
                                    dependencyName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                            }
                        }
                    }
                }
                ++d;
            }

            dependencies.clear();
            moduleInfoEntry->GetModuleLinkerDependencyNames( targetPlatform, dependencies );

            d = dependencies.begin();
            while ( d != dependencies.end() )
            {
                const CORE::CString& dependencyName = (*d);
                CModuleDependencyNodePtr dependencyModule;
                if ( TryGetModuleDependencyChain( dependencyModule, dependencyName, targetPlatform, false ) && !dependencyModule.IsNULL() )
                {
                    totalSuccess = dependencyChain->SetLinkerDependency( dependencyModule ) && totalSuccess;
                }
                else
                {
                    CModuleInfoEntryPtr dependencyModuleInfoEntry = GetModuleInfoEntry( dependencyName );
                    if ( !dependencyModuleInfoEntry.IsNULL() )
                    {
                        if ( dependencyModuleInfoEntry->HasIndependentModuleTypeForPlatform( targetPlatform ) )
                        {
                            if ( !dependencyModuleInfoEntry->IsBroken() )
                            {
                                bool issueWasFixed = false;
                                if ( SanitizeModulePlatformUsage( moduleInfoEntry ) )
                                {
                                    // try again now that we have sanitized the module
                                    if ( TryGetModuleDependencyChain( dependencyModule, dependencyName, targetPlatform, false ) && !dependencyModule.IsNULL() )
                                    {
                                        issueWasFixed = dependencyChain->SetDependency( dependencyModule );
                                        totalSuccess = issueWasFixed && totalSuccess;
                                    }
                                }
                                if ( !issueWasFixed )
                                {
                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:UpdateDependencyChain: Failed to obtain actual module for linker dependency name: \"" +
                                        dependencyName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                                    totalSuccess = false;
                                }
                            }
                            else
                            {
                                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChain: Ignoring broken dependency with name: \"" +
                                    dependencyName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                            }  
                        }
                    }
                }
                ++d;
            }

            dependencies.clear();
            moduleInfoEntry->GetModuleRuntimeDependencyNames( targetPlatform, dependencies );

            d = dependencies.begin();
            while ( d != dependencies.end() )
            {
                const CORE::CString& dependencyName = (*d);
                CModuleDependencyNodePtr dependencyModule;
                if ( TryGetModuleDependencyChain( dependencyModule, dependencyName, targetPlatform, false ) && !dependencyModule.IsNULL() )
                {
                    totalSuccess = dependencyChain->SetRuntimeDependency( dependencyModule ) && totalSuccess;
                }
                else
                {
                    CModuleInfoEntryPtr dependencyModuleInfoEntry = GetModuleInfoEntry( dependencyName );
                    if ( !dependencyModuleInfoEntry.IsNULL() )
                    {
                        if ( dependencyModuleInfoEntry->HasIndependentModuleTypeForPlatform( targetPlatform ) )
                        {
                            if ( !dependencyModuleInfoEntry->IsBroken() )
                            {
                                bool issueWasFixed = false;
                                if ( SanitizeModulePlatformUsage( moduleInfoEntry ) )
                                {
                                    // try again now that we have sanitized the module
                                    if ( TryGetModuleDependencyChain( dependencyModule, dependencyName, targetPlatform, false ) && !dependencyModule.IsNULL() )
                                    {
                                        issueWasFixed = dependencyChain->SetRuntimeDependency( dependencyModule );
                                        totalSuccess = issueWasFixed && totalSuccess;
                                    }
                                }
                                if ( !issueWasFixed )
                                {
                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:UpdateDependencyChain: Failed to obtain actual module for runtime dependency name: \"" +
                                        dependencyName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                                    totalSuccess = false;
                                }
                            }
                            else
                            {
                                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChain: Ignoring broken dependency with name: \"" +
                                    dependencyName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                            }                            
                        }
                    }
                }
                ++d;
            }

            dependencies.clear();
            GetNamesOfModulesWhichDependOnModuleForPlatform( targetPlatform, consensusModuleName, dependencies, false );

            d = dependencies.begin();
            while ( d != dependencies.end() )
            {
                const CORE::CString& dependentName = (*d);
                CModuleDependencyNodePtr dependentModule;
                if ( TryGetModuleDependencyChain( dependentModule, dependentName, targetPlatform, false ) && !dependentModule.IsNULL() )
                {
                    totalSuccess = dependencyChain->SetDependent( dependentModule ) && totalSuccess;
                }
                else
                {
                    CModuleInfoEntryPtr dependencyModuleInfoEntry = GetModuleInfoEntry( dependentName );
                    if ( !dependencyModuleInfoEntry.IsNULL() )
                    {
                        if ( dependencyModuleInfoEntry->HasIndependentModuleTypeForPlatform( targetPlatform ) )
                        {
                            if ( !dependencyModuleInfoEntry->IsBroken() )
                            {
                                bool issueWasFixed = false;
                                if ( SanitizeModulePlatformUsage( moduleInfoEntry ) )
                                {
                                    // try again now that we have sanitized the module
                                    if ( TryGetModuleDependencyChain( dependentModule, dependentName, targetPlatform, false ) && !dependentModule.IsNULL() )
                                    {
                                        issueWasFixed = dependencyChain->SetDependent( dependentModule );
                                        totalSuccess = issueWasFixed && totalSuccess;
                                    }
                                }
                                if ( !issueWasFixed )
                                {
                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:UpdateDependencyChain: Failed to obtain actual module for dependent name: \"" +
                                        dependentName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                                    totalSuccess = false;
                                }
                            }
                            else
                            {
                                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChain: Ignoring broken dependent with name: \"" +
                                    dependentName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                            }
                        }
                    }
                }
                ++d;
            }

            dependencies.clear();
            GetNamesOfModulesWhichLinkerDependOnModuleForPlatform( targetPlatform, consensusModuleName, dependencies, false );

            d = dependencies.begin();
            while ( d != dependencies.end() )
            {
                const CORE::CString& dependentName = (*d);
                CModuleDependencyNodePtr dependentModule;
                if ( TryGetModuleDependencyChain( dependentModule, dependentName, targetPlatform, false ) && !dependentModule.IsNULL() )
                {
                    totalSuccess = dependencyChain->SetLinkerDependent( dependentModule ) && totalSuccess;
                }
                else
                {
                    CModuleInfoEntryPtr dependencyModuleInfoEntry = GetModuleInfoEntry( dependentName );
                    if ( !dependencyModuleInfoEntry.IsNULL() )
                    {
                        if ( dependencyModuleInfoEntry->HasIndependentModuleTypeForPlatform( targetPlatform ) )
                        {
                            if ( !dependencyModuleInfoEntry->IsBroken() )
                            {
                                bool issueWasFixed = false;
                                if ( SanitizeModulePlatformUsage( moduleInfoEntry ) )
                                {
                                    // try again now that we have sanitized the module
                                    if ( TryGetModuleDependencyChain( dependentModule, dependentName, targetPlatform, false ) && !dependentModule.IsNULL() )
                                    {
                                        issueWasFixed = dependencyChain->SetLinkerDependent( dependentModule );
                                        totalSuccess = issueWasFixed && totalSuccess;
                                    }
                                }
                                if ( !issueWasFixed )
                                {
                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:UpdateDependencyChain: Failed to obtain actual module for linker dependent name: \"" +
                                        dependentName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                                    totalSuccess = false;
                                }
                            }
                            else
                            {
                                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChain: Ignoring broken linker dependent with name: \"" +
                                    dependentName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                            }                                
                        }
                    }
                }
                ++d;
            }

            dependencies.clear();
            GetNamesOfModulesWhichRuntimeDependOnModuleForPlatform( targetPlatform, consensusModuleName, dependencies, false );

            d = dependencies.begin();
            while ( d != dependencies.end() )
            {
                const CORE::CString& dependentName = (*d);
                CModuleDependencyNodePtr dependentModule;
                if ( TryGetModuleDependencyChain( dependentModule, dependentName, targetPlatform, false ) && !dependentModule.IsNULL() )
                {
                    totalSuccess = dependencyChain->SetRuntimeDependent( dependentModule ) && totalSuccess;
                }
                else
                {
                    CModuleInfoEntryPtr dependencyModuleInfoEntry = GetModuleInfoEntry( dependentName );
                    if ( !dependencyModuleInfoEntry.IsNULL() )
                    {
                        if ( dependencyModuleInfoEntry->HasIndependentModuleTypeForPlatform( targetPlatform ) )
                        {
                            if ( !dependencyModuleInfoEntry->IsBroken() )
                            {
                                bool issueWasFixed = false;
                                if ( SanitizeModulePlatformUsage( moduleInfoEntry ) )
                                {
                                    // try again now that we have sanitized the module
                                    if ( TryGetModuleDependencyChain( dependentModule, dependentName, targetPlatform, false ) && !dependentModule.IsNULL() )
                                    {
                                        issueWasFixed = dependencyChain->SetRuntimeDependent( dependentModule );
                                        totalSuccess = issueWasFixed && totalSuccess;
                                    }
                                }
                                if ( !issueWasFixed )
                                {
                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:UpdateDependencyChain: Failed to obtain actual module for runtime dependent name: \"" +
                                        dependentName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                                    totalSuccess = false;
                                }
                            }
                            else
                            {
                                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChain: Ignoring broken runtime dependent with name: \"" +
                                    dependentName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                            }
                        }
                    }
                }
                ++d;
            }
        }
        else
        {
            totalSuccess = false;
        }
    }
    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::TryGetModuleDependencyChain( CModuleDependencyNodePtr& dependencyChain ,
                                           const CORE::CString& consensusModuleName  ,
                                           const CORE::CString& targetPlatform       ,
                                           bool onlyCheckPlatformSpecific            ) const
{GUCEF_TRACE;

    dependencyChain.Unlink();

    TStringToModuleDependencyNodePtrMap::const_iterator i = m_moduleDependencyChains.find( targetPlatform );
    if ( i != m_moduleDependencyChains.end() )
    {
        // we have chains for this platform, now look for the specific module
        const TModuleDependencyNodePtrMap& chainsForPlatform = (*i).second;
        TModuleDependencyNodePtrMap::const_iterator n = chainsForPlatform.find( consensusModuleName );
        if ( n != chainsForPlatform.end() )
        {
            // We found a chain for this platform & module consensus name combo
            dependencyChain = (*n).second;
            return true;
        }
    }

    if ( targetPlatform != AllPlatforms && !onlyCheckPlatformSpecific )
    {
        TStringToModuleDependencyNodePtrMap::const_iterator i = m_moduleDependencyChains.find( AllPlatforms );
        if ( i != m_moduleDependencyChains.end() )
        {
            // we have chains for this platform, now look for the specific module
            const TModuleDependencyNodePtrMap& chainsForPlatform = (*i).second;
            TModuleDependencyNodePtrMap::const_iterator n = chainsForPlatform.find( consensusModuleName );
            if ( n != chainsForPlatform.end() )
            {
                // We found a chain for this platform & module consensus name combo
                dependencyChain = (*n).second;
                return true;
            }
        }        
    }

    return false;
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::ClearDependencyChains( void )
{GUCEF_TRACE;

    TStringToModuleDependencyNodePtrMap::iterator i = m_moduleDependencyChains.begin();
    while ( i != m_moduleDependencyChains.end() )
    {
        TModuleDependencyNodePtrMap& map = (*i).second;
        TModuleDependencyNodePtrMap::iterator n = map.begin();
        while ( n != map.end() )
        {
            CModuleDependencyNodePtr& dependency = (*n).second;
            if ( !dependency.IsNULL() )
            {
                dependency->Clear();
            }
            ++n;
        }

        ++i;
    }
    m_moduleDependencyChains.clear();
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::Clear( void )
{GUCEF_TRACE;

    projectName.Clear();
    rootDirs.clear();
    modules.clear();
    dirProcessingInstructions.clear();
    globalDirExcludeList.clear();
    platforms.clear();
    ClearDependencyChains();
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::AreDependencyChainsInitialized( void ) const
{GUCEF_TRACE;

    return !m_actualPlatformsUsed.empty() && ( m_moduleDependencyChains.size() == m_actualPlatformsUsed.size() );
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::DetermineBuildOrderForAllModules( void )
{GUCEF_TRACE;

    // In order for this functionality to work we need the dependency chains determined ahead of time
    if ( !AreDependencyChainsInitialized() )
    {
        UpdateDependencyChains();
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DetermineBuildOrderForAllModules: Start determining build order for every platform" );

    // Important: First determine the build order which applies to all modules
    // This build order will be used for specific platforms as well unless no 'AllPlatforms'
    // target is available or if the build order differs
    // Doing it this way cuts down on the number platform specific entries generated just to store
    // the build order
    DetermineBuildOrderForAllModulesForPlatform( AllPlatforms );

    TStringToModuleDependencyNodePtrMap::iterator i = m_moduleDependencyChains.begin();
    while ( i != m_moduleDependencyChains.end() )
    {
        const CORE::CString& platformName = (*i).first;
        if ( platformName != AllPlatforms )
        {
            DetermineBuildOrderForAllModulesForPlatform( platformName );
        }
        ++i;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DetermineBuildOrderForAllModules: Finished determining build order for every platform" );
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::DetermineBuildOrderForAllModulesForPlatform( const CORE::CString& targetPlatform )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DetermineBuildOrderForAllModules: Start determining build order for platform " + targetPlatform );

    // Get the initial build leaf nodes
    // these are nodes which are truly leafs in that things may depend on them but they have no dependencies of their own

    TModuleDependencyNodePtrMap currentLevelNodes;
    GetBuildLeafNodes( targetPlatform, currentLevelNodes );

    Int32 buildIndex = 1;
    TStringToInt32Map buildOrderMap;

    // We take advantage of map alphabetic ordering
    TModuleDependencyNodePtrMap::iterator n = currentLevelNodes.begin();
    while ( n != currentLevelNodes.end() )
    {
        const CORE::CString& consensusName = (*n).first;
        buildOrderMap[ consensusName ] = buildIndex;

        ++buildIndex;
        ++n;
    }

    TModuleDependencyNodePtrMap nextLevelNodesMissingDeps;    
    do
    {
        // Now do a pass building the dependency pyramids for the current set of leaf nodes
        // As we do this more and more of the nodes with unsatisfied dependencies get their
        // dependencies fulfilled 

        TModuleDependencyNodePtrMap nextLevelNodes;
        do
        {
            nextLevelNodes.clear();
            GetNextLevelUpBuildNodes( targetPlatform, currentLevelNodes, buildOrderMap, nextLevelNodes, nextLevelNodesMissingDeps );

            n = nextLevelNodes.begin();
            while ( n != nextLevelNodes.end() )
            {
                const CORE::CString& consensusName = (*n).first;

                // Check if we haven't already assigned a build order
                // The same module can occur at different depth in the overall tree
                if ( buildOrderMap.find( consensusName ) == buildOrderMap.end() )
                {
                    buildOrderMap[ consensusName ] = buildIndex;
                    ++buildIndex;
                    nextLevelNodesMissingDeps.erase( consensusName );
                }

                ++n;
            }

            currentLevelNodes = nextLevelNodes;
        }
        while ( !nextLevelNodes.empty() );

        if ( !nextLevelNodesMissingDeps.empty() )
        {
            // We have gone as high as we can without missing dependencies
            // Now we reset based on whatever was left over and determine new leaf nodes
            
            GetUnprocessedBuildLeafNodes( buildOrderMap, nextLevelNodesMissingDeps, currentLevelNodes );

            TModuleDependencyNodePtrMap::iterator n = currentLevelNodes.begin();
            while ( n != currentLevelNodes.end() )
            {
                const CORE::CString& consensusName = (*n).first;

                // Check if we haven't already assigned a build order
                // The same module can occur at different depth in the overall tree
                if ( buildOrderMap.find( consensusName ) == buildOrderMap.end() )
                {
                    buildOrderMap[ consensusName ] = buildIndex;                    
                    ++buildIndex;
                    nextLevelNodesMissingDeps.erase( consensusName );
                }
                ++n;
            }
        }
    }
    while ( !currentLevelNodes.empty() );

    // Apply the build order information to the modules
    // Note that we put in extra effort here to avoid defining a platform specific
    // module definition just for the the sake of preserving the build order
    // if it matches the 'all' platform we just use that
    TStringToInt32Map::iterator o = buildOrderMap.begin();
    while ( o != buildOrderMap.end() )
    {
        const CORE::CString& consensusName = (*o).first;
        Int32 buildIndex = (*o).second; 

        CModuleInfoEntryPtr moduleEntry = GetModuleInfoEntry( consensusName );
        if GUCEF_PREDICT_TRUE( !moduleEntry.IsNULL() )
        {
            bool platformSpecificBuildOrderMatchesAllPlatforms = false;
            CModuleInfoPtr allPlatformsInfo = moduleEntry->FindModuleInfoForPlatform( AllPlatforms );
            if ( !allPlatformsInfo.IsNULL() )
            {
                if ( targetPlatform == AllPlatforms )
                {
                    allPlatformsInfo->buildOrder = buildIndex;
                    platformSpecificBuildOrderMatchesAllPlatforms = true;
                }
                else
                {
                    if ( -1 != allPlatformsInfo->buildOrder && allPlatformsInfo->buildOrder == buildIndex )
                    {
                        platformSpecificBuildOrderMatchesAllPlatforms = true;
                    }
                }                    
            }

            if ( !platformSpecificBuildOrderMatchesAllPlatforms )
            {
                CModuleInfoPtr moduleSpecForPlatform = moduleEntry->FindOrCreateModuleInfoForPlatform( targetPlatform, true );
                if GUCEF_PREDICT_TRUE( !moduleSpecForPlatform.IsNULL() )
                {
                    moduleSpecForPlatform->buildOrder = (*o).second;
                }                
            }
        }
        ++o;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DetermineBuildOrderForAllModules: Finished determining build order for platform " + targetPlatform );
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::GetUnprocessedBuildLeafNodes( const TStringToInt32Map& buildOrderMap                      ,
                                            const TModuleDependencyNodePtrMap& nodesThatWereMissingDeps ,
                                            TModuleDependencyNodePtrMap& newLeafNodes                   )
{GUCEF_TRACE;

    TModuleDependencyNodePtrMap::const_iterator i = nodesThatWereMissingDeps.begin();
    while ( i != nodesThatWereMissingDeps.end() )
    {
        const CORE::CString& consensusName = (*i).first;
        CModuleDependencyNodePtr dependent = (*i).second;

        if GUCEF_PREDICT_TRUE( !dependent.IsNULL() )
        {
            // The new leafs are not modules with 0 linker deps but rather those with 0 remaining
            // unprocessed dependencies

            bool hasUnprocessedDependency = false;
            const TModuleDependencyNodePtrMap& dependentDependencies = dependent->GetLinkerDependencies();
            TModuleDependencyNodePtrMap::const_iterator d = dependentDependencies.begin();
            while ( d != dependentDependencies.end() )
            {
                const CORE::CString& dependencyName = (*d).first;
                if ( buildOrderMap.find( dependencyName ) == buildOrderMap.end() )
                {
                    hasUnprocessedDependency = true;
                    break;
                }

                ++d;
            }

            if ( !hasUnprocessedDependency )
            {
                newLeafNodes[ consensusName ] = dependent;
            }
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::GetNextLevelUpBuildNodes( const CORE::CString& platform                          ,
                                        const TModuleDependencyNodePtrMap& currentLevel        ,
                                        const TStringToInt32Map& buildOrderMap                 ,
                                        TModuleDependencyNodePtrMap& nextLevelNodes            ,
                                        TModuleDependencyNodePtrMap& nextLevelNodesMissingDeps )
{GUCEF_TRACE;

    TModuleDependencyNodePtrMap::const_iterator n = currentLevel.begin();
    while ( n != currentLevel.end() )
    {
        const CORE::CString& consensusName = (*n).first;
        const CModuleDependencyNodePtr& chainNode = (*n).second;
        if GUCEF_PREDICT_TRUE( !chainNode.IsNULL() )
        {
            const TModuleDependencyNodePtrMap& dependents = chainNode->GetLinkerDependents();
            TModuleDependencyNodePtrMap::const_iterator i = dependents.begin();
            while ( i != dependents.end() )
            {
                CModuleDependencyNodePtr dependent = (*i).second;
                if GUCEF_PREDICT_TRUE( !dependent.IsNULL() )
                {
                    bool hasUnprocessedDependency = false;
                    const TModuleDependencyNodePtrMap& dependentDependencies = dependent->GetLinkerDependencies();
                    TModuleDependencyNodePtrMap::const_iterator d = dependentDependencies.begin();
                    while ( d != dependentDependencies.end() )
                    {
                        const CORE::CString& dependencyName = (*d).first;
                        if ( buildOrderMap.find( dependencyName ) == buildOrderMap.end() )
                        {
                            hasUnprocessedDependency = true;
                            break;
                        }

                        ++d;
                    }

                    if ( hasUnprocessedDependency )
                    {
                        nextLevelNodesMissingDeps[ dependent->GetConsensusName() ] = dependent;
                        nextLevelNodes.erase( dependent->GetConsensusName() );
                    }
                    else
                    {
                        nextLevelNodes[ dependent->GetConsensusName() ] = dependent;
                        nextLevelNodesMissingDeps.erase( dependent->GetConsensusName() );
                    }
                }
                ++i;
            }
        }
        ++n;
    }
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::GetBuildLeafNodes( const CORE::CString& platform          ,
                                 TModuleDependencyNodePtrMap& leafNodes )
{GUCEF_TRACE;

    TStringToModuleDependencyNodePtrMap::iterator i = m_moduleDependencyChains.find( platform );
    if ( i != m_moduleDependencyChains.end() )
    {
        TModuleDependencyNodePtrMap& chainNodeMap = (*i).second;
        TModuleDependencyNodePtrMap::iterator n = chainNodeMap.begin();
        while ( n != chainNodeMap.end() )
        {
            CModuleDependencyNodePtr& chainNode = (*n).second;
            if GUCEF_PREDICT_TRUE( !chainNode.IsNULL() )
            {
                if ( chainNode->GetLinkerDependencies().empty() )
                {
                    CModuleInfoEntryPtr module = chainNode->GetModule();
                    if GUCEF_PREDICT_TRUE( !module.IsNULL() )
                    {
                        const CORE::CString& consensusName = module->GetConsensusName();
                        leafNodes[ consensusName ] = chainNode;
                    }
                }
            }
            ++n;
        }
    }
}
/*-------------------------------------------------------------------------*/

void
CProjectInfo::GetNamesOfModulesWhichDependOnModuleForPlatform( const CORE::CString& targetPlatform ,
                                                               const CORE::CString& dependencyName ,
                                                               CORE::CStringSet& moduleNames       ,
                                                               bool platformSpecificOnly           ) const
{GUCEF_TRACE;

    // Loop trough all modules and check if they depend on the given module
    TMutableModuleInfoEntryPairVector resultList;
    TModuleInfoEntryPtrVector::const_iterator i = modules.begin();
    while ( i != modules.end() )
    {
        const CModuleInfoEntryPtr& moduleInfoEntry = (*i);
        if ( !moduleInfoEntry.IsNULL() )
        {
            if ( moduleInfoEntry->HasDependency( targetPlatform, dependencyName, platformSpecificOnly ) )
            {
                moduleNames.insert( moduleInfoEntry->GetConsensusName() );
            }
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CProjectInfo::GetNamesOfModulesWhichLinkerDependOnModuleForPlatform( const CORE::CString& targetPlatform ,
                                                                     const CORE::CString& dependencyName ,
                                                                     CORE::CStringSet& moduleNames       ,
                                                                     bool platformSpecificOnly           ) const
{GUCEF_TRACE;

    // Loop trough all modules and check if they depend on the given module
    TMutableModuleInfoEntryPairVector resultList;
    TModuleInfoEntryPtrVector::const_iterator i = modules.begin();
    while ( i != modules.end() )
    {
        const CModuleInfoEntryPtr& moduleInfoEntry = (*i);
        if ( !moduleInfoEntry.IsNULL() )
        {
            if ( moduleInfoEntry->HasLinkerDependency( targetPlatform, dependencyName, platformSpecificOnly ) )
            {
                moduleNames.insert( moduleInfoEntry->GetConsensusName() );
            }
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CProjectInfo::GetNamesOfModulesWhichRuntimeDependOnModuleForPlatform( const CORE::CString& targetPlatform ,
                                                                      const CORE::CString& dependencyName ,
                                                                      CORE::CStringSet& moduleNames       ,
                                                                      bool platformSpecificOnly           ) const
{GUCEF_TRACE;

    // Loop trough all modules and check if they depend on the given module
    TMutableModuleInfoEntryPairVector resultList;
    TModuleInfoEntryPtrVector::const_iterator i = modules.begin();
    while ( i != modules.end() )
    {
        const CModuleInfoEntryPtr& moduleInfoEntry = (*i);
        if ( !moduleInfoEntry.IsNULL() )
        {
            if ( moduleInfoEntry->HasDependency( targetPlatform, dependencyName, platformSpecificOnly ) )
            {
                moduleNames.insert( moduleInfoEntry->GetConsensusName() );
            }
            else
            if ( moduleInfoEntry->HasLinkerDependency( targetPlatform, dependencyName, platformSpecificOnly ) )
            {
                moduleNames.insert( moduleInfoEntry->GetConsensusName() );
            }
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CProjectInfo::GetModuleDependencies( const CModuleInfoEntryPtr& moduleInfoEntry ,
                                     const CORE::CString& targetPlatform        ,
                                     TModuleInfoEntryPtrSet& dependencies       ,
                                     bool includeDependenciesOfDependencies     ,
                                     bool includeRuntimeDependencies            ) const
{GUCEF_TRACE;

    // In order for this functionality to work we need the dependency chains determined ahead of time
    if ( !AreDependencyChainsInitialized() )
    {
        // We are no longer supporting doing this without dependency chains, it just takes too long
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetModuleDependencies: Determine the dependency chains before using this functionality" );
        return false;
    }

    const CORE::CString& moduleName = moduleInfoEntry->GetConsensusName();

    CModuleDependencyNodePtr dependencyChain;
    if ( TryGetModuleDependencyChain( dependencyChain  ,
                                      moduleName       ,
                                      targetPlatform   ,
                                      true             ) && !dependencyChain.IsNULL() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetModuleDependencies: Found dependency chain for module " + moduleName + " for platform " + targetPlatform );

        bool totalSuccess = true;

        totalSuccess = dependencyChain->GatherDependencyModules( dependencies, includeDependenciesOfDependencies ) && totalSuccess;
        totalSuccess = dependencyChain->GatherLinkerDependencyModules( dependencies, includeDependenciesOfDependencies ) && totalSuccess;
        if ( includeRuntimeDependencies )
            totalSuccess = dependencyChain->GatherRuntimeDependencyModules( dependencies, includeDependenciesOfDependencies ) && totalSuccess;

        return totalSuccess;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CProjectInfo::GetModuleDependencies( const CModuleInfoEntryPtr& moduleInfoEntry ,
                                     const CORE::CString& targetPlatform        ,
                                     TStringSet& dependencies                   ,
                                     bool includeRuntimeDependencies            ) const
{GUCEF_TRACE;

    CModuleInfoEntryPtr mutableModuleInfoEntry = moduleInfoEntry;
    CModuleInfoPtr moduleInfo = mutableModuleInfoEntry->FindOrCreateModuleInfoForPlatform( targetPlatform, false );
    if ( !moduleInfo.IsNULL() )
    {
        MergeStringSet( dependencies, moduleInfo->GetNamesOfDependencies(), false );
        if ( includeRuntimeDependencies )
            MergeStringSet( dependencies, moduleInfo->runtimeDependencies, false );
    }
    if ( targetPlatform != AllPlatforms && !targetPlatform.IsNULLOrEmpty() )
    {
        moduleInfo = mutableModuleInfoEntry->FindOrCreateModuleInfoForPlatform( AllPlatforms, false );
        if ( !moduleInfo.IsNULL() )
        {
            MergeStringSet( dependencies, moduleInfo->GetNamesOfDependencies(), false );
            if ( includeRuntimeDependencies )
                MergeStringSet( dependencies, moduleInfo->runtimeDependencies, false );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CProjectInfo::FindModulesWhichDependOnModuleForPlatform( TMutableModuleInfoEntryPairVector& foundModules ,
                                                         const CORE::CString& targetPlatform             ,
                                                         const CORE::CString& dependencyName             ,
                                                         bool tryToUseDependencyChains                   ) const
{GUCEF_TRACE;

    if ( tryToUseDependencyChains )
    {
        // Use the dependency chains whenever possible for this kind of thing
        CModuleDependencyNodePtr dependencyChain;
        if ( TryGetModuleDependencyChain( dependencyChain  ,
                                          dependencyName   ,
                                          targetPlatform   ,
                                          true             ) && !dependencyChain.IsNULL() )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:FindModulesWhichDependOnModuleForPlatform: Found dependency chain for " + dependencyName + " for platform " + targetPlatform );

            TModuleInfoEntryPtrSet dependents;
            if ( dependencyChain->GatherDependentModules( dependents, false ) )
            {
                TModuleInfoEntryPtrSet::iterator n = dependents.begin();
                while ( n != dependents.end() )
                {
                    const CModuleInfoEntryPtr& dependent = (*n);
                    if GUCEF_PREDICT_TRUE( !dependent.IsNULL() )
                    {
                        CModuleInfoPtr platformSpecific = dependent->FindModuleInfoForPlatform( targetPlatform, true );
                        TMutableModuleInfoEntryPair infoPair( dependent, platformSpecific );
                        foundModules.push_back( infoPair );
                    }
                    ++n;
                }
            }
            return true;
        }
    }

    // brute force method
    // Loop trough all modules and check if they depend on the given module    
    TModuleInfoEntryPtrVector::const_iterator i = modules.begin();
    while ( i != modules.end() )
    {
        const CModuleInfoEntryPtr& moduleInfoEntry = (*i);
        TModuleInfoPtrMap::const_iterator n = moduleInfoEntry->GetModulesPerPlatform().find( targetPlatform );
        if ( n != moduleInfoEntry->GetModulesPerPlatform().end() )
        {
            // This module has info for for this module, check its type
            CModuleInfoPtr moduleInfo = (*n).second;
            if ( IsStringInList( moduleInfo->GetNamesOfDependencies(), false, dependencyName ) )
            {
                TMutableModuleInfoEntryPair infoPair( moduleInfoEntry, moduleInfo );
                foundModules.push_back( infoPair );
            }
        }

        ++i;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CProjectInfo::FindModulesWhichDependOnModule( TMutableModuleInfoEntryPairVector& foundModules ,
                                              const CORE::CString& dependencyName             ,
                                              bool tryToUseDependencyChains                   ) const
{GUCEF_TRACE;

    TMutableModuleInfoEntryPairVector results;

    // we specifically use a platforms list which is sourced from our flat list of modules
    // so that we can ensure that we have all platforms used by the modules
    TStringSet platformList;
    GetAllPlatformsUsed( platformList, true );

    // Loop trough all platforms
    bool totalSuccess = true;
    TStringSet::const_iterator i = platformList.begin();
    while ( i != platformList.end() )
    {
        totalSuccess = FindModulesWhichDependOnModuleForPlatform( foundModules, (*i), dependencyName, tryToUseDependencyChains ) && totalSuccess;
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CProjectInfo::FindModulesWithModuleType( TModuleType moduleType               ,
                                         TModuleInfoEntryPtrSet& foundModules ) const
{GUCEF_TRACE;

    // Loop trough all modules and process each code as we go
    TModuleInfoEntryPtrVector::const_iterator i = modules.begin();
    while ( i != modules.end() )
    {
        const CModuleInfoEntryPtr& moduleInfoEntry = (*i);
        if GUCEF_PREDICT_TRUE( !moduleInfoEntry.IsNULL() )
        {
            if ( moduleInfoEntry->HasAnyModuleInfoWithModuleType( moduleType ) )
            {
                foundModules.insert( moduleInfoEntry );
            }
        }

        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CProjectInfo::MergeBinaryPackageInfoIntoModules( CModuleInfoEntryPtr binaryPackageModule )
{GUCEF_TRACE;

    // When we find a binary package we need to add its information to the modules that depend on it
    // its not a module in the same sense as the other modules, its a way to designate a dependency
    // We now resolved and integrate that dependency

    if GUCEF_PREDICT_FALSE( binaryPackageModule.IsNULL() )
        return false;

    TModuleInfoPtrSet binaryPackageInfoPerPlatform;
    binaryPackageModule->FindAllModuleInfoWithModuleType( MODULETYPE_BINARY_PACKAGE, binaryPackageInfoPerPlatform );

    bool totalSuccess = true;

    TModuleInfoPtrSet::iterator p = binaryPackageInfoPerPlatform.begin();
    while ( p != binaryPackageInfoPerPlatform.end() )
    {
        const CModuleInfoPtr& moduleInfo = (*p);
        const CORE::CString& targetPlatform = moduleInfo->GetPlatformName();
        CORE::CString moduleName = binaryPackageModule->GetModuleNameAlways( targetPlatform );

        TMutableModuleInfoEntryPairVector links;
        FindModulesWhichDependOnModuleForPlatform( links, targetPlatform, moduleName, false );

        TMutableModuleInfoEntryPairVector::iterator m = links.begin();
        while ( m != links.end() )
        {
            CModuleInfoEntryPtr depModuleInfoEntry = (*m).first;
            CModuleInfoPtr depModuleInfo = (*m).second;

            if ( !depModuleInfoEntry.IsNULL() )
            {
                if ( !depModuleInfo.IsNULL() )
                {
                    const CORE::CString& targetPlatformOfDependent = depModuleInfo->GetPlatformName();

                    // For this library replace the binary package reference with the information from said message
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:MergeBinaryPackageInfoIntoModules: Replacing binary package reference \"" + moduleName +
                        "\" for package platform " + moduleInfo->GetPlatformName() + " and dependent platform " + targetPlatformOfDependent + " with information from said package for module: " + depModuleInfoEntry->GetConsensusName() );

                    // Not every library may need every part of a dependency
                    // some might elect to only use the linker dependency, others also the general dependency for headers, 
                    // some only that and no linker dependency.
                    // Point being, respect what is specified

                    bool mergeSuccess = true;

                    if ( depModuleInfoEntry->HasLinkerDependency( targetPlatformOfDependent, moduleName, true ) )
                    {
                        // We need to merge in the linker dependencies and paths (additive)
                        mergeSuccess = depModuleInfo->linkerSettings.MoveLinkedLibraryToLogicalLibraries( moduleName ) && mergeSuccess;
                        mergeSuccess = depModuleInfo->linkerSettings.MergeLinkedLibraries( moduleInfo->linkerSettings, true ) && mergeSuccess;
                    }

                    if ( depModuleInfoEntry->HasDependency( targetPlatformOfDependent, moduleName, true ) )
                    {
                        // we also need to merge in any include paths for any headers provided with the binary                    
                        TStringSet binaryPackagePlatformIncludeDirs;
                        binaryPackageModule->GetPathsToIncludeDirsForPlatform( targetPlatform, depModuleInfoEntry->rootDir, binaryPackagePlatformIncludeDirs, false );
                        depModuleInfo->AddDependencyIncludeDirs( binaryPackagePlatformIncludeDirs );
                        depModuleInfo->MoveDependencyToLogicalDependencies( moduleName );

                        if ( AllPlatforms != targetPlatform )
                        {
                            // Unlikely but if there are any 'all' platforms headers for a binary package put them on the 'all' platforms definition of the dependent
                            // module if there is one
                            TStringSet binaryPackageIncludeDirs;
                            binaryPackageModule->GetPathsToIncludeDirsForPlatform( AllPlatforms, depModuleInfoEntry->rootDir, binaryPackageIncludeDirs, false );
                            if ( !binaryPackageIncludeDirs.empty() )
                            {
                                if ( targetPlatformOfDependent == AllPlatforms )
                                {
                                    depModuleInfo->AddDependencyIncludeDirs( binaryPackageIncludeDirs );
                                }
                                else
                                {
                                    CModuleInfoPtr allPlatformsDepModuleInfo = depModuleInfoEntry->FindModuleInfoForPlatform( AllPlatforms );
                                    if ( !allPlatformsDepModuleInfo.IsNULL() )
                                    {
                                        allPlatformsDepModuleInfo->AddDependencyIncludeDirs( binaryPackageIncludeDirs );
                                        allPlatformsDepModuleInfo->MoveDependencyToLogicalDependencies( moduleName );
                                    }
                                    else
                                    {
                                        depModuleInfo->AddDependencyIncludeDirs( binaryPackageIncludeDirs );
                                    }
                                }
                            }
                        }
                    }

                    if ( !mergeSuccess )
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to merge binary package reference \"" + moduleName +
                            "\" for platform " + moduleInfo->GetPlatformName() + " information into module: " + depModuleInfoEntry->GetConsensusName() );
                    }
                    totalSuccess = mergeSuccess && totalSuccess;
                }
                
            }
            ++m;
        }

        ++p;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CProjectInfo::MergeAllBinaryPackageInfoIntoModules( void )
{GUCEF_TRACE;

    TModuleInfoEntryPtrSet binaryPackageModules;
    if ( FindModulesWithModuleType( MODULETYPE_BINARY_PACKAGE, binaryPackageModules ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:MergeAllBinaryPackageInfoIntoModules: Found " + CORE::ToString( binaryPackageModules.size() ) + " binary packages" );
        bool totalSuccess = true;

        TModuleInfoEntryPtrSet::iterator i = binaryPackageModules.begin();
        while ( i != binaryPackageModules.end() )
        {
            const CModuleInfoEntryPtr& moduleInfoEntry = (*i);
            totalSuccess = MergeBinaryPackageInfoIntoModules( moduleInfoEntry ) && totalSuccess;
            ++i;
        }

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:MergeAllBinaryPackageInfoIntoModules: Finished merging " + CORE::ToString( binaryPackageModules.size() ) +
            " binary packages. totalSuccess=" + CORE::ToString( totalSuccess ) );
        return totalSuccess;
    }
    return false;
}

/*---------------------------------------------------------------------------*/
               
bool 
CProjectInfo::Serialize( CORE::CDataNode& domRootNode                        ,
                         const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    // start from a clean slate
    domRootNode.Clear();

    // Add project info
    domRootNode.SetName( "Project" );
    domRootNode.SetAttribute( "ModuleCount", CORE::ToString( modules.size() ) );
    domRootNode.SetAttribute( "Name", projectName );

    // Add info for each module
    TModuleInfoEntryPtrVector::const_iterator i = modules.begin();
    while ( i != modules.end() )
    {
        if ( !SerializeModuleInfo( (*i)        ,
                                   domRootNode ) )
        {
            // Failed to serialize this module
            return false;
        }
        ++i;
    }

    return true;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::DeserializeModuleEntries( const CORE::CDataNode& domRootNode                  ,
                                        const CORE::CDataNodeSerializableSettings& settings ,
                                        TModuleInfoEntryPtrVector& moduleInfoEntries        )
{GUCEF_TRACE;

    CORE::CDataNode::TConstDataNodeSet nodeSet = domRootNode.FindChildrenOfType( "ModuleInfoEntry", true );
    if ( nodeSet.empty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DeserializeModuleEntries: No ModuleInfoEntry nodes were found in the document" );
        return false;
    }

    UInt32 errorCount = 0;
    UInt32 modulesAdded = 0;
    CORE::CDataNode::TConstDataNodeSet::const_iterator i = nodeSet.begin();
    while ( i != nodeSet.end() )
    {
        CModuleInfoEntryPtr newModuleInfo = CModuleInfoEntry::CreateSharedObj();
        if ( !newModuleInfo.IsNULL() )
        {                                         
            const CORE::CDataNode& domRootNode = *(*i);
            if ( newModuleInfo->Deserialize( domRootNode, settings ) )
            {
                moduleInfoEntries.push_back( newModuleInfo );
                ++modulesAdded;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeProjectInfo: Failed deserialize module info entry" );
                ++errorCount;
            }
        }
        ++i;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DeserializeModuleEntries: Added " + CORE::ToString( modulesAdded ) +
        " modules and experienced " + CORE::ToString( errorCount ) + " errors" );

    return errorCount == 0;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::DeserializeModuleEntries( const CORE::CString& pathToModuleInfoFile           ,
                                        const CORE::CDataNodeSerializableSettings& settings ,
                                        TModuleInfoEntryPtrVector& moduleInfoEntries        )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec = GetXmlDStoreCodec();
    if ( !codec.IsNULL() )
    {
        CORE::CDataNode rootNode;
        if ( codec->BuildDataTree( &rootNode, pathToModuleInfoFile ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DeserializeModuleEntries: Successfully loaded module information from file \"" + pathToModuleInfoFile + "\", now we will parse the information" );

            if ( DeserializeModuleEntries( rootNode, settings, moduleInfoEntries ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DeserializeModuleEntries: Successfully deserialized information from file \"" + pathToModuleInfoFile + "\"" );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeModuleInfo: Failed to deserialize module information from file at " + pathToModuleInfoFile );
                return false;
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DeserializeModuleEntries: Failed to load the module information from file at " + pathToModuleInfoFile );
            return false;
        }
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DeserializeModuleEntries: Cannot deserialize since no codec is registered that can be used for deserialization" );
    return false;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::DeserializeModuleEntries( const CORE::CDataNode& domRootNode                  ,
                                        const CORE::CDataNodeSerializableSettings& settings ,
                                        size_t suggestedNrOfModules                         )
{GUCEF_TRACE;

    TModuleInfoEntryPtrVector moduleInfoEntries;
    if ( suggestedNrOfModules > 0 )
        moduleInfoEntries.reserve( suggestedNrOfModules );
        
    if ( DeserializeModuleEntries( domRootNode       ,
                                   settings          ,
                                   moduleInfoEntries ) )
    {
        // Add the modules to the project
        TModuleInfoEntryPtrVector::const_iterator i = moduleInfoEntries.begin();
        while ( i != moduleInfoEntries.end() )
        {
            const CModuleInfoEntryPtr& newModuleInfo = (*i);
            if ( !newModuleInfo.IsNULL() )
            {
                modules.push_back( newModuleInfo );
            }
            ++i;
        }
        return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::Deserialize( const CORE::CDataNode& domRootNode                  ,
                           const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    const CORE::CDataNode* node = domRootNode.Search( "Project", '\\', true );
    if ( GUCEF_NULL == node )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeProjectInfo: Failed locate project root data node" );
        return false;
    }

    projectName = node->GetAttributeValueOrChildValueByName( "Name" ).AsString( projectName );
    size_t suggestedNrOfModules = node->GetAttributeValueOrChildValueByName( "ModuleCount" ).AsSizeT();

    return DeserializeModuleEntries( *node, settings, suggestedNrOfModules );
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::Deserialize( const CORE::CString& inputFilepath )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec = GetXmlDStoreCodec();
    if ( !codec.IsNULL() )
    {
        CORE::CDataNode rootNode;
        if ( codec->BuildDataTree( &rootNode, inputFilepath ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeModuleInfo: Successfully loaded module information from file \"" + inputFilepath + "\", now we will parse the information" );

            // @TODO: temp hack to get ProjectInfo into the deserializer which is needed for normalization
            CORE::CDataNodeSerializableSettings defaultSerializableSettings;
            defaultSerializableSettings.SetUserData( const_cast< CProjectInfo* >( this ) );

            if ( Deserialize( rootNode, defaultSerializableSettings ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeModuleInfo: Successfully deserialized information from file \"" + inputFilepath + "\"" );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeModuleInfo: Failed to deserialize module information from file at " + inputFilepath );
                return false;
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeModuleInfo: Failed to load the module information from file at " + inputFilepath );
            return false;
        }
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeModuleInfo: Cannot deserialize since no codec is registered that can be used for deserialization" );
    return false;
}

/*---------------------------------------------------------------------------*/

CORE::CICloneable*
CProjectInfo::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CProjectInfo( *this );
}

/*---------------------------------------------------------------------------*/

const CORE::CString&
CProjectInfo::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
