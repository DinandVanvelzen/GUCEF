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

#ifndef GUCEF_CORE_CDEFERREDTASK_H
#define GUCEF_CORE_CDEFERREDTASK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

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
 *  C++98 compatible implementation of deferred function calls with automatic
 *  function argument type retention
 *
 *  @TODO: To be renamed to CDeferredTaskData
 *         Its the user supplied data for a task not the task itself
 */
class GUCEF_CORE_PUBLIC_CPP CDeferredTask : public CICloneable
{
    public:

    virtual void Invoke( void ) = 0;

    virtual ~CDeferredTask() GUCEF_VIRTUAL_OVERRIDE {GUCEF_TRACE; }

    template< typename R >
    static CDeferredTask* Construct( R (*f)() );

    template< typename R, typename A1 >
    static CDeferredTask* Construct( R (*f)(A1), A1 a1 );

    template< typename R, typename A1, typename A2 >
    static CDeferredTask* Construct( R (*f)(A1, A2), A1 a1, A2 a2 );

    template< typename R, typename A1, typename A2, typename A3 >
    static CDeferredTask* Construct( R (*f)(A1, A2, A3), A1 a1, A2 a2, A3 a3 );

    template< typename R, typename A1, typename A2, typename A3, typename A4 >
    static CDeferredTask* Construct( R (*f)(A1, A2, A3, A4), A1 a1, A2 a2, A3 a3, A4 a4 );

    /**
     *  Constructs task data for a deferred member function call via raw object pointer.
     *  Caller is responsible for ensuring the object remains alive until invocation.
     *  Arity 0–4 variants match the existing free-function Construct() coverage.
     */
    template< typename C, typename MF >
    static CDeferredTask* ConstructMember( C* obj, MF f );

    template< typename C, typename MF, typename A1 >
    static CDeferredTask* ConstructMember( C* obj, MF f, A1 a1 );

    template< typename C, typename MF, typename A1, typename A2 >
    static CDeferredTask* ConstructMember( C* obj, MF f, A1 a1, A2 a2 );

    template< typename C, typename MF, typename A1, typename A2, typename A3 >
    static CDeferredTask* ConstructMember( C* obj, MF f, A1 a1, A2 a2, A3 a3 );

    template< typename C, typename MF, typename A1, typename A2, typename A3, typename A4 >
    static CDeferredTask* ConstructMember( C* obj, MF f, A1 a1, A2 a2, A3 a3, A4 a4 );

    /**
     *  Constructs task data for a deferred member function call via shared pointer.
     *  The shared pointer is stored by value inside the task object, keeping the target
     *  object alive for the entire duration of the deferred invocation.
     *  Works with any type that exposes TContainedType and GetPointerAlways()
     *  (e.g. CTBasicSharedPtr, CTSharedPtr).
     */
    template< typename SharedPtrT, typename MF >
    static CDeferredTask* ConstructMemberShared( SharedPtrT obj, MF f );

    template< typename SharedPtrT, typename MF, typename A1 >
    static CDeferredTask* ConstructMemberShared( SharedPtrT obj, MF f, A1 a1 );

    template< typename SharedPtrT, typename MF, typename A1, typename A2 >
    static CDeferredTask* ConstructMemberShared( SharedPtrT obj, MF f, A1 a1, A2 a2 );

    template< typename SharedPtrT, typename MF, typename A1, typename A2, typename A3 >
    static CDeferredTask* ConstructMemberShared( SharedPtrT obj, MF f, A1 a1, A2 a2, A3 a3 );

    template< typename SharedPtrT, typename MF, typename A1, typename A2, typename A3, typename A4 >
    static CDeferredTask* ConstructMemberShared( SharedPtrT obj, MF f, A1 a1, A2 a2, A3 a3, A4 a4 );

    /**
     *  Constructs an object to hold with all the information needed to forward the results
     *  of a prior callback invocation to the next callback. The main state holder object 'StateLifeCycle' is
     *  forwarded to ensure overall lifecycle management plus other miscellaneous state.
     *
     *  We also have P which is the prior invoker used
     *  Beyond that the result R and params for the prior callback are all passed to the next
     */
    template< typename StateLifeCycle, typename P, typename R >
    static CDeferredTask* ResultFwdConstruct( StateLifeCycle stateLife, P* prior, R (*f)( StateLifeCycle, R ) );

    template< typename StateLifeCycle, typename P, typename R, typename A1 >
    static CDeferredTask* ResultFwdConstruct( StateLifeCycle stateLife, P* prior, R (*f)( StateLifeCycle, R, A1 ) );

    template< typename StateLifeCycle, typename P, typename R, typename A1, typename A2 >
    static CDeferredTask* ResultFwdConstruct( StateLifeCycle stateLife, P* prior, R (*f)( StateLifeCycle, R, A1, A2 ) );

    template< typename StateLifeCycle, typename P, typename R, typename A1, typename A2, typename A3 >
    static CDeferredTask* ResultFwdConstruct( StateLifeCycle stateLife, P* prior, R (*f)( StateLifeCycle, R, A1, A2, A3 ) );

    template< typename StateLifeCycle, typename P, typename R, typename A1, typename A2, typename A3, typename A4 >
    static CDeferredTask* ResultFwdConstruct( StateLifeCycle stateLife, P* prior, R (*f)( StateLifeCycle, R, A1, A2, A3, A4 ) );

    template< typename StateLifeCycle, typename P, typename R, typename A1, typename A2, typename A3, typename A4, typename A5 >
    static CDeferredTask* ResultFwdConstruct( StateLifeCycle stateLife, P* prior, R (*f)( StateLifeCycle, R, A1, A2, A3, A4, A5 ) );
};

/*-------------------------------------------------------------------------*/

//— deferred_invoker specializations
template< typename FuncPtr, typename Tag > struct deferred_invoker;

// arity 0
template<typename FuncPtr>
class deferred_invoker<FuncPtr, arity_0> : public CDeferredTask
{
    public:

    typedef function_traits<typename remove_pointer<FuncPtr>::type> traits;
    FuncPtr m_f;
    typename traits::result_type m_result;

    deferred_invoker(FuncPtr f)
        : m_f(f), m_result() {GUCEF_TRACE; }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_result = (*m_f)();
    }

    virtual ~deferred_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW deferred_invoker(m_f); }
};

// arity 1
template< typename FuncPtr >
class deferred_invoker< FuncPtr, arity_1 > : public CDeferredTask
{
    public:

    typedef function_traits<typename remove_pointer<FuncPtr>::type> traits;
    FuncPtr m_f;
    typename traits::result_type m_result;
    typename traits::arg1_type m_a1;    

    deferred_invoker( FuncPtr f                     ,
                      typename traits::arg1_type a1 )
      : m_f(f)
      , m_result()
      , m_a1(a1)
    {GUCEF_TRACE;

    }

    virtual ~deferred_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_result = (*m_f)(m_a1);
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW deferred_invoker(m_f, m_a1); }
};

// arity 2
template< typename FuncPtr >
class deferred_invoker< FuncPtr, arity_2 > : public CDeferredTask
{
    public:

    typedef function_traits<typename remove_pointer<FuncPtr>::type> traits;
    FuncPtr m_f;
    typename traits::result_type m_result;
    typename traits::arg1_type m_a1;
    typename traits::arg2_type m_a2;

    deferred_invoker( FuncPtr f                     ,
                      typename traits::arg1_type a1 ,
                      typename traits::arg2_type a2 )
      : m_f( f )
      , m_result()
      , m_a1( a1 )
      , m_a2( a2 )
    {GUCEF_TRACE;

    }

    virtual ~deferred_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_result = (*m_f)(m_a1, m_a2);
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_invoker( m_f, m_a1, m_a2 );
    }
};

// arity 3
template< typename FuncPtr >
class deferred_invoker< FuncPtr, arity_3 > : public CDeferredTask
{
    public:

    typedef function_traits<typename remove_pointer<FuncPtr>::type> traits;
    FuncPtr m_f;
    typename traits::result_type m_result;
    typename traits::arg1_type m_a1;
    typename traits::arg2_type m_a2;
    typename traits::arg3_type m_a3;

    deferred_invoker( FuncPtr f                     ,
                      typename traits::arg1_type a1 ,
                      typename traits::arg2_type a2 ,
                      typename traits::arg3_type a3 )
      : m_f( f )
      , m_result()
      , m_a1( a1 )
      , m_a2( a2 )
      , m_a3( a3 )
    {GUCEF_TRACE;

    }

    virtual ~deferred_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_result = (*m_f)(m_a1, m_a2, m_a3);
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_invoker( m_f, m_a1, m_a2, m_a3 );
    }
};

// arity 4
template< typename FuncPtr >
class deferred_invoker< FuncPtr, arity_4 > : public CDeferredTask
{
    public:

    typedef function_traits<typename remove_pointer<FuncPtr>::type> traits;
    FuncPtr m_f;
    typename traits::result_type m_result;
    typename traits::arg1_type m_a1;
    typename traits::arg2_type m_a2;
    typename traits::arg3_type m_a3;
    typename traits::arg4_type m_a4;

    deferred_invoker( FuncPtr f                     ,
                      typename traits::arg1_type a1 ,
                      typename traits::arg2_type a2 ,
                      typename traits::arg3_type a3 ,
                      typename traits::arg4_type a4 )
      : m_f( f )
      , m_result()
      , m_a1( a1 )
      , m_a2( a2 )
      , m_a3( a3 )
      , m_a4( a4 )
    {GUCEF_TRACE;

    }

    virtual ~deferred_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_result = (*m_f)(m_a1, m_a2, m_a3, m_a4 );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_invoker( m_f, m_a1, m_a2, m_a3, m_a4 );
    }
};

// arity 5
template< typename FuncPtr >
class deferred_invoker< FuncPtr, arity_5 > : public CDeferredTask
{
    public:

    typedef function_traits<typename remove_pointer<FuncPtr>::type> traits;
    FuncPtr m_f;
    typename traits::result_type m_result;
    typename traits::arg1_type m_a1;
    typename traits::arg2_type m_a2;
    typename traits::arg3_type m_a3;
    typename traits::arg4_type m_a4;
    typename traits::arg5_type m_a5;

    deferred_invoker( FuncPtr f                     ,
                      typename traits::arg1_type a1 ,
                      typename traits::arg2_type a2 ,
                      typename traits::arg3_type a3 ,
                      typename traits::arg4_type a4 ,
                      typename traits::arg5_type a5 )
      : m_f( f )
      , m_result()
      , m_a1( a1 )
      , m_a2( a2 )
      , m_a3( a3 )
      , m_a4( a4 )
      , m_a5( a5 )
    {GUCEF_TRACE;

    }

    virtual ~deferred_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_result = (*m_f)(m_a1, m_a2, m_a3, m_a4, m_a5 );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_invoker( m_f, m_a1, m_a2, m_a3, m_a4, m_a5 );
    }
};

/*-------------------------------------------------------------------------*/

//— deferred_member_invoker: raw object pointer + member function pointer, arity 0–5

template< typename C, typename MF, typename ArityTag > struct deferred_member_invoker;

// arity 0
template< typename C, typename MF >
class deferred_member_invoker< C, MF, arity_0 > : public CDeferredTask
{
    public:

    typedef GUCEF::member_function_traits< MF >     traits;
    typedef typename traits::result_type             TResult;

    C*     m_obj;
    MF     m_f;
    TResult m_result;

    deferred_member_invoker( C* obj, MF f )
        : m_obj( obj )
        , m_f( f )
        , m_result()
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_obj )
            m_result = (m_obj->*m_f)();
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_invoker( m_obj, m_f );
    }
};

// arity 1
template< typename C, typename MF >
class deferred_member_invoker< C, MF, arity_1 > : public CDeferredTask
{
    public:

    typedef GUCEF::member_function_traits< MF >     traits;
    typedef typename traits::result_type             TResult;
    typedef typename traits::arg1_type               TArg1;

    C*     m_obj;
    MF     m_f;
    TResult m_result;
    TArg1  m_a1;

    deferred_member_invoker( C* obj, MF f, TArg1 a1 )
        : m_obj( obj )
        , m_f( f )
        , m_result()
        , m_a1( a1 )
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_obj )
            m_result = (m_obj->*m_f)( m_a1 );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_invoker( m_obj, m_f, m_a1 );
    }
};

// arity 2
template< typename C, typename MF >
class deferred_member_invoker< C, MF, arity_2 > : public CDeferredTask
{
    public:

    typedef GUCEF::member_function_traits< MF >     traits;
    typedef typename traits::result_type             TResult;
    typedef typename traits::arg1_type               TArg1;
    typedef typename traits::arg2_type               TArg2;

    C*     m_obj;
    MF     m_f;
    TResult m_result;
    TArg1  m_a1;
    TArg2  m_a2;

    deferred_member_invoker( C* obj, MF f, TArg1 a1, TArg2 a2 )
        : m_obj( obj )
        , m_f( f )
        , m_result()
        , m_a1( a1 )
        , m_a2( a2 )
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_obj )
            m_result = (m_obj->*m_f)( m_a1, m_a2 );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_invoker( m_obj, m_f, m_a1, m_a2 );
    }
};

// arity 3
template< typename C, typename MF >
class deferred_member_invoker< C, MF, arity_3 > : public CDeferredTask
{
    public:

    typedef GUCEF::member_function_traits< MF >     traits;
    typedef typename traits::result_type             TResult;
    typedef typename traits::arg1_type               TArg1;
    typedef typename traits::arg2_type               TArg2;
    typedef typename traits::arg3_type               TArg3;

    C*     m_obj;
    MF     m_f;
    TResult m_result;
    TArg1  m_a1;
    TArg2  m_a2;
    TArg3  m_a3;

    deferred_member_invoker( C* obj, MF f, TArg1 a1, TArg2 a2, TArg3 a3 )
        : m_obj( obj )
        , m_f( f )
        , m_result()
        , m_a1( a1 )
        , m_a2( a2 )
        , m_a3( a3 )
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_obj )
            m_result = (m_obj->*m_f)( m_a1, m_a2, m_a3 );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_invoker( m_obj, m_f, m_a1, m_a2, m_a3 );
    }
};

// arity 4
template< typename C, typename MF >
class deferred_member_invoker< C, MF, arity_4 > : public CDeferredTask
{
    public:

    typedef GUCEF::member_function_traits< MF >     traits;
    typedef typename traits::result_type             TResult;
    typedef typename traits::arg1_type               TArg1;
    typedef typename traits::arg2_type               TArg2;
    typedef typename traits::arg3_type               TArg3;
    typedef typename traits::arg4_type               TArg4;

    C*     m_obj;
    MF     m_f;
    TResult m_result;
    TArg1  m_a1;
    TArg2  m_a2;
    TArg3  m_a3;
    TArg4  m_a4;

    deferred_member_invoker( C* obj, MF f, TArg1 a1, TArg2 a2, TArg3 a3, TArg4 a4 )
        : m_obj( obj )
        , m_f( f )
        , m_result()
        , m_a1( a1 )
        , m_a2( a2 )
        , m_a3( a3 )
        , m_a4( a4 )
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_obj )
            m_result = (m_obj->*m_f)( m_a1, m_a2, m_a3, m_a4 );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_invoker( m_obj, m_f, m_a1, m_a2, m_a3, m_a4 );
    }
};

// arity 5
template< typename C, typename MF >
class deferred_member_invoker< C, MF, arity_5 > : public CDeferredTask
{
    public:

    typedef GUCEF::member_function_traits< MF >     traits;
    typedef typename traits::result_type             TResult;
    typedef typename traits::arg1_type               TArg1;
    typedef typename traits::arg2_type               TArg2;
    typedef typename traits::arg3_type               TArg3;
    typedef typename traits::arg4_type               TArg4;
    typedef typename traits::arg5_type               TArg5;

    C*     m_obj;
    MF     m_f;
    TResult m_result;
    TArg1  m_a1;
    TArg2  m_a2;
    TArg3  m_a3;
    TArg4  m_a4;
    TArg5  m_a5;

    deferred_member_invoker( C* obj, MF f, TArg1 a1, TArg2 a2, TArg3 a3, TArg4 a4, TArg5 a5 )
        : m_obj( obj )
        , m_f( f )
        , m_result()
        , m_a1( a1 )
        , m_a2( a2 )
        , m_a3( a3 )
        , m_a4( a4 )
        , m_a5( a5 )
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_obj )
            m_result = (m_obj->*m_f)( m_a1, m_a2, m_a3, m_a4, m_a5 );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_invoker( m_obj, m_f, m_a1, m_a2, m_a3, m_a4, m_a5 );
    }
};

/*-------------------------------------------------------------------------*/

//— deferred_member_shared_invoker: shared pointer + member function pointer, arity 0–5
//  The shared pointer is stored by value, keeping the target object alive for the
//  duration of the deferred invocation. Works with CTBasicSharedPtr, CTSharedPtr, etc.

template< typename SharedPtrT, typename MF, typename ArityTag > struct deferred_member_shared_invoker;

// arity 0
template< typename SharedPtrT, typename MF >
class deferred_member_shared_invoker< SharedPtrT, MF, arity_0 > : public CDeferredTask
{
    public:

    typedef typename SharedPtrT::TContainedType      C;
    typedef GUCEF::member_function_traits< MF >      traits;
    typedef typename traits::result_type              TResult;

    SharedPtrT m_obj;    // strong reference — keeps target alive
    MF         m_f;
    TResult    m_result;

    deferred_member_shared_invoker( SharedPtrT obj, MF f )
        : m_obj( obj )
        , m_f( f )
        , m_result()
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_shared_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        C* ptr = m_obj.GetPointerAlways();
        if ( GUCEF_NULL != ptr )
            m_result = (ptr->*m_f)();
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_shared_invoker( m_obj, m_f );
    }
};

// arity 1
template< typename SharedPtrT, typename MF >
class deferred_member_shared_invoker< SharedPtrT, MF, arity_1 > : public CDeferredTask
{
    public:

    typedef typename SharedPtrT::TContainedType      C;
    typedef GUCEF::member_function_traits< MF >      traits;
    typedef typename traits::result_type              TResult;
    typedef typename traits::arg1_type                TArg1;

    SharedPtrT m_obj;
    MF         m_f;
    TResult    m_result;
    TArg1      m_a1;

    deferred_member_shared_invoker( SharedPtrT obj, MF f, TArg1 a1 )
        : m_obj( obj )
        , m_f( f )
        , m_result()
        , m_a1( a1 )
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_shared_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        C* ptr = m_obj.GetPointerAlways();
        if ( GUCEF_NULL != ptr )
            m_result = (ptr->*m_f)( m_a1 );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_shared_invoker( m_obj, m_f, m_a1 );
    }
};

// arity 2
template< typename SharedPtrT, typename MF >
class deferred_member_shared_invoker< SharedPtrT, MF, arity_2 > : public CDeferredTask
{
    public:

    typedef typename SharedPtrT::TContainedType      C;
    typedef GUCEF::member_function_traits< MF >      traits;
    typedef typename traits::result_type              TResult;
    typedef typename traits::arg1_type                TArg1;
    typedef typename traits::arg2_type                TArg2;

    SharedPtrT m_obj;
    MF         m_f;
    TResult    m_result;
    TArg1      m_a1;
    TArg2      m_a2;

    deferred_member_shared_invoker( SharedPtrT obj, MF f, TArg1 a1, TArg2 a2 )
        : m_obj( obj )
        , m_f( f )
        , m_result()
        , m_a1( a1 )
        , m_a2( a2 )
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_shared_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        C* ptr = m_obj.GetPointerAlways();
        if ( GUCEF_NULL != ptr )
            m_result = (ptr->*m_f)( m_a1, m_a2 );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_shared_invoker( m_obj, m_f, m_a1, m_a2 );
    }
};

// arity 3
template< typename SharedPtrT, typename MF >
class deferred_member_shared_invoker< SharedPtrT, MF, arity_3 > : public CDeferredTask
{
    public:

    typedef typename SharedPtrT::TContainedType      C;
    typedef GUCEF::member_function_traits< MF >      traits;
    typedef typename traits::result_type              TResult;
    typedef typename traits::arg1_type                TArg1;
    typedef typename traits::arg2_type                TArg2;
    typedef typename traits::arg3_type                TArg3;

    SharedPtrT m_obj;
    MF         m_f;
    TResult    m_result;
    TArg1      m_a1;
    TArg2      m_a2;
    TArg3      m_a3;

    deferred_member_shared_invoker( SharedPtrT obj, MF f, TArg1 a1, TArg2 a2, TArg3 a3 )
        : m_obj( obj )
        , m_f( f )
        , m_result()
        , m_a1( a1 )
        , m_a2( a2 )
        , m_a3( a3 )
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_shared_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        C* ptr = m_obj.GetPointerAlways();
        if ( GUCEF_NULL != ptr )
            m_result = (ptr->*m_f)( m_a1, m_a2, m_a3 );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_shared_invoker( m_obj, m_f, m_a1, m_a2, m_a3 );
    }
};

// arity 4
template< typename SharedPtrT, typename MF >
class deferred_member_shared_invoker< SharedPtrT, MF, arity_4 > : public CDeferredTask
{
    public:

    typedef typename SharedPtrT::TContainedType      C;
    typedef GUCEF::member_function_traits< MF >      traits;
    typedef typename traits::result_type              TResult;
    typedef typename traits::arg1_type                TArg1;
    typedef typename traits::arg2_type                TArg2;
    typedef typename traits::arg3_type                TArg3;
    typedef typename traits::arg4_type                TArg4;

    SharedPtrT m_obj;
    MF         m_f;
    TResult    m_result;
    TArg1      m_a1;
    TArg2      m_a2;
    TArg3      m_a3;
    TArg4      m_a4;

    deferred_member_shared_invoker( SharedPtrT obj, MF f, TArg1 a1, TArg2 a2, TArg3 a3, TArg4 a4 )
        : m_obj( obj )
        , m_f( f )
        , m_result()
        , m_a1( a1 )
        , m_a2( a2 )
        , m_a3( a3 )
        , m_a4( a4 )
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_shared_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        C* ptr = m_obj.GetPointerAlways();
        if ( GUCEF_NULL != ptr )
            m_result = (ptr->*m_f)( m_a1, m_a2, m_a3, m_a4 );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_shared_invoker( m_obj, m_f, m_a1, m_a2, m_a3, m_a4 );
    }
};

// arity 5
template< typename SharedPtrT, typename MF >
class deferred_member_shared_invoker< SharedPtrT, MF, arity_5 > : public CDeferredTask
{
    public:

    typedef typename SharedPtrT::TContainedType      C;
    typedef GUCEF::member_function_traits< MF >      traits;
    typedef typename traits::result_type              TResult;
    typedef typename traits::arg1_type                TArg1;
    typedef typename traits::arg2_type                TArg2;
    typedef typename traits::arg3_type                TArg3;
    typedef typename traits::arg4_type                TArg4;
    typedef typename traits::arg5_type                TArg5;

    SharedPtrT m_obj;
    MF         m_f;
    TResult    m_result;
    TArg1      m_a1;
    TArg2      m_a2;
    TArg3      m_a3;
    TArg4      m_a4;
    TArg5      m_a5;

    deferred_member_shared_invoker( SharedPtrT obj, MF f, TArg1 a1, TArg2 a2, TArg3 a3, TArg4 a4, TArg5 a5 )
        : m_obj( obj )
        , m_f( f )
        , m_result()
        , m_a1( a1 )
        , m_a2( a2 )
        , m_a3( a3 )
        , m_a4( a4 )
        , m_a5( a5 )
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_shared_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        C* ptr = m_obj.GetPointerAlways();
        if ( GUCEF_NULL != ptr )
            m_result = (ptr->*m_f)( m_a1, m_a2, m_a3, m_a4, m_a5 );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_shared_invoker( m_obj, m_f, m_a1, m_a2, m_a3, m_a4, m_a5 );
    }
};

/*-------------------------------------------------------------------------*/

//— deferred_member_fwd_invoker: raw object pointer + member function, forwarding prior invoker's results
//  StateLifeCycle is the chain lifecycle token type (CTaskPtr in practice).
//  ArityTag encodes the number of prior user args forwarded (matches the prior invoker's arity).
//  Prior must expose public m_result and, depending on arity, m_a1/m_a2/etc.

template< typename StateLifeCycle, typename C, typename MF, typename Prior, typename ArityTag >
struct deferred_member_fwd_invoker;

// arity_0: prior had 0 user args; new MF takes (StateLifeCycle, Prior::TResult)
template< typename StateLifeCycle, typename C, typename MF, typename Prior >
class deferred_member_fwd_invoker< StateLifeCycle, C, MF, Prior, arity_0 > : public CDeferredTask
{
    public:

    typedef GUCEF::member_function_traits< MF >  traits;
    typedef typename traits::result_type          TResult;

    C*             m_obj;
    MF             m_f;
    StateLifeCycle m_stateLife;
    Prior*         m_prior;
    TResult        m_result;

    deferred_member_fwd_invoker( C* obj, MF f, StateLifeCycle stateLife, Prior* prior )
        : m_obj( obj )
        , m_f( f )
        , m_stateLife( stateLife )
        , m_prior( prior )
        , m_result()
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_fwd_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_obj )
            m_result = (m_obj->*m_f)( m_stateLife, m_prior->m_result );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_fwd_invoker( m_obj, m_f, m_stateLife, m_prior );
    }
};

// arity_1: prior had 1 user arg; new MF takes (StateLifeCycle, Prior::TResult, Prior::TArg1)
template< typename StateLifeCycle, typename C, typename MF, typename Prior >
class deferred_member_fwd_invoker< StateLifeCycle, C, MF, Prior, arity_1 > : public CDeferredTask
{
    public:

    typedef GUCEF::member_function_traits< MF >  traits;
    typedef typename traits::result_type          TResult;

    C*             m_obj;
    MF             m_f;
    StateLifeCycle m_stateLife;
    Prior*         m_prior;
    TResult        m_result;

    deferred_member_fwd_invoker( C* obj, MF f, StateLifeCycle stateLife, Prior* prior )
        : m_obj( obj )
        , m_f( f )
        , m_stateLife( stateLife )
        , m_prior( prior )
        , m_result()
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_fwd_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_obj )
            m_result = (m_obj->*m_f)( m_stateLife       ,
                                      m_prior->m_result ,
                                      m_prior->m_a1     );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_fwd_invoker( m_obj, m_f, m_stateLife, m_prior );
    }
};

// arity_2: prior had 2 user args; new MF takes (StateLifeCycle, Prior::TResult, Prior::TArg1, Prior::TArg2)
template< typename StateLifeCycle, typename C, typename MF, typename Prior >
class deferred_member_fwd_invoker< StateLifeCycle, C, MF, Prior, arity_2 > : public CDeferredTask
{
    public:

    typedef GUCEF::member_function_traits< MF >  traits;
    typedef typename traits::result_type          TResult;

    C*             m_obj;
    MF             m_f;
    StateLifeCycle m_stateLife;
    Prior*         m_prior;
    TResult        m_result;

    deferred_member_fwd_invoker( C* obj, MF f, StateLifeCycle stateLife, Prior* prior )
        : m_obj( obj )
        , m_f( f )
        , m_stateLife( stateLife )
        , m_prior( prior )
        , m_result()
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_fwd_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_obj )
            m_result = (m_obj->*m_f)( m_stateLife       ,
                                      m_prior->m_result ,
                                      m_prior->m_a1     ,
                                      m_prior->m_a2     );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_fwd_invoker( m_obj, m_f, m_stateLife, m_prior );
    }
};

// arity_3: prior had 3 user args; new MF takes (StateLifeCycle, Prior::TResult, Prior::TArg1, ..3)
template< typename StateLifeCycle, typename C, typename MF, typename Prior >
class deferred_member_fwd_invoker< StateLifeCycle, C, MF, Prior, arity_3 > : public CDeferredTask
{
    public:

    typedef GUCEF::member_function_traits< MF >  traits;
    typedef typename traits::result_type          TResult;

    C*             m_obj;
    MF             m_f;
    StateLifeCycle m_stateLife;
    Prior*         m_prior;
    TResult        m_result;

    deferred_member_fwd_invoker( C* obj, MF f, StateLifeCycle stateLife, Prior* prior )
        : m_obj( obj )
        , m_f( f )
        , m_stateLife( stateLife )
        , m_prior( prior )
        , m_result()
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_fwd_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_obj )
            m_result = (m_obj->*m_f)( m_stateLife       ,
                                      m_prior->m_result ,
                                      m_prior->m_a1     ,
                                      m_prior->m_a2     ,
                                      m_prior->m_a3     );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_fwd_invoker( m_obj, m_f, m_stateLife, m_prior );
    }
};

// arity_4: prior had 4 user args; new MF takes (StateLifeCycle, Prior::TResult, Prior::TArg1, ..4)
template< typename StateLifeCycle, typename C, typename MF, typename Prior >
class deferred_member_fwd_invoker< StateLifeCycle, C, MF, Prior, arity_4 > : public CDeferredTask
{
    public:

    typedef GUCEF::member_function_traits< MF >  traits;
    typedef typename traits::result_type          TResult;

    C*             m_obj;
    MF             m_f;
    StateLifeCycle m_stateLife;
    Prior*         m_prior;
    TResult        m_result;

    deferred_member_fwd_invoker( C* obj, MF f, StateLifeCycle stateLife, Prior* prior )
        : m_obj( obj )
        , m_f( f )
        , m_stateLife( stateLife )
        , m_prior( prior )
        , m_result()
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_fwd_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_obj )
            m_result = (m_obj->*m_f)( m_stateLife       ,
                                      m_prior->m_result ,
                                      m_prior->m_a1     ,
                                      m_prior->m_a2     ,
                                      m_prior->m_a3     ,
                                      m_prior->m_a4     );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_fwd_invoker( m_obj, m_f, m_stateLife, m_prior );
    }
};

/*-------------------------------------------------------------------------*/

//— deferred_member_shared_fwd_invoker: shared pointer + member function, forwarding prior invoker's results
//  The shared pointer is stored by value (strong reference), keeping the target alive.
//  ArityTag encodes the number of prior user args forwarded.

template< typename StateLifeCycle, typename SharedPtrT, typename MF, typename Prior, typename ArityTag >
struct deferred_member_shared_fwd_invoker;

// arity_0: prior had 0 user args; new MF takes (StateLifeCycle, Prior::TResult)
template< typename StateLifeCycle, typename SharedPtrT, typename MF, typename Prior >
class deferred_member_shared_fwd_invoker< StateLifeCycle, SharedPtrT, MF, Prior, arity_0 > : public CDeferredTask
{
    public:

    typedef typename SharedPtrT::TContainedType  C;
    typedef GUCEF::member_function_traits< MF >  traits;
    typedef typename traits::result_type          TResult;

    SharedPtrT     m_obj;    // strong reference — keeps target alive
    MF             m_f;
    StateLifeCycle m_stateLife;
    Prior*         m_prior;
    TResult        m_result;

    deferred_member_shared_fwd_invoker( SharedPtrT obj, MF f, StateLifeCycle stateLife, Prior* prior )
        : m_obj( obj )
        , m_f( f )
        , m_stateLife( stateLife )
        , m_prior( prior )
        , m_result()
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_shared_fwd_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        C* ptr = m_obj.GetPointerAlways();
        if ( GUCEF_NULL != ptr )
            m_result = (ptr->*m_f)( m_stateLife, m_prior->m_result );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_shared_fwd_invoker( m_obj, m_f, m_stateLife, m_prior );
    }
};

// arity_1: prior had 1 user arg; new MF takes (StateLifeCycle, Prior::TResult, Prior::TArg1)
template< typename StateLifeCycle, typename SharedPtrT, typename MF, typename Prior >
class deferred_member_shared_fwd_invoker< StateLifeCycle, SharedPtrT, MF, Prior, arity_1 > : public CDeferredTask
{
    public:

    typedef typename SharedPtrT::TContainedType  C;
    typedef GUCEF::member_function_traits< MF >  traits;
    typedef typename traits::result_type          TResult;

    SharedPtrT     m_obj;
    MF             m_f;
    StateLifeCycle m_stateLife;
    Prior*         m_prior;
    TResult        m_result;

    deferred_member_shared_fwd_invoker( SharedPtrT obj, MF f, StateLifeCycle stateLife, Prior* prior )
        : m_obj( obj )
        , m_f( f )
        , m_stateLife( stateLife )
        , m_prior( prior )
        , m_result()
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_shared_fwd_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        C* ptr = m_obj.GetPointerAlways();
        if ( GUCEF_NULL != ptr )
            m_result = (ptr->*m_f)( m_stateLife       ,
                                    m_prior->m_result ,
                                    m_prior->m_a1     );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_shared_fwd_invoker( m_obj, m_f, m_stateLife, m_prior );
    }
};

// arity_2: prior had 2 user args; new MF takes (StateLifeCycle, Prior::TResult, Prior::TArg1, Prior::TArg2)
template< typename StateLifeCycle, typename SharedPtrT, typename MF, typename Prior >
class deferred_member_shared_fwd_invoker< StateLifeCycle, SharedPtrT, MF, Prior, arity_2 > : public CDeferredTask
{
    public:

    typedef typename SharedPtrT::TContainedType  C;
    typedef GUCEF::member_function_traits< MF >  traits;
    typedef typename traits::result_type          TResult;

    SharedPtrT     m_obj;
    MF             m_f;
    StateLifeCycle m_stateLife;
    Prior*         m_prior;
    TResult        m_result;

    deferred_member_shared_fwd_invoker( SharedPtrT obj, MF f, StateLifeCycle stateLife, Prior* prior )
        : m_obj( obj )
        , m_f( f )
        , m_stateLife( stateLife )
        , m_prior( prior )
        , m_result()
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_shared_fwd_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        C* ptr = m_obj.GetPointerAlways();
        if ( GUCEF_NULL != ptr )
            m_result = (ptr->*m_f)( m_stateLife       ,
                                    m_prior->m_result ,
                                    m_prior->m_a1     ,
                                    m_prior->m_a2     );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_shared_fwd_invoker( m_obj, m_f, m_stateLife, m_prior );
    }
};

// arity_3: prior had 3 user args; new MF takes (StateLifeCycle, Prior::TResult, Prior::TArg1, ..3)
template< typename StateLifeCycle, typename SharedPtrT, typename MF, typename Prior >
class deferred_member_shared_fwd_invoker< StateLifeCycle, SharedPtrT, MF, Prior, arity_3 > : public CDeferredTask
{
    public:

    typedef typename SharedPtrT::TContainedType  C;
    typedef GUCEF::member_function_traits< MF >  traits;
    typedef typename traits::result_type          TResult;

    SharedPtrT     m_obj;
    MF             m_f;
    StateLifeCycle m_stateLife;
    Prior*         m_prior;
    TResult        m_result;

    deferred_member_shared_fwd_invoker( SharedPtrT obj, MF f, StateLifeCycle stateLife, Prior* prior )
        : m_obj( obj )
        , m_f( f )
        , m_stateLife( stateLife )
        , m_prior( prior )
        , m_result()
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_shared_fwd_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        C* ptr = m_obj.GetPointerAlways();
        if ( GUCEF_NULL != ptr )
            m_result = (ptr->*m_f)( m_stateLife       ,
                                    m_prior->m_result ,
                                    m_prior->m_a1     ,
                                    m_prior->m_a2     ,
                                    m_prior->m_a3     );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_shared_fwd_invoker( m_obj, m_f, m_stateLife, m_prior );
    }
};

// arity_4: prior had 4 user args; new MF takes (StateLifeCycle, Prior::TResult, Prior::TArg1, ..4)
template< typename StateLifeCycle, typename SharedPtrT, typename MF, typename Prior >
class deferred_member_shared_fwd_invoker< StateLifeCycle, SharedPtrT, MF, Prior, arity_4 > : public CDeferredTask
{
    public:

    typedef typename SharedPtrT::TContainedType  C;
    typedef GUCEF::member_function_traits< MF >  traits;
    typedef typename traits::result_type          TResult;

    SharedPtrT     m_obj;
    MF             m_f;
    StateLifeCycle m_stateLife;
    Prior*         m_prior;
    TResult        m_result;

    deferred_member_shared_fwd_invoker( SharedPtrT obj, MF f, StateLifeCycle stateLife, Prior* prior )
        : m_obj( obj )
        , m_f( f )
        , m_stateLife( stateLife )
        , m_prior( prior )
        , m_result()
    {GUCEF_TRACE;

    }

    virtual ~deferred_member_shared_fwd_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        C* ptr = m_obj.GetPointerAlways();
        if ( GUCEF_NULL != ptr )
            m_result = (ptr->*m_f)( m_stateLife       ,
                                    m_prior->m_result ,
                                    m_prior->m_a1     ,
                                    m_prior->m_a2     ,
                                    m_prior->m_a3     ,
                                    m_prior->m_a4     );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_member_shared_fwd_invoker( m_obj, m_f, m_stateLife, m_prior );
    }
};

/*-------------------------------------------------------------------------*/

//— deferred_invoker specializations
template< typename StateLifeCycle, typename Prior, typename FuncPtr, typename Tag > struct deferred_fwd_invoker;

// arity 0
template< typename StateLifeCycle, typename Prior, typename FuncPtr >
class deferred_fwd_invoker< StateLifeCycle, Prior, FuncPtr, arity_0> : public CDeferredTask
{
    public:

    typedef function_traits<typename remove_pointer<FuncPtr>::type>         traits;
    typedef FuncPtr                                                         TGivenFuncPtr;
    typedef Prior                                                           TPriorInvoker;
    typedef typename traits::result_type                                    TFuncResult;

    StateLifeCycle m_stateLife;
    TPriorInvoker* m_prior;
    TGivenFuncPtr m_f;
    TFuncResult m_result;

    deferred_fwd_invoker( StateLifeCycle stateLife, TPriorInvoker* prior, FuncPtr f )
        : m_stateLife( stateLife )
        , m_prior( prior )
        , m_f( f )
        , m_result()
    {GUCEF_TRACE;

    }

    virtual ~deferred_fwd_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_result = (*m_f)( m_stateLife       ,
                           m_prior->m_result );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_fwd_invoker( m_stateLife, m_prior, m_f );
    }
};


// arity 1
template< typename StateLifeCycle, typename Prior, typename FuncPtr >
class deferred_fwd_invoker< StateLifeCycle, Prior, FuncPtr, arity_1> : public CDeferredTask
{
    public:

    typedef function_traits<typename remove_pointer<FuncPtr>::type>         traits;
    typedef FuncPtr                                                         TGivenFuncPtr;
    typedef Prior                                                           TPriorInvoker;
    typedef typename traits::result_type                                    TFuncResult;
    typedef typename traits::arg1_type                                      TFuncArgument1;

    StateLifeCycle m_stateLife;
    TPriorInvoker* m_prior;
    TGivenFuncPtr m_f;
    TFuncResult m_result;

    deferred_fwd_invoker( StateLifeCycle stateLife, TPriorInvoker* prior, FuncPtr f )
        : m_stateLife( stateLife )
        , m_prior( prior )
        , m_f( f )
        , m_result()
    {GUCEF_TRACE;

    }

    virtual ~deferred_fwd_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_result = (*m_f)( m_stateLife       ,
                           m_prior->m_result ,
                           m_prior->m_a1     );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_fwd_invoker( m_stateLife, m_prior, m_f );
    }
};

// arity 2
template< typename StateLifeCycle, typename Prior, typename FuncPtr >
class deferred_fwd_invoker< StateLifeCycle, Prior, FuncPtr, arity_2> : public CDeferredTask
{
    public:

    typedef function_traits<typename remove_pointer<FuncPtr>::type>         traits;
    typedef FuncPtr                                                         TGivenFuncPtr;
    typedef Prior                                                           TPriorInvoker;
    typedef typename traits::result_type                                    TFuncResult;
    typedef typename traits::arg1_type                                      TFuncArgument1;
    typedef typename traits::arg2_type                                      TFuncArgument2;

    StateLifeCycle m_stateLife;
    TPriorInvoker* m_prior;
    TGivenFuncPtr m_f;
    TFuncResult m_result;

    deferred_fwd_invoker( StateLifeCycle stateLife, TPriorInvoker* prior, FuncPtr f )
        : m_stateLife( stateLife )
        , m_prior( prior )
        , m_f( f )
        , m_result()
    {GUCEF_TRACE;

    }

    virtual ~deferred_fwd_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_result = (*m_f)( m_stateLife       ,
                           m_prior->m_result ,
                           m_prior->m_a1     ,
                           m_prior->m_a2     );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_fwd_invoker( m_stateLife, m_prior, m_f );
    }
};

// arity 3
template< typename StateLifeCycle, typename Prior, typename FuncPtr >
class deferred_fwd_invoker< StateLifeCycle, Prior, FuncPtr, arity_3> : public CDeferredTask
{
    public:

    typedef function_traits<typename remove_pointer<FuncPtr>::type>         traits;
    typedef FuncPtr                                                         TGivenFuncPtr;
    typedef Prior                                                           TPriorInvoker;
    typedef typename traits::result_type                                    TFuncResult;
    typedef typename traits::arg1_type                                      TFuncArgument1;
    typedef typename traits::arg2_type                                      TFuncArgument2;
    typedef typename traits::arg3_type                                      TFuncArgument3;

    StateLifeCycle m_stateLife;
    TPriorInvoker* m_prior;
    TGivenFuncPtr m_f;
    TFuncResult m_result;

    deferred_fwd_invoker( StateLifeCycle stateLife, TPriorInvoker* prior, FuncPtr f )
        : m_stateLife( stateLife )
        , m_prior( prior )
        , m_f( f )
        , m_result()
    {GUCEF_TRACE;

    }

    virtual ~deferred_fwd_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_result = (*m_f)( m_stateLife       ,
                           m_prior->m_result ,
                           m_prior->m_a1     ,
                           m_prior->m_a2     ,
                           m_prior->m_a3     );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_fwd_invoker( m_stateLife, m_prior, m_f );
    }
};

// arity 4
template< typename StateLifeCycle, typename Prior, typename FuncPtr >
class deferred_fwd_invoker< StateLifeCycle, Prior, FuncPtr, arity_4> : public CDeferredTask
{
    public:

    typedef function_traits<typename remove_pointer<FuncPtr>::type>         traits;
    typedef FuncPtr                                                         TGivenFuncPtr;
    typedef Prior                                                           TPriorInvoker;
    typedef typename traits::result_type                                    TFuncResult;
    typedef typename traits::arg1_type                                      TFuncArgument1;
    typedef typename traits::arg2_type                                      TFuncArgument2;
    typedef typename traits::arg3_type                                      TFuncArgument3;
    typedef typename traits::arg4_type                                      TFuncArgument4;

    StateLifeCycle m_stateLife;
    TPriorInvoker* m_prior;
    TGivenFuncPtr m_f;
    TFuncResult m_result;

    deferred_fwd_invoker( StateLifeCycle stateLife, TPriorInvoker* prior, FuncPtr f )
        : m_stateLife( stateLife )
        , m_prior( prior )
        , m_f( f )
        , m_result()
    {GUCEF_TRACE;

    }

    virtual ~deferred_fwd_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_result = (*m_f)( m_stateLife       ,
                           m_prior->m_result ,
                           m_prior->m_a1     ,
                           m_prior->m_a2     ,
                           m_prior->m_a3     ,
                           m_prior->m_a4     );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_fwd_invoker( m_stateLife, m_prior, m_f );
    }
};

// arity 5
template< typename StateLifeCycle, typename Prior, typename FuncPtr >
class deferred_fwd_invoker< StateLifeCycle, Prior, FuncPtr, arity_5> : public CDeferredTask
{
    public:

    typedef function_traits<typename remove_pointer<FuncPtr>::type>         traits;
    typedef FuncPtr                                                         TGivenFuncPtr;
    typedef Prior                                                           TPriorInvoker;
    typedef typename traits::result_type                                    TFuncResult;
    typedef typename traits::arg1_type                                      TFuncArgument1;
    typedef typename traits::arg2_type                                      TFuncArgument2;
    typedef typename traits::arg3_type                                      TFuncArgument3;
    typedef typename traits::arg4_type                                      TFuncArgument4;
    typedef typename traits::arg5_type                                      TFuncArgument5;

    StateLifeCycle m_stateLife;
    TPriorInvoker* m_prior;
    TGivenFuncPtr m_f;
    TFuncResult m_result;

    deferred_fwd_invoker( StateLifeCycle stateLife, TPriorInvoker* prior, FuncPtr f )
        : m_stateLife( stateLife )
        , m_prior( prior )
        , m_f( f )
        , m_result()
    {GUCEF_TRACE;

    }

    virtual ~deferred_fwd_invoker() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual void Invoke( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_result = (*m_f)( m_stateLife       ,
                           m_prior->m_result ,
                           m_prior->m_a1     ,
                           m_prior->m_a2     ,
                           m_prior->m_a3     ,
                           m_prior->m_a4     ,
                           m_prior->m_a5     );
    }

    virtual CICloneable* Clone() const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW deferred_fwd_invoker( m_stateLife, m_prior, m_f );
    }
};

/*-------------------------------------------------------------------------*/

// arity 0
template< typename R >
CDeferredTask*
CDeferredTask::Construct( R (*f)() )
{GUCEF_TRACE;

    typedef deferred_invoker< R (*)(), arity_0 > Impl;
    return GUCEF_NEW Impl( f );
}

// arity 1
template< typename R, typename A1 >
CDeferredTask*
CDeferredTask::Construct( R (*f)(A1), A1 a1 )
{GUCEF_TRACE;

    typedef deferred_invoker< R (*)(A1), arity_1 > Impl;
    return GUCEF_NEW Impl( f, a1 );
}

// arity 2
template< typename R, typename A1, typename A2 >
CDeferredTask*
CDeferredTask::Construct( R (*f)(A1, A2), A1 a1, A2 a2 )
{GUCEF_TRACE;

    typedef deferred_invoker< R (*)(A1, A2), arity_2 > Impl;
    return GUCEF_NEW Impl( f, a1, a2 );
}

// arity 3
template< typename R, typename A1, typename A2, typename A3 >
CDeferredTask*
CDeferredTask::Construct( R (*f)(A1, A2, A3), A1 a1, A2 a2, A3 a3 )
{GUCEF_TRACE;

    typedef deferred_invoker< R (*)(A1, A2, A3), arity_3 > Impl;
    return GUCEF_NEW Impl( f, a1, a2, a3 );
}

// arity 4
template< typename R, typename A1, typename A2, typename A3, typename A4 >
CDeferredTask*
CDeferredTask::Construct( R (*f)(A1, A2, A3, A4), A1 a1, A2 a2, A3 a3, A4 a4 )
{GUCEF_TRACE;

    typedef deferred_invoker< R (*)(A1, A2, A3, A4), arity_4 > Impl;
    return GUCEF_NEW Impl( f, a1, a2, a3, a4 );
}

/*-------------------------------------------------------------------------*/

// ConstructMember — raw pointer factories (arity 0–4)

// arity 0
template< typename C, typename MF >
CDeferredTask*
CDeferredTask::ConstructMember( C* obj, MF f )
{GUCEF_TRACE;

    typedef GUCEF::member_function_traits< MF >                     traits;
    typedef deferred_member_invoker< C, MF, typename traits::tag >  Impl;
    return GUCEF_NEW Impl( obj, f );
}

// arity 1
template< typename C, typename MF, typename A1 >
CDeferredTask*
CDeferredTask::ConstructMember( C* obj, MF f, A1 a1 )
{GUCEF_TRACE;

    typedef GUCEF::member_function_traits< MF >                     traits;
    typedef deferred_member_invoker< C, MF, typename traits::tag >  Impl;
    return GUCEF_NEW Impl( obj, f, a1 );
}

// arity 2
template< typename C, typename MF, typename A1, typename A2 >
CDeferredTask*
CDeferredTask::ConstructMember( C* obj, MF f, A1 a1, A2 a2 )
{GUCEF_TRACE;

    typedef GUCEF::member_function_traits< MF >                     traits;
    typedef deferred_member_invoker< C, MF, typename traits::tag >  Impl;
    return GUCEF_NEW Impl( obj, f, a1, a2 );
}

// arity 3
template< typename C, typename MF, typename A1, typename A2, typename A3 >
CDeferredTask*
CDeferredTask::ConstructMember( C* obj, MF f, A1 a1, A2 a2, A3 a3 )
{GUCEF_TRACE;

    typedef GUCEF::member_function_traits< MF >                     traits;
    typedef deferred_member_invoker< C, MF, typename traits::tag >  Impl;
    return GUCEF_NEW Impl( obj, f, a1, a2, a3 );
}

// arity 4
template< typename C, typename MF, typename A1, typename A2, typename A3, typename A4 >
CDeferredTask*
CDeferredTask::ConstructMember( C* obj, MF f, A1 a1, A2 a2, A3 a3, A4 a4 )
{GUCEF_TRACE;

    typedef GUCEF::member_function_traits< MF >                     traits;
    typedef deferred_member_invoker< C, MF, typename traits::tag >  Impl;
    return GUCEF_NEW Impl( obj, f, a1, a2, a3, a4 );
}

/*-------------------------------------------------------------------------*/

// ConstructMemberShared — shared pointer factories (arity 0–4)

// arity 0
template< typename SharedPtrT, typename MF >
CDeferredTask*
CDeferredTask::ConstructMemberShared( SharedPtrT obj, MF f )
{GUCEF_TRACE;

    typedef GUCEF::member_function_traits< MF >                                         traits;
    typedef deferred_member_shared_invoker< SharedPtrT, MF, typename traits::tag >     Impl;
    return GUCEF_NEW Impl( obj, f );
}

// arity 1
template< typename SharedPtrT, typename MF, typename A1 >
CDeferredTask*
CDeferredTask::ConstructMemberShared( SharedPtrT obj, MF f, A1 a1 )
{GUCEF_TRACE;

    typedef GUCEF::member_function_traits< MF >                                         traits;
    typedef deferred_member_shared_invoker< SharedPtrT, MF, typename traits::tag >     Impl;
    return GUCEF_NEW Impl( obj, f, a1 );
}

// arity 2
template< typename SharedPtrT, typename MF, typename A1, typename A2 >
CDeferredTask*
CDeferredTask::ConstructMemberShared( SharedPtrT obj, MF f, A1 a1, A2 a2 )
{GUCEF_TRACE;

    typedef GUCEF::member_function_traits< MF >                                         traits;
    typedef deferred_member_shared_invoker< SharedPtrT, MF, typename traits::tag >     Impl;
    return GUCEF_NEW Impl( obj, f, a1, a2 );
}

// arity 3
template< typename SharedPtrT, typename MF, typename A1, typename A2, typename A3 >
CDeferredTask*
CDeferredTask::ConstructMemberShared( SharedPtrT obj, MF f, A1 a1, A2 a2, A3 a3 )
{GUCEF_TRACE;

    typedef GUCEF::member_function_traits< MF >                                         traits;
    typedef deferred_member_shared_invoker< SharedPtrT, MF, typename traits::tag >     Impl;
    return GUCEF_NEW Impl( obj, f, a1, a2, a3 );
}

// arity 4
template< typename SharedPtrT, typename MF, typename A1, typename A2, typename A3, typename A4 >
CDeferredTask*
CDeferredTask::ConstructMemberShared( SharedPtrT obj, MF f, A1 a1, A2 a2, A3 a3, A4 a4 )
{GUCEF_TRACE;

    typedef GUCEF::member_function_traits< MF >                                         traits;
    typedef deferred_member_shared_invoker< SharedPtrT, MF, typename traits::tag >     Impl;
    return GUCEF_NEW Impl( obj, f, a1, a2, a3, a4 );
}

/*-------------------------------------------------------------------------*/

// arity 0
template< typename StateLifeCycle, typename P, typename R >
CDeferredTask*
CDeferredTask::ResultFwdConstruct( StateLifeCycle stateLife, P* prior, R (*f)( StateLifeCycle, R ) )
{GUCEF_TRACE;

    typedef R (*TGivenFuncPtr)( StateLifeCycle, R );
    typedef deferred_fwd_invoker< StateLifeCycle, P, TGivenFuncPtr, arity_0 > Impl;

    return GUCEF_NEW Impl( stateLife, prior, f );
}

// arity 1
template< typename StateLifeCycle, typename P, typename R, typename A1 >
CDeferredTask*
CDeferredTask::ResultFwdConstruct( StateLifeCycle stateLife, P* prior, R (*f)( StateLifeCycle, R, A1 ) )
{GUCEF_TRACE;

    typedef R (*TGivenFuncPtr)( StateLifeCycle, R, A1 );
    typedef deferred_fwd_invoker< StateLifeCycle, P, TGivenFuncPtr, arity_1 > Impl;

    return GUCEF_NEW Impl( stateLife, prior, f );
}

// arity 2
template< typename StateLifeCycle, typename P, typename R, typename A1, typename A2 >
CDeferredTask*
CDeferredTask::ResultFwdConstruct( StateLifeCycle stateLife, P* prior, R (*f)( StateLifeCycle, R, A1, A2 ) )
{GUCEF_TRACE;

    typedef R (*TGivenFuncPtr)( StateLifeCycle, R, A1, A2 );
    typedef deferred_fwd_invoker< StateLifeCycle, P, TGivenFuncPtr, arity_2 > Impl;

    return GUCEF_NEW Impl( stateLife, prior, f );
}

// arity 3
template< typename StateLifeCycle, typename P, typename R, typename A1, typename A2, typename A3 >
CDeferredTask*
CDeferredTask::ResultFwdConstruct( StateLifeCycle stateLife, P* prior, R (*f)( StateLifeCycle, R, A1, A2, A3 ) )
{GUCEF_TRACE;

    typedef R (*TGivenFuncPtr)( StateLifeCycle, R, A1, A2, A3 );
    typedef deferred_fwd_invoker< StateLifeCycle, P, TGivenFuncPtr, arity_3 > Impl;

    return GUCEF_NEW Impl( stateLife, prior, f );
}

// arity 4
template< typename StateLifeCycle, typename P, typename R, typename A1, typename A2, typename A3, typename A4 >
CDeferredTask*
CDeferredTask::ResultFwdConstruct( StateLifeCycle stateLife, P* prior, R (*f)( StateLifeCycle, R, A1, A2, A3, A4 ) )
{GUCEF_TRACE;

    typedef R (*TGivenFuncPtr)( StateLifeCycle, R, A1, A2, A3, A4 );
    typedef deferred_fwd_invoker< StateLifeCycle, P, TGivenFuncPtr, arity_4 > Impl;

    return GUCEF_NEW Impl( stateLife, prior, f );
}

// arity 5
template< typename StateLifeCycle, typename P, typename R, typename A1, typename A2, typename A3, typename A4, typename A5 >
CDeferredTask*
CDeferredTask::ResultFwdConstruct( StateLifeCycle stateLife, P* prior, R (*f)( StateLifeCycle, R, A1, A2, A3, A4, A5 ) )
{GUCEF_TRACE;

    typedef R (*TGivenFuncPtr)( StateLifeCycle, R, A1, A2, A3, A4, A5 );
    typedef deferred_fwd_invoker< StateLifeCycle, P, TGivenFuncPtr, arity_5 > Impl;

    return GUCEF_NEW Impl( stateLife, prior, f );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDEFERREDTASK_H ? */
