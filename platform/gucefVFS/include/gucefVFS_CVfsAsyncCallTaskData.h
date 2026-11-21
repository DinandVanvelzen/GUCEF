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


#ifndef GUCEF_VFS_CVFSASYNCCALLTASKDATA_H
#define GUCEF_VFS_CVFSASYNCCALLTASKDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_VFS_CIVFSASYNCCALL_H
#include "gucefVFS_CIVfsAsyncCall.h"
#define GUCEF_VFS_CIVFSASYNCCALL_H
#endif /* GUCEF_VFS_CIVFSASYNCCALL_H ? */

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

/** The task data of an asynchronous VFS call on a thread of the pool: a shared reference to the call */
class GUCEF_VFS_PUBLIC_CPP CVfsAsyncCallTaskData : public CORE::CICloneable
{
    public:

    CIVfsAsyncCallPtr call;

    CVfsAsyncCallTaskData( void );

    explicit CVfsAsyncCallTaskData( const CIVfsAsyncCallPtr& taskCall );

    CVfsAsyncCallTaskData( const CVfsAsyncCallTaskData& src );

    virtual ~CVfsAsyncCallTaskData() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CVfsAsyncCallTaskData& operator=( const CVfsAsyncCallTaskData& src );       /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSASYNCCALLTASKDATA_H ? */
