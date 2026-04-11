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

/**
 *  Chain step returned when a prior step stored results via a raw member-function-pointer invoker.
 *  PriorInvoker is the exact invoker type (deferred_member_invoker or deferred_member_fwd_invoker).
 *  ArityTag is the number of prior user args stored in PriorInvoker (arity_0 = result-only).
 *  Same-type-prior rule: only ThenPassToMemberCallback(raw) is provided here.
 */
template< typename PriorInvoker, typename ArityTag > class CASyncRawMemberChainStep;

/**
 *  Chain step returned when a prior step stored results via a shared-pointer member-function invoker.
 *  Same-type-prior rule: only ThenPassToMemberCallback(shared) is provided here.
 */
template< typename SharedPriorInvoker, typename ArityTag > class CASyncSharedMemberChainStep;

/*-------------------------------------------------------------------------*/

/**
 *  Class which provides syntactic sugar around easily defining and submitting
 *  a chain of 'work' for asynchronous processing using the framework's thread pools
 *  This class is intended to be used as a local scope helper object
 *
 *  Also check the CASyncChainStepArity*<> templates as they offer additional functionality
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
     *  Starts an async work chain with a C++ member function callback via raw object pointer.
     *  The caller is responsible for ensuring the object remains alive until invocation.
     *
     *  @param obj  Raw pointer to the object whose member function will be called
     *  @param f    Pointer to the member function to call at time of deferred invocation
     */
    template< typename R, typename C >
    CASync& QueueMemberCallback( C* obj, R (C::*f)() );

    template< typename R, typename C, typename A1 >
    CASyncRawMemberChainStep< deferred_member_invoker< C, R (C::*)(A1), arity_1 >, arity_1 >
    QueueMemberCallback( C* obj, R (C::*f)(A1), A1 a1 );

    template< typename R, typename C, typename A1, typename A2 >
    CASyncRawMemberChainStep< deferred_member_invoker< C, R (C::*)(A1, A2), arity_2 >, arity_2 >
    QueueMemberCallback( C* obj, R (C::*f)(A1, A2), A1 a1, A2 a2 );

    template< typename R, typename C, typename A1, typename A2, typename A3 >
    CASyncRawMemberChainStep< deferred_member_invoker< C, R (C::*)(A1, A2, A3), arity_3 >, arity_3 >
    QueueMemberCallback( C* obj, R (C::*f)(A1, A2, A3), A1 a1, A2 a2, A3 a3 );

    template< typename R, typename C, typename A1, typename A2, typename A3, typename A4 >
    CASyncRawMemberChainStep< deferred_member_invoker< C, R (C::*)(A1, A2, A3, A4), arity_4 >, arity_4 >
    QueueMemberCallback( C* obj, R (C::*f)(A1, A2, A3, A4), A1 a1, A2 a2, A3 a3, A4 a4 );

    /**
     *  Starts an async work chain with a C++ member function callback via shared pointer.
     *  The shared pointer is stored by value inside the task, keeping the object alive for
     *  the full duration of the deferred invocation.
     *  Works with CTBasicSharedPtr<C,L>, CTSharedPtr<C,L>, or any type exposing
     *  TContainedType and GetPointerAlways().
     *
     *  @param obj  Shared pointer to the object (stored by value as a strong reference)
     *  @param f    Pointer to the member function to call at time of deferred invocation
     */
    template< typename R, typename SharedPtrT >
    CASync& QueueMemberCallback( SharedPtrT obj, R (SharedPtrT::TContainedType::*f)() );

    template< typename R, typename SharedPtrT, typename A1 >
    CASyncSharedMemberChainStep< deferred_member_shared_invoker< SharedPtrT, R (SharedPtrT::TContainedType::*)(A1), arity_1 >, arity_1 >
    QueueMemberCallback( SharedPtrT obj, R (SharedPtrT::TContainedType::*f)(A1), A1 a1 );

    template< typename R, typename SharedPtrT, typename A1, typename A2 >
    CASyncSharedMemberChainStep< deferred_member_shared_invoker< SharedPtrT, R (SharedPtrT::TContainedType::*)(A1, A2), arity_2 >, arity_2 >
    QueueMemberCallback( SharedPtrT obj, R (SharedPtrT::TContainedType::*f)(A1, A2), A1 a1, A2 a2 );

    template< typename R, typename SharedPtrT, typename A1, typename A2, typename A3 >
    CASyncSharedMemberChainStep< deferred_member_shared_invoker< SharedPtrT, R (SharedPtrT::TContainedType::*)(A1, A2, A3), arity_3 >, arity_3 >
    QueueMemberCallback( SharedPtrT obj, R (SharedPtrT::TContainedType::*f)(A1, A2, A3), A1 a1, A2 a2, A3 a3 );

    template< typename R, typename SharedPtrT, typename A1, typename A2, typename A3, typename A4 >
    CASyncSharedMemberChainStep< deferred_member_shared_invoker< SharedPtrT, R (SharedPtrT::TContainedType::*)(A1, A2, A3, A4), arity_4 >, arity_4 >
    QueueMemberCallback( SharedPtrT obj, R (SharedPtrT::TContainedType::*f)(A1, A2, A3, A4), A1 a1, A2 a2, A3 a3, A4 a4 );

    /**
     *  Continues an async work chain with a C++ member function callback via raw object pointer.
     *  The mandatory first parameter of the member function must be CTaskPtr, which carries
     *  the results of the prior task in the chain.
     *
     *  @param obj  Raw pointer to the object
     *  @param f    Pointer to the member function; first param must be CTaskPtr
     */
    template< typename R, typename C >
    CASyncRawMemberChainStep< deferred_member_invoker< C, R (C::*)(CTaskPtr), arity_1 >, arity_1 >
    ThenMemberCallback( C* obj, R (C::*f)(CTaskPtr) );

    template< typename R, typename C, typename A2 >
    CASyncRawMemberChainStep< deferred_member_invoker< C, R (C::*)(CTaskPtr, A2), arity_2 >, arity_2 >
    ThenMemberCallback( C* obj, R (C::*f)(CTaskPtr, A2), A2 a2 );

    template< typename R, typename C, typename A2, typename A3 >
    CASyncRawMemberChainStep< deferred_member_invoker< C, R (C::*)(CTaskPtr, A2, A3), arity_3 >, arity_3 >
    ThenMemberCallback( C* obj, R (C::*f)(CTaskPtr, A2, A3), A2 a2, A3 a3 );

    /**
     *  Continues an async work chain with a C++ member function callback via shared pointer.
     *  The mandatory first parameter of the member function must be CTaskPtr, which carries
     *  the results of the prior task in the chain.
     *  The shared pointer is stored by value, keeping the object alive through invocation.
     *
     *  @param obj  Shared pointer to the object (stored by value as a strong reference)
     *  @param f    Pointer to the member function; first param must be CTaskPtr
     */
    template< typename R, typename SharedPtrT >
    CASyncSharedMemberChainStep< deferred_member_shared_invoker< SharedPtrT, R (SharedPtrT::TContainedType::*)(CTaskPtr), arity_1 >, arity_1 >
    ThenMemberCallback( SharedPtrT obj, R (SharedPtrT::TContainedType::*f)(CTaskPtr) );

    template< typename R, typename SharedPtrT, typename A2 >
    CASyncSharedMemberChainStep< deferred_member_shared_invoker< SharedPtrT, R (SharedPtrT::TContainedType::*)(CTaskPtr, A2), arity_2 >, arity_2 >
    ThenMemberCallback( SharedPtrT obj, R (SharedPtrT::TContainedType::*f)(CTaskPtr, A2), A2 a2 );

    template< typename R, typename SharedPtrT, typename A2, typename A3 >
    CASyncSharedMemberChainStep< deferred_member_shared_invoker< SharedPtrT, R (SharedPtrT::TContainedType::*)(CTaskPtr, A2, A3), arity_3 >, arity_3 >
    ThenMemberCallback( SharedPtrT obj, R (SharedPtrT::TContainedType::*f)(CTaskPtr, A2, A3), A2 a2, A3 a3 );

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

    typedef CTask::TTaskIdVector    TTaskIdVector;

    CASync( const CASync& thisChain );

    CASync& SetLastTaskStatus( TTaskStatus taskStatus );

    CASync& QueueCallbackCommonImpl( CICloneable* taskData );

    CASync& ThenCallbackCommonImpl( CICloneable* taskData );

    CASync& StartChain( const CString& taskType        ,
                        CICloneable* taskData          ,
                        bool assumeOwnershipOfTaskData ,
                        bool startRightAwayOnSubmit    );

    bool SetNextTask( CTaskPtr& nextTask );

    CTaskPtr GetFirstTask( void ) const;

    CTaskPtr GetLastTask( void ) const;

    CTaskPtr GetFirstErrorStateTask( void ) const;

    bool GetTaskIdsInChain( TTaskIdVector& taskIds ) const;

    class GUCEF_CORE_PRIVATE_CPP CASyncChainState : public CTSharedObjCreator< CASyncChainState, MT::CNoLock >
    {
        public:

        typedef CTSharedObjCreator< CASyncChainState, MT::CNoLock > TSharedObjCreator;
        typedef TSharedObjCreator::TBasicSharedPtrType              TASyncChainStatePtr;
        typedef CTask::TTaskPtrVector                               TTaskPtrVector;

        ThreadPoolPtr m_threadPool;
        TTaskPtrVector m_tasks;
        bool m_chainIsHealthy;
        bool m_startRightAwayOnSubmit;
        bool m_chainHasBeenSubmitted;

        CASyncChainState( const CString& threadPoolName = CString::Empty );

        CASyncChainState( ThreadPoolPtr threadPool );

        virtual ~CASyncChainState();
    };

    typedef CASyncChainState::TASyncChainStatePtr TASyncChainStatePtr;

    TASyncChainStatePtr GetChainState( void ) const;

    private:

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

/*-------------------------------------------------------------------------*/

/**
 *  Chain step for raw-pointer member-function callbacks.
 *
 *  arity_0 specialization: prior invoker stored only m_result (no user args).
 *  ThenPassToMemberCallback takes a new raw member function f(CTaskPtr, PriorResult).
 */
template< typename PriorInvoker >
class CASyncRawMemberChainStep< PriorInvoker, arity_0 > : public CASync
{
    public:

    typedef typename PriorInvoker::TResult   TResult;

    CASyncRawMemberChainStep( CASync& thisChain );

    template< typename R2, typename C2 >
    CASyncRawMemberChainStep<
        deferred_member_fwd_invoker< CTaskPtr, C2, R2 (C2::*)(CTaskPtr, TResult), PriorInvoker, arity_0 >,
        arity_0 >
    ThenPassToMemberCallback( C2* obj, R2 (C2::*f)(CTaskPtr, TResult) );
};

/**
 *  arity_1 specialization: prior invoker stored m_result + m_a1 (1 user arg).
 *  ThenPassToMemberCallback takes f(CTaskPtr, PriorResult, PriorArg1).
 */
template< typename PriorInvoker >
class CASyncRawMemberChainStep< PriorInvoker, arity_1 > : public CASync
{
    public:

    typedef typename PriorInvoker::TResult   TResult;
    typedef typename PriorInvoker::TArg1     TArg1;

    CASyncRawMemberChainStep( CASync& thisChain );

    template< typename R2, typename C2 >
    CASyncRawMemberChainStep<
        deferred_member_fwd_invoker< CTaskPtr, C2, R2 (C2::*)(CTaskPtr, TResult, TArg1), PriorInvoker, arity_1 >,
        arity_0 >
    ThenPassToMemberCallback( C2* obj, R2 (C2::*f)(CTaskPtr, TResult, TArg1) );
};

/**
 *  arity_2 specialization: prior invoker stored m_result + m_a1 + m_a2.
 *  ThenPassToMemberCallback takes f(CTaskPtr, PriorResult, PriorArg1, PriorArg2).
 */
template< typename PriorInvoker >
class CASyncRawMemberChainStep< PriorInvoker, arity_2 > : public CASync
{
    public:

    typedef typename PriorInvoker::TResult   TResult;
    typedef typename PriorInvoker::TArg1     TArg1;
    typedef typename PriorInvoker::TArg2     TArg2;

    CASyncRawMemberChainStep( CASync& thisChain );

    template< typename R2, typename C2 >
    CASyncRawMemberChainStep<
        deferred_member_fwd_invoker< CTaskPtr, C2, R2 (C2::*)(CTaskPtr, TResult, TArg1, TArg2), PriorInvoker, arity_2 >,
        arity_0 >
    ThenPassToMemberCallback( C2* obj, R2 (C2::*f)(CTaskPtr, TResult, TArg1, TArg2) );
};

/**
 *  arity_3 specialization: prior invoker stored m_result + m_a1 + m_a2 + m_a3.
 */
template< typename PriorInvoker >
class CASyncRawMemberChainStep< PriorInvoker, arity_3 > : public CASync
{
    public:

    typedef typename PriorInvoker::TResult   TResult;
    typedef typename PriorInvoker::TArg1     TArg1;
    typedef typename PriorInvoker::TArg2     TArg2;
    typedef typename PriorInvoker::TArg3     TArg3;

    CASyncRawMemberChainStep( CASync& thisChain );

    template< typename R2, typename C2 >
    CASyncRawMemberChainStep<
        deferred_member_fwd_invoker< CTaskPtr, C2, R2 (C2::*)(CTaskPtr, TResult, TArg1, TArg2, TArg3), PriorInvoker, arity_3 >,
        arity_0 >
    ThenPassToMemberCallback( C2* obj, R2 (C2::*f)(CTaskPtr, TResult, TArg1, TArg2, TArg3) );
};

/**
 *  arity_4 specialization: prior invoker stored m_result + m_a1 + m_a2 + m_a3 + m_a4.
 */
template< typename PriorInvoker >
class CASyncRawMemberChainStep< PriorInvoker, arity_4 > : public CASync
{
    public:

    typedef typename PriorInvoker::TResult   TResult;
    typedef typename PriorInvoker::TArg1     TArg1;
    typedef typename PriorInvoker::TArg2     TArg2;
    typedef typename PriorInvoker::TArg3     TArg3;
    typedef typename PriorInvoker::TArg4     TArg4;

    CASyncRawMemberChainStep( CASync& thisChain );

    template< typename R2, typename C2 >
    CASyncRawMemberChainStep<
        deferred_member_fwd_invoker< CTaskPtr, C2, R2 (C2::*)(CTaskPtr, TResult, TArg1, TArg2, TArg3, TArg4), PriorInvoker, arity_4 >,
        arity_0 >
    ThenPassToMemberCallback( C2* obj, R2 (C2::*f)(CTaskPtr, TResult, TArg1, TArg2, TArg3, TArg4) );
};

/*-------------------------------------------------------------------------*/

/**
 *  Chain step for shared-pointer member-function callbacks.
 *
 *  arity_0 specialization: prior invoker stored only m_result (no user args).
 *  ThenPassToMemberCallback takes a shared-pointer object + new member function f(CTaskPtr, PriorResult).
 */
template< typename SharedPriorInvoker >
class CASyncSharedMemberChainStep< SharedPriorInvoker, arity_0 > : public CASync
{
    public:

    typedef typename SharedPriorInvoker::TResult   TResult;

    CASyncSharedMemberChainStep( CASync& thisChain );

    template< typename R2, typename SharedPtrT2 >
    CASyncSharedMemberChainStep<
        deferred_member_shared_fwd_invoker< CTaskPtr, SharedPtrT2, R2 (SharedPtrT2::TContainedType::*)(CTaskPtr, TResult), SharedPriorInvoker, arity_0 >,
        arity_0 >
    ThenPassToMemberCallback( SharedPtrT2 obj, R2 (SharedPtrT2::TContainedType::*f)(CTaskPtr, TResult) );
};

/**
 *  arity_1 specialization: prior invoker stored m_result + m_a1.
 */
template< typename SharedPriorInvoker >
class CASyncSharedMemberChainStep< SharedPriorInvoker, arity_1 > : public CASync
{
    public:

    typedef typename SharedPriorInvoker::TResult   TResult;
    typedef typename SharedPriorInvoker::TArg1     TArg1;

    CASyncSharedMemberChainStep( CASync& thisChain );

    template< typename R2, typename SharedPtrT2 >
    CASyncSharedMemberChainStep<
        deferred_member_shared_fwd_invoker< CTaskPtr, SharedPtrT2, R2 (SharedPtrT2::TContainedType::*)(CTaskPtr, TResult, TArg1), SharedPriorInvoker, arity_1 >,
        arity_0 >
    ThenPassToMemberCallback( SharedPtrT2 obj, R2 (SharedPtrT2::TContainedType::*f)(CTaskPtr, TResult, TArg1) );
};

/**
 *  arity_2 specialization: prior invoker stored m_result + m_a1 + m_a2.
 */
template< typename SharedPriorInvoker >
class CASyncSharedMemberChainStep< SharedPriorInvoker, arity_2 > : public CASync
{
    public:

    typedef typename SharedPriorInvoker::TResult   TResult;
    typedef typename SharedPriorInvoker::TArg1     TArg1;
    typedef typename SharedPriorInvoker::TArg2     TArg2;

    CASyncSharedMemberChainStep( CASync& thisChain );

    template< typename R2, typename SharedPtrT2 >
    CASyncSharedMemberChainStep<
        deferred_member_shared_fwd_invoker< CTaskPtr, SharedPtrT2, R2 (SharedPtrT2::TContainedType::*)(CTaskPtr, TResult, TArg1, TArg2), SharedPriorInvoker, arity_2 >,
        arity_0 >
    ThenPassToMemberCallback( SharedPtrT2 obj, R2 (SharedPtrT2::TContainedType::*f)(CTaskPtr, TResult, TArg1, TArg2) );
};

/**
 *  arity_3 specialization: prior invoker stored m_result + m_a1 + m_a2 + m_a3.
 */
template< typename SharedPriorInvoker >
class CASyncSharedMemberChainStep< SharedPriorInvoker, arity_3 > : public CASync
{
    public:

    typedef typename SharedPriorInvoker::TResult   TResult;
    typedef typename SharedPriorInvoker::TArg1     TArg1;
    typedef typename SharedPriorInvoker::TArg2     TArg2;
    typedef typename SharedPriorInvoker::TArg3     TArg3;

    CASyncSharedMemberChainStep( CASync& thisChain );

    template< typename R2, typename SharedPtrT2 >
    CASyncSharedMemberChainStep<
        deferred_member_shared_fwd_invoker< CTaskPtr, SharedPtrT2, R2 (SharedPtrT2::TContainedType::*)(CTaskPtr, TResult, TArg1, TArg2, TArg3), SharedPriorInvoker, arity_3 >,
        arity_0 >
    ThenPassToMemberCallback( SharedPtrT2 obj, R2 (SharedPtrT2::TContainedType::*f)(CTaskPtr, TResult, TArg1, TArg2, TArg3) );
};

/**
 *  arity_4 specialization: prior invoker stored m_result + m_a1 + m_a2 + m_a3 + m_a4.
 */
template< typename SharedPriorInvoker >
class CASyncSharedMemberChainStep< SharedPriorInvoker, arity_4 > : public CASync
{
    public:

    typedef typename SharedPriorInvoker::TResult   TResult;
    typedef typename SharedPriorInvoker::TArg1     TArg1;
    typedef typename SharedPriorInvoker::TArg2     TArg2;
    typedef typename SharedPriorInvoker::TArg3     TArg3;
    typedef typename SharedPriorInvoker::TArg4     TArg4;

    CASyncSharedMemberChainStep( CASync& thisChain );

    template< typename R2, typename SharedPtrT2 >
    CASyncSharedMemberChainStep<
        deferred_member_shared_fwd_invoker< CTaskPtr, SharedPtrT2, R2 (SharedPtrT2::TContainedType::*)(CTaskPtr, TResult, TArg1, TArg2, TArg3, TArg4), SharedPriorInvoker, arity_4 >,
        arity_0 >
    ThenPassToMemberCallback( SharedPtrT2 obj, R2 (SharedPtrT2::TContainedType::*f)(CTaskPtr, TResult, TArg1, TArg2, TArg3, TArg4) );
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

    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );

    CICloneable* taskData = CDeferredTask::Construct( f, lastTask );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A2 >
CASyncChainStepArity2< R, CTaskPtr, A2 >
CASync::ThenCallback( R (*f)( CTaskPtr, A2 ), A2 a2 )
{GUCEF_TRACE;

    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );

    CICloneable* taskData = CDeferredTask::Construct( f, lastTask, a2 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A2, typename A3 >
CASyncChainStepArity3< R, CTaskPtr, A2, A3 >
CASync::ThenCallback( R (*f)( CTaskPtr, A2, A3 ), A2 a2, A3 a3 )
{GUCEF_TRACE;

    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );

    CICloneable* taskData = CDeferredTask::Construct( f, lastTask, a2, a3 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

// QueueMemberCallback — raw pointer implementations

template< typename R, typename C >
CASync&
CASync::QueueMemberCallback( C* obj, R (C::*f)() )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::ConstructMember( obj, f );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return QueueCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename C, typename A1 >
CASyncRawMemberChainStep< deferred_member_invoker< C, R (C::*)(A1), arity_1 >, arity_1 >
CASync::QueueMemberCallback( C* obj, R (C::*f)(A1), A1 a1 )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::ConstructMember( obj, f, a1 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return QueueCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename C, typename A1, typename A2 >
CASyncRawMemberChainStep< deferred_member_invoker< C, R (C::*)(A1, A2), arity_2 >, arity_2 >
CASync::QueueMemberCallback( C* obj, R (C::*f)(A1, A2), A1 a1, A2 a2 )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::ConstructMember( obj, f, a1, a2 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return QueueCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename C, typename A1, typename A2, typename A3 >
CASyncRawMemberChainStep< deferred_member_invoker< C, R (C::*)(A1, A2, A3), arity_3 >, arity_3 >
CASync::QueueMemberCallback( C* obj, R (C::*f)(A1, A2, A3), A1 a1, A2 a2, A3 a3 )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::ConstructMember( obj, f, a1, a2, a3 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return QueueCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename C, typename A1, typename A2, typename A3, typename A4 >
CASyncRawMemberChainStep< deferred_member_invoker< C, R (C::*)(A1, A2, A3, A4), arity_4 >, arity_4 >
CASync::QueueMemberCallback( C* obj, R (C::*f)(A1, A2, A3, A4), A1 a1, A2 a2, A3 a3, A4 a4 )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::ConstructMember( obj, f, a1, a2, a3, a4 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return QueueCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

// QueueMemberCallback — shared pointer implementations

template< typename R, typename SharedPtrT >
CASync&
CASync::QueueMemberCallback( SharedPtrT obj, R (SharedPtrT::TContainedType::*f)() )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::ConstructMemberShared( obj, f );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return QueueCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename SharedPtrT, typename A1 >
CASyncSharedMemberChainStep< deferred_member_shared_invoker< SharedPtrT, R (SharedPtrT::TContainedType::*)(A1), arity_1 >, arity_1 >
CASync::QueueMemberCallback( SharedPtrT obj, R (SharedPtrT::TContainedType::*f)(A1), A1 a1 )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::ConstructMemberShared( obj, f, a1 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return QueueCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename SharedPtrT, typename A1, typename A2 >
CASyncSharedMemberChainStep< deferred_member_shared_invoker< SharedPtrT, R (SharedPtrT::TContainedType::*)(A1, A2), arity_2 >, arity_2 >
CASync::QueueMemberCallback( SharedPtrT obj, R (SharedPtrT::TContainedType::*f)(A1, A2), A1 a1, A2 a2 )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::ConstructMemberShared( obj, f, a1, a2 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return QueueCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename SharedPtrT, typename A1, typename A2, typename A3 >
CASyncSharedMemberChainStep< deferred_member_shared_invoker< SharedPtrT, R (SharedPtrT::TContainedType::*)(A1, A2, A3), arity_3 >, arity_3 >
CASync::QueueMemberCallback( SharedPtrT obj, R (SharedPtrT::TContainedType::*f)(A1, A2, A3), A1 a1, A2 a2, A3 a3 )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::ConstructMemberShared( obj, f, a1, a2, a3 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return QueueCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename SharedPtrT, typename A1, typename A2, typename A3, typename A4 >
CASyncSharedMemberChainStep< deferred_member_shared_invoker< SharedPtrT, R (SharedPtrT::TContainedType::*)(A1, A2, A3, A4), arity_4 >, arity_4 >
CASync::QueueMemberCallback( SharedPtrT obj, R (SharedPtrT::TContainedType::*f)(A1, A2, A3, A4), A1 a1, A2 a2, A3 a3, A4 a4 )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::ConstructMemberShared( obj, f, a1, a2, a3, a4 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return QueueCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

// ThenMemberCallback — raw pointer implementations

template< typename R, typename C >
CASyncRawMemberChainStep< deferred_member_invoker< C, R (C::*)(CTaskPtr), arity_1 >, arity_1 >
CASync::ThenMemberCallback( C* obj, R (C::*f)(CTaskPtr) )
{GUCEF_TRACE;

    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );

    CICloneable* taskData = CDeferredTask::ConstructMember( obj, f, lastTask );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename C, typename A2 >
CASyncRawMemberChainStep< deferred_member_invoker< C, R (C::*)(CTaskPtr, A2), arity_2 >, arity_2 >
CASync::ThenMemberCallback( C* obj, R (C::*f)(CTaskPtr, A2), A2 a2 )
{GUCEF_TRACE;

    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );

    CICloneable* taskData = CDeferredTask::ConstructMember( obj, f, lastTask, a2 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename C, typename A2, typename A3 >
CASyncRawMemberChainStep< deferred_member_invoker< C, R (C::*)(CTaskPtr, A2, A3), arity_3 >, arity_3 >
CASync::ThenMemberCallback( C* obj, R (C::*f)(CTaskPtr, A2, A3), A2 a2, A3 a3 )
{GUCEF_TRACE;

    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );

    CICloneable* taskData = CDeferredTask::ConstructMember( obj, f, lastTask, a2, a3 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

// ThenMemberCallback — shared pointer implementations

template< typename R, typename SharedPtrT >
CASyncSharedMemberChainStep< deferred_member_shared_invoker< SharedPtrT, R (SharedPtrT::TContainedType::*)(CTaskPtr), arity_1 >, arity_1 >
CASync::ThenMemberCallback( SharedPtrT obj, R (SharedPtrT::TContainedType::*f)(CTaskPtr) )
{GUCEF_TRACE;

    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );

    CICloneable* taskData = CDeferredTask::ConstructMemberShared( obj, f, lastTask );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename SharedPtrT, typename A2 >
CASyncSharedMemberChainStep< deferred_member_shared_invoker< SharedPtrT, R (SharedPtrT::TContainedType::*)(CTaskPtr, A2), arity_2 >, arity_2 >
CASync::ThenMemberCallback( SharedPtrT obj, R (SharedPtrT::TContainedType::*f)(CTaskPtr, A2), A2 a2 )
{GUCEF_TRACE;

    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );

    CICloneable* taskData = CDeferredTask::ConstructMemberShared( obj, f, lastTask, a2 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename SharedPtrT, typename A2, typename A3 >
CASyncSharedMemberChainStep< deferred_member_shared_invoker< SharedPtrT, R (SharedPtrT::TContainedType::*)(CTaskPtr, A2, A3), arity_3 >, arity_3 >
CASync::ThenMemberCallback( SharedPtrT obj, R (SharedPtrT::TContainedType::*f)(CTaskPtr, A2, A3), A2 a2, A3 a3 )
{GUCEF_TRACE;

    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );

    CICloneable* taskData = CDeferredTask::ConstructMemberShared( obj, f, lastTask, a2, a3 );
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
    CICloneable* taskData = CDeferredTask::ResultFwdConstruct( lastTask, priorInvoker, f );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2 >
CASyncChainStepArity2< R, A1, A2, plain_true >::CASyncChainStepArity2( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2 >
CASyncChainStepArity2< R, CTaskPtr, A2 >
CASyncChainStepArity2< R, A1, A2, plain_true >::ThenPassToCallback( R (*f)( CTaskPtr, R, A2 ) )
{GUCEF_TRACE;

    typedef R (*TGivenCallBackFunc)( CTaskPtr, R, A2 );
    typedef R (*TPriorGivenCallBackFunc)( A1, A2 );
    typedef deferred_invoker< TPriorGivenCallBackFunc, arity_2 >      TPriorDeferredInvoker;

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

template< typename R, typename A1, typename A2 >
CASyncChainStepArity2< R, A1, A2, plain_false >::CASyncChainStepArity2( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2 >
CASyncChainStepArity3< R, CTaskPtr, A1, A2 >
CASyncChainStepArity2< R, A1, A2, plain_false >::ThenPassToCallback( R (*f)( CTaskPtr, R, A1, A2 ) )
{GUCEF_TRACE;

    typedef R (*TGivenCallBackFunc)( CTaskPtr, R, A1, A2 );
    typedef R (*TPriorGivenCallBackFunc)( A1, A2 );
    typedef deferred_invoker< TPriorGivenCallBackFunc, arity_2 >      TPriorDeferredInvoker;

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

template< typename R, typename A1, typename A2, typename A3 >
CASyncChainStepArity3< R, A1, A2, A3, plain_true >::CASyncChainStepArity3( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3 >
CASyncChainStepArity3< R, CTaskPtr, A2, A3 >
CASyncChainStepArity3< R, A1, A2, A3, plain_true >::ThenPassToCallback( R (*f)( CTaskPtr, R, A2, A3 ) )
{GUCEF_TRACE;

    typedef R (*TGivenCallBackFunc)( CTaskPtr, R, A2, A3 );
    typedef R (*TPriorGivenCallBackFunc)( A1, A2, A3 );
    typedef deferred_invoker< TPriorGivenCallBackFunc, arity_3 >      TPriorDeferredInvoker;

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

template< typename R, typename A1, typename A2, typename A3 >
CASyncChainStepArity3< R, A1, A2, A3, plain_false >::CASyncChainStepArity3( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3 >
CASyncChainStepArity4< R, CTaskPtr, A1, A2, A3 >
CASyncChainStepArity3< R, A1, A2, A3, plain_false >::ThenPassToCallback( R (*f)( CTaskPtr, R, A1, A2, A3 ) )
{GUCEF_TRACE;

    typedef R (*TGivenCallBackFunc)( CTaskPtr, R, A1, A2, A3 );
    typedef R (*TPriorGivenCallBackFunc)( A1, A2, A3 );
    typedef deferred_invoker< TPriorGivenCallBackFunc, arity_3 >      TPriorDeferredInvoker;

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

/*-------------------------------------------------------------------------*/

// CASyncRawMemberChainStep — constructor implementations

template< typename PriorInvoker >
CASyncRawMemberChainStep< PriorInvoker, arity_0 >::CASyncRawMemberChainStep( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename PriorInvoker >
CASyncRawMemberChainStep< PriorInvoker, arity_1 >::CASyncRawMemberChainStep( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename PriorInvoker >
CASyncRawMemberChainStep< PriorInvoker, arity_2 >::CASyncRawMemberChainStep( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename PriorInvoker >
CASyncRawMemberChainStep< PriorInvoker, arity_3 >::CASyncRawMemberChainStep( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename PriorInvoker >
CASyncRawMemberChainStep< PriorInvoker, arity_4 >::CASyncRawMemberChainStep( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

// CASyncRawMemberChainStep — ThenPassToMemberCallback implementations

template< typename PriorInvoker >
template< typename R2, typename C2 >
CASyncRawMemberChainStep<
    deferred_member_fwd_invoker< CTaskPtr, C2, R2 (C2::*)(CTaskPtr, typename PriorInvoker::TResult), PriorInvoker, arity_0 >,
    arity_0 >
CASyncRawMemberChainStep< PriorInvoker, arity_0 >::ThenPassToMemberCallback( C2* obj, R2 (C2::*f)(CTaskPtr, TResult) )
{GUCEF_TRACE;

    typedef R2 (C2::*MF2)(CTaskPtr, TResult);
    typedef deferred_member_fwd_invoker< CTaskPtr, C2, MF2, PriorInvoker, arity_0 > NewInvoker;

    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
    CICloneable* opaqueTaskData = lastTask->GetTaskData();
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == opaqueTaskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    PriorInvoker* priorInvoker = static_cast< PriorInvoker* >( opaqueTaskData );

    CICloneable* taskData = GUCEF_NEW NewInvoker( obj, f, lastTask, priorInvoker );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename PriorInvoker >
template< typename R2, typename C2 >
CASyncRawMemberChainStep<
    deferred_member_fwd_invoker< CTaskPtr, C2, R2 (C2::*)(CTaskPtr, typename PriorInvoker::TResult, typename PriorInvoker::TArg1), PriorInvoker, arity_1 >,
    arity_0 >
CASyncRawMemberChainStep< PriorInvoker, arity_1 >::ThenPassToMemberCallback( C2* obj, R2 (C2::*f)(CTaskPtr, TResult, TArg1) )
{GUCEF_TRACE;

    typedef R2 (C2::*MF2)(CTaskPtr, TResult, TArg1);
    typedef deferred_member_fwd_invoker< CTaskPtr, C2, MF2, PriorInvoker, arity_1 > NewInvoker;

    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
    CICloneable* opaqueTaskData = lastTask->GetTaskData();
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == opaqueTaskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    PriorInvoker* priorInvoker = static_cast< PriorInvoker* >( opaqueTaskData );

    CICloneable* taskData = GUCEF_NEW NewInvoker( obj, f, lastTask, priorInvoker );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename PriorInvoker >
template< typename R2, typename C2 >
CASyncRawMemberChainStep<
    deferred_member_fwd_invoker< CTaskPtr, C2, R2 (C2::*)(CTaskPtr, typename PriorInvoker::TResult, typename PriorInvoker::TArg1, typename PriorInvoker::TArg2), PriorInvoker, arity_2 >,
    arity_0 >
CASyncRawMemberChainStep< PriorInvoker, arity_2 >::ThenPassToMemberCallback( C2* obj, R2 (C2::*f)(CTaskPtr, TResult, TArg1, TArg2) )
{GUCEF_TRACE;

    typedef R2 (C2::*MF2)(CTaskPtr, TResult, TArg1, TArg2);
    typedef deferred_member_fwd_invoker< CTaskPtr, C2, MF2, PriorInvoker, arity_2 > NewInvoker;

    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
    CICloneable* opaqueTaskData = lastTask->GetTaskData();
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == opaqueTaskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    PriorInvoker* priorInvoker = static_cast< PriorInvoker* >( opaqueTaskData );

    CICloneable* taskData = GUCEF_NEW NewInvoker( obj, f, lastTask, priorInvoker );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename PriorInvoker >
template< typename R2, typename C2 >
CASyncRawMemberChainStep<
    deferred_member_fwd_invoker< CTaskPtr, C2, R2 (C2::*)(CTaskPtr, typename PriorInvoker::TResult, typename PriorInvoker::TArg1, typename PriorInvoker::TArg2, typename PriorInvoker::TArg3), PriorInvoker, arity_3 >,
    arity_0 >
CASyncRawMemberChainStep< PriorInvoker, arity_3 >::ThenPassToMemberCallback( C2* obj, R2 (C2::*f)(CTaskPtr, TResult, TArg1, TArg2, TArg3) )
{GUCEF_TRACE;

    typedef R2 (C2::*MF2)(CTaskPtr, TResult, TArg1, TArg2, TArg3);
    typedef deferred_member_fwd_invoker< CTaskPtr, C2, MF2, PriorInvoker, arity_3 > NewInvoker;

    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
    CICloneable* opaqueTaskData = lastTask->GetTaskData();
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == opaqueTaskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    PriorInvoker* priorInvoker = static_cast< PriorInvoker* >( opaqueTaskData );

    CICloneable* taskData = GUCEF_NEW NewInvoker( obj, f, lastTask, priorInvoker );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename PriorInvoker >
template< typename R2, typename C2 >
CASyncRawMemberChainStep<
    deferred_member_fwd_invoker< CTaskPtr, C2, R2 (C2::*)(CTaskPtr, typename PriorInvoker::TResult, typename PriorInvoker::TArg1, typename PriorInvoker::TArg2, typename PriorInvoker::TArg3, typename PriorInvoker::TArg4), PriorInvoker, arity_4 >,
    arity_0 >
CASyncRawMemberChainStep< PriorInvoker, arity_4 >::ThenPassToMemberCallback( C2* obj, R2 (C2::*f)(CTaskPtr, TResult, TArg1, TArg2, TArg3, TArg4) )
{GUCEF_TRACE;

    typedef R2 (C2::*MF2)(CTaskPtr, TResult, TArg1, TArg2, TArg3, TArg4);
    typedef deferred_member_fwd_invoker< CTaskPtr, C2, MF2, PriorInvoker, arity_4 > NewInvoker;

    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
    CICloneable* opaqueTaskData = lastTask->GetTaskData();
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == opaqueTaskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    PriorInvoker* priorInvoker = static_cast< PriorInvoker* >( opaqueTaskData );

    CICloneable* taskData = GUCEF_NEW NewInvoker( obj, f, lastTask, priorInvoker );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

// CASyncSharedMemberChainStep — constructor implementations

template< typename SharedPriorInvoker >
CASyncSharedMemberChainStep< SharedPriorInvoker, arity_0 >::CASyncSharedMemberChainStep( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename SharedPriorInvoker >
CASyncSharedMemberChainStep< SharedPriorInvoker, arity_1 >::CASyncSharedMemberChainStep( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename SharedPriorInvoker >
CASyncSharedMemberChainStep< SharedPriorInvoker, arity_2 >::CASyncSharedMemberChainStep( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename SharedPriorInvoker >
CASyncSharedMemberChainStep< SharedPriorInvoker, arity_3 >::CASyncSharedMemberChainStep( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename SharedPriorInvoker >
CASyncSharedMemberChainStep< SharedPriorInvoker, arity_4 >::CASyncSharedMemberChainStep( CASync& thisChain )
    : CASync( thisChain )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

// CASyncSharedMemberChainStep — ThenPassToMemberCallback implementations

template< typename SharedPriorInvoker >
template< typename R2, typename SharedPtrT2 >
CASyncSharedMemberChainStep<
    deferred_member_shared_fwd_invoker< CTaskPtr, SharedPtrT2, R2 (SharedPtrT2::TContainedType::*)(CTaskPtr, typename SharedPriorInvoker::TResult), SharedPriorInvoker, arity_0 >,
    arity_0 >
CASyncSharedMemberChainStep< SharedPriorInvoker, arity_0 >::ThenPassToMemberCallback( SharedPtrT2 obj, R2 (SharedPtrT2::TContainedType::*f)(CTaskPtr, TResult) )
{GUCEF_TRACE;

    typedef R2 (SharedPtrT2::TContainedType::*MF2)(CTaskPtr, TResult);
    typedef deferred_member_shared_fwd_invoker< CTaskPtr, SharedPtrT2, MF2, SharedPriorInvoker, arity_0 > NewInvoker;

    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
    CICloneable* opaqueTaskData = lastTask->GetTaskData();
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == opaqueTaskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    SharedPriorInvoker* priorInvoker = static_cast< SharedPriorInvoker* >( opaqueTaskData );

    CICloneable* taskData = GUCEF_NEW NewInvoker( obj, f, lastTask, priorInvoker );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename SharedPriorInvoker >
template< typename R2, typename SharedPtrT2 >
CASyncSharedMemberChainStep<
    deferred_member_shared_fwd_invoker< CTaskPtr, SharedPtrT2, R2 (SharedPtrT2::TContainedType::*)(CTaskPtr, typename SharedPriorInvoker::TResult, typename SharedPriorInvoker::TArg1), SharedPriorInvoker, arity_1 >,
    arity_0 >
CASyncSharedMemberChainStep< SharedPriorInvoker, arity_1 >::ThenPassToMemberCallback( SharedPtrT2 obj, R2 (SharedPtrT2::TContainedType::*f)(CTaskPtr, TResult, TArg1) )
{GUCEF_TRACE;

    typedef R2 (SharedPtrT2::TContainedType::*MF2)(CTaskPtr, TResult, TArg1);
    typedef deferred_member_shared_fwd_invoker< CTaskPtr, SharedPtrT2, MF2, SharedPriorInvoker, arity_1 > NewInvoker;

    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
    CICloneable* opaqueTaskData = lastTask->GetTaskData();
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == opaqueTaskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    SharedPriorInvoker* priorInvoker = static_cast< SharedPriorInvoker* >( opaqueTaskData );

    CICloneable* taskData = GUCEF_NEW NewInvoker( obj, f, lastTask, priorInvoker );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename SharedPriorInvoker >
template< typename R2, typename SharedPtrT2 >
CASyncSharedMemberChainStep<
    deferred_member_shared_fwd_invoker< CTaskPtr, SharedPtrT2, R2 (SharedPtrT2::TContainedType::*)(CTaskPtr, typename SharedPriorInvoker::TResult, typename SharedPriorInvoker::TArg1, typename SharedPriorInvoker::TArg2), SharedPriorInvoker, arity_2 >,
    arity_0 >
CASyncSharedMemberChainStep< SharedPriorInvoker, arity_2 >::ThenPassToMemberCallback( SharedPtrT2 obj, R2 (SharedPtrT2::TContainedType::*f)(CTaskPtr, TResult, TArg1, TArg2) )
{GUCEF_TRACE;

    typedef R2 (SharedPtrT2::TContainedType::*MF2)(CTaskPtr, TResult, TArg1, TArg2);
    typedef deferred_member_shared_fwd_invoker< CTaskPtr, SharedPtrT2, MF2, SharedPriorInvoker, arity_2 > NewInvoker;

    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
    CICloneable* opaqueTaskData = lastTask->GetTaskData();
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == opaqueTaskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    SharedPriorInvoker* priorInvoker = static_cast< SharedPriorInvoker* >( opaqueTaskData );

    CICloneable* taskData = GUCEF_NEW NewInvoker( obj, f, lastTask, priorInvoker );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename SharedPriorInvoker >
template< typename R2, typename SharedPtrT2 >
CASyncSharedMemberChainStep<
    deferred_member_shared_fwd_invoker< CTaskPtr, SharedPtrT2, R2 (SharedPtrT2::TContainedType::*)(CTaskPtr, typename SharedPriorInvoker::TResult, typename SharedPriorInvoker::TArg1, typename SharedPriorInvoker::TArg2, typename SharedPriorInvoker::TArg3), SharedPriorInvoker, arity_3 >,
    arity_0 >
CASyncSharedMemberChainStep< SharedPriorInvoker, arity_3 >::ThenPassToMemberCallback( SharedPtrT2 obj, R2 (SharedPtrT2::TContainedType::*f)(CTaskPtr, TResult, TArg1, TArg2, TArg3) )
{GUCEF_TRACE;

    typedef R2 (SharedPtrT2::TContainedType::*MF2)(CTaskPtr, TResult, TArg1, TArg2, TArg3);
    typedef deferred_member_shared_fwd_invoker< CTaskPtr, SharedPtrT2, MF2, SharedPriorInvoker, arity_3 > NewInvoker;

    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
    CICloneable* opaqueTaskData = lastTask->GetTaskData();
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == opaqueTaskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    SharedPriorInvoker* priorInvoker = static_cast< SharedPriorInvoker* >( opaqueTaskData );

    CICloneable* taskData = GUCEF_NEW NewInvoker( obj, f, lastTask, priorInvoker );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );

    return ThenCallbackCommonImpl( taskData );
}

/*-------------------------------------------------------------------------*/

template< typename SharedPriorInvoker >
template< typename R2, typename SharedPtrT2 >
CASyncSharedMemberChainStep<
    deferred_member_shared_fwd_invoker< CTaskPtr, SharedPtrT2, R2 (SharedPtrT2::TContainedType::*)(CTaskPtr, typename SharedPriorInvoker::TResult, typename SharedPriorInvoker::TArg1, typename SharedPriorInvoker::TArg2, typename SharedPriorInvoker::TArg3, typename SharedPriorInvoker::TArg4), SharedPriorInvoker, arity_4 >,
    arity_0 >
CASyncSharedMemberChainStep< SharedPriorInvoker, arity_4 >::ThenPassToMemberCallback( SharedPtrT2 obj, R2 (SharedPtrT2::TContainedType::*f)(CTaskPtr, TResult, TArg1, TArg2, TArg3, TArg4) )
{GUCEF_TRACE;

    typedef R2 (SharedPtrT2::TContainedType::*MF2)(CTaskPtr, TResult, TArg1, TArg2, TArg3, TArg4);
    typedef deferred_member_shared_fwd_invoker< CTaskPtr, SharedPtrT2, MF2, SharedPriorInvoker, arity_4 > NewInvoker;

    CTaskPtr lastTask = GetLastTask();
    if GUCEF_PREDICT_FALSE( lastTask.IsNULL() )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASK_CHAINING_FAILED );
    CICloneable* opaqueTaskData = lastTask->GetTaskData();
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == opaqueTaskData )
        return SetLastTaskStatus( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    SharedPriorInvoker* priorInvoker = static_cast< SharedPriorInvoker* >( opaqueTaskData );

    CICloneable* taskData = GUCEF_NEW NewInvoker( obj, f, lastTask, priorInvoker );
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
