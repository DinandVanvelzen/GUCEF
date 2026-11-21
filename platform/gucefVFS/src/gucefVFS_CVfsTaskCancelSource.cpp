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

#include "gucefVFS_CVfsTaskCancelSource.h"

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

CVfsTaskCancelSource::CVfsTaskCancelSource( const CORE::CTaskPtr& task, const CORE::CTaskConsumer* consumer )
    : CIVfsCancelSource()
    , m_task( task )
    , m_consumer( consumer )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsTaskCancelSource::~CVfsTaskCancelSource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CVfsTaskCancelSource::IsCancelRequested( void ) const
{GUCEF_TRACE;

    if ( !m_task.IsNULL() && m_task->IsCancellationRequested() )
        return true;
    return GUCEF_NULL != m_consumer && m_consumer->IsDeactivationRequested();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
