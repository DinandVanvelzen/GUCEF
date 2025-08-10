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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PROJECTGEN_CDIRPREPROCESSORMANAGER_H
#include "gucefProjectGen_CDirPreprocessorManager.h"
#define GUCEF_PROJECTGEN_CDIRPREPROCESSORMANAGER_H
#endif /* GUCEF_PROJECTGEN_CDIRPREPROCESSORMANAGER_H ? */

#ifndef GUCEF_PROJECTGEN_CPROJECTPREPROCESSORMANAGER_H
#include "gucefProjectGen_CProjectPreprocessorManager.h"
#define GUCEF_PROJECTGEN_CPROJECTPREPROCESSORMANAGER_H
#endif /* GUCEF_PROJECTGEN_CPROJECTPREPROCESSORMANAGER_H ? */

#include "gucefProjectGen_CProjectGenGlobal.h"

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

CProjectGenGlobal* CProjectGenGlobal::g_instance = NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CProjectGenGlobal*
CProjectGenGlobal::Instance()
{GUCEF_TRACE;

    if ( NULL == g_instance )
    {
        g_instance = new CProjectGenGlobal();
        g_instance->Initialize();
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CProjectGenGlobal::Deinstance( void )
{GUCEF_TRACE;

    delete g_instance;
    g_instance = NULL;
}

/*-------------------------------------------------------------------------*/

void
CProjectGenGlobal::Initialize( void )
{GUCEF_TRACE;

    CORE::CCoreGlobal::Instance();
    
    /*
     *  Instantiate all the singletons
     */
    m_dirPreprocessorManager = new CDirPreprocessorManager();
    m_projectPreprocessorManager = new CProjectPreprocessorManager();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefProjectGen Global systems initialized" );
}

/*-------------------------------------------------------------------------*/

CProjectGenGlobal::CProjectGenGlobal( void )
    : m_dirPreprocessorManager( NULL )     ,
      m_projectPreprocessorManager( NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CProjectGenGlobal::~CProjectGenGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Shutting down gucefProjectGen global systems" );

    /*
     *      cleanup all singletons
     */
    delete m_dirPreprocessorManager;
    m_dirPreprocessorManager = NULL;
    delete m_projectPreprocessorManager;
    m_projectPreprocessorManager = NULL;
}

/*-------------------------------------------------------------------------*/

CDirPreprocessorManager&
CProjectGenGlobal::GetDirPreprocessorManager( void )
{GUCEF_TRACE;

    return *m_dirPreprocessorManager;
}

/*-------------------------------------------------------------------------*/

CProjectPreprocessorManager&
CProjectGenGlobal::GetProjectPreprocessorManager( void )
{GUCEF_TRACE;

    return *m_projectPreprocessorManager;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
