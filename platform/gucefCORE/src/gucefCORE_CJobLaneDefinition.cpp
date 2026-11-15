/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#include "gucefCORE_CJobLaneDefinition.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CJobLaneDefinition::DisabledModeName = "disabled";

/*-------------------------------------------------------------------------*/

CJobLaneDefinition::CJobLaneDefinition( void )
    : m_name()
    , m_maxConcurrent( 1 )
    , m_throttlePolicy( GUCEF_NULL )
    , m_declaredModes()
    , m_initialMode()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CJobLaneDefinition::SetName( const CString& name )
{GUCEF_TRACE;

    m_name = name;
}

/*-------------------------------------------------------------------------*/

const CString&
CJobLaneDefinition::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

void
CJobLaneDefinition::SetMaxConcurrent( UInt32 maxConcurrent )
{GUCEF_TRACE;

    m_maxConcurrent = maxConcurrent;
}

/*-------------------------------------------------------------------------*/

UInt32
CJobLaneDefinition::GetMaxConcurrent( void ) const
{GUCEF_TRACE;

    return m_maxConcurrent;
}

/*-------------------------------------------------------------------------*/

void
CJobLaneDefinition::SetThrottlePolicy( CIJobThrottlePolicy* throttlePolicy )
{GUCEF_TRACE;

    m_throttlePolicy = throttlePolicy;
}

/*-------------------------------------------------------------------------*/

CIJobThrottlePolicy*
CJobLaneDefinition::GetThrottlePolicy( void ) const
{GUCEF_TRACE;

    return m_throttlePolicy;
}

/*-------------------------------------------------------------------------*/

void
CJobLaneDefinition::SetDeclaredModes( const CString::StringVector& declaredModes )
{GUCEF_TRACE;

    m_declaredModes = declaredModes;
}

/*-------------------------------------------------------------------------*/

const CString::StringVector&
CJobLaneDefinition::GetDeclaredModes( void ) const
{GUCEF_TRACE;

    return m_declaredModes;
}

/*-------------------------------------------------------------------------*/

void
CJobLaneDefinition::SetInitialMode( const CString& initialMode )
{GUCEF_TRACE;

    m_initialMode = initialMode;
}

/*-------------------------------------------------------------------------*/

const CString&
CJobLaneDefinition::GetInitialMode( void ) const
{GUCEF_TRACE;

    return m_initialMode;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
