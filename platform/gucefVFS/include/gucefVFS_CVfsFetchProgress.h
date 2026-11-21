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

#ifndef GUCEF_VFS_CVFSFETCHPROGRESS_H
#define GUCEF_VFS_CVFSFETCHPROGRESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_VFS_CVFSCALLRESULT_H
#include "gucefVFS_CVfsCallResult.h"
#define GUCEF_VFS_CVFSCALLRESULT_H
#endif /* GUCEF_VFS_CVFSCALLRESULT_H ? */

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
 *  How far a fetch is and the request to stop it. The caller and the fetch each hold a reference to the same object (a
 *  shared pointer), so neither has to outlive the other. The counters can be read from any thread while the fetch runs.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsFetchProgress
{
    public:

    MT::CTAtomic< UInt64 > bytesTotal;              /**< the bytes of the files of the fetch; known per file once its metadata is */
    MT::CTAtomic< UInt64 > bytesDone;               /**< the bytes added to what the cache held */
    MT::CTAtomic< UInt32 > filesTotal;              /**< the files the fetch took on, a file counted once whatever came of it (a directory listing counts as one as well) */
    MT::CTAtomic< UInt32 > filesDone;               /**< those that were fetched */
    MT::CTAtomic< UInt32 > filesFailed;             /**< those that were not */
    MT::CTAtomic< bool > cancel;                    /**< set by the caller to stop the fetch between block runs */

    CVfsFetchProgress( void );

    /** The result of the latest failure of the fetch, a success when there was none */
    CVfsCallResult GetLastFailure( void ) const;

    void SetLastFailure( const CVfsCallResult& failure );

    /**
     *  Whether the fetch is to stop: the cancel flag is set, or the source of cancellation that the call running the fetch
     *  gave (the cancellation request of the future of an asynchronous call) says so. The fetch asks this between its steps.
     */
    bool IsCancelRequested( void ) const;

    /**
     *  Lets the call that runs the fetch make its own cancellation request count as the cancel flag. The call sets the source
     *  before the fetch starts and sets none (a NULL) when it is done, before the source goes; the progress never keeps it
     *  longer. One call at a time can use a progress this way.
     */
    void SetCancelSource( const CIVfsCancelSource* source );

    private:

    CVfsFetchProgress( const CVfsFetchProgress& src );                  /**< not implemented */
    CVfsFetchProgress& operator=( const CVfsFetchProgress& src );       /**< not implemented */

    mutable MT::CMutex m_lock;
    CVfsCallResult m_lastFailure;
    const CIVfsCancelSource* m_cancelSource;
};

typedef CORE::CTSharedPtr< CVfsFetchProgress, MT::CMutex > CVfsFetchProgressPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSFETCHPROGRESS_H ? */
