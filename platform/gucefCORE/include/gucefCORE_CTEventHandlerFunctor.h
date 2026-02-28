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

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_LOGTYPES_H
#include "gucefCORE_LogTypes.h"
#define GUCEF_CORE_LOGTYPES_H
#endif /* GUCEF_CORE_LOGTYPES_H ? */

#ifndef GUCEF_CORE_LOGGINGMACROS_H
#include "gucefCORE_LoggingMacros.h"
#define GUCEF_CORE_LOGGINGMACROS_H
#endif /* GUCEF_CORE_LOGGINGMACROS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CIEVENTHANDLERFUNCTORBASE_H
#include "gucefCORE_CIEventHandlerFunctorBase.h"
#define GUCEF_CORE_CIEVENTHANDLERFUNCTORBASE_H
#endif /* GUCEF_CORE_CIEVENTHANDLERFUNCTORBASE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASS TEMPLATES                                                    //
//                                                                         //
//-------------------------------------------------------------------------*/

template < class IObserverDerived >
class CTEventHandlerFunctor : public virtual CIEventHandlerFunctorBase
{
    public:

    // Member function pointer type definition.
    typedef void (IObserverDerived::*TMemberFunctionType)( CNotifier* notifier   ,
                                                           const CEvent& event   ,
                                                           CICloneable* evenData );

    CTEventHandlerFunctor( IObserverDerived* observer  ,
                           TMemberFunctionType functor );

    CTEventHandlerFunctor( const CTEventHandlerFunctor& src );

    virtual ~CTEventHandlerFunctor();

    CTEventHandlerFunctor& operator=( const CTEventHandlerFunctor& src );

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CIObserver* GetTargetObserver( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CIObserver* GetTargetObserver( void ) GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnNotify( CNotifier* notifier                ,
                           const CEvent& eventId              ,
                           CICloneable* evenData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    void SetMemberFunctionPointer( TMemberFunctionType functor );

    private:

    CTEventHandlerFunctor( void );

    private:

    IObserverDerived* m_observer;
    TMemberFunctionType m_functor;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template< class IObserverDerived >
CTEventHandlerFunctor< IObserverDerived >::CTEventHandlerFunctor( IObserverDerived* observer  ,
                                                                  TMemberFunctionType functor )
    : CIEventHandlerFunctorBase() ,
      m_observer( observer )      ,
      m_functor( functor )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< class IObserverDerived >
CTEventHandlerFunctor< IObserverDerived >::CTEventHandlerFunctor( const CTEventHandlerFunctor& src )
    :  CIEventHandlerFunctorBase( src ) ,
       m_observer( src.m_observer )     ,
       m_functor( src.m_functor )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename IObserverDerived >
CTEventHandlerFunctor< IObserverDerived >::~CTEventHandlerFunctor()
{GUCEF_TRACE;

    m_observer = GUCEF_NULL;
    m_functor = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

template< class IObserverDerived >
CTEventHandlerFunctor< IObserverDerived >&
CTEventHandlerFunctor< IObserverDerived >::operator=( const CTEventHandlerFunctor& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CIEventHandlerFunctorBase::operator=( src );

        m_observer = src.m_observer;
        m_functor = src.m_functor;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

template< class IObserverDerived >
const CString&
CTEventHandlerFunctor< IObserverDerived >::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString typeName = "GUCEF::CORE::CTEventHandlerFunctor< IObserverDerived >";
    return typeName;
}

/*-------------------------------------------------------------------------*/

template< class IObserverDerived >
void
CTEventHandlerFunctor< IObserverDerived >::OnNotify( CNotifier* notifier   ,
                                                     const CEvent& eventID ,
                                                     CICloneable* evenData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_observer && GUCEF_NULL != m_functor )
    {
        GUCEF_DEBUG_LOG_EVERYTHING( "CTEventHandlerFunctor(" + CORE::PointerToString( this ) + "): Class " + notifier->GetClassTypeName() + 
            ": Dispatching event \"" + eventID.GetName() + "\" to " + m_observer->GetClassTypeName() + "(" + CORE::PointerToString( m_observer ) + ")" );

        (m_observer->*m_functor)( notifier ,
                                  eventID  ,
                                  evenData );
    }
    else
    {
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "CTEventHandlerFunctor(" + CORE::PointerToString( this ) + "): Class " + notifier->GetClassTypeName() + 
            ": Dispatching event \"" + eventID.GetName() + "\" to invalid functor destination" );
    }
}

/*-------------------------------------------------------------------------*/

template< class IObserverDerived >
CICloneable*
CTEventHandlerFunctor< IObserverDerived >::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CTEventHandlerFunctor< IObserverDerived >( *this );
}

/*-------------------------------------------------------------------------*/

template< class IObserverDerived >
const CIObserver* 
CTEventHandlerFunctor< IObserverDerived >::GetTargetObserver( void ) const
{GUCEF_TRACE;

    return static_cast< const CIObserver* >( m_observer );
}

/*-------------------------------------------------------------------------*/

template< class IObserverDerived >
CIObserver* 
CTEventHandlerFunctor< IObserverDerived >::GetTargetObserver( void )
{GUCEF_TRACE;

    return static_cast< CIObserver* >( m_observer );
}

/*-------------------------------------------------------------------------*/

template< class IObserverDerived >
void
CTEventHandlerFunctor< IObserverDerived >::SetMemberFunctionPointer( TMemberFunctionType functor )
{GUCEF_TRACE;

    m_functor = functor;
}

/*-------------------------------------------------------------------------*/

template< class IObserverDerived >
const MT::CILockable* 
CTEventHandlerFunctor< IObserverDerived >::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

template< class IObserverDerived >
MT::TLockStatus
CTEventHandlerFunctor< IObserverDerived >::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return MT::LOCKSTATUS_NOT_APPLICABLE;
}

/*-------------------------------------------------------------------------*/

template< class IObserverDerived >
MT::TLockStatus
CTEventHandlerFunctor< IObserverDerived >::Unlock( void ) const
{GUCEF_TRACE;

    return MT::LOCKSTATUS_NOT_APPLICABLE;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */
