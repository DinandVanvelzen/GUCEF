/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#include "gucefVFS_CVfsAsyncCallGateScope.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsAsyncCallGateScope::CVfsAsyncCallGateScope( const CVfsAsyncCallGatePtr& gate )
    : m_gate( gate )
    , m_entered( false )
{GUCEF_TRACE;

    if ( !m_gate.IsNULL() )
        m_entered = m_gate->TryEnter();
}

/*-------------------------------------------------------------------------*/

CVfsAsyncCallGateScope::~CVfsAsyncCallGateScope()
{GUCEF_TRACE;

    if ( m_entered )
        m_gate->Leave();
}

/*-------------------------------------------------------------------------*/

bool
CVfsAsyncCallGateScope::IsEntered( void ) const
{GUCEF_TRACE;

    return m_entered;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
