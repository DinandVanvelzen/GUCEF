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


#include "gucefProjectGen_CProjectTargetInfo.h"

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

const CORE::CString CProjectTargetInfo::ClassTypeName = "GUCEF::PROJECTGEN::CProjectTargetInfo";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CProjectTargetInfo::CProjectTargetInfo( void )
    : CORE::CIDataNodeSerializable()
    , CORE::CTSharedObjCreator< CProjectTargetInfo, MT::CMutex >( this )
    , projectName()
    , mainModule()
    , mainModuleName()
    , modules()
    , moduleNames()
    , m_platformName()
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CProjectTargetInfo::CProjectTargetInfo( const CProjectTargetInfo& src )
    : CORE::CIDataNodeSerializable( src )
    , CORE::CTSharedObjCreator< CProjectTargetInfo, MT::CMutex >( this )
    , projectName( src.projectName )
    , mainModule( src.mainModule )
    , mainModuleName( src.mainModuleName )
    , modules( src.modules )
    , moduleNames( src.moduleNames )
    , m_platformName( src.m_platformName )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

CProjectTargetInfo::CProjectTargetInfo( CProjectTargetInfo&& src ) GUCEF_NOEXCEPT
    : CORE::CIDataNodeSerializable( GUCEF_MOVE( src ) )
    , CORE::CTSharedObjCreator< CProjectTargetInfo, MT::CMutex >( GUCEF_MOVE( src ), this )
    , projectName( GUCEF_MOVE( src.projectName ) )
    , mainModule( GUCEF_MOVE( src.mainModule ) )
    , mainModuleName( GUCEF_MOVE( src.mainModuleName ) )
    , modules( GUCEF_MOVE( src.modules ) )
    , moduleNames( GUCEF_MOVE( src.moduleNames ) )
    , m_platformName( GUCEF_MOVE( src.m_platformName ) )
{GUCEF_TRACE;

}

#endif

/*---------------------------------------------------------------------------*/

CProjectTargetInfo::~CProjectTargetInfo()
{GUCEF_TRACE;

    Clear();
}

/*---------------------------------------------------------------------------*/

void
CProjectTargetInfo::Clear( void )
{GUCEF_TRACE;

    projectName.Clear();
    mainModule.Unlink();
    mainModuleName.Clear();
    modules.clear();
    moduleNames.clear();
    m_platformName.Clear();
}

/*---------------------------------------------------------------------------*/

void
CProjectTargetInfo::SetPlatformName( const CORE::CString& platformName )
{GUCEF_TRACE;

    m_platformName = platformName;
}

/*---------------------------------------------------------------------------*/

const CORE::CString&
CProjectTargetInfo::GetPlatformName( void ) const
{GUCEF_TRACE;

    return m_platformName;
}

/*---------------------------------------------------------------------------*/

void
CProjectTargetInfo::SyncObjectsToNames( void )
{GUCEF_TRACE;

    if ( !mainModule.IsNULL() )
        mainModuleName = mainModule->GetConsensusName();

    moduleNames.clear();
    TModuleInfoEntryPtrSet::iterator i = modules.begin();
    while ( i != modules.end() )
    {
        const CModuleInfoEntryPtr& module = (*i);
        if ( !module.IsNULL() )
        {
            moduleNames.insert( module->GetConsensusName() );
        }

        ++i;
    }
}

/*---------------------------------------------------------------------------*/

bool
CProjectTargetInfo::SyncNamesToObjects( const CProjectInfo& projectInfo )
{GUCEF_TRACE;

    bool totalSuccess = true;

    mainModule.Unlink();
    if ( !mainModuleName.IsNULLOrEmpty() )
    {
        mainModule = projectInfo.GetModuleInfoEntry( mainModuleName );
        totalSuccess = !mainModule.IsNULL();
    }

    modules.clear();
    CORE::CString::StringSet::const_iterator i = moduleNames.begin();
    while ( i != moduleNames.end() )
    {
        const CORE::CString& moduleName = (*i);
        CModuleInfoEntryPtr module = projectInfo.GetModuleInfoEntry( moduleName );
        if ( !module.IsNULL() )
        {
            modules.insert( module );
        }
        else
        {
            totalSuccess = false;
        }
        ++i;
    }

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/
               
bool 
CProjectTargetInfo::Serialize( CORE::CDataNode& domRootNode                        ,
                               const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    totalSuccess = domRootNode.SetAttribute( "ProjectName", projectName ) && totalSuccess;
    if ( !mainModuleName.IsNULLOrEmpty() )
        totalSuccess = domRootNode.SetAttribute( "MainModuleName", mainModuleName ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "Platform", m_platformName ) && totalSuccess;
    totalSuccess = domRootNode.SetValuesOfChildByName( "ModuleNames", moduleNames, false, "ModuleName" ) && totalSuccess; 
    
    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

bool
CProjectTargetInfo::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                 const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    bool totalSuccess = true;

    projectName = domRootNode.GetAttributeValueOrChildValueByName( "ProjectName", projectName ).AsString( projectName, true );   
    mainModuleName = domRootNode.GetAttributeValueOrChildValueByName( "MainModuleName", mainModuleName ).AsString( mainModuleName, true );
    m_platformName = domRootNode.GetAttributeValueOrChildValueByName( "Platform", m_platformName ).AsString( m_platformName, true );
    totalSuccess = domRootNode.GetValuesOfChildByName( "ModuleNames", moduleNames ) && totalSuccess;

    return totalSuccess;
}

/*---------------------------------------------------------------------------*/

CORE::CICloneable* 
CProjectTargetInfo::Clone( void ) const 
{GUCEF_TRACE;

    return new CProjectTargetInfo( *this );
}

/*---------------------------------------------------------------------------*/

const CORE::CString& 
CProjectTargetInfo::GetClassTypeName( void ) const 
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
