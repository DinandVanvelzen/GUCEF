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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#include "gucefVFS_CVfsThreadPolicyScope.h"

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

CVfsThreadPolicyScope::CVfsThreadPolicyScope( const CVfsThreadPolicy& policy )
    : m_registry( &CVfsGlobal::Instance()->GetThreadPolicyRegistry() )
    , m_threadId( MT::GetCurrentThreadID() )
    , m_hadPolicy( false )
    , m_entered( false )
    , m_previous()
{GUCEF_TRACE;

    Enter( policy );
}

/*-------------------------------------------------------------------------*/

CVfsThreadPolicyScope::CVfsThreadPolicyScope( CVfsThreadPolicyRegistry& registry, const CVfsThreadPolicy& policy )
    : m_registry( &registry )
    , m_threadId( MT::GetCurrentThreadID() )
    , m_hadPolicy( false )
    , m_entered( false )
    , m_previous()
{GUCEF_TRACE;

    Enter( policy );
}

/*-------------------------------------------------------------------------*/

void
CVfsThreadPolicyScope::Enter( const CVfsThreadPolicy& policy )
{GUCEF_TRACE;

    m_hadPolicy = m_registry->GetThreadPolicy( m_threadId, m_previous );
    m_entered = m_registry->SetThreadPolicy( m_threadId, policy );
}

/*-------------------------------------------------------------------------*/

CVfsThreadPolicyScope::~CVfsThreadPolicyScope()
{GUCEF_TRACE;

    // a scope whose registration was refused (the table was full) has nothing to put back
    if ( !m_entered )
        return;

    if ( m_hadPolicy )
        m_registry->SetThreadPolicy( m_threadId, m_previous );
    else
        m_registry->ClearThreadPolicy( m_threadId );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
