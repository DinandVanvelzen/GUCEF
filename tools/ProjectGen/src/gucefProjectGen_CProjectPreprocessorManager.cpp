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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefProjectGen_CProjectPreprocessorManager.h"

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

CProjectPreprocessorManager::CProjectPreprocessorManager( void )
    : m_projectPreprocessors()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CProjectPreprocessorManager::~CProjectPreprocessorManager()
{GUCEF_TRACE;

    m_projectPreprocessors.clear();
}

/*-------------------------------------------------------------------------*/
       
void
CProjectPreprocessorManager::RegisterProjectPreprocessor( CIProjectPreprocessor* preprocessor )
{GUCEF_TRACE;

    m_projectPreprocessors.insert( preprocessor );
}

/*-------------------------------------------------------------------------*/

void
CProjectPreprocessorManager::UnregisterProjectPreprocessor( CIProjectPreprocessor* preprocessor )
{GUCEF_TRACE;

    m_projectPreprocessors.erase( preprocessor );
}
    
/*-------------------------------------------------------------------------*/

const CProjectPreprocessorManager::TProjectPreprocessorsList&
CProjectPreprocessorManager::GetProjectPreprocessors( void ) const
{GUCEF_TRACE;

    return m_projectPreprocessors;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
