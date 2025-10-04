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

#ifndef GUCEF_MT_CREADWRITELOCK_H
#include "gucefMT_CReadWriteLock.h"
#define GUCEF_MT_CREADWRITELOCK_H
#endif /* GUCEF_MT_CREADWRITELOCK_H ? */

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

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

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

class CProjectTargetInfo;
class CProjectTargetInfoBundle;
class CProjectInfo;
class CModuleInfo;
class CModuleInfoEntry;
class CModuleMetaData;

typedef CORE::CTBasicSharedPtr< CModuleInfo, MT::CMutex >      CModuleInfoPtr;
typedef CORE::CTBasicSharedPtr< CModuleInfoEntry, MT::CMutex > CModuleInfoEntryPtr;

/*---------------------------------------------------------------------------*/

typedef GUCEF::vector< CModuleInfoPtr > TModuleInfoPtrVector;
typedef GUCEF::set< CModuleInfoPtr > TModuleInfoPtrSet;
typedef GUCEF::map< CORE::CString, TModuleInfoPtrVector > TModuleInfoPtrVectorMap;
typedef GUCEF::map< CORE::CString, CModuleInfoPtr > TModuleInfoPtrMap;

/*---------------------------------------------------------------------------*/

typedef GUCEF::vector< CModuleInfoEntryPtr >                            TModuleInfoEntryPtrVector;
typedef GUCEF::map< CORE::CString, CModuleInfoEntryPtr >                TStringToModuleInfoEntryPtrMap;
typedef GUCEF::map< CORE::CString, TModuleInfoEntryPtrVector >          TStringToModuleInfoEntryPtrVectorMap;
typedef CORE::CTSharedPtr< TModuleInfoEntryPtrVector, MT::CMutex >      TModuleInfoEntryPtrVectorPtr;
typedef CORE::CTSharedPtr< TStringToModuleInfoEntryPtrMap, MT::CMutex > TStringToModuleInfoEntryPtrMapPtr;
typedef std::pair< const CModuleInfoEntryPtr, const CModuleInfoPtr >    TModuleInfoEntryPair;
typedef std::pair< CModuleInfoEntryPtr, CModuleInfoPtr >                TMutableModuleInfoEntryPair;
typedef GUCEF::vector< TModuleInfoEntryPair >                           TModuleInfoEntryPairVector;
typedef GUCEF::vector< TMutableModuleInfoEntryPair >                    TMutableModuleInfoEntryPairVector;
typedef GUCEF::set< CModuleInfoEntryPtr >                               TModuleInfoEntryPtrSet;
typedef GUCEF::map< Int64, CModuleInfoEntryPtr >                        TModuleInfoEntryPrioMap;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_PROJECTGEN_PUBLIC_CPP KnownPlatforms
{    
    public:

    static const CORE::CString AllPlatforms;
    static const CORE::CString Win32;
    static const CORE::CString Win64;
    static const CORE::CString Linux32;
    static const CORE::CString Linux64;
    static const CORE::CString Android32;
    static const CORE::CString Android64;
    static const CORE::CString Arduino;
    static const CORE::CString Emscripten32;
    static const CORE::CString Emscripten64;
};

class GUCEF_PROJECTGEN_PUBLIC_CPP KnownMultiPlatforms
{    
    public:

    static const CORE::CString MsWin;
    static const CORE::CString Linux;
    static const CORE::CString Android;
    static const CORE::CString Emscripten;
    static const CORE::CString LinuxLike;
    static const CORE::CString UnixLike;
    static const CORE::CString MacOS;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::CString::StringSet TStringSet;
typedef GUCEF::set< CORE::Int32 > TInt32Set;
typedef GUCEF::map< CORE::CString, Int32 >    TStringToInt32Map;
typedef CORE::CString::StringMapSet TStringSetMap;
typedef CORE::CString::StringVector TStringVector;
typedef GUCEF::map< CORE::CString, TStringVector > TStringVectorMap;
typedef GUCEF::map< CORE::CString, TStringVectorMap > TStringVectorMapMap;

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

typedef GUCEF::map< CORE::CString, TModuleType > TModuleTypeMap;

/*---------------------------------------------------------------------------*/

/**
 *  Class where all linker related information should be stored
 *  for a specific linked library
 */
class GUCEF_PROJECTGEN_PUBLIC_CPP CLinkedLibrarySettings : public CORE::CTSharedObjCreator< CLinkedLibrarySettings, MT::CMutex >
{
    public:

    typedef typename CORE::CTSharedObjCreator< CLinkedLibrarySettings, MT::CMutex >::TBasicSharedPtrType  CLinkedLibrarySettingsPtr;

    void SetModuleType( TModuleType moduleType );

    TModuleType GetModuleType( void ) const;

    void SetLibraryPath( const CORE::CString& libPath );

    const CORE::CString& GetLibraryPath( void ) const;

    bool Merge( const CLinkedLibrarySettings& linkedLibrarySettingsToMergeIn ,
                bool onConflictOriginalInfoStays = true                      );

    void Clear( void );

    CLinkedLibrarySettings( void );

    CLinkedLibrarySettings( const CLinkedLibrarySettings& src );

    virtual ~CLinkedLibrarySettings() GUCEF_VIRTUAL_OVERRIDE;

    private:

    TModuleType m_moduleType;               // Module type of the linked library if already known
    CORE::CString m_libPath;                // optional extra path for the linker to search for the given library
};

typedef CLinkedLibrarySettings::CLinkedLibrarySettingsPtr  CLinkedLibrarySettingsPtr;
typedef GUCEF::map< CORE::CString, CLinkedLibrarySettingsPtr > TLinkedLibrarySettingsPtrMap;

/*---------------------------------------------------------------------------*/

/**
 *  Class where all linker related information should be stored
 */
class GUCEF_PROJECTGEN_PUBLIC_CPP CLinkerSettings : public CORE::CTSharedObjCreator< CLinkerSettings, MT::CMutex >
{
    public:

    typedef typename CORE::CTSharedObjCreator< CLinkerSettings, MT::CMutex >::TBasicSharedPtrType  CLinkerSettingsPtr;

    bool MergeLinkedLibraries( const CLinkerSettings& linkerSettingsToMergeIn ,
                               bool onConflictOriginalInfoStays               );

    bool Merge( const CLinkerSettings& linkerSettingsToMergeIn ,
                bool onConflictOriginalInfoStays = true        );

    void Clear( void );

    const TLinkedLibrarySettingsPtrMap& GetLinkedLibraries( void ) const;

    const TLinkedLibrarySettingsPtrMap& GetLinkedLogicalLibraries( void ) const;

    bool HasLinkerDependency( const CORE::CString& dependencyName ) const;

    bool HasAnyStaticLibraryLinkerDependency( void ) const;

    void GetListOfLinkedLibraries( CORE::CStringSet& linkedLibraries ) const;

    bool TryGetLinkedLibrary( const CORE::CString& libraryName         ,                                     
                              CLinkedLibrarySettingsPtr& linkedLibrary ,
                              bool createDefaultIfNotExist             );

    bool TryGetLinkedLibrary( const CORE::CString& libraryName         ,
                              CLinkedLibrarySettingsPtr& linkedLibrary ) const;

    void DeleteLinkedLibrary( const CORE::CString& libraryName );

    bool AddLinkedLibraryAsCopy( const CORE::CString& libraryName               ,
                                 const CLinkedLibrarySettingsPtr& linkedLibrary );

    /**
     *  A logically linked library only exists as a concept in this code its not a standard
     *  concept. When a library is determined to be a logical name representing a concept to
     *  be processed it can be moved to the collection of 'logical' linked libraries
     */
    bool MoveLinkedLibraryToLogicalLibraries( const CORE::CString& libraryName );

    const TStringSet& GetLibraryPaths( void ) const;

    bool MergeLibraryPaths( const TStringSet& libPathsToMergeIn );

    void SetTargetName( const CORE::CString& targetName );

    const CORE::CString& GetTargetName( void ) const;

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     */
    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const;

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     *
     *  @param domRootNode Node that acts as root of the DOM data tree from which to deserialize
     *  @return whether deserializing the object data from the given DOM was successful.
     */
    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings );

    CLinkerSettings( void );

    CLinkerSettings( const CLinkerSettings& src );

    virtual ~CLinkerSettings() GUCEF_VIRTUAL_OVERRIDE;

    static bool IsStaticLibrary( const CORE::CString& testStr                         ,
                                 const CORE::CString** knownPlatformIfSo = GUCEF_NULL );

    private:

    TLinkedLibrarySettingsPtrMap m_linkedLibraries;           // list of all libraries the module links against
    TLinkedLibrarySettingsPtrMap m_linkedLogicalLibraries;    // list of all libraries the module links against
    TStringSet m_libPaths;                                    // list of hint paths where to look for libraries
    CORE::CString m_targetName;                               // optional name for the linker target if desired from the module name
};

typedef CLinkerSettings::CLinkerSettingsPtr  CLinkerSettingsPtr;

/*---------------------------------------------------------------------------*/

/**
 *  Class where all compiler related information should be stored
 */
class GUCEF_PROJECTGEN_PUBLIC_CPP CCompilerSettings : public CORE::CTSharedObjCreator< CCompilerSettings, MT::CMutex >
{
    public:

    CCompilerSettings( void );

    CCompilerSettings( const CCompilerSettings& src );

    virtual ~CCompilerSettings() GUCEF_VIRTUAL_OVERRIDE;

    void AddUsedLanguage( const CORE::CString& languageUsed );

    const CORE::CStringSet& GetLanguagesUsed( void ) const;

    const CORE::CStringMap& GetCompilerFlags( void ) const;

    bool Merge( const CCompilerSettings& compilerSettingsToMergeIn ,
                bool onConflictOriginalInfoStays = true            );

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     *
     *  @param domRootNode Node that acts as root of the DOM data tree from which to deserialize
     *  @return whether deserializing the object data from the given DOM was successful.
     */
    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings );

    void Clear( void );

    private:

    TStringSet m_languagesUsed;                // list of all programming languages used within this module
    CORE::CStringMap m_compilerFlags;          // map of flags to pass to the specific compilers
};

/*---------------------------------------------------------------------------*/

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
 *  Class where all preprocessor related information should be stored
 */
class GUCEF_PROJECTGEN_PUBLIC_CPP CPreprocessorSettings : public CORE::CTSharedObjCreator< CPreprocessorSettings, MT::CMutex >
{
    public:

    CPreprocessorSettings( void );

    CPreprocessorSettings( const CPreprocessorSettings& src );

    virtual ~CPreprocessorSettings() GUCEF_VIRTUAL_OVERRIDE;

    static CORE::CString SanitizeDefine( const CORE::CString& define );

    void AddDefine( const CORE::CString& define );

    const CORE::CStringSet& GetDefines( void ) const;

    bool Merge( const CPreprocessorSettings& preprocessorSettingsToMergeIn ,
                bool onConflictOriginalInfoStays = true                    );

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     *
     *  @param domRootNode Node that acts as root of the DOM data tree from which to deserialize
     *  @return whether deserializing the object data from the given DOM was successful.
     */
    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings );

    void Clear( void );

    private:

    CORE::CStringSet m_defines;   // list of all precompiler definitions for this module
};

/*---------------------------------------------------------------------------*/

struct SPlatformDefinition
{
    TStringSet aliases;                                    // alternate names for this platform, can be 1-N across multiple platforms as a group name
    TStringSet platformDirs;                               // Directory names which will be considered holders of platform specific files
};
typedef struct SPlatformDefinition TPlatformDefinition;
typedef GUCEF::map< CORE::CString, TPlatformDefinition > TPlatformDefinitionMap;

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

typedef GUCEF::map< CORE::CString, TDirProcessingInstructions >             TDirProcessingInstructionsMap;
typedef CORE::CTSharedPtr< TDirProcessingInstructionsMap, MT::CMutex >      TDirProcessingInstructionsMapPtr;

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
                      CProjectInfo& projectInfo           );
                     
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
MergeModuleInfo( CModuleInfo& targetModuleInfo          ,
                 const CModuleInfo& moduleInfoToMergeIn );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
MergeModuleInfo( const CModuleInfoEntryPtr& moduleInfo  ,
                 const CORE::CString& targetPlatform    ,
                 CModuleInfo& mergedModuleInfo          );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
MergeModuleInfoEntry( const CModuleInfoEntryPtr& moduleInfoEntryToMergeIn ,
                      CModuleInfoEntry& mergedModuleInfoEntry             );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
MergeModuleInfoEntries( const TStringToModuleInfoEntryPtrMap& moduleInfoEntriesToMergeIn ,
                        TStringToModuleInfoEntryPtrMap& moduleInfoEntries                );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
MergeAllModuleInfoForPlatform( const TStringToModuleInfoEntryPtrMap& allInfo ,
                               const CORE::CString& platform                 ,
                               TModuleInfoPtrVector& allMergedInfo           ,
                               TModuleInfoEntryPairVector& mergeLinks        );


/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
MergeAllModuleInfoForPlatform( const TModuleInfoEntryPtrSet& allInfo  ,
                               const CORE::CString& platform          ,
                               TModuleInfoPtrVector& allMergedInfo    ,
                               TModuleInfoEntryPairVector& mergeLinks );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
MergePlatformDefinition( TPlatformDefinition& targetPlatform          ,
                         const TPlatformDefinition& platformToMergeIn ,
                         bool caseSensitive                           );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
MergePlatformDefinitionMap( TPlatformDefinitionMap& targetPlatforms          ,
                            const TPlatformDefinitionMap& platformsToMergeIn ,
                            bool caseSensitive                               );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
MergeDirProcessingInstructions( TDirProcessingInstructions& mergedInstructions    ,
                                const TDirProcessingInstructions& newInstructions ,
                                bool caseSensitive                                );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
MergeDirProcessingInstructionsMap( TDirProcessingInstructionsMap& mergedInstructions    ,
                                   const TDirProcessingInstructionsMap& newInstructions ,
                                   bool caseSensitive                                   );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
const CModuleInfoPtr
FindModuleByName( const TModuleInfoEntryPairVector& mergeLinks ,
                  const CORE::CString& moduleName              );

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

// Determines a list of module types and how they relate to the plaforms
// This functions has some smarts because it filters non-deviating info wrt
// a AllPlatforms definition or cases where a platform specific module definition
// yields to 'AllPlatforms' wrt the module type to use.
GUCEF_PROJECTGEN_PUBLIC_CPP
void
GetModuleInfoWithUniqueModulesTypes( const CModuleInfoEntryPtr& moduleInfoEntry ,
                                     TModuleInfoPtrMap& moduleMap               );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
void
GetModuleInfoWithUniqueModuleNames( const CModuleInfoEntryPtr& moduleInfoEntry ,
                                    TModuleInfoPtrMap& moduleMap               );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
const CModuleInfoEntryPtr
GetModuleInfoEntry( const TModuleInfoEntryPtrVector& moduleInfoEntries ,
                    const CORE::CString& moduleName                    ,
                    const CORE::CString& platform                      ,
                    CModuleInfoPtr* moduleInfo = GUCEF_NULL            );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
CORE::CString
LocalizeDirSepCharForPlatform( const CORE::CString& path     ,
                               const CORE::CString& platform );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
void
GetAllModuleInfoFilePaths( const CModuleInfoEntryPtr& moduleInfoEntry ,
                           const CORE::CString& platform              ,
                           CORE::CString::StringSet& allPaths         ,
                           bool includeModuleRootPath                 );
                 

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
void
GetAllModuleInfoFilePaths( const CProjectInfo& projectInfo            ,
                           const CModuleInfoEntryPtr& moduleInfoEntry ,
                           const CORE::CString& platform              ,
                           CORE::CString::StringSet& allPaths         ,
                           bool includeModuleRootPath                 ,
                           bool relativeToProjectRoot                 );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
void
GetAllModuleInfoPaths( const CModuleInfoEntryPtr& moduleInfoEntry ,
                       const CORE::CString& platform              ,
                       CORE::CString::StringSet& allPaths         ,
                       bool includeModuleRootPath                 ,
                       bool includeDepencencyIncludePaths         );
                 

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
void
GetAllModuleInfoPaths( const CProjectInfo& projectInfo            ,
                       const CModuleInfoEntryPtr& moduleInfoEntry ,
                       const CORE::CString& platform              ,
                       CORE::CString::StringSet& allPaths         ,
                       bool includeModuleRootPath                 ,
                       bool relativeToProjectRoot                 ,
                       bool includeDepencencyIncludePaths         );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
CORE::CString
GetShortestRelativePathFromModuleToProjectRoot( const CProjectInfo& projectInfo            ,
                                                const CModuleInfoEntryPtr& moduleInfoEntry );


/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
CORE::CString
GetShortestRelativePathFromAbsPathToProjectRoot( const CProjectInfo& projectInfo ,
                                                 const CORE::CString& absPath    );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
IsDirALegacyModuleDir( const CORE::CString& dir );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
IsDirAModuleDir( const CORE::CString& dir );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
CORE::CString
GetLanguageForModule( const CModuleInfoPtr& moduleInfo );
                      
/*-------------------------------------------------------------------------*/

// Determines whether the given list of module definitions has an independent definition
// An independent definition is a module who has a module type defined which is not a purely logical
// module type used to structure modules (such as code/header integrate locations)
GUCEF_PROJECTGEN_PUBLIC_CPP
bool
HasIndependentModuleType( const TModuleInfoPtrMap& moduleDefs );
                      
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
ShouldModuleBeIgnored( const CModuleInfoEntryPtr& moduleInfo ,
                       const CORE::CString& platformName     );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
IsAnyLicenseDefined( const TStringToModuleInfoEntryPtrMap& moduleInfoEntries );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
const CORE::CString::StringSet&
GetKnownLicenseFiles( void );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
DirHasLicenseFile( const CORE::CString& path      ,
                   CORE::CString& licenceFilePath );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
TryAutoLicenceDetection( const CORE::CString& fileContent ,
                         CORE::CString& detectedLicense   );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
IsLicenseMIT( const CORE::CString& fileContentUpperCase );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
const CORE::CString::StringSet&
GetKnownSemVerFiles( void );

/*-------------------------------------------------------------------------*/;

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
IsAnySemVerDefined( const TStringToModuleInfoEntryPtrMap& moduleInfoEntries );

/*-------------------------------------------------------------------------*/;

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
DirHasSemVerFile( const CORE::CString& path     ,
                  CORE::CString& semverFilePath );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
TryAutoSemVerDetection( const CORE::CString& fileContent ,
                        CORE::CVersion& detectedSemVer   );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
IsStringInList( const TStringVector& list       ,
                bool caseSensitive              ,
                const CORE::CString& testString );

/*---------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
IsStringInList( const TStringSet& list          ,
                bool caseSensitive              ,
                const CORE::CString& testString );

/*---------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
IsStringInList( const TStringVector& list       ,
                bool caseSensitive              ,
                const CORE::CString& testString ,
                bool wildcardMatching           );

/*---------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
IsStringInList( const TStringVector& list       ,
                bool caseSensitive              ,
                const CORE::CString& testString ,
                bool wildcardMatching           );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PROJECTGEN_DATATYPES_H ? */
