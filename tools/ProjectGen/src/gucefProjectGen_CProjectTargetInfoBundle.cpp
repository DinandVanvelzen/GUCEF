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

#include "gucefProjectGen_CProjectTargetInfoBundle.h"

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

const CORE::CString CProjectTargetInfoBundle::ClassTypeName = "GUCEF::PROJECTGEN::CProjectTargetInfoBundle";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CProjectTargetInfoBundle::CProjectTargetInfoBundle( void )
    : CORE::CIDataNodeSerializable()
    , CORE::CTSharedObjCreator< CProjectTargetInfoBundle, MT::CMutex >( this )
    , m_projects()
{GUCEF_TRACE;

}


/*---------------------------------------------------------------------------*/

CProjectTargetInfoBundle::CProjectTargetInfoBundle( const CProjectTargetInfoBundle& src )
    : CORE::CIDataNodeSerializable( src )
    , CORE::CTSharedObjCreator< CProjectTargetInfoBundle, MT::CMutex >( this )
    , m_projects( src.m_projects )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

CProjectTargetInfoBundle::CProjectTargetInfoBundle( CProjectTargetInfoBundle&& src ) GUCEF_NOEXCEPT
    : CORE::CIDataNodeSerializable( GUCEF_MOVE( src ) )
    , CORE::CTSharedObjCreator< CProjectTargetInfoBundle, MT::CMutex >( GUCEF_MOVE( src ), this )
    , m_projects( GUCEF_MOVE( src.m_projects ) )
{GUCEF_TRACE;

}

#endif

/*---------------------------------------------------------------------------*/

CProjectTargetInfoBundle::~CProjectTargetInfoBundle()
{GUCEF_TRACE;

    Clear();
}

/*---------------------------------------------------------------------------*/

void
CProjectTargetInfoBundle::Clear( void )
{GUCEF_TRACE;

    m_projects.clear();
}

/*---------------------------------------------------------------------------*/

const CProjectTargetInfoBundle::TProjectTargetInfoPtrMapMap&
CProjectTargetInfoBundle::GetAllTargets( void ) const
{GUCEF_TRACE;

    return m_projects;
}

/*---------------------------------------------------------------------------*/

CProjectTargetInfoPtr
CProjectTargetInfoBundle::GetPlatformProjectTarget( const CORE::CString& targetName   ,
                                                    const CORE::CString& platformName ) const
{GUCEF_TRACE;

    TProjectTargetInfoPtrMapMap::const_iterator i = m_projects.find( targetName );
    if ( i != m_projects.end() )
    {
        const TProjectTargetInfoPtrMap& targetDefsPerPlatform = (*i).second;
        TProjectTargetInfoPtrMap::const_iterator n = targetDefsPerPlatform.find( platformName );
        if ( n != targetDefsPerPlatform.end() )
        {
            return (*n).second;
        }
    }

    return CProjectTargetInfoPtr();
}

/*---------------------------------------------------------------------------*/

CProjectTargetInfoPtr
CProjectTargetInfoBundle::GetOrCreatePlatformProjectTarget( const CORE::CString& targetName   ,
                                                            const CORE::CString& platformName )
{GUCEF_TRACE;

    TProjectTargetInfoPtrMap& targetDefsPerPlatform = m_projects[ targetName ];
    CProjectTargetInfoPtr& projectTarget = targetDefsPerPlatform[ platformName ];
    if ( projectTarget.IsNULL() )
    {
        projectTarget = CProjectTargetInfo::CreateSharedObj();
        if ( !projectTarget.IsNULL() )
        {
            projectTarget->projectName = targetName;
            projectTarget->SetPlatformName( platformName );
        }
    }

    return projectTarget;
}

/*---------------------------------------------------------------------------*/

CProjectTargetInfoBundle::TProjectTargetInfoPtrMap&
CProjectTargetInfoBundle::GetOrCreateTargetEntry( const CORE::CString& targetName )
{GUCEF_TRACE;

    return m_projects[ targetName ];
}

/*---------------------------------------------------------------------------*/

const CORE::CString*
CProjectTargetInfoBundle::GetTargetMainModuleName( const CORE::CString& targetName     ,
                                                   const CORE::CString& targetPlatform ,
                                                   CModuleInfoPtr* outModuleInfo       ) const
{GUCEF_TRACE;

    CProjectTargetInfoPtr target = GetPlatformProjectTarget( targetName, targetPlatform );
    if ( !target.IsNULL() )
    {        
        if ( !target->mainModule.IsNULL() )
        {
            return target->mainModule->GetModuleName( targetPlatform, outModuleInfo );
        }
    }

    // If no target is specified for a specific platform then there might still be a
    // default for all platforms
    if ( targetPlatform != KnownPlatforms::AllPlatforms )
    {
        target = GetPlatformProjectTarget( targetName, KnownPlatforms::AllPlatforms );
        if ( !target.IsNULL() )
        {        
            if ( !target->mainModule.IsNULL() )
            {
                return target->mainModule->GetModuleName( targetPlatform, outModuleInfo );
            }
        }
    }

    return GUCEF_NULL;
}

/*---------------------------------------------------------------------------*/

CORE::CString
CProjectTargetInfoBundle::GetConsensusTargetName( const TProjectTargetInfoPtrMap& targetPlatforms ,
                                                  const CORE::CString& targetPlatform             )
{GUCEF_TRACE;

    TProjectTargetInfoPtrMap::const_iterator n = targetPlatforms.find( targetPlatform );
    if ( n != targetPlatforms.end() )
    {
        // A target was specified for this platform which makes our job easy
        const CProjectTargetInfoPtr& target = (*n).second;
        if ( !target->mainModule.IsNULL() )
        {
            return target->mainModule->GetModuleTargetName( targetPlatform, true );
        }
    }

    // If no name is specified for all platforms then we will have to
    // determine the best name to use. We do this by getting the name
    // for all platforms and counting how often each is used. The most used
    // name is considered the general consensus name. If the same count applies
    // to multiple we will try to use a popular platform to improve our 'guess'

    typedef GUCEF::map< CORE::CString, CORE::UInt32 > TStringCountMap;

    TStringCountMap countMap;
    n = targetPlatforms.begin();
    while ( n != targetPlatforms.end() )
    {
        CProjectTargetInfoPtr target = (*n).second;
        if ( GUCEF_NULL != target->mainModule )
        {
            CORE::CString targetName = target->mainModule->GetModuleTargetName( targetPlatform, true );
            TStringCountMap::iterator m = countMap.find( targetName );
            if ( m != countMap.end() )
            {
                ++((*m).second);
            }
            else
            {
                countMap[ targetName ] = 1;
            }
        }
        ++n;
    }

    // Not all target origins cause a main module to be defined.
    // As such its perfectly possible be unable to define a consensus target name because the concept does not apply
    // to the collection of targets due to the origins of the collection
    if ( countMap.empty() )
        return CORE::CString();

    // Now that we have the popularity count of each name get the highest count
    CORE::UInt32 highestCount = 0;
    TStringCountMap::iterator i = countMap.begin();
    while ( i != countMap.end() )
    {
        if ( highestCount < (*i).second )
        {
            highestCount = (*i).second;
        }
        ++i;
    }

    // Make the list of most popular names
    TStringSet topNames;
    i = countMap.begin();
    while ( i != countMap.end() )
    {
        if ( highestCount == (*i).second )
        {
            topNames.insert( (*i).first );
        }
        ++i;
    }

    // If we have multiple use a popular platform if
    // possible, otherwise just grab one

    //@TODO: popular platform check

    CString consensusName = (*topNames.begin());
    return consensusName;
}

/*---------------------------------------------------------------------------*/

void
CProjectTargetInfoBundle::CollapseRedundantPlatformTargets( void )
{GUCEF_TRACE;

    TProjectTargetInfoPtrMapMap::iterator t = m_projects.begin();
    while ( t != m_projects.end() )
    {
        // We only process for modules which have an 'AllPlatforms' definition since any additional platforms
        // should be additive to that baseline definition
        // Without one we cannot collapse since there is no unifying target to collapse to as we cannot assume
        // that a given target is available on all platforms unless explicitly defined as such

        TProjectTargetInfoPtrMap& targetByPlatform = (*t).second;

        CORE::CStringMapSet platformRedundancyMap;
        IdentifyRedundantPlatformTargetsForPlatform( targetByPlatform             ,
                                                     KnownPlatforms::AllPlatforms ,
                                                     platformRedundancyMap       );

        CORE::CStringMapSet::const_iterator a = platformRedundancyMap.find( KnownPlatforms::AllPlatforms );
        if ( a != platformRedundancyMap.end() )
        {
            const TStringSet& redundantPlatforms = (*a).second;
            TStringSet::const_iterator r = redundantPlatforms.begin();
            while ( r != redundantPlatforms.end() )
            {
                targetByPlatform.erase( (*r) );
                ++r;
            }
        }
        // else: targets that don't have a 'AllPlatforms' target cannot be collapsed
        ++t;
    }
}

/*---------------------------------------------------------------------------*/

void
CProjectTargetInfoBundle::IdentifyRedundantPlatformTargetsForPlatform( const TProjectTargetInfoPtrMap& targetByPlatform   ,
                                                                       const CORE::CString& platformName                  ,
                                                                       CORE::CStringMapSet& redundantPlatformsPerPlatform )
{GUCEF_TRACE;

    TProjectTargetInfoPtrMap::const_iterator a = targetByPlatform.find( platformName );
    if ( a != targetByPlatform.end() )
    {
        const CProjectTargetInfoPtr& chosenPlatformsTarget = (*a).second;
        TStringSet& redundantPlatforms = redundantPlatformsPerPlatform[ platformName ];

        // We check to see if the modules match across platforms which is all that is needed here
        // The modules themselves will deal with platform specifics at an intra-module level
        // The use-case we look for are cases where some platforms have different modules then others
        // in which case we need to keep them as distinct targets        
        TProjectTargetInfoPtrMap::const_iterator m = targetByPlatform.begin();
        while (  m != targetByPlatform.end() )
        {
            const CORE::CString& currentPlatform = (*m).first;
            if ( currentPlatform != platformName )
            {
                const CProjectTargetInfoPtr& somePlatformTarget = (*m).second;
                if ( somePlatformTarget->mainModule == chosenPlatformsTarget->mainModule         &&
                     somePlatformTarget->mainModuleName == chosenPlatformsTarget->mainModuleName &&
                     somePlatformTarget->modules == chosenPlatformsTarget->modules                )
                {
                    redundantPlatforms.insert( currentPlatform );
                }
            }
            ++m;
        }
    }
}

/*---------------------------------------------------------------------------*/

void
CProjectTargetInfoBundle::IdentifyRedundantPlatformTargets( CORE::CStringMapMapSet& redundantPlatformsPerProjectTarget ) const
{GUCEF_TRACE;

    TProjectTargetInfoPtrMapMap::const_iterator i = m_projects.begin();
    while ( i != m_projects.end() )
    {
        const CORE::CString& targetProjectName = (*i).first;
        const TProjectTargetInfoPtrMap& platformsForProject = (*i).second;
        CORE::CStringMapSet& redundantPlatformsForProject = redundantPlatformsPerProjectTarget[ targetProjectName ];

        TProjectTargetInfoPtrMap::const_iterator n = platformsForProject.begin();
        while ( n != platformsForProject.end() )
        {
            const CORE::CString& platformName = (*n).first;

            // Don't re-process platforms already marked as redundant
            bool alreadyRedundant = false;
            CORE::CStringMapSet::iterator r = redundantPlatformsForProject.begin();
            while ( r != redundantPlatformsForProject.end() )
            {
                const CORE::CStringSet& currentRedundantPlatforms = (*r).second;
                if ( currentRedundantPlatforms.find( platformName ) != currentRedundantPlatforms.end() )
                {
                    // This platform is already marked as redundant against another platform
                    alreadyRedundant = true;
                    break;
                }

                ++r;
            }

            if ( !alreadyRedundant )
            {
                IdentifyRedundantPlatformTargetsForPlatform( platformsForProject          ,
                                                             platformName                 ,
                                                             redundantPlatformsForProject );
            }

            ++n;
        }

        ++i;
    }
}

/*---------------------------------------------------------------------------*/
               
bool 
CProjectTargetInfoBundle::Serialize( CORE::CDataNode& domRootNode                        ,
                                     const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    CORE::CDataNode* projectsNode = domRootNode.FindOrAdd( "Targets", GUCEF_DATATYPE_ARRAY );
    if ( GUCEF_NULL == projectsNode )
        return false;

    // Before we serialize we identify redundant platform targets so that we can avoid adding
    // duplicate content to the DOM where we support a notation of multiple platforms sharing the same target info via a single section
    // this is done via the ';' delimited list in the 'Platform' attribute of the PlatformTargetInfo node
    CORE::CStringMapMapSet redundantPlatformsPerProjectTarget;
    IdentifyRedundantPlatformTargets( redundantPlatformsPerProjectTarget );

    TProjectTargetInfoPtrMapMap::const_iterator i = m_projects.begin();
    while ( i != m_projects.end() )
    {
        const CORE::CString& targetProjectName = (*i).first;
        const TProjectTargetInfoPtrMap& platformsForProject = (*i).second;
        CORE::CStringMapSet& redundantPlatformsForProject = redundantPlatformsPerProjectTarget[ targetProjectName ];

        CORE::CDataNode* targetNode = projectsNode->AddChild( "Target", GUCEF_DATATYPE_OBJECT );
        if ( GUCEF_NULL == targetNode )
            return false;

        totalSuccess = targetNode->SetAttribute( "name", targetProjectName ) && totalSuccess;

        TProjectTargetInfoPtrMap::const_iterator n = platformsForProject.begin();
        while ( n != platformsForProject.end() )
        {
            const CORE::CString& platformName = (*n).first;
            const CProjectTargetInfoPtr& project = (*n).second;

            if ( !project.IsNULL() )
            {
                // We should only serialize platform targets that have content to avoid clutter which doesn't add value
                if ( !project->modules.empty() || !project->mainModule.IsNULL() )
                {
                    bool isRedundant = false;
                    bool isMultiPlatform = false;
                    CORE::CString comboPlatformStr;

                    CORE::CStringMapSet::iterator m = redundantPlatformsForProject.find( platformName );
                    if ( m != redundantPlatformsForProject.end() )
                    {
                        CORE::CStringSet& redundantPlatforms = m->second;
                        if ( !redundantPlatforms.empty() )
                        {
                            CORE::CStringSet reducedRedundantPlatforms( redundantPlatforms );
                            reducedRedundantPlatforms.insert( platformName );
                            reducedRedundantPlatforms = ReduceToUseMultiPlatformNamesIfFeasible( reducedRedundantPlatforms );
                            comboPlatformStr = CORE::StringSetToString( reducedRedundantPlatforms, CORE::CString::Empty, ';' );
                            isMultiPlatform = true;
                        }
                    }
                    else
                    {
                        m = redundantPlatformsForProject.begin();
                        while ( m != redundantPlatformsForProject.end() )
                        {
                            const CORE::CStringSet& redundantPlatforms = (*m).second;
                            if ( redundantPlatforms.find( platformName ) != redundantPlatforms.end() )
                            {
                                isRedundant = true;
                                break;
                            }
                            ++m;
                        }
                    }

                    if ( !isRedundant )
                    {
                        CORE::CDataNode* platformTargetInfoNode = targetNode->AddChild( "PlatformTargetInfo", GUCEF_DATATYPE_OBJECT );
                        if ( GUCEF_NULL == platformTargetInfoNode )
                            return false;

                        totalSuccess = project->Serialize( *platformTargetInfoNode, settings ) && totalSuccess;

                        // if in reality this target info applies to multiple platforms then denote that
                        // we do so by overriding the platform attribute with a ';' delimited list of platforms
                        if ( isMultiPlatform )
                        {
                            platformTargetInfoNode->SetAttribute( "Platform", comboPlatformStr );
                            comboPlatformStr.Clear();
                            isMultiPlatform = false;
                        }
                    }
                }
            }
            ++n;
        }

        ++i;
    }
    
    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CProjectTargetInfoBundle::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                       const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    bool totalSuccess = true;

    const CORE::CDataNode* targetsNode = domRootNode.Find( "Targets" );
    if ( GUCEF_NULL == targetsNode )
        return true; // simply doesn't have project info which is allowable

    CORE::CDataNode::const_iterator i = targetsNode->ConstBegin();
    while ( i != targetsNode->ConstEnd() )
    {
        const CORE::CDataNode* targetNode = (*i);
        if GUCEF_PREDICT_TRUE( GUCEF_NULL != targetNode )
        {
            CORE::CString targetProjectName = targetNode->GetAttributeValueOrChildValueByName( "name" ).AsString();

            CORE::CDataNode::TConstDataNodeSet platformNodes = targetNode->FindChildrenOfType( "PlatformTargetInfo" );
            CORE::CDataNode::TConstDataNodeSet::const_iterator n = platformNodes.begin();
            while ( n != platformNodes.end() )
            {
                const CORE::CDataNode* platformTargetInfoNode = (*n);
                if GUCEF_PREDICT_TRUE( GUCEF_NULL != platformTargetInfoNode )
                {
                    // Identical platforms, content wise, can be grouped under one target info node
                    // we will split them up here as its a serialization/deserialization convenience/compression feature but they are distinct targets
                    // this is done via the ';' delimited list in the 'Platform' attribute of the PlatformTargetInfo node
                    // its a DOM convenience feature only
                    CORE::CStringSet platforms = ResolveMultiPlatformName( platformTargetInfoNode->GetAttributeValueOrChildValueByName( "Platform" ).AsString() );

                    CORE::CStringSet::iterator p = platforms.begin();
                    while ( p != platforms.end() )
                    {
                        const CORE::CString& platformName = (*p);

                        CProjectTargetInfoPtr projectTargetInfo = CProjectTargetInfo::CreateSharedObj();
                        if GUCEF_PREDICT_TRUE( !projectTargetInfo.IsNULL() )
                        {
                            bool deserializeTargetSuccess = projectTargetInfo->Deserialize( *platformTargetInfoNode, settings );
                            if ( deserializeTargetSuccess )
                            {
                                projectTargetInfo->SetPlatformName( platformName );
                                m_projects[ targetProjectName ][ platformName ] = projectTargetInfo;
                            }
                            else
                            {
                                totalSuccess = false;
                            }
                        }
                        else
                        {
                            totalSuccess = false;
                        }
                        ++p;
                    }
                }
                else
                {
                    totalSuccess = false;
                }
            }
        }
        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

CORE::CICloneable* 
CProjectTargetInfoBundle::Clone( void ) const 
{GUCEF_TRACE;

    return new CProjectTargetInfoBundle( *this );
}

/*---------------------------------------------------------------------------*/

const CORE::CString& 
CProjectTargetInfoBundle::GetClassTypeName( void ) const 
{GUCEF_TRACE;

    return ClassTypeName;
}

/*---------------------------------------------------------------------------*/

void
CProjectTargetInfoBundle::SyncObjectsToNames( void )
{GUCEF_TRACE;

    TProjectTargetInfoPtrMapMap::iterator i = m_projects.begin();
    while ( i != m_projects.end() )
    {
        TProjectTargetInfoPtrMap& projectPerPlatform = (*i).second;
        TProjectTargetInfoPtrMap::iterator n = projectPerPlatform.begin();
        while ( n != projectPerPlatform.end() )
        {
            CProjectTargetInfoPtr& projectTargetInfo = (*n).second;
            if ( !projectTargetInfo.IsNULL() )
            {
                projectTargetInfo->SyncObjectsToNames();
            }
            ++n;
        }

        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
CProjectTargetInfoBundle::SyncNamesToObjects( const CProjectInfo& projectInfo )
{GUCEF_TRACE;

    TProjectTargetInfoPtrMapMap::iterator i = m_projects.begin();
    while ( i != m_projects.end() )
    {
        TProjectTargetInfoPtrMap& projectPerPlatform = (*i).second;
        TProjectTargetInfoPtrMap::iterator n = projectPerPlatform.begin();
        while ( n != projectPerPlatform.end() )
        {
            CProjectTargetInfoPtr& projectTargetInfo = (*n).second;
            if ( !projectTargetInfo.IsNULL() )
            {
                projectTargetInfo->SyncNamesToObjects( projectInfo );
            }
            ++n;
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
