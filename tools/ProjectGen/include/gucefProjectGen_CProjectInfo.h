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
 
#ifndef GUCEF_PROJECTGEN_CPROJECTINFO_H
#define GUCEF_PROJECTGEN_CPROJECTINFO_H

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

#ifndef GUCEF_PROJECTGEN_DATATYPES_H
#include "gucefProjectGen_DataTypes.h"
#define GUCEF_PROJECTGEN_DATATYPES_H
#endif /* GUCEF_PROJECTGEN_DATATYPES_H ? */

#ifndef GUCEF_PROJECTGEN_CMODULEDEPENDENCYNODE_H
#include "gucefProjectGen_CModuleDependencyNode.h"
#define GUCEF_PROJECTGEN_CMODULEDEPENDENCYNODE_H
#endif /* GUCEF_PROJECTGEN_CMODULEDEPENDENCYNODE_H ? */

#ifndef GUCEF_PROJECTGEN_CPROJECTTARGETINFO_H
#include "gucefProjectGen_CProjectTargetInfo.h"
#define GUCEF_PROJECTGEN_CPROJECTTARGETINFO_H
#endif /* GUCEF_PROJECTGEN_CPROJECTTARGETINFO_H ? */

#ifndef GUCEF_PROJECTGEN_CPROJECTTARGETINFOBUNDLE_H
#include "gucefProjectGen_CProjectTargetInfoBundle.h"
#define GUCEF_PROJECTGEN_CPROJECTTARGETINFOBUNDLE_H
#endif /* GUCEF_PROJECTGEN_CPROJECTTARGETINFOBUNDLE_H ? */

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PROJECTGEN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_PROJECTGEN_PUBLIC_CPP CProjectInfo : public CORE::CTSharedObjCreator< CProjectInfo, MT::CMutex > ,
                                                 public CORE::CIDataNodeSerializable ,
                                                 public MT::CILockable
{
    public:

    static const CORE::CString ClassTypeName;

    TStringVector rootDirs;                                  // Root dirs used to gather all project info
    TStringToModuleInfoEntryPtrMap modules;                  // All generated module information
    TDirProcessingInstructionsMap dirProcessingInstructions; // All loaded processing instructions mapped per path
    TStringVector globalDirExcludeList;                      // Dirs that should never be included in processing regardless of path

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
     *  Obtains an entire set of modules based on module names
     *  if 'includeDependenciesForTheGivenPlatform' is true the resulting set will include all platform relative dependencies
     *  as a superset based on the set of modules in the 'moduleNames' collection
     */
    bool
    GetModuleInfoEntries( const CORE::CStringSet& moduleNames             ,
                          const CORE::CString& platform                   ,
                          bool includeDependenciesForTheGivenPlatform     ,
                          TModuleInfoEntryPtrSet& foundModules            ) const;

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
     *  Provides the names of all known platforms
     *  Known platforms are either defined by config or as a fallback the default hardcoded known platforms
     *  If you want to exclude disabled platforms use GetKnownEnabledPlatforms()
     */
    void
    GetKnownPlatforms( TStringSet& platformList ) const;

    /**
     *  Provides the directory names of all known platforms
     *  Known platforms are either defined by config or as a fallback the default hardcoded known platforms
     *  Directory names include multi-platform aliases which helps signify its content belongs to multiple
     *  specific platforms per the multi-platform aliasing rules
     */
    void
    GetKnownPlatformDirs( TStringSet& platformList ) const;

    /**
     *  Provides the directory names of all known platforms
     *  Known platforms are either defined by config or as a fallback the default hardcoded known platforms
     *  Directory names include multi-platform aliases which helps signify its content belongs to multiple
     *  specific platforms per the multi-platform aliasing rules
     */
    void
    GetKnownPlatformDirsPerPlatform( CORE::CStringMapSet& platformDirs ) const;

    const TPlatformDefinitionMap& GetPlatformDefinitions( void ) const;

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

    /**
     *  Provides the names of all known platforms minus the disabled ones
     *  If you also want the disabled ones use GetKnownPlatforms()
     */
    void
    GetKnownEnabledPlatforms( TStringSet& platformList ,
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


    bool TryGetDependencyChainsForModules( TModuleDependencyNodePtrSet& dependencyChains ,
                                           const CORE::CStringSet& moduleNames           ,
                                           const CORE::CString& targetPlatform           ,
                                           bool onlyCheckPlatformSpecific                ) const;

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

    /**
     *  Obtains the delta in dependencies between two platforms for a given module
     *  This allows for denoting any platform specifics relative to a base platform
     *
     *  Note that delta directionality matters so for the typical use case of wanting to know
     *  which dependencies are added on a given platform relative to the 'all' platforms base platform
     *  you should invoke this the with 'all' platform as the base and the specific platform as the delta platform
     *
     *  Note that this is a module delta NOT a platform delta. Meaning that an 'all' platforms module could
     *  be listed as a delta vs an 'all' baseline if it happened to only be pulled in on the specific platform
     */
    bool GetModuleDependencyDeltaAcrossPlatforms( TModuleInfoEntryPtrSet& dependencyDelta  ,
                                                  const CORE::CString& moduleName          ,
                                                  const CORE::CString& basePlatform        ,
                                                  const CORE::CString& deltaPlatform       ,
                                                  bool includeDependenciesOfDependencies   ,
                                                  bool addDependencies                     ,
                                                  bool addLinkerDependencies               ,
                                                  bool addRuntimeDependencies              ) const;

    /**
     *  Obtains the delta in dependencies between two platforms for a given set of modules
     *  This allows for denoting any platform specifics relative to a base platform
     *
     *  Note that delta directionality matters so for the typical use case of wanting to know
     *  which dependencies are added on a given platform relative to the 'all' platforms base platform
     *  you should invoke this the with 'all' platform as the base and the specific platform as the delta platform
     *
     *  Note that this is a module delta NOT a platform delta. Meaning that an 'all' platforms module could
     *  be listed as a delta vs an 'all' baseline if it happened to only be pulled in on the specific platform
     *
     *  Note that if you already have the dependency trees for the base platform you can provide them here
     *  and use this variant to avoid having to look them up again for combo with the delta platforms
     */
    bool
    GetAllModuleDependenciesDeltaAcrossPlatforms( TModuleInfoEntryPtrSet& dependencyDelta      ,
                                                  const TModuleDependencyNodePtrSet& baseTrees ,
                                                  const CORE::CStringSet& moduleNames          ,
                                                  const CORE::CString& deltaPlatform           ,
                                                  bool includeDependenciesOfDependencies       ,
                                                  bool addDependencies                         ,
                                                  bool addLinkerDependencies                   ,
                                                  bool addRuntimeDependencies                  ) const;

    /**
     *  Obtains the delta in dependencies between two platforms for a given set of modules
     *  This allows for denoting any platform specifics relative to a base platform
     *
     *  Note that delta directionality matters so for the typical use case of wanting to know
     *  which dependencies are added on a given platform relative to the 'all' platforms base platform
     *  you should invoke this the with 'all' platform as the base and the specific platform as the delta platform
     *
     *  Note that this is a module delta NOT a platform delta. Meaning that an 'all' platforms module could
     *  be listed as a delta vs an 'all' baseline if it happened to only be pulled in on the specific platform
     */
    bool
    GetAllModuleDependenciesDeltaAcrossPlatforms( TModuleInfoEntryPtrSet& dependencyDelta  ,
                                                  const CORE::CStringSet& moduleNames      ,
                                                  const CORE::CString& basePlatform        ,
                                                  const CORE::CString& deltaPlatform       ,
                                                  bool includeDependenciesOfDependencies   ,
                                                  bool addDependencies                     ,
                                                  bool addLinkerDependencies               ,
                                                  bool addRuntimeDependencies              ) const;

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

    bool DeserializeProjectTargets( const CORE::CDataNode& domRootNode                  ,
                                    const CORE::CDataNodeSerializableSettings& settings );

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

    void GetAllTagsUsed( TStringSet& tagsUsed ) const;

    void
    GetTaggedModules( const CORE::CString& tag              ,
                      TModuleInfoEntryPtrSet& taggedModules ,
                      const CORE::CString& platform         ) const;

    void
    GetTaggedModulesByName( const CORE::CString& tag            ,
                            CORE::CStringSet& taggedModuleNames ,
                            const CORE::CString& platform       ) const;

    /**
     *  Obtains the subset of modules that represent executables for the given platform
     */
    void GetExecutables( TModuleInfoEntryPtrSet& executableTargets ,
                         const CORE::CString& platform             ) const;

    /**
     *  Reduced the total set of modules in the project to just the ones that can act as top level 'targets'
     *  Same as GetPlatformTargets() except that all known platforms are considered
     */
    bool GetAllTargets( CProjectTargetInfoBundle& targets     ,
                        bool tagsAsTargets                    ,
                        bool deltaFormatForSpecificPlatforms  ,
                        const TStringSet& platformsToConsider ) const;

    /**
     *  Same as other GetAllTargets() except that all used platforms are considered
     */
    bool GetAllTargets( CProjectTargetInfoBundle& targets    ,
                        bool tagsAsTargets                   ,
                        bool deltaFormatForSpecificPlatforms ) const;

    bool GenerateAllProjectTargetInfo( bool okToUseCachedData );

    const CORE::CString& GetProjectName( void ) const;

    const CORE::CValueList& GetSettings( void ) const;

    void SetSetttings( const CORE::CValueList& settings );

    void ApplyConfig( const CORE::CDataNode& loadedConfig );

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

    bool ReduceAllPlatformsDefinitionToSustainablePlatforms( CModuleInfoEntryPtr moduleInfoEntry );

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

    void SetKnownPlatformsToHardcodedDefaults( void );

    private:

    mutable TStringSet m_actualPlatformsUsed;           // Cached list of platforms actually used in the project, derived from the platforms map and the modules
    TStringToModuleDependencyNodePtrMap m_moduleDependencyChains;
    CORE::CValueList m_settings;
    TPlatformDefinitionMap m_knownPlatforms;        // known platforms for this project
    CORE::CStringSet m_disabledPlatforms;           // platforms which we will ignore for processing
    CORE::CString m_projectName;                    // Name of the overall project
    CProjectTargetInfoBundlePtr m_projectTargets; 
    MT::CReadWriteLock m_rwLock;

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE {GUCEF_TRACE; return m_rwLock.Lock( lockWaitTimeoutInMs ); };
    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE {GUCEF_TRACE; return m_rwLock.Unlock(); };
    virtual MT::TLockStatus ReadOnlyLock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE {GUCEF_TRACE; return m_rwLock.ReadOnlyLock( lockWaitTimeoutInMs ); };
    virtual MT::TLockStatus ReadOnlyUnlock( void ) const GUCEF_VIRTUAL_OVERRIDE {GUCEF_TRACE; return m_rwLock.ReadOnlyUnlock(); };
};

typedef CProjectInfo::TSharedPtrType    CProjectInfoPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PROJECTGEN_CPROJECTINFO_H ? */
