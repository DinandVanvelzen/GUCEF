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
 
#ifndef GUCEF_PROJECTGEN_CMODULEINFOENTRY_H
#define GUCEF_PROJECTGEN_CMODULEINFOENTRY_H

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

#ifndef GUCEF_PROJECTGEN_CMODULEINFO_H
#include "gucefProjectGen_CModuleInfo.h"
#define GUCEF_PROJECTGEN_CMODULEINFO_H
#endif /* GUCEF_PROJECTGEN_CMODULEINFO_H ? */

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

    bool
    HasTag( const CORE::CString& tag      ,
            const CORE::CString& platform ) const;

    bool
    HasTag( const CORE::CString::StringSet& tags ,
            const CORE::CString& platform        ) const;

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

    CORE::CString GetModuleTargetName( const CORE::CString& targetPlatform ,
                                       bool useModuleNameIfNoTargetName    ) const;

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PROJECTGEN_CMODULEINFOENTRY_H ? */
