/*
 *  ProjectGenerator: Tool to generate module/project files
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
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
                bool caseSensitive             ,
                bool concatExistingEntries     )
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
GetLanguageForModule( const CModuleInfo& moduleInfo )
{GUCEF_TRACE;

    const TStringSet& languageSet = moduleInfo.compilerSettings.languagesUsed;
    if ( languageSet.empty() )
    {
        // No language was specified but premake requires one
        // We will determine the languege based on the files in the project
        if ( ContainsFileWithFileExtension( moduleInfo.sourceDirs, "CS" ) )
        {
            return "C#";
        }
        else
        if ( ( ContainsFileWithFileExtension( moduleInfo.sourceDirs, "CPP" ) ) ||
             ( ContainsFileWithFileExtension( moduleInfo.sourceDirs, "CXX" ) ) ||
             ( ContainsFileWithFileExtension( moduleInfo.sourceDirs, "CHH" ) ) )
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
SerializeModuleInfo( const CModuleInfoEntry& moduleEntry ,
                     const CModuleInfo& moduleInfo       ,
                     const CORE::CString& platform       ,
                     CORE::CDataNode& parentNode         )
{GUCEF_TRACE;

    // Add basic module info
    CORE::CDataNode moduleInfoNode;
    moduleInfoNode.SetName( "Module" );
    
    if ( !moduleInfo.name.IsNULLOrEmpty() )
    {
        moduleInfoNode.SetAttribute( "Name", moduleInfo.name );
    }
    moduleInfoNode.SetAttribute( "SemVer", moduleEntry.metadata.semver.ToString() );
    if ( moduleInfo.buildOrder != -1 )
    {
        moduleInfoNode.SetAttribute( "BuildOrder", CORE::Int32ToString( moduleInfo.buildOrder ) );
    }
    if ( moduleInfo.buildChain != -1 )
    {
        moduleInfoNode.SetAttribute( "BuildChain", CORE::Int32ToString( moduleInfo.buildChain ) );
    }
    if ( !moduleInfo.buildChainDependencies.empty() )
    {
        TInt32Set::iterator n = moduleInfo.buildChainDependencies.begin();
        CORE::CString buildChainDepsValue;
        for ( UInt32 i=0; i+1 < moduleInfo.buildChainDependencies.size(); ++i )
        {                     
            buildChainDepsValue += (*n) + ';';
            ++n;
        }
        buildChainDepsValue += (*n);
        moduleInfoNode.SetAttribute( "BuildChainDeps", buildChainDepsValue );
    }
    if ( moduleInfo.moduleType != MODULETYPE_UNDEFINED )
    {
        moduleInfoNode.SetAttribute( "Type", ModuleTypeToString( moduleInfo.moduleType ) );
    }
    if ( !moduleInfo.tags.empty() )
    {
        UInt32 i=0;
        CORE::CString tagValue;
        TStringSet::iterator s = moduleInfo.tags.begin();
        while ( s != moduleInfo.tags.end() )
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
    if ( moduleInfo.includeDirs.size() > 0 )
    {
        CORE::CDataNode headersInfoNode;
        headersInfoNode.SetName( "Files" );
        headersInfoNode.SetAttribute( "Type", "Headers" );
        headersInfoNode.SetAttribute( "DirCount", CORE::ToString( moduleInfo.includeDirs.size() ) );
        TStringSetMap::const_iterator n = moduleInfo.includeDirs.begin();
        while ( n != moduleInfo.includeDirs.end() )
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
    if ( moduleInfo.sourceDirs.size() > 0 )
    {
        CORE::CDataNode sourceInfoNode;
        sourceInfoNode.SetName( "Files" );
        sourceInfoNode.SetAttribute( "Type", "Source" );
        sourceInfoNode.SetAttribute( "DirCount", CORE::ToString( moduleInfo.sourceDirs.size() ) );
        TStringSetMap::const_iterator n = moduleInfo.sourceDirs.begin();
        while ( n != moduleInfo.sourceDirs.end() )
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
    if ( moduleInfo.dependencyIncludeDirs.size() > 0 )
    {
        CORE::CDataNode includesInfoNode( "Includes" );
        includesInfoNode.SetAttribute( "Count", CORE::ToString( moduleInfo.dependencyIncludeDirs.size() ) );
        includesInfoNode.SetAttribute( "Source", "Dependency" );
        TStringSet::const_iterator q = moduleInfo.dependencyIncludeDirs.begin();
        while ( q != moduleInfo.dependencyIncludeDirs.end() )
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
    if ( moduleInfo.includeDirs.size() > 0 )
    {
       CORE::CDataNode includesInfoNode( "Includes" );
        includesInfoNode.SetAttribute( "Count", CORE::ToString( moduleInfo.includeDirs.size() ) );
        includesInfoNode.SetAttribute( "Source", "Self" );
        TStringSetMap::const_iterator n = moduleInfo.includeDirs.begin();
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
                    CORE::CString includeDir = "../" + CORE::LastSubDir( moduleEntry.rootDir ) + " ";
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
    if ( moduleInfo.dependencies.size() > 0 )
    {
        CORE::CDataNode dependenciesNode;
        dependenciesNode.SetName( "Dependencies" );
        dependenciesNode.SetAttribute( "Count", CORE::ToString( moduleInfo.dependencies.size() ) );
        TStringSet::const_iterator m = moduleInfo.dependencies.begin();
        while ( m != moduleInfo.dependencies.end() )
        {
            CORE::CDataNode dependencyNode;
            dependencyNode.SetName( "Dependency" );
            dependencyNode.SetAttribute( "Name", (*m) );
            dependenciesNode.AddChild( dependencyNode );
            ++m;
        }
        moduleInfoNode.AddChild( dependenciesNode );
    }

    // Add all the runtime module dependencies
    if ( moduleInfo.runtimeDependencies.size() > 0 )
    {
        CORE::CDataNode dependenciesNode;
        dependenciesNode.SetName( "RuntimeDependencies" );
        dependenciesNode.SetAttribute( "Count", CORE::ToString( moduleInfo.runtimeDependencies.size() ) );
        TStringSet::const_iterator m = moduleInfo.runtimeDependencies.begin();
        while ( m != moduleInfo.runtimeDependencies.end() )
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
    if ( moduleInfo.preprocessorSettings.defines.size() > 0 )
    {
        CORE::CDataNode preprocessorNode;
        preprocessorNode.SetName( "Preprocessor" );
        TStringSet::const_iterator m = moduleInfo.preprocessorSettings.defines.begin();
        while ( m != moduleInfo.preprocessorSettings.defines.end() )
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
    if ( moduleInfo.linkerSettings.linkedLibraries.size() > 0 )
    {
        addedLinkedSettings = true;
        TLinkedLibrarySettingsMap::const_iterator m = moduleInfo.linkerSettings.linkedLibraries.begin();
        while ( m != moduleInfo.linkerSettings.linkedLibraries.end() )
        {
            CORE::CDataNode libraryNode;
            libraryNode.SetName( "Dependency" );
            libraryNode.SetAttribute( "Name", (*m).first );

            TModuleType linkedLibType = (*m).second.moduleType;
            if ( ( MODULETYPE_UNDEFINED == linkedLibType ) ||
                 ( MODULETYPE_UNKNOWN == linkedLibType )    )
            {
                libraryNode.SetAttribute( "Type", ModuleTypeToString( linkedLibType ) );
            }
            if ( !(*m).second.libPath.IsNULLOrEmpty() )
            {
                libraryNode.SetAttribute( "Path", (*m).second.libPath );
            }
            linkerNode.AddChild( libraryNode );
            ++m;
        }

    }
    if ( !moduleInfo.linkerSettings.targetName.IsNULLOrEmpty() )
    {
        addedLinkedSettings = true;
        linkerNode.SetAttribute( "TargetName", moduleInfo.linkerSettings.targetName ); 
    }

    if ( !moduleInfo.linkerSettings.libPaths.empty() )
    {
        CORE::CString libPaths;
        TStringSet::const_iterator r = moduleInfo.linkerSettings.libPaths.begin();
        while ( r != moduleInfo.linkerSettings.libPaths.end() )
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
SerializeModuleInfo( const CModuleInfoEntry& moduleEntry ,
                     CORE::CDataNode& parentNode         )
{GUCEF_TRACE;

    CORE::CDataNode* node = parentNode.AddChild( "ModuleInfoEntry" );
    if ( GUCEF_NULL != node )
    {       
        CORE::CDataNodeSerializableSettings defaultSerializableSettings;
        if ( moduleEntry.Serialize( *node, defaultSerializableSettings ) )
        {
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
SerializeModuleInfo( const CModuleInfoEntry& moduleEntry ,
                     const CORE::CString& outputFilepath )
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
SerializeProjectInfo( const CProjectInfo& projectInfo ,
                      CORE::CDataNode& rootNodeToBe   )
{GUCEF_TRACE;

    // start from a clean slate
    rootNodeToBe.Clear();

    // Add project info
    rootNodeToBe.SetName( "Project" );
    rootNodeToBe.SetAttribute( "ModuleCount", CORE::ToString( projectInfo.modules.size() ) );
    rootNodeToBe.SetAttribute( "Name", projectInfo.projectName );

    // Add info for each module
    TModuleInfoEntryVector::const_iterator i = projectInfo.modules.begin();
    while ( i != projectInfo.modules.end() )
    {
        if ( !SerializeModuleInfo( (*i)         ,
                                   rootNodeToBe ) )
        {
            // Failed to serialize this module
            return false;
        }
        ++i;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
DeserializeProjectInfo( CProjectInfo& projectInfo       ,
                        const CORE::CDataNode& rootNode )
{GUCEF_TRACE;

    const CORE::CDataNode* node = rootNode.Search( "Project", '\\', true );
    if ( NULL == node )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeProjectInfo: Failed locate project root data node" );
        return false;
    }

    int errorCount = 0;
    projectInfo.projectName = node->GetAttributeValueOrChildValueByName( "Name" );
    CORE::CDataNode::TConstDataNodeSet nodeSet = node->FindChildrenOfType( "ModuleInfoEntry" );
    CORE::CDataNode::TConstDataNodeSet::const_iterator i = nodeSet.begin();
    while ( i != nodeSet.end() )
    {
        CModuleInfoEntry newModuleInfo;
        projectInfo.modules.push_back( newModuleInfo );
        CModuleInfoEntry& moduleInfoEntry = projectInfo.modules.back();

        if ( !DeserializeModuleInfo( projectInfo, moduleInfoEntry, *(*i) ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeProjectInfo: Failed deserialize module info entry" );
            ++errorCount;
        }
        
        ++i;
    }

    return errorCount == 0;
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
        if ( SerializeProjectInfo( projectInfo, info ) )
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

            if ( DeserializeProjectInfo( projectInfo, info ) )
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
DeserializeModuleInfo( CModuleInfo& moduleInfo           ,
                       const CORE::CDataNode& parentNode )
{GUCEF_TRACE;

    const CORE::CDataNode* moduleInfoNode = parentNode.Find( "Module" );
    if ( GUCEF_NULL == moduleInfoNode ) 
        return false;

    // Find the overall module properties
    CORE::CString tmpStr = moduleInfoNode->GetAttributeValue( "BuildOrder", "-1" );
    moduleInfo.buildOrder = CORE::StringToInt32( tmpStr );
    tmpStr = moduleInfoNode->GetAttributeValue( "BuildChain", "-1" );
    moduleInfo.buildChain = CORE::StringToInt32( tmpStr );
    tmpStr = moduleInfoNode->GetAttributeValue( "BuildChainDeps" );
    if ( !tmpStr.IsNULLOrEmpty() )
    {
        TStringVector tmpStrVec = tmpStr.ParseElements( ';', false );
        if ( tmpStrVec.empty() )
        {
            for ( UInt32 i=0; i<tmpStrVec.size(); ++i )
            {
                Int32 chainId = CORE::StringToInt32( tmpStrVec[ i ] );
                moduleInfo.buildChainDependencies.insert( chainId );
            }
        }
    }
    moduleInfo.tags = StringVectorToStringSet( moduleInfoNode->GetAttributeValue( "Tags" ).AsString().ParseElements( ';', false ) );
    moduleInfo.moduleType = StringToModuleType( moduleInfoNode->GetAttributeValue( "Type" ) );
    
    if ( moduleInfoNode->IsAttribute( "ConsiderSubDirs" ) )
    {
        moduleInfo.considerSubDirs = moduleInfoNode->GetAttributeValue( "ConsiderSubDirs", moduleInfo.considerSubDirs, false ).AsBool( moduleInfo.considerSubDirs, true );
        moduleInfo.hasConsiderSubDirs = true;
    }
    else
        moduleInfo.hasConsiderSubDirs = false;

    if ( moduleInfoNode->IsAttribute( "IgnoreModule" ) )
    {
        moduleInfo.ignoreModule = moduleInfoNode->GetAttributeValue( "IgnoreModule", moduleInfo.ignoreModule, false ).AsBool( moduleInfo.ignoreModule, true );
        moduleInfo.hasIgnoreModule = true;
    }
    else
        moduleInfo.hasIgnoreModule = false;

    // Check to see if a name was defined
    tmpStr = moduleInfoNode->GetAttributeValue( "Name" );
    if ( !tmpStr.IsNULLOrEmpty() )
    {
        moduleInfo.name = tmpStr;
    }

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
                    moduleInfo.includeDirs[ path ].insert( filename );
                }
                else
                if ( filesType == "Source" )
                {
                    // We have a list of source files
                    moduleInfo.sourceDirs[ path ].insert( filename );
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

                moduleInfo.dependencyIncludeDirs.insert( path );
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

                if ( moduleInfo.includeDirs[ path ].empty() )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeModuleInfo: Adding include dir of source \"self\" which does not currently have a reference to include files: " + path );
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
                moduleInfo.dependencies.insert( dependencyName );
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
                moduleInfo.runtimeDependencies.insert( dependencyName );
            }
            ++i;
        }
    }

    // Go through all linker related settings, if any exist
    const CORE::CDataNode* linkerNode = moduleInfoNode->Find( "Linker" );
    if ( NULL != linkerNode )
    {
        moduleInfo.linkerSettings.targetName = linkerNode->GetAttributeValueOrChildValueByName( "TargetName" );
        
        // If general paths were defined at the linker level add them to each dependency
        // They should be added after the dependency paths this could have been added above because
        // those should take priority in a linker search for paths        
        moduleInfo.linkerSettings.libPaths = StringVectorToStringSet( linkerNode->GetAttributeValueOrChildValueByName( "LibPaths" ).AsString().ParseElements( ';' ) );
        
        // Find all the libraries that are linked but not part of the overall project
        CORE::CDataNode::TConstDataNodeSet linkedLibs = linkerNode->FindChildrenOfType( "Dependency" );
        i = linkedLibs.begin();
        while ( i != linkedLibs.end() )
        {
            const CORE::CDataNode* linkedLibNode = (*i);
            CORE::CString linkedLibName = linkedLibNode->GetAttributeValue( "Name" );
            if ( !linkedLibName.IsNULLOrEmpty() )
            {
                TLinkedLibrarySettings& libSettings = moduleInfo.linkerSettings.linkedLibraries[ linkedLibName ]; 
                
                CORE::CString linkedLibType = linkedLibNode->GetAttributeValue( "Type" );
                if ( !linkedLibType.IsNULLOrEmpty() )
                {
                    libSettings.moduleType = StringToModuleType( linkedLibType );
                }
                else
                {
                    libSettings.moduleType = MODULETYPE_UNDEFINED;
                }

                libSettings.libPath = linkedLibNode->GetAttributeValue( "Path" );
            }
            ++i;
        }
    }

   // Go through all preprocessor related settings, if any exist
    const CORE::CDataNode* preprocessorNode = moduleInfoNode->Find( "Preprocessor" );
    if ( NULL != preprocessorNode )
    {
        // Find all the preprocessor definitions
        CORE::CDataNode::TConstDataNodeSet defines = preprocessorNode->FindChildrenOfType( "Define" );
        i = defines.begin();
        while ( i != defines.end() )
        {
            const CORE::CDataNode* defineNode = (*i);
            CORE::CString defineValue = defineNode->GetAttributeValue( "String" );
            if ( !defineValue.IsNULLOrEmpty() )
            {
                moduleInfo.preprocessorSettings.defines.insert( defineValue );
            }
            ++i;
        }
    }

   // Go through all preprocessor related settings, if any exist
    const CORE::CDataNode* compilerNode = moduleInfoNode->Find( "Compiler" );
    if ( NULL != compilerNode )
    {
        // Find all the code languages defined for the compiler
        CORE::CDataNode::TConstDataNodeSet languages = compilerNode->FindChildrenOfType( "Language" );
        i = languages.begin();
        while ( i != languages.end() )
        {
            const CORE::CDataNode* languageNode = (*i);
            CORE::CString name = languageNode->GetAttributeValue( "Name" );
            if ( !name.IsNULLOrEmpty() )
            {
                moduleInfo.compilerSettings.languagesUsed.insert( name );
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
                moduleInfo.compilerSettings.compilerFlags[ compilerName ] = flags;
            }
            ++i;
        }
    }

    return true;
}

/*-------------------------------------------------------------------------*/

const CModuleInfo*
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
    return NULL;
}

/*-------------------------------------------------------------------------*/

void
CleanupIncludeDirs( CModuleInfoEntry& moduleInfoEntry )
{GUCEF_TRACE;

    TModuleInfoMap::iterator i = moduleInfoEntry.modulesPerPlatform.begin();
    while ( i != moduleInfoEntry.modulesPerPlatform.end() )
    {
        CModuleInfo& moduleInfo = (*i).second;
        
        // Check for empty include dirs
        // If the include dir does not have include files as part of this module then 
        // the dir should have been a dependency include dir
        TStringSet dirs;
        TStringSetMap::iterator n = moduleInfo.includeDirs.begin();
        while ( n != moduleInfo.includeDirs.end() )
        {
            TStringSet& filesInDirList = (*n).second;
            if ( filesInDirList.empty() )
            {
                dirs.insert( (*n).first );
            }
            ++n;
        }

        TStringSet::iterator m = dirs.begin();
        while ( m != dirs.end() )
        {
            moduleInfo.includeDirs.erase( (*m) ); 
            moduleInfo.dependencyIncludeDirs.insert( (*m) );

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Moved dir from module header dir to dependency include dir as it had no files: " + (*m) );

            ++m;
        }

        // Check for duplicates,.. dirs that are listed as 
        // dependency includes but which also have headers part of this module
        // as such its not a dir coming from a dependency
        n = moduleInfo.includeDirs.begin();
        while ( n != moduleInfo.includeDirs.end() )
        {
            TStringSet::iterator p = moduleInfo.dependencyIncludeDirs.find( (*n).first );
            if ( p != moduleInfo.dependencyIncludeDirs.end() )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Deleting dependency include dir since it already exists as a module header dir: " + (*p) );                
                moduleInfo.dependencyIncludeDirs.erase( p );
            }            
            ++n;
        }
                
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
DeserializeModuleInfo( const CProjectInfo& projectInfo   ,
                       CModuleInfoEntry& moduleInfoEntry ,
                       const CORE::CDataNode& parentNode )
{GUCEF_TRACE;

    const CORE::CDataNode* moduleEntryNode = parentNode.Find( "ModuleInfoEntry" );
    if ( GUCEF_NULL == moduleEntryNode ) 
        return false;

    CORE::CDataNodeSerializableSettings defaultSerializableSettings;
    
    // @TODO: temp hack to get ProjectInfo into the deserializer which is needed for normalization
    defaultSerializableSettings.SetUserData( const_cast< CProjectInfo* >( &projectInfo ) );

    return moduleInfoEntry.Deserialize( *moduleEntryNode, defaultSerializableSettings );
}

/*-------------------------------------------------------------------------*/

bool
DeserializeModuleInfo( const CProjectInfo& projectInfo           ,
                       TModuleInfoEntryVector& moduleInfoEntries ,
                       const CORE::CString& inputFilepath        )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec = GetXmlDStoreCodec();
    if ( 0 != codec )
    {
        CORE::CDataNode rootNode;
        if ( codec->BuildDataTree( &rootNode, inputFilepath ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeModuleInfo: Successfully loaded module information from file \"" + inputFilepath + "\", now we will parse the information" );

            CORE::CDataNode::TDataNodeSet moduleEntryNodes = rootNode.FindNodesOfType( "ModuleInfoEntry", true );
            if ( moduleEntryNodes.empty() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeModuleInfo: No ModuleInfoEntry nodes were obtained from file \"" + inputFilepath + "\"" );
                return false;
            }

            UInt32 n=0; UInt32 errorCount = 0;
            CORE::CDataNode::TDataNodeSet::iterator i = moduleEntryNodes.begin();
            while ( i != moduleEntryNodes.end() )
            {
                CModuleInfoEntry entry;                
                if ( !DeserializeModuleInfo( projectInfo, entry, *(*i) ) )
                    ++errorCount;
                
                moduleInfoEntries.push_back( entry );
                ++i;
            }

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeModuleInfo: Successfully deserialized " + CORE::ToString( moduleEntryNodes.size()-errorCount ) + "/" + CORE::ToString( moduleEntryNodes.size() ) + " entries from file \"" + inputFilepath + "\"" );
            return 0 == errorCount;
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

/*-------------------------------------------------------------------------*/

void
InitializeModuleInfo( CModuleInfo& moduleInfo )
{GUCEF_TRACE;

    // reset all fields
    moduleInfo.buildOrder = -1;
    moduleInfo.buildChain = -1;
    moduleInfo.buildChainDependencies.clear();
    moduleInfo.name.Clear();
    moduleInfo.tags.clear();
    moduleInfo.includeDirs.clear();
    moduleInfo.sourceDirs.clear();
    moduleInfo.dependencyIncludeDirs.clear();
    moduleInfo.linkerSettings.linkedLibraries.clear();
    moduleInfo.compilerSettings.languagesUsed.clear();
    moduleInfo.preprocessorSettings.defines.clear();
    moduleInfo.dependencies.clear();
    moduleInfo.runtimeDependencies.clear();
    moduleInfo.considerSubDirs = true;
    moduleInfo.hasConsiderSubDirs = false;
    moduleInfo.ignoreModule = false;
    moduleInfo.hasIgnoreModule = false;
    moduleInfo.moduleType = MODULETYPE_UNDEFINED;
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

void
MergeLinkedLibrarySettingsMap( TLinkedLibrarySettingsMap& baseMap           ,
                               const TLinkedLibrarySettingsMap& incomingMap )
{GUCEF_TRACE;

    TLinkedLibrarySettingsMap::const_iterator i = incomingMap.begin();
    while ( i != incomingMap.end() )
    {
        TLinkedLibrarySettingsMap::iterator n = baseMap.find( (*i).first );
        if ( n != baseMap.end() )
        {
            TModuleType moduleType = (*i).second.moduleType;
            if ( ( MODULETYPE_UNDEFINED != moduleType ) &&
                 ( MODULETYPE_UNKNOWN != moduleType )    )
            {
                baseMap[ (*i).first ].moduleType = moduleType;
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

void
MergeModuleInfo( CModuleInfo& targetModuleInfo          ,
                 const CModuleInfo& moduleInfoToMergeIn )
{GUCEF_TRACE;

    // First take care of items which we know overwrite
    // We overwrite when the 'moduleInfoToMergeIn' has these attributes set to a
    // value other then the initialization value
    if ( moduleInfoToMergeIn.buildOrder > -1 )
    {
        targetModuleInfo.buildOrder = moduleInfoToMergeIn.buildOrder;
    }
    if ( moduleInfoToMergeIn.buildChain > -1 )
    {
        targetModuleInfo.buildChain = moduleInfoToMergeIn.buildChain;
    }
    TInt32Set::iterator i = moduleInfoToMergeIn.buildChainDependencies.begin();
    while ( i != moduleInfoToMergeIn.buildChainDependencies.end() )
    {
        targetModuleInfo.buildChainDependencies.insert( (*i) );
    }
    if ( !moduleInfoToMergeIn.name.IsNULLOrEmpty() )
    {
        targetModuleInfo.name = moduleInfoToMergeIn.name;
    }
    if ( moduleInfoToMergeIn.moduleType != MODULETYPE_UNDEFINED )
    {
        targetModuleInfo.moduleType = moduleInfoToMergeIn.moduleType;
    }
    if ( !moduleInfoToMergeIn.linkerSettings.targetName.IsNULLOrEmpty() )
    {
        targetModuleInfo.linkerSettings.targetName = moduleInfoToMergeIn.linkerSettings.targetName; 
    }
    if ( moduleInfoToMergeIn.hasIgnoreModule )
    {
        targetModuleInfo.ignoreModule = moduleInfoToMergeIn.ignoreModule;
        targetModuleInfo.hasIgnoreModule = true;
    }
    if ( moduleInfoToMergeIn.hasConsiderSubDirs )
    {
        targetModuleInfo.considerSubDirs = moduleInfoToMergeIn.considerSubDirs;
        targetModuleInfo.hasConsiderSubDirs = true;
    }

    // Now combine the other items without overwriting
    MergeStringSet( targetModuleInfo.tags    ,
                    moduleInfoToMergeIn.tags ,
                    true                     );
    MergeStringSet( targetModuleInfo.compilerSettings.languagesUsed    ,
                    moduleInfoToMergeIn.compilerSettings.languagesUsed ,
                    false                                              );
    MergeStringMap( targetModuleInfo.compilerSettings.compilerFlags    ,
                    moduleInfoToMergeIn.compilerSettings.compilerFlags ,
                    false                                              ,
                    true                                               );
    MergeStringSet( targetModuleInfo.preprocessorSettings.defines    ,
                    moduleInfoToMergeIn.preprocessorSettings.defines ,
                    true                                             );
    MergeLinkedLibrarySettingsMap( targetModuleInfo.linkerSettings.linkedLibraries    ,
                                   moduleInfoToMergeIn.linkerSettings.linkedLibraries );
    MergeStringSet( targetModuleInfo.dependencies    ,
                    moduleInfoToMergeIn.dependencies ,
                    false                            );
    MergeStringSet( targetModuleInfo.runtimeDependencies    ,
                    moduleInfoToMergeIn.runtimeDependencies ,
                    true                                    );
    MergeStringSet( targetModuleInfo.dependencyIncludeDirs    ,
                    moduleInfoToMergeIn.dependencyIncludeDirs ,
                    true                                      );
    MergeStringSetMap( targetModuleInfo.includeDirs    ,
                       moduleInfoToMergeIn.includeDirs ,
                       true                            );
    MergeStringSetMap( targetModuleInfo.sourceDirs    ,
                       moduleInfoToMergeIn.sourceDirs ,
                       true                           );
}

/*-------------------------------------------------------------------------*/

CModuleInfo*
FindModuleInfoForPlatform( CModuleInfoEntry& moduleInfoEntry ,
                           const CORE::CString& platform     ,
                           bool createNewIfNoneExists        )
{GUCEF_TRACE;

    TModuleInfoMap::iterator i = moduleInfoEntry.modulesPerPlatform.find( platform.Lowercase() );
    if ( i != moduleInfoEntry.modulesPerPlatform.end() )
    {
        return &(*i).second;
    }
    if ( createNewIfNoneExists )
    {
        CModuleInfo moduleInfo;
        InitializeModuleInfo( moduleInfo );

        moduleInfoEntry.modulesPerPlatform.insert( std::pair< CORE::CString, CModuleInfo >( platform, moduleInfo ) );
        return &moduleInfoEntry.modulesPerPlatform[ platform ];
    }
    return NULL;
}

/*-------------------------------------------------------------------------*/

const CModuleInfo*
FindModuleInfoForPlatform( const CModuleInfoEntry& moduleInfoEntry ,
                           const CORE::CString& platform           )
{GUCEF_TRACE;

    TModuleInfoMap::const_iterator i = moduleInfoEntry.modulesPerPlatform.find( platform.Lowercase() );
    if ( i != moduleInfoEntry.modulesPerPlatform.end() )
    {
        return &(*i).second;
    }
    return NULL;
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
    target.semver = priorityA.semver.IsAllZero() ? priorityB.semver : priorityA.semver;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
MergeModuleMetaData( const CModuleInfoEntry& moduleInfoEntry ,
                     const CORE::CString& targetPlatform     ,
                     CModuleInfo& mergedModuleInfo           )
{GUCEF_TRACE;

    const CModuleInfo* allPlatformsInfo = FindModuleInfoForPlatform( moduleInfoEntry, AllPlatforms );
    const CModuleInfo* targetPlatformInfo = FindModuleInfoForPlatform( moduleInfoEntry, targetPlatform );

    // an all-platforms module level meta-data overrules overarching meta-data
    if ( GUCEF_NULL != allPlatformsInfo )
        MergeModuleMetaData( allPlatformsInfo->metadata, moduleInfoEntry.metadata, mergedModuleInfo.metadata );
    
    // a platform specific module level meta-data overrules all-platforms meta-data
    if ( GUCEF_NULL != targetPlatformInfo )
        MergeModuleMetaData( targetPlatformInfo->metadata, mergedModuleInfo.metadata, mergedModuleInfo.metadata );
        
    return true;
}

/*-------------------------------------------------------------------------*/

bool
MergeModuleInfo( const CModuleInfoEntry& moduleInfoEntry ,
                 const CORE::CString& targetPlatform     ,
                 CModuleInfo& mergedModuleInfo           )
{GUCEF_TRACE;

    InitializeModuleInfo( mergedModuleInfo );

    const CModuleInfo* allPlatformsInfo = FindModuleInfoForPlatform( moduleInfoEntry, AllPlatforms );
    const CModuleInfo* targetPlatformInfo = FindModuleInfoForPlatform( moduleInfoEntry, targetPlatform );
    if ( ( GUCEF_NULL != allPlatformsInfo ) || ( GUCEF_NULL != targetPlatformInfo ) )
    {
        // Check if we have both
        if ( GUCEF_NULL != allPlatformsInfo && GUCEF_NULL != targetPlatformInfo )
        {
            // Check if at least one of them has a build order set
            if ( allPlatformsInfo->buildOrder > -1 || targetPlatformInfo->buildOrder > -1 )
            {
                // Check if at least one of them has a module type set
                if ( allPlatformsInfo->moduleType != MODULETYPE_UNDEFINED || targetPlatformInfo->moduleType != MODULETYPE_UNDEFINED )
                {
                    // Use the 'all' platform as a base to work from
                    mergedModuleInfo = *allPlatformsInfo;

                    // Now merge in the platform specific info
                    MergeModuleInfo( mergedModuleInfo, *targetPlatformInfo );

                    MergeModuleMetaData( moduleInfoEntry, targetPlatform, mergedModuleInfo );
                    return true;
                }
            }

            // Even though some info is specified for both platforms neither counts as independent
            // definition for which a build order must be set
            return false;
        }
        else
        if ( GUCEF_NULL != allPlatformsInfo && allPlatformsInfo->buildOrder > -1 )
        {
            // We only have the 'all' platform which is fine, we will just use that
            mergedModuleInfo = *allPlatformsInfo;
            
            MergeModuleMetaData( moduleInfoEntry, targetPlatform, mergedModuleInfo );
            return true;
        }
        else
        if ( GUCEF_NULL != targetPlatformInfo && targetPlatformInfo->buildOrder > -1 )
        {
            // We only have the target platform which is fine, we will just use that
            // this module apparently is not available for all platforms even in altered form
            mergedModuleInfo = *targetPlatformInfo;
            
            MergeModuleMetaData( moduleInfoEntry, targetPlatform, mergedModuleInfo );
            return true;
        }
    }

    // This module should not be used since it doesn't have platform specific info
    // nor info which applies to all platforms.
    return false;
}

/*-------------------------------------------------------------------------*/

bool
MergeModuleInfoEntries( const TModuleInfoEntryVector& moduleInfoEntriesToMergeIn ,
                        TModuleInfoEntryVector& moduleInfoEntries                )
{GUCEF_TRACE;

    bool totalSuccess = true;
    TModuleInfoEntryVector::const_iterator i = moduleInfoEntriesToMergeIn.begin();
    while ( i != moduleInfoEntriesToMergeIn.end() )
    {
        const CModuleInfoEntry& entryToMergeIn = (*i);
        CString consensusName = GetConsensusModuleName( entryToMergeIn );

        bool foundMatch = false;
        TModuleInfoEntryVector::iterator e = moduleInfoEntries.begin();
        while ( e != moduleInfoEntries.end() )
        {
            CModuleInfoEntry& existingEntry = (*e);
            CString existingEntryConsensusName = GetConsensusModuleName( existingEntry );

            if ( existingEntryConsensusName == consensusName )
            {
                // We already have such a module entry, we need to merge
                foundMatch = true;
                totalSuccess = existingEntry.Merge( entryToMergeIn ) && totalSuccess;
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
MergeAllModuleInfoForPlatform( const TModuleInfoEntryConstPtrSet& allInfo ,
                               const CORE::CString& platform              ,
                               TModuleInfoVector& allMergedInfo           ,
                               TModuleInfoEntryPairVector& mergeLinks     )
{GUCEF_TRACE;

    typedef std::vector< const CModuleInfoEntry* > TModuleInfoEntryPtrVector;

    allMergedInfo.clear();

    TModuleInfoEntryPtrVector indexMap;
    TModuleInfoEntryConstPtrSet::const_iterator i = allInfo.begin();
    while ( i != allInfo.end() )
    {
        // For each module we create a merged platform specific module
        // description which is easy to process if you only care about that platform
        CModuleInfo mergedInfo;
        if ( MergeModuleInfo( *(*i), platform, mergedInfo ) )
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
        TModuleInfoEntryPair mergeLink( indexMap[ m ], &(allMergedInfo[ m ]) );
        mergeLinks.push_back( mergeLink );
    }
    return true;
}

/*---------------------------------------------------------------------------*/

bool
MergeAllModuleInfoForPlatform( const TModuleInfoEntryVector& allInfo  ,
                               const CORE::CString& platform          ,
                               TModuleInfoVector& allMergedInfo       ,
                               TModuleInfoEntryPairVector& mergeLinks )
{GUCEF_TRACE;

    TModuleInfoEntryConstPtrSet infoPtrs;
    TModuleInfoEntryVector::const_iterator i = allInfo.begin();
    while ( i != allInfo.end() )
    {
        infoPtrs.insert( &(*i) );
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
GetModuleName( const CModuleInfoEntry& moduleInfoEntry ,
               const CORE::CString& targetPlatform     ,
               const CModuleInfo** outModuleInfo       )
{GUCEF_TRACE;

    TModuleInfoMap::const_iterator n = moduleInfoEntry.modulesPerPlatform.find( targetPlatform );
    if ( n != moduleInfoEntry.modulesPerPlatform.end() )
    {
        // A module was specified for this platform
        // Just because we have a module definition doenst mean we have a name
        if ( !(*n).second.name.IsNULLOrEmpty() )
        {
            // We have a name for this specific plaform
            if ( GUCEF_NULL != outModuleInfo )
            {
                *outModuleInfo = &(*n).second;
            }
            return &( (*n).second.name );
        }
    }

    // If no name is specified for a specific platform then there might still be a
    // default for all platforms
    if ( targetPlatform != AllPlatforms )
    {
        n = moduleInfoEntry.modulesPerPlatform.find( AllPlatforms );
        if ( n != moduleInfoEntry.modulesPerPlatform.end() )
        {
            // An 'AllPlatforms' definition is available for this module
            // Just because we have a module definition doenst mean we have a name
            if ( !(*n).second.name.IsNULLOrEmpty() )
            {
                // We have a name for this specific plaform
                if ( GUCEF_NULL != outModuleInfo )
                {
                    *outModuleInfo = &(*n).second;
                }
                return &( (*n).second.name );
            }
        }
    }
    return GUCEF_NULL;
}

/*---------------------------------------------------------------------------*/

const CORE::CString*
GetModuleName( const TProjectTargetInfoMap& targetPlatforms ,
               const CORE::CString& targetPlatform          ,
               const CModuleInfo** outModuleInfo            )
{GUCEF_TRACE;


    TProjectTargetInfoMap::const_iterator i = targetPlatforms.find( targetPlatform );
    if ( i != targetPlatforms.end() )
    {
        const CModuleInfoEntry* mainModule = (*i).second.mainModule;
        if ( GUCEF_NULL != mainModule )
        {
            return GetModuleName( *mainModule, targetPlatform, outModuleInfo );
        }
    }

    // If no target is specified for a specific platform then there might still be a
    // default for all platforms
    if ( targetPlatform != AllPlatforms )
    {
        i = targetPlatforms.find( AllPlatforms );
        if ( i != targetPlatforms.end() )
        {
            const CModuleInfoEntry* mainModule = (*i).second.mainModule;
            if ( GUCEF_NULL != mainModule )
            {
                return GetModuleName( *mainModule, AllPlatforms, outModuleInfo );
            }
        }
    }

    return GUCEF_NULL;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GetConsensusModuleName( const CModuleInfoEntry& moduleInfoEntry ,
                        const CModuleInfo** moduleInfo          )
{GUCEF_TRACE;

    if ( NULL != moduleInfo )
        *moduleInfo = GUCEF_NULL;

    TModuleInfoMap::const_iterator n = moduleInfoEntry.modulesPerPlatform.find( AllPlatforms );
    if ( n != moduleInfoEntry.modulesPerPlatform.end() )
    {
        // A name was specified for all platforms which makes our job easy
        // an all platform name always counts as the general consensus name
        if ( !(*n).second.name.IsNULLOrEmpty() )
        {
            if ( NULL != moduleInfo )
                *moduleInfo = &(*n).second;
            return (*n).second.name;
        }
    }

    // If no name is specified for all platforms then we will have to
    // determine the best name to use. We do this by getting the name
    // for all platforms and counting how often each is used. The most used
    // name is considered the general consensus name. If the same count applies
    // to multiple we will try to use a popular platform to improve our 'guess'

    typedef std::map< CORE::CString, CORE::UInt32 > TStringCountMap;

    TStringCountMap countMap;
    n = moduleInfoEntry.modulesPerPlatform.begin();
    while ( n != moduleInfoEntry.modulesPerPlatform.end() )
    {
        const CModuleInfo& mInfo = (*n).second;
        if ( !mInfo.name.IsNULLOrEmpty() )
        {
            TStringCountMap::iterator m = countMap.find( mInfo.name );
            if ( m != countMap.end() )
            {
                ++((*m).second);
            }
            else
            {
                countMap[ mInfo.name ] = 1;
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
        return CORE::LastSubDir( moduleInfoEntry.rootDir );
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
        n = moduleInfoEntry.modulesPerPlatform.begin();
        while ( n != moduleInfoEntry.modulesPerPlatform.end() )
        {
            if ( (*n).second.name == consensusName )
            {
                *moduleInfo = &(*n).second;
                break;
            }
            ++n;
        }
    }
    return consensusName;
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
            return GetModuleTargetName( *target.mainModule, targetPlatform, true );
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
            CORE::CString targetName = GetModuleTargetName( *target.mainModule, targetPlatform, true );
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
GetModuleNameAlways( const CModuleInfoEntry& moduleInfoEntry ,
                     const CORE::CString& targetPlatform     ,
                     const CModuleInfo** moduleInfo          )
{GUCEF_TRACE;

    const CORE::CString* strPtr = GetModuleName( moduleInfoEntry, targetPlatform, moduleInfo );
    if ( GUCEF_NULL == strPtr )
    {
        return GetConsensusModuleName( moduleInfoEntry, moduleInfo );
    }
    return *strPtr;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GetModuleTargetName( const CModuleInfoEntry& moduleInfoEntry ,
                     const CORE::CString& targetPlatform     ,
                     bool useModuleNameIfNoTargetName        )
{GUCEF_TRACE;

    const CModuleInfo* moduleInfo = FindModuleInfoForPlatform( moduleInfoEntry, targetPlatform );
    if ( NULL != moduleInfo )
    {
        if ( !moduleInfo->linkerSettings.targetName.IsNULLOrEmpty() )
        {
            return moduleInfo->linkerSettings.targetName;
        }
    }
    if ( targetPlatform != AllPlatforms && !targetPlatform.IsNULLOrEmpty() )
    {
        moduleInfo = FindModuleInfoForPlatform( moduleInfoEntry, AllPlatforms );
        if ( !moduleInfo->linkerSettings.targetName.IsNULLOrEmpty() )
        {
            return moduleInfo->linkerSettings.targetName;
        }
    }

    // If we got here no target name was defined so the only option left is the 
    // module name. For most projects this is the same thing.
    if ( useModuleNameIfNoTargetName )
    {
        return GetModuleNameAlways( moduleInfoEntry, targetPlatform );
    }

    return CORE::CString(); 
}

/*---------------------------------------------------------------------------*/

void
GetModuleDependencies( const CModuleInfoEntry& moduleInfoEntry ,
                       const CORE::CString& targetPlatform     ,
                       TStringSet& dependencies                ,
                       bool includeRuntimeDependencies         )
{GUCEF_TRACE;

    CModuleInfoEntry& mutableModuleInfoEntry = const_cast< CModuleInfoEntry& >( moduleInfoEntry );
    CModuleInfo* moduleInfo = FindModuleInfoForPlatform( mutableModuleInfoEntry, targetPlatform, false );
    if ( NULL != moduleInfo )
    {
        MergeStringSet( dependencies, moduleInfo->dependencies, false );
        if ( includeRuntimeDependencies )
            MergeStringSet( dependencies, moduleInfo->runtimeDependencies, false );
    }
    if ( targetPlatform != AllPlatforms && !targetPlatform.IsNULLOrEmpty() )
    {
        moduleInfo = FindModuleInfoForPlatform( mutableModuleInfoEntry, AllPlatforms, false );
        if ( NULL != moduleInfo )
        {
            MergeStringSet( dependencies, moduleInfo->dependencies, false );
            if ( includeRuntimeDependencies )
                MergeStringSet( dependencies, moduleInfo->runtimeDependencies, false );
        }
    }
}

/*---------------------------------------------------------------------------*/

bool
GetModuleDependencies( const CProjectInfo& projectInfo           ,
                       const CModuleInfoEntry& moduleInfoEntry   ,
                       const CORE::CString& targetPlatform       ,
                       TModuleInfoEntryConstPtrSet& dependencies ,
                       bool includeDependenciesOfDependencies    ,
                       bool includeRuntimeDependencies           )
{GUCEF_TRACE;

    TStringSet deps;
    GetModuleDependencies( moduleInfoEntry, targetPlatform, deps, includeRuntimeDependencies );     
    
    CORE::CString moduleName = GetConsensusModuleName( moduleInfoEntry );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GetModuleDependencies: For module " + moduleName + ": found \"" + CORE::ToString( deps.size() ) + "\" dependencies for platform " + targetPlatform );

    TModuleInfoEntryConstPtrSet depsPtrs;    
    bool foundAllDeps = true;
    TStringSet::iterator m = deps.begin();
    while ( m != deps.end() )
    {
        const CModuleInfoEntry* dependency = GetModuleInfoEntry( projectInfo, (*m), targetPlatform );
        if ( GUCEF_NULL != dependency )
            depsPtrs.insert( dependency );
        else
        {
            foundAllDeps = false; // We cannot satisfy the full dependency chain for the executable for the given platform
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GetModuleDependencies: For module " + moduleName + ": Found that for platform \"" + targetPlatform + "\" we cannot satisfy the full dependency chain because of dependency " + (*m) );
        }
        ++m;
    }
    
    if ( includeDependenciesOfDependencies )
    {
        TModuleInfoEntryConstPtrSet depsOfdependencies;
        TModuleInfoEntryConstPtrSet::iterator i = depsPtrs.begin();
        while ( i != depsPtrs.end() )
        {
            const CModuleInfoEntry* entry = (*i);            
            CORE::CString depModuleName = GetConsensusModuleName( *entry );

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GetModuleDependencies: For module " + moduleName + ": Recursively looking for dependencies of dependency \"" + depModuleName + "\" for platform " + targetPlatform );

            foundAllDeps = foundAllDeps && GetModuleDependencies( projectInfo, 
                                                                  *entry, 
                                                                  targetPlatform, 
                                                                  depsOfdependencies, 
                                                                  includeDependenciesOfDependencies, 
                                                                  includeRuntimeDependencies ); 
            ++i;
        }
        TModuleInfoEntryConstPtrSet::iterator n = depsOfdependencies.begin();
        while ( n != depsOfdependencies.end() )
        {
            depsPtrs.insert( (*n) );
            ++n;
        }
    }

    TModuleInfoEntryConstPtrSet::iterator i = depsPtrs.begin();
    while ( i != depsPtrs.end() )
    {
        dependencies.insert( (*i) );
        ++i;
    }

    return foundAllDeps;
}

/*---------------------------------------------------------------------------*/

TModuleType
GetModuleType( const CModuleInfoEntry& moduleInfoEntry ,
               const CORE::CString& targetPlatform     )
{GUCEF_TRACE;

    const CModuleInfo* moduleInfo = FindModuleInfoForPlatform( moduleInfoEntry, targetPlatform );
    if ( NULL != moduleInfo )
    {
        if ( MODULETYPE_UNDEFINED != moduleInfo->moduleType )
        {
            return moduleInfo->moduleType;
        }
    }
    if ( targetPlatform != AllPlatforms && !targetPlatform.IsNULLOrEmpty() )
    {
        moduleInfo = FindModuleInfoForPlatform( moduleInfoEntry, AllPlatforms );
        if ( NULL != moduleInfo )
        {
            return moduleInfo->moduleType;
        }
    }

    // Since there is no specific info for the given platform and no AllPlatform info
    // we will see if we can derive from another if there is consensus
    std::set< TModuleType > typeSet;
    TModuleInfoMap::const_iterator i = moduleInfoEntry.modulesPerPlatform.begin();
    while ( i != moduleInfoEntry.modulesPerPlatform.end() )
    {
        const CModuleInfo& platformModuleInfo = (*i).second;
        if ( MODULETYPE_UNDEFINED != platformModuleInfo.moduleType )
            typeSet.insert( platformModuleInfo.moduleType );
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
GetModuleInfoWithUniqueModulesTypes( const CModuleInfoEntry& moduleInfoEntry ,
                                     TConstModuleInfoPtrMap& moduleMap       )
{GUCEF_TRACE;

    // First try and get a 'AllPlatforms' definition which makes all the difference
    const CModuleInfo* moduleInfo = FindModuleInfoForPlatform( moduleInfoEntry, AllPlatforms );
    if ( NULL != moduleInfo && ( MODULETYPE_UNDEFINED != moduleInfo->moduleType ) )
    {
        // Since a 'AllPlatforms' definition is available we have a baseline to compare against
        // We will only add platform specific entries if they differ from our baseline
        TModuleInfoMap::const_iterator i = moduleInfoEntry.modulesPerPlatform.begin();
        while ( i != moduleInfoEntry.modulesPerPlatform.end() )
        {
            const CORE::CString& platformName = (*i).first;
            const CModuleInfo& platformModuleInfo = (*i).second;

            if ( ( MODULETYPE_UNDEFINED != platformModuleInfo.moduleType )  &&
                 ( moduleInfo->moduleType != platformModuleInfo.moduleType ) )
            {
                moduleMap[ platformName ] = &((*i).second);
            }
            ++i;
        }
        moduleMap[ AllPlatforms ] = moduleInfo;
    }
    else
    {
        // If we get here: We cannot filter in this case,.. just add all platforms
        TModuleInfoMap::const_iterator i = moduleInfoEntry.modulesPerPlatform.begin();
        while ( i != moduleInfoEntry.modulesPerPlatform.end() )
        {
            const CORE::CString& platformName = (*i).first;
            const CModuleInfo& platformModuleInfo = (*i).second;

            if ( MODULETYPE_UNDEFINED != platformModuleInfo.moduleType )
            {
                moduleMap[ platformName ] = &((*i).second);
            }
            ++i;
        }
    }
}

/*---------------------------------------------------------------------------*/

void
GetModuleInfoWithUniqueModuleNames( const CModuleInfoEntry& moduleInfoEntry ,
                                    TConstModuleInfoPtrMap& moduleMap       )
{GUCEF_TRACE;

    // First try and get a 'AllPlatforms' definition which makes all the difference
    const CModuleInfo* moduleInfo = FindModuleInfoForPlatform( moduleInfoEntry, AllPlatforms );
    if ( NULL != moduleInfo && !moduleInfo->name.IsNULLOrEmpty() )
    {
        // Since a 'AllPlatforms' definition is available we have a baseline to compare against
        // We will only add platform specific entries if they differ from our baseline
        TModuleInfoMap::const_iterator i = moduleInfoEntry.modulesPerPlatform.begin();
        while ( i != moduleInfoEntry.modulesPerPlatform.end() )
        {
            const CORE::CString& platformName = (*i).first;
            const CModuleInfo& platformModuleInfo = (*i).second;

            if ( ( !platformModuleInfo.name.IsNULLOrEmpty() )  &&
                 ( moduleInfo->name != platformModuleInfo.name ) )
            {
                moduleMap[ platformName ] = &((*i).second);
            }
            ++i;
        }
        moduleMap[ AllPlatforms ] = moduleInfo;
    }
    else
    {
        // If we get here: We cannot filter in this case,.. just add all platforms
        TModuleInfoMap::const_iterator i = moduleInfoEntry.modulesPerPlatform.begin();
        while ( i != moduleInfoEntry.modulesPerPlatform.end() )
        {
            const CORE::CString& platformName = (*i).first;
            const CModuleInfo& platformModuleInfo = (*i).second;

            if ( MODULETYPE_UNDEFINED != platformModuleInfo.moduleType )
            {
                moduleMap[ platformName ] = &((*i).second);
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
GetAllModuleInfoPaths( const CModuleInfoEntry& moduleInfoEntry ,
                       const CORE::CString& platform           ,
                       CORE::CString::StringSet& allPaths      ,
                       bool includeModuleRootPath              ,
                       bool includeDepencencyIncludePaths      )
{GUCEF_TRACE;

    TModuleInfoMap::const_iterator i = moduleInfoEntry.modulesPerPlatform.find( platform );
    if ( i != moduleInfoEntry.modulesPerPlatform.end() )
    {
        if ( includeModuleRootPath )
        {
            allPaths.insert( moduleInfoEntry.rootDir );
        }
        
        const TStringSetMap& includeDirs = (*i).second.includeDirs; 
        TStringSetMap::const_iterator n = includeDirs.begin();
        while ( n != includeDirs.end() )
        {
            const CORE::CString& includeDir = (*n).first;
            if ( includeModuleRootPath )
            {
                CORE::CString path = CORE::CombinePath( moduleInfoEntry.rootDir, includeDir );
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

        const TStringSetMap& sourceDirs = (*i).second.sourceDirs; 
        n = sourceDirs.begin();
        while ( n != sourceDirs.end() )
        {
            const CORE::CString& sourceDir = (*n).first;

            if ( includeModuleRootPath )
            {
                CORE::CString path = CORE::CombinePath( moduleInfoEntry.rootDir, sourceDir );
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
            const TStringSet& dependencyIncludeDirs = (*i).second.dependencyIncludeDirs;
            TStringSet::const_iterator m = dependencyIncludeDirs.begin();
            while ( m != dependencyIncludeDirs.end() )
            {
                if ( includeModuleRootPath )
                {
                    CORE::CString path = CORE::CombinePath( moduleInfoEntry.rootDir, (*m) );
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
GetAllModuleInfoFilePaths( const CModuleInfoEntry& moduleInfoEntry ,
                           const CORE::CString& platform           ,
                           CORE::CString::StringSet& allPaths      ,
                           bool includeModuleRootPath              )
{GUCEF_TRACE;

    TModuleInfoMap::const_iterator i = moduleInfoEntry.modulesPerPlatform.find( platform );
    if ( i != moduleInfoEntry.modulesPerPlatform.end() )
    {
        const TStringSetMap& includeDirs = (*i).second.includeDirs; 
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
                    path = CORE::CombinePath( moduleInfoEntry.rootDir, path );
                }

                path = CORE::RelativePath( path, true );
                path = LocalizeDirSepCharForPlatform( path, platform );
                allPaths.insert( path );
                ++m;
            }
            ++n;
        }        

        const TStringSetMap& sourceDirs = (*i).second.sourceDirs; 
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
                    path = CORE::CombinePath( moduleInfoEntry.rootDir, path );
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
GetShortestRelativePathFromModuleToProjectRoot( const CProjectInfo& projectInfo         ,
                                                const CModuleInfoEntry& moduleInfoEntry )
{GUCEF_TRACE;

    return GetShortestRelativePathFromAbsPathToProjectRoot( projectInfo, moduleInfoEntry.rootDir );
}

/*---------------------------------------------------------------------------*/

void
GetAllModuleInfoFilePaths( const CProjectInfo& projectInfo         ,
                           const CModuleInfoEntry& moduleInfoEntry ,
                           const CORE::CString& platform           ,
                           CORE::CString::StringSet& allPaths      ,
                           bool includeModuleRootPath              ,
                           bool relativeToProjectRoot              )
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
GetAllModuleInfoPaths( const CProjectInfo& projectInfo         ,
                       const CModuleInfoEntry& moduleInfoEntry ,
                       const CORE::CString& platform           ,
                       CORE::CString::StringSet& allPaths      ,
                       bool includeModuleRootPath              ,
                       bool relativeToProjectRoot              ,
                       bool includeDepencencyIncludePaths      )
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

const CModuleInfoEntry*
GetModuleInfoEntry( const TModuleInfoEntryVector& moduleInfoEntries ,
                    const CORE::CString& moduleName                 ,
                    const CORE::CString& platform                   ,
                    const CModuleInfo** moduleInfo                  )
{GUCEF_TRACE;

    TModuleInfoEntryVector::const_iterator i = moduleInfoEntries.begin();
    while ( i != moduleInfoEntries.end() )
    {
        CORE::CString nameOfCurrentModule = GetModuleNameAlways( (*i), platform, moduleInfo );
        if ( nameOfCurrentModule == moduleName )
        {
            return &(*i);
        }
        ++i;
    }
    return GUCEF_NULL;
}

/*---------------------------------------------------------------------------*/

const CModuleInfoEntry*
GetModuleInfoEntry( const CProjectInfo& projectInfo ,
                    const CORE::CString& moduleName ,
                    const CORE::CString& platform   ,
                    const CModuleInfo** moduleInfo  )
{GUCEF_TRACE;

    return GetModuleInfoEntry( projectInfo.modules, moduleName, platform, moduleInfo );
}

/*---------------------------------------------------------------------------*/

void
GetAllPlatformsUsed( const CProjectInfo& projectInfo ,
                     TStringSet& platformList        )
{GUCEF_TRACE;

    TModuleInfoEntryVector::const_iterator i = projectInfo.modules.begin();
    while ( i != projectInfo.modules.end() )
    {
        const TModuleInfoMap& modulesPerPlatform = (*i).modulesPerPlatform;
        TModuleInfoMap::const_iterator n = modulesPerPlatform.begin();
        while ( n != modulesPerPlatform.end() )
        {
            platformList.insert( (*n).first );
            ++n;
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
GetAllTagsUsed( const CProjectInfo& projectInfo ,
                TStringSet& tagsUsed            )
{GUCEF_TRACE;

    TModuleInfoEntryVector::const_iterator i = projectInfo.modules.begin();
    while ( i != projectInfo.modules.end() )
    {
        const TModuleInfoMap& modulesPerPlatform = (*i).modulesPerPlatform;
        TModuleInfoMap::const_iterator n = modulesPerPlatform.begin();
        while ( n != modulesPerPlatform.end() )
        {
            MergeStringSet( tagsUsed, (*n).second.tags, true );
            ++n;
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

bool
IsModuleTagged( const CModuleInfoEntry& module ,
                const CORE::CString& tag       ,
                const CORE::CString& platform  )
{GUCEF_TRACE;

    TModuleInfoMap::const_iterator i = module.modulesPerPlatform.find( AllPlatforms );
    if ( i != module.modulesPerPlatform.end() )
    {
        if ( (*i).second.tags.find( tag ) != (*i).second.tags.end() )
            return true;
    }
    i = module.modulesPerPlatform.find( platform );
    if ( i != module.modulesPerPlatform.end() )
    {
        if ( (*i).second.tags.find( tag ) != (*i).second.tags.end() )
            return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
IsModuleTagged( const CModuleInfoEntry& module       ,
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
GetTaggedModules( const CProjectInfo& projectInfo            ,
                  const CORE::CString& tag                   ,
                  TModuleInfoEntryConstPtrSet& taggedModules ,
                  const CORE::CString& platform              )
{GUCEF_TRACE;

    TModuleInfoEntryVector::const_iterator i = projectInfo.modules.begin();
    while ( i != projectInfo.modules.end() )
    {
        if ( IsModuleTagged( (*i), tag, platform ) )
            taggedModules.insert( &(*i) );
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

bool
ShouldModuleBeIgnored( const CModuleInfoEntry& moduleInfo ,
                       const CORE::CString& platformName  )
{GUCEF_TRACE;

    const TModuleInfoMap& modulesPerPlatform = moduleInfo.modulesPerPlatform;
    TModuleInfoMap::const_iterator i = modulesPerPlatform.find( platformName );
    if ( i != modulesPerPlatform.end() )
    {
        const CModuleInfo& moduleInfo = (*i).second;
        if ( moduleInfo.hasIgnoreModule )
            return moduleInfo.ignoreModule;
    }
    i = modulesPerPlatform.find( AllPlatforms );
    if ( i != modulesPerPlatform.end() )
    {
        const CModuleInfo& moduleInfo = (*i).second;
        if ( moduleInfo.hasIgnoreModule )
            return moduleInfo.ignoreModule;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
IsModuleTaggedWith( const CModuleInfoEntry& moduleInfo ,
                    const CORE::CString& platformName  ,
                    const CORE::CString& tag           )
{GUCEF_TRACE;

    const TModuleInfoMap& modulesPerPlatform = moduleInfo.modulesPerPlatform;
    TModuleInfoMap::const_iterator i = modulesPerPlatform.find( platformName );
    if ( i != modulesPerPlatform.end() )
    {
        if ( IsStringInList( (*i).second.tags, false, tag ) )
            return true;
    }
    i = modulesPerPlatform.find( AllPlatforms );
    if ( i != modulesPerPlatform.end() )
    {
        if ( IsStringInList( (*i).second.tags, false, tag ) )
            return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
HasIndependentModuleType( const TModuleInfoMap& moduleDefs )
{GUCEF_TRACE;

    TModuleInfoMap::const_iterator i = moduleDefs.begin();
    while ( i != moduleDefs.end() )
    {
        TModuleType moduleType = (*i).second.moduleType;

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
GetExecutables( const CProjectInfo& projectInfo                ,
                TModuleInfoEntryConstPtrSet& executableTargets ,
                const CORE::CString& platform                  )
{GUCEF_TRACE;

    TModuleInfoEntryVector::const_iterator i = projectInfo.modules.begin();
    while ( i != projectInfo.modules.end() )
    {
        if ( MODULETYPE_EXECUTABLE == GetModuleType( (*i), platform ) )
            executableTargets.insert( &(*i) );
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

        TModuleInfoEntryConstPtrSet executables;
        GetExecutables( projectInfo, executables, (*p) );

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SplitProjectPerTarget: Located " + CORE::ToString( executables.size() ) + " executable target candidates for platform " + (*p) );

        TModuleInfoEntryConstPtrSet::iterator i = executables.begin();
        while ( i != executables.end() )
        {
            const CModuleInfoEntry& executable = *(*i);
            CORE::CString targetName = GetModuleNameAlways( executable, (*p) );

            // Don't bother if the executable itself doesnt have a platform definition for the current platform            
            if ( executable.modulesPerPlatform.find( (*p) ) != executable.modulesPerPlatform.end() )
            {            
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SplitProjectPerTarget: Locating dependencies for target candidate \"" + 
                    targetName + "\" for platform " + (*p) );

                TModuleInfoEntryConstPtrSet foundDependencies;
                if ( GetModuleDependencies( projectInfo, executable, (*p), foundDependencies, true, true ) )
                {
                    // if we made it here we found the executable and were able to satisfy all dependencies
                    // for the current platform

                    CORE::CString projectName = projectInfo.projectName + "_exe_" + targetName; 
                    TProjectTargetInfoMap& targetPerPlatform = targets[ projectName ];
                    TProjectTargetInfo& target = targetPerPlatform[ (*p) ];

                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SplitProjectPerTarget: Located " + CORE::ToString( foundDependencies.size() ) + 
                        " dependencies for executable target \"" + targetName + "\" for platform " + (*p) );

                    target.projectName = projectName;
                    target.mainModule = &executable;
                    target.modules.insert( &executable );
                    TModuleInfoEntryConstPtrSet::iterator j = foundDependencies.begin();
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
                TModuleInfoEntryConstPtrSet taggedModules;
                GetTaggedModules( projectInfo, (*i), taggedModules, (*p) );
                CORE::CString projectName = projectInfo.projectName + "_tag_" + (*i);

                TModuleInfoEntryConstPtrSet::iterator m = taggedModules.begin();
                while ( m != taggedModules.end() )
                {
                    const CModuleInfoEntry& taggedModule = *(*m);
                                            
                    // Don't include this module if it doesnt have a definition for the current platform
                    if ( taggedModule.modulesPerPlatform.find( (*p) ) != taggedModule.modulesPerPlatform.end() )
                    {                    
                        // Tagged or not we need to include the dependencies of tagged modules as well
                        // We don't want to make projects that cannot compile
                        TModuleInfoEntryConstPtrSet foundDependencies;
                        if ( GetModuleDependencies( projectInfo, taggedModule, (*p), foundDependencies, true, true ) )
                        {
                            TProjectTargetInfoMap& targetPerPlatform = targets[ projectName ];
                            TProjectTargetInfo& target = targetPerPlatform[ (*p) ];

                            target.projectName = projectName;
                            target.modules.insert( &taggedModule );

                            // Since many modules can have the same tag there really is no such thing as a 'main' tagged module.
                            // It may be that there is only 1 in a repo but that is a coincidence and not relevant
                            target.mainModule = GUCEF_NULL;

                            TModuleInfoEntryConstPtrSet::iterator j = foundDependencies.begin();
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
    fullProjectTarget.mainModule = GUCEF_NULL;
    TModuleInfoEntryVector::const_iterator w = projectInfo.modules.begin();
    while ( w != projectInfo.modules.end() )
    {
        fullProjectTarget.modules.insert( &(*w) );
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
    GetAllPlatformsUsed( projectInfo, platformsUsed );

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
HasPlatformDefinition( const CModuleInfoEntry& moduleInfoEntry ,
                       const CORE::CString& platform           )
{GUCEF_TRACE;

    TModuleInfoMap::const_iterator i = moduleInfoEntry.modulesPerPlatform.begin();
    while ( i != moduleInfoEntry.modulesPerPlatform.end() )
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
IsAnyLicenseDefined( const TModuleInfoEntryVector& moduleInfoEntries )
{GUCEF_TRACE;

    TModuleInfoEntryVector::const_iterator i = moduleInfoEntries.begin();
    while ( i != moduleInfoEntries.end() )
    {
        const CModuleInfoEntry& entry = (*i);
        if ( !entry.metadata.license.IsNULLOrEmpty() )
        {
            return true;
        }
        ++i;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
IsAnySemVerDefined( const TModuleInfoEntryVector& moduleInfoEntries )
{GUCEF_TRACE;

    TModuleInfoEntryVector::const_iterator i = moduleInfoEntries.begin();
    while ( i != moduleInfoEntries.end() )
    {
        const CModuleInfoEntry& entry = (*i);
        if ( !entry.metadata.semver.IsAllZero() )
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

const CORE::CString CModuleMetaData::ClassTypeName = "GUCEF::PROJECTGEN::CModuleMetaData";

/*---------------------------------------------------------------------------*/

CModuleMetaData::CModuleMetaData( void ) 
    : CORE::CIDataNodeSerializable()
    , lastEditBy()
    , authors()
    , maintainers()
    , semver()
    , descriptionHeadline()
    , descriptionDetails()
    , license()
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CModuleMetaData::CModuleMetaData( const CModuleMetaData& src ) 
    : CORE::CIDataNodeSerializable( src )
    , lastEditBy( src.lastEditBy )
    , authors( src.authors )
    , maintainers( src.maintainers )
    , semver( src.semver )
    , descriptionHeadline( src.descriptionHeadline )
    , descriptionDetails( src.descriptionDetails )
    , license( src.license )
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
    semver.Clear();
    descriptionHeadline.Clear();
    descriptionDetails.Clear();
    license.Clear();
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
        semver = src.semver;
        descriptionHeadline = src.descriptionHeadline;
        descriptionDetails = src.descriptionDetails;
        license = src.license;
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
    if ( !semver.IsAllZero() )
    {
        domRootNode.AddChildWithValue( "SemVer", semver.ToString(), GUCEF_DATATYPE_STRING );
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
    semver.FromString( domRootNode.GetAttributeValueOrChildValueByName( "SemVer", semver.ToString(), false ).AsString( semver.ToString(), true ) );

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
    : name()    
    , moduleType( MODULETYPE_UNDEFINED )
    , tags()
    , dependencies()
    , dependencyIncludeDirs()
    , runtimeDependencies()
    , includeDirs()
    , sourceDirs()
    , buildOrder( -1 )
    , buildChain( -1 )
    , buildChainDependencies()
    , considerSubDirs( false )
    , hasConsiderSubDirs( false )
    , linkerSettings()
    , compilerSettings()
    , preprocessorSettings()
    , ignoreModule( false )
    , hasIgnoreModule( false )
    , metadata()
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CModuleInfo::CModuleInfo( const CModuleInfo& src ) 
    : name( src.name )
    , moduleType( src.moduleType )
    , tags( src.tags )
    , dependencies( src.dependencies )
    , dependencyIncludeDirs( src.dependencyIncludeDirs )
    , runtimeDependencies( src.runtimeDependencies )
    , includeDirs( src.includeDirs )
    , sourceDirs( src.sourceDirs )
    , buildOrder( src.buildOrder )
    , buildChain( src.buildChain )
    , buildChainDependencies(src.buildChainDependencies)
    , considerSubDirs( src.considerSubDirs )
    , hasConsiderSubDirs( src.hasConsiderSubDirs )
    , linkerSettings( src.linkerSettings )
    , compilerSettings( src.compilerSettings )
    , preprocessorSettings( src.preprocessorSettings )
    , ignoreModule( src.ignoreModule )
    , hasIgnoreModule( src.hasIgnoreModule )
    , metadata( src.metadata )
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
    dependencies.clear();
    dependencyIncludeDirs.clear();
    runtimeDependencies.clear();
    includeDirs.clear();
    sourceDirs.clear();
    buildOrder = -1;
    buildChain = -1;
    buildChainDependencies.clear();
    considerSubDirs = false;
    hasConsiderSubDirs = false;
    //linkerSettings.Clear();
    //compilerSettings.Clear();
    //preprocessorSettings.Clear();
    ignoreModule = false;
    hasIgnoreModule = false;
    metadata.Clear();
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfo::Merge( const CModuleInfo& moduleInfoToMergeIn ,
                    bool onConflictOriginalInfoStays       )
{GUCEF_TRACE;

    bool totalSuccess = true;

    if ( name.IsNULLOrEmpty() )
        name = moduleInfoToMergeIn.name;
    else
    if ( !onConflictOriginalInfoStays && !moduleInfoToMergeIn.name.IsNULLOrEmpty() )
        name = moduleInfoToMergeIn.name;

    if ( MODULETYPE_UNDEFINED == moduleType )
        moduleType = moduleInfoToMergeIn.moduleType;
    else
    if ( !onConflictOriginalInfoStays && MODULETYPE_UNDEFINED != moduleInfoToMergeIn.moduleType )
        moduleType = moduleInfoToMergeIn.moduleType;

    MergeStringSet( tags, moduleInfoToMergeIn.tags, true );
    MergeStringSet( dependencies, moduleInfoToMergeIn.dependencies, true );
    MergeStringSet( dependencyIncludeDirs, moduleInfoToMergeIn.dependencyIncludeDirs, true );
    MergeStringSet( runtimeDependencies, moduleInfoToMergeIn.runtimeDependencies, true );
        

    if ( -1 == buildOrder )
        buildOrder = moduleInfoToMergeIn.buildOrder;
    else
    if ( !onConflictOriginalInfoStays && -1 != moduleInfoToMergeIn.buildOrder )
        buildOrder = moduleInfoToMergeIn.buildOrder;

    if ( -1 == buildChain )
        buildChain = moduleInfoToMergeIn.buildChain;
    else
    if ( !onConflictOriginalInfoStays && -1 != moduleInfoToMergeIn.buildChain )
        buildChain = moduleInfoToMergeIn.buildChain;

    buildChainDependencies.insert( moduleInfoToMergeIn.buildChainDependencies.begin(), moduleInfoToMergeIn.buildChainDependencies.end() );

    if ( !hasConsiderSubDirs )
    {
        considerSubDirs = moduleInfoToMergeIn.considerSubDirs;
        hasConsiderSubDirs = moduleInfoToMergeIn.hasConsiderSubDirs;
    }
    else
    if ( !onConflictOriginalInfoStays && moduleInfoToMergeIn.hasConsiderSubDirs )
    {
        considerSubDirs = moduleInfoToMergeIn.considerSubDirs;
        hasConsiderSubDirs = moduleInfoToMergeIn.hasConsiderSubDirs;
    }

    if ( !hasIgnoreModule )
    {
        ignoreModule = moduleInfoToMergeIn.ignoreModule;
        hasIgnoreModule = moduleInfoToMergeIn.hasIgnoreModule;
    }
    else
    if ( !onConflictOriginalInfoStays && moduleInfoToMergeIn.hasIgnoreModule )
    {
        ignoreModule = moduleInfoToMergeIn.ignoreModule;
        hasIgnoreModule = moduleInfoToMergeIn.hasIgnoreModule;
    }

    totalSuccess = metadata.Merge( moduleInfoToMergeIn.metadata, onConflictOriginalInfoStays ) && totalSuccess;

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

const CORE::CString CModuleInfoEntry::ClassTypeName = "GUCEF::PROJECTGEN::CModuleInfoEntry";

/*---------------------------------------------------------------------------*/

CModuleInfoEntry::CModuleInfoEntry( void ) 
    : CORE::CIDataNodeSerializable()
    , modulesPerPlatform()
    , rootDir()
    , metadata()
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CModuleInfoEntry::CModuleInfoEntry( const CModuleInfoEntry& src ) 
    : CORE::CIDataNodeSerializable( src )
    , modulesPerPlatform( src.modulesPerPlatform )
    , rootDir( src.rootDir )
    , metadata( src.metadata )
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

    modulesPerPlatform.clear();
    rootDir.Clear();
    metadata.Clear();
}

/*---------------------------------------------------------------------------*/

CModuleInfoEntry&
CModuleInfoEntry::operator=( const CModuleInfoEntry& src ) 
{GUCEF_TRACE;

    if ( &src != this )
    {
        modulesPerPlatform = src.modulesPerPlatform;
        rootDir = src.rootDir;
        metadata = src.metadata;
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

    CORE::CDataNode* metaDataNode = domRootNode.AddChild( "ModuleMetaData" );
    if ( GUCEF_NULL != metaDataNode )
    {
        totalSuccess = metadata.Serialize( *metaDataNode, settings ) && totalSuccess;
    }

    TModuleInfoMap::const_iterator i = modulesPerPlatform.begin();
    while ( i != modulesPerPlatform.end() )
    {
        const CORE::CString& platform = (*i).first;
        const CModuleInfo& moduleInfo = (*i).second;

        if ( !SerializeModuleInfo( *this           ,
                                   moduleInfo      ,
                                   platform        ,
                                   domRootNode     ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "SerializeModuleInfo: Failed to serialize moduleInfo of module " + moduleInfo.name + " for platform " + platform );
            totalSuccess = false;
        }

        ++i;
    }
    
    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfoEntry::SetModuleInfo( CModuleInfo& moduleInfo       ,
                                 const CORE::CString& platform )
{GUCEF_TRACE;

    // First check if we already have a entry for this platform
    TModuleInfoMap::iterator i = modulesPerPlatform.find( platform );
    if ( i != modulesPerPlatform.end() )
    {
        // Since we already have an entry for this platform we will merge the two
        MergeModuleInfo( (*i).second, moduleInfo );
    }
    else
    {
        modulesPerPlatform[ platform ] = moduleInfo;
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

    const CORE::CDataNode* metaDataNode = domRootNode.FindChild( "ModuleMetaData", false );
    if ( GUCEF_NULL != metaDataNode )
    {
        if ( !metadata.Deserialize( *metaDataNode, settings ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CModuleInfoEntry:Deserialize: Failed to deserialize metadata" );
            return false;
        }
    }

    const CORE::CDataNode::TConstDataNodeSet moduleInfoNodes = domRootNode.FindChildrenOfType( "Module" );
    if ( moduleInfoNodes.size() == 0 ) 
        return false;

    CORE::CDataNode::TConstDataNodeSet::const_iterator n = moduleInfoNodes.begin();
    while ( n != moduleInfoNodes.end() )
    {
        CModuleInfo moduleInfoForPlatform;
        InitializeModuleInfo( moduleInfoForPlatform );

        const CORE::CDataNode* moduleNode = (*n);

        if ( DeserializeModuleInfo( moduleInfoForPlatform ,
                                    *moduleNode           ) )
        {
            if ( moduleInfoForPlatform.name.IsNULLOrEmpty() )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully deserialized module definition for module with no name specified for the applicable platforms" );
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully deserialized module definition for module with name " + moduleInfoForPlatform.name );
            }

            // Get all platforms for which this info applies.
            // Keep in mind that multiple platforms can be specified for ease of use.
            // This feature requires platform entries to be seperated by a ';'
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

                TStringSet actualPlatforms = ResolveMultiPlatformName( (*i), &projectInfo->platforms );
                TStringSet::iterator n = actualPlatforms.begin();
                while ( n != actualPlatforms.end() )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Adding module definition for platform " + (*n) );
                    SetModuleInfo( moduleInfoForPlatform, (*n) );
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
CModuleInfoEntry::Merge( const CModuleInfoEntry& infoToMergeIn ,
                         bool onConflictOriginalInfoStays      ) 
{GUCEF_TRACE;

    bool totalSuccess = true;

    if ( rootDir.IsNULLOrEmpty() )
        rootDir = infoToMergeIn.rootDir;
    else
    if ( !onConflictOriginalInfoStays && !infoToMergeIn.rootDir.IsNULLOrEmpty() )
        rootDir = infoToMergeIn.rootDir;

    totalSuccess = metadata.Merge( infoToMergeIn.metadata ) && totalSuccess;

    TModuleInfoMap::const_iterator i = infoToMergeIn.modulesPerPlatform.begin();
    while ( i != infoToMergeIn.modulesPerPlatform.end() )
    {
        const CString& platformToMergeIn = (*i).first;
        const CModuleInfo& moduleInfoToMergeIn = (*i).second;

        TModuleInfoMap::iterator e = modulesPerPlatform.find( platformToMergeIn );
        if ( e != modulesPerPlatform.end() )
        {
            // We already have info for this platform
            // we need to merge
            CModuleInfo& moduleInfo = (*e).second;
            totalSuccess = moduleInfo.Merge( moduleInfoToMergeIn ) && totalSuccess;

            ++e;
        }
        else
        {
            // No entry exists yet for this platform so we can just do a simple insert
            modulesPerPlatform[ platformToMergeIn ] = moduleInfoToMergeIn;
        }

        ++i;
    }

    return totalSuccess;
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

CProjectInfo::CProjectInfo( void ) 
    : CORE::CTSharedObjCreator< CProjectInfo, MT::CMutex >( this )
    , m_rwLock( true )
    , projectName()
    , rootDirs()
    , modules()
    , dirProcessingInstructions()
    , globalDirExcludeList()
    , platforms()
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
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CProjectInfo::~CProjectInfo() 
{GUCEF_TRACE;

    Clear();
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
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
