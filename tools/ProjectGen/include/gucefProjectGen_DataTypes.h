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

typedef CORE::CString::StringSet TStringSet;
typedef std::set< CORE::Int32 > TInt32Set;
typedef std::map< CORE::CString, Int32 >    TStringToInt32Map;
typedef CORE::CString::StringMapSet TStringSetMap;
typedef CORE::CString::StringVector TStringVector;
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
typedef std::map< CORE::CString, CLinkedLibrarySettingsPtr > TLinkedLibrarySettingsPtrMap;

/*---------------------------------------------------------------------------*/

/**
 *  Class where all linker related information should be stored
 */
class GUCEF_PROJECTGEN_PUBLIC_CPP CLinkerSettings : public CORE::CTSharedObjCreator< CLinkerSettings, MT::CMutex >
{
    public:

    typedef typename CORE::CTSharedObjCreator< CLinkerSettings, MT::CMutex >::TBasicSharedPtrType  CLinkerSettingsPtr;

    bool Merge( const CLinkerSettings& linkerSettingsToMergeIn ,
                bool onConflictOriginalInfoStays = true        );

    void Clear( void );

    const TLinkedLibrarySettingsPtrMap& GetLinkedLibraries( void ) const;

    bool HasLinkerDependency( const CORE::CString& dependencyName ) const;

    void GetListOfLinkedLibraries( CORE::CStringSet& linkedLibraries ) const;

    bool TryGetLinkedLibrary( const CORE::CString& libraryName         ,                                     
                              CLinkedLibrarySettingsPtr& linkedLibrary ,
                              bool createDefaultIfNotExist             );

    bool TryGetLinkedLibrary( const CORE::CString& libraryName         ,
                              CLinkedLibrarySettingsPtr& linkedLibrary ) const;

    void DeleteLinkedLibrary( const CORE::CString& libraryName );

    const TStringSet& GetLibraryPaths( void ) const;

    bool MergeLibraryPaths( const TStringSet& libPathsToMergeIn );

    void SetTargetName( const CORE::CString& targetName );

    const CORE::CString& GetTargetName( void ) const;

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

    TLinkedLibrarySettingsPtrMap m_linkedLibraries;    // list of all libraries the module links against
    TStringSet m_libPaths;                             // list of hint paths where to look for libraries
    CORE::CString m_targetName;                        // optional name for the linker target if desired from the module name
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
    
    CORE::CString  lastEditBy;             // optional info listing who last updated the information
    CORE::CString::StringSet authors;
    CORE::CString::StringSet maintainers;
    CORE::CVersion semver;
    CORE::CString descriptionHeadline;
    CORE::CString descriptionDetails;
    CORE::CString license;

    CModuleMetaData( void );

    CModuleMetaData( const CModuleMetaData& src );

    virtual ~CModuleMetaData() GUCEF_VIRTUAL_OVERRIDE;

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

    int buildOrder;                              // order number of this module in the build dependency chain
    int buildChain;                              // index of the build chain, different build chains can be build independently but may depend on other chains
    TInt32Set buildChainDependencies;            // other build chains this build chain is dependent on, if any
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

    void MergeNamesOfDependencies( const TStringSet& dependenciesToMergeIn );

    void AddNameOfRuntimeDependency( const CORE::CString& dependency );

    void RemoveNameOfRuntimeDependency( const CORE::CString& dependency );

    /**
     *  Independent modules have relevance in their own right.
     *  Non-independent modules are logical constructs to aid with project organization
     */
    bool HasIndependentModuleType( void ) const;

    /**
     *  Returns whether a proper module type was set on the module
     */
    bool HasValidModuleType( void ) const;

    void Clear( void );

    bool Merge( CModuleInfoPtr moduleInfoToMergeIn      ,
                bool onConflictOriginalInfoStays = true );
    
    CModuleInfo( void );
    CModuleInfo( const CModuleInfo& src );
    virtual ~CModuleInfo() GUCEF_VIRTUAL_OVERRIDE;

    private:

    CORE::CString m_platformName;
    TStringSet m_namesOfDependencies;
    TStringSetMap m_includeDirs;                   // include directories of this module's own headers
    TStringSetMap m_sourceDirs;                    // source directories of this module's own source

};

typedef CModuleInfo::CModuleInfoPtr CModuleInfoPtr;

/*---------------------------------------------------------------------------*/

typedef std::vector< CModuleInfoPtr > TModuleInfoPtrVector;
typedef std::map< CORE::CString, TModuleInfoPtrVector > TModuleInfoPtrVectorMap;
typedef std::map< CORE::CString, CModuleInfoPtr > TModuleInfoPtrMap;

/*---------------------------------------------------------------------------*/

class GUCEF_PROJECTGEN_PUBLIC_CPP CModuleInfoEntry : public CORE::CIDataNodeSerializable ,
                                                     public CORE::CTSharedObjCreator< CModuleInfoEntry, MT::CMutex >
{
    public:

    typedef typename CORE::CTSharedObjCreator< CModuleInfoEntry, MT::CMutex >::TBasicSharedPtrType   CModuleInfoEntryPtr;

    static const CORE::CString ClassTypeName;
    
    CORE::CString  rootDir;                // the absolute path to the root of this module's directory tree
    CModuleMetaData metadata;              // MetaData relating to the module

    CModuleInfoEntry( void );

    CModuleInfoEntry( const CModuleInfoEntry& src );

    virtual ~CModuleInfoEntry() GUCEF_VIRTUAL_OVERRIDE;

    void Clear( void );

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

    const CModuleInfoPtr FindModuleInfoForPlatform( const CORE::CString& platform ) const;

    CModuleInfoPtr FindModuleInfoForPlatform( const CORE::CString& platform, bool createNewIfNoneExists );

    const TModuleInfoPtrMap& GetModulesPerPlatform( void ) const;

    const TModuleInfoPtrMap& GetFlattenedModulesPerPlatform( void ) const;

    bool HasDependency( const CORE::CString& platform       ,
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
     *  If a module is written as generic code, not hard dependent on a given platform(s)
     *  it should have a valid AllPlatforms definition with potentially platform definitions
     *  as extensions to said definition for a given platform.
     */
    bool HasAllPlatformsDefinition( void ) const;

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

    CModuleInfoEntry& operator=( const CModuleInfoEntry& src );

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

    mutable CORE::CString m_consensusName;          // Derived cached info: the consensus name across all platforms
    TModuleInfoPtrMap m_modulesPerPlatform;         // ModuleInfo per platform
    TModuleInfoPtrMap m_flattenedInfoPerPlatform;   // Derived cached info: Module overlays per platform applied to the 'all platforms' module info
    bool m_isBroken;                                // flag whether the module has a broken definition in need of intervention
};

typedef CModuleInfoEntry::CModuleInfoEntryPtr CModuleInfoEntryPtr;

/*---------------------------------------------------------------------------*/

typedef std::vector< CModuleInfoEntryPtr >                           TModuleInfoEntryPtrVector;
typedef CORE::CTSharedPtr< TModuleInfoEntryPtrVector, MT::CMutex >   TModuleInfoEntryPtrVectorPtr;
typedef std::pair< const CModuleInfoEntryPtr, const CModuleInfoPtr > TModuleInfoEntryPair;
typedef std::pair< CModuleInfoEntryPtr, CModuleInfoPtr >             TMutableModuleInfoEntryPair;
typedef std::vector< TModuleInfoEntryPair >                          TModuleInfoEntryPairVector;
typedef std::vector< TMutableModuleInfoEntryPair >                   TMutableModuleInfoEntryPairVector;
typedef std::set< CModuleInfoEntryPtr >                              TModuleInfoEntryPtrSet;
typedef std::map< int, CModuleInfoEntryPtr >                         TModuleInfoEntryPrioMap;

/*---------------------------------------------------------------------------*/

class GUCEF_PROJECTGEN_PUBLIC_CPP CModuleDependencyNode : public CORE::CTSharedObjCreator< CModuleDependencyNode, MT::CMutex >
{
    public:

    typedef typename CORE::CTSharedObjCreator< CModuleDependencyNode, MT::CMutex >::TBasicSharedPtrType    CModuleDependencyNodePtr;
    typedef std::map< CORE::CString, CModuleDependencyNodePtr >                                            TModuleDependencyNodePtrMap;
    typedef std::set< CModuleDependencyNodePtr >                                                           TModuleDependencyNodePtrSet;

    void SetModule( const CModuleInfoEntryPtr& module );

    CModuleInfoEntryPtr GetModule( void ) const;

    const CORE::CString& GetConsensusName( void ) const;

    bool SetDependency( CModuleDependencyNodePtr dependency );

    void SetDependencies( const TModuleDependencyNodePtrMap& dependencies );

    const TModuleDependencyNodePtrMap& GetDependencies( void ) const;

    bool GatherDependencyModules( TModuleInfoEntryPtrSet& dependencies    ,
                                  bool includeDependenciesOfDependencies  ) const;

    bool SetLinkerDependency( CModuleDependencyNodePtr dependency );

    void SetLinkerDependencies( const TModuleDependencyNodePtrMap& dependencies );

    const TModuleDependencyNodePtrMap& GetLinkerDependencies( void ) const;

    bool GatherLinkerDependencyModules( TModuleInfoEntryPtrSet& dependencies    ,
                                        bool includeDependenciesOfDependencies  ) const;

    bool SetRuntimeDependency( CModuleDependencyNodePtr dependency );

    void SetRuntimeDependencies( const TModuleDependencyNodePtrMap& dependencies );

    const TModuleDependencyNodePtrMap& GetRuntimeDependencies( void ) const;

    bool GatherRuntimeDependencyModules( TModuleInfoEntryPtrSet& dependencies    ,
                                         bool includeDependenciesOfDependencies  ) const;

    bool SetDependent( CModuleDependencyNodePtr dependent );

    void SetDependents( const TModuleDependencyNodePtrMap& dependents );

    const TModuleDependencyNodePtrMap& GetDependents( void ) const;

    bool SetLinkerDependent( CModuleDependencyNodePtr dependent );

    void SetLinkerDependents( const TModuleDependencyNodePtrMap& dependents );

    const TModuleDependencyNodePtrMap& GetLinkerDependents( void ) const;

    bool SetRuntimeDependent( CModuleDependencyNodePtr dependent );

    void SetRuntimeDependents( const TModuleDependencyNodePtrMap& dependents );

    const TModuleDependencyNodePtrMap& GetRuntimeDependents( void ) const;

    void SetTargetPlatform( const CORE::CString& targetPlatform );

    const CORE::CString& GetTargetPlatform( void ) const;

    void Clear( void );

    bool HasMissingDependencies( void ) const;

    void SetHasMissingDependencies( bool hasMissing );

    CModuleDependencyNode( void );

    virtual ~CModuleDependencyNode() GUCEF_VIRTUAL_OVERRIDE;

    private:

    CModuleInfoEntryPtr m_module;
    TModuleDependencyNodePtrMap m_dependencies;
    TModuleDependencyNodePtrMap m_linkerDependencies;
    TModuleDependencyNodePtrMap m_runtimeDependencies;
    TModuleDependencyNodePtrMap m_dependents;
    TModuleDependencyNodePtrMap m_linkerDependents;
    TModuleDependencyNodePtrMap m_runtimeDependents;
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
                                                 public MT::CILockable
{
    public:

    CORE::CString projectName;                               // Name of the overall project
    TStringVector rootDirs;                                  // Root dirs used to gather all project info
    TModuleInfoEntryPtrVector modules;                       // All generated module information
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
     */
    void
    GetAllPlatformsUsed( TStringSet& platformList       ,
                         bool okToUseCachedValue = true ) const;

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

    bool TryGetModuleDependencyChain( CModuleDependencyNodePtr& dependencyChain ,
                                      const CORE::CString& consensusModuleName  ,
                                      const CORE::CString& targetPlatform       ,
                                      bool onlyCheckPlatformSpecific            ) const;

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
    void SanitizeAllPlatformsUsage( void );

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

    void SanitizeRuntimeDependencies( CModuleInfoEntryPtr moduleInfoEntry );

    void SanitizeRuntimeDependencies( void );

    void DetermineBuildOrderForAllModules( void );

    void DetermineBuildOrderForAllModulesForPlatform( const CORE::CString& targetPlatform );

    bool GetModuleDependencies( const CModuleInfoEntryPtr& moduleInfoEntry ,
                                const CORE::CString& targetPlatform        ,
                                TModuleInfoEntryPtrSet& dependencies       ,
                                bool includeDependenciesOfDependencies     ,
                                bool includeRuntimeDependencies            ) const;

    void GetModuleDependencies( const CModuleInfoEntryPtr& moduleInfoEntry ,
                                const CORE::CString& targetPlatform        ,
                                TStringSet& dependencies                   ,
                                bool includeRuntimeDependencies            ) const;

    void Clear( void );
    
    CProjectInfo( void );
    CProjectInfo( const CProjectInfo& src );
    virtual ~CProjectInfo() GUCEF_VIRTUAL_OVERRIDE;

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE {GUCEF_TRACE; return this; };

    private:

    bool GenerateDependencyChainNodes( const CORE::CString& targetPlatform     ,
                                       bool okToUseCachedValuesWhereApplicable );

    void ReduceAllPlatformsDefinitionToSustainablePlatforms( CModuleInfoEntryPtr moduleInfoEntry );

    bool CanModuleSustainAllPlatformsDefinition( const CORE::CStringSet& dependencies   ,
                                                 bool okIfNoSuchDependencyDefined       ,
                                                 TModuleInfoEntryPtrSet& problemModules ,
                                                 CORE::CStringSet& missingModules       ) const;

    void SanitizeRuntimeDependenciesForPlatform( CModuleInfoEntryPtr moduleInfoEntry ,
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

    private:

    mutable TStringSet m_actualPlatformsUsed; // Cached list of platforms actually used in the project, derived from the platforms map and the modules
    TStringToModuleDependencyNodePtrMap m_moduleDependencyChains;
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
MergeModuleInfoEntries( const TModuleInfoEntryPtrVector& moduleInfoEntriesToMergeIn ,
                        TModuleInfoEntryPtrVector& moduleInfoEntries                );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
MergeAllModuleInfoForPlatform( const TModuleInfoEntryPtrVector& allInfo ,
                               const CORE::CString& platform            ,
                               TModuleInfoPtrVector& allMergedInfo      ,
                               TModuleInfoEntryPairVector& mergeLinks   );


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
void
CleanupIncludeDirs( CModuleInfoEntryPtr& moduleInfoEntry );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
SerializeModuleInfo( const CModuleInfoEntryPtr& moduleInfo ,
                     const CORE::CString& outputFilepath   );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
DeserializeModuleInfo( const CProjectInfo& projectInfo ,  
                       CModuleInfoEntryPtr& moduleInfo ,
                       const CORE::CDataNode& rootNode );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
DeserializeModuleInfo( const CProjectInfo& projectInfo              ,
                       TModuleInfoEntryPtrVector& moduleInfoEntries ,
                       const CORE::CString& inputFilepath           );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
SerializeProjectInfo( const CProjectInfo& projectInfo ,
                      CORE::CDataNode& rootNodeToBe   );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
DeserializeProjectInfo( CProjectInfo& projectInfo       ,
                        const CORE::CDataNode& rootNode );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
SerializeProjectInfo( const CProjectInfo& projectInfo     ,
                      const CORE::CString& outputFilepath );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
bool
DeserializeProjectInfo( CProjectInfo& projectInfo            ,
                        const CORE::CString& projectInfoPath );

/*-------------------------------------------------------------------------*/

GUCEF_PROJECTGEN_PUBLIC_CPP
TModuleType
GetModuleType( const CModuleInfoEntryPtr& moduleInfoEntry ,
               const CORE::CString& targetPlatform        );

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
IsAnyLicenseDefined( const TModuleInfoEntryPtrVector& moduleInfoEntries );

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
IsAnySemVerDefined( const TModuleInfoEntryPtrVector& moduleInfoEntries );

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
