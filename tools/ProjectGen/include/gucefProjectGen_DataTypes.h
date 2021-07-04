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
 
#ifndef GUCEF_PROJECTGEN_DATATYPES_H
#define GUCEF_PROJECTGEN_DATATYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <set>
#include <map>
#include <vector>

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CVERSIONRANGE_H
#include "gucefCORE_CVersionRange.h"
#define GUCEF_CORE_CVERSIONRANGE_H
#endif /* GUCEF_CORE_CVERSIONRANGE_H ? */

#ifndef GUCEF_PROJECTGEN_MACROS_H
#include "gucefProjectGen_macros.h"
#define GUCEF_PROJECTGEN_MACROS_H
#endif /* GUCEF_PROJECTGEN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PROJECTGEN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef std::set< CORE::CString > TStringSet;
typedef std::set< CORE::Int32 > TInt32Set;
typedef std::map< CORE::CString, CORE::CString > TStringMap;
typedef std::map< CORE::CString, TStringSet > TStringSetMap;
typedef std::vector< CORE::CString > TStringVector;
typedef std::map< CORE::CString, TStringVector > TStringVectorMap;
typedef std::map< CORE::CString, TStringVectorMap > TStringVectorMapMap;

/*---------------------------------------------------------------------------*/

enum EModuleType
{
    MODULETYPE_UNDEFINED                  = 0 ,    // <- this is the initialization value
    
    MODULETYPE_EXECUTABLE                 = 1 ,
    MODULETYPE_SHARED_LIBRARY             = 2 ,    // <- shared library (.dll on win32, .so on linux/android)
    MODULETYPE_STATIC_LIBRARY             = 3 ,    // <- static library (.lib on win32, .a on linux/android)
    MODULETYPE_HEADER_INCLUDE_LOCATION    = 4 ,    // <- location where headers can be placed for reference by other modules
    MODULETYPE_CODE_INTEGRATE_LOCATION    = 5 ,    // <- location where shared code can be placed for inclusion as part of other modules
    MODULETYPE_HEADER_INTEGRATE_LOCATION  = 6 ,    // <- location where shared headers can be placed for inclusion as part of other modules
    MODULETYPE_REFERENCE_LIBRARY          = 7 ,    // <- C# code library
    
    MODULETYPE_BINARY_PACKAGE             = 8,     // <- definition of a compiled binary package, usually with headers
    
    MODULETYPE_UNKNOWN                    = 999    // <- to be used when initialized BUT we cannot determine the module type
};
typedef enum EModuleType TModuleType;

typedef std::map< CORE::CString, TModuleType > TModuleTypeMap;

/*---------------------------------------------------------------------------*/

/**
 *  Structure where all linker related information should be stored
 *  for a specific linked library
 */
struct SLinkedLibrarySettings
{
    TModuleType moduleType;               // Module type of the linked library if already known
    CORE::CString libPath;                // optional extra path for the linker to search for the given library
};
typedef struct SLinkedLibrarySettings TLinkedLibrarySettings;
typedef std::map< CORE::CString, TLinkedLibrarySettings > TLinkedLibrarySettingsMap;

/*---------------------------------------------------------------------------*/

/**
 *  Structure where all linker related information should be stored
 */
struct SLinkerSettings
{
    TLinkedLibrarySettingsMap linkedLibraries;    // list of all libraries the module links against
    TStringSet libPaths;                          // list of hint paths where to look for libraries
    CORE::CString targetName;                     // optional name for the linker target if desired from the module name
};
typedef struct SLinkerSettings TLinkerSettings;

/*---------------------------------------------------------------------------*/

/**
 *  Structure where all compiler related information should be stored
 */
struct SCompilerSettings
{
    TStringSet languagesUsed;                // list of all programming languages used within this module
    TStringMap compilerFlags;                // map of flags to pass to the specific compilers
};
typedef struct SCompilerSettings TCompilerSettings;

/**
 *  Structure where all dependency related information should be stored
 */
struct SDependencyInfo
{
    CORE::CString name;                // name of the dependency
    TStringSet includePaths;           // include directories needed for the headers of the dependencies, paths only no files
    
    CORE::CVersionRange version;
    bool ignoreVersion;
};
typedef struct SDependencyInfo TDependencyInfo;

/*---------------------------------------------------------------------------*/

/**
 *  Structure where all preprocessor related information should be stored
 */
struct SPreprocessorSettings
{
    TStringSet defines;                       // list of all precompiler definitions for this module
};
typedef struct SPreprocessorSettings TPreprocessorSettings;

/*---------------------------------------------------------------------------*/

struct SModuleInfo
{
    CORE::CString name;                          // the name of the module
    TModuleType moduleType;                      // The type of module we are dealing with
    TStringSet tags;                             // optional tags that can be associated which allows filtering of modules
    
    TStringSet dependencies;                     // list of module names of all modules this module depends on
    TStringSet dependencyIncludeDirs;            // include directories needed for the headers of the dependencies, paths only no files
    TStringSet runtimeDependencies;              // dependencies not relative for builds but desired to be easily accessable due to runtime dependency, typically plugins
       
    TStringSetMap includeDirs;                   // include directories of this module's own headers
    TStringSetMap sourceDirs;                    // source directories of this module's own source

    int buildOrder;                              // order number of this module in the build dependency chain
    int buildChain;                              // index of the build chain, different build chains can be build independently but may depend on other chains
    TInt32Set buildChainDependencies;            // other build chains this build chain is dependent on, if any
    bool considerSubDirs;                        // Whether only the dir with the ModuleInfo is to be considered or whether subdirs are recursively considered

    TLinkerSettings linkerSettings;              // all linker related settings for this module
    TCompilerSettings compilerSettings;          // all compiler related settings for this module
    TPreprocessorSettings preprocessorSettings;  // all preprocessor related settings for this module

    bool ignoreModule;                           // whether this module should be included in the build
};
typedef struct SModuleInfo TModuleInfo;

/*---------------------------------------------------------------------------*/

typedef std::vector< TModuleInfo > TModuleInfoVector;
typedef std::map< CORE::CString, TModuleInfoVector > TModuleInfoVectorMap;
typedef std::map< CORE::CString, TModuleInfo > TModuleInfoMap;
typedef std::vector< TModuleInfo* > TModuleInfoPtrVector;
typedef std::map< CORE::CString, const TModuleInfo* > TConstModuleInfoPtrMap;

/*---------------------------------------------------------------------------*/

struct SModuleInfoEntry
{
    TModuleInfoMap modulesPerPlatform;     // ModuleInfo per platform
    CORE::CString  rootDir;                // the absolute path to the root of this module's directory tree
    CORE::CString  lastEditBy;             // optional info listing who last updated the information
};
typedef struct SModuleInfoEntry TModuleInfoEntry;

/*---------------------------------------------------------------------------*/

typedef std::vector< TModuleInfoEntry > TModuleInfoEntryVector;
typedef std::pair< const TModuleInfoEntry*, const TModuleInfo* > TModuleInfoEntryPair;
typedef std::pair< TModuleInfoEntry*, TModuleInfo* > TMutableModuleInfoEntryPair;
typedef std::vector< TModuleInfoEntryPair > TModuleInfoEntryPairVector;
typedef std::vector< TMutableModuleInfoEntryPair > TMutableModuleInfoEntryPairVector;
typedef std::vector< TModuleInfoEntry* > TModuleInfoEntryPtrVector;
typedef std::vector< const TModuleInfoEntry* > TModuleInfoEntryConstPtrVector;
typedef std::set< TModuleInfoEntry* > TModuleInfoEntryPtrSet;
typedef std::set< const TModuleInfoEntry* > TModuleInfoEntryConstPtrSet;
typedef std::map< int, TModuleInfoEntry* > TModuleInfoEntryPrioMap;

/*---------------------------------------------------------------------------*/

struct SPlatformDefinition
{
    TStringSet aliases;                                    // alternate names for this platform, can be 1-N across multiple platforms as a group name
    TStringSet platformDirs;                               // Directory names which will be considered holders of platform specific files
};
typedef struct SPlatformDefinition TPlatformDefinition;
typedef std::map< CORE::CString, TPlatformDefinition > TPlatformDefinitionMap;

/*---------------------------------------------------------------------------*/

struct SDirProcessingInstructions
{
    TStringVectorMap dirExcludeList;        // list of directories that are to be excluded, maps a list of per platform  
    TStringVectorMap dirIncludeList;        // list of directories that are to be included, maps a list of per platform

    TStringVectorMap fileExcludeList;       // list of files that are to be excluded, maps a list of per platform
    TStringVectorMap fileIncludeList;       // list of files that are to be included, maps a list of per platform

    TPlatformDefinitionMap platforms;       // Supplemental platform definitions
    
    CORE::CDataNode processingInstructions; // All unparsed processing instruction data
};
typedef struct SDirProcessingInstructions TDirProcessingInstructions;

/*---------------------------------------------------------------------------*/

typedef std::map< CORE::CString, TDirProcessingInstructions > TDirProcessingInstructionsMap;

/*---------------------------------------------------------------------------*/

struct SProjectInfo
{
    CORE::CString projectName;                               // Name of the overall project
    TStringVector rootDirs;                                  // Root dirs used to gather all project info
    TModuleInfoEntryVector modules;                          // All generated module information
    TDirProcessingInstructionsMap dirProcessingInstructions; // All loaded processing instructions mapped per path
    TStringVector globalDirExcludeList;                      // Dirs that should never be included in processing regardless of path
    TPlatformDefinitionMap platforms;                        // All supported platforms for this project
};
typedef struct SProjectInfo TProjectInfo;

/*---------------------------------------------------------------------------*/

struct SProjectTargetInfo
{
    CORE::CString projectName;                               // Name of the overall project (bundling target)
    const TModuleInfoEntry* mainModule;                      // Reference to the main module for the project if applicable
    TModuleInfoEntryConstPtrSet modules;                     // All generated module information
};
typedef struct SProjectTargetInfo TProjectTargetInfo;
typedef std::map< CORE::CString, TProjectTargetInfo > TProjectTargetInfoMap;        // maps a given target platform name, for example 'win32' to everything linked/needed for a given auto-generated target
typedef std::map< CORE::CString, TProjectTargetInfoMap > TProjectTargetInfoMapMap;  // maps a auto-generated target project name to another map which maps on a per target platform basis

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP 
CORE::CDStoreCodecRegistry::TDStoreCodecPtr
GetXmlDStoreCodec( void );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP 
void
ApplyConfigToProject( const CORE::CDataNode& loadedConfig , 
                      TProjectInfo& projectInfo           );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP 
void
InitializeModuleInfoEntry( TModuleInfoEntry& moduleEntry );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
void
InitializeModuleInfo( TModuleInfo& moduleInfo );


/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
const TModuleInfo*
FindModuleInfoForPlatform( const TModuleInfoEntry& moduleInfoEntry ,
                           const CORE::CString& platform           );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
TModuleInfo*
FindModuleInfoForPlatform( TModuleInfoEntry& moduleInfoEntry ,
                           const CORE::CString& platform     ,
                           bool createNewIfNoneExists        );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
const CORE::CString*
GetModuleName( const TModuleInfoEntry& moduleInfoEntry     ,
               const CORE::CString& targetPlatform         ,
               const TModuleInfo** moduleInfo = GUCEF_NULL );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
const CORE::CString*
GetModuleName( const TProjectTargetInfoMap& targetPlatforms ,
               const CORE::CString& targetPlatform          ,
               const TModuleInfo** moduleInfo = GUCEF_NULL  );
               
/*-------------------------------------------------------------------------*/

// The name to use in config files etc can't always be multiple names or defined
// per platform. For that we have this function which looks at the different names
// available across the platforms and find the general consensus name which is the
// best guess name that could be used to label this module generally without specifying 
// a platform
GUCEF_PROJECTGEN_PUBLIC_CPP
CORE::CString
GetConsensusModuleName( const TModuleInfoEntry& moduleInfoEntry     ,
                        const TModuleInfo** moduleInfo = GUCEF_NULL );

/*-------------------------------------------------------------------------*/

// Across multiple platforms targets can have multiple names
// This allows for coming up with a consensus name across the various platforms
// for a given target.
// Note that this considers module linker target names and module names
//
// Not all target origins cause a main module to be defined.
// As such its perfectly possible be unable to define a consensus target name because the concept does not apply
// to the collection of targets due to the origins of the collection
GUCEF_PROJECTGEN_PUBLIC_CPP
CORE::CString
GetConsensusTargetName( const TProjectTargetInfoMap& targetPlatforms ,
                        const CORE::CString& targetPlatform          );

/*-------------------------------------------------------------------------*/

// Same as GetConsensusTargetName( targetPlatforms, targetPlatform ) 
// Always uses AllPlatforms as the target platform
GUCEF_PROJECTGEN_PUBLIC_CPP
CORE::CString
GetConsensusTargetName( const TProjectTargetInfoMap& targetPlatforms );

/*-------------------------------------------------------------------------*/

// Tries to get the properly merged module name first for the given platform
// if this does not yield a module name it will use the general consensus
// module name instead
GUCEF_PROJECTGEN_PUBLIC_CPP
CORE::CString
GetModuleNameAlways( const TModuleInfoEntry& moduleInfoEntry ,
                     const CORE::CString& targetPlatform     ,
                     const TModuleInfo** moduleInfo = NULL   );
                     
/*-------------------------------------------------------------------------*/
                     
GUCEF_PROJECTGEN_PUBLIC_CPP
void
MergeStringVector( TStringVector& targetList          ,
                   const TStringVector& listToMergeIn ,
                   bool caseSensitive                 );

/*-------------------------------------------------------------------------*/
                   
GUCEF_PROJECTGEN_PUBLIC_CPP
void
MergeStringVectorMap( TStringVectorMap& targetMap          ,
                      const TStringVectorMap& mapToMergeIn ,
                      bool caseSensitive                   );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
void
MergeStringSet( TStringSet& targetList          ,
                const TStringSet& listToMergeIn ,
                bool caseSensitive              );

/*-------------------------------------------------------------------------*/
                   
GUCEF_PROJECTGEN_PUBLIC_CPP
void
MergeStringSetMap( TStringSetMap& targetMap          ,
                   const TStringSetMap& mapToMergeIn ,
                   bool caseSensitive                );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
void
MergeModuleInfo( TModuleInfo& targetModuleInfo          ,
                 const TModuleInfo& moduleInfoToMergeIn );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
MergeModuleInfo( const TModuleInfoEntry& moduleInfo     ,
                 const CORE::CString& targetPlatform    ,
                 TModuleInfo& mergedModuleInfo          );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
MergeAllModuleInfoForPlatform( const TModuleInfoEntryVector& allInfo  ,
                               const CORE::CString& platform          ,
                               TModuleInfoVector& allMergedInfo       ,
                               TModuleInfoEntryPairVector& mergeLinks );


/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
MergeAllModuleInfoForPlatform( const TModuleInfoEntryConstPtrSet& allInfo ,
                               const CORE::CString& platform              ,
                               TModuleInfoVector& allMergedInfo           ,
                               TModuleInfoEntryPairVector& mergeLinks     );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
CORE::CString
ModuleTypeToString( const TModuleType moduleType );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
TModuleType
StringToModuleType( const CORE::CString& moduleTypeStr );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
TStringSet
StringVectorToStringSet( const TStringVector& stringVector );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
void
CleanupIncludeDirs( TModuleInfoEntry& moduleInfoEntry );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
SerializeModuleInfo( const TModuleInfoEntry& moduleInfo  ,
                     const CORE::CString& outputFilepath );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
DeserializeModuleInfo( const TProjectInfo& projectInfo ,  
                       TModuleInfoEntry& moduleInfo    ,
                       const CORE::CDataNode& rootNode );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
DeserializeModuleInfo( const TProjectInfo& projectInfo           ,
                       TModuleInfoEntryVector& moduleInfoEntries ,
                       const CORE::CString& inputFilepath        );


/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
SerializeProjectInfo( const TProjectInfo& projectInfo ,
                      CORE::CDataNode& rootNodeToBe   );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
DeserializeProjectInfo( TProjectInfo& projectInfo       ,
                        const CORE::CDataNode& rootNode );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
SerializeProjectInfo( const TProjectInfo& projectInfo     ,
                      const CORE::CString& outputFilepath );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
DeserializeProjectInfo( TProjectInfo& projectInfo            ,
                        const CORE::CString& projectInfoPath );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
void
GetModuleDependencies( const TModuleInfoEntry& moduleInfoEntry ,
                       const CORE::CString& targetPlatform     ,
                       TStringSet& dependencies                ,
                       bool includeRuntimeDependencies         );
                       
/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
GetModuleDependencies( const TProjectInfo& projectInfo           ,
                       const TModuleInfoEntry& moduleInfoEntry   ,
                       const CORE::CString& targetPlatform       ,
                       TModuleInfoEntryConstPtrSet& dependencies ,
                       bool includeDependenciesOfDependencies    ,
                       bool includeRuntimeDependencies           );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
TModuleType
GetModuleType( const TModuleInfoEntry& moduleInfoEntry ,
               const CORE::CString& targetPlatform     );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
CORE::CString
GetModuleTargetName( const TModuleInfoEntry& moduleInfoEntry ,
                     const CORE::CString& targetPlatform     ,
                     bool useModuleNameIfNoTargetName        );

/*-------------------------------------------------------------------------*/

// Determines a list of module types and how they relate to the plaforms
// This functions has some smarts because it filters non-deviating info wrt
// a AllPlatforms definition or cases where a platform specific module definition
// yields to 'AllPlatforms' wrt the module type to use.
GUCEF_PROJECTGEN_PUBLIC_CPP
void
GetModuleInfoWithUniqueModulesTypes( const TModuleInfoEntry& moduleInfoEntry ,
                                     TConstModuleInfoPtrMap& moduleMap       );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
void
GetModuleInfoWithUniqueModuleNames( const TModuleInfoEntry& moduleInfoEntry ,
                                    TConstModuleInfoPtrMap& moduleMap       );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
const TModuleInfoEntry*
GetModuleInfoEntry( const TProjectInfo& projectInfo       ,
                    const CORE::CString& moduleName       ,
                    const CORE::CString& platform         ,
                    const TModuleInfo** moduleInfo = NULL );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
CORE::CString
LocalizeDirSepCharForPlatform( const CORE::CString& path     ,
                               const CORE::CString& platform );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
void
GetAllModuleInfoFilePaths( const TModuleInfoEntry& moduleInfoEntry ,
                           const CORE::CString& platform           ,
                           CORE::CString::StringSet& allPaths      ,
                           bool includeModuleRootPath              );
                 

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
void
GetAllModuleInfoFilePaths( const TProjectInfo& projectInfo         ,
                           const TModuleInfoEntry& moduleInfoEntry ,
                           const CORE::CString& platform           ,
                           CORE::CString::StringSet& allPaths      ,
                           bool includeModuleRootPath              ,
                           bool relativeToProjectRoot              );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
void
GetAllModuleInfoPaths( const TModuleInfoEntry& moduleInfoEntry ,
                       const CORE::CString& platform           ,
                       CORE::CString::StringSet& allPaths      ,
                       bool includeModuleRootPath              ,
                       bool includeDepencencyIncludePaths      );
                 

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
void
GetAllModuleInfoPaths( const TProjectInfo& projectInfo         ,
                       const TModuleInfoEntry& moduleInfoEntry ,
                       const CORE::CString& platform           ,
                       CORE::CString::StringSet& allPaths      ,
                       bool includeModuleRootPath              ,
                       bool relativeToProjectRoot              ,
                       bool includeDepencencyIncludePaths      );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
CORE::CString
GetShortestRelativePathFromModuleToProjectRoot( const TProjectInfo& projectInfo         ,
                                                const TModuleInfoEntry& moduleInfoEntry );


/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
CORE::CString
GetShortestRelativePathFromAbsPathToProjectRoot( const TProjectInfo& projectInfo ,
                                                 const CORE::CString& absPath    );

/*-------------------------------------------------------------------------*/

// Determines which platforms are actually used in the project and returns the 
// names of the platforms in the given platformList
GUCEF_PROJECTGEN_PUBLIC_CPP
void
GetAllPlatformsUsed( const TProjectInfo& projectInfo ,
                     TStringSet& platformList        );

/*-------------------------------------------------------------------------*/

// Collects a list of all unique tag values used
GUCEF_PROJECTGEN_PUBLIC_CPP
void
GetAllTagsUsed( const TProjectInfo& projectInfo ,
                TStringSet& tagsUsed            );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
IsModuleTagged( const TModuleInfoEntry& module ,
                const CORE::CString& tag       ,
                const CORE::CString& platform  );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
void
GetTaggedModules( const TProjectInfo& projectInfo            ,
                  const CORE::CString& tag                   ,
                  TModuleInfoEntryConstPtrSet& taggedModules ,
                  const CORE::CString& platform              );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
CORE::CString
GetLanguageForModule( const TModuleInfo& moduleInfo );
                      
/*-------------------------------------------------------------------------*/

// Determines whether the given list of module definitions has an indepdendant definition
// An independent definition is a module who has a module type defined which is not a purely logical
// module type used to structure modules (such as code/header integrate locations)
GUCEF_PROJECTGEN_PUBLIC_CPP
bool
HasIndependentModuleType( const TModuleInfoMap& moduleDefs );
                      
/*-------------------------------------------------------------------------*/

// if the platform name is a multiplatform name this function will resolve it as such
// and return the actual platforms the multiplatform name maps to. If the name is not
// a multiplatform name the same name is returned
GUCEF_PROJECTGEN_PUBLIC_CPP
TStringSet
ResolveMultiPlatformName( const CORE::CString& platformName          ,
                          const TPlatformDefinitionMap* platformDefs );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
ShouldModuleBeIgnored( const TModuleInfoEntry& moduleInfo ,
                       const CORE::CString& platformName  );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
IsModuleTaggedWith( const TModuleInfoEntry& moduleInfo ,
                    const CORE::CString& platformName  ,
                    const CORE::CString& tag           );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
void
GetExecutables( const TProjectInfo& projectInfo                ,
                TModuleInfoEntryConstPtrSet& executableTargets ,
                const CORE::CString& platform                  );

/*-------------------------------------------------------------------------*/

/**
 *  Splits out the projectInfo into different projects per platform
 *  Will only consider the platforms requested in "platformsUsed"
 */
GUCEF_PROJECTGEN_PUBLIC_CPP
void
SplitProjectPerTarget( const TProjectInfo& projectInfo   ,
                       TProjectTargetInfoMapMap& targets ,
                       bool tagsAsTargets                ,
                       bool collapseRedundantPlatforms   ,
                       const TStringSet& platformsUsed   );

/*-------------------------------------------------------------------------*/

/**
 *  Splits out the projectInfo into different projects per platform
 *  Same as the other variant of SplitProjectPerTarget() except that all known
 *  platforms are considered
 */
GUCEF_PROJECTGEN_PUBLIC_CPP
void
SplitProjectPerTarget( const TProjectInfo& projectInfo   ,
                       TProjectTargetInfoMapMap& targets ,
                       bool tagsAsTargets                ,
                       bool collapseRedundantPlatforms   );

/*-------------------------------------------------------------------------*/

/**
 *  Attempts to retrieve the project target for the given platform
 *  Note that not find a result even if the project exists can be a valid result
 *  because not all project targets need support the various platforms that other project targets support
 */
GUCEF_PROJECTGEN_PUBLIC_CPP
const TProjectTargetInfo*
GetPlatformProjectTarget( const TProjectTargetInfoMap& platformTargets ,
                          const CORE::CString& platformName            );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PROJECTGEN_DATATYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 27-11-2004 :
        - Dinand: Initial implementation

---------------------------------------------------------------------------*/
