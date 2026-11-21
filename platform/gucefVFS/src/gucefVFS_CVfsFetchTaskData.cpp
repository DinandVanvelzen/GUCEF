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

#include "gucefVFS_CVfsFetchTaskData.h"

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

CVfsFetchTaskData::CVfsFetchTaskData( void )
    : CORE::CICloneable()
    , scheduler( GUCEF_NULL )
    , job()
    , key()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsFetchTaskData::CVfsFetchTaskData( CVfsFetchScheduler* taskScheduler, const CIVfsFetchJobPtr& taskJob, const CORE::CString& taskKey )
    : CORE::CICloneable()
    , scheduler( taskScheduler )
    , job( taskJob )
    , key( taskKey )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsFetchTaskData::CVfsFetchTaskData( const CVfsFetchTaskData& src )
    : CORE::CICloneable( src )
    , scheduler( src.scheduler )
    , job( src.job )
    , key( src.key )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsFetchTaskData::~CVfsFetchTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CVfsFetchTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CVfsFetchTaskData( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
