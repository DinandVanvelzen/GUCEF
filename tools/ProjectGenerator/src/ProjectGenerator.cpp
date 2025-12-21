/*
 *  ProjectGenerator: Tool to generate module/project files
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CONFIGSTORE_H
#include "CConfigStore.h"
#define GUCEF_CORE_CONFIGSTORE_H
#endif /* GUCEF_CORE_CONFIGSTORE_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#include "ProjectGenerator.h"

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

#ifndef GUCEF_PROJECTGEN_DATATYPES_H
#include "gucefProjectGen_DataTypes.h"
#define GUCEF_PROJECTGEN_DATATYPES_H
#endif /* GUCEF_PROJECTGEN_DATATYPES_H ? */

#ifndef GUCEF_PROJECTGEN_CIPROJECTGENERATOR_H
#include "gucefProjectGen_CIProjectGenerator.h"
#define GUCEF_PROJECTGEN_CIPROJECTGENERATOR_H
#endif /* GUCEF_PROJECTGEN_CIPROJECTGENERATOR_H ? */

#ifndef GUCEF_PROJECTGEN_CIPROJECTINFOGATHERER_H
#include "gucefProjectGen_CIProjectInfoGatherer.h"
#define GUCEF_PROJECTGEN_CIPROJECTINFOGATHERER_H
#endif /* GUCEF_PROJECTGEN_CIPROJECTINFOGATHERER_H ? */

#ifndef GUCEF_PROJECTGEN_CDIRCRAWLINGPROJECTINFOGATHERER_H
#include "gucefProjectGen_CDirCrawlingProjectInfoGatherer.h"
#define GUCEF_PROJECTGEN_CDIRCRAWLINGPROJECTINFOGATHERER_H
#endif /* GUCEF_PROJECTGEN_CDIRCRAWLINGPROJECTINFOGATHERER_H ? */

#ifndef GUCEF_PROJECTGEN_CXMLPROJECTGENERATOR_H
#include "gucefProjectGen_CXmlProjectGenerator.h"
#define GUCEF_PROJECTGEN_CXMLPROJECTGENERATOR_H
#endif /* GUCEF_PROJECTGEN_CXMLPROJECTGENERATOR_H ? */

#ifndef GUCEF_PROJECTGEN_CCMAKEPROJECTGENERATOR_H
#include "gucefProjectGen_CCMakeProjectGenerator.h"
#define GUCEF_PROJECTGEN_CCMAKEPROJECTGENERATOR_H
#endif /* GUCEF_PROJECTGEN_CCMAKEPROJECTGENERATOR_H ? */

#ifndef GUCEF_PROJECTGEN_CANDROIDMAKEFILEGENERATOR_H
#include "gucefProjectGen_CAndroidMakefileGenerator.h"
#define GUCEF_PROJECTGEN_CANDROIDMAKEFILEGENERATOR_H
#endif /* GUCEF_PROJECTGEN_CANDROIDMAKEFILEGENERATOR_H ? */

#ifndef GUCEF_PROJECTGEN_CPREMAKE4PROJECTGENERATOR_H
#include "gucefProjectGen_CPremake4ProjectGenerator.h"
#define GUCEF_PROJECTGEN_CPREMAKE4PROJECTGENERATOR_H
#endif /* GUCEF_PROJECTGEN_CPREMAKE4PROJECTGENERATOR_H ? */

#ifndef GUCEF_PROJECTGEN_CPREMAKE5PROJECTGENERATOR_H
#include "gucefProjectGen_CPremake5ProjectGenerator.h"
#define GUCEF_PROJECTGEN_CPREMAKE5PROJECTGENERATOR_H
#endif /* GUCEF_PROJECTGEN_CPREMAKE5PROJECTGENERATOR_H ? */

#ifndef GUCEF_PROJECTGEN_CARDUINOCLIGENERATOR_H
#include "gucefProjectGen_CArduinoCLIGenerator.h"
#define GUCEF_PROJECTGEN_CARDUINOCLIGENERATOR_H
#endif /* GUCEF_PROJECTGEN_CARDUINOCLIGENERATOR_H ? */

#ifndef GUCEF_PROJECTGEN_CCHIHELPERGENERATOR_H
#include "gucefProjectGen_CCIHelperGenerator.h"
#define GUCEF_PROJECTGEN_CCHIHELPERGENERATOR_H
#endif /* GUCEF_PROJECTGEN_CCHIHELPERGENERATOR_H ? */

#ifndef GUCEF_PROJECTGEN_CPROJECTGENGLOBAL_H
#include "gucefProjectGen_CProjectGenGlobal.h"
#define GUCEF_PROJECTGEN_CPROJECTGENGLOBAL_H
#endif /* GUCEF_PROJECTGEN_CPROJECTGENGLOBAL_H ? */

#ifndef GUCEF_PROJECTGEN_CPROJECTPREPROCESSORMANAGER_H
#include "gucefProjectGen_CProjectPreprocessorManager.h"
#define GUCEF_PROJECTGEN_CPROJECTPREPROCESSORMANAGER_H
#endif /* GUCEF_PROJECTGEN_CPROJECTPREPROCESSORMANAGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {

using namespace GUCEF::PROJECTGEN;

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

ProjectGenerator::ProjectGenerator( void )
    : CORE::CObservingNotifier()
    , m_workStartTrigger()
    , m_console( GUCEF_NULL )
    , m_params()
    , m_config()
    , m_returnCode( 0 )
{GUCEF_TRACE;

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

ProjectGenerator::~ProjectGenerator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

int
ProjectGenerator::GetReturnCode( void ) const
{GUCEF_TRACE;

    return m_returnCode;
}

/*-------------------------------------------------------------------------*/

void
ProjectGenerator::OnWorkStartTrigger( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    PerformWork();
}

/*-------------------------------------------------------------------------*/

void 
ProjectGenerator::RegisterEventHandlers( void )
{GUCEF_TRACE;

    // Get access to the systems this application depends on
    CORE::CGUCEFApplication& app = CORE::CCoreGlobal::Instance()->GetApplication();

    // Set our trigger criteria for starting work
    // depending on the application needs you want different systems to be ready and initialized before starting work
    m_workStartTrigger.SubscribeAndAddEventToTriggerCriterea( &app, CORE::CGUCEFApplication::FirstCycleEvent );
    
    // Now hook up the actual trigger for starting work
    TEventCallback callback( this, &ProjectGenerator::OnWorkStartTrigger );
    SubscribeTo( &m_workStartTrigger                                 ,
                 CORE::CEventAggregateTrigger::AggregateTriggerEvent ,
                 callback                                            );
}

/*-------------------------------------------------------------------------*/

void 
ProjectGenerator::SetConsoleAccess( CORE::CPlatformNativeConsoleLogger& console )
{GUCEF_TRACE;

    m_console = &console;
}

/*-------------------------------------------------------------------------*/

void 
ProjectGenerator::SetParams( const CORE::CValueList& keyValueList )
{GUCEF_TRACE;

    m_params = keyValueList;
}

/*-------------------------------------------------------------------------*/

void
ProjectGenerator::SetConfig( CORE::CDataNode& config )
{GUCEF_TRACE;

    m_config = config;
}

/*-------------------------------------------------------------------------*/

int 
ProjectGenerator::PerformWork( void )
{GUCEF_TRACE;

    CORE::CString outputDir = m_params.GetValueAlways( "outputDir" );

    CORE::CValueList::TStringVector rootDirs;
    try
    {
        rootDirs = m_params.GetValueStringVector( "rootDir" );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Number of rootDir arguments passed from command line: " + CORE::ToString( rootDirs.size() ) );
    }
    catch ( CORE::CValueList::EUnknownKey& )
    {
        rootDirs.push_back( CORE::RelativePath( "$CURWORKDIR$" ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Using current working directory since no rootDir arguments where passed from the command line" );
    }

    bool addToolCompileTimeToOutput = CORE::StringToBool( m_params.GetValueAlways( "addToolCompileTimeToOutput" ) );

    // Get the generators to use
    TStringVector generatorList = m_params.GetValueAlways( "generators" ).AsString().ParseElements( ';', false );
    if ( generatorList.size() == 0  )
    {
        // No specific generators where specified, defaulting...
        generatorList.push_back( "xml" );
    }

    CProjectInfoPtr projectInfo = CProjectInfo::CreateSharedObj();
    if ( projectInfo.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to create ProjectInfo" );
        return -1;
    }

    ApplyConfigToProject( m_config, *projectInfo.GetPointerAlways() );
    projectInfo->SetSetttings( m_params );

    bool useProjectInfoCache = m_params.GetValueAlways( "useProjectInfoCache", false ).AsBool( false, true );
    bool projectInfoLoadedFromCache = false;
    if ( useProjectInfoCache )
    {
        CORE::CString cachedInfoPath = CORE::CombinePath( CORE::RelativePath( outputDir ), "Project.xml" );
        if ( CORE::FileExists( cachedInfoPath ) )
        {
            projectInfo->SetRootDir( outputDir );
            projectInfoLoadedFromCache = projectInfo->Deserialize( cachedInfoPath );
        }
    }

    if ( !projectInfoLoadedFromCache )
    {
        // Use an info gatherer to get all the project information for us
        CDirCrawlingProjectInfoGatherer infoGatherer;
        infoGatherer.GatherInfo( rootDirs    ,
                                 projectInfo ,
                                 m_params    );
    }

    // Before we hand the data we collected and generated to the generator(s) for the desired output we will check
    // for preprocessors which can be executed before any output generator. These apply changes to the project data
    // that will apply to all output generators
    const CProjectPreprocessorManager::TProjectPreprocessorsList& projectPreProcessors = CProjectGenGlobal::Instance()->GetProjectPreprocessorManager().GetProjectPreprocessors();
    CProjectPreprocessorManager::TProjectPreprocessorsList::const_iterator n = projectPreProcessors.begin();
    while ( n != projectPreProcessors.end() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Invoking project preprocessor" );
        (*n)->ProccessProjects( *projectInfo.GetPointerAlways()  ,
                                outputDir  ,
                                m_params   );
        ++n;
    }

    // Now we output the project info using all generators specified
    TStringVector::iterator i = generatorList.begin();
    while ( i != generatorList.end() )
    {
        if ( (*i).Lowercase() == "xml" )
        {
            if ( !projectInfoLoadedFromCache )
            {
                CXmlProjectGenerator xmlGenerator;
                xmlGenerator.GenerateProject( *projectInfo.GetPointerAlways()                ,
                                              outputDir                  ,
                                              addToolCompileTimeToOutput ,
                                              m_params                   );
            }
        }
        else
        if ( (*i).Lowercase() == "androidmake" )
        {
            CAndroidMakefileGenerator androidMakefileGenerator;
            androidMakefileGenerator.GenerateProject( *projectInfo.GetPointerAlways()                ,
                                                      outputDir                  ,
                                                      addToolCompileTimeToOutput ,
                                                      m_params                   );
        }
        else
        if ( (*i).Lowercase() == "cmake" )
        {
            CCMakeProjectGenerator cmakeGenerator;
            cmakeGenerator.GenerateProject( *projectInfo.GetPointerAlways()                ,
                                            outputDir                  ,
                                            addToolCompileTimeToOutput ,
                                            m_params                   );
        }
        else
        if ( (*i).Lowercase() == "premake4" )
        {
            CPremake4ProjectGenerator premake4Generator;
            premake4Generator.GenerateProject( *projectInfo.GetPointerAlways()                ,
                                               outputDir                  ,
                                               addToolCompileTimeToOutput ,
                                               m_params                   );
        }
        else
        if ( (*i).Lowercase() == "premake5" )
        {
            CPremake5ProjectGenerator premake5Generator;
            premake5Generator.GenerateProject( *projectInfo.GetPointerAlways()                ,
                                               outputDir                  ,
                                               addToolCompileTimeToOutput ,
                                               m_params                   );
        }
        else
        if ( (*i).Lowercase() == "cihelper" )
        {
            CCIHelperGenerator ciHelper;
            ciHelper.GenerateProject( *projectInfo.GetPointerAlways()                ,
                                      outputDir                  ,
                                      addToolCompileTimeToOutput ,
                                      m_params                   );
        }
        else
        if ( (*i).Lowercase() == "arduinocli" )
        {
            CArduinoCLIGenerator cliHelper;
            cliHelper.GenerateProject( *projectInfo.GetPointerAlways()                ,
                                       outputDir                  ,
                                       addToolCompileTimeToOutput ,
                                       m_params                   );
        }        
        ++i;
    }

    CORE::CCoreGlobal::Instance()->GetApplication().Stop();
    return 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
