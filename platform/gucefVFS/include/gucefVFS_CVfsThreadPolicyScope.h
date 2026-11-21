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

#ifndef GUCEF_VFS_CVFSTHREADPOLICYSCOPE_H
#define GUCEF_VFS_CVFSTHREADPOLICYSCOPE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VFS_CVFSTHREADPOLICYREGISTRY_H
#include "gucefVFS_CVfsThreadPolicyRegistry.h"
#define GUCEF_VFS_CVFSTHREADPOLICYREGISTRY_H
#endif /* GUCEF_VFS_CVFSTHREADPOLICYREGISTRY_H ? */

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
 *  Gives the calling thread a rule for misses for as long as the object lives, and puts back what the thread had when
 *  it goes: no rule, or the rule that an outer scope or a registration made. Scopes nest.
 *
 *  Use it for "this one call must not wait". For a thread that must never wait for as long as it lives (a user interface
 *  thread) register the rule with CVfsThreadPolicyRegistry::SetThreadPolicy() instead.
 *  The scope belongs to the thread that made it and has to be destroyed on that thread.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsThreadPolicyScope
{
    public:

    /** Uses the registry of the VFS (CVfsGlobal) */
    explicit CVfsThreadPolicyScope( const CVfsThreadPolicy& policy );

    CVfsThreadPolicyScope( CVfsThreadPolicyRegistry& registry, const CVfsThreadPolicy& policy );

    ~CVfsThreadPolicyScope();

    private:

    CVfsThreadPolicyScope( const CVfsThreadPolicyScope& src );                  /**< not implemented */
    CVfsThreadPolicyScope& operator=( const CVfsThreadPolicyScope& src );       /**< not implemented */

    void Enter( const CVfsThreadPolicy& policy );

    CVfsThreadPolicyRegistry* m_registry;
    TThreadId m_threadId;
    bool m_hadPolicy;
    bool m_entered;
    CVfsThreadPolicy m_previous;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSTHREADPOLICYSCOPE_H ? */
