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

#include "gucefProjectGen_CModuleInfo.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PROJECTGEN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CModuleInfo::CModuleInfo( void ) 
    : CORE::CTSharedObjCreator< CModuleInfo, MT::CMutex >( this )
    , name()    
    , moduleType( MODULETYPE_UNDEFINED )
    , tags()
    , dependencyIncludeDirs()
    , runtimeDependencies()
    , buildOrder( -1 )
    , considerSubDirs( true )
    , hasConsiderSubDirs( false )
    , linkerSettings()
    , compilerSettings()
    , preprocessorSettings()
    , ignoreModule( false )
    , hasIgnoreModule( false )
    , metadata()
    , m_platformName()
    , m_namesOfDependencies()
    , m_namesOfLogicalDependencies()
    , m_includeDirs()
    , m_sourceDirs()
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CModuleInfo::CModuleInfo( const CModuleInfo& src ) 
    : CORE::CTSharedObjCreator< CModuleInfo, MT::CMutex >( this )
    , name( src.name )
    , moduleType( src.moduleType )
    , tags( src.tags )
    , dependencyIncludeDirs( src.dependencyIncludeDirs )
    , runtimeDependencies( src.runtimeDependencies )
    , buildOrder( src.buildOrder )
    , considerSubDirs( src.considerSubDirs )
    , hasConsiderSubDirs( src.hasConsiderSubDirs )
    , linkerSettings( src.linkerSettings )
    , compilerSettings( src.compilerSettings )
    , preprocessorSettings( src.preprocessorSettings )
    , ignoreModule( src.ignoreModule )
    , hasIgnoreModule( src.hasIgnoreModule )
    , metadata( src.metadata )
    , m_platformName( src.m_platformName )
    , m_namesOfDependencies( src.m_namesOfDependencies )
    , m_namesOfLogicalDependencies( src.m_namesOfLogicalDependencies )
    , m_includeDirs( src.m_includeDirs )
    , m_sourceDirs( src.m_sourceDirs )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

CModuleInfo::CModuleInfo( CModuleInfo&& src ) GUCEF_NOEXCEPT
    : CORE::CTSharedObjCreator< CModuleInfo, MT::CMutex >( GUCEF_MOVE( src ), this )
    , name( GUCEF_MOVE( src.name ) )
    , moduleType( src.moduleType )
    , tags( GUCEF_MOVE( src.tags ) )
    , dependencyIncludeDirs( GUCEF_MOVE( src.dependencyIncludeDirs ) )
    , runtimeDependencies( GUCEF_MOVE( src.runtimeDependencies ) )
    , buildOrder( src.buildOrder )
    , considerSubDirs( src.considerSubDirs )
    , hasConsiderSubDirs( src.hasConsiderSubDirs )
    , linkerSettings( GUCEF_MOVE( src.linkerSettings ) )
    , compilerSettings( GUCEF_MOVE( src.compilerSettings ) )
    , preprocessorSettings( GUCEF_MOVE( src.preprocessorSettings ) )
    , ignoreModule( src.ignoreModule )
    , hasIgnoreModule( src.hasIgnoreModule )
    , metadata( GUCEF_MOVE( src.metadata ) )
    , m_platformName( GUCEF_MOVE( src.m_platformName ) )
    , m_namesOfDependencies( GUCEF_MOVE( src.m_namesOfDependencies ) )
    , m_namesOfLogicalDependencies( GUCEF_MOVE( src.m_namesOfLogicalDependencies ) )
    , m_includeDirs( GUCEF_MOVE( src.m_includeDirs ) )
    , m_sourceDirs( GUCEF_MOVE( src.m_sourceDirs ) )
{GUCEF_TRACE;

}

#endif /* GUCEF_MOVE_SEMANTICS_SUPPORTED ? */

/*---------------------------------------------------------------------------*/

CModuleInfo::~CModuleInfo() 
{GUCEF_TRACE;

    Clear();
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::Clear( void ) 
{GUCEF_TRACE;

    name.Clear();
    moduleType = MODULETYPE_UNDEFINED;
    tags.clear();
    dependencyIncludeDirs.clear();
    runtimeDependencies.clear();
    buildOrder = -1;
    considerSubDirs = true;
    hasConsiderSubDirs = false;
    linkerSettings.Clear();
    compilerSettings.Clear();
    preprocessorSettings.Clear();
    ignoreModule = false;
    hasIgnoreModule = false;
    metadata.Clear();

    m_platformName.Clear();
    m_namesOfDependencies.clear();
    m_namesOfLogicalDependencies.clear();
    m_includeDirs.clear();
    m_sourceDirs.clear();
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfo::Merge( CModuleInfoPtr moduleInfoToMergeIn ,
                    bool onConflictOriginalInfoStays   )
{GUCEF_TRACE;

    bool totalSuccess = true;

    if ( m_platformName.IsNULLOrEmpty() )
        m_platformName = moduleInfoToMergeIn->m_platformName;
    else
    if ( !onConflictOriginalInfoStays && !moduleInfoToMergeIn->m_platformName.IsNULLOrEmpty() )
        m_platformName = moduleInfoToMergeIn->m_platformName;

    if ( name.IsNULLOrEmpty() )
        name = moduleInfoToMergeIn->name;
    else
    if ( !onConflictOriginalInfoStays && !moduleInfoToMergeIn->name.IsNULLOrEmpty() )
        name = moduleInfoToMergeIn->name;

    if ( MODULETYPE_UNDEFINED == moduleType )
        moduleType = moduleInfoToMergeIn->moduleType;
    else
    if ( !onConflictOriginalInfoStays && MODULETYPE_UNDEFINED != moduleInfoToMergeIn->moduleType )
        moduleType = moduleInfoToMergeIn->moduleType;

    MergeStringSetMap( m_includeDirs, moduleInfoToMergeIn->m_includeDirs, true );
    MergeStringSetMap( m_sourceDirs, moduleInfoToMergeIn->m_sourceDirs, true );

    MergeStringSet( tags, moduleInfoToMergeIn->tags, true );
    MergeStringSet( m_namesOfDependencies, moduleInfoToMergeIn->m_namesOfDependencies, true );
    MergeStringSet( m_namesOfLogicalDependencies, moduleInfoToMergeIn->m_namesOfLogicalDependencies, true );
    MergeStringSet( dependencyIncludeDirs, moduleInfoToMergeIn->dependencyIncludeDirs, true );
    MergeStringSet( runtimeDependencies, moduleInfoToMergeIn->runtimeDependencies, true );

    totalSuccess = metadata.Merge( moduleInfoToMergeIn->metadata, onConflictOriginalInfoStays ) && totalSuccess;
    totalSuccess = linkerSettings.Merge( moduleInfoToMergeIn->linkerSettings, onConflictOriginalInfoStays ) && totalSuccess;
    totalSuccess = compilerSettings.Merge( moduleInfoToMergeIn->compilerSettings, onConflictOriginalInfoStays ) && totalSuccess;
    totalSuccess = preprocessorSettings.Merge( moduleInfoToMergeIn->preprocessorSettings, onConflictOriginalInfoStays ) && totalSuccess;

    if ( -1 == buildOrder )
        buildOrder = moduleInfoToMergeIn->buildOrder;
    else
    if ( !onConflictOriginalInfoStays && -1 != moduleInfoToMergeIn->buildOrder )
        buildOrder = moduleInfoToMergeIn->buildOrder;

    if ( !hasConsiderSubDirs )
    {
        considerSubDirs = moduleInfoToMergeIn->considerSubDirs;
        hasConsiderSubDirs = moduleInfoToMergeIn->hasConsiderSubDirs;
    }
    else
    if ( !onConflictOriginalInfoStays && moduleInfoToMergeIn->hasConsiderSubDirs )
    {
        considerSubDirs = moduleInfoToMergeIn->considerSubDirs;
        hasConsiderSubDirs = moduleInfoToMergeIn->hasConsiderSubDirs;
    }

    if ( !hasIgnoreModule )
    {
        ignoreModule = moduleInfoToMergeIn->ignoreModule;
        hasIgnoreModule = moduleInfoToMergeIn->hasIgnoreModule;
    }
    else
    if ( !onConflictOriginalInfoStays && moduleInfoToMergeIn->hasIgnoreModule )
    {
        ignoreModule = moduleInfoToMergeIn->ignoreModule;
        hasIgnoreModule = moduleInfoToMergeIn->hasIgnoreModule;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfo::Serialize( CORE::CDataNode& domRootNode                        ,
                        const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    return false;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfo::Deserialize( const CORE::CDataNode& domRootNode                  ,
                          const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    const CORE::CDataNode* moduleInfoNode = domRootNode.Find( "Module" );
    if ( GUCEF_NULL == moduleInfoNode ) 
        return false;

    // Check to see if a name was defined
    CORE::CString tmpStr = moduleInfoNode->GetAttributeValue( "Name" );
    if ( !tmpStr.IsNULLOrEmpty() )
    {
        name = tmpStr;
    }

    // Find the overall module properties
    tmpStr = moduleInfoNode->GetAttributeValue( "BuildOrder", "-1" );
    buildOrder = CORE::StringToInt64( tmpStr );

    tags = StringVectorToStringSet( moduleInfoNode->GetAttributeValue( "Tags" ).AsString().ParseElements( ';', false ) );
    moduleType = StringToModuleType( moduleInfoNode->GetAttributeValue( "Type" ) );
    
    if ( moduleInfoNode->IsAttribute( "ConsiderSubDirs" ) )
    {
        considerSubDirs = moduleInfoNode->GetAttributeValue( "ConsiderSubDirs", considerSubDirs, false ).AsBool( considerSubDirs, true );
        hasConsiderSubDirs = true;
    }
    else
        hasConsiderSubDirs = false;

    if ( moduleInfoNode->IsAttribute( "IgnoreModule" ) )
    {
        ignoreModule = moduleInfoNode->GetAttributeValue( "IgnoreModule", ignoreModule, false ).AsBool( ignoreModule, true );
        hasIgnoreModule = true;
    }
    else
        hasIgnoreModule = false;

    // Find any/all files for which are part of this module
    CORE::CDataNode::TConstDataNodeSet fileNodes = moduleInfoNode->FindChildrenOfType( "Files" );
    CORE::CDataNode::TConstDataNodeSet::iterator i = fileNodes.begin();
    while ( i != fileNodes.end() )
    {
        const CORE::CDataNode* filesDirsNode = (*i);
        CORE::CString filesType = filesDirsNode->GetAttributeValue( "Type" );

        CORE::CDataNode::TConstDataNodeSet dirs = filesDirsNode->FindChildrenOfType( "Dir" );
        CORE::CDataNode::TConstDataNodeSet::iterator n = dirs.begin();
        while ( n != dirs.end() )
        {
            const CORE::CDataNode* dirsNode = (*n);
            CORE::CString path = dirsNode->GetAttributeValue( "Path" );

            CORE::CDataNode::TConstDataNodeSet files = dirsNode->FindChildrenOfType( "File" );
            CORE::CDataNode::TConstDataNodeSet::iterator m = files.begin();
            while ( m != files.end() )
            {
                CORE::CString filename = (*m)->GetAttributeValue( "Name" );

                if ( filesType == "Headers" )
                {
                    // We have a list of header files
                    AddIncludeFile( path, filename );
                }
                else
                if ( filesType == "Source" )
                {
                    // We have a list of source files
                    AddSourceFile( path, filename );
                }
                ++m;
            }
            ++n;
        }
        ++i;
    }

    // Find dependency includes
    CORE::CDataNode::TConstDataNodeSet includesNodes = moduleInfoNode->FindChildrenOfType( "Includes" );
    i = includesNodes.begin();
    while ( i != includesNodes.end() )
    {
        const CORE::CDataNode* includesNode = (*i);
        CORE::CString source = includesNode->GetAttributeValue( "Source" ).AsString().Lowercase();
        if ( source == "dependency" )
        {
            CORE::CDataNode::TConstDataNodeSet includes = includesNode->FindChildrenOfType( "Include" );
            CORE::CDataNode::TConstDataNodeSet::iterator n = includes.begin();
            while ( n != includes.end() )
            {
                const CORE::CDataNode* includeNode = (*n);
                CORE::CString path = includeNode->GetAttributeValue( "Path" );

                dependencyIncludeDirs.insert( path );
                ++n;
            }
        }
        else
        if ( source == "self" )
        {
            // If the "self" includes came from actual include files then they would be automatically
            // added when the files are added. However,... some importers might place additional includes
            // under self which would not be auto-detected via include files. As such we have to add them 
            // here as well to make sure. 
            // Importers might not have any way of knowing whether a path was added due to dependency needs

            CORE::CDataNode::TConstDataNodeSet includes = includesNode->FindChildrenOfType( "Include" );
            CORE::CDataNode::TConstDataNodeSet::iterator n = includes.begin();
            while ( n != includes.end() )
            {
                const CORE::CDataNode* includeNode = (*n);
                CORE::CString path = includeNode->GetAttributeValue( "Path" );

                if ( GetIncludeFiles( path ).empty() )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DeserializeModuleInfo: Adding include dir of source \"self\" which does not currently have a reference to include files: " + path );
                    AddIncludeDir( path );
                }
                ++n;
            }
        }
        ++i;
    }

    // Find all the module dependencies
    const CORE::CDataNode* dependenciesNode = moduleInfoNode->Find( "Dependencies" );
    if ( NULL != dependenciesNode )
    {
        CORE::CDataNode::TConstDataNodeSet dependencies = dependenciesNode->FindChildrenOfType( "Dependency" );
        i = dependencies.begin();
        while ( i != dependencies.end() )
        {
            const CORE::CDataNode* dependencyNode = (*i);
            CORE::CString dependencyName = dependencyNode->GetAttributeValue( "Name" );
            if ( !dependencyName.IsNULLOrEmpty() )
            {
                AddNameOfDependency( dependencyName );
            }
            ++i;
        }
    }

    // Get all the runtime module dependencies if any
    const CORE::CDataNode* rdependenciesNode = moduleInfoNode->Find( "RuntimeDependencies" );
    if ( NULL != rdependenciesNode )
    {
        CORE::CDataNode::TConstDataNodeSet dependencies = rdependenciesNode->FindChildrenOfType( "RuntimeDependency" );
        i = dependencies.begin();
        while ( i != dependencies.end() )
        {
            const CORE::CDataNode* dependencyNode = (*i);
            CORE::CString dependencyName = dependencyNode->GetAttributeValue( "Name" );
            if ( !dependencyName.IsNULLOrEmpty() )
            {
                runtimeDependencies.insert( dependencyName );
            }
            ++i;
        }
    }

    // Get all the runtime module dependencies if any
    const CORE::CDataNode* ldependenciesNode = moduleInfoNode->Find( "LogicalDependencies" );
    if ( NULL != ldependenciesNode )
    {
        CORE::CDataNode::TConstDataNodeSet dependencies = ldependenciesNode->FindChildrenOfType( "LogicalDependency" );
        i = dependencies.begin();
        while ( i != dependencies.end() )
        {
            const CORE::CDataNode* dependencyNode = (*i);
            CORE::CString dependencyName = dependencyNode->GetAttributeValue( "Name" );
            if ( !dependencyName.IsNULLOrEmpty() )
            {
                m_namesOfLogicalDependencies.insert( dependencyName );
            }
            ++i;
        }
    }

    CORE::CDataNodeSerializableSettings serializableSettings;

    // Go through all linker related settings, if any exist    
    linkerSettings.Deserialize( *moduleInfoNode, serializableSettings );

    // Go through all preprocessor related settings, if any exist
    preprocessorSettings.Deserialize( *moduleInfoNode, serializableSettings );

    // Go through all compiler related settings, if any exist
    compilerSettings.Deserialize( *moduleInfoNode, serializableSettings );

    // Go through all metadata, if any exist
    metadata.Deserialize( *moduleInfoNode, serializableSettings );

    return true;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::SetPlatformName( const CORE::CString& platformName )
{GUCEF_TRACE;

    m_platformName = platformName;
}

/*---------------------------------------------------------------------------*/

const CORE::CString&
CModuleInfo::GetPlatformName( void ) const
{GUCEF_TRACE;

    return m_platformName;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddIncludeDir( const CORE::CString& pathToIncludeDir )
{GUCEF_TRACE;

    m_includeDirs[ pathToIncludeDir ];
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddIncludeDirs( const CORE::CStringSet& pathsToIncludeDirs )
{GUCEF_TRACE;

    CORE::CStringSet::const_iterator i = pathsToIncludeDirs.begin();
    while ( i != pathsToIncludeDirs.end() )
    {
        const CORE::CString& pathToIncludeDir = (*i);
        m_includeDirs[ pathToIncludeDir ];
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfo::RemoveIncludeDir( const CORE::CString& pathToIncludeDir ,
                               bool mustBeEmpty                      )
{GUCEF_TRACE;

    if ( mustBeEmpty )
    {
        TStringSetMap::iterator i = m_includeDirs.find( pathToIncludeDir );
        if ( i != m_includeDirs.end() )
        {
            // it exists thus it must be empty
            if ( (*i).second.empty() )
            {
                m_includeDirs.erase( i );
                return true;
            }
            return false;
        }
        return true;
    }
    else
    {
        m_includeDirs.erase( pathToIncludeDir );
        return true;
    }
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::SetIncludeFiles( const TStringSetMap& files )
{GUCEF_TRACE;

    m_includeDirs = files;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::SetIncludeFiles( const CORE::CString& pathToIncludeFiles ,
                              const TStringSet& files                 )
{GUCEF_TRACE;

    m_includeDirs[ pathToIncludeFiles ] = files;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddIncludeFiles( const CORE::CString& pathToIncludeFiles ,
                              const TStringSet& files                 )
{GUCEF_TRACE;

    TStringSet& preExisting = m_includeDirs[ pathToIncludeFiles ];
    MergeStringSet( preExisting, files, true );
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddIncludeFiles( const TStringSetMap& files )
{GUCEF_TRACE;

    MergeStringSetMap( m_includeDirs, files, false );
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddIncludeFile( const CORE::CString& pathToFiles ,
                             const CORE::CString& filename    )
{GUCEF_TRACE;

    m_includeDirs[ pathToFiles ].insert( filename );
}

/*---------------------------------------------------------------------------*/

const TStringSetMap&
CModuleInfo::GetIncludeDirs( void ) const
{GUCEF_TRACE;

    return m_includeDirs;
}

/*---------------------------------------------------------------------------*/

const TStringSet&
CModuleInfo::GetIncludeFiles( const CORE::CString& pathToFiles ) const
{GUCEF_TRACE;

    TStringSetMap::const_iterator i = m_includeDirs.find( pathToFiles );
    if ( i != m_includeDirs.end() )
    {
        return (*i).second;
    }
    return CORE::CString::EmptyStringSet;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::SetSourceFiles( const TStringSetMap& files )
{GUCEF_TRACE;

    m_sourceDirs = files;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::SetSourceFiles( const CORE::CString& pathToFiles ,
                             const TStringSet& files          )
{GUCEF_TRACE;

    m_sourceDirs[ pathToFiles ] = files;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddSourceFiles( const CORE::CString& pathToFiles ,
                             const TStringSet& files          )
{GUCEF_TRACE;

    TStringSet& preExisting = m_sourceDirs[ pathToFiles ];
    MergeStringSet( preExisting, files, false );
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddSourceFiles( const TStringSetMap& files )
{GUCEF_TRACE;

    MergeStringSetMap( m_sourceDirs, files, false );
}

/*---------------------------------------------------------------------------*/

const TStringSetMap&
CModuleInfo::GetSourceDirs( void ) const
{GUCEF_TRACE;

    return m_sourceDirs;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddSourceFile( const CORE::CString& pathToFiles ,
                            const CORE::CString& filename    )
{GUCEF_TRACE;

    m_sourceDirs[ pathToFiles ].insert( filename );
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::SetNamesOfDependencies( const TStringSet& dependencies )
{GUCEF_TRACE;

    m_namesOfDependencies = dependencies;
}

/*---------------------------------------------------------------------------*/

const TStringSet&
CModuleInfo::GetNamesOfDependencies( void ) const
{GUCEF_TRACE;

    return m_namesOfDependencies;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddNameOfDependency( const CORE::CString& dependency )
{GUCEF_TRACE;

    m_namesOfDependencies.insert( dependency );
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::RemoveNameOfDependency( const CORE::CString& dependency )
{GUCEF_TRACE;

    m_namesOfDependencies.erase( dependency );
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfo::HasDependencyWithName( const CORE::CString& dependency ) const
{GUCEF_TRACE;

    return m_namesOfDependencies.find( dependency ) != m_namesOfDependencies.end();
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::MergeNamesOfDependencies( const TStringSet& dependenciesToMergeIn )
{GUCEF_TRACE;

    MergeStringSet( m_namesOfDependencies, dependenciesToMergeIn, false );
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfo::MoveDependencyToLogicalDependencies( const CORE::CString& dependency )
{GUCEF_TRACE;

    TStringSet::iterator i = m_namesOfDependencies.find( dependency );
    if ( i != m_namesOfDependencies.end() )
    {
        m_namesOfDependencies.erase( dependency );
        m_namesOfLogicalDependencies.insert( dependency );
        return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/

const TStringSet&
CModuleInfo::GetNamesOfLogicalDependencies( void ) const
{GUCEF_TRACE;

    return m_namesOfLogicalDependencies;
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfo::HasLogicalDependencyWithName( const CORE::CString& dependency ) const
{GUCEF_TRACE;

    return m_namesOfLogicalDependencies.find( dependency ) != m_namesOfLogicalDependencies.end();
}
/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddNameOfRuntimeDependency( const CORE::CString& dependency )
{GUCEF_TRACE;

    runtimeDependencies.insert( dependency );
}

/*---------------------------------------------------------------------------*/

const TStringSet&
CModuleInfo::GetNamesOfRuntimeDependencies( void ) const
{GUCEF_TRACE;

    return runtimeDependencies;
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::RemoveNameOfRuntimeDependency( const CORE::CString& dependency )
{GUCEF_TRACE;

    runtimeDependencies.erase( dependency );
}

/*---------------------------------------------------------------------------*/

void
CModuleInfo::AddDependencyIncludeDirs( const CORE::CStringSet& pathsToIncludeDirs )
{GUCEF_TRACE;

    dependencyIncludeDirs.insert( pathsToIncludeDirs.begin(), pathsToIncludeDirs.end() );
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfo::HasIndependentModuleType( void ) const
{GUCEF_TRACE;

    switch ( moduleType )
    {
        case MODULETYPE_HEADER_INCLUDE_LOCATION:
        case MODULETYPE_HEADER_INTEGRATE_LOCATION:
        case MODULETYPE_CODE_INTEGRATE_LOCATION:
        case MODULETYPE_UNDEFINED:
        case MODULETYPE_UNKNOWN:
        {
            return false;
        }
        default:
        {
            return true;
        }
    }
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfo::HasValidModuleType( void ) const
{GUCEF_TRACE;

    switch ( moduleType )
    {
        case MODULETYPE_UNDEFINED:
        case MODULETYPE_UNKNOWN:
        {
            return false;
        }
        default:
        {
            return true;
        }
    }
}

/*---------------------------------------------------------------------------*/

bool
CModuleInfo::IsLogicalModuleType( void ) const
{GUCEF_TRACE;

    switch ( moduleType )
    {
        case MODULETYPE_HEADER_INCLUDE_LOCATION:
        case MODULETYPE_HEADER_INTEGRATE_LOCATION:
        case MODULETYPE_CODE_INTEGRATE_LOCATION:
        {
            return true;
        }
        default:
        {
            return false;
        }
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
