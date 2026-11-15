/*
 *  AssetConverterLib: the lock of an intermediate folder, so that two runs never write it at the same time
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

#ifndef ASSETCONVERTERLIB_CINTERMEDIATELOCK_H
#define ASSETCONVERTERLIB_CINTERMEDIATELOCK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The lock of the intermediate folder of a pack (RAII): the constructor takes it, the destructor gives it back. The
 *  pipeline takes it for every run that writes (CAssetPipeline::RunPack(), all modes but validate-only), so a command
 *  line run, a run of a tool or a service and a run of the viewer on the same pack exclude each other, in one process
 *  and across processes.
 *
 *  - The lock is the file <intermediate>/.lock, made with exclusive creation on the file system of the folder
 *    (CORE::CreateOwnerOnlyFile(): only the user that runs the process can read it), holding the process id, the host
 *    name, the UTC time it was taken and a random token, as "name=value" lines.
 *  - A lock whose process is gone on this host is stale: it is taken over, with a warning in the log. A lock of a
 *    process that runs, of this process (another run), of another host, or that cannot be read, is not.
 *  - When the folder is not on a file system the lock file cannot be made exclusively; then the lock is taken
 *    through the VFS (check, then write), which excludes runs that do not overlap at the very same moment only, and
 *    the log says so.
 *  - Release removes the file only when it still holds this lock's token.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CIntermediateLock
{
    public:

    static const CORE::CString LockFileName;        /**< ".lock" */

    /** Takes the lock of the folder (a VFS path); IsHeld() tells whether it worked */
    explicit CIntermediateLock( const CORE::CString& intermediateDir );

    /** Gives the lock back */
    ~CIntermediateLock();

    bool IsHeld( void ) const;

    /** When the lock was not taken: who holds it ("process 1234 on host build-1 since 2026-...") or why it failed */
    const CORE::CString& GetHolder( void ) const;

    /** Gives the lock back before the end of the scope */
    void Release( void );

    /** Who holds the lock of the folder now, as GetHolder() says it; false when nobody does */
    static bool GetCurrentHolder( const CORE::CString& intermediateDir, CORE::CString& holder );

    private:

    struct SLockInfo
    {
        UInt32 processId;
        CORE::CString host;
        CORE::CString started;
        CORE::CString token;
    };

    CIntermediateLock( const CIntermediateLock& src );              /**< not implemented */
    CIntermediateLock& operator=( const CIntermediateLock& src );   /**< not implemented */

    bool Acquire( void );
    bool ResolveNativeLockPath( CORE::CString& nativeLockPath ) const;
    bool ReadLock( SLockInfo& info ) const;
    static bool ParseLock( const CORE::CString& text, SLockInfo& info );
    static CORE::CString DescribeHolder( const SLockInfo& info );
    bool IsStale( const SLockInfo& info ) const;
    bool RemoveLockFile( void );

    CORE::CString m_intermediateDir;
    CORE::CString m_vfsLockPath;
    CORE::CString m_nativeLockPath;     /**< empty when the folder is not on a file system */
    CORE::CString m_token;
    CORE::CString m_holder;
    bool m_held;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CINTERMEDIATELOCK_H ? */
