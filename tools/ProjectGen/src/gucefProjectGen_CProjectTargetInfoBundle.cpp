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


/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CProjectTargetInfoBundle::CProjectTargetInfoBundle( void )
    : CORE::CTSharedObjCreator< CProjectTargetInfoBundle, MT::CMutex >( this )
    , m_targets()
{GUCEF_TRACE;

    Clear();
}

/*---------------------------------------------------------------------------*/

CProjectTargetInfoBundle::~CProjectTargetInfoBundle()
{GUCEF_TRACE;

    Clear();
}

/*---------------------------------------------------------------------------*/

void
CProjectTargetInfoBundle::Clear( void )
{GUCEF_TRACE;

    m_targets.clear();
}

/*---------------------------------------------------------------------------*/

const CProjectTargetInfoBundle::TProjectTargetInfoPtrMapMap&
CProjectTargetInfoBundle::GetAllTargets( void ) const
{GUCEF_TRACE;

    return m_targets;
}

/*---------------------------------------------------------------------------*/

CProjectTargetInfoPtr
CProjectTargetInfoBundle::GetPlatformProjectTarget( const CORE::CString& targetName   ,
                                                    const CORE::CString& platformName ) const
{GUCEF_TRACE;

    TProjectTargetInfoPtrMapMap::const_iterator i = m_targets.find( targetName );
    if ( i != m_targets.end() )
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

    TProjectTargetInfoPtrMap& targetDefsPerPlatform = m_targets[ targetName ];
    CProjectTargetInfoPtr& projectTarget = targetDefsPerPlatform[ platformName ];
    if ( projectTarget.IsNULL() )
    {
        projectTarget = CProjectTargetInfo::CreateSharedObj();
        projectTarget->projectName = targetName;
    }

    return projectTarget;
}

/*---------------------------------------------------------------------------*/

CProjectTargetInfoBundle::TProjectTargetInfoPtrMap&
CProjectTargetInfoBundle::GetOrCreateTargetEntry( const CORE::CString& targetName )
{GUCEF_TRACE;

    return m_targets[ targetName ];
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

    typedef std::map< CORE::CString, CORE::UInt32 > TStringCountMap;

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

    TProjectTargetInfoPtrMapMap::iterator t = m_targets.begin();
    while ( t != m_targets.end() )
    {
        // First check to see if this module has a 'AllPlatforms' definition
        // Without one we cannot collapse since there is no unifying target to collapse to
        TProjectTargetInfoPtrMap& targetByPlatform = (*t).second;
        TProjectTargetInfoPtrMap::iterator a = targetByPlatform.find( KnownPlatforms::AllPlatforms );
        if ( a != targetByPlatform.end() )
        {
            CProjectTargetInfoPtr& allPlatformsTarget = (*a).second;
                
            // We now check to see if the modules match across platforms which is all that is needed here
            // The modules themselves will deal with platform specifics at an intra-module level
            // The use-case we look for are cases where some platforms have different modules then others
            // in which case we need to keep them as distinct targets
            TStringSet redundantPlatforms; 
            TProjectTargetInfoPtrMap::iterator m = targetByPlatform.begin();
            while (  m != targetByPlatform.end() )
            {
                const CORE::CString& currentPlatform = (*m).first;
                if ( currentPlatform != KnownPlatforms::AllPlatforms )
                {
                    CProjectTargetInfoPtr& somePlatformTarget = (*m).second;
                    if ( somePlatformTarget->modules == allPlatformsTarget->modules )
                        redundantPlatforms.insert( (*m).first );
                }
                ++m;
            }
            TStringSet::iterator r = redundantPlatforms.begin();
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
