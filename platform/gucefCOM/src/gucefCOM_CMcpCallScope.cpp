/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#include "gucefCOM_CMcpCallScope.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpCallScope::CMcpCallScope( CMcpCallGate& gate, CMcpRequestContext& context )
    : m_gate( gate )
    , m_entered( false )
    , m_ticket( 0 )
{GUCEF_TRACE;

    m_entered = m_gate.Enter( &context, m_ticket );
}

/*-------------------------------------------------------------------------*/

CMcpCallScope::CMcpCallScope( CMcpCallGate& gate )
    : m_gate( gate )
    , m_entered( false )
    , m_ticket( 0 )
{GUCEF_TRACE;

    m_entered = m_gate.Enter( GUCEF_NULL, m_ticket );
}

/*-------------------------------------------------------------------------*/

CMcpCallScope::~CMcpCallScope()
{GUCEF_TRACE;

    Leave();
}

/*-------------------------------------------------------------------------*/

bool
CMcpCallScope::IsEntered( void ) const
{GUCEF_TRACE;

    return m_entered;
}

/*-------------------------------------------------------------------------*/

void
CMcpCallScope::Leave( void )
{GUCEF_TRACE;

    if ( m_entered )
    {
        m_entered = false;
        m_gate.Leave( m_ticket );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
