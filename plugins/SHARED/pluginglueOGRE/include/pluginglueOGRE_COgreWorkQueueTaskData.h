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

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUETASKDATA_H
#define GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUETASKDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CICLONEABLE_H
#include "gucefMT_CICloneable.h"
#define GUCEF_MT_CICLONEABLE_H
#endif /* GUCEF_MT_CICLONEABLE_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUECORE_H
#include "pluginglueOGRE_COgreWorkQueueCore.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUECORE_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUECORE_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The data of a worker task of the Ogre work queue: the queue it works for, which it keeps alive, and its number */
class GUCEF_PLUGINGLUE_OGRE_EXPORT_CPP COgreWorkQueueTaskData : public MT::CICloneable
{
    public:

    COgreWorkQueueTaskData( const COgreWorkQueueCorePtr& core, UInt32 workerIndex );

    COgreWorkQueueTaskData( const COgreWorkQueueTaskData& src );

    virtual ~COgreWorkQueueTaskData() GUCEF_VIRTUAL_OVERRIDE;

    COgreWorkQueueTaskData& operator=( const COgreWorkQueueTaskData& src );

    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Type check of task data without RTTI, see CICloneable::CloneableAs() */
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    const COgreWorkQueueCorePtr& GetCore( void ) const;

    UInt32 GetWorkerIndex( void ) const;

    private:

    COgreWorkQueueTaskData( void );     /**< not implemented */

    private:

    COgreWorkQueueCorePtr m_core;
    UInt32 m_workerIndex;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUETASKDATA_H ? */
