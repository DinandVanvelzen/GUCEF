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
 
#ifndef GUCEF_PROJECTGEN_CMODULEDEPENDENCYNODE_H
#define GUCEF_PROJECTGEN_CMODULEDEPENDENCYNODE_H

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

#ifndef GUCEF_PROJECTGEN_CMODULEINFOENTRY_H
#include "gucefProjectGen_CModuleInfoEntry.h"
#define GUCEF_PROJECTGEN_CMODULEINFOENTRY_H
#endif /* GUCEF_PROJECTGEN_CMODULEINFOENTRY_H ? */

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

    bool GetDependencyNames( CORE::CStringSet& dependencyNames      ,
                             bool includeDependenciesOfDependencies ,
                             bool addDependencies                   ,
                             bool addLinkerDependencies             ,
                             bool addRuntimeDependencies            ) const;

    /**
     *  Determines the delta module wise between the aggregate selected dependency sets
     * 
     *  Note that delta directionality matters so invoke this function on the appropriate chain to use as base
     *  relative to the other platform (the 'otherTree')
     */
    bool GetDependencyDelta( TModuleInfoEntryPtrSet& dependencyDelta  ,
                             const CModuleDependencyNodePtr otherTree ,
                             bool includeDependenciesOfDependencies   ,
                             bool addDependencies                     ,
                             bool addLinkerDependencies               ,
                             bool addRuntimeDependencies              ) const;


    /**
     *  Determines the delta module wise between the aggregate selected dependency sets
     * 
     *  Note that delta directionality matters and thus be sure to pass the correct set for 'base' vs 'other' for your use-case
     */
    static bool GetSetDependencyDelta( TModuleInfoEntryPtrSet& dependencyDelta      ,
                                       const TModuleDependencyNodePtrSet& baseTree  ,
                                       const TModuleDependencyNodePtrSet& otherTree ,
                                       bool includeDependenciesOfDependencies       ,
                                       bool addDependencies                         ,
                                       bool addLinkerDependencies                   ,
                                       bool addRuntimeDependencies                  );

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
typedef GUCEF::map< CORE::CString, TModuleDependencyNodePtrMap >    TStringToModuleDependencyNodePtrMap;


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PROJECTGEN_CMODULEDEPENDENCYNODE_H ? */
