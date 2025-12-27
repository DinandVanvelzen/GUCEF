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
 
#ifndef GUCEF_PROJECTGEN_CMODULEINFO_H
#define GUCEF_PROJECTGEN_CMODULEINFO_H

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

    #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED
    CModuleInfo( CModuleInfo&& src ) GUCEF_NOEXCEPT;
    #endif 

    virtual ~CModuleInfo() GUCEF_VIRTUAL_OVERRIDE;

    private:

    CORE::CString m_platformName;
    TStringSet m_namesOfDependencies;
    TStringSet m_namesOfLogicalDependencies;

    TStringSetMap m_includeDirs;                   // include directories of this module's own headers
    TStringSetMap m_sourceDirs;                    // source directories of this module's own source

};

typedef CModuleInfo::CModuleInfoPtr CModuleInfoPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PROJECTGEN_CMODULEINFO_H ? */
