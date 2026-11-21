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

#ifndef GUCEF_VFS_CVFSFETCHTASKDATA_H
#define GUCEF_VFS_CVFSFETCHTASKDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_VFS_CIVFSFETCHJOB_H
#include "gucefVFS_CIVfsFetchJob.h"
#define GUCEF_VFS_CIVFSFETCHJOB_H
#endif /* GUCEF_VFS_CIVFSFETCHJOB_H ? */

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

class CVfsFetchScheduler;

/** The task data of a fetch job on a thread of the pool: the scheduler that runs it and a shared reference to the job */
class GUCEF_VFS_PUBLIC_CPP CVfsFetchTaskData : public CORE::CICloneable
{
    public:

    CVfsFetchScheduler* scheduler;
    CIVfsFetchJobPtr job;
    CORE::CString key;

    CVfsFetchTaskData( void );

    CVfsFetchTaskData( CVfsFetchScheduler* taskScheduler, const CIVfsFetchJobPtr& taskJob, const CORE::CString& taskKey );

    CVfsFetchTaskData( const CVfsFetchTaskData& src );

    virtual ~CVfsFetchTaskData() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CVfsFetchTaskData& operator=( const CVfsFetchTaskData& src );       /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSFETCHTASKDATA_H ? */
