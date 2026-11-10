/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#define GUCEF_CORE_CFUTURERESULTSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The producing side of a CFutureResult: the 'promise'.
 *
 *  A CFutureResult handed out by the task manager is completed by the task manager's own threads.
 *  Work that is carried out some other way, on a dedicated thread, a thread GUCEF did not create, an
 *  interrupt or driver callback, a hardware event or later a coroutine, can not complete such a future.
 *  This class lets any such code create a future and complete it from any thread:
 *
 *      CFutureResultSource source;
 *      CFutureResult future = source.GetFuture();      // hand this to the caller
 *      // ... later, on whatever thread finishes the work:
 *      source.Complete();
 *
 *  The first completion wins, later attempts return false. The source can be copied freely (copies refer to
 *  the same future), which is how it is passed to the thread that finishes the work.
 *  Completing is not real time safe (it takes locks, may notify observers): code on a real time thread
 *  records the outcome and lets another thread complete the source.
 *
 *  A source that is dropped without ever being completed leaves its future pending forever, as the work might
 *  still be completed by something that holds a copy. Waiters should use the timeout of CFutureResult::Await().
 */
class GUCEF_CORE_PUBLIC_CPP CFutureResultSource
{
    public:

    /** Creates a future in the running state: the work is under way somewhere */
    CFutureResultSource( void );

    /** Creates a future in the given non end state, for example TASKSTATUS_QUEUED when the work is waiting for something */
    explicit CFutureResultSource( TTaskStatus initialStatus );

    CFutureResultSource( const CFutureResultSource& src );

    CFutureResultSource& operator=( const CFutureResultSource& src );

    ~CFutureResultSource();

    /** The future that this source completes. All futures from one source (and its copies) are the same future */
    CFutureResult GetFuture( void ) const;

    /**
     *  Sets a status that is not an end state: queued, startup, running, paused, resumed, and also the conditions that
     *  may clear again such as TASKSTATUS_RESOURCE_LIMIT_REACHED (a throttle). The future stays pending, while it is in
     *  an error status HasNoFuture() says so. False if the future already ended or the status is an end state.
     */
    bool SetStatus( TTaskStatus status );

    /** Ends the future successfully (TASKSTATUS_FINISHED). False if it had already ended */
    bool Complete( void );

    /** Same, with result data that the waiter can reach through the task that CFutureResult::GetResult() returns. Ownership of the data is taken when asked */
    bool Complete( CICloneable* resultData, bool assumeOwnershipOfResultData );

    /** Ends the future in the given end state, with optional extra info for the waiter. False if it had already ended or the status is not an end state */
    bool Complete( TTaskStatus endState, const CString& extraInfo = CString() );

    /**
     *  Ends the future in an error status that is also an end state (TaskStatusIsAnError() and TaskStatusIsAnEndState()).
     *  Error statuses that are not end states, like TASKSTATUS_RESOURCE_LIMIT_REACHED, describe a condition that may still
     *  clear: those are refused here (false), use SetStatus() for them. False as well if the future had already ended.
     */
    bool Fail( TTaskStatus errorStatus, const CString& extraInfo = CString() );

    /** Ends the future as cancelled */
    bool Cancel( void );

    /** True once the future is in an end state */
    bool IsCompleted( void ) const;

    /** True when a waiter called RequestCancellation() on the future. The code doing the work should look and end the future with Cancel() */
    bool IsCancellationRequested( void ) const;

    private:

    bool EndWith( TTaskStatus endState, const CString& extraInfo, CICloneable* resultData, bool assumeOwnershipOfResultData );

    private:

    CTaskPtr m_task;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */
