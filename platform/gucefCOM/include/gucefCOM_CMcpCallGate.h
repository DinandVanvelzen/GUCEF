/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CMCPCALLGATE_H
#define GUCEF_COM_CMCPCALLGATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_COM_CMCPREQUESTCONTEXT_H
#include "gucefCOM_CMcpRequestContext.h"
#define GUCEF_COM_CMCPREQUESTCONTEXT_H
#endif /* GUCEF_COM_CMCPREQUESTCONTEXT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Counts the calls that run in the tools (resources, prompts) of one tool set, so that the set can be taken away
 *  while a server serves on: unregistering a tool set whose calls still run in its code would pull the code and the
 *  objects they use from under them.
 *
 *  - A call enters before it does its work and leaves when it is done (CMcpCallScope does both). Entering fails
 *    while the gate is closed: the tool answers that it is no longer available.
 *  - Close() lets no new call in, asks every call inside to stop (the cancellation flag of its request context, which
 *    a well behaved tool polls with CMcpRequestContext::IsCancellationRequested()) and waits until they left, at most
 *    the given time. A call that does not stop in time is reported; it keeps its count, so a later Close() waits
 *    for it again.
 *  - Open() lets calls in again, for a tool set that is registered anew.
 *
 *  Thread safe: calls enter and leave on the protocol thread, pool threads and in-process callers. Close() blocks the
 *  calling thread (it must not be the only thread that could finish the calls, so not inside a call of the set).
 *  The lock is a leaf.
 */
class GUCEF_COM_PUBLIC_CPP CMcpCallGate
{
    public:

    CMcpCallGate( void );

    /** Closes the gate without waiting; logs calls that are still inside */
    ~CMcpCallGate();

    /**
     *  A call enters. 'context' (may be GUCEF_NULL for a call without one) is told to cancel when the gate closes
     *  while the call is inside.
     *  @param ticket   to give back to Leave()
     *  @return false when the gate is closed: the call must not run
     */
    bool Enter( CMcpRequestContext* context, UInt64& ticket );

    /** The call of the ticket left */
    void Leave( UInt64 ticket );

    /**
     *  No call enters from now on; the calls inside are asked to cancel; waits until they left, at most timeoutMs
     *  @return true when no call is inside any more
     */
    bool Close( UInt32 timeoutMs );

    /** Calls may enter again */
    void Open( void );

    bool IsOpen( void ) const;

    /** For a call inside: true once Close() was called */
    bool IsCloseRequested( void ) const;

    /** The number of calls inside */
    UInt32 GetActiveCount( void ) const;

    private:

    typedef GUCEF::map< UInt64, CMcpRequestContextPtr > TCallMap;

    CMcpCallGate( const CMcpCallGate& src );                /**< not implemented */
    CMcpCallGate& operator=( const CMcpCallGate& src );     /**< not implemented */

    mutable MT::CMutex m_lock;
    bool m_open;
    UInt64 m_nextTicket;
    TCallMap m_calls;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPCALLGATE_H ? */
