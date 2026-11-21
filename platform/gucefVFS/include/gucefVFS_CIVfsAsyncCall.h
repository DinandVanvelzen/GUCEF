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


#ifndef GUCEF_VFS_CIVFSASYNCCALL_H
#define GUCEF_VFS_CIVFSASYNCCALL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

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
 *  One asynchronous call of the VFS as a piece of work for a thread of the remote operations pool
 *  (CVfsAsyncCallTaskConsumer runs it). Run() does the work with the synchronous calls, may wait for a remote place
 *  (it is on a pool thread, never on the thread that asked) and puts what it delivers where the caller of the call
 *  looks for it, before it returns: the future of the call ends after Run() has returned.
 *
 *  A call that is cancelled before a thread takes it is never run.
 */
class GUCEF_VFS_PUBLIC_CPP CIVfsAsyncCall
{
    public:

    CIVfsAsyncCall( void );

    virtual ~CIVfsAsyncCall();

    /** Does the work; 'cancel' says when the call is asked to stop, which it looks at between its steps */
    virtual void Run( const CIVfsCancelSource& cancel ) = 0;

    private:

    CIVfsAsyncCall( const CIVfsAsyncCall& src );                    /**< not implemented */
    CIVfsAsyncCall& operator=( const CIVfsAsyncCall& src );         /**< not implemented */
};

typedef CORE::CTSharedPtr< CIVfsAsyncCall, MT::CMutex > CIVfsAsyncCallPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CIVFSASYNCCALL_H ? */
