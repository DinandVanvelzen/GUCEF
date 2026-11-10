/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_CFILELOCK_H
#define GUCEF_AUTH_CFILELOCK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_AUTH_MACROS_H
#include "gucefAUTH_macros.h"
#define GUCEF_AUTH_MACROS_H
#endif /* GUCEF_AUTH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An advisory lock on a lock file, by which the instances of a file based store, in one process or in several,
 *  take turns: shared for a read, exclusive for a change. The lock is let go by Release() or when the object goes
 *  away; the kernel lets it go when a process ends.
 *
 *  Linux, Android and the Apple platforms: flock(), which belongs to the open file description, so two objects in
 *  one process exclude each other as well (fcntl() record locks belong to the process and would not). Windows:
 *  LockFileEx() on the first byte, per handle. Other platforms (WebAssembly) have one process and no file locks:
 *  Acquire() answers LOCKOUTCOME_UNAVAILABLE and IsSupported() is false.
 *
 *  The lock file is made when it is not there (owner only, opened close-on-exec, not through a symbolic link) and is
 *  never deleted: deleting a lock file that another instance holds open breaks the exclusion.
 */
class GUCEF_AUTH_EXPORT_CPP CFileLock
{
    public:

    enum ELockOutcome
    {
        LOCKOUTCOME_TAKEN = 0,
        LOCKOUTCOME_TIMED_OUT,          /**< another holder kept the lock for longer than the time given */
        LOCKOUTCOME_UNAVAILABLE         /**< the lock file cannot be opened or made, or the platform has no file locks */
    };
    typedef enum ELockOutcome TLockOutcome;

    /** The longest pause between two attempts while waiting for the lock */
    static const UInt32 MaxPollDelayInMs;

    CFileLock( void );

    /** Lets go of the lock */
    ~CFileLock();

    /**
     *  Takes the lock on the lock file at the path, waiting up to the time given (0: without a limit). A lock that
     *  the object holds is let go first
     */
    TLockOutcome Acquire( const CORE::CString& lockFilePath, bool exclusive, UInt32 timeoutInMs );

    /** Lets go of the lock and closes the lock file; nothing happens when no lock is held */
    void Release( void );

    /** Whether the object holds a lock */
    bool IsHeld( void ) const;

    /** Whether the platform has file locks: false means one process with the mutex of the instance as the only exclusion */
    static bool IsSupported( void );

    private:

    CFileLock( const CFileLock& src );
    CFileLock& operator=( const CFileLock& src );

    private:

    void* m_handle;     /**< Windows: the handle of the lock file, GUCEF_NULL for none */
    int m_fd;           /**< POSIX: the descriptor of the lock file, -1 for none */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CFILELOCK_H ? */
