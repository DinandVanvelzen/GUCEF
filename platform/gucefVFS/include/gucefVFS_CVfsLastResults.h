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

#ifndef GUCEF_VFS_CVFSLASTRESULTS_H
#define GUCEF_VFS_CVFSLASTRESULTS_H

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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

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
 *  The result of the latest call per thread. The bool, size and empty string answers of the archive interface say nothing
 *  about why a call failed, so whoever makes the calls records the outcome here for the thread that made it (by
 *  MT::GetCurrentThreadID()) and the caller asks for it right after the call. A success removes the entry of the thread,
 *  so Get() gives a success when the latest call of the calling thread went well or none was made.
 *
 *  Clear() costs no lock while no thread has an entry, so a caller that clears at the start of every call pays nothing
 *  as long as no failure was ever recorded. The number of threads that have an entry is bounded: past the bound the
 *  table starts over.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsLastResults
{
    public:

    static const UInt32 MaxThreads = 256;

    CVfsLastResults( void );

    ~CVfsLastResults();

    /** Records the result of the call the calling thread just made; a success removes the entry */
    void Set( const CVfsCallResult& result ) const;

    /** The result of the latest call of the calling thread */
    CVfsCallResult Get( void ) const;

    /** Removes the entry of the calling thread (the thread starts a call that has no result yet) */
    void Clear( void ) const;

    /** The number of threads that have an entry */
    UInt32 GetEntryCount( void ) const;

    private:

    CVfsLastResults( const CVfsLastResults& src );                  /**< not implemented */
    CVfsLastResults& operator=( const CVfsLastResults& src );       /**< not implemented */

    typedef GUCEF::map< TThreadId, CVfsCallResult > TResultMap;

    mutable MT::CMutex m_lock;
    mutable TResultMap m_results;
    mutable MT::CTAtomic< UInt32 > m_entryCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSLASTRESULTS_H ? */
