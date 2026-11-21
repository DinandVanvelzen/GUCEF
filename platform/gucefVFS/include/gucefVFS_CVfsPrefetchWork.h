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


#ifndef GUCEF_VFS_CVFSPREFETCHWORK_H
#define GUCEF_VFS_CVFSPREFETCHWORK_H

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

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_VFS_CVFSCALLRESULT_H
#include "gucefVFS_CVfsCallResult.h"
#define GUCEF_VFS_CVFSCALLRESULT_H
#endif /* GUCEF_VFS_CVFSCALLRESULT_H ? */

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
 *  The files that a prefetch still has to fetch, shared by the call that runs the prefetch and the helpers it gave to other threads
 *  of the pool: every worker takes the next file from it until none is left, so the files are shared out by who is free, and a
 *  helper that never gets a thread costs nothing (the call works through the list itself). It also gathers how each file ended.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsPrefetchWork
{
    public:

    explicit CVfsPrefetchWork( const CORE::CString::StringVector& files );

    /** The next file that nobody has taken; false when there is none */
    bool TakeNext( CORE::CString& file );

    /** Records how a file ended */
    void NoteEnded( const CVfsCallResult& result );

    /** A helper counts itself in before it takes its first file and out when it is done: the call waits for the helpers that are in */
    void HelperStarted( void );
    void HelperEnded( void );
    UInt32 GetHelpersRunning( void ) const;

    UInt32 GetFilesTaken( void ) const;
    UInt32 GetFilesSucceeded( void ) const;
    UInt32 GetFilesFailed( void ) const;

    /** The first failure that was not a cancellation, a success when there was none */
    CVfsCallResult GetFirstFailure( void ) const;

    /** Whether a file ended cancelled */
    bool HasCancelled( void ) const;

    private:

    CVfsPrefetchWork( const CVfsPrefetchWork& src );                    /**< not implemented */
    CVfsPrefetchWork& operator=( const CVfsPrefetchWork& src );         /**< not implemented */

    mutable MT::CMutex m_lock;
    CORE::CString::StringVector m_files;
    size_t m_next;
    UInt32 m_helpersRunning;
    UInt32 m_succeeded;
    UInt32 m_failed;
    bool m_cancelled;
    CVfsCallResult m_firstFailure;
};

typedef CORE::CTSharedPtr< CVfsPrefetchWork, MT::CMutex > CVfsPrefetchWorkPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSPREFETCHWORK_H ? */
