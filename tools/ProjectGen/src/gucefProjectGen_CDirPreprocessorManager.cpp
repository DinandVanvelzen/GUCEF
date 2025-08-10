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

#include "gucefProjectGen_CDirPreprocessorManager.h"

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

CDirPreprocessorManager::CDirPreprocessorManager( void )
    : m_dirPreprocessors()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDirPreprocessorManager::~CDirPreprocessorManager()
{GUCEF_TRACE;

    m_dirPreprocessors.clear();
}

/*-------------------------------------------------------------------------*/
       
void
CDirPreprocessorManager::RegisterDirPreprocessor( CIDirPreprocessor* preprocessor )
{GUCEF_TRACE;

    m_dirPreprocessors.insert( preprocessor );
}

/*-------------------------------------------------------------------------*/

void
CDirPreprocessorManager::UnregisterDirPreprocessor( CIDirPreprocessor* preprocessor )
{GUCEF_TRACE;

    m_dirPreprocessors.erase( preprocessor );
}
    
/*-------------------------------------------------------------------------*/

const CDirPreprocessorManager::TDirPreprocessorsList&
CDirPreprocessorManager::GetDirPreprocessors( void ) const
{GUCEF_TRACE;

    return m_dirPreprocessors;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
