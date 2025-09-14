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

    void GetListOfLinkedLibraries( CORE::CStringSet& linkedLibraries ) const;

    bool TryGetLinkedLibrary( const CORE::CString& libraryName         ,                                     
                              CLinkedLibrarySettingsPtr& linkedLibrary ,
                              bool createDefaultIfNotExist             );

    bool TryGetLinkedLibrary( const CORE::CString& libraryName         ,
                              CLinkedLibrarySettingsPtr& linkedLibrary ) const;

    void DeleteLinkedLibrary( const CORE::CString& libraryName );

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

class GUCEF_PROJECTGEN_PUBLIC_CPP CModuleMetaData : public CORE::CIDataNodeSerializable
{
    public:

    static const CORE::CString ClassTypeName;
    
    CORE::CString lastEditBy;             // optional info listing who last updated the information
    CORE::CString::StringSet authors;
    CORE::CString::StringSet maintainers;
    CORE::CString descriptionHeadline;
    CORE::CString descriptionDetails;
    CORE::CString license;

    CModuleMetaData( void );

    CModuleMetaData( const CModuleMetaData& src );

    virtual ~CModuleMetaData() GUCEF_VIRTUAL_OVERRIDE;

    void SetSemVer( const CORE::CVersion& semVer );

    const CORE::CVersion& GetSemVer( void ) const;

    bool HasSemVer( void ) const;

    bool HasAnyMetaData( void ) const;

    void Clear( void );

    bool Merge( const CModuleMetaData& moduleMetaDataToMergeIn ,
                bool onConflictOriginalInfoStays = true        );

    CModuleMetaData& operator=( const CModuleMetaData& src );

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     */
    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     *
     *  @param domRootNode Node that acts as root of the DOM data tree from which to deserialize
     *  @return whether deserializing the object data from the given DOM was successful.
     */
    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CORE::CVersion m_semver;
    bool m_hasSemVer;
};

/*---------------------------------------------------------------------------*/

class GUCEF_PROJECTGEN_PUBLIC_CPP CModuleInfo : public CORE::CTSharedObjCreator< CModuleInfo, MT::CMutex >
{
    public:

    typedef typename CORE::CTSharedObjCreator< CModuleInfo, MT::CMutex >::TBasicSharedPtrType    CModuleInfoPtr;
    
    CORE::CString name;                          // the name of the module
    TModuleType moduleType;                      // The type of module we are dealing with
    TStringSet tags;                             // optional tags that can be associated which allows filtering of modules

    TStringSet dependencyIncludeDirs;            // include directories needed for the headers of the dependencies, paths only no files
    TStringSet runtimeDependencies;              // dependencies not relative for builds but desired to be easily accessible due to runtime dependency, typically plugins

    Int64 buildOrder;                            // order number of this module in the build dependency chain
    bool considerSubDirs;                        // Whether only the dir with the ModuleInfo is to be considered or whether subdirs are recursively considered
    bool hasConsiderSubDirs;                     // Whether the considerSubDirs flag is based on an explicit setting or not

    CLinkerSettings linkerSettings;              // all linker related settings for this module
    CCompilerSettings compilerSettings;          // all compiler related settings for this module
    CPreprocessorSettings preprocessorSettings;  // all preprocessor related settings for this module

    bool ignoreModule;                           // whether this module should be included in the build
    bool hasIgnoreModule;                        // whether the ignoreModule flag is based on an explicit setting or not
    CModuleMetaData metadata;                    // module metadata

    void SetPlatformName( const CORE::CString& platformName );

    const CORE::CString& GetPlatformName( void ) const;

    void AddIncludeDir( const CORE::CString& pathToIncludeDir );

    void AddIncludeDirs( const CORE::CStringSet& pathsToIncludeDirs );

    bool RemoveIncludeDir( const CORE::CString& pathToIncludeDir ,
                           bool mustBeEmpty                      );

    void SetIncludeFiles( const TStringSetMap& files );

    void SetIncludeFiles( const CORE::CString& pathToFiles ,
                          const TStringSet& files          );

    void AddIncludeFiles( const CORE::CString& pathToFiles ,
                          const TStringSet& files          );

    void AddIncludeFiles( const TStringSetMap& files );

    void AddIncludeFile( const CORE::CString& pathToFiles ,
                         const CORE::CString& filename    );

    const TStringSetMap& GetIncludeDirs( void ) const;

    const TStringSet& GetIncludeFiles( const CORE::CString& pathToFiles ) const;

    void SetSourceFiles( const TStringSetMap& files );

    void SetSourceFiles( const CORE::CString& pathToFiles ,
                         const TStringSet& files          );

    void AddSourceFiles( const CORE::CString& pathToFiles ,
                         const TStringSet& files          );

    void AddSourceFiles( const TStringSetMap& files );

    void AddSourceFile( const CORE::CString& pathToFiles ,
                        const CORE::CString& filename    );

    // list of module names of all modules this module depends on
    void SetNamesOfDependencies( const TStringSet& dependencies );

    const TStringSetMap& GetSourceDirs( void ) const;

    // list of module names of all modules this module depends on
    const TStringSet& GetNamesOfDependencies( void ) const;

    void AddNameOfDependency( const CORE::CString& dependency );

    void RemoveNameOfDependency( const CORE::CString& dependency );

    bool HasDependencyWithName( const CORE::CString& dependency ) const;

    void MergeNamesOfDependencies( const TStringSet& dependenciesToMergeIn );

    const TStringSet& GetNamesOfLogicalDependencies( void ) const;

    bool HasLogicalDependencyWithName( const CORE::CString& dependency ) const;

    void AddNameOfRuntimeDependency( const CORE::CString& dependency );

    void RemoveNameOfRuntimeDependency( const CORE::CString& dependency );

    void AddDependencyIncludeDirs( const CORE::CStringSet& pathsToIncludeDirs );

    const TStringSet& GetNamesOfRuntimeDependencies( void ) const;

    /**
     *  Independent modules have relevance in their own right.
     *  Non-independent modules are logical constructs to aid with project organization
     */
    bool HasIndependentModuleType( void ) const;

    /**
     *  Returns whether a proper module type was set on the module
     */
    bool HasValidModuleType( void ) const;

    /**
     *  Returns whether the module is of a logical module type
     */
    bool IsLogicalModuleType( void ) const;

    void Clear( void );

    bool Merge( CModuleInfoPtr moduleInfoToMergeIn      ,
                bool onConflictOriginalInfoStays = true );

    /**
     *  If during module information processing a given listed dependency is revealed to be
     *  a logical module, an integration helper concept, this function can be used to move
     *  the dependency to the list of 'logical' dependencies which have different meaning
     */
    bool MoveDependencyToLogicalDependencies( const CORE::CString& dependency );

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
    
    CModuleInfo( void );
    CModuleInfo( const CModuleInfo& src );
    virtual ~CModuleInfo() GUCEF_VIRTUAL_OVERRIDE;

    private:

    CORE::CString m_platformName;
    TStringSet m_namesOfDependencies;
    TStringSet m_namesOfLogicalDependencies;

    TStringSetMap m_includeDirs;                   // include directories of this module's own headers
    TStringSetMap m_sourceDirs;                    // source directories of this module's own source

};

typedef CModuleInfo::CModuleInfoPtr CModuleInfoPtr;

/*---------------------------------------------------------------------------*/

typedef GUCEF::vector< CModuleInfoPtr > TModuleInfoPtrVector;
typedef GUCEF::set< CModuleInfoPtr > TModuleInfoPtrSet;
typedef GUCEF::map< CORE::CString, TModuleInfoPtrVector > TModuleInfoPtrVectorMap;
typedef GUCEF::map< CORE::CString, CModuleInfoPtr > TModuleInfoPtrMap;

/*---------------------------------------------------------------------------*/

class GUCEF_PROJECTGEN_PUBLIC_CPP CModuleInfoEntry : public CORE::CIDataNodeSerializable ,
                                                     public CORE::CTSharedObjCreator< CModuleInfoEntry, MT::CMutex >
{
    public:

    typedef typename CORE::CTSharedObjCreator< CModuleInfoEntry, MT::CMutex >::TBasicSharedPtrType   CModuleInfoEntryPtr;

    static const CORE::CString ClassTypeName;    

    CModuleInfoEntry( void );

    CModuleInfoEntry( const CModuleInfoEntry& src );

    virtual ~CModuleInfoEntry() GUCEF_VIRTUAL_OVERRIDE;

    bool GeneratePreprocessorDefinesFromModuleInfo( void );

    void Clear( void );

    bool SetLicense( const CORE::CString& license  ,
                     const CORE::CString& platform );

    bool IsAnyLicenseDefined( void ) const;

    bool SetSemVer( const CORE::CVersion& semver  ,
                    const CORE::CString& platform );

    bool GetSemVer( CORE::CVersion& semver        ,
                    const CORE::CString& platform );

    bool HasAnySemVer( void ) const;

    bool SetLastEditBy( const CORE::CString& lastEditBy ,
                        const CORE::CString& platform   );

    TModuleType GetModuleType( const CORE::CString& platform ) const;

    Int64 GetBuildOrder( const CORE::CString& platform        ,
                         bool autoConsiderAllPlatforms = true ) const;

    /**
     *  The name to use in config files etc can't always be multiple names or defined
     *  per platform. For that we have this function which looks at the different names
     *  available across the platforms and find the general consensus name which is the
     *  best guess name that could be used to label this module generally without specifying 
     *  a platform
     */
    const CORE::CString& GetConsensusName( CModuleInfoPtr* moduleInfo = GUCEF_NULL ,
                                           bool dontUseCached = false              ) const;

    const CORE::CString* GetModuleName( const CORE::CString& targetPlatform        ,
                                        CModuleInfoPtr* outModuleInfo = GUCEF_NULL ) const;

    /**
     *  Tries to get the properly merged module name first for the given platform
     *  if this does not yield a module name it will use the general consensus
     *  module name instead
     */
    CORE::CString GetModuleNameAlways( const CORE::CString& targetPlatform     ,
                                       CModuleInfoPtr* moduleInfo = GUCEF_NULL ) const;

    void SetModuleInfo( CModuleInfoPtr moduleInfo     ,
                        const CORE::CString& platform );

    const CModuleInfoPtr FindModuleInfoForPlatform( const CORE::CString& platform              ,
                                                    bool considerAllPlatformAsFallback = false ) const;

    CModuleInfoPtr FindOrCreateModuleInfoForPlatform( const CORE::CString& platform ,
                                                      bool createNewIfNoneExists    );

    bool HasAnyModuleInfoWithModuleType( TModuleType moduleType ) const;

    bool FindAllModuleInfoWithModuleType( TModuleType moduleType       ,
                                          TModuleInfoPtrSet& foundInfo ) const;

    CModuleInfoPtr FindModuleInfoWithModuleType( TModuleType moduleType                     ,
                                                 const CORE::CString& platform              ,
                                                 bool considerAllPlatformAsFallback = false ) const;

    const TModuleInfoPtrMap& GetModulesPerPlatform( void ) const;

    const TModuleInfoPtrMap& GetFlattenedModulesPerPlatform( void ) const;

    /**
     *  Obtains the flattened module info for the given platform if any exists
     *  Note that this is cached info which is generated only as requested via
     *          GenerateFlattenedModuleInfo()
     *  The cached flattened info may be out of date if the module info was altered afterwards
     *
     *  The 'flattened' module info is a feature provided to allow simpler and more efficient
     *  logic for generators which only support a singular platform and as such can be spared
     *  their own platform overlay logic.
     */
    const CModuleInfoPtr FindFlattenedModuleInfoForPlatform( const CORE::CString& platform ) const;

    /**
     *  Obtains the flattened module info for the given platform if any exists or can be generated
     *  If no flattened info exists it will be generated on the fly via
     *          GenerateFlattenedModuleInfo()
     *  The cached flattened info may be out of date if the module info was altered afterwards
     *
     *  The 'flattened' module info is a feature provided to allow simpler and more efficient
     *  logic for generators which only support a singular platform and as such can be spared
     *  their own platform overlay logic.
     */
    const CModuleInfoPtr FindOrCreateFlattenedModuleInfoForPlatform( const CORE::CString& platform );

    bool HasDependency( const CORE::CString& platform       ,
                        const CORE::CString& dependencyName ,
                        bool platformSpecificOnly = false   ) const;

    bool HasLogicalDependency( const CORE::CString& platform       ,
                               const CORE::CString& dependencyName ,
                               bool platformSpecificOnly = false   ) const;

    bool HasLinkerDependency( const CORE::CString& platform       ,
                              const CORE::CString& dependencyName ,
                              bool platformSpecificOnly = false   ) const;

    /**
     *  Returns whether the given module as configured is applicable for the given platform
     *  Many modules will only support certain platforms due to depending on platform specifics
     */
    bool IsApplicableForPlatform( const CORE::CString& platform ) const;

    /**
     *  Independent modules have relevance in their own right.
     *  Non-independent modules are logical constructs to aid with project organization
     *  Such modules should not be considered actual 'modules' from a build perspective
     */
    bool HasIndependentModuleTypeForPlatform( const CORE::CString& platform ) const;

    /**
     *  Logical modules are organizational helper constructs and not real modules
     *  They wont result in an executable or .dll/.so etc on their own
     */
    bool HasOnlyLogicalModuleType( void ) const;

    /**
     *  Logical modules are organizational helper constructs and not real modules
     *  They wont result in an executable or .dll/.so etc on their own
     */
    bool HasAnyLogicalModuleType( void ) const;

    /**
     *  If a module is written as generic code, not hard dependent on a given platform(s)
     *  it should have a valid AllPlatforms definition with potentially platform definitions
     *  as extensions to said definition for a given platform.
     *
     *  @param onlyConsiderValidModulesTypes whether to check if the module type is not undefined if an entry does exist
     */
    bool HasAllPlatformsDefinition( bool onlyConsiderValidModulesTypes = true ) const;

    /**
     *  Provides flag whether issues have been detected with the definition such that its considered 'broken'
     *  Broken modules may be ignored and filtered out by generator backends
     */
    void SetIsBroken( bool isBroken );

    /**
     *  Provides flag whether issues have been detected with the definition such that its considered 'broken'
     *  Broken modules may be ignored and filtered out by generator backends
     */
    bool IsBroken( void ) const;

    /**
     *  Obtains the module dependencies by flattening the dependencies on-demand
     *  for the given target platform.
     *
     *  These dependencies may or may not be linked to, that is distinct (see GetModuleLinkerDependencyNames() )
     *  These are used to discern which paths to generate for include headers
     */
    void
    GetModuleDependencyNames( const CORE::CString& targetPlatform ,
                              TStringSet& dependencies            ) const;

    /**
     *  Obtains the logical module dependencies by flattening the dependencies on-demand
     *  for the given target platform.
     * 
     *  A logical dependency is a dependency on something that only exists logically to help with the
     *  management of the overall project and interdependencies. It would not translate by itself into
     *  something that turns into an executable or shared library and the like.
     */
    void
    GetLogicalDependencyNames( const CORE::CString& targetPlatform ,
                               TStringSet& dependencies            ) const;

    /**
     *  Obtains the module linker dependencies by flattening the dependencies on-demand
     *  for the given target platform. Linker dependencies are what are actually needed by the build process.
     */
    void
    GetModuleLinkerDependencyNames( const CORE::CString& targetPlatform ,
                                    TStringSet& dependencies            ) const;

    /**
     *  Obtains the module dependencies by flattening the runtime dependencies on-demand
     *  for the given target platform
     */
    void
    GetModuleRuntimeDependencyNames( const CORE::CString& targetPlatform ,
                                     TStringSet& dependencies            ) const;

    /**
     *  Removes any reference to the a runtime dependency with the given name
     */
    void RemoveRuntimeDependency( const CORE::CString& dependencyName );

    /**
     *  Removes any reference to the a runtime dependency with the given name
     */
    void RemoveRuntimeDependency( const CORE::CString& dependencyName ,
                                  const CORE::CString& platformName   );

    /**
     *  Obtains the set of platforms for which this module has information
     */
    void
    GetReferencedPlatforms( TStringSet& platforms ) const;

    /**
     *  Straightforward upsert style merge
     *  Does not use business logic for merging
     */
    bool Merge( const CModuleInfoEntryPtr& infoToMergeIn ,
                bool onConflictOriginalInfoStays = true  );

    /**
     *  Will generate the flattened module info for the given target platform which will be retained as a snapshot cache
     * 
     *  Note that if you change the module info after this call you will need to call this function again to refresh the flattened info if needed
     * 
     *  Note that this function will NOT generate flattened info for all platforms, only the one given and only if a
     *  valid module definition exists for the given platform and/or the 'all platforms' definition
     */
    bool GenerateFlattenedModuleInfo( const CORE::CString& targetPlatform );

    CModuleInfoEntry& operator=( const CModuleInfoEntry& src );

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     */
    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Attempts to serialize the object and write the result to a file at the path given
     *  Uses XML format
     */
    bool Serialize( const CORE::CString& outputFilePath                 ,
                    const CORE::CDataNodeSerializableSettings& settings ) const;

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     *
     *  @param domRootNode Node that acts as root of the DOM data tree from which to deserialize
     *  @return whether deserializing the object data from the given DOM was successful.
     */
    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Provides all the source folders and their contents for the given platform
     *  By default will merge the dirs/files across the 'all' platform and the given platform to the extent applicable
     */
    void GetSourceFilesForPlatform( const CORE::CString& platformName    ,
                                    TStringSetMap& files                 ,
                                    bool autoConsiderAllPlatforms = true ) const;

    /**
     *  Provides all the include folders and their contents for the given platform
     *  By default will merge the dirs/files across the 'all' platform and the given platform to the extent applicable
     */
    void GetIncludeFilesForPlatform( const CORE::CString& platformName    ,
                                     TStringSetMap& files                 ,
                                     bool autoConsiderAllPlatforms = true ) const;

    /**
     *  Provides all the include folders for the given platform
     *  By default will merge the dirs across the 'all' platform and the given platform to the extent applicable
     */
    void GetIncludeDirsForPlatform( const CORE::CString& platformName      ,
                                    TStringSet& subDirPaths                ,
                                    bool autoConsiderAllPlatforms = true   ,
                                    bool includeDependencyIncludes = false ) const;

    /**
     *  Provides all the dependency based include folders for the given platform
     *  By default will merge the dirs across the 'all' platform and the given platform to the extent applicable
     */
    void GetDependencyIncludeDirsForPlatform( const CORE::CString& platformName    ,
                                              TStringSet& subDirPaths              ,
                                              bool autoConsiderAllPlatforms = true ) const;

    /**
     *  Provides all the paths to include folders for the given platform using the given 'from' path
     *  By default will merge the dirs across the 'all' platform and the given platform to the extent applicable
     */
    void GetPathsToIncludeDirsForPlatform( const CORE::CString& platformName    ,
                                           const CORE::CString& otherFromPath   ,
                                           TStringSet& pathsToSubDirPaths       ,
                                           bool autoConsiderAllPlatforms = true ) const;

    /**
     *  Function to double check dependency includes vc include file paths etc to remove redundancies
     */
    void CleanupIncludeDirs( void );

    void SetAbsolutePathToModuleRootDir( const CORE::CString& absPathToRootDir );

    const CORE::CString& GetAbsolutePathToModuleRootDir( void ) const;

    void SetProjectRelativePathToModuleRootDir( const CORE::CString& relPathToRootDir );

    const CORE::CString& GetProjectRelativePathToModuleRootDir( void ) const;

    void SetDefinitionFileLastModifiedDt( const CORE::CDateTime& fileLastModifiedDt );

    const CORE::CDateTime& GetDefinitionFileLastModifiedDt( void ) const;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    mutable CORE::CString m_consensusName;          // Derived cached info: the consensus name across all platforms    
    TModuleInfoPtrMap m_modulesPerPlatform;         // ModuleInfo per platform
    TModuleInfoPtrMap m_flattenedInfoPerPlatform;   // Derived cached info: Module overlays per platform applied to the 'all platforms' module info
    bool m_isBroken;                                // flag whether the module has a broken definition in need of intervention
    CORE::CString m_absRootDir;                     // the absolute path to the root of this module's directory tree
    CORE::CString m_projRelRootDir;                 // the project root relative path to the root of this module's directory tree
    CORE::CDateTime m_definitionFileLastModifiedDt; // When loaded from a file this has the last modified datetime of said file at the time of load
};

typedef CModuleInfoEntry::CModuleInfoEntryPtr CModuleInfoEntryPtr;

/*---------------------------------------------------------------------------*/

typedef GUCEF::vector< CModuleInfoEntryPtr >                            TModuleInfoEntryPtrVector;
typedef GUCEF::map< CORE::CString, CModuleInfoEntryPtr >                TStringToModuleInfoEntryPtrMap;
typedef CORE::CTSharedPtr< TModuleInfoEntryPtrVector, MT::CMutex >      TModuleInfoEntryPtrVectorPtr;
typedef CORE::CTSharedPtr< TStringToModuleInfoEntryPtrMap, MT::CMutex > TStringToModuleInfoEntryPtrMapPtr;
typedef std::pair< const CModuleInfoEntryPtr, const CModuleInfoPtr >    TModuleInfoEntryPair;
typedef std::pair< CModuleInfoEntryPtr, CModuleInfoPtr >                TMutableModuleInfoEntryPair;
typedef GUCEF::vector< TModuleInfoEntryPair >                           TModuleInfoEntryPairVector;
typedef GUCEF::vector< TMutableModuleInfoEntryPair >                    TMutableModuleInfoEntryPairVector;
typedef GUCEF::set< CModuleInfoEntryPtr >                               TModuleInfoEntryPtrSet;
typedef GUCEF::map< Int64, CModuleInfoEntryPtr >                        TModuleInfoEntryPrioMap;


/*---------------------------------------------------------------------------*/

class GUCEF_PROJECTGEN_PUBLIC_CPP CModuleDependencyNode : public CORE::CTSharedObjCreator< CModuleDependencyNode, MT::CMutex >
{
    public:

    typedef typename CORE::CTSharedObjCreator< CModuleDependencyNode, MT::CMutex >::TBasicSharedPtrType    CModuleDependencyNodePtr;
    typedef GUCEF::map< CORE::CString, CModuleDependencyNodePtr >                                          TModuleDependencyNodePtrMap;
    typedef GUCEF::set< CModuleDependencyNodePtr >                                                         TModuleDependencyNodePtrSet;

    void SetModule( const CModuleInfoEntryPtr& module );

    CModuleInfoEntryPtr GetModule( void ) const;

    Int64 GetBuildOrder( void ) const;

    const CORE::CString& GetConsensusName( void ) const;

    bool SetDependency( CModuleDependencyNodePtr dependency );

    void SetDependencies( const TModuleDependencyNodePtrMap& dependencies );

    const TModuleDependencyNodePtrMap& GetDependencies( void ) const;

    bool GatherDependencyModules( TModuleInfoEntryPtrSet& dependencies                  ,
                                  bool includeDependenciesOfDependencies                ,
                                  bool includeLogicalDependenciesOfDependencies = false ) const;

    bool SetLinkerDependency( CModuleDependencyNodePtr dependency );

    void SetLinkerDependencies( const TModuleDependencyNodePtrMap& dependencies );

    const TModuleDependencyNodePtrMap& GetLinkerDependencies( void ) const;

    bool GatherLinkerDependencyModules( TModuleInfoEntryPtrSet& dependencies                  ,
                                        bool includeDependenciesOfDependencies                ,
                                        bool includeLogicalDependenciesOfDependencies = false ) const;

    bool SetRuntimeDependency( CModuleDependencyNodePtr dependency );

    void SetRuntimeDependencies( const TModuleDependencyNodePtrMap& dependencies );

    const TModuleDependencyNodePtrMap& GetRuntimeDependencies( void ) const;

    bool GatherRuntimeDependencyModules( TModuleInfoEntryPtrSet& dependencies                  ,
                                         bool includeDependenciesOfDependencies                ,
                                         bool includeLogicalDependenciesOfDependencies = false ) const;

    bool SetLogicalDependency( CModuleDependencyNodePtr dependency );

    void SetLogicalDependencies( const TModuleDependencyNodePtrMap& dependencies );

    const TModuleDependencyNodePtrMap& GetLogicalDependencies( void ) const;

    bool GatherLogicalDependencyModules( TModuleInfoEntryPtrSet& dependencies    ,
                                         bool includeDependenciesOfDependencies  ) const;

    bool SetDependent( CModuleDependencyNodePtr dependent );

    void SetDependents( const TModuleDependencyNodePtrMap& dependents );

    const TModuleDependencyNodePtrMap& GetDependents( void ) const;

    bool GatherDependentModules( TModuleInfoEntryPtrSet& dependents ,
                                 bool includeDependentsOfDependents ) const;

    bool SetLinkerDependent( CModuleDependencyNodePtr dependent );

    void SetLinkerDependents( const TModuleDependencyNodePtrMap& dependents );

    const TModuleDependencyNodePtrMap& GetLinkerDependents( void ) const;

    bool GatherLinkerDependentModules( TModuleInfoEntryPtrSet& dependents ,
                                       bool includeDependentsOfDependents ) const;

    bool SetRuntimeDependent( CModuleDependencyNodePtr dependent );

    void SetRuntimeDependents( const TModuleDependencyNodePtrMap& dependents );

    const TModuleDependencyNodePtrMap& GetRuntimeDependents( void ) const;

    bool GatherRuntimeDependentModules( TModuleInfoEntryPtrSet& dependents ,
                                        bool includeDependentsOfDependents ) const;

    bool SetLogicalDependent( CModuleDependencyNodePtr dependent );

    void SetLogicalDependents( const TModuleDependencyNodePtrMap& dependents );

    const TModuleDependencyNodePtrMap& GetLogicalDependents( void ) const;

    bool GatherLogicallyDependentModules( TModuleInfoEntryPtrSet& dependents ,
                                          bool includeDependentsOfDependents ) const;

    bool GatherDependenciesOfDependencies( TModuleInfoEntryPtrSet& dependencies ,
                                           bool includeLogicalDependencies      ) const;

    void SetTargetPlatform( const CORE::CString& targetPlatform );

    const CORE::CString& GetTargetPlatform( void ) const;

    void Clear( void );

    bool HasMissingDependencies( void ) const;

    void SetHasMissingDependencies( bool hasMissing );

    bool GetDependencyModulesMappedByBuildOrder( TModuleInfoEntryPrioMap& modulesMappedByBuildOrder ,
                                                 bool includeDependenciesOfDependencies             ,
                                                 bool addDependencies                               ,
                                                 bool addLinkerDependencies                         ,
                                                 bool addRuntimeDependencies                        ) const;

    CModuleDependencyNode( void );

    virtual ~CModuleDependencyNode() GUCEF_VIRTUAL_OVERRIDE;

    private:

    bool GatherDependenciesOfDependenciesImpl( const TModuleDependencyNodePtrMap& dependenciesAtLevel ,
                                               TModuleInfoEntryPtrSet& dependencies                   ,
                                               bool includeLogicalDependencies                        ) const;

    private:

    CModuleInfoEntryPtr m_module;
    TModuleDependencyNodePtrMap m_dependencies;
    TModuleDependencyNodePtrMap m_linkerDependencies;
    TModuleDependencyNodePtrMap m_runtimeDependencies;
    TModuleDependencyNodePtrMap m_logicalDependencies;
    TModuleDependencyNodePtrMap m_dependents;
    TModuleDependencyNodePtrMap m_linkerDependents;
    TModuleDependencyNodePtrMap m_runtimeDependents;
    TModuleDependencyNodePtrMap m_logicalDependents;
    CORE::CString m_targetPlatform;
    bool m_hasMissingDependencies;
};

typedef CModuleDependencyNode::CModuleDependencyNodePtr             CModuleDependencyNodePtr;   
typedef CModuleDependencyNode::TModuleDependencyNodePtrMap          TModuleDependencyNodePtrMap;
typedef CModuleDependencyNode::TModuleDependencyNodePtrSet          TModuleDependencyNodePtrSet;
typedef std::map< CORE::CString, TModuleDependencyNodePtrMap >      TStringToModuleDependencyNodePtrMap;

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

typedef std::map< CORE::CString, TDirProcessingInstructions >               TDirProcessingInstructionsMap;
typedef CORE::CTSharedPtr< TDirProcessingInstructionsMap, MT::CMutex >      TDirProcessingInstructionsMapPtr;

/*---------------------------------------------------------------------------*/

class GUCEF_PROJECTGEN_PUBLIC_CPP CProjectInfo : public CORE::CTSharedObjCreator< CProjectInfo, MT::CMutex > ,
                                                 public CORE::CIDataNodeSerializable ,
                                                 public MT::CILockable
{
    public:

    static const CORE::CString ClassTypeName;

    CORE::CString projectName;                               // Name of the overall project
    TStringVector rootDirs;                                  // Root dirs used to gather all project info
    TStringToModuleInfoEntryPtrMap modules;                  // All generated module information
    TDirProcessingInstructionsMap dirProcessingInstructions; // All loaded processing instructions mapped per path
    TStringVector globalDirExcludeList;                      // Dirs that should never be included in processing regardless of path
    TPlatformDefinitionMap platforms;                        // All supported platforms for this project    

    CModuleInfoEntryPtr
    GetModuleInfoEntry( const CORE::CString& consensusModuleName ) const;

    CModuleInfoEntryPtr
    GetModuleInfoEntry( const CORE::CString& consensusModuleName        ,
                        const CORE::CString& platform                   ,
                        CModuleInfoPtr* platformModuleInfo = GUCEF_NULL ) const;

    CModuleInfoEntryPtr GetModuleInfoEntry( const CORE::CString& moduleName    ,
                                            const CORE::CString& platform      ,
                                            CModuleInfoPtr* platformModuleInfo ,
                                            bool createNewIfNoneExists         );

    /**
     *  Determines which platforms are actually used in the project and returns the 
     *  names of the platforms in the given platformList
     * 
     *  If you want to exclude platforms which are not enabled even if the data may reference them
     *  please use GetAllEnabledPlatformsUsed()
     */
    void
    GetAllPlatformsUsed( TStringSet& platformList       ,
                         bool okToUseCachedValue = true ) const;

    /**
     *  Determines which platforms are actually used in the project AND are enabled for processing
     *  and returns the names of the platforms in the given platformList
     * 
     *  If you want ALL platforms which referenced regardless of them being disabled
     *  please use GetAllPlatformsUsed()
     */
    void
    GetAllEnabledPlatformsUsed( TStringSet& platformList ,
                                bool okToUseCachedValue  ) const;

    void
    GetNamesOfModulesWhichDependOnModuleForPlatform( const CORE::CString& targetPlatform ,
                                                     const CORE::CString& dependencyName ,
                                                     CORE::CStringSet& moduleNames       ,
                                                     bool platformSpecificOnly = false   ) const;

    void
    GetNamesOfModulesWhichLinkerDependOnModuleForPlatform( const CORE::CString& targetPlatform ,
                                                           const CORE::CString& dependencyName ,
                                                           CORE::CStringSet& moduleNames       ,
                                                           bool platformSpecificOnly = false   ) const;

    void
    GetNamesOfModulesWhichRuntimeDependOnModuleForPlatform( const CORE::CString& targetPlatform ,
                                                            const CORE::CString& dependencyName ,
                                                            CORE::CStringSet& moduleNames       ,
                                                            bool platformSpecificOnly = false   ) const;

    void
    GetNamesOfModulesWhichLogicallyDependOnModuleForPlatform( const CORE::CString& targetPlatform ,
                                                              const CORE::CString& dependencyName ,
                                                              CORE::CStringSet& moduleNames       ,
                                                              bool platformSpecificOnly = false   ) const;

    bool TryGetModuleDependencyChain( CModuleDependencyNodePtr& dependencyChain ,
                                      const CORE::CString& consensusModuleName  ,
                                      const CORE::CString& targetPlatform       ,
                                      bool onlyCheckPlatformSpecific            ) const;

    bool GetModuleDependencies( const CModuleInfoEntryPtr& moduleInfoEntry ,
                                const CORE::CString& targetPlatform        ,
                                TModuleInfoEntryPtrSet& dependencies       ,
                                bool includeDependenciesOfDependencies     ,
                                bool includeRuntimeDependencies            ,
                                bool includeLogicalDependencies            ) const;

    void GetModuleDependencies( const CModuleInfoEntryPtr& moduleInfoEntry ,
                                const CORE::CString& targetPlatform        ,
                                TStringSet& dependencies                   ,
                                bool includeRuntimeDependencies            ,
                                bool includeLogicalDependencies            ) const;

    bool
    FindModulesWhichDependOnModuleForPlatform( TMutableModuleInfoEntryPairVector& foundModules ,
                                               const CORE::CString& targetPlatform             ,
                                               const CORE::CString& dependencyName             ,
                                               bool tryToUseDependencyChains                   ,
                                               bool includeLogicalDependents                   ) const;

    bool
    FindModulesWhichDependOnModule( TMutableModuleInfoEntryPairVector& foundModules ,
                                    const CORE::CString& dependencyName             ,
                                    bool tryToUseDependencyChains                   ,
                                    bool includeLogicalDependents                   ) const;

    bool
    FindModulesWhichDependOnModule( TModuleInfoEntryPtrSet& foundModules ,
                                    const CORE::CString& dependencyName  ,
                                    bool tryToUseDependencyChains        ,
                                    bool includeLogicalDependents        ) const;

    bool FindModulesWithModuleType( TModuleType moduleType               ,
                                    TModuleInfoEntryPtrSet& foundModules ) const;

    /**
     *  Builds or updates dependency chain trees based on the name based module references
     *  This allows directly traversing dependency trees after the tree has been build
     *  versus relying on the initial names as used in config, which is a lot slower lookup wise
     */
    bool UpdateDependencyChains( bool okToUseCachedValuesWhereApplicable = true );

    /**
     *  Updates the information contained within a specific dependency chain
     *  This depends on chain nodes already having been created for every module by
     *  UpdateDependencyChains()
     */
    bool UpdateDependencyChain( CModuleDependencyNodePtr dependencyChain );

    void ClearDependencyChains( void );

    /**
     *  Sanity check functionality which will verify for all defined modules whether they can sustain
     *  an 'all' platforms definition. Any module found to be unable to sustain such a definition will be
     *  reduced to the feasible platforms based on the dependency chain
     */
    bool SanitizeAllPlatformsUsage( void );

    /**
     *  Checks a module's dependency tree to verify whether based on dependencies an All platforms
     *  definition is sustainable. In order for that to be true we cannot have any missing dependencies
     * 
     *  Its valid for an 'All' platform definition to have platform specific extensions to allow it to work on a given platform
     *  Such extensions must from that point forward all support 'All' platform or the given platform for the remainder of that chain.
     *  The problem comes when an 'All' platforms definition directly depends on a non-all-platforms dependency. That is invalid wrt sustaining
     *  a 'all' platforms definition. It has to be via a platform specific definition extension, not part of the 'all' platforms definition in that case
     *  This function looks for such cases throughout the dependency tree and validates that the requirement holds.
     *  Its all or nothing. It only takes one bad link in the chain to make all dependent modules lose their 'all' platform sustainability.      
     */
    bool CanModuleSustainAllPlatformsDefinition( CModuleInfoEntryPtr moduleInfoEntry    ,
                                                 TModuleInfoEntryPtrSet& problemModules ,
                                                 CORE::CStringSet& missingModules       ) const;

    /**
     *  Obtains the list of platforms derived as being supported by checking all the module
     *  dependencies and finding the actually supported subset of platforms
     */
    void
    GetSupportedPlatformsBasedOnDependencies( CModuleInfoEntryPtr moduleInfoEntry ,
                                              CORE::CStringSet& platforms         ) const;

    bool SanitizeModulePlatformUsage( CModuleInfoEntryPtr moduleInfoEntry );

    bool SanitizeRuntimeDependencies( CModuleInfoEntryPtr moduleInfoEntry );

    bool SanitizeRuntimeDependencies( void );

    bool DetermineBuildOrderForAllModules( void );

    void DetermineBuildOrderForAllModulesForPlatform( const CORE::CString& targetPlatform );

    void Clear( void );

    bool AreDependencyChainsInitialized( void ) const;

    /**
     *  After bulk loading or discovering various modules this function can be used to post-process
     *  the information to take care of all the automatically derives information and relationships
     *  Some of these things require a particular order of operations so its advised to use this function
     *  for bulk application of such operations.
     */
    bool BulkPostProcessAllModuleInfo( void );

    /**
     *  Modules can have dependencies which are logical in nature
     *  Logical dependencies are organizational constructs. Usually things which would have been directly specified
     *  by a developer on the module that depends on such a logical dependency. With automation it makes sense to
     *  centralize such things for easy reference from multiple locations, easing management with centralized administration.
     *
     *  This function will determine if a given dependency is a logical one and if so will move it under the category
     *  of logical dependencies if it wasn't there already. This allows modules to simply specify dependencies and not
     *  worry about whether its a logical dependency or not
     */
    bool DetermineAndRelocateLogicalDependencies( void );

    /**
     *  Binary packages which are taken as dependencies may have various properties that you need merged into the dependent module
     *  The concept of a binary package is to represent that set of properties such that it can be simplistically referenced
     *  The actual settings themselves remain centralized and easy to maintain while getting merged in as needed.
     *  This function takes care of merging in any such dependencies not already merged in
     */
    bool MergeAllBinaryPackageInfoIntoModules( void );

    bool MergeBinaryPackageInfoIntoModules( CModuleInfoEntryPtr binaryPackageModule );

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     */
    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Attempts to serialize the object to a file in XML format
     *
     *  @param outputFilePath Path to the file to write to
     *  @return whether serializing the object data to the given file was successful.
     */
    bool Serialize( const CORE::CString& outputFilePath                 ,
                    const CORE::CDataNodeSerializableSettings& settings ) const;

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     *
     *  @param domRootNode Node that acts as root of the DOM data tree from which to deserialize
     *  @return whether deserializing the object data from the given DOM was successful.
     */
    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;


    bool Deserialize( const CORE::CString& inputFilepath );

    bool DeserializeModuleEntries( const CORE::CDataNode& domRootNode                  ,
                                   const CORE::CDataNodeSerializableSettings& settings ,
                                   size_t suggestedNrOfModules = 0                     );

    static bool DeserializeModuleEntries( const CORE::CDataNode& domRootNode                  ,
                                          const CORE::CDataNodeSerializableSettings& settings ,
                                          TStringToModuleInfoEntryPtrMap& moduleInfoEntries   );

    static bool DeserializeModuleEntries( const CORE::CString& pathToModuleInfoFile           ,
                                          const CORE::CDataNodeSerializableSettings& settings ,
                                          TStringToModuleInfoEntryPtrMap& moduleInfoEntries   );

    CModuleInfoEntryPtr FindModuleAccordingToBuildOrder( const CORE::CString& targetPlatform ,
                                                         Int64 buildOrderIndex               ) const;

    CModuleInfoEntryPtr FindFirstModuleAccordingToBuildOrder( const CORE::CString& targetPlatform ) const;

    bool GenerateDependencyIncludes( void );

    void GenerateDependencyIncludesForPlatform( const CORE::CString& platformName );

    bool GeneratePreprocessorDefinesFromModuleInfo( void );

    bool MergeIntegrationLocationsIntoModules( void );

    bool GetModulesMappedByBuildOrder( const CORE::CString& targetPlatform                ,
                                       TModuleInfoEntryPrioMap& modulesMappedByBuildOrder ) const;

    bool GetModulesMappedByBuildOrderForTarget( const CORE::CString& consensusModuleName           ,
                                                const CORE::CString& targetPlatform                ,
                                                TModuleInfoEntryPrioMap& modulesMappedByBuildOrder ,
                                                bool includeDependenciesOfDependencies             ,
                                                bool addDependencies                               ,
                                                bool addLinkerDependencies                         ,
                                                bool addRuntimeDependencies                        ) const;

    bool FlagTaggedModulesToIgnoreAsSpecified( const CORE::CValueList& params );

    const CORE::CValueList& GetSettings( void ) const;

    void SetSetttings( const CORE::CValueList& settings );

    void SetRootDirs( const TStringVector& rootDirs );

    void SetRootDir( const CORE::CString& rootDir );

    CORE::CString GetRelativePathFromProjectSubDirToProjectRootDir( const CORE::CString& projectSubDir ) const;

    CProjectInfo( void );
    CProjectInfo( const CProjectInfo& src );
    virtual ~CProjectInfo() GUCEF_VIRTUAL_OVERRIDE;

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE {GUCEF_TRACE; return this; };

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    void DeriveAbsModuleRootSubSirsFromProjRelDirs( void );

    bool GenerateDependencyChainNodes( const CORE::CString& targetPlatform     ,
                                       bool okToUseCachedValuesWhereApplicable );

    void ReduceAllPlatformsDefinitionToSustainablePlatforms( CModuleInfoEntryPtr moduleInfoEntry );

    bool CanModuleSustainAllPlatformsDefinition( const CORE::CStringSet& dependencies   ,
                                                 bool okIfNoSuchDependencyDefined       ,
                                                 TModuleInfoEntryPtrSet& problemModules ,
                                                 CORE::CStringSet& missingModules       ) const;

    bool SanitizeRuntimeDependenciesForPlatform( CModuleInfoEntryPtr moduleInfoEntry ,
                                                 const CORE::CString& targetPlatform );

    bool SanitizeModulePlatformUsage( CModuleInfoEntryPtr moduleInfoEntry ,
                                      const CORE::CString& platform       ,
                                      const TStringSet& dependencies      );

    bool SanitizeModulePlatformUsage( CModuleInfoEntryPtr moduleInfoEntry ,
                                      const CORE::CString& platform       );

    void GetBuildLeafNodes( const CORE::CString& platform          ,
                            TModuleDependencyNodePtrMap& leafNodes );

    void GetUnprocessedBuildLeafNodes( const TStringToInt32Map& buildOrderMap                      ,
                                       const TModuleDependencyNodePtrMap& nodesThatWereMissingDeps ,
                                       TModuleDependencyNodePtrMap& newLeafNodes                   );

    void GetNextLevelUpBuildNodes( const CORE::CString& platform                          ,
                                   const TModuleDependencyNodePtrMap& currentLevel        ,
                                   const TStringToInt32Map& buildOrderMap                 ,
                                   TModuleDependencyNodePtrMap& nextLevelNodes            ,
                                   TModuleDependencyNodePtrMap& nextLevelNodesMissingDeps );

    static bool GenerateModuleDependencyIncludes( CModuleInfoEntryPtr& moduleInfoEntry            ,
                                                  const CModuleInfoEntryPtr dependencyModuleEntry ,
                                                  const CORE::CString& platformName               );

    bool GenerateModuleDependencyIncludesForPlatform( CModuleInfoEntryPtr& moduleInfoEntry ,
                                                      const CORE::CString& platformName    );

    void MergeIntegrationLocationsIntoModuleForPlatform( const CORE::CString& targetPlatform       ,
                                                         const CModuleInfoPtr& moduleInfoToMergeIn ,
                                                         const CORE::CString& codeIncludeRoot      ,
                                                         const TModuleType moduleType              );

    void MergeIntegrationLocationsIntoModuleForAllPlatformsPlatform( const CModuleInfoPtr& moduleInfoToMergeIn ,
                                                                     const CORE::CString& codeIncludeRoot      );

    void MergeIntegrationLocationsIntoModuleForAllPlatformsPlatform( void );

    void MergeIntegrationLocationsIntoModuleForPlatform( const CORE::CString& targetPlatform );

    bool BulkPostProcessAllModuleInfo( bool isLoadFromProjectInfo );

    private:

    mutable TStringSet m_actualPlatformsUsed;           // Cached list of platforms actually used in the project, derived from the platforms map and the modules
    TStringToModuleDependencyNodePtrMap m_moduleDependencyChains;
    CORE::CValueList m_settings;
    CORE::CStringSet m_disabledPlatforms; // platforms which we will ignore for processing
    MT::CReadWriteLock m_rwLock;

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE {GUCEF_TRACE; return m_rwLock.Lock( lockWaitTimeoutInMs ); };
    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE {GUCEF_TRACE; return m_rwLock.Unlock(); };
    virtual MT::TLockStatus ReadOnlyLock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const {GUCEF_TRACE; return m_rwLock.ReadOnlyLock( lockWaitTimeoutInMs ); };
    virtual MT::TLockStatus ReadOnlyUnlock( void ) const {GUCEF_TRACE; return m_rwLock.ReadOnlyUnlock(); };
};

typedef CProjectInfo::TSharedPtrType    CProjectInfoPtr;

/*---------------------------------------------------------------------------*/

struct SProjectTargetInfo
{
    CORE::CString projectName;                               // Name of the overall project (bundling target)
    CModuleInfoEntryPtr mainModule;                          // Reference to the main module for the project if applicable
    TModuleInfoEntryPtrSet modules;                          // All generated module information
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
                      CProjectInfo& projectInfo           );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
const CORE::CString*
GetModuleName( const TProjectTargetInfoMap& targetPlatforms ,
               const CORE::CString& targetPlatform          ,
               CModuleInfoPtr* moduleInfo = GUCEF_NULL      );

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

GUCEF_PROJECTGEN_PUBLIC_CPP
CORE::CString
GetModuleTargetName( const CModuleInfoEntryPtr& moduleInfoEntry ,
                     const CORE::CString& targetPlatform        ,
                     bool useModuleNameIfNoTargetName           );

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

// Checks if an explicit platform definition exists for the module 
GUCEF_PROJECTGEN_PUBLIC_CPP
bool
HasPlatformDefinition( const CModuleInfoEntryPtr& moduleInfoEntry ,
                       const CORE::CString& platform              );

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

// Collects a list of all unique tag values used
GUCEF_PROJECTGEN_PUBLIC_CPP
void
GetAllTagsUsed( const CProjectInfo& projectInfo ,
                TStringSet& tagsUsed            );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
IsModuleTagged( const CModuleInfoEntryPtr& module ,
                const CORE::CString& tag          ,
                const CORE::CString& platform     );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
IsModuleTagged( const CModuleInfoEntryPtr& module    ,
                const CORE::CString::StringSet& tags ,
                const CORE::CString& platform        );

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
void
GetTaggedModules( const CProjectInfo& projectInfo       ,
                  const CORE::CString& tag              ,
                  TModuleInfoEntryPtrSet& taggedModules ,
                  const CORE::CString& platform         );

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
                       const CORE::CString& platformName  );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
IsModuleTaggedWith( const CModuleInfoEntryPtr& moduleInfo ,
                    const CORE::CString& platformName     ,
                    const CORE::CString& tag              );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
void
GetExecutables( const CProjectInfo& projectInfo           ,
                TModuleInfoEntryPtrSet& executableTargets ,
                const CORE::CString& platform             );

/*-------------------------------------------------------------------------*/

/**
 *  Splits out the projectInfo into different projects per platform
 *  Will only consider the platforms requested in "platformsUsed"
 */
GUCEF_PROJECTGEN_PUBLIC_CPP
void
SplitProjectPerTarget( const CProjectInfo& projectInfo   ,
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
SplitProjectPerTarget( const CProjectInfo& projectInfo   ,
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

/*-------------------------------------------------------------------------*/;

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
TryAutoSemVerDetection( const CORE::CString& fileContent ,
                        CORE::CVersion& detectedSemVer   );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PROJECTGEN_DATATYPES_H ? */
