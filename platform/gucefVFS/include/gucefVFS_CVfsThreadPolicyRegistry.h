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

#ifndef GUCEF_VFS_CVFSTHREADPOLICYREGISTRY_H
#define GUCEF_VFS_CVFSTHREADPOLICYREGISTRY_H

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

#ifndef GUCEF_VFS_CVFSTHREADPOLICY_H
#include "gucefVFS_CVfsThreadPolicy.h"
#define GUCEF_VFS_CVFSTHREADPOLICY_H
#endif /* GUCEF_VFS_CVFSTHREADPOLICY_H ? */

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
 *  The table of the threads that have a rule for a miss on a remote archive (CVfsThreadPolicy), by MT::GetCurrentThreadID().
 *  A thread that is not in the table has the default rule (it waits), and finding that out costs no lock: the number of
 *  registered threads is kept in an atomic that GetCurrent() looks at first.
 *
 *  A registration lasts until its owner clears it: an operating system may give the id of a thread that ended to a new
 *  thread, which then inherits a registration that was left behind. The scope form (CVfsThreadPolicyScope) clears by
 *  construction and is the preferred one. The table holds at most MaxThreads threads; past that a new registration is
 *  refused.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsThreadPolicyRegistry
{
    public:

    static const UInt32 MaxThreads = 256;

    CVfsThreadPolicyRegistry( void );

    ~CVfsThreadPolicyRegistry();

    /** Registers the rule of a thread, replacing the one it had. False when the table is full (the thread keeps what it had) */
    bool SetThreadPolicy( TThreadId threadId, const CVfsThreadPolicy& policy );

    /** Removes the rule of a thread, which then waits again; nothing happens when it had none */
    void ClearThreadPolicy( TThreadId threadId );

    /** The rule of a thread when it has one */
    bool GetThreadPolicy( TThreadId threadId, CVfsThreadPolicy& policy ) const;

    /** The rule of the calling thread: its own, or the default (WAIT) when it has none */
    CVfsThreadPolicy GetCurrent( void ) const;

    /** The number of threads that have a rule */
    UInt32 GetRegisteredThreadCount( void ) const;

    /** How often the lock of the table was taken: a test hook, a thread without a rule must not add to it while no thread at all has one */
    UInt64 GetLockAcquisitionCount( void ) const;

    private:

    CVfsThreadPolicyRegistry( const CVfsThreadPolicyRegistry& src );                /**< not implemented */
    CVfsThreadPolicyRegistry& operator=( const CVfsThreadPolicyRegistry& src );     /**< not implemented */

    typedef GUCEF::map< TThreadId, CVfsThreadPolicy > TPolicyMap;

    mutable MT::CMutex m_lock;
    TPolicyMap m_policies;
    MT::CTAtomic< UInt32 > m_registeredCount;
    mutable MT::CTAtomic< UInt64 > m_lockAcquisitions;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSTHREADPOLICYREGISTRY_H ? */
