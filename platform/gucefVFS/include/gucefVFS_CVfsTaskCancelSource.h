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


#ifndef GUCEF_VFS_CVFSTASKCANCELSOURCE_H
#define GUCEF_VFS_CVFSTASKCANCELSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_VFS_CIVFSCANCELSOURCE_H
#include "gucefVFS_CIVfsCancelSource.h"
#define GUCEF_VFS_CIVFSCANCELSOURCE_H
#endif /* GUCEF_VFS_CIVFSCANCELSOURCE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The cancellation of a task of a thread pool as a CIVfsCancelSource: it says yes once someone called RequestCancellation()
 *  on the future of the task, or the pool asked the consumer of the task to stop. It lives for the run of one task, on the
 *  stack of the consumer.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsTaskCancelSource : public CIVfsCancelSource
{
    public:

    CVfsTaskCancelSource( const CORE::CTaskPtr& task, const CORE::CTaskConsumer* consumer );

    virtual ~CVfsTaskCancelSource() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsCancelRequested( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CVfsTaskCancelSource( const CVfsTaskCancelSource& src );                    /**< not implemented */
    CVfsTaskCancelSource& operator=( const CVfsTaskCancelSource& src );         /**< not implemented */

    CORE::CTaskPtr m_task;
    const CORE::CTaskConsumer* m_consumer;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSTASKCANCELSOURCE_H ? */
