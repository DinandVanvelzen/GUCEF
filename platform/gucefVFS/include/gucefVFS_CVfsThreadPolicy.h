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

#ifndef GUCEF_VFS_CVFSTHREADPOLICY_H
#define GUCEF_VFS_CVFSTHREADPOLICY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VFS_MACROS_H
#include "gucefVFS_macros.h"
#define GUCEF_VFS_MACROS_H
#endif /* GUCEF_VFS_MACROS_H ? */

#ifndef GUCEF_VFS_ETYPES_H
#include "gucefVFS_ETypes.h"
#define GUCEF_VFS_ETYPES_H
#endif /* GUCEF_VFS_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a call does when the answer is not held locally and has to come from a remote place */
enum EMissAction
{
    MISSACTION_WAIT = 0,                /**< block the calling thread until the remote has answered */
    MISSACTION_FAIL_AND_FETCH,          /**< return at once and start a background fetch, an arrival or failure event follows */
    MISSACTION_FAIL                     /**< return at once and start nothing: a thread that only wants to know what is local */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The rule of a thread for a miss on a remote archive: whether it may wait for the remote. A thread that must never
 *  block (a user interface, a render loop, a thread that pumps something else) is given a rule that returns at once.
 *  A thread without a rule waits, which is how a synchronous call behaves everywhere else: it does its work on the
 *  calling thread.
 *
 *  This is a value. The rules of the threads are kept by CVfsThreadPolicyRegistry and set for a scope of code by
 *  CVfsThreadPolicyScope.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsThreadPolicy
{
    public:

    EMissAction missAction;             /**< WAIT: block on this thread. FAIL_AND_FETCH: return at once and start the fetch. FAIL: return at once, start nothing */
    bool serveStaleMetadata;            /**< FAIL_AND_FETCH: an entry past its time to live is answered and checked again in the background */
    bool openRequiresCompleteFile;      /**< FAIL_AND_FETCH: opening a file succeeds only when every block is held, else the call fails and a fetch of the whole file starts */

    /** WAIT, stale metadata is served, a partly held file opens */
    CVfsThreadPolicy( void );

    explicit CVfsThreadPolicy( EMissAction action );

    CVfsThreadPolicy( const CVfsThreadPolicy& src );

    CVfsThreadPolicy& operator=( const CVfsThreadPolicy& src );

    /** Whether a miss returns at once (FAIL_AND_FETCH or FAIL) */
    bool NeverWaits( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSTHREADPOLICY_H ? */
