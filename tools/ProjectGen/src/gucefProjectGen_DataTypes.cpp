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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

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

#ifndef GUCEF_PROJECTGEN_CPROJECTINFO_H
#include "gucefProjectGen_CProjectInfo.h"
#define GUCEF_PROJECTGEN_CPROJECTINFO_H
#endif /* GUCEF_PROJECTGEN_CPROJECTINFO_H ? */

#ifndef GUCEF_PROJECTGEN_CMODULEINFO_H
#include "gucefProjectGen_CModuleInfo.h"
#define GUCEF_PROJECTGEN_CMODULEINFO_H
#endif /* GUCEF_PROJECTGEN_CMODULEINFO_H ? */

#ifndef GUCEF_PROJECTGEN_CMODULEINFOENTRY_H
#include "gucefProjectGen_CModuleInfoEntry.h"
#define GUCEF_PROJECTGEN_CMODULEINFOENTRY_H
#endif /* GUCEF_PROJECTGEN_CMODULEINFOENTRY_H ? */

#ifndef GUCEF_PROJECTGEN_CMODULEMETADATA_H
#include "gucefProjectGen_CModuleMetaData.h"
#define GUCEF_PROJECTGEN_CMODULEMETADATA_H
#endif /* GUCEF_PROJECTGEN_CMODULEMETADATA_H ? */

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

const CORE::CString KnownPlatforms::AllPlatforms = "all";
const CORE::CString KnownPlatforms::Win32 = "win32";
const CORE::CString KnownPlatforms::Win64 = "win64";
const CORE::CString KnownPlatforms::Linux32 = "linux32";
const CORE::CString KnownPlatforms::Linux64 = "linux64";
const CORE::CString KnownPlatforms::Android32 = "android32";
const CORE::CString KnownPlatforms::Android64 = "android64";
const CORE::CString KnownPlatforms::Arduino = "arduino";
const CORE::CString KnownPlatforms::Emscripten32 = "emscripten32";
const CORE::CString KnownPlatforms::Emscripten64 = "emscripten64";

const CORE::CString KnownMultiPlatforms::MsWin = "mswin";
const CORE::CString KnownMultiPlatforms::Linux = "linux";
const CORE::CString KnownMultiPlatforms::LinuxLike = "linuxlike";
const CORE::CString KnownMultiPlatforms::UnixLike = "unixlike";
const CORE::CString KnownMultiPlatforms::Android = "android";
const CORE::CString KnownMultiPlatforms::Emscripten = "emscripten";
const CORE::CString KnownMultiPlatforms::MacOS = "macos";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

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

bool
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

bool
IsStringInList( const TStringSet& list          ,
                bool caseSensitive              ,
                const CORE::CString& testString ,
                bool wildcardMatching           )
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
{GUCEF_TRACE;

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

CORE::CString
LinkedDependencyScopeToString( const TLinkedDependencyScope scope )
{GUCEF_TRACE;

    switch ( scope )
    {        
        case TLinkedDependencyScope::LINKEDDEPENDENCYSCOPE_PRIVATE: return "private";
        case TLinkedDependencyScope::LINKEDDEPENDENCYSCOPE_PUBLIC: return "public";
        case TLinkedDependencyScope::LINKEDDEPENDENCYSCOPE_AUTO: return "auto";

        case TLinkedDependencyScope::LINKEDDEPENDENCYSCOPE_UNDEFINED:
        default:
        {
            return "undefined";
        }
    }
}

/*-------------------------------------------------------------------------*/

TLinkedDependencyScope
StringToLinkedDependencyScope( const CORE::CString& linkedDependencyScopeStr )
{GUCEF_TRACE;

    CORE::CString scopeStr = linkedDependencyScopeStr.Lowercase();
    if ( scopeStr == "private" ) return TLinkedDependencyScope::LINKEDDEPENDENCYSCOPE_PRIVATE;
    if ( scopeStr == "public" ) return TLinkedDependencyScope::LINKEDDEPENDENCYSCOPE_PUBLIC;
    if ( scopeStr == "auto" ) return TLinkedDependencyScope::LINKEDDEPENDENCYSCOPE_AUTO;

    return TLinkedDependencyScope::LINKEDDEPENDENCYSCOPE_UNDEFINED;
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

CORE::CStringSet
ResolveMultiPlatformName( const CORE::CString& platformName          ,
                          const TPlatformDefinitionMap* platformDefs )
{GUCEF_TRACE;

    CORE::CStringSet resultSet;

    // Save some effort for "All Platforms"...
    if ( platformName == KnownPlatforms::AllPlatforms )
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

            if ( (*i).Equals( KnownMultiPlatforms::MsWin, false ) )
            {
                resultSet.insert( KnownPlatforms::Win32 );
                resultSet.insert( KnownPlatforms::Win64 );
            }
            else
            if ( (*i).Equals( KnownMultiPlatforms::Linux, false ) )
            {
                resultSet.insert( KnownPlatforms::Linux32 );
                resultSet.insert( KnownPlatforms::Linux64 );
            }
            else
            if ( (*i).Equals( KnownMultiPlatforms::Android, false ) )
            {
                resultSet.insert( KnownPlatforms::Android32 );
                resultSet.insert( KnownPlatforms::Android64 );
            }
            else
            if ( (*i).Equals( KnownMultiPlatforms::Emscripten, false ) )
            {
                resultSet.insert( KnownPlatforms::Emscripten32 );
                resultSet.insert( KnownPlatforms::Emscripten64 );
            }
            else
            if ( (*i).Equals( KnownMultiPlatforms::LinuxLike, false ) )
            {
                resultSet.insert( KnownPlatforms::Linux32 );
                resultSet.insert( KnownPlatforms::Linux64 );
                resultSet.insert( KnownPlatforms::Android32 );
                resultSet.insert( KnownPlatforms::Android64 );
                resultSet.insert( KnownPlatforms::Emscripten32 );
                resultSet.insert( KnownPlatforms::Emscripten64 );
            }
            else
            if ( (*i).Equals( KnownMultiPlatforms::UnixLike, false ) )
            {

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

CORE::CStringSet
ReduceToUseMultiPlatformNamesIfFeasible( const CORE::CStringSet& platformNames      ,
                                         const TPlatformDefinitionMap* platformDefs )
{GUCEF_TRACE;

    CORE::CStringSet resultSet;

    if ( platformNames.find( KnownPlatforms::Win32 ) != platformNames.end() )
        if ( platformNames.find( KnownPlatforms::Win64 ) != platformNames.end() )
            resultSet.insert( KnownMultiPlatforms::MsWin );

    if ( platformNames.find( KnownPlatforms::Win64 ) != platformNames.end() )
        if ( platformNames.find( KnownPlatforms::Win32 ) != platformNames.end() )
            resultSet.insert( KnownMultiPlatforms::MsWin );

    if ( platformNames.find( KnownPlatforms::Linux32 ) != platformNames.end() )
        if ( platformNames.find( KnownPlatforms::Linux64 ) != platformNames.end() )
            resultSet.insert( KnownMultiPlatforms::Linux );

    if ( platformNames.find( KnownPlatforms::Linux64 ) != platformNames.end() )
        if ( platformNames.find( KnownPlatforms::Linux32 ) != platformNames.end() )
            resultSet.insert( KnownMultiPlatforms::Linux );

    if ( platformNames.find( KnownPlatforms::Android32 ) != platformNames.end() )
        if ( platformNames.find( KnownPlatforms::Android64 ) != platformNames.end() )
            resultSet.insert( KnownMultiPlatforms::Android );

    if ( platformNames.find( KnownPlatforms::Android64 ) != platformNames.end() )
        if ( platformNames.find( KnownPlatforms::Android32 ) != platformNames.end() )
            resultSet.insert( KnownMultiPlatforms::Android );

    if ( platformNames.find( KnownPlatforms::Emscripten32 ) != platformNames.end() )
        if ( platformNames.find( KnownPlatforms::Emscripten64 ) != platformNames.end() )
            resultSet.insert( KnownMultiPlatforms::Emscripten );

    if ( platformNames.find( KnownPlatforms::Emscripten64 ) != platformNames.end() )
        if ( platformNames.find( KnownPlatforms::Emscripten32 ) != platformNames.end() )
            resultSet.insert( KnownMultiPlatforms::Emscripten );

    CORE::CStringSet::const_iterator i = platformNames.begin();
    while ( i != platformNames.end() )
    {
        const CORE::CString& platformName = (*i);
        if ( ( platformName != KnownPlatforms::Win32 ) &&
             ( platformName != KnownPlatforms::Win64 ) &&
             ( platformName != KnownPlatforms::Linux32 ) &&
             ( platformName != KnownPlatforms::Linux64 ) &&
             ( platformName != KnownPlatforms::Android32 ) &&
             ( platformName != KnownPlatforms::Android64 ) &&
             ( platformName != KnownPlatforms::Emscripten32 ) &&
             ( platformName != KnownPlatforms::Emscripten64 ) )
        {
            resultSet.insert( platformName );
        }

        ++i;
    }

    return resultSet;
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
        defaultSettings.levelOfDetail = CORE::CDataNodeSerializableSettings::DataNodeSerializableLod_AverageDetails;
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

/*---------------------------------------------------------------------------*/

bool
IsDirALegacyModuleDir( const CORE::CString& dir )
{GUCEF_TRACE;

    // The dir is a module dir if it has a suffix file in it
    CORE::CString suffixFilePath = dir;
    CORE::AppendToPath( suffixFilePath, "CMakeListsSuffix.txt" );

    return CORE::FileExists( suffixFilePath );
}

/*---------------------------------------------------------------------------*/

bool
IsDirAModuleDir( const CORE::CString& dir )
{GUCEF_TRACE;

    // The dir is a module dir if it has a suffix file in it
    CORE::CString moduleInfoFilePath = dir;
    CORE::AppendToPath( moduleInfoFilePath, "ModuleInfo.xml" );

    if ( !CORE::FileExists( moduleInfoFilePath ) )
    {
        return IsDirALegacyModuleDir( dir );
    }
    return true;
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

    const CModuleInfoPtr allPlatformsInfo = moduleInfoEntry->FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms );
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
    }

    // This module should not be used since it doesn't have platform specific info
    // nor info which applies to all platforms.
    return false;
}

/*-------------------------------------------------------------------------*/

bool
MergeModuleInfoEntries( const TStringToModuleInfoEntryPtrMap& moduleInfoEntriesToMergeIn ,
                        TStringToModuleInfoEntryPtrMap& moduleInfoEntries                )
{GUCEF_TRACE;

    bool totalSuccess = true;
    TStringToModuleInfoEntryPtrMap::const_iterator i = moduleInfoEntriesToMergeIn.begin();
    while ( i != moduleInfoEntriesToMergeIn.end() )
    {
        const CModuleInfoEntryPtr& entryToMergeIn = (*i).second;
        CString consensusName = entryToMergeIn->GetConsensusName();

        bool foundMatch = false;
        TStringToModuleInfoEntryPtrMap::iterator e = moduleInfoEntries.begin();
        while ( e != moduleInfoEntries.end() )
        {
            CModuleInfoEntryPtr& existingEntry = (*e).second;
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
            moduleInfoEntries[ entryToMergeIn->GetConsensusName() ] = entryToMergeIn;
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
MergeAllModuleInfoForPlatform( const TStringToModuleInfoEntryPtrMap& allInfo ,
                               const CORE::CString& platform                 ,
                               TModuleInfoPtrVector& allMergedInfo           ,
                               TModuleInfoEntryPairVector& mergeLinks        )
{GUCEF_TRACE;

    TModuleInfoEntryPtrSet infoPtrs;
    TStringToModuleInfoEntryPtrMap::const_iterator i = allInfo.begin();
    while ( i != allInfo.end() )
    {
        infoPtrs.insert( (*i).second );
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

void
GetModuleInfoWithUniqueModulesTypes( const CModuleInfoEntryPtr& moduleInfoEntry ,
                                     TModuleInfoPtrMap& moduleMap               )
{GUCEF_TRACE;

    // First try and get a 'AllPlatforms' definition which makes all the difference
    const CModuleInfoPtr moduleInfo = moduleInfoEntry->FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms );
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
        moduleMap[ KnownPlatforms::AllPlatforms ] = moduleInfo;
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
    const CModuleInfoPtr moduleInfo = moduleInfoEntry->FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms );
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
        moduleMap[ KnownPlatforms::AllPlatforms ] = moduleInfo;
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

    if ( KnownPlatforms::Win32 == platform || KnownPlatforms::Win64 == platform )
    {
        return path.ReplaceChar( '/', '\\' );
    }
    if ( KnownPlatforms::Linux32 == platform || KnownPlatforms::Linux64 == platform ||
         KnownPlatforms::Android32 == platform || KnownPlatforms::Android64 == platform ||
         KnownPlatforms::Emscripten32 == platform || KnownPlatforms::Emscripten64 == platform )
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
            allPaths.insert( moduleInfoEntry->GetAbsolutePathToModuleRootDir() );
        }
        
        const TStringSetMap& includeDirs = (*i).second->GetIncludeDirs(); 
        TStringSetMap::const_iterator n = includeDirs.begin();
        while ( n != includeDirs.end() )
        {
            const CORE::CString& includeDir = (*n).first;
            if ( includeModuleRootPath )
            {
                CORE::CString path = CORE::CombinePath( moduleInfoEntry->GetAbsolutePathToModuleRootDir(), includeDir );
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
                CORE::CString path = CORE::CombinePath( moduleInfoEntry->GetAbsolutePathToModuleRootDir(), sourceDir );
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
                    CORE::CString path = CORE::CombinePath( moduleInfoEntry->GetAbsolutePathToModuleRootDir(), (*m) );
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
    if ( platform != KnownPlatforms::AllPlatforms )
    {
        GetAllModuleInfoPaths( moduleInfoEntry, KnownPlatforms::AllPlatforms, allPaths, includeModuleRootPath, includeDepencencyIncludePaths );
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
                    path = CORE::CombinePath( moduleInfoEntry->GetAbsolutePathToModuleRootDir(), path );
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
                    path = CORE::CombinePath( moduleInfoEntry->GetAbsolutePathToModuleRootDir(), path );
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
    if ( platform != KnownPlatforms::AllPlatforms )
    {
        GetAllModuleInfoFilePaths( moduleInfoEntry, KnownPlatforms::AllPlatforms, allPaths, includeModuleRootPath );
    }
}

/*---------------------------------------------------------------------------*/

CORE::CString
GetShortestRelativePathFromAbsPathToProjectRoot( const CProjectInfo& projectInfo ,
                                                 const CORE::CString& absPath    )
{GUCEF_TRACE;

    typedef GUCEF::map< CORE::UInt32, CORE::CString > TUInt32ToStringMap;

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

    return GetShortestRelativePathFromAbsPathToProjectRoot( projectInfo, moduleInfoEntry->GetAbsolutePathToModuleRootDir() );
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
    i = modulesPerPlatform.find( KnownPlatforms::AllPlatforms );
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
    i = modulesPerPlatform.find( KnownPlatforms::AllPlatforms );
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
IsAnyLicenseDefined( const TStringToModuleInfoEntryPtrMap& moduleInfoEntries )
{GUCEF_TRACE;

    TStringToModuleInfoEntryPtrMap::const_iterator i = moduleInfoEntries.begin();
    while ( i != moduleInfoEntries.end() )
    {
        const CModuleInfoEntryPtr& entry = (*i).second;
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
IsAnySemVerDefined( const TStringToModuleInfoEntryPtrMap& moduleInfoEntries )
{GUCEF_TRACE;

    TStringToModuleInfoEntryPtrMap::const_iterator i = moduleInfoEntries.begin();
    while ( i != moduleInfoEntries.end() )
    {
        const CModuleInfoEntryPtr& entry = (*i).second;
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
    , m_linkedDependencyScope( TLinkedDependencyScope::LINKEDDEPENDENCYSCOPE_UNDEFINED )
    , m_libPath()
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CLinkedLibrarySettings::CLinkedLibrarySettings( const CLinkedLibrarySettings& src )
    : CORE::CTSharedObjCreator< CLinkedLibrarySettings, MT::CMutex >( this )
    , m_moduleType( src.m_moduleType )
    , m_linkedDependencyScope( src.m_linkedDependencyScope )
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

void
CLinkedLibrarySettings::SetLinkedDependecyScope( TLinkedDependencyScope linkedDependencyScope )
{GUCEF_TRACE;

    m_linkedDependencyScope = linkedDependencyScope;
}

/*---------------------------------------------------------------------------*/

TLinkedDependencyScope
CLinkedLibrarySettings::GetLinkedDependencyScope( void ) const
{GUCEF_TRACE;

    return m_linkedDependencyScope;
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

    if ( TLinkedDependencyScope::LINKEDDEPENDENCYSCOPE_UNDEFINED == m_linkedDependencyScope || ( TLinkedDependencyScope::LINKEDDEPENDENCYSCOPE_UNDEFINED == m_linkedDependencyScope && TLinkedDependencyScope::LINKEDDEPENDENCYSCOPE_UNDEFINED != linkedLibrarySettingsToMergeIn.m_linkedDependencyScope ) )
        m_linkedDependencyScope = linkedLibrarySettingsToMergeIn.m_linkedDependencyScope;
    else
    if ( !onConflictOriginalInfoStays && !( TLinkedDependencyScope::LINKEDDEPENDENCYSCOPE_UNDEFINED == linkedLibrarySettingsToMergeIn.m_linkedDependencyScope || TLinkedDependencyScope::LINKEDDEPENDENCYSCOPE_UNDEFINED == linkedLibrarySettingsToMergeIn.m_linkedDependencyScope ) )
        m_linkedDependencyScope = linkedLibrarySettingsToMergeIn.m_linkedDependencyScope;

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

bool
CLinkerSettings::IsStaticLibrary( const CORE::CString& testStr            ,
                                  const CORE::CString** knownPlatformIfSo )
{GUCEF_TRACE;

    CORE::CString extensionIfAny = CORE::ExtractFileExtention( testStr ).Lowercase();
    if ( !extensionIfAny.IsNULLOrEmpty() )
    {
        if ( extensionIfAny == "lib" ) // static library extension Microsoft Windows
        {
            if ( knownPlatformIfSo != GUCEF_NULL )
                *knownPlatformIfSo = &KnownMultiPlatforms::MsWin;
            return true;
        }
        if ( extensionIfAny == "a" ) // static library extension for Linux / Android / Emscripten
        {
            if ( knownPlatformIfSo != GUCEF_NULL )
                *knownPlatformIfSo = &KnownMultiPlatforms::LinuxLike;
            return true;
        }
        if ( extensionIfAny == "dylib" ) // MacOS dynamic library extension
        {
            if ( knownPlatformIfSo != GUCEF_NULL )
                *knownPlatformIfSo = &KnownMultiPlatforms::MacOS;
            return true;
        }
    }

    return false;
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

bool
CLinkerSettings::HasAnyStaticLibraryLinkerDependency( void ) const
{
    TLinkedLibrarySettingsPtrMap::const_iterator i = m_linkedLibraries.begin();
    while ( i != m_linkedLibraries.end() )
    {
        const CORE::CString& libName = (*i).first;
        const CLinkedLibrarySettingsPtr& linkedLib = (*i).second;

        if ( !libName.IsNULLOrEmpty() )
        {
            if ( IsStaticLibrary( libName ) )
                return true;
        }

        TModuleType linkedLibType = linkedLib->GetModuleType();
        if ( MODULETYPE_STATIC_LIBRARY == linkedLibType )
            return true;

        if ( MODULETYPE_UNDEFINED == linkedLibType ||
             MODULETYPE_UNKNOWN == linkedLibType    )
        {
            const CORE::CString& libPath = linkedLib->GetLibraryPath();
            if ( !libPath.IsNULLOrEmpty() )
            {
                if ( IsStaticLibrary( libPath ) )
                    return true;
            }
        }
        ++i;
    }
    return false;
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
CLinkerSettings::AddLinkedLibraryAsCopy( const CORE::CString& libraryName               ,
                                         const CLinkedLibrarySettingsPtr& linkedLibrary )
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( libraryName.IsNULLOrEmpty() ||  linkedLibrary.IsNULL() )
        return false;

    CLinkedLibrarySettingsPtr libSettingsCopy = CLinkedLibrarySettings::CreateSharedObjWithParam( *linkedLibrary );
    m_linkedLibraries[ libraryName ] = libSettingsCopy;
    return true;
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
CLinkerSettings::Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    CORE::CDataNode linkerNode;
    linkerNode.SetName( "Linker" );
    bool addedLinkedSettings = false;

    // Now Serialize all linker related info
    // First add all the libraries that are linked but not part of the overall project
    if ( m_linkedLibraries.size() > 0 )
    {
        addedLinkedSettings = true;
        TLinkedLibrarySettingsPtrMap::const_iterator m = m_linkedLibraries.begin();
        while ( m != m_linkedLibraries.end() )
        {
            const CORE::CString& linkedLibName = (*m).first;
            const CLinkedLibrarySettingsPtr& linkedLibSettings = (*m).second;

            CORE::CDataNode libraryNode;
            libraryNode.SetName( "Dependency" );
            libraryNode.SetAttribute( "Name", linkedLibName );

            TModuleType linkedLibType = linkedLibSettings->GetModuleType();
            if ( ( MODULETYPE_UNDEFINED != linkedLibType ) &&
                 ( MODULETYPE_UNKNOWN != linkedLibType )    )
            {
                libraryNode.SetAttribute( "Type", ModuleTypeToString( linkedLibType ) );
            }

            TLinkedDependencyScope linkedDependencyScope = linkedLibSettings->GetLinkedDependencyScope();
            if ( LINKEDDEPENDENCYSCOPE_UNDEFINED != linkedDependencyScope )
            {
                libraryNode.SetAttribute( "Scope", LinkedDependencyScopeToString( linkedDependencyScope ) );
            }

            if ( !linkedLibSettings->GetLibraryPath().IsNULLOrEmpty() )
            {
                libraryNode.SetAttribute( "Path", linkedLibSettings->GetLibraryPath() );
            }
            linkerNode.AddChild( libraryNode );
            ++m;
        }

        m = m_linkedLogicalLibraries.begin();
        while ( m != m_linkedLogicalLibraries.end() )
        {
            CORE::CDataNode libraryNode;
            libraryNode.SetName( "LogicalDependency" );
            libraryNode.SetAttribute( "Name", (*m).first );

            TModuleType linkedLibType = (*m).second->GetModuleType();
            if ( ( MODULETYPE_UNDEFINED != linkedLibType ) &&
                 ( MODULETYPE_UNKNOWN != linkedLibType )    )
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
    if ( !m_targetName.IsNULLOrEmpty() )
    {
        addedLinkedSettings = true;
        linkerNode.SetAttribute( "TargetName", m_targetName ); 
    }

    if ( !m_libPaths.empty() )
    {
        CORE::CString libPaths;
        TStringSet::const_iterator r = m_libPaths.begin();
        while ( r != m_libPaths.end() )
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
        domRootNode.AddChild( linkerNode );
    }

    return true;
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

                    CORE::CString linkedDependencyScope = linkedLibNode->GetAttributeValue( "Scope" );
                    if ( !linkedDependencyScope.IsNULLOrEmpty() )
                    {
                        linkedLibrary->SetLinkedDependecyScope( StringToLinkedDependencyScope( linkedDependencyScope ) );
                    }
                    else
                    {
                        linkedLibrary->SetLinkedDependecyScope( LINKEDDEPENDENCYSCOPE_UNDEFINED );
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

CORE::CString
CPreprocessorSettings::SanitizeDefine( const CORE::CString& define )
{GUCEF_TRACE;

    /*
     *  Allowed characters: A preprocessor define or macro name can contain uppercase letters (A-Z), lowercase letters (a-z), digits (0-9), and the underscore (_).
     *  Starting character: A preprocessor define or macro name cannot begin with a digit. It must start with a letter or an underscore.
     *  Reserved identifiers: Names with a leading underscore followed by an uppercase letter (_Foo) or a double underscore (__foo)
     *  are reserved for the compiler and standard library. You should not use these for your own macros.
     */

    // All ASCII chars outside [A-Z], [a-z], [0-9], and '_'
    static const Int32 invalidChars[] = {

        // Control characters 0x00–0x1F
        '\x00','\x01','\x02','\x03','\x04','\x05','\x06','\x07',
        '\x08','\x09','\x0A','\x0B','\x0C','\x0D','\x0E','\x0F',
        '\x10','\x11','\x12','\x13','\x14','\x15','\x16','\x17',
        '\x18','\x19','\x1A','\x1B','\x1C','\x1D','\x1E','\x1F',

        // Printable symbols and whitespace
        ' ', '!', '"', '#', '$', '%', '&', '\'',
        '(', ')', '*', '+', ',', '-', '.', '/',
        ':', ';', '<', '=', '>', '?', '@',
        '[', '\\', ']', '^', '`', '{', '|', '}', '~',

        // DEL (0x7F)
        '\x7F'
    };

    static const UInt32 invalidCharCount = (UInt32) ( sizeof(invalidChars) / sizeof(invalidChars[0]) );

    Int32 equalsOffset = define.HasChar( '=' );
    if ( equalsOffset > 0 )
    {
        CORE::CString beforeEquals = define.SubstrToIndex( (UInt32) equalsOffset, true );
        CORE::CString afterEquals = define.SubstrToIndex( (UInt32) equalsOffset+1, false );

        beforeEquals = beforeEquals.ReplaceChars( invalidChars, invalidCharCount, '_' );
        afterEquals = afterEquals.ReplaceChars( invalidChars, invalidCharCount, '_' );

        CORE::CString sanitizedDefine = beforeEquals + '=' + afterEquals;
        return sanitizedDefine;
    }
    else
    {
        CORE::CString sanitizedDefine = define.ReplaceChars( invalidChars, invalidCharCount, '_' );
        return sanitizedDefine;
    }
}

/*---------------------------------------------------------------------------*/

void
CPreprocessorSettings::AddDefine( const CORE::CString& define )
{GUCEF_TRACE;

     m_defines.insert( SanitizeDefine( define ) );
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
