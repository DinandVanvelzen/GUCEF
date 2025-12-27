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

#ifndef GUCEF_PROJECTGEN_CPROJECTINFO_H
#include "gucefProjectGen_CProjectInfo.h"
#define GUCEF_PROJECTGEN_CPROJECTINFO_H
#endif /* GUCEF_PROJECTGEN_CPROJECTINFO_H ? */

#ifndef GUCEF_PROJECTGEN_DATATYPES_H
#include "gucefProjectGen_DataTypes.h"
#define GUCEF_PROJECTGEN_DATATYPES_H
#endif /* GUCEF_PROJECTGEN_DATATYPES_H ? */

#include "gucefProjectGen_CModuleInfoEntry.h"

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

const CORE::CString CModuleInfoEntry::ClassTypeName = "GUCEF::PROJECTGEN::CModuleInfoEntry";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CModuleInfoEntry::CModuleInfoEntry( void ) 
    : CORE::CIDataNodeSerializable()
    , CORE::CTSharedObjCreator< CModuleInfoEntry, MT::CMutex >( this )
    , m_consensusName()
    , m_modulesPerPlatform()
    , m_flattenedInfoPerPlatform()
    , m_isBroken( false )
    , m_absRootDir()
    , m_projRelRootDir()
    , m_definitionFileLastModifiedDt( CORE::CDateTime::PastMax )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CModuleInfoEntry::CModuleInfoEntry( const CModuleInfoEntry& src ) 
    : CORE::CIDataNodeSerializable( src )
    , CORE::CTSharedObjCreator< CModuleInfoEntry, MT::CMutex >( this )
    , m_consensusName( src.m_consensusName )
    , m_modulesPerPlatform( src.m_modulesPerPlatform )
    , m_flattenedInfoPerPlatform( src.m_flattenedInfoPerPlatform )
    , m_isBroken( src.m_isBroken )
    , m_absRootDir( src.m_absRootDir )
    , m_projRelRootDir( src.m_projRelRootDir )
    , m_definitionFileLastModifiedDt( src.m_definitionFileLastModifiedDt )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

CModuleInfoEntry::CModuleInfoEntry( CModuleInfoEntry&& src ) GUCEF_NOEXCEPT
    : CORE::CIDataNodeSerializable( GUCEF_MOVE( src ) )
    , CORE::CTSharedObjCreator< CModuleInfoEntry, MT::CMutex >( GUCEF_MOVE( src ), this )
    , m_consensusName( GUCEF_MOVE( src.m_consensusName ) )
    , m_modulesPerPlatform( GUCEF_MOVE( src.m_modulesPerPlatform ) )
    , m_flattenedInfoPerPlatform( GUCEF_MOVE( src.m_flattenedInfoPerPlatform ) )
    , m_isBroken( src.m_isBroken )
    , m_absRootDir( GUCEF_MOVE( src.m_absRootDir ) )
    , m_projRelRootDir( GUCEF_MOVE( src.m_projRelRootDir ) )
    , m_definitionFileLastModifiedDt( GUCEF_MOVE( src.m_definitionFileLastModifiedDt ) )
{GUCEF_TRACE;

}

#endif

/*---------------------------------------------------------------------------*/

CModuleInfoEntry::~CModuleInfoEntry() 
{GUCEF_TRACE;

    Clear();
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfoEntry::GeneratePreprocessorDefinesFromModuleInfo( const CORE::CString& platform ,
                                                             CORE::CStringMap& defines     )
{GUCEF_TRACE;

    CModuleInfoPtr platformsDef = FindModuleInfoForPlatform( platform );
    if ( platformsDef.IsNULL() )
        return false;

    const CORE::CString* moduleApiPrefix = GUCEF_NULL;
    const CORE::CString* moduleName = GetModuleName( platform );
    if ( GUCEF_NULL != moduleName )
    {
        defines[ "MODULE_NAME" ] = *moduleName;
        moduleApiPrefix = moduleName;
    }

    if ( platform == KnownPlatforms::AllPlatforms )
    {
        const CORE::CString& moduleConsensusName = GetConsensusName();
        if ( !moduleConsensusName.IsNULLOrEmpty() )
        {
            defines[ "MODULE_CONSENSUS_NAME" ] = moduleConsensusName;
            moduleApiPrefix = &moduleConsensusName;
        }
    }

    CORE::CVersion semver;
    if ( GetSemVer( semver, platform ) )
    {
        CORE::CString semverStr = semver.ToString();
        defines[ "MODULE_SEMVER" ] = semverStr;
    }

    if ( !platformsDef->linkerSettings.GetTargetName().IsNULLOrEmpty() )
    {
        defines[ "MODULE_TARGET_NAME" ] = platformsDef->linkerSettings.GetTargetName();
    }

    bool isPlugin = HasTag( "plugin", platform );
    if ( isPlugin )
    {
        defines[ "MODULE_IS_PLUGIN" ] = "1";
        TModuleType moduleType = GetModuleType( platform );
        if ( GUCEF_NULL != moduleApiPrefix && moduleType == MODULETYPE_STATIC_LIBRARY )
        {
            // static plugins are provided with an auto generated API prefix
            defines[ "MODULE_API_PREFIX" ] = *moduleApiPrefix;
        }
    }

    return true;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfoEntry::GeneratePreprocessorDefinesFromModuleInfo( void )
{GUCEF_TRACE;

    CORE::CStringMap allPlatformDefines;
    GeneratePreprocessorDefinesFromModuleInfo( KnownPlatforms::AllPlatforms, allPlatformDefines );
    CModuleInfoPtr allPlatform = FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms );
    if ( !allPlatform.IsNULL() )
    {
        CORE::CStringMap::iterator n = allPlatformDefines.begin();
        while ( n != allPlatformDefines.end() )
        {
            const CORE::CString& platformDefineKey = (*n).first;
            const CORE::CString& platformDefineValue = (*n).second;
            CORE::CString platformDefine = platformDefineKey + '=' + platformDefineValue;
            allPlatform->preprocessorSettings.AddDefine( platformDefine );
            ++n;
        }
    }

    TModuleInfoPtrMap::iterator i = m_modulesPerPlatform.begin();
    while ( i != m_modulesPerPlatform.end() )
    {
        const CORE::CString& platform = (*i).first;
        CModuleInfoPtr& moduleInfo = (*i).second;

        if ( KnownPlatforms::AllPlatforms != platform )
        {
            CORE::CStringMap platformDefines;
            GeneratePreprocessorDefinesFromModuleInfo( platform, platformDefines );

            CORE::CStringMap::iterator m = platformDefines.begin();
            while ( m != platformDefines.end() )
            {
                const CORE::CString& platformDefineKey = (*m).first;
                const CORE::CString& platformDefineValue = (*m).second;
                if ( allPlatformDefines.find( platformDefineKey ) == allPlatformDefines.end() )
                {
                    // This platform has a define that isnt present for the 'all' platform
                    CORE::CString platformDefine = platformDefineKey + '=' + platformDefineValue;
                    moduleInfo->preprocessorSettings.AddDefine( platformDefine );
                }
                ++m;
            }
        }
        ++i;
    }

    return true;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfoEntry::Clear( void ) 
{GUCEF_TRACE;

    m_consensusName.Clear();
    TModuleInfoPtrMap::iterator i = m_modulesPerPlatform.begin();
    while ( i != m_modulesPerPlatform.end() )
    {
        CModuleInfoPtr& moduleInfo = (*i).second;
        if ( !moduleInfo.IsNULL() )
            moduleInfo->Clear();
        moduleInfo.Unlink();
        ++i;
    }
    m_modulesPerPlatform.clear();
    i = m_flattenedInfoPerPlatform.begin();
    while ( i != m_flattenedInfoPerPlatform.end() )
    {
        CModuleInfoPtr& moduleInfo = (*i).second;
        if ( !moduleInfo.IsNULL() )
            moduleInfo->Clear();
        moduleInfo.Unlink();
        ++i;
    }
    m_flattenedInfoPerPlatform.clear();
    m_isBroken = false;
    m_absRootDir.Clear();
    m_projRelRootDir.Clear();
    m_definitionFileLastModifiedDt = CORE::CDateTime::PastMax;
}

/*---------------------------------------------------------------------------*/

const CORE::CString&
CModuleInfoEntry::GetConsensusName( CModuleInfoPtr* moduleInfo ,
                                    bool dontUseCached         ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != moduleInfo )
        *moduleInfo = GUCEF_NULL;

    if ( !dontUseCached && !m_consensusName.IsNULLOrEmpty() )
        return m_consensusName;

    TModuleInfoPtrMap::const_iterator n = m_modulesPerPlatform.find( KnownPlatforms::AllPlatforms );
    if ( n != m_modulesPerPlatform.end() )
    {
        // A name was specified for all platforms which makes our job easy
        // an all platform name always counts as the general consensus name
        if ( !(*n).second->name.IsNULLOrEmpty() )
        {
            if ( GUCEF_NULL != moduleInfo )
                *moduleInfo = (*n).second;
            return m_consensusName = (*n).second->name;
        }
    }

    // If no name is specified for all platforms then we will have to
    // determine the best name to use. We do this by getting the name
    // for all platforms and counting how often each is used. The most used
    // name is considered the general consensus name. If the same count applies
    // to multiple we will try to use a popular platform to improve our 'guess'

    typedef GUCEF::map< CORE::CString, CORE::UInt32 > TStringCountMap;

    TStringCountMap countMap;
    n = m_modulesPerPlatform.begin();
    while ( n != m_modulesPerPlatform.end() )
    {
        const CModuleInfoPtr& mInfo = (*n).second;
        if ( !mInfo->name.IsNULLOrEmpty() )
        {
            TStringCountMap::iterator m = countMap.find( mInfo->name );
            if ( m != countMap.end() )
            {
                ++((*m).second);
            }
            else
            {
                countMap[ mInfo->name ] = 1;
            }
        }
        ++n;
    }

    if ( countMap.empty() )
    {
        // This should not happen:
        // Given that we have no module definitions for ANY platform and yet have a higher level concept of it...
        // We promised we would always return a module name. The only thing we have at this point is the directory path
        // As such we will assume the dir is named after the module it houses as our last ditch guess
        return m_consensusName = CORE::LastSubDir( GetAbsolutePathToModuleRootDir() );
    }

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
    if ( NULL != moduleInfo )
    {
        // Now turn the consensus name back into a module pointer
        n = m_modulesPerPlatform.begin();
        while ( n != m_modulesPerPlatform.end() )
        {
            if ( (*n).second->name == consensusName )
            {
                *moduleInfo = (*n).second;
                break;
            }
            ++n;
        }
    }
    return m_consensusName = consensusName;
}

/*---------------------------------------------------------------------------*/

CORE::CString
CModuleInfoEntry::GetModuleTargetName( const CORE::CString& targetPlatform ,
                                       bool useModuleNameIfNoTargetName    ) const
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindModuleInfoForPlatform( targetPlatform );
    if ( !moduleInfo.IsNULL() )
    {
        if ( !moduleInfo->linkerSettings.GetTargetName().IsNULLOrEmpty() )
        {
            return moduleInfo->linkerSettings.GetTargetName();
        }
    }
    if ( targetPlatform != KnownPlatforms::AllPlatforms && !targetPlatform.IsNULLOrEmpty() )
    {
        moduleInfo = FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms );
        if ( !moduleInfo.IsNULL() )
        {
            if ( !moduleInfo->linkerSettings.GetTargetName().IsNULLOrEmpty() )
            {
                return moduleInfo->linkerSettings.GetTargetName();
            }
        }
    }

    // If we got here no target name was defined so the only option left is the 
    // module name. For most projects this is the same thing.
    if ( useModuleNameIfNoTargetName )
    {
        return GetModuleNameAlways( targetPlatform );
    }

    return CORE::CString(); 
}

/*---------------------------------------------------------------------------*/

const CORE::CString*
CModuleInfoEntry::GetModuleName( const CORE::CString& targetPlatform ,
                                 CModuleInfoPtr* outModuleInfo       ) const
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator n = m_modulesPerPlatform.find( targetPlatform );
    if ( n != m_modulesPerPlatform.end() )
    {
        // A module was specified for this platform
        // Just because we have a module definition doens't mean we have a name
        if ( !(*n).second->name.IsNULLOrEmpty() )
        {
            // We have a name for this specific platform
            if ( GUCEF_NULL != outModuleInfo )
            {
                *outModuleInfo = (*n).second;
            }
            return &( (*n).second->name );
        }
    }

    // If no name is specified for a specific platform then there might still be a
    // default for all platforms
    if ( targetPlatform != KnownPlatforms::AllPlatforms )
    {
        n = m_modulesPerPlatform.find( KnownPlatforms::AllPlatforms );
        if ( n != m_modulesPerPlatform.end() )
        {
            // An 'AllPlatforms' definition is available for this module
            // Just because we have a module definition doesn't mean we have a name
            if ( !(*n).second->name.IsNULLOrEmpty() )
            {
                // We have a name for this specific platform
                if ( GUCEF_NULL != outModuleInfo )
                {
                    *outModuleInfo = (*n).second;
                }
                return &( (*n).second->name );
            }
        }
    }
    return GUCEF_NULL;
}

/*---------------------------------------------------------------------------*/

CORE::CString
CModuleInfoEntry::GetModuleNameAlways( const CORE::CString& targetPlatform ,
                                       CModuleInfoPtr* moduleInfo          ) const
{GUCEF_TRACE;

    const CORE::CString* strPtr = GetModuleName( targetPlatform, moduleInfo );
    if ( GUCEF_NULL == strPtr )
    {
        return GetConsensusName( moduleInfo );
    }
    return *strPtr;
}

/*---------------------------------------------------------------------------*/

CModuleInfoEntry&
CModuleInfoEntry::operator=( const CModuleInfoEntry& src ) 
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_isBroken = src.m_isBroken;
        m_modulesPerPlatform = src.m_modulesPerPlatform;
        m_flattenedInfoPerPlatform = src.m_flattenedInfoPerPlatform;
        m_absRootDir = src.m_absRootDir;
        m_projRelRootDir = src.m_projRelRootDir;
        m_definitionFileLastModifiedDt = src.m_definitionFileLastModifiedDt;
        m_consensusName = src.m_consensusName;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

// Serialize a specific moduleInfo which is derived off of
// the given entry. The given moduleInfo can be an independent platform
// moduleInfo or a merged moduleInfo
bool
SerializeModuleInfo( const CModuleInfoEntryPtr& moduleEntry ,
                     const CModuleInfoPtr& moduleInfo       ,
                     const CORE::CString& platform          ,
                     CORE::CDataNode& parentNode            )
{GUCEF_TRACE;

    bool totalSuccess = true;

    // Add basic module info
    CORE::CDataNode moduleInfoNode;
    moduleInfoNode.SetName( "Module" );
    
    if ( !moduleInfo->name.IsNULLOrEmpty() )
    {
        moduleInfoNode.SetAttribute( "Name", moduleInfo->name );
    }

    if ( moduleInfo->metadata.HasAnyMetaData() )
    {
        CORE::CDataNode* metaDataNode = moduleInfoNode.AddChild( "ModuleMetaData" );
        if ( GUCEF_NULL != metaDataNode )
        {
            CORE::CDataNodeSerializableSettings defaultSettings;
            totalSuccess = moduleInfo->metadata.Serialize( *metaDataNode, defaultSettings ) && totalSuccess;
        }
    }

    if ( moduleInfo->buildOrder != -1 )
    {
        moduleInfoNode.SetAttribute( "BuildOrder", moduleInfo->buildOrder );
    }

    if ( moduleInfo->moduleType != MODULETYPE_UNDEFINED )
    {
        moduleInfoNode.SetAttribute( "Type", ModuleTypeToString( moduleInfo->moduleType ) );
    }
    if ( !moduleInfo->tags.empty() )
    {
        UInt32 i=0;
        CORE::CString tagValue;
        TStringSet::iterator s = moduleInfo->tags.begin();
        while ( s != moduleInfo->tags.end() )
        {                     
            tagValue += (*s) + ';';
            ++s;
        }
        if ( !tagValue.IsNULLOrEmpty() )
            tagValue = tagValue.CutChars( 1, false, 0 );
        moduleInfoNode.SetAttribute( "Tags", tagValue );
    }

    moduleInfoNode.SetAttribute( "Platform", platform );

    // Add headers
    if ( moduleInfo->GetIncludeDirs().size() > 0 )
    {
        CORE::CDataNode headersInfoNode;
        headersInfoNode.SetName( "Files" );
        headersInfoNode.SetAttribute( "Type", "Headers" );
        headersInfoNode.SetAttribute( "DirCount", CORE::ToString( moduleInfo->GetIncludeDirs().size() ) );
        TStringSetMap::const_iterator n = moduleInfo->GetIncludeDirs().begin();
        while ( n != moduleInfo->GetIncludeDirs().end() )
        {
            CORE::CDataNode pathNode;
            pathNode.SetName( "Dir" );
            pathNode.SetAttribute( "Path", (*n).first );

            CORE::CDataNode fileNode;
            fileNode.SetName( "File" );

            const TStringSet& fileSet = (*n).second;
            pathNode.SetAttribute( "FileCount", CORE::ToString( fileSet.size() ) );
            TStringSet::const_iterator m = fileSet.begin();
            while ( m != fileSet.end() )
            {
                fileNode.SetAttribute( "Name", (*m) );
                pathNode.AddChild( fileNode );
                ++m;
            }

            // Don't add dirs that have no files in them
            // These should not be present in our data in the first place. But just in case,...
            if ( fileSet.size() > 0 )
            {
                headersInfoNode.AddChild( pathNode );
            }
            ++n;
        }

        moduleInfoNode.AddChild( headersInfoNode );
        headersInfoNode.DelSubTree();
    }

    // Add sources
    if ( moduleInfo->GetSourceDirs().size() > 0 )
    {
        CORE::CDataNode sourceInfoNode;
        sourceInfoNode.SetName( "Files" );
        sourceInfoNode.SetAttribute( "Type", "Source" );
        sourceInfoNode.SetAttribute( "DirCount", CORE::ToString( moduleInfo->GetSourceDirs().size() ) );
        TStringSetMap::const_iterator n = moduleInfo->GetSourceDirs().begin();
        while ( n != moduleInfo->GetSourceDirs().end() )
        {
            CORE::CDataNode pathNode;
            pathNode.SetName( "Dir" );
            pathNode.SetAttribute( "Path", (*n).first );

            CORE::CDataNode fileNode;
            fileNode.SetName( "File" );

            const TStringSet& fileSet = (*n).second;
            pathNode.SetAttribute( "FileCount", CORE::ToString( fileSet.size() ) );
            TStringSet::const_iterator m = fileSet.begin();
            while ( m != fileSet.end() )
            {
                fileNode.SetAttribute( "Name", (*m) );
                pathNode.AddChild( fileNode );
                ++m;
            }

            // Don't add dirs that have no files in them
            // These should not be present in our data in the first place. But just in case,...
            if ( fileSet.size() > 0 )
            {
                sourceInfoNode.AddChild( pathNode );
            }
            ++n;
        }

        moduleInfoNode.AddChild( sourceInfoNode );
        sourceInfoNode.DelSubTree();
    }

    // Add include paths inherited from dependencies
    if ( moduleInfo->dependencyIncludeDirs.size() > 0 )
    {
        CORE::CDataNode includesInfoNode( "Includes" );
        includesInfoNode.SetAttribute( "Count", CORE::ToString( moduleInfo->dependencyIncludeDirs.size() ) );
        includesInfoNode.SetAttribute( "Source", "Dependency" );
        TStringSet::const_iterator q = moduleInfo->dependencyIncludeDirs.begin();
        while ( q != moduleInfo->dependencyIncludeDirs.end() )
        {
            CORE::CDataNode includeNode;
            includeNode.SetName( "Include" );
            includeNode.SetAttribute( "Path", (*q) );

            includesInfoNode.AddChild( includeNode );
            ++q;
        }

        moduleInfoNode.AddChild( includesInfoNode );
        includesInfoNode.DelSubTree();
    }

    // Add all the regular include dirs for this module
    // These are already represented in the path attribute of the files section
    // but for ease of processing and clarity they are provided again in the includes section
    if ( moduleInfo->GetIncludeDirs().size() > 0 )
    {
       CORE::CDataNode includesInfoNode( "Includes" );
        includesInfoNode.SetAttribute( "Count", CORE::ToString( moduleInfo->GetIncludeDirs().size() ) );
        includesInfoNode.SetAttribute( "Source", "Self" );
        TStringSetMap::const_iterator n = moduleInfo->GetIncludeDirs().begin();
        while ( n != moduleInfo->GetIncludeDirs().end() )
        {
            CORE::CString includeDir = (*n).first.ReplaceChar( '\\', '/' );
            if ( 0 != includeDir.Length() )
            {
                CORE::CDataNode includeNode;
                includeNode.SetName( "Include" );
                includeNode.SetAttribute( "Path", includeDir );
                includesInfoNode.AddChild( includeNode );
            }
            else
            {
                // Check if there is more then one include dir
                // If so we have create an include for an empty include dir
                // to ensure files in subdirs can include the file with the zero length
                // subdir.
                if ( 1 < moduleInfo->GetIncludeDirs().size() )
                {
                    CORE::CString includeDir = "../" + CORE::LastSubDir( moduleEntry->GetAbsolutePathToModuleRootDir() ) + " ";
                    CORE::CDataNode includeNode;
                    includeNode.SetName( "Include" );
                    includeNode.SetAttribute( "Path", includeDir );
                    includesInfoNode.AddChild( includeNode );
                }
            }
            ++n;
        }
        moduleInfoNode.AddChild( includesInfoNode );
        includesInfoNode.DelSubTree();
    }

    // Add all the module dependencies
    if ( moduleInfo->GetNamesOfDependencies().size() > 0 )
    {
        CORE::CDataNode dependenciesNode;
        dependenciesNode.SetName( "Dependencies" );
        dependenciesNode.SetAttribute( "Count", CORE::ToString( moduleInfo->GetNamesOfDependencies().size() ) );
        TStringSet::const_iterator m = moduleInfo->GetNamesOfDependencies().begin();
        while ( m != moduleInfo->GetNamesOfDependencies().end() )
        {
            CORE::CDataNode dependencyNode;
            dependencyNode.SetName( "Dependency" );
            dependencyNode.SetAttribute( "Name", (*m) );
            dependenciesNode.AddChild( dependencyNode );
            ++m;
        }
        moduleInfoNode.AddChild( dependenciesNode );
    }

    // Add all the logical module dependencies
    if ( moduleInfo->GetNamesOfLogicalDependencies().size() > 0 )
    {
        CORE::CDataNode dependenciesNode;
        dependenciesNode.SetName( "LogicalDependencies" );
        dependenciesNode.SetAttribute( "Count", CORE::ToString( moduleInfo->GetNamesOfLogicalDependencies().size() ) );
        TStringSet::const_iterator m = moduleInfo->GetNamesOfLogicalDependencies().begin();
        while ( m != moduleInfo->GetNamesOfLogicalDependencies().end() )
        {
            CORE::CDataNode dependencyNode;
            dependencyNode.SetName( "LogicalDependency" );
            dependencyNode.SetAttribute( "Name", (*m) );
            dependenciesNode.AddChild( dependencyNode );
            ++m;
        }
        moduleInfoNode.AddChild( dependenciesNode );
    }

    // Add all the runtime module dependencies
    if ( moduleInfo->runtimeDependencies.size() > 0 )
    {
        CORE::CDataNode dependenciesNode;
        dependenciesNode.SetName( "RuntimeDependencies" );
        dependenciesNode.SetAttribute( "Count", CORE::ToString( moduleInfo->GetNamesOfRuntimeDependencies().size() ) );
        TStringSet::const_iterator m = moduleInfo->GetNamesOfRuntimeDependencies().begin();
        while ( m != moduleInfo->GetNamesOfRuntimeDependencies().end() )
        {
            CORE::CDataNode dependencyNode;
            dependencyNode.SetName( "RuntimeDependency" );
            dependencyNode.SetAttribute( "Name", (*m) );
            dependenciesNode.AddChild( dependencyNode );
            ++m;
        }
        moduleInfoNode.AddChild( dependenciesNode );
    }

    // Add all the module preprocessor instructions
    if ( moduleInfo->preprocessorSettings.GetDefines().size() > 0 )
    {
        CORE::CDataNode preprocessorNode;
        preprocessorNode.SetName( "Preprocessor" );
        TStringSet::const_iterator m = moduleInfo->preprocessorSettings.GetDefines().begin();
        while ( m != moduleInfo->preprocessorSettings.GetDefines().end() )
        {
            CORE::CDataNode defineNode;
            defineNode.SetName( "Define" );
            defineNode.SetAttribute( "String", (*m) );
            preprocessorNode.AddChild( defineNode );
            ++m;
        }
        moduleInfoNode.AddChild( preprocessorNode );
    }

    CORE::CDataNodeSerializableSettings defaultSettings;
    moduleInfo->linkerSettings.Serialize( moduleInfoNode, defaultSettings );

    // Add all the info for this module to the overall project
    parentNode.AddChild( moduleInfoNode );

    return true;
}

/*---------------------------------------------------------------------------*/
               
bool 
CModuleInfoEntry::Serialize( CORE::CDataNode& domRootNode                        ,
                             const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    if ( CORE::CDataNodeSerializableSettings::DataNodeSerializableLod_MaximumDetails <= settings.levelOfDetail &&
         !m_absRootDir.IsNULLOrEmpty()                                                                          )
    {
        domRootNode.SetAttribute( "AbsRootDir", m_absRootDir.ReplaceChar( '\\', '/' ) );
    }
    if ( !m_projRelRootDir.IsNULLOrEmpty() )
    {
        domRootNode.SetAttribute( "ProjRelRootDir", m_projRelRootDir.ReplaceChar( '\\', '/' ) );
    }

    if ( m_definitionFileLastModifiedDt != CORE::CDateTime::PastMax )
    {
        domRootNode.SetAttribute( "DefFileLastModified", m_definitionFileLastModifiedDt.ToIso8601DateTimeString( true, true ) );    
    }

    TModuleInfoPtrMap::const_iterator i = m_modulesPerPlatform.begin();
    while ( i != m_modulesPerPlatform.end() )
    {
        const CORE::CString& platform = (*i).first;
        CModuleInfoPtr moduleInfo = (*i).second;

        if ( !SerializeModuleInfo( CreateBasicSharedPtr() ,
                                   moduleInfo             ,
                                   platform               ,
                                   domRootNode            ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "SerializeModuleInfo: Failed to serialize moduleInfo of module " + moduleInfo->name + " for platform " + platform );
            totalSuccess = false;
        }

        ++i;
    }
    
    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfoEntry::SetModuleInfo( CModuleInfoPtr moduleInfo     ,
                                 const CORE::CString& platform )
{GUCEF_TRACE;

    // First check if we already have a entry for this platform
    TModuleInfoPtrMap::iterator i = m_modulesPerPlatform.find( platform );
    if ( i != m_modulesPerPlatform.end() )
    {
        // Since we already have an entry for this platform we will merge the two
        CModuleInfoPtr preExistingModuleInfo = (*i).second;
        preExistingModuleInfo->Merge( moduleInfo );
    }
    else
    {
        m_modulesPerPlatform[ platform ] = moduleInfo;
        moduleInfo->SetPlatformName( platform );
    }
}

/*-------------------------------------------------------------------------*/

CModuleInfoPtr
CModuleInfoEntry::FindOrCreateModuleInfoForPlatform( const CORE::CString& platform ,
                                                     bool createNewIfNoneExists    )
{GUCEF_TRACE;

    TModuleInfoPtrMap::iterator i = m_modulesPerPlatform.find( platform.Lowercase() );
    if ( i != m_modulesPerPlatform.end() )
    {
        return (*i).second;
    }
    if ( createNewIfNoneExists )
    {
        CModuleInfoPtr moduleInfo = CModuleInfo::CreateSharedObj();
        moduleInfo->SetPlatformName( platform );
        m_modulesPerPlatform[ platform ] = moduleInfo;
        return moduleInfo;
    }
    return CModuleInfoPtr();
}

/*-------------------------------------------------------------------------*/

const CModuleInfoPtr
CModuleInfoEntry::FindModuleInfoForPlatform( const CORE::CString& platform      ,
                                             bool considerAllPlatformAsFallback ) const
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator i = m_modulesPerPlatform.find( platform.Lowercase() );
    if ( i != m_modulesPerPlatform.end() )
    {
        return (*i).second;
    }

    if ( considerAllPlatformAsFallback && ( platform != KnownPlatforms::AllPlatforms && !platform.IsNULLOrEmpty() ) )
    {
        TModuleInfoPtrMap::const_iterator i = m_modulesPerPlatform.find( KnownPlatforms::AllPlatforms );
        if ( i != m_modulesPerPlatform.end() )
        {
            return (*i).second;
        }
    }
    return CModuleInfoPtr();
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::FindAllModuleInfoWithModuleType( TModuleType moduleType       ,
                                                   TModuleInfoPtrSet& foundInfo ) const
{GUCEF_TRACE;

    // We may not have a platform specific module type defined (fairly typical)
    CModuleInfoPtr allPlatformsDef;
    TModuleInfoPtrMap::const_iterator i = m_modulesPerPlatform.find( KnownPlatforms::AllPlatforms );
    if ( i != m_modulesPerPlatform.end() )
    {
        const CModuleInfoPtr& info = (*i).second;
        if GUCEF_PREDICT_TRUE( !info.IsNULL() )
        {
            if ( info->moduleType == moduleType )
            {
                allPlatformsDef = info;
            }
        }
    }

    // no luck, try the other platforms, if any
    i = m_modulesPerPlatform.begin();
    while ( i != m_modulesPerPlatform.end() )
    {
        const CORE::CString& targetPlatform = (*i).first;
        if ( KnownPlatforms::AllPlatforms != targetPlatform )
        {
            const CModuleInfoPtr& info = (*i).second;
            if GUCEF_PREDICT_TRUE( !info.IsNULL() )
            {
                TModuleType platformModuleType = info->moduleType;
                if ( MODULETYPE_UNDEFINED == platformModuleType )
                {
                    // When undefined the 'all' platforms value overules
                    if ( !allPlatformsDef.IsNULL() )
                    {
                        // we count this one because the 'all' platform has the module type desired and
                        // we have a platform specific definition but it doesnt override the module type as defined by
                        // the 'all' platform definition hence it logically has the type we are looking for
                        foundInfo.insert( info );
                    }
                }
                else
                if ( platformModuleType == moduleType )
                {
                    foundInfo.insert( info );
                }
            }
        }
        ++i;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::HasAnyModuleInfoWithModuleType( TModuleType moduleType ) const
{GUCEF_TRACE;

    TModuleInfoPtrSet foundInfo;
    FindAllModuleInfoWithModuleType( moduleType, foundInfo );
    return !foundInfo.empty();
}

/*-------------------------------------------------------------------------*/

CModuleInfoPtr
CModuleInfoEntry::FindModuleInfoWithModuleType( TModuleType moduleType             ,
                                                const CORE::CString& platform      ,
                                                bool considerAllPlatformAsFallback ) const
{GUCEF_TRACE;

    // Considering a specific platform is given, try that one first
    TModuleInfoPtrMap::const_iterator i = m_modulesPerPlatform.find( platform );
    if ( i != m_modulesPerPlatform.end() )
    {
        const CModuleInfoPtr& info = (*i).second;
        if GUCEF_PREDICT_TRUE( !info.IsNULL() )
        {
            if ( info->moduleType == moduleType )
            {
                return info;
            }
        }
    }

    // no luck, check if we can try the 'all' platform
    if ( considerAllPlatformAsFallback && ( platform != KnownPlatforms::AllPlatforms && !platform.IsNULLOrEmpty() ) )
    {
        i = m_modulesPerPlatform.find( KnownPlatforms::AllPlatforms );
        if ( i != m_modulesPerPlatform.end() )
        {
            const CModuleInfoPtr& info = (*i).second;
            if GUCEF_PREDICT_TRUE( !info.IsNULL() )
            {
                if ( info->moduleType == moduleType )
                {
                    return info;
                }
            }
        }
    }
    return CModuleInfoPtr();
}

/*-------------------------------------------------------------------------*/

const TModuleInfoPtrMap&
CModuleInfoEntry::GetModulesPerPlatform( void ) const
{GUCEF_TRACE;

    return m_modulesPerPlatform;
}

/*-------------------------------------------------------------------------*/

const TModuleInfoPtrMap&
CModuleInfoEntry::GetFlattenedModulesPerPlatform( void ) const
{GUCEF_TRACE;

    return m_flattenedInfoPerPlatform;
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::HasDependency( const CORE::CString& platform       ,
                                 const CORE::CString& dependencyName ,
                                 bool platformSpecificOnly           ) const
{GUCEF_TRACE;

    // Check for a platform specific dependency
    TModuleInfoPtrMap::const_iterator n = m_modulesPerPlatform.find( platform );
    if ( n != m_modulesPerPlatform.end() )
    {
        // This module has info for for this module, check its type
        const CModuleInfoPtr& moduleInfo = (*n).second;
        if ( IsStringInList( moduleInfo->GetNamesOfDependencies(), false, dependencyName ) )
        {
            return true;
        }
    }

    // Alternatively check the 'all platforms' definition
    if ( !platformSpecificOnly && platform != KnownPlatforms::AllPlatforms && !platform.IsNULLOrEmpty() )
    {
        n = m_modulesPerPlatform.find( KnownPlatforms::AllPlatforms );
        if ( n != m_modulesPerPlatform.end() )
        {
            // This module has info for for this module, check its type
            const CModuleInfoPtr& moduleInfo = (*n).second;
            if ( IsStringInList( moduleInfo->GetNamesOfDependencies(), false, dependencyName ) )
            {
                return true;
            }
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::HasLogicalDependency( const CORE::CString& platform       ,
                                        const CORE::CString& dependencyName ,
                                        bool platformSpecificOnly           ) const
{GUCEF_TRACE;

    // Check for a platform specific dependency
    TModuleInfoPtrMap::const_iterator n = m_modulesPerPlatform.find( platform );
    if ( n != m_modulesPerPlatform.end() )
    {
        // This module has info for for this module, check its type
        const CModuleInfoPtr& moduleInfo = (*n).second;
        if ( moduleInfo->HasLogicalDependencyWithName( dependencyName ) )
        {
            return true;
        }
    }

    // Alternatively check the 'all platforms' definition
    if ( !platformSpecificOnly && platform != KnownPlatforms::AllPlatforms && !platform.IsNULLOrEmpty() )
    {
        n = m_modulesPerPlatform.find( KnownPlatforms::AllPlatforms );
        if ( n != m_modulesPerPlatform.end() )
        {
            // This module has info for for this module, check its type
            const CModuleInfoPtr& moduleInfo = (*n).second;
            if ( moduleInfo->HasLogicalDependencyWithName( dependencyName ) )
            {
                return true;
            }
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::HasLinkerDependency( const CORE::CString& platform       ,
                                       const CORE::CString& dependencyName ,
                                       bool platformSpecificOnly           ) const
{GUCEF_TRACE;

    // Check for a platform specific dependency
    TModuleInfoPtrMap::const_iterator n = m_modulesPerPlatform.find( platform );
    if ( n != m_modulesPerPlatform.end() )
    {
        // This module has info for for this module, check its type
        const CModuleInfoPtr& moduleInfo = (*n).second;
        if ( moduleInfo->linkerSettings.HasLinkerDependency( dependencyName ) )
        {
            return true;
        }
    }

    // Alternatively check the 'all platforms' definition
    if ( !platformSpecificOnly && platform != KnownPlatforms::AllPlatforms && !platform.IsNULLOrEmpty() )
    {
        n = m_modulesPerPlatform.find( KnownPlatforms::AllPlatforms );
        if ( n != m_modulesPerPlatform.end() )
        {
            // This module has info for for this module, check its type
            const CModuleInfoPtr& moduleInfo = (*n).second;
            if ( moduleInfo->linkerSettings.HasLinkerDependency( dependencyName ) )
            {
                return true;
            }
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::IsApplicableForPlatform( const CORE::CString& platform ,
                                           bool autoConsiderAllPlatforms ) const
{GUCEF_TRACE;

    // Check for a platform specific definition
    TModuleInfoPtrMap::const_iterator n = m_modulesPerPlatform.find( platform );
    if ( n != m_modulesPerPlatform.end() )
    {
        // This module has info for for this module, check its type
        const CModuleInfoPtr& moduleInfo = (*n).second;
        if GUCEF_PREDICT_TRUE( !moduleInfo.IsNULL() )
        {
            if ( moduleInfo->HasValidModuleType() )
            {
                return true;
            }
        }
    }

    // Alternatively check the 'all platforms' definition
    if ( autoConsiderAllPlatforms && ( platform != KnownPlatforms::AllPlatforms && !platform.IsNULLOrEmpty() ) )
    {
        n = m_modulesPerPlatform.find( KnownPlatforms::AllPlatforms );
        if ( n != m_modulesPerPlatform.end() )
        {
            // This module has info for for this module, check its type
            const CModuleInfoPtr& moduleInfo = (*n).second;
            if GUCEF_PREDICT_TRUE( !moduleInfo.IsNULL() )
            {
                if ( moduleInfo->HasValidModuleType() )
                {
                    return true;
                }
            }
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::HasIndependentModuleTypeForPlatform( const CORE::CString& platform ) const
{GUCEF_TRACE;

    // Check for a platform specific definition
    TModuleInfoPtrMap::const_iterator n = m_modulesPerPlatform.find( platform );
    if ( n != m_modulesPerPlatform.end() )
    {
        // This module has info for for this module, check its type
        const CModuleInfoPtr& moduleInfo = (*n).second;
        if GUCEF_PREDICT_TRUE( !moduleInfo.IsNULL() )
        {
            if ( moduleInfo->HasIndependentModuleType() )
            {
                return true;
            }
        }
    }

    // Alternatively check the 'all platforms' definition
    if ( platform != KnownPlatforms::AllPlatforms && !platform.IsNULLOrEmpty() )
    {
        n = m_modulesPerPlatform.find( KnownPlatforms::AllPlatforms );
        if ( n != m_modulesPerPlatform.end() )
        {
            // This module has info for for this module, check its type
            const CModuleInfoPtr& moduleInfo = (*n).second;
            if GUCEF_PREDICT_TRUE( !moduleInfo.IsNULL() )
            {
                if ( moduleInfo->HasIndependentModuleType() )
                {
                    return true;
                }
            }
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::HasAnyLogicalModuleType( void ) const
{GUCEF_TRACE;

    bool foundSetNonLogical = false;
    TModuleInfoPtrMap::const_iterator n = m_modulesPerPlatform.begin();
    while ( n != m_modulesPerPlatform.end() )
    {
        // Check if its not just a placeholder, it has to be valid
        const CModuleInfoPtr& platformDefinition = (*n).second;
        if GUCEF_PREDICT_TRUE( !platformDefinition.IsNULL() )
        {
            if ( platformDefinition->HasValidModuleType() )
            {
                if ( platformDefinition->IsLogicalModuleType() )
                    return true;
            }
        }
        ++n;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::HasOnlyLogicalModuleType( void ) const
{GUCEF_TRACE;

    bool foundSetNonLogical = false;
    TModuleInfoPtrMap::const_iterator n = m_modulesPerPlatform.begin();
    while ( n != m_modulesPerPlatform.end() )
    {
        // Check if its not just a placeholder, it has to be valid
        const CModuleInfoPtr& platformDefinition = (*n).second;
        if GUCEF_PREDICT_TRUE( !platformDefinition.IsNULL() )
        {
            if ( platformDefinition->HasValidModuleType() )
            {
                if ( !platformDefinition->IsLogicalModuleType() )
                    foundSetNonLogical = true;
            }
        }
        ++n;
    }
    return !foundSetNonLogical;
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::HasAllPlatformsDefinition( bool onlyConsiderValidModulesTypes ) const
{GUCEF_TRACE;

    // Check for the AllPlatforms definition
    TModuleInfoPtrMap::const_iterator n = m_modulesPerPlatform.find( KnownPlatforms::AllPlatforms );
    if ( n != m_modulesPerPlatform.end() )
    {
        // Check if its not just a placeholder, it has to be valid
        const CModuleInfoPtr& allPlatformsDefinition = (*n).second;
        if ( !allPlatformsDefinition.IsNULL() && ( !onlyConsiderValidModulesTypes || ( onlyConsiderValidModulesTypes && allPlatformsDefinition->HasValidModuleType() ) ) )
        {
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CModuleInfoEntry::SetIsBroken( bool isBroken )
{GUCEF_TRACE;

    m_isBroken = isBroken;
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::IsBroken( void ) const
{GUCEF_TRACE;

    return m_isBroken;
}

/*-------------------------------------------------------------------------*/

void
CModuleInfoEntry::GetModuleDependencyNames( const CORE::CString& targetPlatform ,
                                            TStringSet& dependencies            ) const
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindModuleInfoForPlatform( targetPlatform );
    if ( !moduleInfo.IsNULL() )
    {
        MergeStringSet( dependencies, moduleInfo->GetNamesOfDependencies(), false );
    }
    if ( targetPlatform != KnownPlatforms::AllPlatforms && !targetPlatform.IsNULLOrEmpty() )
    {
        moduleInfo = FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms );
        if ( !moduleInfo.IsNULL() )
        {
            MergeStringSet( dependencies, moduleInfo->GetNamesOfDependencies(), false );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CModuleInfoEntry::GetLogicalDependencyNames( const CORE::CString& targetPlatform ,
                                             TStringSet& dependencies            ) const
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindModuleInfoForPlatform( targetPlatform );
    if ( !moduleInfo.IsNULL() )
    {
        MergeStringSet( dependencies, moduleInfo->GetNamesOfLogicalDependencies(), false );
    }
    if ( targetPlatform != KnownPlatforms::AllPlatforms && !targetPlatform.IsNULLOrEmpty() )
    {
        moduleInfo = FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms );
        if ( !moduleInfo.IsNULL() )
        {
            MergeStringSet( dependencies, moduleInfo->GetNamesOfLogicalDependencies(), false );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CModuleInfoEntry::GetModuleLinkerDependencyNames( const CORE::CString& targetPlatform ,
                                                  TStringSet& dependencies            ) const
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindModuleInfoForPlatform( targetPlatform );
    if ( !moduleInfo.IsNULL() )
    {
        moduleInfo->linkerSettings.GetListOfLinkedLibraries( dependencies );
    }
    if ( targetPlatform != KnownPlatforms::AllPlatforms && !targetPlatform.IsNULLOrEmpty() )
    {
        moduleInfo = FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms );
        if ( !moduleInfo.IsNULL() )
        {
            moduleInfo->linkerSettings.GetListOfLinkedLibraries( dependencies );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CModuleInfoEntry::GetModuleRuntimeDependencyNames( const CORE::CString& targetPlatform ,
                                                   TStringSet& dependencies            ) const
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindModuleInfoForPlatform( targetPlatform );
    if ( !moduleInfo.IsNULL() )
    {
        MergeStringSet( dependencies, moduleInfo->runtimeDependencies, false );
    }
    if ( targetPlatform != KnownPlatforms::AllPlatforms && !targetPlatform.IsNULLOrEmpty() )
    {
        moduleInfo = FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms );
        if ( !moduleInfo.IsNULL() )
        {
            MergeStringSet( dependencies, moduleInfo->runtimeDependencies, false );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CModuleInfoEntry::RemoveRuntimeDependency( const CORE::CString& dependencyName ,
                                           const CORE::CString& platformName   )
{GUCEF_TRACE;

    // Check for platform specific definitions
    TModuleInfoPtrMap::iterator n = m_modulesPerPlatform.find( platformName );
    if ( n != m_modulesPerPlatform.end() )
    {
        // This module has info for for this module, check its type
        CModuleInfoPtr& platformModuleDefinition = (*n).second;
        if ( !platformModuleDefinition.IsNULL() )
        {
            platformModuleDefinition->RemoveNameOfRuntimeDependency( dependencyName );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CModuleInfoEntry::RemoveRuntimeDependency( const CORE::CString& dependencyName )
{GUCEF_TRACE;

    // Check for platform specific definitions
    TModuleInfoPtrMap::iterator n = m_modulesPerPlatform.begin();
    while ( n != m_modulesPerPlatform.end() )
    {
        // This module has info for for this module, check its type
        const CORE::CString& platform = (*n).first;
        CModuleInfoPtr& platformModuleDefinition = (*n).second;
        if ( !platformModuleDefinition.IsNULL() )
        {
            platformModuleDefinition->RemoveNameOfRuntimeDependency( dependencyName );
        }
        
        ++n;
    }
}

/*-------------------------------------------------------------------------*/

void
CModuleInfoEntry::GetReferencedPlatforms( TStringSet& platforms ) const
{GUCEF_TRACE;

    // Check for platform specific definitions
    TModuleInfoPtrMap::const_iterator n = m_modulesPerPlatform.begin();
    while ( n != m_modulesPerPlatform.end() )
    {
        // This module has info for for this module, check its type
        const CORE::CString& platform = (*n).first; 
        platforms.insert( platform );
        ++n;
    }
}

/*---------------------------------------------------------------------------*/
               
bool 
CModuleInfoEntry::Serialize( const CORE::CString& outputFilePath                 ,
                             const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec = GetXmlDStoreCodec();
    if ( !codec.IsNULL() )
    {
        CORE::CDataNode info;
        if ( Serialize( info, settings ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ModuleInfoEntry:Serialize: Successfully generated a data tree with all module information" );

            if ( codec->StoreDataTree( &info, outputFilePath ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ModuleInfoEntry:Serialize: Successfully wrote all module information to disk file \"" + outputFilePath + "\"" );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ModuleInfoEntry:Serialize: Failed to store the serialized module information to disk at " + outputFilePath );
                return false;
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ModuleInfoEntry:Serialize: Failed to serialize the module information" );
            return false;
        }
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ModuleInfoEntry:Serialize: Cannot serialize since no codec is registered that can be used for serialization" );
    return false;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfoEntry::Deserialize( const CORE::CDataNode& domRootNode                  ,
                               const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    m_absRootDir = domRootNode.GetAttributeValue( "AbsRootDir", m_absRootDir ).AsString( m_absRootDir, true );
    m_projRelRootDir = domRootNode.GetAttributeValue( "ProjRelRootDir", m_projRelRootDir ).AsString( m_projRelRootDir, true );
    m_definitionFileLastModifiedDt = domRootNode.GetAttributeValue( "DefFileLastModified", m_definitionFileLastModifiedDt ).AsDateTime( m_definitionFileLastModifiedDt, true );    

    const CORE::CDataNode::TConstDataNodeSet moduleInfoNodes = domRootNode.FindChildrenOfType( "Module" );
    if ( moduleInfoNodes.size() == 0 ) 
        return false;

    CORE::CDataNode::TConstDataNodeSet::const_iterator n = moduleInfoNodes.begin();
    while ( n != moduleInfoNodes.end() )
    {
        CModuleInfoPtr moduleInfoForPlatform = CModuleInfo::CreateSharedObj();
        const CORE::CDataNode* moduleNode = (*n);

        if ( moduleInfoForPlatform->Deserialize( *moduleNode, settings ) )
        {
            if ( moduleInfoForPlatform->name.IsNULLOrEmpty() )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully deserialized module definition for module with no name specified for the applicable platforms" );
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully deserialized module definition for module with name " + moduleInfoForPlatform->name );
            }

            // Get all platforms for which this info applies.
            // Keep in mind that multiple platforms can be specified for ease of use.
            // This feature requires platform entries to be separated by a ';'
            TStringVector platforms = moduleNode->GetAttributeValue( "Platform" ).AsString().Lowercase().ParseElements( ';', false);

            if ( platforms.empty() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Failed to locate a Platform value for a module, will default to all platforms but this may not be correct" );
                platforms.push_back( KnownPlatforms::AllPlatforms );
            }

            TStringVector::iterator i = platforms.begin();
            while ( i != platforms.end() )
            {
                // @TODO: find a cleaner way to resolve this vs userdata
                CProjectInfo* projectInfo = static_cast<CProjectInfo*>( settings.GetUserData() );  
                if ( GUCEF_NULL == projectInfo )
                    return false;

                const CORE::CString& multiPlatformName = (*i); 

                TStringSet actualPlatforms = ResolveMultiPlatformName( multiPlatformName, &projectInfo->platforms );
                TStringSet::iterator n = actualPlatforms.begin();
                while ( n != actualPlatforms.end() )
                {
                    const CORE::CString& actualPlatformName = (*n);

                    if ( multiPlatformName != actualPlatformName )
                    {
                        CModuleInfoPtr actualModuleInfoForPlatform = CModuleInfo::CreateSharedObjWithParam( *moduleInfoForPlatform );
                        actualModuleInfoForPlatform->SetPlatformName( actualPlatformName );
                        SetModuleInfo( actualModuleInfoForPlatform, actualPlatformName );
                    }
                    else
                    {
                        SetModuleInfo( moduleInfoForPlatform, actualPlatformName );
                    }
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Adding module definition for platform " + actualPlatformName );
                    
                    ++n;
                }                

                ++i;
            }
        }
        else
        {
            return false;
        }
        ++n;
    }

    return true;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfoEntry::Merge( const CModuleInfoEntryPtr& infoToMergeIn ,
                         bool onConflictOriginalInfoStays         ) 
{GUCEF_TRACE;

    bool totalSuccess = true;

    if ( m_absRootDir.IsNULLOrEmpty() )
        m_absRootDir = infoToMergeIn->m_absRootDir;
    else
    if ( !onConflictOriginalInfoStays && !infoToMergeIn->m_absRootDir.IsNULLOrEmpty() )
        m_absRootDir = infoToMergeIn->m_absRootDir;

    if ( m_projRelRootDir.IsNULLOrEmpty() )
        m_projRelRootDir = infoToMergeIn->m_projRelRootDir;
    else
    if ( !onConflictOriginalInfoStays && !infoToMergeIn->m_projRelRootDir.IsNULLOrEmpty() )
        m_projRelRootDir = infoToMergeIn->m_projRelRootDir;

    TModuleInfoPtrMap::const_iterator i = infoToMergeIn->m_modulesPerPlatform.begin();
    while ( i != infoToMergeIn->m_modulesPerPlatform.end() )
    {
        const CString& platformToMergeIn = (*i).first;
        const CModuleInfoPtr& moduleInfoToMergeIn = (*i).second;

        TModuleInfoPtrMap::iterator e = m_modulesPerPlatform.find( platformToMergeIn );
        if ( e != m_modulesPerPlatform.end() )
        {
            // We already have info for this platform
            // we need to merge
            CModuleInfoPtr& moduleInfo = (*e).second;
            totalSuccess = moduleInfo->Merge( moduleInfoToMergeIn ) && totalSuccess;

            ++e;
        }
        else
        {
            // No entry exists yet for this platform so we can just do a simple insert
            m_modulesPerPlatform[ platformToMergeIn ] = moduleInfoToMergeIn;
        }

        ++i;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CModuleInfoEntry::GenerateFlattenedModuleInfo( const CORE::CString& targetPlatform )
{GUCEF_TRACE;

    const CModuleInfoPtr allPlatformsInfo = FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms );
    const CModuleInfoPtr targetPlatformInfo = FindModuleInfoForPlatform( targetPlatform );

    if ( ( !allPlatformsInfo.IsNULL() ) || ( !targetPlatformInfo.IsNULL() ) )
    {
        // Check if we have both
        if ( !allPlatformsInfo.IsNULL() && !targetPlatformInfo.IsNULL() )
        {
            // Use the 'all' platform as a base to work from
            CModuleInfoPtr flattenedModuleInfo = CModuleInfo::CreateSharedObjWithParam( *allPlatformsInfo );

            // Now merge in the platform specific info
            if ( flattenedModuleInfo->Merge( targetPlatformInfo ) )
            {
                m_flattenedInfoPerPlatform[ targetPlatform ] = flattenedModuleInfo;
                return true;
            }
            return false;
        }
        else
        if ( allPlatformsInfo.IsNULL() && !targetPlatformInfo.IsNULL() )
        {
            // We only have platform specific info so that is by definition the flattened info
            CModuleInfoPtr flattenedModuleInfo = CModuleInfo::CreateSharedObjWithParam( *targetPlatformInfo );
            m_flattenedInfoPerPlatform[ targetPlatform ] = flattenedModuleInfo;
            return true;
        }
        else
        if ( !allPlatformsInfo.IsNULL() && targetPlatformInfo.IsNULL() )
        {
            // We only have the 'all platforms' specific info so that is by definition the flattened info
            CModuleInfoPtr flattenedModuleInfo = CModuleInfo::CreateSharedObjWithParam( *allPlatformsInfo );
            m_flattenedInfoPerPlatform[ targetPlatform ] = flattenedModuleInfo;
            return true;
        }
    }

    // This module should not be used since it doesn't have platform specific info
    // nor info which applies to all platforms.
    return false;
}

/*---------------------------------------------------------------------------*/

const CModuleInfoPtr
CModuleInfoEntry::FindFlattenedModuleInfoForPlatform( const CORE::CString& platform ) const
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator i = m_flattenedInfoPerPlatform.find( platform );
    if ( i != m_flattenedInfoPerPlatform.end() )
    {
        return (*i).second;
    }
    return CModuleInfoPtr();
}

/*---------------------------------------------------------------------------*/

const CModuleInfoPtr
CModuleInfoEntry::FindOrCreateFlattenedModuleInfoForPlatform( const CORE::CString& platform )
{GUCEF_TRACE;

    const CModuleInfoPtr flatInfo = FindFlattenedModuleInfoForPlatform( platform );
    if ( !flatInfo.IsNULL() )
        return flatInfo;

    if ( GenerateFlattenedModuleInfo( platform ) )
    {
        return FindFlattenedModuleInfoForPlatform( platform );
    }
    return CModuleInfoPtr();
}

/*---------------------------------------------------------------------------*/

void
CModuleInfoEntry::GetSourceFilesForPlatform( const CORE::CString& platformName ,
                                             TStringSetMap& files              ,
                                             bool autoConsiderAllPlatforms     ) const
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindModuleInfoForPlatform( platformName );
    if ( !moduleInfo.IsNULL() )
    {
        MergeStringSetMap( files, moduleInfo->GetSourceDirs(), false );
    }
    if ( autoConsiderAllPlatforms && ( platformName != KnownPlatforms::AllPlatforms && !platformName.IsNULLOrEmpty() ) )
    {
        moduleInfo = FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms );
        if ( !moduleInfo.IsNULL() )
        {
            MergeStringSetMap( files, moduleInfo->GetSourceDirs(), false );
        }
    }
}

/*---------------------------------------------------------------------------*/

void
CModuleInfoEntry::GetIncludeFilesForPlatform( const CORE::CString& platformName ,
                                              TStringSetMap& files              ,
                                              bool autoConsiderAllPlatforms     ) const
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindModuleInfoForPlatform( platformName );
    if ( !moduleInfo.IsNULL() )
    {
        MergeStringSetMap( files, moduleInfo->GetIncludeDirs(), false );
    }
    if ( autoConsiderAllPlatforms && ( platformName != KnownPlatforms::AllPlatforms && !platformName.IsNULLOrEmpty() ) )
    {
        moduleInfo = FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms );
        if ( !moduleInfo.IsNULL() )
        {
            MergeStringSetMap( files, moduleInfo->GetIncludeDirs(), false );
        }
    }
}

/*---------------------------------------------------------------------------*/

void
CModuleInfoEntry::GetDependencyIncludeDirsForPlatform( const CORE::CString& platformName ,
                                                       TStringSet& subDirPaths           ,
                                                       bool autoConsiderAllPlatforms     ) const
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindModuleInfoForPlatform( platformName );
    if ( !moduleInfo.IsNULL() )
    {
        subDirPaths.insert( moduleInfo->dependencyIncludeDirs.begin(), moduleInfo->dependencyIncludeDirs.end() );
    }
    if ( autoConsiderAllPlatforms && ( platformName != KnownPlatforms::AllPlatforms && !platformName.IsNULLOrEmpty() ) )
    {
        moduleInfo = FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms );
        if ( !moduleInfo.IsNULL() )
        {
           subDirPaths.insert( moduleInfo->dependencyIncludeDirs.begin(), moduleInfo->dependencyIncludeDirs.end() );
        }
    }
}

/*---------------------------------------------------------------------------*/

void
CModuleInfoEntry::GetIncludeDirsForPlatform( const CORE::CString& platformName ,
                                             TStringSet& subDirPaths           ,
                                             bool autoConsiderAllPlatforms     ,
                                             bool includeDependencyIncludes    ) const
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindModuleInfoForPlatform( platformName );
    if ( !moduleInfo.IsNULL() )
    {
        const TStringSetMap& includes = moduleInfo->GetIncludeDirs();
        TStringSetMap::const_iterator i = includes.begin();
        while ( i != includes.end() )
        {
            const CORE::CString& subPath = (*i).first;
            subDirPaths.insert( subPath );
            ++i;
        }

        if ( includeDependencyIncludes )
            subDirPaths.insert( moduleInfo->dependencyIncludeDirs.begin(), moduleInfo->dependencyIncludeDirs.end() );
    }
    if ( autoConsiderAllPlatforms && ( platformName != KnownPlatforms::AllPlatforms && !platformName.IsNULLOrEmpty() ) )
    {
        moduleInfo = FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms );
        if ( !moduleInfo.IsNULL() )
        {
            const TStringSetMap& includes = moduleInfo->GetIncludeDirs();
            TStringSetMap::const_iterator i = includes.begin();
            while ( i != includes.end() )
            {
                const CORE::CString& subPath = (*i).first;
                subDirPaths.insert( subPath );
                ++i;
            }

            if ( includeDependencyIncludes )
                subDirPaths.insert( moduleInfo->dependencyIncludeDirs.begin(), moduleInfo->dependencyIncludeDirs.end() );
        }
    }
}

/*---------------------------------------------------------------------------*/

void
CModuleInfoEntry::GetPathsToIncludeDirsForPlatform( const CORE::CString& platformName  ,
                                                    const CORE::CString& otherFromPath ,
                                                    TStringSet& pathsToSubDirPaths     ,
                                                    bool autoConsiderAllPlatforms      ) const
{GUCEF_TRACE;

    TStringSet localSubDirPaths;
    GetIncludeDirsForPlatform( platformName, localSubDirPaths, autoConsiderAllPlatforms );

    TStringSet::iterator i = localSubDirPaths.begin();
    while ( i != localSubDirPaths.end() )
    {
        const CORE::CString& subPath = (*i);

        if ( !CORE::IsAbsolutePath( subPath ) && 0 != subPath.HasSubstr( "#$#ENVVAR:", true ) )
        {
            CORE::CString fullLocalPath = CORE::CombinePath( m_absRootDir, subPath );
            CORE::CString relativePath = CORE::GetRelativePathToOtherPathRoot( otherFromPath, fullLocalPath, true ); 
            pathsToSubDirPaths.insert( relativePath );
        }
        else
        {
            pathsToSubDirPaths.insert( subPath );
        }

        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
CModuleInfoEntry::CleanupIncludeDirs( void )
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator i = m_modulesPerPlatform.begin();
    while ( i != m_modulesPerPlatform.end() )
    {
        CModuleInfoPtr moduleInfo = (*i).second;
        
        // Check for empty include dirs
        // If the include dir does not have include files as part of this module then 
        // the dir should have been a dependency include dir
        TStringSet dirs;
        TStringSetMap::const_iterator n = moduleInfo->GetIncludeDirs().begin();
        while ( n != moduleInfo->GetIncludeDirs().end() )
        {
            const TStringSet& filesInDirList = (*n).second;
            if ( filesInDirList.empty() )
            {
                dirs.insert( (*n).first );
            }
            ++n;
        }

        TStringSet::iterator m = dirs.begin();
        while ( m != dirs.end() )
        {
            moduleInfo->RemoveIncludeDir( (*m), true ); 
            moduleInfo->dependencyIncludeDirs.insert( (*m) );

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ModuleInfoEntry:CleanupIncludeDirs: Moved dir from module header dir to dependency include dir as it had no files: " + (*m) );

            ++m;
        }

        // Check for duplicates,.. dirs that are listed as 
        // dependency includes but which also have headers part of this module
        // as such its not a dir coming from a dependency
        n = moduleInfo->GetIncludeDirs().begin();
        while ( n != moduleInfo->GetIncludeDirs().end() )
        {
            TStringSet::iterator p = moduleInfo->dependencyIncludeDirs.find( (*n).first );
            if ( p != moduleInfo->dependencyIncludeDirs.end() )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ModuleInfoEntry:CleanupIncludeDirs: Deleting dependency include dir since it already exists as a module header dir: " + (*p) );                
                moduleInfo->dependencyIncludeDirs.erase( p );
            }            
            ++n;
        }
                
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

void
CModuleInfoEntry::SetAbsolutePathToModuleRootDir( const CORE::CString& absPathToRootDir )
{GUCEF_TRACE;

    m_absRootDir = absPathToRootDir;
}

/*---------------------------------------------------------------------------*/

const CORE::CString&
CModuleInfoEntry::GetAbsolutePathToModuleRootDir( void ) const
{GUCEF_TRACE;

    return m_absRootDir;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfoEntry::SetProjectRelativePathToModuleRootDir( const CORE::CString& relPathToRootDir )
{GUCEF_TRACE;

    m_projRelRootDir = relPathToRootDir;
}

/*---------------------------------------------------------------------------*/

const CORE::CString&
CModuleInfoEntry::GetProjectRelativePathToModuleRootDir( void ) const
{GUCEF_TRACE;

    return m_projRelRootDir;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfoEntry::SetDefinitionFileLastModifiedDt( const CORE::CDateTime& fileLastModifiedDt )
{GUCEF_TRACE;

    m_definitionFileLastModifiedDt = fileLastModifiedDt;
}

/*---------------------------------------------------------------------------*/

const CORE::CDateTime&
CModuleInfoEntry::GetDefinitionFileLastModifiedDt( void ) const
{GUCEF_TRACE;

    return m_definitionFileLastModifiedDt;
}

/*---------------------------------------------------------------------------*/

CORE::CICloneable* 
CModuleInfoEntry::Clone( void ) const 
{GUCEF_TRACE;

    return new CModuleInfoEntry( *this );
}

/*---------------------------------------------------------------------------*/

const CORE::CString& 
CModuleInfoEntry::GetClassTypeName( void ) const 
{GUCEF_TRACE;

    return ClassTypeName;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfoEntry::SetLicense( const CORE::CString& license  ,
                              const CORE::CString& platform )
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindOrCreateModuleInfoForPlatform( platform, true );
    if ( !moduleInfo.IsNULL() )
    {
        moduleInfo->metadata.license = license;
        return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfoEntry::IsAnyLicenseDefined( void ) const
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator i = m_modulesPerPlatform.begin();
    while ( i != m_modulesPerPlatform.end() )
    {
        const CModuleInfoPtr& moduleInfo = (*i).second;
        if ( !moduleInfo.IsNULL() )
        {
            if ( !moduleInfo->metadata.license.IsNULLOrEmpty() )
                return true;
        }

        ++i;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfoEntry::SetSemVer( const CORE::CVersion& semver  ,
                             const CORE::CString& platform )
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindOrCreateModuleInfoForPlatform( platform, true );
    if ( !moduleInfo.IsNULL() )
    {
        moduleInfo->metadata.SetSemVer( semver );
        return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfoEntry::GetSemVer( CORE::CVersion& semver        ,
                             const CORE::CString& platform )
{GUCEF_TRACE;

    bool foundNonZeroSemVer = false;
    CModuleInfoPtr moduleInfo = FindModuleInfoForPlatform( platform, false );
    if ( !moduleInfo.IsNULL() )
    {
        semver = moduleInfo->metadata.GetSemVer();
        foundNonZeroSemVer = !semver.IsAllZero();
    }

    if ( !foundNonZeroSemVer )
    {
        moduleInfo = FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms, false );
        if ( !moduleInfo.IsNULL() )
        {
            semver = moduleInfo->metadata.GetSemVer();
            foundNonZeroSemVer = !semver.IsAllZero();
        }
    }
    return foundNonZeroSemVer;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfoEntry::HasAnySemVer( void ) const
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator i = m_modulesPerPlatform.begin();
    while ( i != m_modulesPerPlatform.end() )
    {
        const CModuleInfoPtr& moduleInfo = (*i).second;
        if ( !moduleInfo.IsNULL() )
        {
            if ( moduleInfo->metadata.HasSemVer() )
                return true;
        }

        ++i;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfoEntry::SetLastEditBy( const CORE::CString& lastEditBy ,
                                 const CORE::CString& platform   )
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindOrCreateModuleInfoForPlatform( platform, true );
    if ( !moduleInfo.IsNULL() )
    {
        moduleInfo->metadata.lastEditBy = lastEditBy;
        return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfoEntry::HasTag( const CORE::CString& tag      ,
                          const CORE::CString& platform ) const
{GUCEF_TRACE;

    TModuleInfoPtrMap::const_iterator i = m_modulesPerPlatform.find( KnownPlatforms::AllPlatforms );
    if ( i != m_modulesPerPlatform.end() )
    {
        if ( (*i).second->tags.find( tag ) != (*i).second->tags.end() )
            return true;
    }
    i = m_modulesPerPlatform.find( platform );
    if ( i != m_modulesPerPlatform.end() )
    {
        if ( (*i).second->tags.find( tag ) != (*i).second->tags.end() )
            return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfoEntry::HasTag( const CORE::CString::StringSet& tags ,
                          const CORE::CString& platform        ) const
{GUCEF_TRACE;

    CORE::CString::StringSet::const_iterator i = tags.begin();
    while ( i != tags.end() )
    {
        if ( HasTag( (*i), platform ) )
            return true;
        ++i;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

TModuleType
CModuleInfoEntry::GetModuleType( const CORE::CString& targetPlatform ) const
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindModuleInfoForPlatform( targetPlatform );
    if ( !moduleInfo.IsNULL() )
    {
        if ( MODULETYPE_UNDEFINED != moduleInfo->moduleType )
        {
            return moduleInfo->moduleType;
        }
    }
    if ( targetPlatform != KnownPlatforms::AllPlatforms && !targetPlatform.IsNULLOrEmpty() )
    {
        moduleInfo = FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms );
        if ( !moduleInfo.IsNULL() )
        {
            return moduleInfo->moduleType;
        }
    }

    // Since there is no specific info for the given platform and no AllPlatform info
    // we will see if we can derive from another if there is consensus
    GUCEF::set< TModuleType > typeSet;
    TModuleInfoPtrMap::const_iterator i = m_modulesPerPlatform.begin();
    while ( i != m_modulesPerPlatform.end() )
    {
        const CModuleInfoPtr& platformModuleInfo = (*i).second;
        if ( MODULETYPE_UNDEFINED != platformModuleInfo->moduleType )
            typeSet.insert( platformModuleInfo->moduleType );
        ++i;
    }
    if ( 1 == typeSet.size() )
    {
        return *typeSet.begin();
    }

    return MODULETYPE_UNDEFINED;
}

/*---------------------------------------------------------------------------*/

Int64
CModuleInfoEntry::GetBuildOrder( const CORE::CString& platform ,
                                 bool autoConsiderAllPlatforms ) const
{GUCEF_TRACE;

    CModuleInfoPtr moduleInfo = FindModuleInfoForPlatform( platform );
    if ( !moduleInfo.IsNULL() )
    {
        if ( moduleInfo->buildOrder > -1 )
        {
            return moduleInfo->buildOrder;
        }
    }
    if ( autoConsiderAllPlatforms && ( platform != KnownPlatforms::AllPlatforms && !platform.IsNULLOrEmpty() ) )
    {
        moduleInfo = FindModuleInfoForPlatform( KnownPlatforms::AllPlatforms );
        if ( !moduleInfo.IsNULL() )
        {
            if ( moduleInfo->buildOrder > -1 )
            {
                return moduleInfo->buildOrder;
            }
        }
    }
    return -1;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
