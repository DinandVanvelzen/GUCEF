/*
 *  ProjectGen-DependsFilter: Plugin for ProjectGen to leverage depends tool output
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

#ifndef PROJECTGENDEPENDSFILTER_CDEPENDSFILTER_H
#include "ProjectGenDependsFilter_CDependsFilter.h"
#define PROJECTGENDEPENDSFILTER_CDEPENDSFILTER_H
#endif /* PROJECTGENDEPENDSFILTER_CDEPENDSFILTER_H ? */

#ifndef GUCEF_PROJECTGEN_CPROJECTGENGLOBAL_H
#include "gucefProjectGen_CProjectGenGlobal.h"
#define GUCEF_PROJECTGEN_CPROJECTGENGLOBAL_H
#endif /* GUCEF_PROJECTGEN_CPROJECTGENGLOBAL_H ? */

#ifndef GUCEF_PROJECTGEN_CPROJECTPREPROCESSORMANAGER_H
#include "gucefProjectGen_CProjectPreprocessorManager.h"
#define GUCEF_PROJECTGEN_CPROJECTPREPROCESSORMANAGER_H
#endif /* GUCEF_PROJECTGEN_CPROJECTPREPROCESSORMANAGER_H ? */

#include "ProjectGenDependsFilter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PROJECTGEN {
namespace DEPFILTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static CDependsFilter g_dependsFilter;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    PROJECTGEN::CProjectGenGlobal::Instance()->GetProjectPreprocessorManager().RegisterProjectPreprocessor( &g_dependsFilter );
    return 1;
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    PROJECTGEN::CProjectGenGlobal::Instance()->GetProjectPreprocessorManager().UnregisterProjectPreprocessor( &g_dependsFilter );
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUCEFPlugin_GetVersion( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    versionInfo->major = 1; 
    versionInfo->minor = 0;
    versionInfo->patch = 0;
    versionInfo->release = 0;
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;
    
    return "Copyright (C) Dinand Vanvelzen, LGPL license version 3";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Project preprocessor plugin which filters out projects based on the output of the Depends tool";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DEPFILTER */
}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
