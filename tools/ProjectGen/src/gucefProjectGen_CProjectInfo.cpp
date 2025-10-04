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

#ifndef GUCEF_PROJECTGEN_DATATYPES_H
#include "gucefProjectGen_DataTypes.h"
#define GUCEF_PROJECTGEN_DATATYPES_H
#endif /* GUCEF_PROJECTGEN_DATATYPES_H ? */

#include "gucefProjectGen_CProjectInfo.h"

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

const CORE::CString CProjectInfo::ClassTypeName = "GUCEF::PROJECTGEN::CProjectInfo";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CProjectInfo::CProjectInfo( void ) 
    : CORE::CTSharedObjCreator< CProjectInfo, MT::CMutex >( this )
    , rootDirs()
    , modules()
    , dirProcessingInstructions()
    , globalDirExcludeList()
    , platforms()
    , m_moduleDependencyChains()
    , m_actualPlatformsUsed()
    , m_settings()
    , m_disabledPlatforms()
    , m_projectName()
    , m_rwLock( true )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CProjectInfo::CProjectInfo( const CProjectInfo& src ) 
    : CORE::CTSharedObjCreator< CProjectInfo, MT::CMutex >( this )
    , rootDirs( src.rootDirs )
    , modules( src.modules )
    , dirProcessingInstructions( src.dirProcessingInstructions )
    , globalDirExcludeList( src.globalDirExcludeList )
    , platforms( src.platforms )
    , m_moduleDependencyChains( src.m_moduleDependencyChains )
    , m_actualPlatformsUsed( src.m_actualPlatformsUsed )
    , m_settings( src.m_settings )
    , m_disabledPlatforms( src.m_disabledPlatforms )
    , m_projectName( src.m_projectName )
    , m_rwLock( true )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CProjectInfo::~CProjectInfo() 
{GUCEF_TRACE;

    Clear();
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::GetExecutables( TModuleInfoEntryPtrSet& executableTargets ,
                              const CORE::CString& platform             ) const
{GUCEF_TRACE;

    TStringToModuleInfoEntryPtrMap::const_iterator i = modules.begin();
    while ( i != modules.end() )
    {
        const CModuleInfoEntryPtr& entry = (*i).second;
        if ( MODULETYPE_EXECUTABLE == entry->GetModuleType( platform ) )
            executableTargets.insert( entry );
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::GetAllTargets( CProjectTargetInfoBundle& targets     ,
                             bool tagsAsTargets                    ,
                             bool deltaFormatForSpecificPlatforms  ,
                             const TStringSet& platformsToConsider ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetAllTargets: Commencing splitting of the gathered project info into derived targets. Will split among the " + 
        CORE::ToString( platformsToConsider.size() ) + " platforms defined" );

    // sanity check
    if ( deltaFormatForSpecificPlatforms &&
         ( platformsToConsider.find( KnownPlatforms::AllPlatforms ) == platformsToConsider.end() ||
           platformsToConsider.size() < 2 ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetAllTargets: Cannot perform delta determination without 'all' platform and at least 2 platforms" );
        return false;
    }
    if ( deltaFormatForSpecificPlatforms &&
         !AreDependencyChainsInitialized() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetAllTargets: Cannot perform delta determination without dependency chains being initialized" );
        return false;
    }

    // Go through the platforms we were asked to consider (less platforms = less work)
    // For each determine what the valid targets are
    TStringSet::iterator p = platformsToConsider.begin();
    while ( p != platformsToConsider.end() )
    {
        const CORE::CString& platform = (*p);

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetAllTargets: Locating executables (if any) for platform " + platform );

        // First let's process all the executables for the given platform as the obvious targets

        TModuleInfoEntryPtrSet executables;
        GetExecutables( executables, platform );

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetAllTargets: Located " + CORE::ToString( executables.size() ) + " executable target candidates for platform " + (*p) );

        TModuleInfoEntryPtrSet::iterator i = executables.begin();
        while ( i != executables.end() )
        {
            CModuleInfoEntryPtr executable = (*i);
            CORE::CString targetName = executable->GetModuleNameAlways( platform );
            bool hasAllPlatformsDef = executable->HasAllPlatformsDefinition();
            bool determineDeltaForSpecificPlatforms = hasAllPlatformsDef && deltaFormatForSpecificPlatforms;

            // Don't bother if the executable itself doesn't have a platform definition for the current platform            
            if ( executable->IsApplicableForPlatform( platform ) )
            {            
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetAllTargets: Locating dependencies for target candidate \"" + 
                    targetName + "\" for platform " + (*p) );

                CORE::CString projectName = m_projectName + "_exe_" + targetName;                        
                CProjectTargetInfoPtr target = targets.GetOrCreatePlatformProjectTarget( projectName, platform );

                target->projectName = projectName;
                target->mainModule = executable;

                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetAllTargets: Executable Target \"" + targetName + "\" has been defined for platform " + platform ); 

                if ( !determineDeltaForSpecificPlatforms )
                {
                    target->modules.insert( executable );

                    TModuleInfoEntryPtrSet foundDependencies;
                    if ( GetModuleDependencies( executable, platform, foundDependencies, true, true, false ) )
                    {
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetAllTargets: Located " + CORE::ToString( foundDependencies.size() ) + 
                            " dependencies for executable target \"" + targetName + "\" for platform " + platform );

                        TModuleInfoEntryPtrSet::iterator j = foundDependencies.begin();
                        while ( j != foundDependencies.end() )
                        {
                            const CModuleInfoEntryPtr& dependency = (*j);
                            target->modules.insert( dependency );

                            ++j;
                        }
                    }
                    else
                    {
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetAllTargets: We cannot satisfy the full dependency chain for executable \"" + targetName + 
                        "\" for the given platform \"" + platform + "\", it will not be available as a target specific to this platform" );
                    }
                }
                else
                {
                    if ( platform != KnownPlatforms::AllPlatforms )
                    {
                        bool createdDelta = GetModuleDependencyDeltaAcrossPlatforms( target->modules              ,
                                                                                     targetName                   ,
                                                                                     KnownPlatforms::AllPlatforms ,
                                                                                     platform                     ,
                                                                                     true                         ,
                                                                                     true                         ,
                                                                                     true                         ,
                                                                                     true                         );
                        if ( createdDelta )
                        {
                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetAllTargets: Generated module dependency delta for executable \"" + targetName + 
                                "\" for the given platform \"" + platform + "\". There are " + CORE::ToString( target->modules.size() ) + " modules specific to this platform" );
                        }
                        else
                        {
                            totalSuccess = false;
                            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetAllTargets: Cannot obtain dependency delta for executable \"" + targetName + 
                                "\" for the given platform \"" + platform + "\"" );
                        }
                    }
                    else
                    {
                        target->modules.insert( executable );

                        TModuleInfoEntryPtrSet foundDependencies;
                        if ( GetModuleDependencies( executable, platform, foundDependencies, true, true, false ) )
                        {
                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetAllTargets: Located " + CORE::ToString( foundDependencies.size() ) + 
                                " dependencies for executable target \"" + targetName + "\" for platform " + platform );

                            TModuleInfoEntryPtrSet::iterator j = foundDependencies.begin();
                            while ( j != foundDependencies.end() )
                            {
                                const CModuleInfoEntryPtr& dependency = (*j);
                                target->modules.insert( dependency );

                                ++j;
                            }

                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetAllTargets: Generated module dependency listing for executable \"" + targetName + 
                                "\" for the 'All' platform which has " + CORE::ToString( target->modules.size() ) + " baseline modules. Specific platforms may add their own platform delta" );
                        }
                        else
                        {
                             GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetAllTargets: We cannot satisfy the full dependency chain for executable \"" + targetName + 
                                "\" for the given platform \"" + platform + "\", it will not be available as a target specific to this platform" );
                        }
                    }
                }
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetAllTargets: The executable \"" + targetName + "\" has no definition specific to the given platform \"" 
                    + platform + "\" and thus will not be available as a target specific to this platform" ); 
            }
            ++i;
        }
        ++p;
    }

    if ( tagsAsTargets )
    {
        // As requested: Next we will also look for tagged modules
        // The tags themselves will make them part of a 'tag' group target

        TStringSet tagsUsed;
        GetAllTagsUsed( tagsUsed );

        TStringSet::iterator i = tagsUsed.begin();
        while ( i != tagsUsed.end() )
        {
            const CORE::CString& tag = (*i);
            CORE::CString projectName = m_projectName + "_tag_" + tag;

            // First establish the baseline superset
            // We want to do this once since its the same set for every non-'all'-platform we compare against

            CORE::CStringSet taggedModuleNames;
            GetTaggedModulesByName( tag, taggedModuleNames, KnownPlatforms::AllPlatforms );

            CProjectTargetInfoPtr allPlatformsTarget = targets.GetOrCreatePlatformProjectTarget( projectName, KnownPlatforms::AllPlatforms );
            if ( !allPlatformsTarget.IsNULL() )
            {
                bool collectedBaseSet = GetModuleInfoEntries( taggedModuleNames            ,
                                                              KnownPlatforms::AllPlatforms ,
                                                              true                         ,
                                                              allPlatformsTarget->modules  );

                TStringSet::iterator p = platformsToConsider.begin();
                while ( p != platformsToConsider.end() )
                {
                    const CORE::CString& platform = (*p);

                    // ignore the 'all' platform since we already gathered that
                    if ( platform != KnownPlatforms::AllPlatforms )
                    {
                        CProjectTargetInfoPtr platformTarget = targets.GetOrCreatePlatformProjectTarget( projectName, platform );
                        if ( !platformTarget.IsNULL() )
                        {
                            CORE::CStringSet taggedModuleNames;
                            GetTaggedModulesByName( tag, taggedModuleNames, platform );

                            bool createdDelta = GetAllModuleDependenciesDeltaAcrossPlatforms( platformTarget->modules      ,
                                                                                              taggedModuleNames            ,
                                                                                              KnownPlatforms::AllPlatforms ,
                                                                                              platform                     ,
                                                                                              true                         ,
                                                                                              true                         ,
                                                                                              true                         ,
                                                                                              true                         );
                            if ( createdDelta )
                            {
                                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetAllTargets: Generated module set delta for tag \"" + tag + 
                                    "\" for the given platform \"" + platform + "\". There are " + CORE::ToString( platformTarget->modules.size() ) + " modules specific to this platform vs 'all' for this tag" );
                            }
                            else
                            {
                                totalSuccess = false;
                                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetAllTargets: Cannot obtain dependency delta for tag \"" + tag + 
                                    "\" for the given platform \"" + platform + "\"" );
                            }
                        }
                    }
                    ++p;
                }                
            }
            ++i;
        }
    }

    if ( platformsToConsider.find( KnownPlatforms::AllPlatforms ) != platformsToConsider.end() )
    {
        // In order to facilitate uniform processing we also include the complete project as its own target
        // This ensures that backend code doesn't need different code to process the complete project vs some
        // target based subset
        // Note that the full project is by definition "all" platforms because there is no target differentiation
        // It relies solely on module level per-platform differences to be processed

        CProjectTargetInfoPtr fullProjectTarget = targets.GetOrCreatePlatformProjectTarget( GetProjectName(), KnownPlatforms::AllPlatforms );
        fullProjectTarget->projectName = GetProjectName();
        fullProjectTarget->mainModule.Unlink();
        TStringToModuleInfoEntryPtrMap::const_iterator w = modules.begin();
        while ( w != modules.end() )
        {
            fullProjectTarget->modules.insert( (*w).second );
            ++w;
        }
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::GetAllTargets( CProjectTargetInfoBundle& targets    ,
                             bool tagsAsTargets                   ,
                             bool deltaFormatForSpecificPlatforms ) const
{GUCEF_TRACE;

    TStringSet platformList;
    GetAllEnabledPlatformsUsed( platformList, true );

    return GetAllTargets( targets, tagsAsTargets, deltaFormatForSpecificPlatforms, platformList );
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::GetAllPlatformsUsed( TStringSet& platformList ,
                                   bool okToUseCachedValue  ) const
{GUCEF_TRACE;

    platformList.clear();

    if ( !m_actualPlatformsUsed.empty() && okToUseCachedValue )
    {
        // We have cached the platforms used so we can just return that
        platformList = m_actualPlatformsUsed;
        return;
    }

    // We have no cached value or not allowed to use it so we will have to (re)determine the platforms used
    TStringToModuleInfoEntryPtrMap::const_iterator i = modules.begin();
    while ( i != modules.end() )
    {
        const TModuleInfoPtrMap& modulesPerPlatform = (*i).second->GetModulesPerPlatform();
        TModuleInfoPtrMap::const_iterator n = modulesPerPlatform.begin();
        while ( n != modulesPerPlatform.end() )
        {
            platformList.insert( (*n).first );
            ++n;
        }
        ++i;
    }

    m_actualPlatformsUsed = platformList;
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::GetAllEnabledPlatformsUsed( TStringSet& platformList ,
                                          bool okToUseCachedValue  ) const
{GUCEF_TRACE;

    // First just get all platforms we actually reference in our project data
    GetAllPlatformsUsed( platformList, okToUseCachedValue );

    // Now we erase the ones that are disabled from the 'actually used' list
    CORE::CStringSet::const_iterator i = m_disabledPlatforms.begin();
    while ( i != m_disabledPlatforms.end() )
    {
        const CORE::CString& disabledPlatform = (*i); 
        platformList.erase( disabledPlatform );
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

CModuleInfoEntryPtr
CProjectInfo::GetModuleInfoEntry( const CORE::CString& consensusModuleName ) const
{GUCEF_TRACE;

    // the consensus name is the key 
    TStringToModuleInfoEntryPtrMap::const_iterator m = modules.find( consensusModuleName );
    if ( m != modules.end() )
    {
        const CModuleInfoEntryPtr& moduleInfoEntry = (*m).second;
        if ( !moduleInfoEntry.IsNULL() )
        {            
            return moduleInfoEntry;
        }
    }
    return CModuleInfoEntryPtr();
}

/*---------------------------------------------------------------------------*/

CModuleInfoEntryPtr
CProjectInfo::GetModuleInfoEntry( const CORE::CString& moduleName    ,
                                  const CORE::CString& platform      ,
                                  CModuleInfoPtr* platformModuleInfo ) const
{GUCEF_TRACE;

    // Check the dependency chains first since its conveniently mapped by platform and module name
    TStringToModuleDependencyNodePtrMap::const_iterator i = m_moduleDependencyChains.find( platform );
    if ( i != m_moduleDependencyChains.end() )
    {
        // we have chains for this platform, now look for the specific module
        const TModuleDependencyNodePtrMap& chainsForPlatform = (*i).second;
        TModuleDependencyNodePtrMap::const_iterator n = chainsForPlatform.find( moduleName );
        if ( n != chainsForPlatform.end() )
        {            
            const CModuleDependencyNodePtr dependencyChain = (*n).second;
            CModuleInfoEntryPtr moduleInfoEntry = dependencyChain->GetModule();
            if ( !moduleInfoEntry.IsNULL() && GUCEF_NULL != platformModuleInfo )
            {
                *platformModuleInfo = moduleInfoEntry->FindModuleInfoForPlatform( platform );
            }
            return moduleInfoEntry;
        }
    }
    if ( platform != KnownPlatforms::AllPlatforms && !platform.IsNULLOrEmpty() )
    {
        // Also check for 'all' platforms
        i = m_moduleDependencyChains.find( KnownPlatforms::AllPlatforms );
        if ( i != m_moduleDependencyChains.end() )
        {
            // we have chains for this platform, now look for the specific module
            const TModuleDependencyNodePtrMap& chainsForPlatform = (*i).second;
            TModuleDependencyNodePtrMap::const_iterator n = chainsForPlatform.find( moduleName );
            if ( n != chainsForPlatform.end() )
            {            
                const CModuleDependencyNodePtr dependencyChain = (*n).second;
                CModuleInfoEntryPtr moduleInfoEntry = dependencyChain->GetModule();
                if ( !moduleInfoEntry.IsNULL() && GUCEF_NULL != platformModuleInfo )
                {
                    *platformModuleInfo = moduleInfoEntry->FindModuleInfoForPlatform( platform );
                }
                return moduleInfoEntry;
            }
        }
    }

    // No such reference in the dependency chains
    // fall back to the flat list of modules
    CModuleInfoEntryPtr moduleInfoEntry = GetModuleInfoEntry( moduleName );
    if ( !moduleInfoEntry.IsNULL() )
    {
        if ( moduleInfoEntry->IsApplicableForPlatform( platform ) )
        {
            if ( GUCEF_NULL != platformModuleInfo )
                *platformModuleInfo = moduleInfoEntry->FindModuleInfoForPlatform( platform );
                
            return moduleInfoEntry;
        }
    }

    return CModuleInfoEntryPtr();    
}

/*---------------------------------------------------------------------------*/

CModuleInfoEntryPtr
CProjectInfo::GetModuleInfoEntry( const CORE::CString& moduleName    ,
                                  const CORE::CString& platform      ,
                                  CModuleInfoPtr* platformModuleInfo ,
                                  bool createNewIfNoneExists         )
{GUCEF_TRACE;

    // Check the dependency chains first since its conveniently mapped by platform and module name
    TStringToModuleDependencyNodePtrMap::iterator i = m_moduleDependencyChains.find( platform );
    if ( i != m_moduleDependencyChains.end() )
    {
        // we have chains for this platform, now look for the specific module
        const TModuleDependencyNodePtrMap& chainsForPlatform = (*i).second;
        TModuleDependencyNodePtrMap::const_iterator n = chainsForPlatform.find( moduleName );
        if ( n != chainsForPlatform.end() )
        {            
            const CModuleDependencyNodePtr dependencyChain = (*n).second;
            CModuleInfoEntryPtr moduleInfoEntry = dependencyChain->GetModule();
            if ( !moduleInfoEntry.IsNULL() && GUCEF_NULL != platformModuleInfo )
            {
                *platformModuleInfo = moduleInfoEntry->FindOrCreateModuleInfoForPlatform( platform, createNewIfNoneExists );
            }
            return moduleInfoEntry;
        }
    }
    if ( platform != KnownPlatforms::AllPlatforms && !platform.IsNULLOrEmpty() )
    {
        // Also check for 'all' platforms
        i = m_moduleDependencyChains.find( KnownPlatforms::AllPlatforms );
        if ( i != m_moduleDependencyChains.end() )
        {
            // we have chains for this platform, now look for the specific module
            const TModuleDependencyNodePtrMap& chainsForPlatform = (*i).second;
            TModuleDependencyNodePtrMap::const_iterator n = chainsForPlatform.find( moduleName );
            if ( n != chainsForPlatform.end() )
            {            
                const CModuleDependencyNodePtr dependencyChain = (*n).second;
                CModuleInfoEntryPtr moduleInfoEntry = dependencyChain->GetModule();
                if ( !moduleInfoEntry.IsNULL() && GUCEF_NULL != platformModuleInfo )
                {
                    *platformModuleInfo = moduleInfoEntry->FindOrCreateModuleInfoForPlatform( platform, createNewIfNoneExists );
                }
                return moduleInfoEntry;
            }
        }
    }

    // No such reference in the dependency chains
    // fall back to the flat list of modules
    CModuleInfoEntryPtr moduleInfoEntry = GetModuleInfoEntry( moduleName );
    if ( !moduleInfoEntry.IsNULL() )
    {
        if ( moduleInfoEntry->IsApplicableForPlatform( platform ) )
        {
            if ( GUCEF_NULL != platformModuleInfo )
                *platformModuleInfo = moduleInfoEntry->FindOrCreateModuleInfoForPlatform( platform, createNewIfNoneExists );
                
            return moduleInfoEntry;
        }
    }

    return CModuleInfoEntryPtr();    
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::GetModuleInfoEntries( const CORE::CStringSet& moduleNames         ,
                                    const CORE::CString& platform               ,
                                    bool includeDependenciesForTheGivenPlatform ,
                                    TModuleInfoEntryPtrSet& foundModules        ) const
{GUCEF_TRACE;

    // This functionality requires dependency chains to be built
    if ( !AreDependencyChainsInitialized() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:GetModuleInfoEntries: Dependency chains are not initialized" );
        return false;
    }

    bool totalSuccess = true;

    TStringToModuleDependencyNodePtrMap::const_iterator i = m_moduleDependencyChains.find( platform );
    if ( i != m_moduleDependencyChains.end() )
    {
        // we have chains for this platform, now look for the specific module
        const TModuleDependencyNodePtrMap& chainsForPlatform = (*i).second;

        CORE::CStringSet::const_iterator n = moduleNames.begin();
        while ( n != moduleNames.end() )
        {
            const CORE::CString& moduleName = (*n);
            TModuleDependencyNodePtrMap::const_iterator m = chainsForPlatform.find( moduleName );
            if ( m != chainsForPlatform.end() )
            {            
                const CModuleDependencyNodePtr dependencyChain = (*m).second;
                CModuleInfoEntryPtr moduleInfoEntry = dependencyChain->GetModule();
                if ( !moduleInfoEntry.IsNULL() )
                {
                    foundModules.insert( moduleInfoEntry );
                    if ( includeDependenciesForTheGivenPlatform )
                    {
                        totalSuccess = dependencyChain->GatherDependenciesOfDependencies( foundModules, false ) && totalSuccess;
                    }
                }
            }
            else
            {
                totalSuccess = false;
            }
            ++n;
        }
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::ReduceAllPlatformsDefinitionToSustainablePlatforms( CModuleInfoEntryPtr moduleInfoEntry )
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( moduleInfoEntry.IsNULL() )
        return false;

    CModuleInfoPtr allPlatformsModuleInfo = moduleInfoEntry->FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms );
    if GUCEF_PREDICT_FALSE( allPlatformsModuleInfo.IsNULL() )
        return false;

    bool totalFixSuccess = true;

    // Check for problem scenario 1: Static binary dependencies at the 'all' platform level
    if ( allPlatformsModuleInfo->linkerSettings.HasAnyStaticLibraryLinkerDependency() )
    {
        // We found a problem. We cannot have static binary dependencies at the 'all' platform level
        // We will have to reduce the 'all' platform definition to a set of specific platforms for this dependency

        const TLinkedLibrarySettingsPtrMap& libs = allPlatformsModuleInfo->linkerSettings.GetLinkedLibraries();
        TLinkedLibrarySettingsPtrMap::const_iterator l = libs.begin();
        while ( l != libs.end() )
        {
            const CORE::CString& libName = (*l).first;
            const CLinkedLibrarySettingsPtr& libSettings = (*l).second;

            // Check if this dependency is the troublemaker
            const CORE::CString* knownMultiPlatforms = GUCEF_NULL;
            bool isStaticLib = CLinkerSettings::IsStaticLibrary( libName, &knownMultiPlatforms );
            if ( !isStaticLib && !libSettings.IsNULL() )
            {
                isStaticLib = CLinkerSettings::IsStaticLibrary( libSettings->GetLibraryPath(), &knownMultiPlatforms );
            }

            if ( isStaticLib )
            {
                if ( GUCEF_NULL == knownMultiPlatforms )
                {
                    // No hint was provided as to usable alternate platforms
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:ReduceAllPlatformsDefinitionToSustainablePlatforms: Found that module \"" + moduleInfoEntry->GetConsensusName() +
                        "\" has static library linker dependency \"" + libName + "\" at the 'all' platforms level. Could not deduce alternate platforms"  );
                    totalFixSuccess = false;
                }
                else
                {
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:ReduceAllPlatformsDefinitionToSustainablePlatforms: Found that module \"" + moduleInfoEntry->GetConsensusName() +
                        "\" has static library linker dependency \"" + libName + "\" at the 'all' platforms level. Will move the dependency to specific platforms"  );

                    CORE::CString movedLibName = libName;
                    CORE::CStringSet specificPlatforms = ResolveMultiPlatformName( *knownMultiPlatforms, &platforms );

                    CORE::CStringSet::iterator p = specificPlatforms.begin();
                    while ( p != specificPlatforms.end() )
                    {
                        const CORE::CString& platform = (*p);
                        CModuleInfoPtr platformModuleInfo = moduleInfoEntry->FindOrCreateModuleInfoForPlatform( platform, true );

                        totalFixSuccess = platformModuleInfo->linkerSettings.AddLinkedLibraryAsCopy( libName, libSettings ) && totalFixSuccess;

                        ++p;
                    }

                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:ReduceAllPlatformsDefinitionToSustainablePlatforms: Tried to add static library linker dependency \"" + libName +
                        "\" to " + CORE::ToString( specificPlatforms.size() ) + " specific platforms"  );

                    allPlatformsModuleInfo->linkerSettings.DeleteLinkedLibrary( libName );
                    l = libs.begin();
                    continue;
                }
            }

            ++l;
        }
    }

    // Check for problem scenario 2: All platforms definition has dependencies which are not 'all' platforms compatible
    // @TODO

    return totalFixSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::SanitizeRuntimeDependenciesForPlatform( CModuleInfoEntryPtr moduleInfoEntry ,
                                                      const CORE::CString& targetPlatform )
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( moduleInfoEntry.IsNULL() )
        return false;

    const CORE::CString& moduleName = moduleInfoEntry->GetConsensusName();

    CORE::CStringSet runtimeDependencyModuleNames;
    moduleInfoEntry->GetModuleRuntimeDependencyNames( targetPlatform, runtimeDependencyModuleNames );

    CORE::CStringSet::iterator i = runtimeDependencyModuleNames.begin();
    while ( i != runtimeDependencyModuleNames.end() )
    {
        const CORE::CString& dependencyName = (*i);

        // Getting the module for the given entry checks if it exists and if its applicable to the given platform
        CModuleInfoPtr dependencyModulePlatformSpecifics;
        CModuleInfoEntryPtr dependencyModule = GetModuleInfoEntry( dependencyName, targetPlatform, &dependencyModulePlatformSpecifics );
        if ( !dependencyModule.IsNULL() )
        {
            if ( !dependencyModule->IsBroken() )
            {

            }
            else
            {
                // We found a problematic entry
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:SanitizeRuntimeDependencies: Found that module \"" + moduleName +
                    "\" has broken runtime dependency module definition \"" + dependencyName + "\". Will remove the dependency"  );

                moduleInfoEntry->RemoveRuntimeDependency( dependencyName, targetPlatform );
            }
        }
        else
        {
            // We found a problematic entry
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:SanitizeRuntimeDependencies: Found that module \"" + moduleName +
                "\" has missing runtime dependency module definition \"" + dependencyName + "\" for platform " + targetPlatform + ". Will remove the dependency"  );

            moduleInfoEntry->RemoveRuntimeDependency( dependencyName, targetPlatform );

            if ( targetPlatform == KnownPlatforms::AllPlatforms )
            {
                // Just because we cannot satisfy the dependency for 'all' platforms we can still perhaps make it available as an
                // extention for the specific platforms that the runtime dependency supports
                dependencyModule = GetModuleInfoEntry( dependencyName );
                if ( !dependencyModule.IsNULL() && !dependencyModule->IsBroken() )
                {
                    CORE::CStringSet supportedPlatforms;
                    dependencyModule->GetReferencedPlatforms( supportedPlatforms );
                    supportedPlatforms.erase( KnownPlatforms::AllPlatforms );

                    CORE::CStringSet::iterator s = supportedPlatforms.begin();
                    while ( s != supportedPlatforms.end() )
                    {
                        const CORE::CString& supportedPlatform = (*s);
                        CModuleInfoPtr platformSpecificDefinition = moduleInfoEntry->FindOrCreateModuleInfoForPlatform( supportedPlatform, true );
                        if ( !platformSpecificDefinition.IsNULL() )
                        {
                            platformSpecificDefinition->AddNameOfRuntimeDependency( dependencyName );

                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:SanitizeRuntimeDependencies: Added runtime dependency for module \"" + moduleName +
                                "\" to module \"" + dependencyName + "\" for platform " + supportedPlatform + " as substitute for All platforms reference to the same"  );
                        }
                        ++s;
                    }
                }
            }
        }
        ++i;
    }
    return true;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::SanitizeRuntimeDependencies( CModuleInfoEntryPtr moduleInfoEntry )
{GUCEF_TRACE;

    if ( moduleInfoEntry.IsNULL() )
        return false;

    bool totalSuccess = true;

    // Take care of 'All' platform first since that one does allow reducing down
    // to platform specific definitions
    SanitizeRuntimeDependenciesForPlatform( moduleInfoEntry, KnownPlatforms::AllPlatforms );

    CORE::CStringSet referencedPlatforms;
    moduleInfoEntry->GetReferencedPlatforms( referencedPlatforms );
    referencedPlatforms.erase( KnownPlatforms::AllPlatforms );

    CORE::CStringSet::iterator p = referencedPlatforms.begin();
    while ( p != referencedPlatforms.end() )
    {
        const CORE::CString& targetPlatform = (*p);
        totalSuccess = SanitizeRuntimeDependenciesForPlatform( moduleInfoEntry, targetPlatform ) && totalSuccess;
        ++p;
    }
    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::SanitizeRuntimeDependencies( void )
{GUCEF_TRACE;

    bool totalSuccess = true;

    TStringToModuleInfoEntryPtrMap::iterator m = modules.begin();
    while ( m != modules.end() )
    {
        CModuleInfoEntryPtr& moduleInfoEntry = (*m).second;
        if GUCEF_PREDICT_TRUE( !moduleInfoEntry.IsNULL() )
        {
            totalSuccess = SanitizeRuntimeDependencies( moduleInfoEntry ) && totalSuccess;
        }
        ++m;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::SanitizeAllPlatformsUsage( void )
{GUCEF_TRACE;

    TStringToModuleInfoEntryPtrMap::iterator m = modules.begin();
    while ( m != modules.end() )
    {
        CModuleInfoEntryPtr& moduleInfoEntry = (*m).second;
        if GUCEF_PREDICT_TRUE( !moduleInfoEntry.IsNULL() )
        {
            if ( moduleInfoEntry->HasAllPlatformsDefinition() )
            {
                TModuleInfoEntryPtrSet problemModules;
                CORE::CStringSet missingModules;
                if ( !CanModuleSustainAllPlatformsDefinition( moduleInfoEntry, problemModules, missingModules ) )
                {
                    const CORE::CString& moduleName = moduleInfoEntry->GetConsensusName();

                    CORE::CStringSet problemModuleNames; 
                    TModuleInfoEntryPtrSet::iterator p = problemModules.begin();
                    while ( p != problemModules.end() )
                    {
                        const CModuleInfoEntryPtr& problemModule = (*p);
                        if GUCEF_PREDICT_TRUE( !problemModule.IsNULL() )
                        {
                            problemModuleNames.insert( (*p)->GetConsensusName() );
                        }
                        ++p;
                    }

                    // We found a problematic entry
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:SanitizeAllPlatformsUsage: Found that module \"" + moduleName +
                        "\" cannot sustain an 'all' platforms definition due to its dependencies. There are " +
                        CORE::ToString( problemModuleNames.size() ) + " problematic dependencies found \"" + CORE::ToString( problemModuleNames ) +
                        "\" and " + CORE::ToString( missingModules.size() ) + " missing dependencies found \"" + CORE::ToString( missingModules ) + "\""  );

                    if ( missingModules.empty() )
                    {
                        if ( !ReduceAllPlatformsDefinitionToSustainablePlatforms( moduleInfoEntry ) )
                        {
                            // we cannot fix this. The user needs to fix the dependencies or definition
                            moduleInfoEntry->SetIsBroken( true );
                        }
                    }
                    else
                    {
                        // we cannot fix this. The user needs to fix the dependencies or definition
                        moduleInfoEntry->SetIsBroken( true );
                    }
                }
            }
        }
        ++m;
    }
    return true;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::GenerateDependencyChainNodes( const CORE::CString& targetPlatform     ,
                                            bool okToUseCachedValuesWhereApplicable )
{GUCEF_TRACE;

    bool totalSuccess = true;
    TModuleDependencyNodePtrMap& chainsForPlatform = m_moduleDependencyChains[ targetPlatform ];

    // We should not have an empty chains map given that we are iterating over the platforms used by the modules
    // This tells us its not initialized yet, which is fine, we will just create it now
    if ( chainsForPlatform.empty() || !okToUseCachedValuesWhereApplicable )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChains: (re)Building dependency chains for platform " + targetPlatform +
            ". It currently has " + CORE::ToString( chainsForPlatform.size() ) + " dependency chain nodes" );

        TStringToModuleInfoEntryPtrMap::const_iterator m = modules.begin();
        while ( m != modules.end() )
        {
            const CModuleInfoEntryPtr& moduleInfoEntry = (*m).second;
            if GUCEF_PREDICT_TRUE( !moduleInfoEntry.IsNULL() )
            {
                // This dependency chain map is specific to the target platform
                // don't include modules that have no relevance to said platform
                // Also don't include broken module definitions
                if ( moduleInfoEntry->IsApplicableForPlatform( targetPlatform ) && !moduleInfoEntry->IsBroken() )
                {
                    // We will use the consensus name as the key for the chain
                    const CORE::CString& consensusModuleName = moduleInfoEntry->GetConsensusName();

                    // Check for a pre-existing entry
                    // With these chain nodes we have to be careful not to create memory leaks due to the bi-directional
                    // shared ptr linkage
                    CModuleDependencyNodePtr& dependencyChain = chainsForPlatform[ consensusModuleName ];
                    if ( !dependencyChain.IsNULL() )
                    {
                        dependencyChain->Clear();
                    }

                    // Now create the new entry
                    dependencyChain = chainsForPlatform[ consensusModuleName ] = CModuleDependencyNode::CreateSharedObj();
                    if ( !dependencyChain.IsNULL() )
                    {
                        dependencyChain->SetModule( moduleInfoEntry );
                        dependencyChain->SetTargetPlatform( targetPlatform );
                    }
                    else
                    {
                        totalSuccess = false;
                    }
                }
            }
            ++m;
        }

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChains: Platform " + targetPlatform + " has " + CORE::ToString( chainsForPlatform.size() ) + " dependency chain nodes" );
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::UpdateDependencyChains( bool okToUseCachedValuesWhereApplicable )
{GUCEF_TRACE;

    bool totalSuccess = true;

    // we specifically use a platforms list which is sourced from our flat list of modules
    // so that we can ensure that we have all platforms used by the modules
    TStringSet platformList;
    GetAllEnabledPlatformsUsed( platformList, okToUseCachedValuesWhereApplicable );

    // we now access and/or create the map entries for each platform
    TStringSet::const_iterator i = platformList.begin();
    while ( i != platformList.end() )
    {
        const CORE::CString& targetPlatform = (*i);
        totalSuccess = GenerateDependencyChainNodes( targetPlatform, okToUseCachedValuesWhereApplicable ) && totalSuccess;
        ++i;
    }

    // Now that we know for sure that we have a dependency node entry for every relevant
    // module we can use the UpdateDependencyChain() function to do the rest of the work
    i = platformList.begin();
    while ( i != platformList.end() )
    {
        const CORE::CString& targetPlatform = (*i);

        // We do the 'AllPlatforms' platform last so that we can ensure that the chains for the specific platforms
        // are updated first, this way we can ensure that the 'AllPlatforms' chains will have platform specific chains
        // available to them when they are updated.
        if ( targetPlatform != KnownPlatforms::AllPlatforms && !targetPlatform.IsNULLOrEmpty() )
        {
            TModuleDependencyNodePtrMap& chainsForPlatform = m_moduleDependencyChains[ targetPlatform ];

            TModuleDependencyNodePtrMap::iterator n = chainsForPlatform.begin();
            while ( n != chainsForPlatform.end() )
            {            
                const CORE::CString& consensusModuleName = (*n).first;
                CModuleDependencyNodePtr dependencyChain = (*n).second;

                bool chainSuccess = UpdateDependencyChain( dependencyChain );
                if ( !chainSuccess )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:UpdateDependencyChains: Failed to update dependency chain for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                    totalSuccess = false;
                }
                ++n;
            }
        }
        ++i;
    }

    // Now do the 'AllPlatforms' platform
    TModuleDependencyNodePtrMap& chainsForPlatform = m_moduleDependencyChains[ KnownPlatforms::AllPlatforms ];

    TModuleDependencyNodePtrMap::iterator n = chainsForPlatform.begin();
    while ( n != chainsForPlatform.end() )
    {            
        const CORE::CString& consensusModuleName = (*n).first;
        CModuleDependencyNodePtr dependencyChain = (*n).second;

        bool chainSuccess = UpdateDependencyChain( dependencyChain );
        if ( !chainSuccess )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:UpdateDependencyChains: Failed to update dependency chain for module \"" + consensusModuleName + "\" and platform " + KnownPlatforms::AllPlatforms );
            totalSuccess = false;
        }
        ++n;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::CanModuleSustainAllPlatformsDefinition( const CORE::CStringSet& dependencies   ,
                                                      bool okIfNoSuchDependencyDefined       ,
                                                      TModuleInfoEntryPtrSet& problemModules ,
                                                      CORE::CStringSet& missingModules       ) const
{GUCEF_TRACE;

    bool canSustainAllPlatforms = true;

    CORE::CStringSet::const_iterator i = dependencies.begin();
    while ( i != dependencies.end() )
    {
        const CORE::CString& dependencyName = (*i);
        CModuleInfoEntryPtr dependencyModuleEntry = GetModuleInfoEntry( dependencyName );
        if ( !dependencyModuleEntry.IsNULL() )
        {
            if ( !CanModuleSustainAllPlatformsDefinition( dependencyModuleEntry, problemModules, missingModules ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:CanModuleSustainAllPlatformsDefinition: Dependency module \"" + dependencyName + "\" cannot sustain an 'all' platforms definition" );
                canSustainAllPlatforms = false;
            }
        }
        else
        {
            // Ignore static library dependencies as 'missing' here since we check those elsewhere 
            if ( !okIfNoSuchDependencyDefined && !CLinkerSettings::IsStaticLibrary( dependencyName ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:CanModuleSustainAllPlatformsDefinition: Dependency module \"" + dependencyName + "\" cannot be found, no module by that name for ANY platform" );
                canSustainAllPlatforms = false;
                missingModules.insert( dependencyName );
            }
        }
        ++i;
    }

    return canSustainAllPlatforms;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::CanModuleSustainAllPlatformsDefinition( CModuleInfoEntryPtr moduleInfoEntry    ,
                                                      TModuleInfoEntryPtrSet& problemModules ,
                                                      CORE::CStringSet& missingModules       ) const
{GUCEF_TRACE;

    if ( moduleInfoEntry.IsNULL() )
        return false;
    bool canSustainAllPlatforms = true;

    const TModuleInfoPtrMap& modulesPerPlatform = moduleInfoEntry->GetModulesPerPlatform();
    TModuleInfoPtrMap::const_iterator i = modulesPerPlatform.find( KnownPlatforms::AllPlatforms );
    if ( i != modulesPerPlatform.end() )
    {
        const CModuleInfoPtr& allPlatformsDefinition = (*i).second;
        if ( !allPlatformsDefinition.IsNULL() )
        {              
            // Check the build dependencies, used for header include paths and such
            const CORE::CStringSet& dependencies = allPlatformsDefinition->GetNamesOfDependencies();
            if ( !CanModuleSustainAllPlatformsDefinition( dependencies, false, problemModules, missingModules ) )
            {
                canSustainAllPlatforms = false;
            }

            // Check the linker dependencies, used for linking to other known modules and static linking to binaries
            if ( allPlatformsDefinition->linkerSettings.HasAnyStaticLibraryLinkerDependency() )
            {
                // If we have static library dependencies then we cannot sustain an all platforms definition
                // such dependencies are binary blobs which are specific to a platform
                // they should have been specified in platform specific definitions not in the all platforms definition
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:CanModuleSustainAllPlatformsDefinition: Module \"" + moduleInfoEntry->GetConsensusName() +
                    "\" has static library linker dependencies, these cannot be 'all' platforms compatible and as such it cannot sustain an 'all' platforms definition" );
                canSustainAllPlatforms = false;
                problemModules.insert( moduleInfoEntry );
            }

            CORE::CStringSet linkerDependencies;
            allPlatformsDefinition->linkerSettings.GetListOfLinkedLibraries( linkerDependencies );
            if ( !CanModuleSustainAllPlatformsDefinition( linkerDependencies, true, problemModules, missingModules ) )
            {
                canSustainAllPlatforms = false;
            }
        }
        ++i;
    }
    return canSustainAllPlatforms;
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::GetSupportedPlatformsBasedOnDependencies( CModuleInfoEntryPtr moduleInfoEntry ,
                                                        CORE::CStringSet& platforms         ) const
{GUCEF_TRACE;

    if ( moduleInfoEntry.IsNULL() )
        return;

    const TModuleInfoPtrMap& modulesPerPlatform = moduleInfoEntry->GetModulesPerPlatform();
    TModuleInfoPtrMap::const_iterator i = modulesPerPlatform.begin();
    while ( i != modulesPerPlatform.end() )
    {
        const CORE::CString& platformName = (*i).first;
        const CModuleInfoPtr& platformDefinition = (*i).second;

        platformDefinition->GetNamesOfDependencies();

        if ( platformName == KnownPlatforms::AllPlatforms || platformName.IsNULLOrEmpty() )
        {
        }
        else
        {
        }

        ++i;
    }

}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::GetModuleDependencyDeltaAcrossPlatforms( TModuleInfoEntryPtrSet& dependencyDelta  ,
                                                       const CORE::CString& moduleName          ,
                                                       const CORE::CString& basePlatform        ,
                                                       const CORE::CString& deltaPlatform       ,
                                                       bool includeDependenciesOfDependencies   ,
                                                       bool addDependencies                     ,
                                                       bool addLinkerDependencies               ,
                                                       bool addRuntimeDependencies              ) const
{GUCEF_TRACE;

    // This functionality requires dependency chains to be built
    if ( !AreDependencyChainsInitialized() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:GetModuleDependencyDeltaAcrossPlatforms: Dependency chains are not initialized, cannot determine dependency delta" );
        return false;
    }

    CModuleDependencyNodePtr baseTree;
    if ( !TryGetModuleDependencyChain( baseTree, moduleName, basePlatform, true ) || baseTree.IsNULL() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetModuleDependencyDeltaAcrossPlatforms: No dependency chain found for module \"" + moduleName + "\" on platform " + basePlatform );
        return false;
    }

    CModuleDependencyNodePtr otherTree;
    if ( !TryGetModuleDependencyChain( otherTree, moduleName, deltaPlatform, true ) || otherTree.IsNULL() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetModuleDependencyDeltaAcrossPlatforms: No dependency chain found for module \"" + moduleName + "\" on platform " + deltaPlatform );
        return false;
    }

    // Note that the directionality of the delta is important
    // ie it matters which tree you call the function on relative to the other tree
    return otherTree->GetDependencyDelta( dependencyDelta, baseTree, includeDependenciesOfDependencies, addDependencies, addLinkerDependencies, addRuntimeDependencies );        
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::GetAllModuleDependenciesDeltaAcrossPlatforms( TModuleInfoEntryPtrSet& dependencyDelta  ,
                                                            const CORE::CStringSet& moduleNames      ,
                                                            const CORE::CString& basePlatform        ,
                                                            const CORE::CString& deltaPlatform       ,
                                                            bool includeDependenciesOfDependencies   ,
                                                            bool addDependencies                     ,
                                                            bool addLinkerDependencies               ,
                                                            bool addRuntimeDependencies              ) const
{GUCEF_TRACE;

    // This functionality requires dependency chains to be built
    if ( !AreDependencyChainsInitialized() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:GetAllModuleDependenciesDeltaAcrossPlatforms: Dependency chains are not initialized, cannot determine dependency delta" );
        return false;
    }

    TModuleDependencyNodePtrSet baseTreeNodes;
    CORE::CStringSet::const_iterator i = moduleNames.begin();
    while ( i != moduleNames.end() )
    {
        const CORE::CString& moduleName = (*i);

        CModuleDependencyNodePtr baseTree;
        if ( !TryGetModuleDependencyChain( baseTree, moduleName, basePlatform, true ) || baseTree.IsNULL() )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetAllModuleDependenciesDeltaAcrossPlatforms: No dependency chain found for module \"" + moduleName + "\" on platform " + basePlatform );
            return false;
        }

        baseTreeNodes.insert( baseTree );

        ++i;
    }

    TModuleDependencyNodePtrSet otherTreeNodes;
    i = moduleNames.begin();
    while ( i != moduleNames.end() )
    {
        const CORE::CString& moduleName = (*i);

        CModuleDependencyNodePtr otherTree;
        if ( !TryGetModuleDependencyChain( otherTree, moduleName, basePlatform, true ) || otherTree.IsNULL() )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetAllModuleDependenciesDeltaAcrossPlatforms: No dependency chain found for module \"" + moduleName + "\" on platform " + basePlatform );
            return false;
        }

        otherTreeNodes.insert( otherTree );

        ++i;
    }

    // Note that the directionality of the delta is important
    // ie it matters which tree you call the function on relative to the other tree
    return CModuleDependencyNode::GetSetDependencyDelta( dependencyDelta, baseTreeNodes, otherTreeNodes, includeDependenciesOfDependencies, addDependencies, addLinkerDependencies, addRuntimeDependencies );  
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::SanitizeModulePlatformUsage( CModuleInfoEntryPtr moduleInfoEntry ,
                                           const CORE::CString& platform       ,
                                           const TStringSet& dependencies      )
{GUCEF_TRACE;

    if ( dependencies.empty() )
        return true;

    bool passedSanityCheck = true;

    // We have dependencies, we need to ensure that they are also defined for all platforms
    TStringSet::iterator d = dependencies.begin();
    while ( d != dependencies.end() )
    {
        const CORE::CString& dependencyName = (*d);
        CModuleInfoPtr dependencyPlatformModule;
        CModuleInfoEntryPtr dependencyModule = GetModuleInfoEntry( dependencyName, platform, &dependencyPlatformModule );
        if ( dependencyModule.IsNULL() )
        {
            // Dependency does not exist at all, let alone for the specific platform
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:SanitizeModulePlatformUsage: Module \"" + moduleInfoEntry->GetConsensusName() +
                "\" has a dependency on \"" + dependencyName + "\". No such module exists, this is not auto fixable and the module will not be usable. Please change the module specification or supply the dependency" );

            // We cannot fix this
            CModuleDependencyNodePtr dependencyChain;
            if ( TryGetModuleDependencyChain( dependencyChain, moduleInfoEntry->GetConsensusName(), platform, true ) )
            {
                dependencyChain->SetHasMissingDependencies( true );
            }
            passedSanityCheck = false;
        }
        else
        if ( dependencyPlatformModule.IsNULL() )
        {
            if ( platform == KnownPlatforms::AllPlatforms || platform.IsNULLOrEmpty()  )
            {
                // The dependency is not defined for the platform so we will have to downgrade the dependent module to the platform for which the dependency is defined
                CORE::CStringSet supportedPlatforms;
                dependencyModule->GetReferencedPlatforms( supportedPlatforms );
            }

            ++d;
        }
        else
        {
            // we have what we need: No issues
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:SanitizeModulePlatformUsage: Module \"" + moduleInfoEntry->GetConsensusName() +
                "\" has a dependency on \"" + dependencyName + "\". No such module exists" );
        }
    }

    return passedSanityCheck;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::SanitizeModulePlatformUsage( CModuleInfoEntryPtr moduleInfoEntry ,
                                           const CORE::CString& platform       )
{GUCEF_TRACE;

    bool passedSanityCheck = true;
    TStringSet dependencies;

    moduleInfoEntry->GetModuleDependencyNames( platform, dependencies );
    passedSanityCheck = SanitizeModulePlatformUsage( moduleInfoEntry, platform, dependencies ) && passedSanityCheck;
    dependencies.clear();

    moduleInfoEntry->GetModuleLinkerDependencyNames( platform, dependencies );
    passedSanityCheck = SanitizeModulePlatformUsage( moduleInfoEntry, platform, dependencies ) && passedSanityCheck;
    dependencies.clear();

    return passedSanityCheck;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::SanitizeModulePlatformUsage( CModuleInfoEntryPtr moduleInfoEntry )
{GUCEF_TRACE;

    // Things to check:
    //      For AllPlatforms: fixable:
    //          - If present ensure that all the relevant dependencies are also present for all platforms
    //            If not we need to downgrade the module to the platforms for which the dependencies are present
    //      For platform specific: not-fixable (thus error and delete):
    //          - If present ensure that all the relevant dependencies are also present for the platform
    //                  ie win32->win32 or win32->allPlatforms
    //                  not allowed would be: win32->win64 or win32->linux

    CORE::CStringSet platforms;
    moduleInfoEntry->GetReferencedPlatforms( platforms );

    bool passedSanityCheck = true;

    //CORE::CStringSet::const_iterator i = platforms.begin();
    //while ( i != platforms.end() )
    //{
    //    const CORE::CString& platform = (*i);
    //    passedSanityCheck = SanitizeModulePlatformUsage( moduleInfoEntry, platform ) && passedSanityCheck;
    //    ++i;
    //}

    //

    //TStringSet dependencies;

    //moduleInfoEntry->GetModuleDependencyNames( AllPlatforms, dependencies );
    //SanitizeModulePlatformUsage( moduleInfoEntry, dependencies );
    //dependencies.clear();

    //moduleInfoEntry->GetModuleLinkerDependencyNames( AllPlatforms, dependencies );
    //SanitizeModulePlatformUsage( moduleInfoEntry, dependencies );
    //dependencies.clear();


    //const TModuleInfoPtrMap& platformModules = moduleInfoEntry->GetModulesPerPlatform();
    //TModuleInfoPtrMap::const_iterator i = platformModules.find( AllPlatforms );
    //if ( i != platformModules.end() )
    //{
    //    // the module is defined for all platforms, we need to ensure that all dependencies are also defined for all platforms
    //    CModuleInfoPtr module = (*i).second;
    //    if ( module->HasIndependentModuleType() )
    //    {
    //        CORE::CStringMap platformSpecificDependencies;

    //        const TStringSet& dependencies = module->GetNamesOfDependencies();
    //        SanitizeModulePlatformUsage( moduleInfoEntry, dependencies );

    //        dependencies = module->GetNamesOfLinkerDependencies();
    //        SanitizeModulePlatformUsage( moduleInfoEntry, dependencies );
    //    }
    //}

    return passedSanityCheck;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::UpdateDependencyChain( CModuleDependencyNodePtr dependencyChain )
{GUCEF_TRACE;

    bool totalSuccess = true;

    if ( !dependencyChain.IsNULL() )
    {
        const CORE::CString& targetPlatform = dependencyChain->GetTargetPlatform();
        const CModuleInfoEntryPtr& moduleInfoEntry = dependencyChain->GetModule();

        if ( !moduleInfoEntry.IsNULL() )
        {
            const CORE::CString& consensusModuleName = moduleInfoEntry->GetConsensusName();

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChain: Updating chain node for module \"" + consensusModuleName + "\" for target platform " + targetPlatform );

            TStringSet dependencies;
            moduleInfoEntry->GetModuleDependencyNames( targetPlatform, dependencies );

            TStringSet::iterator d = dependencies.begin();
            while ( d != dependencies.end() )
            {
                const CORE::CString& dependencyName = (*d);
                CModuleDependencyNodePtr dependencyModule;
                if ( TryGetModuleDependencyChain( dependencyModule, dependencyName, targetPlatform, false ) && !dependencyModule.IsNULL() )
                {
                    totalSuccess = dependencyChain->SetDependency( dependencyModule ) && totalSuccess;
                }
                else
                {
                    CModuleInfoEntryPtr dependencyModuleInfoEntry = GetModuleInfoEntry( dependencyName );
                    if ( !dependencyModuleInfoEntry.IsNULL() )
                    {
                        if ( dependencyModuleInfoEntry->HasIndependentModuleTypeForPlatform( targetPlatform ) )
                        {
                            if ( !dependencyModuleInfoEntry->IsBroken() )
                            {
                                bool issueWasFixed = false;
                                if ( SanitizeModulePlatformUsage( moduleInfoEntry ) )
                                {
                                    // try again now that we have sanitized the module
                                    if ( TryGetModuleDependencyChain( dependencyModule, dependencyName, targetPlatform, false ) && !dependencyModule.IsNULL() )
                                    {
                                        issueWasFixed = dependencyChain->SetDependency( dependencyModule );
                                        totalSuccess = issueWasFixed && totalSuccess;
                                    }
                                }
                                if ( !issueWasFixed )
                                {
                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:UpdateDependencyChain: Failed to obtain actual module for dependency name: \"" +
                                        dependencyName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                                    totalSuccess = false;
                                }
                            }
                            else
                            {
                                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChain: Ignoring broken dependency with name: \"" +
                                    dependencyName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                            }
                        }
                    }
                }
                ++d;
            }

            dependencies.clear();
            moduleInfoEntry->GetLogicalDependencyNames( targetPlatform, dependencies );

            d = dependencies.begin();
            while ( d != dependencies.end() )
            {
                const CORE::CString& dependencyName = (*d);
                CModuleDependencyNodePtr dependencyModule;
                if ( TryGetModuleDependencyChain( dependencyModule, dependencyName, targetPlatform, false ) && !dependencyModule.IsNULL() )
                {
                    totalSuccess = dependencyChain->SetLogicalDependency( dependencyModule ) && totalSuccess;
                }
                else
                {
                    CModuleInfoEntryPtr dependencyModuleInfoEntry = GetModuleInfoEntry( dependencyName );
                    if ( !dependencyModuleInfoEntry.IsNULL() )
                    {
                        if ( dependencyModuleInfoEntry->HasIndependentModuleTypeForPlatform( targetPlatform ) )
                        {
                            if ( !dependencyModuleInfoEntry->IsBroken() )
                            {
                                bool issueWasFixed = false;
                                if ( SanitizeModulePlatformUsage( moduleInfoEntry ) )
                                {
                                    // try again now that we have sanitized the module
                                    if ( TryGetModuleDependencyChain( dependencyModule, dependencyName, targetPlatform, false ) && !dependencyModule.IsNULL() )
                                    {
                                        issueWasFixed = dependencyChain->SetLogicalDependency( dependencyModule );
                                        totalSuccess = issueWasFixed && totalSuccess;
                                    }
                                }
                                if ( !issueWasFixed )
                                {
                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:UpdateDependencyChain: Failed to obtain actual module for logical dependency name: \"" +
                                        dependencyName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                                    totalSuccess = false;
                                }
                            }
                            else
                            {
                                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChain: Ignoring broken dependency with name: \"" +
                                    dependencyName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                            }  
                        }
                    }
                }
                ++d;
            }

            dependencies.clear();
            moduleInfoEntry->GetModuleLinkerDependencyNames( targetPlatform, dependencies );

            d = dependencies.begin();
            while ( d != dependencies.end() )
            {
                const CORE::CString& dependencyName = (*d);
                CModuleDependencyNodePtr dependencyModule;
                if ( TryGetModuleDependencyChain( dependencyModule, dependencyName, targetPlatform, false ) && !dependencyModule.IsNULL() )
                {
                    totalSuccess = dependencyChain->SetLinkerDependency( dependencyModule ) && totalSuccess;
                }
                else
                {
                    CModuleInfoEntryPtr dependencyModuleInfoEntry = GetModuleInfoEntry( dependencyName );
                    if ( !dependencyModuleInfoEntry.IsNULL() )
                    {
                        if ( dependencyModuleInfoEntry->HasIndependentModuleTypeForPlatform( targetPlatform ) )
                        {
                            if ( !dependencyModuleInfoEntry->IsBroken() )
                            {
                                bool issueWasFixed = false;
                                if ( SanitizeModulePlatformUsage( moduleInfoEntry ) )
                                {
                                    // try again now that we have sanitized the module
                                    if ( TryGetModuleDependencyChain( dependencyModule, dependencyName, targetPlatform, false ) && !dependencyModule.IsNULL() )
                                    {
                                        issueWasFixed = dependencyChain->SetDependency( dependencyModule );
                                        totalSuccess = issueWasFixed && totalSuccess;
                                    }
                                }
                                if ( !issueWasFixed )
                                {
                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:UpdateDependencyChain: Failed to obtain actual module for linker dependency name: \"" +
                                        dependencyName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                                    totalSuccess = false;
                                }
                            }
                            else
                            {
                                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChain: Ignoring broken dependency with name: \"" +
                                    dependencyName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                            }  
                        }
                    }
                }
                ++d;
            }

            dependencies.clear();
            moduleInfoEntry->GetModuleRuntimeDependencyNames( targetPlatform, dependencies );

            d = dependencies.begin();
            while ( d != dependencies.end() )
            {
                const CORE::CString& dependencyName = (*d);
                CModuleDependencyNodePtr dependencyModule;
                if ( TryGetModuleDependencyChain( dependencyModule, dependencyName, targetPlatform, false ) && !dependencyModule.IsNULL() )
                {
                    totalSuccess = dependencyChain->SetRuntimeDependency( dependencyModule ) && totalSuccess;
                }
                else
                {
                    CModuleInfoEntryPtr dependencyModuleInfoEntry = GetModuleInfoEntry( dependencyName );
                    if ( !dependencyModuleInfoEntry.IsNULL() )
                    {
                        if ( dependencyModuleInfoEntry->HasIndependentModuleTypeForPlatform( targetPlatform ) )
                        {
                            if ( !dependencyModuleInfoEntry->IsBroken() )
                            {
                                bool issueWasFixed = false;
                                if ( SanitizeModulePlatformUsage( moduleInfoEntry ) )
                                {
                                    // try again now that we have sanitized the module
                                    if ( TryGetModuleDependencyChain( dependencyModule, dependencyName, targetPlatform, false ) && !dependencyModule.IsNULL() )
                                    {
                                        issueWasFixed = dependencyChain->SetRuntimeDependency( dependencyModule );
                                        totalSuccess = issueWasFixed && totalSuccess;
                                    }
                                }
                                if ( !issueWasFixed )
                                {
                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:UpdateDependencyChain: Failed to obtain actual module for runtime dependency name: \"" +
                                        dependencyName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                                    totalSuccess = false;
                                }
                            }
                            else
                            {
                                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChain: Ignoring broken dependency with name: \"" +
                                    dependencyName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                            }                            
                        }
                    }
                }
                ++d;
            }

            dependencies.clear();
            GetNamesOfModulesWhichDependOnModuleForPlatform( targetPlatform, consensusModuleName, dependencies, false );

            d = dependencies.begin();
            while ( d != dependencies.end() )
            {
                const CORE::CString& dependentName = (*d);
                CModuleDependencyNodePtr dependentModule;
                if ( TryGetModuleDependencyChain( dependentModule, dependentName, targetPlatform, false ) && !dependentModule.IsNULL() )
                {
                    totalSuccess = dependencyChain->SetDependent( dependentModule ) && totalSuccess;
                }
                else
                {
                    CModuleInfoEntryPtr dependencyModuleInfoEntry = GetModuleInfoEntry( dependentName );
                    if ( !dependencyModuleInfoEntry.IsNULL() )
                    {
                        if ( dependencyModuleInfoEntry->HasIndependentModuleTypeForPlatform( targetPlatform ) )
                        {
                            if ( !dependencyModuleInfoEntry->IsBroken() )
                            {
                                bool issueWasFixed = false;
                                if ( SanitizeModulePlatformUsage( moduleInfoEntry ) )
                                {
                                    // try again now that we have sanitized the module
                                    if ( TryGetModuleDependencyChain( dependentModule, dependentName, targetPlatform, false ) && !dependentModule.IsNULL() )
                                    {
                                        issueWasFixed = dependencyChain->SetDependent( dependentModule );
                                        totalSuccess = issueWasFixed && totalSuccess;
                                    }
                                }
                                if ( !issueWasFixed )
                                {
                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:UpdateDependencyChain: Failed to obtain actual module for dependent name: \"" +
                                        dependentName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                                    totalSuccess = false;
                                }
                            }
                            else
                            {
                                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChain: Ignoring broken dependent with name: \"" +
                                    dependentName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                            }
                        }
                    }
                }
                ++d;
            }

            dependencies.clear();
            GetNamesOfModulesWhichLinkerDependOnModuleForPlatform( targetPlatform, consensusModuleName, dependencies, false );

            d = dependencies.begin();
            while ( d != dependencies.end() )
            {
                const CORE::CString& dependentName = (*d);
                CModuleDependencyNodePtr dependentModule;
                if ( TryGetModuleDependencyChain( dependentModule, dependentName, targetPlatform, false ) && !dependentModule.IsNULL() )
                {
                    totalSuccess = dependencyChain->SetLinkerDependent( dependentModule ) && totalSuccess;
                }
                else
                {
                    CModuleInfoEntryPtr dependencyModuleInfoEntry = GetModuleInfoEntry( dependentName );
                    if ( !dependencyModuleInfoEntry.IsNULL() )
                    {
                        if ( dependencyModuleInfoEntry->HasIndependentModuleTypeForPlatform( targetPlatform ) )
                        {
                            if ( !dependencyModuleInfoEntry->IsBroken() )
                            {
                                bool issueWasFixed = false;
                                if ( SanitizeModulePlatformUsage( moduleInfoEntry ) )
                                {
                                    // try again now that we have sanitized the module
                                    if ( TryGetModuleDependencyChain( dependentModule, dependentName, targetPlatform, false ) && !dependentModule.IsNULL() )
                                    {
                                        issueWasFixed = dependencyChain->SetLinkerDependent( dependentModule );
                                        totalSuccess = issueWasFixed && totalSuccess;
                                    }
                                }
                                if ( !issueWasFixed )
                                {
                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:UpdateDependencyChain: Failed to obtain actual module for linker dependent name: \"" +
                                        dependentName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                                    totalSuccess = false;
                                }
                            }
                            else
                            {
                                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChain: Ignoring broken linker dependent with name: \"" +
                                    dependentName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                            }                                
                        }
                    }
                }
                ++d;
            }

            dependencies.clear();
            GetNamesOfModulesWhichRuntimeDependOnModuleForPlatform( targetPlatform, consensusModuleName, dependencies, false );

            d = dependencies.begin();
            while ( d != dependencies.end() )
            {
                const CORE::CString& dependentName = (*d);
                CModuleDependencyNodePtr dependentModule;
                if ( TryGetModuleDependencyChain( dependentModule, dependentName, targetPlatform, false ) && !dependentModule.IsNULL() )
                {
                    totalSuccess = dependencyChain->SetRuntimeDependent( dependentModule ) && totalSuccess;
                }
                else
                {
                    CModuleInfoEntryPtr dependencyModuleInfoEntry = GetModuleInfoEntry( dependentName );
                    if ( !dependencyModuleInfoEntry.IsNULL() )
                    {
                        if ( dependencyModuleInfoEntry->HasIndependentModuleTypeForPlatform( targetPlatform ) )
                        {
                            if ( !dependencyModuleInfoEntry->IsBroken() )
                            {
                                bool issueWasFixed = false;
                                if ( SanitizeModulePlatformUsage( moduleInfoEntry ) )
                                {
                                    // try again now that we have sanitized the module
                                    if ( TryGetModuleDependencyChain( dependentModule, dependentName, targetPlatform, false ) && !dependentModule.IsNULL() )
                                    {
                                        issueWasFixed = dependencyChain->SetRuntimeDependent( dependentModule );
                                        totalSuccess = issueWasFixed && totalSuccess;
                                    }
                                }
                                if ( !issueWasFixed )
                                {
                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:UpdateDependencyChain: Failed to obtain actual module for runtime dependent name: \"" +
                                        dependentName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                                    totalSuccess = false;
                                }
                            }
                            else
                            {
                                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChain: Ignoring broken runtime dependent with name: \"" +
                                    dependentName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                            }
                        }
                    }
                }
                ++d;
            }

            dependencies.clear();
            GetNamesOfModulesWhichLogicallyDependOnModuleForPlatform( targetPlatform, consensusModuleName, dependencies, false );

            d = dependencies.begin();
            while ( d != dependencies.end() )
            {
                const CORE::CString& dependentName = (*d);
                CModuleDependencyNodePtr dependentModule;
                if ( TryGetModuleDependencyChain( dependentModule, dependentName, targetPlatform, false ) && !dependentModule.IsNULL() )
                {
                    totalSuccess = dependencyChain->SetLogicalDependent( dependentModule ) && totalSuccess;
                }
                else
                {
                    CModuleInfoEntryPtr dependencyModuleInfoEntry = GetModuleInfoEntry( dependentName );
                    if ( !dependencyModuleInfoEntry.IsNULL() )
                    {
                        if ( dependencyModuleInfoEntry->HasIndependentModuleTypeForPlatform( targetPlatform ) )
                        {
                            if ( !dependencyModuleInfoEntry->IsBroken() )
                            {
                                bool issueWasFixed = false;
                                if ( SanitizeModulePlatformUsage( moduleInfoEntry ) )
                                {
                                    // try again now that we have sanitized the module
                                    if ( TryGetModuleDependencyChain( dependentModule, dependentName, targetPlatform, false ) && !dependentModule.IsNULL() )
                                    {
                                        issueWasFixed = dependencyChain->SetLogicalDependent( dependentModule );
                                        totalSuccess = issueWasFixed && totalSuccess;
                                    }
                                }
                                if ( !issueWasFixed )
                                {
                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ProjectInfo:UpdateDependencyChain: Failed to obtain actual module for logical dependent name: \"" +
                                        dependentName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                                    totalSuccess = false;
                                }
                            }
                            else
                            {
                                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:UpdateDependencyChain: Ignoring broken logical dependent with name: \"" +
                                    dependentName + "\" for module \"" + consensusModuleName + "\" and platform " + targetPlatform );
                            }
                        }
                    }
                }
                ++d;
            }
        }
        else
        {
            totalSuccess = false;
        }
    }
    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::TryGetModuleDependencyChain( CModuleDependencyNodePtr& dependencyChain ,
                                           const CORE::CString& consensusModuleName  ,
                                           const CORE::CString& targetPlatform       ,
                                           bool onlyCheckPlatformSpecific            ) const
{GUCEF_TRACE;

    dependencyChain.Unlink();

    TStringToModuleDependencyNodePtrMap::const_iterator i = m_moduleDependencyChains.find( targetPlatform );
    if ( i != m_moduleDependencyChains.end() )
    {
        // we have chains for this platform, now look for the specific module
        const TModuleDependencyNodePtrMap& chainsForPlatform = (*i).second;
        TModuleDependencyNodePtrMap::const_iterator n = chainsForPlatform.find( consensusModuleName );
        if ( n != chainsForPlatform.end() )
        {
            // We found a chain for this platform & module consensus name combo
            dependencyChain = (*n).second;
            return true;
        }
    }

    if ( targetPlatform != KnownPlatforms::AllPlatforms && !onlyCheckPlatformSpecific )
    {
        TStringToModuleDependencyNodePtrMap::const_iterator i = m_moduleDependencyChains.find( KnownPlatforms::AllPlatforms );
        if ( i != m_moduleDependencyChains.end() )
        {
            // we have chains for this platform, now look for the specific module
            const TModuleDependencyNodePtrMap& chainsForPlatform = (*i).second;
            TModuleDependencyNodePtrMap::const_iterator n = chainsForPlatform.find( consensusModuleName );
            if ( n != chainsForPlatform.end() )
            {
                // We found a chain for this platform & module consensus name combo
                dependencyChain = (*n).second;
                return true;
            }
        }        
    }

    return false;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::TryGetDependencyChainsForModules( TModuleDependencyNodePtrSet& dependencyChains ,
                                                const CORE::CStringSet& moduleNames           ,
                                                const CORE::CString& targetPlatform           ,
                                                bool onlyCheckPlatformSpecific                ) const
{GUCEF_TRACE;

    
    return false;
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::ClearDependencyChains( void )
{GUCEF_TRACE;

    TStringToModuleDependencyNodePtrMap::iterator i = m_moduleDependencyChains.begin();
    while ( i != m_moduleDependencyChains.end() )
    {
        TModuleDependencyNodePtrMap& map = (*i).second;
        TModuleDependencyNodePtrMap::iterator n = map.begin();
        while ( n != map.end() )
        {
            CModuleDependencyNodePtr& dependency = (*n).second;
            if ( !dependency.IsNULL() )
            {
                dependency->Clear();
            }
            ++n;
        }

        ++i;
    }
    m_moduleDependencyChains.clear();
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::Clear( void )
{GUCEF_TRACE;

    m_projectName.Clear();
    rootDirs.clear();
    modules.clear();
    dirProcessingInstructions.clear();
    globalDirExcludeList.clear();
    platforms.clear();
    ClearDependencyChains();
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::AreDependencyChainsInitialized( void ) const
{GUCEF_TRACE;

    return !m_actualPlatformsUsed.empty() && ( ( m_moduleDependencyChains.size() + m_disabledPlatforms.size() ) == m_actualPlatformsUsed.size() );
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::DetermineBuildOrderForAllModules( void )
{GUCEF_TRACE;

    // In order for this functionality to work we need the dependency chains determined ahead of time
    if ( !AreDependencyChainsInitialized() )
    {
        UpdateDependencyChains();
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DetermineBuildOrderForAllModules: Start determining build order for every platform" );

    // Important: First determine the build order which applies to all modules
    // This build order will be used for specific platforms as well unless no 'AllPlatforms'
    // target is available or if the build order differs
    // Doing it this way cuts down on the number platform specific entries generated just to store
    // the build order
    DetermineBuildOrderForAllModulesForPlatform( KnownPlatforms::AllPlatforms );

    TStringToModuleDependencyNodePtrMap::iterator i = m_moduleDependencyChains.begin();
    while ( i != m_moduleDependencyChains.end() )
    {
        const CORE::CString& platformName = (*i).first;
        if ( platformName != KnownPlatforms::AllPlatforms )
        {
            DetermineBuildOrderForAllModulesForPlatform( platformName );
        }
        ++i;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DetermineBuildOrderForAllModules: Finished determining build order for every platform" );
    return true;
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::DetermineBuildOrderForAllModulesForPlatform( const CORE::CString& targetPlatform )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DetermineBuildOrderForAllModules: Start determining build order for platform " + targetPlatform );

    // Get the initial build leaf nodes
    // these are nodes which are truly leaves in that things may depend on them but they have no dependencies of their own

    TModuleDependencyNodePtrMap currentLevelNodes;
    GetBuildLeafNodes( targetPlatform, currentLevelNodes );

    Int32 buildIndex = 1;
    TStringToInt32Map buildOrderMap;

    // We take advantage of map alphabetic ordering
    TModuleDependencyNodePtrMap::iterator n = currentLevelNodes.begin();
    while ( n != currentLevelNodes.end() )
    {
        const CORE::CString& consensusName = (*n).first;
        buildOrderMap[ consensusName ] = buildIndex;

        ++buildIndex;
        ++n;
    }

    TModuleDependencyNodePtrMap nextLevelNodesMissingDeps;    
    do
    {
        // Now do a pass building the dependency pyramids for the current set of leaf nodes
        // As we do this more and more of the nodes with unsatisfied dependencies get their
        // dependencies fulfilled 

        TModuleDependencyNodePtrMap nextLevelNodes;
        do
        {
            nextLevelNodes.clear();
            GetNextLevelUpBuildNodes( targetPlatform, currentLevelNodes, buildOrderMap, nextLevelNodes, nextLevelNodesMissingDeps );

            n = nextLevelNodes.begin();
            while ( n != nextLevelNodes.end() )
            {
                const CORE::CString& consensusName = (*n).first;

                // Check if we haven't already assigned a build order
                // The same module can occur at different depth in the overall tree
                if ( buildOrderMap.find( consensusName ) == buildOrderMap.end() )
                {
                    buildOrderMap[ consensusName ] = buildIndex;
                    ++buildIndex;
                    nextLevelNodesMissingDeps.erase( consensusName );
                }

                ++n;
            }

            currentLevelNodes = nextLevelNodes;
        }
        while ( !nextLevelNodes.empty() );

        if ( !nextLevelNodesMissingDeps.empty() )
        {
            // We have gone as high as we can without missing dependencies
            // Now we reset based on whatever was left over and determine new leaf nodes
            
            GetUnprocessedBuildLeafNodes( buildOrderMap, nextLevelNodesMissingDeps, currentLevelNodes );

            TModuleDependencyNodePtrMap::iterator n = currentLevelNodes.begin();
            while ( n != currentLevelNodes.end() )
            {
                const CORE::CString& consensusName = (*n).first;

                // Check if we haven't already assigned a build order
                // The same module can occur at different depth in the overall tree
                if ( buildOrderMap.find( consensusName ) == buildOrderMap.end() )
                {
                    buildOrderMap[ consensusName ] = buildIndex;                    
                    ++buildIndex;
                    nextLevelNodesMissingDeps.erase( consensusName );
                }
                ++n;
            }
        }
    }
    while ( !currentLevelNodes.empty() );

    // Apply the build order information to the modules
    // Note that we put in extra effort here to avoid defining a platform specific
    // module definition just for the the sake of preserving the build order
    // if it matches the 'all' platform we just use that
    TStringToInt32Map::iterator o = buildOrderMap.begin();
    while ( o != buildOrderMap.end() )
    {
        const CORE::CString& consensusName = (*o).first;
        Int32 buildIndex = (*o).second; 

        CModuleInfoEntryPtr moduleEntry = GetModuleInfoEntry( consensusName );
        if GUCEF_PREDICT_TRUE( !moduleEntry.IsNULL() )
        {
            bool platformSpecificBuildOrderMatchesAllPlatforms = false;
            CModuleInfoPtr allPlatformsInfo = moduleEntry->FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms );
            if ( !allPlatformsInfo.IsNULL() )
            {
                if ( targetPlatform == KnownPlatforms::AllPlatforms )
                {
                    allPlatformsInfo->buildOrder = buildIndex;
                    platformSpecificBuildOrderMatchesAllPlatforms = true;
                }
                else
                {
                    if ( -1 != allPlatformsInfo->buildOrder && allPlatformsInfo->buildOrder == buildIndex )
                    {
                        platformSpecificBuildOrderMatchesAllPlatforms = true;
                    }
                }                    
            }

            if ( !platformSpecificBuildOrderMatchesAllPlatforms )
            {
                CModuleInfoPtr moduleSpecForPlatform = moduleEntry->FindOrCreateModuleInfoForPlatform( targetPlatform, true );
                if GUCEF_PREDICT_TRUE( !moduleSpecForPlatform.IsNULL() )
                {
                    moduleSpecForPlatform->buildOrder = (*o).second;
                }                
            }
        }
        ++o;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DetermineBuildOrderForAllModules: Finished determining build order for platform " + targetPlatform );
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::GetUnprocessedBuildLeafNodes( const TStringToInt32Map& buildOrderMap                      ,
                                            const TModuleDependencyNodePtrMap& nodesThatWereMissingDeps ,
                                            TModuleDependencyNodePtrMap& newLeafNodes                   )
{GUCEF_TRACE;

    TModuleDependencyNodePtrMap::const_iterator i = nodesThatWereMissingDeps.begin();
    while ( i != nodesThatWereMissingDeps.end() )
    {
        const CORE::CString& consensusName = (*i).first;
        CModuleDependencyNodePtr dependent = (*i).second;

        if GUCEF_PREDICT_TRUE( !dependent.IsNULL() )
        {
            // The new leaf nodes are not modules with 0 linker deps but rather those with 0 remaining
            // unprocessed dependencies

            bool hasUnprocessedDependency = false;
            const TModuleDependencyNodePtrMap& dependentDependencies = dependent->GetLinkerDependencies();
            TModuleDependencyNodePtrMap::const_iterator d = dependentDependencies.begin();
            while ( d != dependentDependencies.end() )
            {
                const CORE::CString& dependencyName = (*d).first;
                if ( buildOrderMap.find( dependencyName ) == buildOrderMap.end() )
                {
                    hasUnprocessedDependency = true;
                    break;
                }

                ++d;
            }

            if ( !hasUnprocessedDependency )
            {
                newLeafNodes[ consensusName ] = dependent;
            }
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::GetNextLevelUpBuildNodes( const CORE::CString& platform                          ,
                                        const TModuleDependencyNodePtrMap& currentLevel        ,
                                        const TStringToInt32Map& buildOrderMap                 ,
                                        TModuleDependencyNodePtrMap& nextLevelNodes            ,
                                        TModuleDependencyNodePtrMap& nextLevelNodesMissingDeps )
{GUCEF_TRACE;

    TModuleDependencyNodePtrMap::const_iterator n = currentLevel.begin();
    while ( n != currentLevel.end() )
    {
        const CORE::CString& consensusName = (*n).first;
        const CModuleDependencyNodePtr& chainNode = (*n).second;
        if GUCEF_PREDICT_TRUE( !chainNode.IsNULL() )
        {
            const TModuleDependencyNodePtrMap& dependents = chainNode->GetLinkerDependents();
            TModuleDependencyNodePtrMap::const_iterator i = dependents.begin();
            while ( i != dependents.end() )
            {
                CModuleDependencyNodePtr dependent = (*i).second;
                if GUCEF_PREDICT_TRUE( !dependent.IsNULL() )
                {
                    bool hasUnprocessedDependency = false;
                    const TModuleDependencyNodePtrMap& dependentDependencies = dependent->GetLinkerDependencies();
                    TModuleDependencyNodePtrMap::const_iterator d = dependentDependencies.begin();
                    while ( d != dependentDependencies.end() )
                    {
                        const CORE::CString& dependencyName = (*d).first;
                        if ( buildOrderMap.find( dependencyName ) == buildOrderMap.end() )
                        {
                            hasUnprocessedDependency = true;
                            break;
                        }

                        ++d;
                    }

                    if ( hasUnprocessedDependency )
                    {
                        nextLevelNodesMissingDeps[ dependent->GetConsensusName() ] = dependent;
                        nextLevelNodes.erase( dependent->GetConsensusName() );
                    }
                    else
                    {
                        nextLevelNodes[ dependent->GetConsensusName() ] = dependent;
                        nextLevelNodesMissingDeps.erase( dependent->GetConsensusName() );
                    }
                }
                ++i;
            }
        }
        ++n;
    }
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::GetBuildLeafNodes( const CORE::CString& platform          ,
                                 TModuleDependencyNodePtrMap& leafNodes )
{GUCEF_TRACE;

    TStringToModuleDependencyNodePtrMap::iterator i = m_moduleDependencyChains.find( platform );
    if ( i != m_moduleDependencyChains.end() )
    {
        TModuleDependencyNodePtrMap& chainNodeMap = (*i).second;
        TModuleDependencyNodePtrMap::iterator n = chainNodeMap.begin();
        while ( n != chainNodeMap.end() )
        {
            CModuleDependencyNodePtr& chainNode = (*n).second;
            if GUCEF_PREDICT_TRUE( !chainNode.IsNULL() )
            {
                if ( chainNode->GetLinkerDependencies().empty() )
                {
                    CModuleInfoEntryPtr module = chainNode->GetModule();
                    if GUCEF_PREDICT_TRUE( !module.IsNULL() )
                    {
                        const CORE::CString& consensusName = module->GetConsensusName();
                        leafNodes[ consensusName ] = chainNode;
                    }
                }
            }
            ++n;
        }
    }
}
/*-------------------------------------------------------------------------*/

void
CProjectInfo::GetNamesOfModulesWhichDependOnModuleForPlatform( const CORE::CString& targetPlatform ,
                                                               const CORE::CString& dependencyName ,
                                                               CORE::CStringSet& moduleNames       ,
                                                               bool platformSpecificOnly           ) const
{GUCEF_TRACE;

    // Loop trough all modules and check if they depend on the given module
    TMutableModuleInfoEntryPairVector resultList;
    TStringToModuleInfoEntryPtrMap::const_iterator i = modules.begin();
    while ( i != modules.end() )
    {
        const CModuleInfoEntryPtr& moduleInfoEntry = (*i).second;
        if ( !moduleInfoEntry.IsNULL() )
        {
            if ( moduleInfoEntry->HasDependency( targetPlatform, dependencyName, platformSpecificOnly ) )
            {
                moduleNames.insert( moduleInfoEntry->GetConsensusName() );
            }
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CProjectInfo::GetNamesOfModulesWhichLinkerDependOnModuleForPlatform( const CORE::CString& targetPlatform ,
                                                                     const CORE::CString& dependencyName ,
                                                                     CORE::CStringSet& moduleNames       ,
                                                                     bool platformSpecificOnly           ) const
{GUCEF_TRACE;

    // Loop trough all modules and check if they depend on the given module
    TMutableModuleInfoEntryPairVector resultList;
    TStringToModuleInfoEntryPtrMap::const_iterator i = modules.begin();
    while ( i != modules.end() )
    {
        const CModuleInfoEntryPtr& moduleInfoEntry = (*i).second;
        if ( !moduleInfoEntry.IsNULL() )
        {
            if ( moduleInfoEntry->HasLinkerDependency( targetPlatform, dependencyName, platformSpecificOnly ) )
            {
                moduleNames.insert( moduleInfoEntry->GetConsensusName() );
            }
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CProjectInfo::GetNamesOfModulesWhichRuntimeDependOnModuleForPlatform( const CORE::CString& targetPlatform ,
                                                                      const CORE::CString& dependencyName ,
                                                                      CORE::CStringSet& moduleNames       ,
                                                                      bool platformSpecificOnly           ) const
{GUCEF_TRACE;

    // Loop trough all modules and check if they depend on the given module
    TMutableModuleInfoEntryPairVector resultList;
    TStringToModuleInfoEntryPtrMap::const_iterator i = modules.begin();
    while ( i != modules.end() )
    {
        const CModuleInfoEntryPtr& moduleInfoEntry = (*i).second;
        if ( !moduleInfoEntry.IsNULL() )
        {
            if ( moduleInfoEntry->HasDependency( targetPlatform, dependencyName, platformSpecificOnly ) )
            {
                moduleNames.insert( moduleInfoEntry->GetConsensusName() );
            }
            else
            if ( moduleInfoEntry->HasLinkerDependency( targetPlatform, dependencyName, platformSpecificOnly ) )
            {
                moduleNames.insert( moduleInfoEntry->GetConsensusName() );
            }
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CProjectInfo::GetNamesOfModulesWhichLogicallyDependOnModuleForPlatform( const CORE::CString& targetPlatform ,
                                                                        const CORE::CString& dependencyName ,
                                                                        CORE::CStringSet& moduleNames       ,
                                                                        bool platformSpecificOnly           ) const
{GUCEF_TRACE;

    // Loop trough all modules and check if they logically depend on the given module
    TMutableModuleInfoEntryPairVector resultList;
    TStringToModuleInfoEntryPtrMap::const_iterator i = modules.begin();
    while ( i != modules.end() )
    {
        const CModuleInfoEntryPtr& moduleInfoEntry = (*i).second;
        if ( !moduleInfoEntry.IsNULL() )
        {
            if ( moduleInfoEntry->HasLogicalDependency( targetPlatform, dependencyName, platformSpecificOnly ) )
            {
                moduleNames.insert( moduleInfoEntry->GetConsensusName() );
            }
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CProjectInfo::GetModuleDependencies( const CModuleInfoEntryPtr& moduleInfoEntry ,
                                     const CORE::CString& targetPlatform        ,
                                     TModuleInfoEntryPtrSet& dependencies       ,
                                     bool includeDependenciesOfDependencies     ,
                                     bool includeRuntimeDependencies            ,
                                     bool includeLogicalDependencies            ) const
{GUCEF_TRACE;

    // In order for this functionality to work we need the dependency chains determined ahead of time
    if ( !AreDependencyChainsInitialized() )
    {
        // We are no longer supporting doing this without dependency chains, it just takes too long
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetModuleDependencies: Determine the dependency chains before using this functionality" );
        return false;
    }

    const CORE::CString& moduleName = moduleInfoEntry->GetConsensusName();

    CModuleDependencyNodePtr dependencyChain;
    if ( TryGetModuleDependencyChain( dependencyChain  ,
                                      moduleName       ,
                                      targetPlatform   ,
                                      true             ) && !dependencyChain.IsNULL() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:GetModuleDependencies: Found dependency chain for module " + moduleName + " for platform " + targetPlatform );

        bool totalSuccess = true;

        totalSuccess = dependencyChain->GatherDependencyModules( dependencies, includeDependenciesOfDependencies ) && totalSuccess;
        totalSuccess = dependencyChain->GatherLinkerDependencyModules( dependencies, includeDependenciesOfDependencies ) && totalSuccess;
        if ( includeRuntimeDependencies )
            totalSuccess = dependencyChain->GatherRuntimeDependencyModules( dependencies, includeDependenciesOfDependencies ) && totalSuccess;
        if ( includeLogicalDependencies )
            totalSuccess = dependencyChain->GatherLogicalDependencyModules( dependencies, includeDependenciesOfDependencies ) && totalSuccess;

        return totalSuccess;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CProjectInfo::GetModuleDependencies( const CModuleInfoEntryPtr& moduleInfoEntry ,
                                     const CORE::CString& targetPlatform        ,
                                     TStringSet& dependencies                   ,
                                     bool includeRuntimeDependencies            ,
                                     bool includeLogicalDependencies            ) const
{GUCEF_TRACE;

    CModuleInfoEntryPtr mutableModuleInfoEntry = moduleInfoEntry;
    CModuleInfoPtr moduleInfo = mutableModuleInfoEntry->FindOrCreateModuleInfoForPlatform( targetPlatform, false );
    if ( !moduleInfo.IsNULL() )
    {
        MergeStringSet( dependencies, moduleInfo->GetNamesOfDependencies(), false );
        if ( includeRuntimeDependencies )
            MergeStringSet( dependencies, moduleInfo->GetNamesOfRuntimeDependencies(), false );
        if ( includeLogicalDependencies )
            MergeStringSet( dependencies, moduleInfo->GetNamesOfLogicalDependencies(), false );
    }
    if ( targetPlatform != KnownPlatforms::AllPlatforms && !targetPlatform.IsNULLOrEmpty() )
    {
        moduleInfo = mutableModuleInfoEntry->FindOrCreateModuleInfoForPlatform( KnownPlatforms::AllPlatforms, false );
        if ( !moduleInfo.IsNULL() )
        {
            MergeStringSet( dependencies, moduleInfo->GetNamesOfDependencies(), false );
            if ( includeRuntimeDependencies )
                MergeStringSet( dependencies, moduleInfo->GetNamesOfRuntimeDependencies(), false );
            if ( includeLogicalDependencies )
                MergeStringSet( dependencies, moduleInfo->GetNamesOfLogicalDependencies(), false );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CProjectInfo::FindModulesWhichDependOnModuleForPlatform( TMutableModuleInfoEntryPairVector& foundModules ,
                                                         const CORE::CString& targetPlatform             ,
                                                         const CORE::CString& dependencyName             ,
                                                         bool tryToUseDependencyChains                   ,
                                                         bool includeLogicalDependents                   ) const
{GUCEF_TRACE;

    if ( tryToUseDependencyChains )
    {
        // Use the dependency chains whenever possible for this kind of thing
        CModuleDependencyNodePtr dependencyChain;
        if ( TryGetModuleDependencyChain( dependencyChain  ,
                                          dependencyName   ,
                                          targetPlatform   ,
                                          true             ) && !dependencyChain.IsNULL() )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:FindModulesWhichDependOnModuleForPlatform: Found dependency chain for " + dependencyName + " for platform " + targetPlatform );

            TModuleInfoEntryPtrSet dependents;
            if ( dependencyChain->GatherDependentModules( dependents, false ) )
            {
                TModuleInfoEntryPtrSet::iterator n = dependents.begin();
                while ( n != dependents.end() )
                {
                    const CModuleInfoEntryPtr& dependent = (*n);
                    if GUCEF_PREDICT_TRUE( !dependent.IsNULL() )
                    {
                        CModuleInfoPtr platformSpecific = dependent->FindModuleInfoForPlatform( targetPlatform, true );
                        TMutableModuleInfoEntryPair infoPair( dependent, platformSpecific );
                        foundModules.push_back( infoPair );
                    }
                    ++n;
                }
            }
            if ( includeLogicalDependents )
            {
                dependents.clear();
                if ( dependencyChain->GatherLogicallyDependentModules( dependents, false ) )
                {
                    TModuleInfoEntryPtrSet::iterator n = dependents.begin();
                    while ( n != dependents.end() )
                    {
                        const CModuleInfoEntryPtr& dependent = (*n);
                        if GUCEF_PREDICT_TRUE( !dependent.IsNULL() )
                        {
                            CModuleInfoPtr platformSpecific = dependent->FindModuleInfoForPlatform( targetPlatform, true );
                            TMutableModuleInfoEntryPair infoPair( dependent, platformSpecific );
                            foundModules.push_back( infoPair );
                        }
                        ++n;
                    }
                }
            }
            return true;
        }
    }

    // brute force method
    // Loop trough all modules and check if they depend on the given module    
    TStringToModuleInfoEntryPtrMap::const_iterator i = modules.begin();
    while ( i != modules.end() )
    {
        const CModuleInfoEntryPtr& moduleInfoEntry = (*i).second;
        TModuleInfoPtrMap::const_iterator n = moduleInfoEntry->GetModulesPerPlatform().find( targetPlatform );
        if ( n != moduleInfoEntry->GetModulesPerPlatform().end() )
        {
            // This module has info for for this module, check its type
            CModuleInfoPtr moduleInfo = (*n).second;
            if ( IsStringInList( moduleInfo->GetNamesOfDependencies(), false, dependencyName ) )
            {
                TMutableModuleInfoEntryPair infoPair( moduleInfoEntry, moduleInfo );
                foundModules.push_back( infoPair );
            }
            if ( includeLogicalDependents )
            {
                if ( IsStringInList( moduleInfo->GetNamesOfLogicalDependencies(), false, dependencyName ) )
                {
                    TMutableModuleInfoEntryPair infoPair( moduleInfoEntry, moduleInfo );
                    foundModules.push_back( infoPair );
                }
            }
        }

        ++i;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CProjectInfo::FindModulesWhichDependOnModule( TMutableModuleInfoEntryPairVector& foundModules ,
                                              const CORE::CString& dependencyName             ,
                                              bool tryToUseDependencyChains                   ,
                                              bool includeLogicalDependents                   ) const
{GUCEF_TRACE;

    TMutableModuleInfoEntryPairVector results;

    // we specifically use a platforms list which is sourced from our flat list of modules
    // so that we can ensure that we have all platforms used by the modules
    TStringSet platformList;
    GetAllPlatformsUsed( platformList, true );

    // Loop trough all platforms
    bool totalSuccess = true;
    TStringSet::const_iterator i = platformList.begin();
    while ( i != platformList.end() )
    {
        totalSuccess = FindModulesWhichDependOnModuleForPlatform( foundModules, (*i), dependencyName, tryToUseDependencyChains, includeLogicalDependents ) && totalSuccess;
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CProjectInfo::FindModulesWhichDependOnModule( TModuleInfoEntryPtrSet& foundModules ,
                                              const CORE::CString& dependencyName  ,
                                              bool tryToUseDependencyChains        ,
                                              bool includeLogicalDependents        ) const
{GUCEF_TRACE;

    TMutableModuleInfoEntryPairVector foundModulesDetails;
    bool totalSuccess = FindModulesWhichDependOnModule( foundModulesDetails, dependencyName, tryToUseDependencyChains, includeLogicalDependents );

    TMutableModuleInfoEntryPairVector::iterator n = foundModulesDetails.begin();
    while ( n != foundModulesDetails.end() )
    {
        foundModules.insert( (*n).first );
        ++n;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CProjectInfo::FindModulesWithModuleType( TModuleType moduleType               ,
                                         TModuleInfoEntryPtrSet& foundModules ) const
{GUCEF_TRACE;

    // Loop trough all modules and process each code as we go
    TStringToModuleInfoEntryPtrMap::const_iterator i = modules.begin();
    while ( i != modules.end() )
    {
        const CModuleInfoEntryPtr& moduleInfoEntry = (*i).second;
        if GUCEF_PREDICT_TRUE( !moduleInfoEntry.IsNULL() )
        {
            if ( moduleInfoEntry->HasAnyModuleInfoWithModuleType( moduleType ) )
            {
                foundModules.insert( moduleInfoEntry );
            }
        }

        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CProjectInfo::MergeBinaryPackageInfoIntoModules( CModuleInfoEntryPtr binaryPackageModule )
{GUCEF_TRACE;

    // When we find a binary package we need to add its information to the modules that depend on it
    // its not a module in the same sense as the other modules, its a way to designate a dependency
    // We now resolved and integrate that dependency

    if GUCEF_PREDICT_FALSE( binaryPackageModule.IsNULL() )
        return false;

    TModuleInfoPtrSet binaryPackageInfoPerPlatform;
    binaryPackageModule->FindAllModuleInfoWithModuleType( MODULETYPE_BINARY_PACKAGE, binaryPackageInfoPerPlatform );

    bool totalSuccess = true;

    TModuleInfoPtrSet::iterator p = binaryPackageInfoPerPlatform.begin();
    while ( p != binaryPackageInfoPerPlatform.end() )
    {
        const CModuleInfoPtr& moduleInfo = (*p);
        const CORE::CString& targetPlatform = moduleInfo->GetPlatformName();
        CORE::CString moduleName = binaryPackageModule->GetModuleNameAlways( targetPlatform );

        TMutableModuleInfoEntryPairVector links;
        FindModulesWhichDependOnModuleForPlatform( links, targetPlatform, moduleName, false, false );

        TMutableModuleInfoEntryPairVector::iterator m = links.begin();
        while ( m != links.end() )
        {
            CModuleInfoEntryPtr depModuleInfoEntry = (*m).first;
            CModuleInfoPtr depModuleInfo = (*m).second;

            if ( !depModuleInfoEntry.IsNULL() )
            {
                if ( !depModuleInfo.IsNULL() )
                {
                    const CORE::CString& targetPlatformOfDependent = depModuleInfo->GetPlatformName();

                    // For this library replace the binary package reference with the information from said message
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:MergeBinaryPackageInfoIntoModules: Replacing binary package reference \"" + moduleName +
                        "\" for package platform " + moduleInfo->GetPlatformName() + " and dependent platform " + targetPlatformOfDependent + " with information from said package for module: " + depModuleInfoEntry->GetConsensusName() );

                    // Not every library may need every part of a dependency
                    // some might elect to only use the linker dependency, others also the general dependency for headers, 
                    // some only that and no linker dependency.
                    // Point being, respect what is specified

                    bool mergeSuccess = true;

                    if ( depModuleInfoEntry->HasLinkerDependency( targetPlatformOfDependent, moduleName, true ) )
                    {
                        // We need to merge in the linker dependencies and paths (additive)
                        mergeSuccess = depModuleInfo->linkerSettings.MoveLinkedLibraryToLogicalLibraries( moduleName ) && mergeSuccess;
                        mergeSuccess = depModuleInfo->linkerSettings.MergeLinkedLibraries( moduleInfo->linkerSettings, true ) && mergeSuccess;
                    }

                    if ( depModuleInfoEntry->HasDependency( targetPlatformOfDependent, moduleName, true ) )
                    {
                        // we also need to merge in any include paths for any headers provided with the binary                    
                        TStringSet binaryPackagePlatformIncludeDirs;
                        binaryPackageModule->GetPathsToIncludeDirsForPlatform( targetPlatform, depModuleInfoEntry->GetAbsolutePathToModuleRootDir(), binaryPackagePlatformIncludeDirs, false );
                        depModuleInfo->AddDependencyIncludeDirs( binaryPackagePlatformIncludeDirs );
                        depModuleInfo->MoveDependencyToLogicalDependencies( moduleName );

                        if ( KnownPlatforms::AllPlatforms != targetPlatform )
                        {
                            // Unlikely but if there are any 'all' platforms headers for a binary package put them on the 'all' platforms definition of the dependent
                            // module if there is one
                            TStringSet binaryPackageIncludeDirs;
                            binaryPackageModule->GetPathsToIncludeDirsForPlatform( KnownPlatforms::AllPlatforms, depModuleInfoEntry->GetAbsolutePathToModuleRootDir(), binaryPackageIncludeDirs, false );
                            if ( !binaryPackageIncludeDirs.empty() )
                            {
                                if ( targetPlatformOfDependent == KnownPlatforms::AllPlatforms )
                                {
                                    depModuleInfo->AddDependencyIncludeDirs( binaryPackageIncludeDirs );
                                }
                                else
                                {
                                    CModuleInfoPtr allPlatformsDepModuleInfo = depModuleInfoEntry->FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms );
                                    if ( !allPlatformsDepModuleInfo.IsNULL() )
                                    {
                                        allPlatformsDepModuleInfo->AddDependencyIncludeDirs( binaryPackageIncludeDirs );
                                        allPlatformsDepModuleInfo->MoveDependencyToLogicalDependencies( moduleName );
                                    }
                                    else
                                    {
                                        depModuleInfo->AddDependencyIncludeDirs( binaryPackageIncludeDirs );
                                    }
                                }
                            }
                        }
                    }

                    if ( !mergeSuccess )
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to merge binary package reference \"" + moduleName +
                            "\" for platform " + moduleInfo->GetPlatformName() + " information into module: " + depModuleInfoEntry->GetConsensusName() );
                    }
                    totalSuccess = mergeSuccess && totalSuccess;
                }
                
            }
            ++m;
        }

        ++p;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CProjectInfo::DetermineAndRelocateLogicalDependencies( void )
{GUCEF_TRACE;

    TStringToModuleInfoEntryPtrMap::iterator i = modules.begin();
    while ( i != modules.end() )
    {
        const CModuleInfoEntryPtr& moduleEntry = (*i).second;
        if GUCEF_PREDICT_TRUE( !moduleEntry.IsNULL() )
        {
            if ( moduleEntry->HasAnyLogicalModuleType() )
            {
                const CORE::CString& dependencyName = moduleEntry->GetConsensusName();
                TModuleInfoEntryPtrSet foundModules;
                if ( FindModulesWhichDependOnModule( foundModules    ,
                                                     dependencyName  ,
                                                     false           ,
                                                     false           ) )
                {
                    TModuleInfoEntryPtrSet::iterator n = foundModules.begin();
                    while ( n != foundModules.end() )
                    {
                        const CModuleInfoEntryPtr& dependentModuleEntry = (*n);
                        if GUCEF_PREDICT_TRUE( !dependentModuleEntry.IsNULL() )
                        {
                            const TModuleInfoPtrMap& modulesPerPlatform = dependentModuleEntry->GetModulesPerPlatform();
                            TModuleInfoPtrMap::const_iterator m = modulesPerPlatform.begin();
                            while ( m != modulesPerPlatform.end() )
                            {
                                const CModuleInfoPtr& dependentModuleInfo = (*m).second;
                                if GUCEF_PREDICT_TRUE( !dependentModuleInfo.IsNULL() )
                                {
                                    if ( dependentModuleEntry->HasDependency( dependentModuleInfo->GetPlatformName(), dependencyName, true ) )
                                    {
                                        // Move from logical to actual dependency
                                        CModuleInfoPtr mutableDependentModuleInfo = dependentModuleInfo;
                                        mutableDependentModuleInfo->MoveDependencyToLogicalDependencies( dependencyName );
                                    }
                                }
                                ++m;
                            }
                        }
                        ++n;
                    }
                }
            }
        }
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CProjectInfo::MergeAllBinaryPackageInfoIntoModules( void )
{GUCEF_TRACE;

    TModuleInfoEntryPtrSet binaryPackageModules;
    if ( FindModulesWithModuleType( MODULETYPE_BINARY_PACKAGE, binaryPackageModules ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:MergeAllBinaryPackageInfoIntoModules: Found " + CORE::ToString( binaryPackageModules.size() ) + " binary packages" );
        bool totalSuccess = true;

        TModuleInfoEntryPtrSet::iterator i = binaryPackageModules.begin();
        while ( i != binaryPackageModules.end() )
        {
            const CModuleInfoEntryPtr& moduleInfoEntry = (*i);
            totalSuccess = MergeBinaryPackageInfoIntoModules( moduleInfoEntry ) && totalSuccess;
            ++i;
        }

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:MergeAllBinaryPackageInfoIntoModules: Finished merging " + CORE::ToString( binaryPackageModules.size() ) +
            " binary packages. totalSuccess=" + CORE::ToString( totalSuccess ) );
        return totalSuccess;
    }
    return false;
}

/*---------------------------------------------------------------------------*/
               
bool 
CProjectInfo::Serialize( CORE::CDataNode& domRootNode                        ,
                         const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    // start from a clean slate
    domRootNode.Clear();

    // Add project info
    domRootNode.SetName( "Project" );
    domRootNode.SetAttribute( "ModuleCount", CORE::ToString( modules.size() ) );
    domRootNode.SetAttribute( "Name", m_projectName );

    // Add info for each module
    TStringToModuleInfoEntryPtrMap::const_iterator i = modules.begin();
    while ( i != modules.end() )
    {
        CORE::CDataNode* moduleNode = domRootNode.AddChild( "ModuleInfoEntry" );
        if ( GUCEF_NULL != moduleNode )
        {       
            const CModuleInfoEntryPtr& moduleEntry = (*i).second;
            if ( !moduleEntry->Serialize( *moduleNode, settings ) )
            {
                totalSuccess = false;
            }
        }
        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/
               
bool 
CProjectInfo::Serialize( const CORE::CString& outputFilePath                 ,
                         const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec = GetXmlDStoreCodec();
    if ( !codec.IsNULL() )
    {
        CORE::CDataNode info;
        if ( Serialize( info, settings ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:Serialize: Successfully generated a data tree with all project information" );

            if ( codec->StoreDataTree( &info, outputFilePath ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:Serialize: Successfully wrote all project information to disk file \"" + outputFilePath + "\"" );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:Serialize: Failed to store the serialized project information to disk at " + outputFilePath );
                return false;
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:Serialize: Failed to serialize the project information" );
            return false;
        }
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:Serialize: Cannot serialize since no codec is registered that can be used for serialization" );
    return false;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::DeserializeModuleEntries( const CORE::CDataNode& domRootNode                  ,
                                        const CORE::CDataNodeSerializableSettings& settings ,
                                        TStringToModuleInfoEntryPtrMap& moduleInfoEntries   )
{GUCEF_TRACE;

    CORE::CDataNode::TConstDataNodeSet nodeSet = domRootNode.FindNodesOfType( "ModuleInfoEntry", true );
    if ( nodeSet.empty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DeserializeModuleEntries: No ModuleInfoEntry nodes were found in the document" );
        return false;
    }

    UInt32 errorCount = 0;
    UInt32 modulesAdded = 0;
    CORE::CDataNode::TConstDataNodeSet::const_iterator i = nodeSet.begin();
    while ( i != nodeSet.end() )
    {
        CModuleInfoEntryPtr newModuleInfo = CModuleInfoEntry::CreateSharedObj();
        if ( !newModuleInfo.IsNULL() )
        {                                         
            const CORE::CDataNode& domRootNode = *(*i);
            if ( newModuleInfo->Deserialize( domRootNode, settings ) )
            {
                moduleInfoEntries[ newModuleInfo->GetConsensusName() ] = newModuleInfo;
                ++modulesAdded;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeProjectInfo: Failed deserialize module info entry" );
                ++errorCount;
            }
        }
        ++i;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DeserializeModuleEntries: Added " + CORE::ToString( modulesAdded ) +
        " modules and experienced " + CORE::ToString( errorCount ) + " errors" );

    return errorCount == 0;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::DeserializeModuleEntries( const CORE::CString& pathToModuleInfoFile           ,
                                        const CORE::CDataNodeSerializableSettings& settings ,
                                        TStringToModuleInfoEntryPtrMap& moduleInfoEntries   )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec = GetXmlDStoreCodec();
    if ( !codec.IsNULL() )
    {
        CORE::CDateTime lastModifiedDt = CORE::GetFileModificationTime( pathToModuleInfoFile );

        CORE::CDataNode rootNode;
        if ( codec->BuildDataTree( &rootNode, pathToModuleInfoFile ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DeserializeModuleEntries: Successfully loaded module information from file \"" + pathToModuleInfoFile + "\", now we will parse the information" );

            if ( DeserializeModuleEntries( rootNode, settings, moduleInfoEntries ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DeserializeModuleEntries: Successfully deserialized information from file \"" + pathToModuleInfoFile + "\". File was last modified " + CORE::ToString( lastModifiedDt ) );

                TStringToModuleInfoEntryPtrMap::iterator i = moduleInfoEntries.begin();
                while ( i != moduleInfoEntries.end() )
                {
                    CModuleInfoEntryPtr& module = (*i).second;
                    if GUCEF_PREDICT_TRUE( !module.IsNULL() )
                        module->SetDefinitionFileLastModifiedDt( lastModifiedDt );
                    ++i;
                }

                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeModuleInfo: Failed to deserialize module information from file at " + pathToModuleInfoFile );
                return false;
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DeserializeModuleEntries: Failed to load the module information from file at " + pathToModuleInfoFile );
            return false;
        }
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DeserializeModuleEntries: Cannot deserialize since no codec is registered that can be used for deserialization" );
    return false;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::DeserializeModuleEntries( const CORE::CDataNode& domRootNode                  ,
                                        const CORE::CDataNodeSerializableSettings& settings ,
                                        size_t suggestedNrOfModules                         )
{GUCEF_TRACE;

    TStringToModuleInfoEntryPtrMap moduleInfoEntries;        
    if ( DeserializeModuleEntries( domRootNode       ,
                                   settings          ,
                                   moduleInfoEntries ) )
    {
        // Add the modules to the project
        TStringToModuleInfoEntryPtrMap::const_iterator i = moduleInfoEntries.begin();
        while ( i != moduleInfoEntries.end() )
        {
            const CModuleInfoEntryPtr& newModuleInfo = (*i).second;
            if ( !newModuleInfo.IsNULL() )
            {
                modules[ newModuleInfo->GetConsensusName() ] = newModuleInfo;
            }
            ++i;
        }
        return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::Deserialize( const CORE::CDataNode& domRootNode                  ,
                           const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    const CORE::CDataNode* node = domRootNode.Search( "Project", '\\', true );
    if ( GUCEF_NULL == node )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeProjectInfo: Failed locate project root data node" );
        return false;
    }

    m_projectName = node->GetAttributeValueOrChildValueByName( "Name" ).AsString( m_projectName );
    size_t suggestedNrOfModules = node->GetAttributeValueOrChildValueByName( "ModuleCount" ).AsSizeT();

    bool deserializeSuccess = DeserializeModuleEntries( *node, settings, suggestedNrOfModules );
    DeriveAbsModuleRootSubSirsFromProjRelDirs();
    bool bulkPostProcessSuccess = BulkPostProcessAllModuleInfo( true );
    return deserializeSuccess && bulkPostProcessSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::Deserialize( const CORE::CString& inputFilepath )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec = GetXmlDStoreCodec();
    if ( !codec.IsNULL() )
    {
        CORE::CDataNode rootNode;
        if ( codec->BuildDataTree( &rootNode, inputFilepath ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeModuleInfo: Successfully loaded module information from file \"" + inputFilepath + "\", now we will parse the information" );

            // @TODO: temp hack to get ProjectInfo into the deserializer which is needed for normalization
            CORE::CDataNodeSerializableSettings defaultSerializableSettings;
            defaultSerializableSettings.SetUserData( const_cast< CProjectInfo* >( this ) );

            if ( Deserialize( rootNode, defaultSerializableSettings ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeModuleInfo: Successfully deserialized information from file \"" + inputFilepath + "\"" );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeModuleInfo: Failed to deserialize module information from file at " + inputFilepath );
                return false;
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeModuleInfo: Failed to load the module information from file at " + inputFilepath );
            return false;
        }
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeModuleInfo: Cannot deserialize since no codec is registered that can be used for deserialization" );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CProjectInfo::DeriveAbsModuleRootSubSirsFromProjRelDirs( void )
{GUCEF_TRACE;

    TStringToModuleInfoEntryPtrMap::iterator i = modules.begin();
    while ( i != modules.end() )
    {
        CModuleInfoEntryPtr& moduleEntry = (*i).second;
        if GUCEF_PREDICT_TRUE( !moduleEntry.IsNULL() )
        {
            if ( moduleEntry->GetAbsolutePathToModuleRootDir().IsNULLOrEmpty()         &&
                 !moduleEntry->GetProjectRelativePathToModuleRootDir().IsNULLOrEmpty() )
            {
                // We will derive the absolute path from the project root and the project relative path for the given module
                TStringVector::const_iterator n = rootDirs.begin();
                while ( n != rootDirs.end() )
                {
                    CORE::CString projectRootDir = (*n);
                    CORE::CString absPath = CORE::CombinePath( projectRootDir, moduleEntry->GetProjectRelativePathToModuleRootDir().ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
                    if ( IsDirAModuleDir( absPath ) )
                    {
                        moduleEntry->SetAbsolutePathToModuleRootDir( absPath );
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:DeriveAbsModuleRootSubSirsFromProjRelDirs: Derived abs module path \"" + absPath +
                            "\" from project root relative path \"" + moduleEntry->GetProjectRelativePathToModuleRootDir() + "\" for module " + moduleEntry->GetConsensusName() );
                        break;
                    }
                    ++n;
                }
            }
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

CModuleInfoEntryPtr
CProjectInfo::FindModuleAccordingToBuildOrder( const CORE::CString& targetPlatform ,
                                               Int64 buildOrderIndex               ) const
{GUCEF_TRACE;

    // In order for this functionality to work we need the dependency chains determined ahead of time
    if ( AreDependencyChainsInitialized() )
    {
        TStringToModuleDependencyNodePtrMap::const_iterator d = m_moduleDependencyChains.find( targetPlatform );
        if ( d != m_moduleDependencyChains.end() )
        {
            const TModuleDependencyNodePtrMap& chainNodeMap = (*d).second;
            TModuleDependencyNodePtrMap::const_iterator i = chainNodeMap.begin();
            while ( i != chainNodeMap.end() )
            {
                const CModuleDependencyNodePtr& chainNode = (*i).second;
                if GUCEF_PREDICT_TRUE( !chainNode.IsNULL() )
                {
                    // Check if this node has the requested build order
                    if ( chainNode->GetBuildOrder() == buildOrderIndex )
                    {
                        // We have a match, return the module info entry for this node
                        return chainNode->GetModule();
                    }
                }
            }
            ++i;
        }
    }       
    else
    {
        TStringToModuleInfoEntryPtrMap::const_iterator i = modules.begin();
        while ( i != modules.end() )
        {
            const CModuleInfoEntryPtr& moduleEntry = (*i).second;

            // Check to see if we have an entry for this platform
            TModuleInfoPtrMap::const_iterator n = moduleEntry->GetModulesPerPlatform().find( targetPlatform );
            if ( n != moduleEntry->GetModulesPerPlatform().end() )
            {
                // Check to see if the entry has a platform specific build order
                const CModuleInfoPtr& info = (*n).second;
                if ( buildOrderIndex == info->buildOrder )
                {
                    return moduleEntry;
                }
            }
            ++i;
        }
    }
    return CModuleInfoEntryPtr();
}

/*---------------------------------------------------------------------------*/

CModuleInfoEntryPtr
CProjectInfo::FindFirstModuleAccordingToBuildOrder( const CORE::CString& targetPlatform ) const
{GUCEF_TRACE;

    CModuleInfoEntryPtr foundModule = FindModuleAccordingToBuildOrder( targetPlatform, 1 );
    if ( !foundModule.IsNULL() )
    {
        if ( !AreDependencyChainsInitialized() )
        {
            // If we are not using dependency chains we cannot rely on layered (meaning all vs specific) weaved together platforms
            // As such we need to check against the "AllPlatforms" platform as well
            if ( targetPlatform != KnownPlatforms::AllPlatforms )
            {
                CModuleInfoEntryPtr allPlatformsModule = FindModuleAccordingToBuildOrder( KnownPlatforms::AllPlatforms, 1 );
                if ( !allPlatformsModule.IsNULL() )
                {
                    // We have a match, return the module info entry for this node
                    foundModule = allPlatformsModule;
                }
            }
        }
    }
    return foundModule;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::GetModulesMappedByBuildOrder( const CORE::CString& targetPlatform                ,
                                            TModuleInfoEntryPrioMap& modulesMappedByBuildOrder ) const
{GUCEF_TRACE;

    // We will let the stl container do the sorting for us
    // All we need to is enter all the modules in the map with their build order as the key

    if ( AreDependencyChainsInitialized() )
    {
        TStringToModuleDependencyNodePtrMap::const_iterator d = m_moduleDependencyChains.find( targetPlatform );
        if ( d != m_moduleDependencyChains.end() )
        {
            const TModuleDependencyNodePtrMap& chainNodeMap = (*d).second;
            TModuleDependencyNodePtrMap::const_iterator i = chainNodeMap.begin();
            while ( i != chainNodeMap.end() )
            {
                const CModuleDependencyNodePtr& chainNode = (*i).second;
                if GUCEF_PREDICT_TRUE( !chainNode.IsNULL() )
                {
                    modulesMappedByBuildOrder[ chainNode->GetBuildOrder() ] = chainNode->GetModule();
                }
                ++i;
            }
        }
    }
    else
    {
        TStringToModuleInfoEntryPtrMap::const_iterator i = modules.begin();
        while ( i != modules.end() )
        {
            const CModuleInfoEntryPtr& moduleEntry = (*i).second;
            if GUCEF_PREDICT_TRUE( !moduleEntry.IsNULL() )
            {
                modulesMappedByBuildOrder[ moduleEntry->GetBuildOrder( targetPlatform ) ] = moduleEntry;
            }
            ++i;
        }
    }
    return true;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::GetModulesMappedByBuildOrderForTarget( const CORE::CString& consensusModuleName           ,
                                                     const CORE::CString& targetPlatform                ,
                                                     TModuleInfoEntryPrioMap& modulesMappedByBuildOrder ,
                                                     bool includeDependenciesOfDependencies             ,
                                                     bool addDependencies                               ,
                                                     bool addLinkerDependencies                         ,
                                                     bool addRuntimeDependencies                        ) const
{GUCEF_TRACE;

    if ( AreDependencyChainsInitialized() )
    {
        CModuleDependencyNodePtr dependencyChain;
        if ( TryGetModuleDependencyChain( dependencyChain     ,
                                          consensusModuleName ,
                                          targetPlatform      ,
                                          true                ) && !dependencyChain.IsNULL() )
        {
            dependencyChain->GetDependencyModulesMappedByBuildOrder( modulesMappedByBuildOrder         ,
                                                                     includeDependenciesOfDependencies ,
                                                                     addDependencies                   ,
                                                                     addLinkerDependencies             ,
                                                                     addRuntimeDependencies            );
            return true;
        }
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::GenerateModuleDependencyIncludes( CModuleInfoEntryPtr& moduleInfoEntry            ,
                                                const CModuleInfoEntryPtr dependencyModuleEntry ,
                                                const CORE::CString& platformName               )
{GUCEF_TRACE;

    // Narrow it down to the platform we are interested in
    TModuleInfoPtrMap::const_iterator n = dependencyModuleEntry->GetModulesPerPlatform().find( platformName );
    if ( n != dependencyModuleEntry->GetModulesPerPlatform().end() )
    {
        // this dependency has module info which is specific to this platform
        const CModuleInfoPtr& dependencyModule = (*n).second;

        CModuleInfoPtr moduleInfo;

        // For header include locations we want to include the module definition
        // location regardless of whether headers were found there. This takes care of the
        // use-case whereby people use complex relative paths in their headers which need a particular
        // starting point
        if ( ( dependencyModule->moduleType == MODULETYPE_HEADER_INCLUDE_LOCATION ) ||
             ( dependencyModule->moduleType == MODULETYPE_BINARY_PACKAGE )           )
        {
            moduleInfo = moduleInfoEntry->FindOrCreateModuleInfoForPlatform( platformName, true );

            // Determine the relative path to this other module's root
            CORE::CString relativePath = CORE::GetRelativePathToOtherPathRoot( moduleInfoEntry->GetAbsolutePathToModuleRootDir()       ,
                                                                               dependencyModuleEntry->GetAbsolutePathToModuleRootDir() );
            
            // Check for an edge case where both modules are in the same directory
            // In this case we still want an entry to the directory even through the relative path between the 2 roots is empty
            if ( relativePath.IsNULLOrEmpty() ||
                 moduleInfoEntry->GetAbsolutePathToModuleRootDir() == dependencyModuleEntry->GetAbsolutePathToModuleRootDir() )
            {
                relativePath = "../" + CORE::LastSubDir( moduleInfoEntry->GetAbsolutePathToModuleRootDir() );
            }
            else
            {
                relativePath = relativePath.ReplaceChar( '\\', '/' );
            }

            moduleInfo->dependencyIncludeDirs.insert( relativePath );
        }

        // Now construct the relative path to each of the dependency module's include dirs
        // These dir will all become include dirs for this module
        const TStringSetMap& headerFiles = dependencyModule->GetIncludeDirs();
        if ( !headerFiles.empty() )
        {
            // Since this platform specific entry has header files we will need to generate
            // and include platform specific dependency paths to this module
            // to that end we will grab this module's definition for this platform or make a
            // new one if no entry exists yet
            if ( moduleInfo.IsNULL() )
                moduleInfo = moduleInfoEntry->FindOrCreateModuleInfoForPlatform( platformName, true );

            // Determine the relative path to this other module and subsequently the include dirs
            CORE::CString relativePath = CORE::GetRelativePathToOtherPathRoot( moduleInfoEntry->GetAbsolutePathToModuleRootDir()       ,
                                                                               dependencyModuleEntry->GetAbsolutePathToModuleRootDir() );
            relativePath = relativePath.ReplaceChar( '\\', '/' );

            TStringSetMap::const_iterator n = headerFiles.begin();
            while ( n != headerFiles.end() )
            {
                // If the path is a relative path we refine it to be from this module's location
                CORE::CString dependencyInclDir = CORE::RelativePath( (*n).first );
                if ( !CORE::IsAbsolutePath( dependencyInclDir ) && 0 != dependencyInclDir.HasSubstr( "#$#ENVVAR:", true ) )
                {
                    dependencyInclDir = CORE::CombinePath( relativePath, (*n).first );
                    dependencyInclDir = CORE::RelativePath( dependencyInclDir );
                }
                dependencyInclDir = dependencyInclDir.ReplaceChar( '\\', '/' );

                // Add the constructed include directory to the list of dependency directories
                // for the current module. This can later be used again by other modules which
                // include this one.
                moduleInfo->dependencyIncludeDirs.insert( dependencyInclDir );
                ++n;
            }

            // On top of that we have to include all the include dirs that the dependency module
            // was including itself since it's headers might be referring to those files.
            TStringSet::const_iterator m = dependencyModule->dependencyIncludeDirs.begin();
            while ( m != dependencyModule->dependencyIncludeDirs.end() )
            {
                CORE::CString dependencyInclDir = relativePath;
                CORE::AppendToPath( dependencyInclDir, (*m) );
                dependencyInclDir = CORE::RelativePath( dependencyInclDir );
                dependencyInclDir = dependencyInclDir.ReplaceChar( '\\', '/' );

                // Add the constructed include directory to the list of dependency directories
                // for the current module. This can later be used again by other modules which
                // include this one.
                moduleInfo->dependencyIncludeDirs.insert( dependencyInclDir );
                ++m;
            }
        }
    }

    return true;
}

/*---------------------------------------------------------------------------*/

// Generates include paths specific to the platform given
bool
CProjectInfo::GenerateModuleDependencyIncludesForPlatform( CModuleInfoEntryPtr& moduleInfoEntry ,
                                                           const CORE::CString& platformName    )
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( moduleInfoEntry.IsNULL() )
        return false;

    // First we grab all the dependencies for this module.
    // We are going to check each of the dependent modules for platform specific includes
    CModuleDependencyNodePtr dependencyChain;
    if ( TryGetModuleDependencyChain( dependencyChain                     ,
                                      moduleInfoEntry->GetConsensusName() ,
                                      platformName                        ,
                                      true                                ) && !dependencyChain.IsNULL() )
    {
        bool totalSuccess = true;

        // First generate the dependency includes for the regular module includes to regular dependencies
        const TModuleDependencyNodePtrMap& dependencies = dependencyChain->GetDependencies();
        TModuleDependencyNodePtrMap::const_iterator i = dependencies.begin();
        while ( i != dependencies.end() )
        {
            const CModuleDependencyNodePtr& dependencyChainNode = (*i).second;
            if GUCEF_PREDICT_TRUE( !dependencyChainNode.IsNULL() )
            {
                const CModuleInfoEntryPtr& dependencyModuleEntry = dependencyChainNode->GetModule();
                totalSuccess = GenerateModuleDependencyIncludes( moduleInfoEntry       ,
                                                                 dependencyModuleEntry ,
                                                                 platformName          ) && totalSuccess;

                // If modules don't have any 'all' platform definitions but has dependencies which are for 'all' platforms
                // we must merge into the current platform instead
                if ( platformName != KnownPlatforms::AllPlatforms       &&
                     !moduleInfoEntry->HasAllPlatformsDefinition()      &&
                     dependencyModuleEntry->HasAllPlatformsDefinition() )
                {
                    totalSuccess = GenerateModuleDependencyIncludes( moduleInfoEntry              ,
                                                                     dependencyModuleEntry        ,
                                                                     KnownPlatforms::AllPlatforms ) && totalSuccess;
                }
            }
            ++i;
        }

        // Now do the same for logical header include location dependencies
        const TModuleDependencyNodePtrMap& logicalDependencies = dependencyChain->GetLogicalDependencies();
        i = logicalDependencies.begin();
        while ( i != logicalDependencies.end() )
        {
            const CModuleDependencyNodePtr& dependencyChainNode = (*i).second;
            if GUCEF_PREDICT_TRUE( !dependencyChainNode.IsNULL() )
            {
                const CModuleInfoEntryPtr& dependencyModuleEntry = dependencyChainNode->GetModule();
                TModuleType moduleType = dependencyModuleEntry->GetModuleType( platformName );

                if ( MODULETYPE_HEADER_INCLUDE_LOCATION == moduleType )
                {
                    totalSuccess = GenerateModuleDependencyIncludes( moduleInfoEntry       ,
                                                                     dependencyModuleEntry ,
                                                                     platformName          ) && totalSuccess;

                    // If modules don't have any 'all' platform definitions but has dependencies which are for 'all' platforms
                    // we must merge into the current platform instead
                    if ( platformName != KnownPlatforms::AllPlatforms       &&
                         !moduleInfoEntry->HasAllPlatformsDefinition()      &&
                         dependencyModuleEntry->HasAllPlatformsDefinition() )
                    {
                        totalSuccess = GenerateModuleDependencyIncludes( moduleInfoEntry              ,
                                                                         dependencyModuleEntry        ,
                                                                         KnownPlatforms::AllPlatforms ) && totalSuccess;
                    }
                }
            }
            ++i;
        }

        return totalSuccess;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::GenerateDependencyIncludesForPlatform( const CORE::CString& platformName )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Generating dependency inherited includes for platform " + platformName );

    TModuleInfoEntryPrioMap modulesMappedByBuildOrder;
    GetModulesMappedByBuildOrder( platformName, modulesMappedByBuildOrder );

    TModuleInfoEntryPrioMap::iterator i = modulesMappedByBuildOrder.begin();
    while ( i != modulesMappedByBuildOrder.end() )
    {
        CModuleInfoEntryPtr moduleInfoEntry = (*i).second;
        GenerateModuleDependencyIncludesForPlatform( moduleInfoEntry ,
                                                     platformName    );
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::GeneratePreprocessorDefinesFromModuleInfo( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Generating preprocessor defines for all modules based on their info" );

    bool totalSuccess = true;
    TStringToModuleInfoEntryPtrMap::iterator i = modules.begin();
    while ( i != modules.end() )
    {
        CModuleInfoEntryPtr& moduleEntry = (*i).second;
        if GUCEF_PREDICT_TRUE( !moduleEntry.IsNULL() )
        {
            totalSuccess = moduleEntry->GeneratePreprocessorDefinesFromModuleInfo() && totalSuccess;
        }
        ++i;
    }
    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CProjectInfo::GenerateDependencyIncludes( void )
{GUCEF_TRACE;

    GenerateDependencyIncludesForPlatform( KnownPlatforms::AllPlatforms );

    TStringSet supportedPlatforms;
    GetAllEnabledPlatformsUsed( supportedPlatforms, true );
    supportedPlatforms.erase( KnownPlatforms::AllPlatforms );

    TStringSet::const_iterator i = supportedPlatforms.begin();
    while ( i != supportedPlatforms.end() )
    {
        GenerateDependencyIncludesForPlatform( (*i).Lowercase() );
        ++i;
    }

    // Normalize the include paths,..
    // It is possible that some dependency include paths are set as empty dir locations
    // or that dependency include paths are actually already include dirs of the project itself
    // we automatically clean that up now
    TStringToModuleInfoEntryPtrMap::iterator n = modules.begin();
    while ( n != modules.end() )
    {
        (*n).second->CleanupIncludeDirs();
        ++n;
    }

    return true;
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::MergeIntegrationLocationsIntoModuleForPlatform( const CORE::CString& targetPlatform       ,
                                                              const CModuleInfoPtr& moduleInfoToMergeIn ,
                                                              const CORE::CString& codeIncludeRoot      ,
                                                              const TModuleType moduleType              )
{GUCEF_TRACE;

    // First find all the modules which depend on this integration location
    TMutableModuleInfoEntryPairVector targetModules;
    FindModulesWhichDependOnModuleForPlatform( targetModules             ,
                                               targetPlatform            ,
                                               moduleInfoToMergeIn->name ,
                                               false                     ,
                                               true                      );

    // Now for each of these modules merge in the files
    TMutableModuleInfoEntryPairVector::iterator i = targetModules.begin();
    while ( i != targetModules.end() )
    {
        CModuleInfoEntryPtr moduleInfoEntry = (*i).first;
        CModuleInfoPtr moduleInfo = (*i).second;

        // Determine the relative path to this other module
        CORE::CString pathToCodeLocation = CORE::GetRelativePathToOtherPathRoot( moduleInfoEntry->GetAbsolutePathToModuleRootDir() ,
                                                                                 codeIncludeRoot                                   );

        // merge in the content for header integration locations as well as code integration locations
        TStringSetMap::const_iterator n;

        n = moduleInfoToMergeIn->GetIncludeDirs().begin();
        while ( n != moduleInfoToMergeIn->GetIncludeDirs().end() )
        {
            // Create the full path to the files
            CORE::CString fullPathToIncludeLocation = pathToCodeLocation;
            CORE::AppendToPath( fullPathToIncludeLocation, (*n).first );

            // Use the merge function just in case this location is already added by other means
            moduleInfo->AddIncludeFiles( fullPathToIncludeLocation, (*n).second );

            ++n;
        }

        // we only merge in source code for code integration locations
        if ( MODULETYPE_CODE_INTEGRATE_LOCATION == moduleInfoToMergeIn->moduleType )
        {
            n = moduleInfoToMergeIn->GetSourceDirs().begin();
            while ( n != moduleInfoToMergeIn->GetSourceDirs().end() )
            {
                // Create the full path to the files
                CORE::CString fullPathToSourceLocation = pathToCodeLocation;
                CORE::AppendToPath( fullPathToSourceLocation, (*n).first );

                // Use the merge function just in case this location is already added by other means
                moduleInfo->AddSourceFiles( fullPathToSourceLocation, (*n).second );

                ++n;
            }
        }

        // merge in any defines that come with the location's header or source
        moduleInfo->preprocessorSettings.Merge( moduleInfoToMergeIn->preprocessorSettings, false );
        // merge in additional include dirs
        MergeStringSet( moduleInfo->dependencyIncludeDirs, moduleInfoToMergeIn->dependencyIncludeDirs, true );

        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::MergeIntegrationLocationsIntoModuleForAllPlatformsPlatform( const CModuleInfoPtr& moduleInfoToMergeIn ,
                                                                          const CORE::CString& codeIncludeRoot      )
{GUCEF_TRACE;

    // First find all the modules which depend on this code include location
    TModuleInfoEntryPtrSet targetModules;
    FindModulesWhichDependOnModule( targetModules, moduleInfoToMergeIn->name, false, true );

    // Now for each of these modules merge in the files
    TModuleInfoEntryPtrSet::iterator i = targetModules.begin();
    while ( i != targetModules.end() )
    {
        CModuleInfoEntryPtr moduleInfoEntry = (*i);

        // Determine the relative path to this other module
        CORE::CString pathToCodeLocation = CORE::GetRelativePathToOtherPathRoot( moduleInfoEntry->GetAbsolutePathToModuleRootDir() ,
                                                                                 codeIncludeRoot                                   );

        TModuleInfoPtrMap::const_iterator m = moduleInfoEntry->GetModulesPerPlatform().find( KnownPlatforms::AllPlatforms );
        if ( m != moduleInfoEntry->GetModulesPerPlatform().end() )
        {
            CModuleInfoPtr moduleInfo = (*m).second;

            TStringSetMap::const_iterator n;

            if ( ( MODULETYPE_CODE_INTEGRATE_LOCATION == moduleInfoToMergeIn->moduleType )   ||
                 ( MODULETYPE_HEADER_INTEGRATE_LOCATION == moduleInfoToMergeIn->moduleType )  )
            {
                n = moduleInfoToMergeIn->GetIncludeDirs().begin();
                while ( n != moduleInfoToMergeIn->GetIncludeDirs().end() )
                {
                    // Create the full path to the files
                    CORE::CString fullPathToIncludeLocation = pathToCodeLocation;
                    CORE::AppendToPath( fullPathToIncludeLocation, (*n).first );

                    // Use the merge function just in case this location is already added by other means
                    moduleInfo->AddIncludeFiles( fullPathToIncludeLocation, (*n).second );

                    ++n;
                }

                // merge in any defines that come with the location's header or source
                moduleInfo->preprocessorSettings.Merge( moduleInfoToMergeIn->preprocessorSettings, false );
                // merge in additional include dirs
                MergeStringSet( moduleInfo->dependencyIncludeDirs, moduleInfoToMergeIn->dependencyIncludeDirs, true );
            }

            if ( MODULETYPE_CODE_INTEGRATE_LOCATION == moduleInfoToMergeIn->moduleType )
            {
                n = moduleInfoToMergeIn->GetSourceDirs().begin();
                while ( n != moduleInfoToMergeIn->GetSourceDirs().end() )
                {
                    // Create the full path to the files
                    CORE::CString fullPathToSourceLocation = pathToCodeLocation;
                    CORE::AppendToPath( fullPathToSourceLocation, (*n).first );

                    // Use the merge function just in case this location is already added by other means
                    moduleInfo->AddSourceFiles( fullPathToSourceLocation, (*n).second );
                    ++n;
                }
            }
        }
        else
        {
            // Since there is no AllPlatforms entry we will merge into each of the platform specific
            // entries which requires the code include dependency
            TModuleInfoPtrMap::const_iterator m = moduleInfoEntry->GetModulesPerPlatform().begin();
            while ( m != moduleInfoEntry->GetModulesPerPlatform().end() )
            {
                CModuleInfoPtr moduleInfo = (*m).second;

                // Check if this particular platform needs the dependency
                if ( IsStringInList( moduleInfo->GetNamesOfDependencies(), false, moduleInfoToMergeIn->name ) ||
                     IsStringInList( moduleInfo->GetNamesOfLogicalDependencies(), false, moduleInfoToMergeIn->name ) )
                {
                    TStringSetMap::const_iterator n;

                    if ( ( MODULETYPE_CODE_INTEGRATE_LOCATION == moduleInfoToMergeIn->moduleType )   ||
                         ( MODULETYPE_HEADER_INTEGRATE_LOCATION == moduleInfoToMergeIn->moduleType )  )
                    {
                        n = moduleInfoToMergeIn->GetIncludeDirs().begin();
                        while ( n != moduleInfoToMergeIn->GetIncludeDirs().end() )
                        {
                            // Create the full path to the files
                            CORE::CString fullPathToIncludeLocation = pathToCodeLocation;
                            CORE::AppendToPath( fullPathToIncludeLocation, (*n).first );

                            // Use the merge function just in case this location is already added by other means
                            moduleInfo->AddIncludeFiles( fullPathToIncludeLocation, (*n).second );

                            ++n;
                        }

                        // merge in any defines that come with the location's header or source
                        moduleInfo->preprocessorSettings.Merge( moduleInfoToMergeIn->preprocessorSettings, false );
                        // merge in additional include dirs
                        MergeStringSet( moduleInfo->dependencyIncludeDirs, moduleInfoToMergeIn->dependencyIncludeDirs, true );
                    }

                    if ( MODULETYPE_CODE_INTEGRATE_LOCATION == moduleInfoToMergeIn->moduleType )
                    {
                        n = moduleInfoToMergeIn->GetSourceDirs().begin();
                        while ( n != moduleInfoToMergeIn->GetSourceDirs().end() )
                        {
                            // Create the full path to the files
                            CORE::CString fullPathToSourceLocation = pathToCodeLocation;
                            CORE::AppendToPath( fullPathToSourceLocation, (*n).first );

                            // Use the merge function just in case this location is already added by other means
                            moduleInfo->AddSourceFiles( fullPathToSourceLocation, (*n).second );

                            ++n;
                        }
                    }
                }
                ++m;
            }
        }

        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::MergeIntegrationLocationsIntoModuleForAllPlatformsPlatform( void )
{GUCEF_TRACE;

    // Loop trough all modules and process each code include as we go
    TStringToModuleInfoEntryPtrMap::iterator i = modules.begin();
    while ( i != modules.end() )
    {
        CModuleInfoEntryPtr& moduleInfoEntry = (*i).second;
        TModuleInfoPtrMap::const_iterator n = moduleInfoEntry->GetModulesPerPlatform().find( KnownPlatforms::AllPlatforms );
        if ( n != moduleInfoEntry->GetModulesPerPlatform().end() )
        {
            // This module has info for for this module, check its type
            CModuleInfoPtr moduleInfo = (*n).second;
            if ( ( MODULETYPE_CODE_INTEGRATE_LOCATION == moduleInfo->moduleType )   ||
                 ( MODULETYPE_HEADER_INTEGRATE_LOCATION == moduleInfo->moduleType )  )
            {
                // We found a code include location, now process it for all modules which proclaim to have a dependency on it
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Processing integration location labeled as \"" + moduleInfo->name + "\" for platform " + KnownPlatforms::AllPlatforms );
                MergeIntegrationLocationsIntoModuleForAllPlatformsPlatform( moduleInfo                                        ,
                                                                            moduleInfoEntry->GetAbsolutePathToModuleRootDir() );
            }
        }

        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CProjectInfo::MergeIntegrationLocationsIntoModuleForPlatform( const CORE::CString& targetPlatform )
{GUCEF_TRACE;

    // Loop trough all modules and process each integration as we go
    TStringToModuleInfoEntryPtrMap::iterator i = modules.begin();
    while ( i != modules.end() )
    {
        CModuleInfoEntryPtr& moduleInfoEntry = (*i).second;

        TModuleInfoPtrMap::const_iterator n = moduleInfoEntry->GetModulesPerPlatform().find( targetPlatform );
        if ( n != moduleInfoEntry->GetModulesPerPlatform().end() )
        {
            // This module has info for for this module, check its type
            CModuleInfoPtr moduleInfo = (*n).second;
            if ( ( MODULETYPE_CODE_INTEGRATE_LOCATION == moduleInfo->moduleType ) ||
                 ( MODULETYPE_HEADER_INTEGRATE_LOCATION == moduleInfo->moduleType ) )
            {
                // We found a code include location, now process it for all modules which proclaim to have a dependency on it
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Processing integration location labeled as \"" + moduleInfo->name + "\" for platform " + targetPlatform );
                MergeIntegrationLocationsIntoModuleForPlatform( targetPlatform                                    ,
                                                                moduleInfo                                        ,
                                                                moduleInfoEntry->GetAbsolutePathToModuleRootDir() ,
                                                                moduleInfo->moduleType                            );
            }
        }

        ++i;
    }

}

/*-------------------------------------------------------------------------*/

bool
CProjectInfo::MergeIntegrationLocationsIntoModules( void )
{GUCEF_TRACE;

    MergeIntegrationLocationsIntoModuleForAllPlatformsPlatform();

    TStringSet supportedPlatforms;
    GetAllPlatformsUsed( supportedPlatforms, true );
    supportedPlatforms.erase( KnownPlatforms::AllPlatforms );

    TStringSet::const_iterator i = supportedPlatforms.begin();
    while ( i != supportedPlatforms.end() )
    {
        MergeIntegrationLocationsIntoModuleForPlatform( (*i).Lowercase() );
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CProjectInfo::FlagTaggedModulesToIgnoreAsSpecified( const CORE::CValueList& params )
{GUCEF_TRACE;

    TStringVector tagsOfModulesToRemove = params.GetValueAlways( "TagsOfModulesToRemove" ).AsString().ParseElements( ';', false );
    if ( tagsOfModulesToRemove.empty() )
        return true; // no work to do

    TStringToModuleInfoEntryPtrMap::iterator n = modules.begin();
    while ( n != modules.end() )
    {
        CModuleInfoEntryPtr& moduleEntry = (*n).second;

        TModuleInfoPtrMap::const_iterator m = moduleEntry->GetModulesPerPlatform().begin();
        while ( m != moduleEntry->GetModulesPerPlatform().end() )
        {
            const CORE::CString& platform = (*m).first;

            TStringVector::iterator i = tagsOfModulesToRemove.begin();
            while ( i != tagsOfModulesToRemove.end() )
            {
                CModuleInfoPtr moduleInfo = (*m).second;
                const CORE::CString& tag = (*i);

                moduleInfo->ignoreModule = moduleEntry->HasTag( tag, platform );
                moduleInfo->hasIgnoreModule = true;

                if ( moduleInfo->ignoreModule )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Setting ignore flag on module " + moduleEntry->GetConsensusName() + " for platform " + (*m).first + ", based on tag " + (*i) );
                }
                ++i;
            }
            ++m;
        }
        ++n;
    }
    return true;
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::GetAllTagsUsed( TStringSet& tagsUsed ) const
{GUCEF_TRACE;

    TStringToModuleInfoEntryPtrMap::const_iterator i = modules.begin();
    while ( i != modules.end() )
    {
        const TModuleInfoPtrMap& modulesPerPlatform = (*i).second->GetModulesPerPlatform();
        TModuleInfoPtrMap::const_iterator n = modulesPerPlatform.begin();
        while ( n != modulesPerPlatform.end() )
        {
            MergeStringSet( tagsUsed, (*n).second->tags, true );
            ++n;
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::GetTaggedModules( const CORE::CString& tag              ,
                                TModuleInfoEntryPtrSet& taggedModules ,
                                const CORE::CString& platform         ) const
{GUCEF_TRACE;

    TStringToModuleInfoEntryPtrMap::const_iterator i = modules.begin();
    while ( i != modules.end() )
    {
        const CModuleInfoEntryPtr& entry = (*i).second;
        if ( entry->HasTag( tag, platform ) )
            taggedModules.insert( entry );
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
CProjectInfo::GetTaggedModulesByName( const CORE::CString& tag            ,
                                      CORE::CStringSet& taggedModuleNames ,
                                      const CORE::CString& platform       ) const
{GUCEF_TRACE;

    TStringToModuleInfoEntryPtrMap::const_iterator i = modules.begin();
    while ( i != modules.end() )
    {
        const CModuleInfoEntryPtr& entry = (*i).second;
        if ( entry->HasTag( tag, platform ) )
            taggedModuleNames.insert( entry->GetConsensusName() );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CProjectInfo::GetProjectName( void ) const
{GUCEF_TRACE;

    return m_projectName;
}

/*-------------------------------------------------------------------------*/

const CORE::CValueList&
CProjectInfo::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CProjectInfo::SetSetttings( const CORE::CValueList& settings )
{GUCEF_TRACE;

    m_settings = settings;

    m_projectName = m_settings.GetValueAlways( "projectName" );

    // Set any global dir excludes per the settings
    globalDirExcludeList = m_settings.GetValueAlways( "dirsToIgnore" ).AsString().ParseElements( ';', false );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:SetSetttings: There are " + CORE::ToString( globalDirExcludeList.size() ) + " dirs in the global dir ignore list: "  + CORE::ToString( globalDirExcludeList ) );

    m_disabledPlatforms = m_settings.GetValueAlways( "disabledPlatforms" ).AsString().ParseUniqueElements( ';', false );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ProjectInfo:SetSetttings: There are " + CORE::ToString( m_disabledPlatforms.size() ) + " ignored platforms: " + CORE::ToString( m_disabledPlatforms ) );
}

/*-------------------------------------------------------------------------*/

void
CProjectInfo::SetRootDirs( const TStringVector& newRootDirs )
{GUCEF_TRACE;

    rootDirs = newRootDirs;
}

/*-------------------------------------------------------------------------*/

void
CProjectInfo::SetRootDir( const CORE::CString& rootDir )
{GUCEF_TRACE;

    rootDirs.clear();
    rootDirs.push_back( rootDir );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CProjectInfo::GetRelativePathFromProjectSubDirToProjectRootDir( const CORE::CString& projectSubDir ) const
{GUCEF_TRACE;

    // We can have multiple root dirs for the overall project so we need to find the right one
    // The right one is the one where the given subdir is a subdir of said project root
    TStringVector::const_iterator i = rootDirs.begin();
    while ( i != rootDirs.end() )
    {
        const CORE::CString& projectRoot = (*i);
        if ( projectSubDir.StartsWith( projectRoot ) )
        {
            // this is the root we need
            return CORE::GetRelativePathToOtherPathRoot( projectRoot, projectSubDir, true );
        }
        ++i;
    }
    return CORE::CString(); 
}

/*-------------------------------------------------------------------------*/

bool
CProjectInfo::BulkPostProcessAllModuleInfo( void )
{GUCEF_TRACE;

    return BulkPostProcessAllModuleInfo( false );
}

/*-------------------------------------------------------------------------*/

bool
CProjectInfo::BulkPostProcessAllModuleInfo( bool isLoadFromProjectInfo )
{GUCEF_TRACE;

    bool totalSuccess = true;

    if ( !isLoadFromProjectInfo )
    {
        // First move all logical dependencies into their own dependency collection
        // this removes noise from the rest of the processing
        totalSuccess = DetermineAndRelocateLogicalDependencies() && totalSuccess;

        // Merge headers and code from integration locations into modules
        // We apply this early on to make sure modules have all the header and source files properly accounted for
        totalSuccess = MergeIntegrationLocationsIntoModules() && totalSuccess;

        // Merge info from binary packages into modules
        totalSuccess = MergeAllBinaryPackageInfoIntoModules() && totalSuccess;

        // In order to avoid bad 'all' platforms links we need to first sanitize the definitions we were given
        // Incorrect 'all' platforms definitions can mess up the feasibility of a dependency tree
        totalSuccess = SanitizeAllPlatformsUsage() && totalSuccess;

        // For runtime dependencies we treat them always as 'optional' best effort
        // We try to retain them where possible, platform specific if need be.
        // However if unsustainable they are simply dropped
        totalSuccess = SanitizeRuntimeDependencies() && totalSuccess;

        // We generate certain preprocessor defines automatically
        // these are intended to help with automation/codegen to keep things consistent
        totalSuccess = GeneratePreprocessorDefinesFromModuleInfo() && totalSuccess;
    }

    // Generate the dependency chains which provides an optimized dependency tree going forward
    totalSuccess = UpdateDependencyChains( false ) && totalSuccess;

    // By default no modules are ignored but if so specified tags can cause a module to be set to ignore
    // This is just an advisory flag and it is up to the generator backends to not include the module
    // in the output while still ensuring the build remains functional
    totalSuccess = FlagTaggedModulesToIgnoreAsSpecified( m_settings ) && totalSuccess;

    if ( !isLoadFromProjectInfo )
    {
        // Based on all the information we have gathered we can now determine the correct build order
        // for all platforms
        totalSuccess = DetermineBuildOrderForAllModules() && totalSuccess;

        // Now we can generate all the include paths
        // this functionality relies on the build orders having been determined ahead of time
        totalSuccess = GenerateDependencyIncludes() && totalSuccess;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

CORE::CICloneable*
CProjectInfo::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CProjectInfo( *this );
}

/*---------------------------------------------------------------------------*/

const CORE::CString&
CProjectInfo::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
