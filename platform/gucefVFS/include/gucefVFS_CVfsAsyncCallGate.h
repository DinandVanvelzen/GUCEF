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


#ifndef GUCEF_VFS_CVFSASYNCCALLGATE_H
#define GUCEF_VFS_CVFSASYNCCALLGATE_H

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
 *  The door that the calls on pool threads pass to use their owner (the VFS): while it is open a call enters, uses the owner
 *  and leaves; when the owner goes away it closes the door and waits for the calls that are inside to leave. A call that
 *  comes to the door after that finds it shut and does not touch the owner, which is how a call that was still queued when
 *  the owner went is harmless. The gate is shared by the owner and by every call, so that it is there for the call
 *  that arrives late.
 *
 *  It is a CIVfsCancelSource: a call inside asks it whether it is to stop, which it is once the door is shut.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsAsyncCallGate : public CIVfsCancelSource
{
    public:

    CVfsAsyncCallGate( void );

    virtual ~CVfsAsyncCallGate() GUCEF_VIRTUAL_OVERRIDE;

    /** Enters when the door is open; a call that entered has to Leave() */
    bool TryEnter( void );

    void Leave( void );

    /** Shuts the door and waits for the calls inside to leave; false when they did not within the time (they are left to end by themselves) */
    bool CloseAndWait( UInt32 timeoutInMs );

    bool IsClosed( void ) const;

    /** The calls inside now */
    UInt32 GetInside( void ) const;

    /** True once the door is shut */
    virtual bool IsCancelRequested( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CVfsAsyncCallGate( const CVfsAsyncCallGate& src );                  /**< not implemented */
    CVfsAsyncCallGate& operator=( const CVfsAsyncCallGate& src );       /**< not implemented */

    mutable MT::CMutex m_lock;
    bool m_closed;
    UInt32 m_inside;
};

typedef CORE::CTSharedPtr< CVfsAsyncCallGate, MT::CMutex > CVfsAsyncCallGatePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSASYNCCALLGATE_H ? */
