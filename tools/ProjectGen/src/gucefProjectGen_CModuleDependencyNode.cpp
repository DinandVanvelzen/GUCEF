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

#include "gucefProjectGen_CModuleDependencyNode.h"

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


/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CModuleDependencyNode::CModuleDependencyNode( void )
    : CORE::CTSharedObjCreator< CModuleDependencyNode, MT::CMutex >( this )
    , m_module()
    , m_dependencies()
    , m_linkerDependencies()
    , m_runtimeDependencies()
    , m_logicalDependencies()
    , m_dependents()
    , m_linkerDependents()
    , m_runtimeDependents()
    , m_logicalDependents()
    , m_targetPlatform()
    , m_hasMissingDependencies( false )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CModuleDependencyNode::~CModuleDependencyNode()
{GUCEF_TRACE;

    // By the time we get here clear should have been called already or you might get
    // memory leaks due to shared ptr chains
    Clear();
}

/*---------------------------------------------------------------------------*/

void
CModuleDependencyNode::Clear( void )
{GUCEF_TRACE;

    m_module.Unlink();
    m_dependencies.clear();
    m_linkerDependencies.clear();
    m_runtimeDependencies.clear();
    m_dependents.clear();
    m_linkerDependents.clear();
    m_runtimeDependents.clear();
    m_logicalDependents.clear();
    m_logicalDependencies.clear();
    m_targetPlatform.Clear();
    m_hasMissingDependencies = false;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::HasMissingDependencies( void ) const
{GUCEF_TRACE;

    return m_hasMissingDependencies;
}

/*---------------------------------------------------------------------------*/

void
CModuleDependencyNode::SetHasMissingDependencies( bool hasMissing )
{GUCEF_TRACE;

    m_hasMissingDependencies = hasMissing;
}

/*---------------------------------------------------------------------------*/

void
CModuleDependencyNode::SetTargetPlatform( const CORE::CString& targetPlatform )
{GUCEF_TRACE;

    m_targetPlatform = targetPlatform;
}

/*---------------------------------------------------------------------------*/

const CORE::CString&
CModuleDependencyNode::GetTargetPlatform( void ) const
{GUCEF_TRACE;

    return m_targetPlatform;
}

/*---------------------------------------------------------------------------*/

void
CModuleDependencyNode::SetModule( const CModuleInfoEntryPtr& module )
{GUCEF_TRACE;

    m_module = module;
}

/*---------------------------------------------------------------------------*/

CModuleInfoEntryPtr
CModuleDependencyNode::GetModule( void ) const
{GUCEF_TRACE;

    return m_module;
}

/*---------------------------------------------------------------------------*/

Int64
CModuleDependencyNode::GetBuildOrder( void ) const
{GUCEF_TRACE;

    if ( !m_module.IsNULL() )
    {
        return m_module->GetBuildOrder( m_targetPlatform, true );
    }
    return -1;
}

/*---------------------------------------------------------------------------*/

const CORE::CString&
CModuleDependencyNode::GetConsensusName( void ) const
{GUCEF_TRACE;

    if ( !m_module.IsNULL() )
    {
        return m_module->GetConsensusName();
    }
    return CORE::CString::Empty;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::SetDependency( CModuleDependencyNodePtr dependency )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( !dependency.IsNULL() )
    {
        CModuleInfoEntryPtr dependencyModule = dependency->GetModule();
        if GUCEF_PREDICT_TRUE( !dependencyModule.IsNULL() )
        {
            const CORE::CString& consensusName = dependencyModule->GetConsensusName();
            if GUCEF_PREDICT_TRUE( !consensusName.IsNULLOrEmpty() )
            {
                m_dependencies[ consensusName ] = dependency;
                return true;
            }
        }
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void 
CModuleDependencyNode::SetDependencies( const TModuleDependencyNodePtrMap& dependencies )
{GUCEF_TRACE;

    m_dependencies = dependencies;
}

/*---------------------------------------------------------------------------*/

const TModuleDependencyNodePtrMap&
CModuleDependencyNode::GetDependencies( void ) const
{GUCEF_TRACE;

    return m_dependencies;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GatherDependencyModules( TModuleInfoEntryPtrSet& dependencies          ,
                                                bool includeDependenciesOfDependencies        ,
                                                bool includeLogicalDependenciesOfDependencies ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    TModuleDependencyNodePtrMap::const_iterator i = m_dependencies.begin();
    while ( i != m_dependencies.end() )
    {
        const CModuleDependencyNodePtr& dependencyNode = (*i).second;
        if GUCEF_PREDICT_TRUE( !dependencyNode.IsNULL() )
        {
            dependencies.insert( dependencyNode->GetModule() );
            if ( includeDependenciesOfDependencies )
            {
                totalSuccess = dependencyNode->GatherDependenciesOfDependencies( dependencies, includeLogicalDependenciesOfDependencies ) && totalSuccess;
            }
        }
        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::SetLinkerDependency( CModuleDependencyNodePtr dependency )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( !dependency.IsNULL() )
    {
        CModuleInfoEntryPtr dependencyModule = dependency->GetModule();
        if GUCEF_PREDICT_TRUE( !dependencyModule.IsNULL() )
        {
            const CORE::CString& consensusName = dependencyModule->GetConsensusName();
            if GUCEF_PREDICT_TRUE( !consensusName.IsNULLOrEmpty() )
            {
                m_linkerDependencies[ consensusName ] = dependency;
                return true;
            }
        }
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void 
CModuleDependencyNode::SetLinkerDependencies( const TModuleDependencyNodePtrMap& dependencies )
{GUCEF_TRACE;

    m_linkerDependencies = dependencies;
}

/*---------------------------------------------------------------------------*/

const TModuleDependencyNodePtrMap&
CModuleDependencyNode::GetLinkerDependencies( void ) const
{GUCEF_TRACE;

    return m_linkerDependencies;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GatherLinkerDependencyModules( TModuleInfoEntryPtrSet& dependencies          ,
                                                      bool includeDependenciesOfDependencies        ,
                                                      bool includeLogicalDependenciesOfDependencies ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    TModuleDependencyNodePtrMap::const_iterator i = m_linkerDependencies.begin();
    while ( i != m_linkerDependencies.end() )
    {
        const CModuleDependencyNodePtr& dependencyNode = (*i).second;
        if GUCEF_PREDICT_TRUE( !dependencyNode.IsNULL() )
        {
            dependencies.insert( dependencyNode->GetModule() );
            if ( includeDependenciesOfDependencies )
            {
                totalSuccess = dependencyNode->GatherDependenciesOfDependencies( dependencies, includeLogicalDependenciesOfDependencies ) && totalSuccess;
            }
        }
        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::SetRuntimeDependency( CModuleDependencyNodePtr dependency )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( !dependency.IsNULL() )
    {
        CModuleInfoEntryPtr dependencyModule = dependency->GetModule();
        if GUCEF_PREDICT_TRUE( !dependencyModule.IsNULL() )
        {
            const CORE::CString& consensusName = dependencyModule->GetConsensusName();
            if GUCEF_PREDICT_TRUE( !consensusName.IsNULLOrEmpty() )
            {
                m_runtimeDependencies[ consensusName ] = dependency;
                return true;
            }
        }
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void 
CModuleDependencyNode::SetRuntimeDependencies( const TModuleDependencyNodePtrMap& dependencies )
{GUCEF_TRACE;

    m_runtimeDependencies = dependencies;
}

/*---------------------------------------------------------------------------*/

const TModuleDependencyNodePtrMap&
CModuleDependencyNode::GetRuntimeDependencies( void ) const
{GUCEF_TRACE;

    return m_runtimeDependencies;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GatherRuntimeDependencyModules( TModuleInfoEntryPtrSet& dependencies          ,
                                                       bool includeDependenciesOfDependencies        ,
                                                       bool includeLogicalDependenciesOfDependencies ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    TModuleDependencyNodePtrMap::const_iterator i = m_runtimeDependencies.begin();
    while ( i != m_runtimeDependencies.end() )
    {
        const CModuleDependencyNodePtr& dependencyNode = (*i).second;
        if GUCEF_PREDICT_TRUE( !dependencyNode.IsNULL() )
        {
            dependencies.insert( dependencyNode->GetModule() );
            if ( includeDependenciesOfDependencies )
            {
                totalSuccess = dependencyNode->GatherDependenciesOfDependencies( dependencies, includeLogicalDependenciesOfDependencies ) && totalSuccess;
            }
        }
        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GatherDependenciesOfDependencies( TModuleInfoEntryPtrSet& dependencies ,
                                                         bool includeLogicalDependencies      ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    totalSuccess = GatherDependenciesOfDependenciesImpl( m_dependencies, dependencies, includeLogicalDependencies ) && totalSuccess;
    totalSuccess = GatherDependenciesOfDependenciesImpl( m_linkerDependencies, dependencies, includeLogicalDependencies ) && totalSuccess;
    totalSuccess = GatherDependenciesOfDependenciesImpl( m_runtimeDependencies, dependencies, includeLogicalDependencies ) && totalSuccess;
    if ( includeLogicalDependencies )
        totalSuccess = GatherDependenciesOfDependenciesImpl( m_logicalDependencies, dependencies, includeLogicalDependencies ) && totalSuccess;
    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GatherDependenciesOfDependenciesImpl( const TModuleDependencyNodePtrMap& dependenciesAtLevel ,
                                                             TModuleInfoEntryPtrSet& dependencies                   ,
                                                             bool includeLogicalDependencies                        ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    TModuleDependencyNodePtrMap::const_iterator i = dependenciesAtLevel.begin();
    while ( i != dependenciesAtLevel.end() )
    {
        const CModuleDependencyNodePtr& dependencyNode = (*i).second;
        if GUCEF_PREDICT_TRUE( !dependencyNode.IsNULL() )
        {
            dependencies.insert( dependencyNode->GetModule() );
            totalSuccess = dependencyNode->GatherDependenciesOfDependencies( dependencies, includeLogicalDependencies ) && totalSuccess;
        }
        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::SetLogicalDependency( CModuleDependencyNodePtr dependency )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( !dependency.IsNULL() )
    {
        CModuleInfoEntryPtr dependencyModule = dependency->GetModule();
        if GUCEF_PREDICT_TRUE( !dependencyModule.IsNULL() )
        {
            const CORE::CString& consensusName = dependencyModule->GetConsensusName();
            if GUCEF_PREDICT_TRUE( !consensusName.IsNULLOrEmpty() )
            {
                m_logicalDependencies[ consensusName ] = dependency;
                return true;
            }
        }
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void 
CModuleDependencyNode::SetLogicalDependencies( const TModuleDependencyNodePtrMap& dependencies )
{GUCEF_TRACE;

    m_logicalDependencies = dependencies;
}

/*---------------------------------------------------------------------------*/

const TModuleDependencyNodePtrMap&
CModuleDependencyNode::GetLogicalDependencies( void ) const
{GUCEF_TRACE;

    return m_logicalDependencies;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GatherLogicalDependencyModules( TModuleInfoEntryPtrSet& dependencies   ,
                                                       bool includeDependenciesOfDependencies ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    TModuleDependencyNodePtrMap::const_iterator i = m_logicalDependencies.begin();
    while ( i != m_logicalDependencies.end() )
    {
        const CModuleDependencyNodePtr& dependencyNode = (*i).second;
        if GUCEF_PREDICT_TRUE( !dependencyNode.IsNULL() )
        {
            dependencies.insert( dependencyNode->GetModule() );
            if ( includeDependenciesOfDependencies )
            {
                totalSuccess = dependencyNode->GatherLogicalDependencyModules( dependencies, includeDependenciesOfDependencies ) && totalSuccess;
            }
        }
        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::SetDependent( CModuleDependencyNodePtr dependent )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( !dependent.IsNULL() )
    {
        CModuleInfoEntryPtr dependentModule = dependent->GetModule();
        if GUCEF_PREDICT_TRUE( !dependentModule.IsNULL() )
        {
            const CORE::CString& consensusName = dependentModule->GetConsensusName();
            if GUCEF_PREDICT_TRUE( !consensusName.IsNULLOrEmpty() )
            {
                m_dependents[ consensusName ] = dependent;
                return true;
            }
        }
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void 
CModuleDependencyNode::SetDependents( const TModuleDependencyNodePtrMap& dependents )
{GUCEF_TRACE;

    m_dependents = dependents;
}

/*---------------------------------------------------------------------------*/

const TModuleDependencyNodePtrMap&
CModuleDependencyNode::GetDependents( void ) const
{GUCEF_TRACE;

    return m_dependents;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GatherDependentModules( TModuleInfoEntryPtrSet& dependents ,
                                               bool includeDependentsOfDependents ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    TModuleDependencyNodePtrMap::const_iterator i = m_dependents.begin();
    while ( i != m_dependents.end() )
    {
        const CModuleDependencyNodePtr& dependentNode = (*i).second;
        if GUCEF_PREDICT_TRUE( !dependentNode.IsNULL() )
        {
            dependents.insert( dependentNode->GetModule() );
            if ( includeDependentsOfDependents )
            {
                totalSuccess = dependentNode->GatherDependentModules( dependents, includeDependentsOfDependents ) && totalSuccess;
            }
        }
        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::SetLinkerDependent( CModuleDependencyNodePtr dependent )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( !dependent.IsNULL() )
    {
        CModuleInfoEntryPtr dependentModule = dependent->GetModule();
        if GUCEF_PREDICT_TRUE( !dependentModule.IsNULL() )
        {
            const CORE::CString& consensusName = dependentModule->GetConsensusName();
            if GUCEF_PREDICT_TRUE( !consensusName.IsNULLOrEmpty() )
            {
                m_linkerDependents[ consensusName ] = dependent;
                return true;
            }
        }
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void 
CModuleDependencyNode::SetLinkerDependents( const TModuleDependencyNodePtrMap& dependents )
{GUCEF_TRACE;

    m_linkerDependents = dependents;
}

/*---------------------------------------------------------------------------*/

const TModuleDependencyNodePtrMap&
CModuleDependencyNode::GetLinkerDependents( void ) const
{GUCEF_TRACE;

    return m_linkerDependents;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GatherLinkerDependentModules( TModuleInfoEntryPtrSet& dependents ,
                                                     bool includeDependentsOfDependents ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    TModuleDependencyNodePtrMap::const_iterator i = m_linkerDependents.begin();
    while ( i != m_linkerDependents.end() )
    {
        const CModuleDependencyNodePtr& dependentNode = (*i).second;
        if GUCEF_PREDICT_TRUE( !dependentNode.IsNULL() )
        {
            dependents.insert( dependentNode->GetModule() );
            if ( includeDependentsOfDependents )
            {
                totalSuccess = dependentNode->GatherLinkerDependentModules( dependents, includeDependentsOfDependents ) && totalSuccess;
            }
        }
        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::SetRuntimeDependent( CModuleDependencyNodePtr dependent )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( !dependent.IsNULL() )
    {
        CModuleInfoEntryPtr dependentModule = dependent->GetModule();
        if GUCEF_PREDICT_TRUE( !dependentModule.IsNULL() )
        {
            const CORE::CString& consensusName = dependentModule->GetConsensusName();
            if GUCEF_PREDICT_TRUE( !consensusName.IsNULLOrEmpty() )
            {
                m_runtimeDependents[ consensusName ] = dependent;
                return true;
            }
        }
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void 
CModuleDependencyNode::SetRuntimeDependents( const TModuleDependencyNodePtrMap& dependents )
{GUCEF_TRACE;

    m_runtimeDependents = dependents;
}

/*---------------------------------------------------------------------------*/

const TModuleDependencyNodePtrMap&
CModuleDependencyNode::GetRuntimeDependents( void ) const
{GUCEF_TRACE;

    return m_runtimeDependents;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GatherRuntimeDependentModules( TModuleInfoEntryPtrSet& dependents ,
                                                      bool includeDependentsOfDependents ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    TModuleDependencyNodePtrMap::const_iterator i = m_runtimeDependents.begin();
    while ( i != m_runtimeDependents.end() )
    {
        const CModuleDependencyNodePtr& dependentNode = (*i).second;
        if GUCEF_PREDICT_TRUE( !dependentNode.IsNULL() )
        {
            dependents.insert( dependentNode->GetModule() );
            if ( includeDependentsOfDependents )
            {
                totalSuccess = dependentNode->GatherRuntimeDependentModules( dependents, includeDependentsOfDependents ) && totalSuccess;
            }
        }
        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::SetLogicalDependent( CModuleDependencyNodePtr dependent )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( !dependent.IsNULL() )
    {
        CModuleInfoEntryPtr dependentModule = dependent->GetModule();
        if GUCEF_PREDICT_TRUE( !dependentModule.IsNULL() )
        {
            const CORE::CString& consensusName = dependentModule->GetConsensusName();
            if GUCEF_PREDICT_TRUE( !consensusName.IsNULLOrEmpty() )
            {
                m_logicalDependents[ consensusName ] = dependent;
                return true;
            }
        }
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void 
CModuleDependencyNode::SetLogicalDependents( const TModuleDependencyNodePtrMap& dependents )
{GUCEF_TRACE;

    m_logicalDependents = dependents;
}

/*---------------------------------------------------------------------------*/

const TModuleDependencyNodePtrMap&
CModuleDependencyNode::GetLogicalDependents( void ) const
{GUCEF_TRACE;

    return m_logicalDependents;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GatherLogicallyDependentModules( TModuleInfoEntryPtrSet& dependents ,
                                                        bool includeDependentsOfDependents ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    TModuleDependencyNodePtrMap::const_iterator i = m_logicalDependents.begin();
    while ( i != m_logicalDependents.end() )
    {
        const CModuleDependencyNodePtr& dependentNode = (*i).second;
        if GUCEF_PREDICT_TRUE( !dependentNode.IsNULL() )
        {
            dependents.insert( dependentNode->GetModule() );
            if ( includeDependentsOfDependents )
            {
                totalSuccess = dependentNode->GatherLogicallyDependentModules( dependents, includeDependentsOfDependents ) && totalSuccess;
            }
        }
        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GetDependencyModulesMappedByBuildOrder( TModuleInfoEntryPrioMap& modulesMappedByBuildOrder ,
                                                               bool includeDependenciesOfDependencies             ,
                                                               bool addDependencies                               ,
                                                               bool addLinkerDependencies                         ,
                                                               bool addRuntimeDependencies                        ) const
{GUCEF_TRACE;

    TModuleInfoEntryPtrSet allDependencies;
    if ( addDependencies )
        GatherDependencyModules( allDependencies, includeDependenciesOfDependencies );
    if ( addLinkerDependencies )
        GatherLinkerDependencyModules( allDependencies, includeDependenciesOfDependencies );
    if ( addRuntimeDependencies )
        GatherRuntimeDependencyModules( allDependencies, includeDependenciesOfDependencies );

    TModuleInfoEntryPtrSet::iterator i = allDependencies.begin();
    while ( i != allDependencies.end() )
    {
        const CModuleInfoEntryPtr& module = (*i);
        modulesMappedByBuildOrder[ module->GetBuildOrder( m_targetPlatform ) ] = module;
        ++i;
    }
    return true;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GetDependencyNames( CORE::CStringSet& dependencyNames      ,
                                           bool includeDependenciesOfDependencies ,
                                           bool addDependencies                   ,
                                           bool addLinkerDependencies             ,
                                           bool addRuntimeDependencies            ) const
{GUCEF_TRACE;

    TModuleInfoEntryPtrSet allDependencies;
    if ( addDependencies )
        GatherDependencyModules( allDependencies, includeDependenciesOfDependencies );
    if ( addLinkerDependencies )
        GatherLinkerDependencyModules( allDependencies, includeDependenciesOfDependencies );
    if ( addRuntimeDependencies )
        GatherRuntimeDependencyModules( allDependencies, includeDependenciesOfDependencies );

    TModuleInfoEntryPtrSet::iterator i = allDependencies.begin();
    while ( i != allDependencies.end() )
    {
        const CModuleInfoEntryPtr& module = (*i);
        const CORE::CString* dependencyName = module->GetModuleName( m_targetPlatform );
        if ( GUCEF_NULL != dependencyName )
            dependencyNames.insert( *dependencyName );
        else
            dependencyNames.insert( module->GetConsensusName() );

        ++i;
    }
    return true;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GetDependencyDelta( TModuleInfoEntryPtrSet& dependencyDelta  ,
                                           const CModuleDependencyNodePtr otherTree ,
                                           bool includeDependenciesOfDependencies   ,
                                           bool addDependencies                     ,
                                           bool addLinkerDependencies               ,
                                           bool addRuntimeDependencies              ) const
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( otherTree.IsNULL() )
        return false;

    TModuleInfoEntryPtrSet allThisNodeDependencies;
    if ( addDependencies )
        GatherDependencyModules( allThisNodeDependencies, includeDependenciesOfDependencies );
    if ( addLinkerDependencies )
        GatherLinkerDependencyModules( allThisNodeDependencies, includeDependenciesOfDependencies );
    if ( addRuntimeDependencies )
        GatherRuntimeDependencyModules( allThisNodeDependencies, includeDependenciesOfDependencies );

    TModuleInfoEntryPtrSet allOtherNodeDependencies;
    if ( addDependencies )
        otherTree->GatherDependencyModules( allOtherNodeDependencies, includeDependenciesOfDependencies );
    if ( addLinkerDependencies )
        otherTree->GatherLinkerDependencyModules( allOtherNodeDependencies, includeDependenciesOfDependencies );
    if ( addRuntimeDependencies )
        otherTree->GatherRuntimeDependencyModules( allOtherNodeDependencies, includeDependenciesOfDependencies );

    // Now determine the delta between the two sets

    // We could do this based on the names but we can also make it a tad more performant by using the knowledge that
    // the same base set of ModuleInfoEntry objects is used to construct the different dependency trees.
    // as such the pointers to said objects can be directly compared.
    // Do note to change this to a consensus name string compare should that assumption no longer hold for some reason in the future
    #if 1

    TModuleInfoEntryPtrSet::iterator i = allThisNodeDependencies.begin();
    while ( i != allThisNodeDependencies.end() )
    {
        const CModuleInfoEntryPtr& module = (*i);
        bool foundSameOther = allOtherNodeDependencies.find( module ) != allOtherNodeDependencies.end();
        if ( !foundSameOther )
        {
            dependencyDelta.insert( module );
        }
        ++i;
    }

    #else

    TModuleInfoEntryPtrSet::iterator i = allThisNodeDependencies.begin();
    while ( i != allThisNodeDependencies.end() )
    {
        const CModuleInfoEntryPtr& module = (*i);
        const CORE::CString& moduleName = module->GetConsensusName();

        bool foundSameOther = false;
        TModuleInfoEntryPtrSet::iterator n = allOtherNodeDependencies.begin();
        while ( n != allOtherNodeDependencies.end() )
        {
            const CModuleInfoEntryPtr& otherModule = (*n);
            const CORE::CString& otherModuleName = otherModule->GetConsensusName();

            if ( moduleName == otherModuleName )
            {
                foundSameOther = true;
                break;
            }                
            ++n;
        }

        if ( !foundSameOther )
        {
            dependencyDelta.insert( module );
        }
        ++i;
    }

    #endif

    return true;
}

/*---------------------------------------------------------------------------*/

bool
CModuleDependencyNode::GetSetDependencyDelta( TModuleInfoEntryPtrSet& dependencyDelta       ,
                                              const TModuleDependencyNodePtrSet& baseTrees  ,
                                              const TModuleDependencyNodePtrSet& otherTrees ,
                                              bool includeDependenciesOfDependencies        ,
                                              bool addDependencies                          ,
                                              bool addLinkerDependencies                    ,
                                              bool addRuntimeDependencies                   )
{GUCEF_TRACE;

    bool totalSuccess = true;

    // Important: We compare as a set since this function specifically looks for the delta of the collection
    //            this matters since the 'base' collection can vary wildly per tree so we need to gather the superset of those trees

    TModuleInfoEntryPtrSet allBaseDependencies;

    TModuleDependencyNodePtrSet::const_iterator b = baseTrees.begin();
    while ( b != baseTrees.end() )
    {
        const CModuleDependencyNodePtr& oneTree = (*b);

        if ( addDependencies )
            oneTree->GatherDependencyModules( allBaseDependencies, includeDependenciesOfDependencies );
        if ( addLinkerDependencies )
            oneTree->GatherLinkerDependencyModules( allBaseDependencies, includeDependenciesOfDependencies );
        if ( addRuntimeDependencies )
            oneTree->GatherRuntimeDependencyModules( allBaseDependencies, includeDependenciesOfDependencies );

        ++b;
    }

    TModuleInfoEntryPtrSet allOtherDependencies;

    TModuleDependencyNodePtrSet::const_iterator o = otherTrees.begin();
    while ( o != otherTrees.end() )
    {
        const CModuleDependencyNodePtr& oneTree = (*o);

        if ( addDependencies )
            oneTree->GatherDependencyModules( allOtherDependencies, includeDependenciesOfDependencies );
        if ( addLinkerDependencies )
            oneTree->GatherLinkerDependencyModules( allOtherDependencies, includeDependenciesOfDependencies );
        if ( addRuntimeDependencies )
            oneTree->GatherRuntimeDependencyModules( allOtherDependencies, includeDependenciesOfDependencies );

        ++b;
    }

    // Now determine the delta between the two sets
    
    // We could do this based on the names but we can also make it a tad more performant by using the knowledge that
    // the same base set of ModuleInfoEntry objects is used to construct the different dependency trees.
    // as such the pointers to said objects can be directly compared.
    // Do note to change this to a consensus name string compare should that assumption no longer hold for some reason in the future
    #if 1 
    TModuleInfoEntryPtrSet::iterator i = allBaseDependencies.begin();
    while ( i != allBaseDependencies.end() )
    {
        const CModuleInfoEntryPtr& module = (*i);
        bool foundSameOther = allOtherDependencies.find( module ) != allOtherDependencies.end();
        if ( !foundSameOther )
        {
            dependencyDelta.insert( module );
        }
        ++i;
    }

    #else

    TModuleInfoEntryPtrSet::iterator i = allBaseDependencies.begin();
    while ( i != allBaseDependencies.end() )
    {
        const CModuleInfoEntryPtr& module = (*i);
        const CORE::CString& moduleName = module->GetConsensusName();

        bool foundSameOther = false;
        TModuleInfoEntryPtrSet::iterator n = allOtherDependencies.begin();
        while ( n != allOtherDependencies.end() )
        {
            const CModuleInfoEntryPtr& otherModule = (*n);
            const CORE::CString& otherModuleName = otherModule->GetConsensusName();

            if ( moduleName == otherModuleName )
            {
                foundSameOther = true;
                break;
            }                
            ++n;
        }

        if ( !foundSameOther )
        {
            dependencyDelta.insert( module );
        }
        ++i;
    }

    #endif

    return totalSuccess;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
