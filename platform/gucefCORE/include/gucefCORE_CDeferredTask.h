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

/** 
 *  Arity tags (number of arguments) for function_traits.
 */
struct arity_0 {};
struct arity_1 {};
struct arity_2 {};
struct arity_3 {};
struct arity_4 {};
struct arity_5 {};
struct arity_6 {};
struct arity_7 {};
struct arity_8 {};
struct arity_9 {};
struct arity_10 {};

/*-------------------------------------------------------------------------*/

//— function_traits for free functions (arity 0…2)
template< typename Sig >     struct function_traits;

// arity 0
template<typename R>
struct function_traits<R()> {
    typedef R       result_type;
    typedef arity_0 tag;
};

// arity 1
template<typename R, typename A1>
struct function_traits<R(A1)> {
    typedef R       result_type;
    typedef A1      arg1_type;
    typedef arity_1 tag;
};

// arity 2
template<typename R, typename A1, typename A2>
struct function_traits<R(A1, A2)> {
    typedef R       result_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef arity_2 tag;
};

// arity 3
template<typename R, typename A1, typename A2, typename A3>
struct function_traits<R(A1, A2, A3)> {
    typedef R       result_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef arity_3 tag;
};

// arity 4
template<typename R, typename A1, typename A2, typename A3, typename A4>
struct function_traits<R(A1, A2, A3, A4)> {
    typedef R       result_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef arity_4 tag;
};

// arity 5
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
struct function_traits<R(A1, A2, A3, A4, A5)> {
    typedef R       result_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef A5      arg5_type;
    typedef arity_5 tag;
};

// arity 6
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct function_traits<R(A1, A2, A3, A4, A5, A6)> {
    typedef R       result_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef A5      arg5_type;
    typedef A6      arg6_type;
    typedef arity_6 tag;
};

// arity 7
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
struct function_traits<R(A1, A2, A3, A4, A5, A6, A7)> {
    typedef R       result_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef A5      arg5_type;
    typedef A6      arg6_type;
    typedef A7      arg7_type;
    typedef arity_7 tag;
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

    deferred_invoker(FuncPtr f, typename traits::arg1_type a1)
      : m_f(f), m_result(), m_a1(a1) {GUCEF_TRACE; }

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
    {GUCEF_TRACE;  }

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
    {GUCEF_TRACE;  }

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
    {GUCEF_TRACE;  }

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
    {GUCEF_TRACE;  }

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
    {GUCEF_TRACE;  }

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
    {GUCEF_TRACE;  }

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
