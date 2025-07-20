/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CASYNC_H
#define GUCEF_CORE_CASYNC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_SFINAE_UTILS_H
#include "gucefCORE_SFINAE_utils.h"
#define GUCEF_CORE_SFINAE_UTILS_H
#endif /* GUCEF_CORE_SFINAE_UTILS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

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

template< typename R, typename A1, typename a1IsTaskPtr = typename TypesAreExactlySame< CTaskPtr, A1 >::plain_value_type >
class CASyncChainStepArity1;
template< typename R, typename A1, typename A2, typename a1IsTaskPtr = typename TypesAreExactlySame< CTaskPtr, A1 >::plain_value_type >
class CASyncChainStepArity2;
template< typename R, typename A1, typename A2, typename A3, typename a1IsTaskPtr = typename TypesAreExactlySame< CTaskPtr, A1 >::plain_value_type >
class CASyncChainStepArity3;
template< typename R, typename A1, typename A2, typename A3, typename A4, typename a1IsTaskPtr = typename TypesAreExactlySame< CTaskPtr, A1 >::plain_value_type >
class CASyncChainStepArity4;
template< typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename a1IsTaskPtr = typename TypesAreExactlySame< CTaskPtr, A1 >::plain_value_type >
class CASyncChainStepArity5;
template< typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename a1IsTaskPtr = typename TypesAreExactlySame< CTaskPtr, A1 >::plain_value_type >
class CASyncChainStepArity6;

/*-------------------------------------------------------------------------*/

/**
 *  Class which provides syntactic sugar around easily defining and submitting
 *  a chain of 'work' for asynchronous processing using the framework's thread pools
 *  This class is intended to be used as a local scope helper object
 *
 *  Also check the CASyncChainStepArity4<> templates as they offer additional functionality
 *  Which is not available on the CASync base class. Those derived classes are for use-cases
 *  where type information from the last chain task carries over to the next.
 */
class GUCEF_CORE_PUBLIC_CPP CASync
{
    public:

    CASync( const CString& threadPoolName = CString::Empty );

    CASync( ThreadPoolPtr threadPool );

    /**
     *  Starts an async work chain with the instruction to start right away creating a dedicated thread if need be
     *  Generally its preferred that you use Queue() instead of Start() to avoid creating too many threads
     *
     *  @param taskType                     The textual task type name, must match a registered task consumer type
     *  @param taskData                     Any data to be passed as part of processing the task
     *  @param assumeOwnershipOfTaskData    Whether the passed 'taskData' (if any) lifecycle is delegated from this point onwards 
     */
    CASync& Start( const CString& taskType                ,
                   CICloneable* taskData = GUCEF_NULL     ,
                   bool assumeOwnershipOfTaskData = false );

    /**
     *  Starts an async work chain with the instruction to queue a task for execution as soon as a thread is available to execute it
     *  Generally this is preferred over Start() to avoid creating too many threads
     *
     *  @param taskType                     The textual task type name, must match a registered task consumer type
     *  @param taskData                     Any data to be passed as part of processing the task
     *  @param assumeOwnershipOfTaskData    Whether the passed 'taskData' (if any) lifecycle is delegated from this point onwards 
     */
    CASync& Queue( const CString& taskType                ,
                   CICloneable* taskData = GUCEF_NULL     ,
                   bool assumeOwnershipOfTaskData = false );

    /**
     *  Starts an async work chain with the instruction to queue a task, in the form of a function callback,
     *  for execution as soon as a thread is available to execute it.
     *
     *  @param f    The callback function pointer                                  
     */    
    template< typename R >
    CASync& QueueCallback( R (*f)() );

    /**
     *  Starts an async work chain with the instruction to queue a task, in the form of a function callback,
     *  for execution as soon as a thread is available to execute it.
     *
     *  @param f    The callback function pointer
     *  @param a1   param 1 to be given to the function at time of deferred invocation   
     */    
    template< typename R, typename A1 >
    CASyncChainStepArity1< R, A1 > QueueCallback( R (*f)(A1), A1 a1 );

    /**
     *  Starts an async work chain with the instruction to queue a task, in the form of a function callback,
     *  for execution as soon as a thread is available to execute it.
     *
     *  @param f    The callback function pointer
     *  @param a1   param 1 to be given to the function at time of deferred invocation
     *  @param a2   param 2 to be given to the function at time of deferred invocation
     */ 
    template< typename R, typename A1, typename A2 >
    CASyncChainStepArity2< R, A1, A2 > QueueCallback( R (*f)(A1, A2), A1 a1, A2 a2 );

    /**
     *  Starts an async work chain with the instruction to queue a task, in the form of a function callback,
     *  for execution as soon as a thread is available to execute it.
     *
     *  @param f    The callback function pointer
     *  @param a1   param 1 to be given to the function at time of deferred invocation
     *  @param a2   param 2 to be given to the function at time of deferred invocation
     *  @param a3   param 3 to be given to the function at time of deferred invocation
     */ 
    template< typename R, typename A1, typename A2, typename A3 >
    CASyncChainStepArity3< R, A1, A2, A3 > QueueCallback( R (*f)(A1, A2, A3), A1 a1, A2 a2, A3 a3 );

    /**
     *  Starts an async work chain with the instruction to queue a task, in the form of a function callback,
     *  for execution as soon as a thread is available to execute it.
     *
     *  @param f    The callback function pointer
     *  @param a1   param 1 to be given to the function at time of deferred invocation
     *  @param a2   param 2 to be given to the function at time of deferred invocation
     *  @param a3   param 3 to be given to the function at time of deferred invocation
     *  @param a4   param 4 to be given to the function at time of deferred invocation
     */ 
    template< typename R, typename A1, typename A2, typename A3, typename A4 >
    CASyncChainStepArity4< R, A1, A2, A3, A4 > QueueCallback( R (*f)(A1, A2, A3, A4), A1 a1, A2 a2, A3 a3, A4 a4 );

    /**
     *  Continues an async work chain with the instruction to add a callback task, in the form of a function callback,
     *  for execution as soon as a thread is available to execute it after completion of the prior task.
     * 
     *  Note that its mandatory for the callback to accept a CTaskPtr as its first argument which holds amongst other things
     *  the results of the prior task.
     *
     *  @param f    The callback function pointer
     */ 
    template< typename R >
    CASyncChainStepArity1< R, CTaskPtr > ThenCallback( R (*f)( CTaskPtr ) );

    /**
     *  Continues an async work chain with the instruction to add a callback task, in the form of a function callback,
     *  for execution as soon as a thread is available to execute it after completion of the prior task.
     * 
     *  Note that its mandatory for the callback to accept a CTaskPtr as its first argument which holds amongst other things
     *  the results of the prior task.
     *  The other argument are freely specified by the caller of the ThenCallback() templated member function.
     *
     *  @param f    The callback function pointer
     *  @param a2   param 2 to be given to the function at time of deferred invocation, note that param 1 to the callback is the mandatory CTaskPtr
     */ 
    template< typename R, typename A2 >
    CASyncChainStepArity2< R, CTaskPtr, A2 > ThenCallback( R (*f)( CTaskPtr, A2 ), A2 a2 );

    /**
     *  Continues an async work chain with the instruction to add a callback task, in the form of a function callback,
     *  for execution as soon as a thread is available to execute it after completion of the prior task.
     * 
     *  Note that its mandatory for the callback to accept a CTaskPtr as its first argument which holds amongst other things
     *  the results of the prior task.
     *  The other argument are freely specified by the caller of the ThenCallback() templated member function.
     *
     *  @param f    The callback function pointer
     *  @param a2   param 2 to be given to the function at time of deferred invocation, note that param 1 to the callback is the mandatory CTaskPtr
     *  @param a3   param 3 to be given to the function at time of deferred invocation
     */ 
    template< typename R, typename A2, typename A3 >
    CASyncChainStepArity3< R, CTaskPtr, A2, A3 > ThenCallback( R (*f)( CTaskPtr, A2, A3 ), A2 a2, A3 a3 );

    /**
     *  Once you are done defining your chain of async tasks you can call this to explicitly end the chain.
     *  This will submit the chain for execution at the specified thread pool
     */
    CFutureResult Submit( void );

    /**
     *  Conversion operator to CFutureResult is provided to make calling Submit() optional
     *  If you assign the end of an CASync call chain to a CFutureResult it has the same result
     */
    operator CFutureResult();

    /**
     *  Prepares the async object for reuse
     *  The thread pool association remains but the rest will be reset
     */
    void ClearChain( void );

    protected:

    CASync( const CASync& thisChain );

    CASync& SetLastTaskStatus( TTaskStatus taskStatus );

    CASync& QueueCallbackCommonImpl( CICloneable* taskData );

    CASync& ThenCallbackCommonImpl( CICloneable* taskData );

    CASync& StartChain( const CString& taskType        ,
                        CICloneable* taskData          ,
                        bool assumeOwnershipOfTaskData ,
                        bool startRightAwayOnSubmit    );

    CTaskPtr GetLastTask( void ) const;

    private:

    class GUCEF_CORE_PRIVATE_CPP CASyncChainState : public CTSharedObjCreator< CASyncChainState, MT::CNoLock >
    {
        public:

        typedef CTSharedObjCreator< CASyncChainState, MT::CNoLock > TSharedObjCreator;
        typedef TSharedObjCreator::TBasicSharedPtrType              TASyncChainStatePtr;

        ThreadPoolPtr m_threadPool;
        CTaskPtr m_lastTask;
        bool m_chainIsHealthy;
        bool m_startRightAwayOnSubmit;
        bool m_chainHasBeenSubmitted;

        CASyncChainState( const CString& threadPoolName = CString::Empty );

        CASyncChainState( ThreadPoolPtr threadPool );
    };

    typedef CASyncChainState::TASyncChainStatePtr TASyncChainStatePtr;

    TASyncChainStatePtr m_state;
};

/*-------------------------------------------------------------------------*/

template< typename R, typename A1 >
class CASyncChainStepArity1< R, A1, plain_true > : public CASync
{
    public:

    typedef R           TResultType;
    typedef CTaskPtr    TArgument1Type;

    CASyncChainStepArity1( CASync& thisChain );

    /**
     *  Continues an async work chain with the instruction to add a callback task, in the form of a function callback,
     *  for execution as soon as a thread is available to execute it after completion of the prior task.
     * 
     *  Note that its mandatory for the callback to accept a CTaskPtr as its first argument which holds amongst other things
     *  the results of the prior task.
     *
     *  A key difference with this function, ThenPassToCallback(), versus ThenCallback() is that instead of passing in new additional
     *  parameters to the callback the prior tasks's arguments, plus the mandatory CTaskPtr as the first parameter, are mapped to the
     *  new callback provided. This simplifies the code in the callback since, depending on your use-case, you might not need to distill
     *  result data from the prior task from the Task object itself.
     *
     *  @param f    The callback function pointer, it needs to match the params of the prior callback plus CTaskPtr as the first param
     */    
    CASyncChainStepArity1< R, CTaskPtr >
    ThenPassToCallback( R (*f)( CTaskPtr, R ) );
};

template< typename R, typename A1 >
class CASyncChainStepArity1< R, A1, plain_false > : public CASync
{
    public:

    typedef R           TResultType;
    typedef A1          TArgument1Type;

    CASyncChainStepArity1( CASync& thisChain );

    /**
     *  Continues an async work chain with the instruction to add a callback task, in the form of a function callback,
     *  for execution as soon as a thread is available to execute it after completion of the prior task.
     * 
     *  Note that its mandatory for the callback to accept a CTaskPtr as its first argument which holds amongst other things
     *  the results of the prior task.
     *
     *  A key difference with this function, ThenPassToCallback(), versus ThenCallback() is that instead of passing in new additional
     *  parameters to the callback the prior tasks's arguments, plus the mandatory CTaskPtr as the first parameter, are mapped to the
     *  new callback provided. This simplifies the code in the callback since, depending on your use-case, you might not need to distill
     *  result data from the prior task from the Task object itself.
     *
     *  @param f    The callback function pointer, it needs to match the params of the prior callback plus CTaskPtr as the first param
     */    
    CASyncChainStepArity2< R, CTaskPtr, A1 >
    ThenPassToCallback( R (*f)( CTaskPtr, R, A1 ) );
};

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2 >
class CASyncChainStepArity2< R, A1, A2, plain_true > : public CASync
{
    public:

    typedef R           TResultType;
    typedef CTaskPtr    TArgument1Type;
    typedef A2          TArgument2Type;

    CASyncChainStepArity2( CASync& thisChain );

    CASyncChainStepArity2< R, CTaskPtr, A2 >
    ThenPassToCallback( R (*f)( CTaskPtr, R, A2 ) );
};

template< typename R, typename A1, typename A2 >
class CASyncChainStepArity2< R, A1, A2, plain_false > : public CASync
{
    public:

    typedef R           TResultType;
    typedef A1          TArgument1Type;
    typedef A2          TArgument2Type;

    CASyncChainStepArity2( CASync& thisChain );

    CASyncChainStepArity3< R, CTaskPtr, A1, A2 >
    ThenPassToCallback( R (*f)( CTaskPtr, R, A1, A2 ) );
};

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3 >
class CASyncChainStepArity3< R, A1, A2, A3, plain_true > : public CASync
{
    public:

    typedef R           TResultType;
    typedef CTaskPtr    TArgument1Type;
    typedef A2          TArgument2Type;
    typedef A3          TArgument3Type;

    CASyncChainStepArity3( CASync& thisChain );

    CASyncChainStepArity3< R, CTaskPtr, A2, A3 >
    ThenPassToCallback( R (*f)( CTaskPtr, R, A2, A3 ) );
};

template< typename R, typename A1, typename A2, typename A3 >
class CASyncChainStepArity3< R, A1, A2, A3, plain_false > : public CASync
{
    public:

    typedef R           TResultType;
    typedef A1          TArgument1Type;
    typedef A2          TArgument2Type;
    typedef A3          TArgument3Type;

    CASyncChainStepArity3( CASync& thisChain );

    CASyncChainStepArity4< R, CTaskPtr, A1, A2, A3 >
    ThenPassToCallback( R (*f)( CTaskPtr, R, A1, A2, A3 ) );
};

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3, typename A4 >
class CASyncChainStepArity4< R, A1, A2, A3, A4, plain_true > : public CASync
{
    public:

    typedef R           TResultType;
    typedef CTaskPtr    TArgument1Type;
    typedef A2          TArgument2Type;
    typedef A3          TArgument3Type;
    typedef A4          TArgument4Type;

    CASyncChainStepArity4( CASync& thisChain );

    CASyncChainStepArity4< R, CTaskPtr, A2, A3, A4 >
    ThenPassToCallback( R (*f)( CTaskPtr, R, A2, A3, A4 ) );
};

template< typename R, typename A1, typename A2, typename A3, typename A4 >
class CASyncChainStepArity4< R, A1, A2, A3, A4, plain_false > : public CASync
{
    public:

    typedef R           TResultType;
    typedef A1          TArgument1Type;
    typedef A2          TArgument2Type;
    typedef A3          TArgument3Type;
    typedef A4          TArgument4Type;

    CASyncChainStepArity4( CASync& thisChain );

    CASyncChainStepArity5< R, CTaskPtr, A1, A2, A3, A4 >
    ThenPassToCallback( R (*f)( CTaskPtr, R, A1, A2, A3, A4 ) );
};

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3, typename A4, typename A5 >
class CASyncChainStepArity5< R, A1, A2, A3, A4, A5, plain_true > : public CASync
{
    public:

    typedef R           TResultType;
    typedef CTaskPtr    TArgument1Type;
    typedef A2          TArgument2Type;
    typedef A3          TArgument3Type;
    typedef A4          TArgument4Type;
    typedef A5          TArgument5Type;

    CASyncChainStepArity5( CASync& thisChain );

    CASyncChainStepArity5< R, CTaskPtr, A2, A3, A4, A5 >
    ThenPassToCallback( R (*f)( CTaskPtr, R, A2, A3, A4, A5 ) );
};

template< typename R, typename A1, typename A2, typename A3, typename A4, typename A5 >
class CASyncChainStepArity5< R, A1, A2, A3, A4, A5, plain_false > : public CASync
{
    public:

    typedef R           TResultType;
    typedef A1          TArgument1Type;
    typedef A2          TArgument2Type;
    typedef A3          TArgument3Type;
    typedef A4          TArgument4Type;
    typedef A5          TArgument5Type;

    CASyncChainStepArity5( CASync& thisChain );

    CASyncChainStepArity6< R, CTaskPtr, A1, A2, A3, A4, A5 >
    ThenPassToCallback( R (*f)( CTaskPtr, R, A1, A2, A3, A4, A5 ) );
};

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6 >
class CASyncChainStepArity6< R, A1, A2, A3, A4, A5, A6, plain_true > : public CASync
{
    public:

    typedef R           TResultType;
    typedef CTaskPtr    TArgument1Type;
    typedef A2          TArgument2Type;
    typedef A3          TArgument3Type;
    typedef A4          TArgument4Type;
    typedef A5          TArgument5Type;
    typedef A6          TArgument6Type;

    CASyncChainStepArity6( CASync& thisChain );

    CASyncChainStepArity6< R, CTaskPtr, A2, A3, A4, A5, A6 >
    ThenPassToCallback( R (*f)( CTaskPtr, R, A2, A3, A4, A5, A6 ) );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename R >
CASync&
CASync::QueueCallback( R (*f)() )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::Construct( f );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

   return QueueCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1 >
CASyncChainStepArity1< R, A1 >
CASync::QueueCallback( R (*f)(A1), A1 a1 )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::Construct( f, a1 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

   return QueueCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2 >
CASyncChainStepArity2< R, A1, A2 >
CASync::QueueCallback( R (*f)(A1, A2), A1 a1, A2 a2 )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::Construct( f, a1, a2 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

   return QueueCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3 >
CASyncChainStepArity3< R, A1, A2, A3 >
CASync::QueueCallback( R (*f)(A1, A2, A3), A1 a1, A2 a2, A3 a3 )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::Construct( f, a1, a2, a3 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

   return QueueCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3, typename A4 >
CASyncChainStepArity4< R, A1, A2, A3, A4 >
CASync::QueueCallback( R (*f)(A1, A2, A3, A4), A1 a1, A2 a2, A3 a3, A4 a4 )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::Construct( f, a1, a2, a3, a4 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

   return QueueCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R >
CASyncChainStepArity1< R, CTaskPtr >
CASync::ThenCallback( R (*f)( CTaskPtr ) )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::Construct( f, GetLastTask() );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A2 >
CASyncChainStepArity2< R, CTaskPtr, A2 >
CASync::ThenCallback( R (*f)( CTaskPtr, A2 ), A2 a2 )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::Construct( f, GetLastTask(), a2 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A2, typename A3 >
CASyncChainStepArity3< R, CTaskPtr, A2, A3 >
CASync::ThenCallback( R (*f)( CTaskPtr, A2, A3 ), A2 a2, A3 a3 )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::Construct( f, GetLastTask(), a2, a3 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1 >
CASyncChainStepArity1< R, A1, plain_true >::CASyncChainStepArity1( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1 >
CASyncChainStepArity1< R, CTaskPtr >
CASyncChainStepArity1< R, A1, plain_true >::ThenPassToCallback( R (*f)( CTaskPtr, R ) )
{GUCEF_TRACE;

    typedef R (*TGivenCallBackFunc)( CTaskPtr, R, A1 );
    typedef R (*TPriorGivenCallBackFunc)( A1 );
    typedef deferred_invoker< TPriorGivenCallBackFunc, arity_1 >      TPriorDeferredInvoker;

    // Obtain access to prior invoker
    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
    CICloneable* opaqueTaskData = lastTask->GetTaskData();
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == opaqueTaskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    TPriorDeferredInvoker* priorInvoker = static_cast< TPriorDeferredInvoker* >( opaqueTaskData );

    // Build a callback invoker as task data of the forwarding variety
    CICloneable* taskData = CDeferredTask::ResultFwdConstruct< CTaskPtr, TPriorDeferredInvoker, TGivenCallBackFunc >( lastTask, priorInvoker, f );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1 >
CASyncChainStepArity1< R, A1, plain_false >::CASyncChainStepArity1( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1 >
CASyncChainStepArity2< R, CTaskPtr, A1 >
CASyncChainStepArity1< R, A1, plain_false >::ThenPassToCallback( R (*f)( CTaskPtr, R, A1 ) )
{GUCEF_TRACE;

    typedef R (*TGivenCallBackFunc)( CTaskPtr, R, A1 );
    typedef R (*TPriorGivenCallBackFunc)( A1 );
    typedef deferred_invoker< TPriorGivenCallBackFunc, arity_1 >      TPriorDeferredInvoker;

    // Obtain access to prior invoker
    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
    CICloneable* opaqueTaskData = lastTask->GetTaskData();
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == opaqueTaskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    TPriorDeferredInvoker* priorInvoker = static_cast< TPriorDeferredInvoker* >( opaqueTaskData );

    // Build a callback invoker as task data of the forwarding variety
    CICloneable* taskData = CDeferredTask::ResultFwdConstruct< CTaskPtr, TPriorDeferredInvoker, TGivenCallBackFunc >( lastTask, priorInvoker, f );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3, typename A4 >
CASyncChainStepArity4< R, A1, A2, A3, A4, plain_true >::CASyncChainStepArity4( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3, typename A4 >
CASyncChainStepArity4< R, CTaskPtr, A2, A3, A4 >
CASyncChainStepArity4< R, A1, A2, A3, A4, plain_true >::ThenPassToCallback( R (*f)( CTaskPtr, R, A2, A3, A4 ) )
{GUCEF_TRACE;

    typedef R (*TGivenCallBackFunc)( CTaskPtr, R, A2, A3, A4 );
    typedef R (*TPriorGivenCallBackFunc)( A1, A2, A3, A4 );
    typedef deferred_invoker< TPriorGivenCallBackFunc, arity_4 >      TPriorDeferredInvoker;

    // Obtain access to prior invoker
    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
    CICloneable* opaqueTaskData = lastTask->GetTaskData();
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == opaqueTaskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    TPriorDeferredInvoker* priorInvoker = static_cast< TPriorDeferredInvoker* >( opaqueTaskData );

    // Build a callback invoker as task data of the forwarding variety
    CICloneable* taskData = CDeferredTask::ResultFwdConstruct< CTaskPtr, TPriorDeferredInvoker, TGivenCallBackFunc >( lastTask, priorInvoker, f );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3, typename A4 >
CASyncChainStepArity4< R, A1, A2, A3, A4, plain_false >::CASyncChainStepArity4( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3, typename A4 >
CASyncChainStepArity5< R, CTaskPtr, A1, A2, A3, A4 >
CASyncChainStepArity4< R, A1, A2, A3, A4, plain_false >::ThenPassToCallback( R (*f)( CTaskPtr, R, A1, A2, A3, A4 ) )
{GUCEF_TRACE;

    typedef R (*TGivenCallBackFunc)( CTaskPtr, R, A1, A2, A3, A4 );
    typedef R (*TPriorGivenCallBackFunc)( A1, A2, A3, A4 );
    typedef deferred_invoker< TPriorGivenCallBackFunc, arity_4 >      TPriorDeferredInvoker;

    // Obtain access to prior invoker
    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
    CICloneable* opaqueTaskData = lastTask->GetTaskData();
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == opaqueTaskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    TPriorDeferredInvoker* priorInvoker = static_cast< TPriorDeferredInvoker* >( opaqueTaskData );

    // Build a callback invoker as task data of the forwarding variety
    CICloneable* taskData = CDeferredTask::ResultFwdConstruct( lastTask, priorInvoker, f );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3, typename A4, typename A5 >
CASyncChainStepArity5< R, A1, A2, A3, A4, A5, plain_true >::CASyncChainStepArity5( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3, typename A4, typename A5 >
CASyncChainStepArity5< R, CTaskPtr, A2, A3, A4, A5 >
CASyncChainStepArity5< R, A1, A2, A3, A4, A5, plain_true >::ThenPassToCallback( R (*f)( CTaskPtr, R, A2, A3, A4, A5 ) )
{GUCEF_TRACE;

    typedef R (*TGivenCallBackFunc)( CTaskPtr, R, A2, A3, A4, A5 );
    typedef R (*TPriorGivenCallBackFunc)( A1, A2, A3, A4 );
    typedef deferred_invoker< TPriorGivenCallBackFunc, arity_4 >      TPriorDeferredInvoker;

    // Obtain access to prior invoker
    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
    CICloneable* opaqueTaskData = lastTask->GetTaskData();
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == opaqueTaskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    TPriorDeferredInvoker* priorInvoker = static_cast< TPriorDeferredInvoker* >( opaqueTaskData );

    // Build a callback invoker as task data of the forwarding variety
    CICloneable* taskData = CDeferredTask::ResultFwdConstruct< CTaskPtr, TPriorDeferredInvoker, TGivenCallBackFunc >( lastTask, priorInvoker, f );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3, typename A4, typename A5 >
CASyncChainStepArity5< R, A1, A2, A3, A4, A5, plain_false >::CASyncChainStepArity5( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3, typename A4, typename A5 >
CASyncChainStepArity6< R, CTaskPtr, A1, A2, A3, A4, A5 >
CASyncChainStepArity5< R, A1, A2, A3, A4, A5, plain_false >::ThenPassToCallback( R (*f)( CTaskPtr, R, A1, A2, A3, A4, A5 ) )
{GUCEF_TRACE;

    typedef R (*TGivenCallBackFunc)( CTaskPtr, R, A1, A2, A3, A4, A5 );
    typedef R (*TPriorGivenCallBackFunc)( A1, A2, A3, A4, A5 );
    typedef deferred_invoker< TPriorGivenCallBackFunc, arity_4 >      TPriorDeferredInvoker;

    // Obtain access to prior invoker
    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
    CICloneable* opaqueTaskData = lastTask->GetTaskData();
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == opaqueTaskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    TPriorDeferredInvoker* priorInvoker = static_cast< TPriorDeferredInvoker* >( opaqueTaskData );

    // Build a callback invoker as task data of the forwarding variety
    CICloneable* taskData = CDeferredTask::ResultFwdConstruct< CTaskPtr, TPriorDeferredInvoker, TGivenCallBackFunc >( lastTask, priorInvoker, f );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6 >
CASyncChainStepArity6< R, A1, A2, A3, A4, A5, A6, plain_true >::CASyncChainStepArity6( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6 >
CASyncChainStepArity6< R, CTaskPtr, A2, A3, A4, A5, A6 >
CASyncChainStepArity6< R, A1, A2, A3, A4, A5, A6, plain_true >::ThenPassToCallback( R (*f)( CTaskPtr, R, A2, A3, A4, A5, A6 ) )
{GUCEF_TRACE;

    typedef R (*TGivenCallBackFunc)( CTaskPtr, R, A2, A3, A4, A5, A6 );
    typedef R (*TPriorGivenCallBackFunc)( A1, A2, A3, A4, A5 );
    typedef deferred_invoker< TPriorGivenCallBackFunc, arity_5 >      TPriorDeferredInvoker;

    // Obtain access to prior invoker
    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
    CICloneable* opaqueTaskData = lastTask->GetTaskData();
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == opaqueTaskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    TPriorDeferredInvoker* priorInvoker = static_cast< TPriorDeferredInvoker* >( opaqueTaskData );

    // Build a callback invoker as task data of the forwarding variety
    CICloneable* taskData = CDeferredTask::ResultFwdConstruct< CTaskPtr, TPriorDeferredInvoker, TGivenCallBackFunc >( lastTask, priorInvoker, f );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CASYNC_H ? */
