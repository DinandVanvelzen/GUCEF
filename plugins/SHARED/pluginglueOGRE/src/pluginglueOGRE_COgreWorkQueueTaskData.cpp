/*
 *  pluginglueOGRE: Support library of the Ogre 1.x plugin set: Ogre Root setup, log, plugin loading
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

#include "pluginglueOGRE_COgreWorkQueueTaskData.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreWorkQueueTaskData::COgreWorkQueueTaskData( const COgreWorkQueueCorePtr& core, UInt32 workerIndex )
    : MT::CICloneable()
    , m_core( core )
    , m_workerIndex( workerIndex )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COgreWorkQueueTaskData::COgreWorkQueueTaskData( const COgreWorkQueueTaskData& src )
    : MT::CICloneable( src )
    , m_core( src.m_core )
    , m_workerIndex( src.m_workerIndex )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COgreWorkQueueTaskData::~COgreWorkQueueTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COgreWorkQueueTaskData&
COgreWorkQueueTaskData::operator=( const COgreWorkQueueTaskData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        MT::CICloneable::operator=( src );
        m_core = src.m_core;
        m_workerIndex = src.m_workerIndex;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

MT::CICloneable*
COgreWorkQueueTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW COgreWorkQueueTaskData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
COgreWorkQueueTaskData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
COgreWorkQueueTaskData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::PLUGINGLUE::OGRE::COgreWorkQueueTaskData";
}

/*-------------------------------------------------------------------------*/

const COgreWorkQueueCorePtr&
COgreWorkQueueTaskData::GetCore( void ) const
{GUCEF_TRACE;

    return m_core;
}

/*-------------------------------------------------------------------------*/

UInt32
COgreWorkQueueTaskData::GetWorkerIndex( void ) const
{GUCEF_TRACE;

    return m_workerIndex;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
