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

#include "gucefProjectGen_CIProjectGenerator.h"

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

CProjectGeneratorCapabilities::CProjectGeneratorCapabilities( void )
    : m_supportedPlatforms()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CProjectGeneratorCapabilities::CProjectGeneratorCapabilities( const CProjectGeneratorCapabilities& src )
    : m_supportedPlatforms( src.m_supportedPlatforms )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CProjectGeneratorCapabilities::~CProjectGeneratorCapabilities()
{GUCEF_TRACE;

    m_supportedPlatforms.clear();
}

/*-------------------------------------------------------------------------*/

CProjectGeneratorCapabilities&
CProjectGeneratorCapabilities::operator=( const CProjectGeneratorCapabilities& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_supportedPlatforms = src.m_supportedPlatforms;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CProjectGeneratorCapabilities::IsPlatformSpecific( void ) const
{GUCEF_TRACE;

    if ( m_supportedPlatforms.size() == 1 )
    {
        const CORE::CString supportedPlatform = (*m_supportedPlatforms.begin());
        if ( !supportedPlatform.IsNULLOrEmpty() && supportedPlatform.Lowercase() != KnownPlatforms::AllPlatforms )
        {
            return true;
        }
        return false;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CProjectGeneratorCapabilities::IsPlatformAgnostic( void ) const
{GUCEF_TRACE;

    if ( m_supportedPlatforms.size() == 1 )
    {
        const CORE::CString supportedPlatform = (*m_supportedPlatforms.begin());
        if ( supportedPlatform.IsNULLOrEmpty() || supportedPlatform.Lowercase() == KnownPlatforms::AllPlatforms )
        {
            return true;
        }
        return false;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CProjectGeneratorCapabilities::AddSupportedPlatform( const CORE::CString& platform )
{GUCEF_TRACE;

    m_supportedPlatforms.insert( platform );
}

/*-------------------------------------------------------------------------*/

void
CProjectGeneratorCapabilities::SetSupportedPlatforms( const CORE::CStringSet& platforms )
{GUCEF_TRACE;

    m_supportedPlatforms = platforms;
}

/*-------------------------------------------------------------------------*/

const CORE::CStringSet&
CProjectGeneratorCapabilities::GetSupportedPlatforms( void ) const
{GUCEF_TRACE;

    return m_supportedPlatforms;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

CIProjectGenerator::CIProjectGenerator( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CIProjectGenerator::CIProjectGenerator( const CIProjectGenerator& src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CIProjectGenerator::~CIProjectGenerator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIProjectGenerator&
CIProjectGenerator::operator=( const CIProjectGenerator& src )
{GUCEF_TRACE;
    
    return *this;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
