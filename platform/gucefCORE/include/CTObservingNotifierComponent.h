/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CTOBSERVINGNOTIFIERCOMPONENT_H
#define GUCEF_CORE_CTOBSERVINGNOTIFIERCOMPONENT_H
 
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

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
 *  Template for encapsulating a CObservingNotifier instance in another class
 *  It will proxy the mandatory overloads to the component owner, allowing instances
 *  to be created without having to implement the pure virtuals.
 *
 *  Note that as a user you will most likely only want to use the
 *  CTObservingNotifierExpansion template. Check it out if all you want to do
 *  is add notification to your class.
 *
 *  Note: When using this template you must implement OnNotify() in your class with the
 *  same semantics as it is implemented here and provide the template instantiation
 *  with access to your OnNotify() (if so desired with a friend relation).
 *  You must also place 'varName.SetOwner( this )' in your class constructor where
 *  'varName' is the name the instance of this class template.
 */
template < class TOwnerClass >
class CTObservingNotifierComponent : public CObservingNotifier
{
    public:
    
    CTObservingNotifierComponent( void );

    CTObservingNotifierComponent( const CTObservingNotifierComponent& src );
    
    virtual ~CTObservingNotifierComponent();

    CTObservingNotifierComponent& operator=( const CTObservingNotifierComponent& src );
    
    /**
     *  Sets the callback owner class pointer.
     *  must be set in your class contructor using
     *      'SetOwner( this );'
     *
     *  @param ownerInstance pointer to the owner of the compound instance.
     */
    void SetOwner( TOwnerClass* ownerInstance );
    
    /**
     *  Dispatches the standard CNotifier::ModifyEvent
     *  Useful for a notification system where you only care if
     *  a mutation is performed on an object.
     *
     *  @return Alive state of the notifier, if false then the notifier died itself as a result of the notification.
     */
    bool DoNotifyObservers( void );
    
    /**
     *  Dispatches the given eventid and eventData to all observers
     *  that are subscribed to all events and the observers that are subscribed
     *  to this specific eventid.
     *
     *  Note that the calling thread is the one in which the observer OnNotify 
     *  event handlers will be processed. Keep this in mind when notification
     *  occurs across thread boundaries.
     *
     *  Note that eventData is not copied. So when passing data across threads
     *  consider allocating a copy and passing that in as the data argument.
     *
     *  @return Alive state of the notifier, if false then the notifier died itself as a result of the notification.
     */
    bool DoNotifyObservers( const CEvent& eventid         ,
                            CICloneable* eventData = GUCEF_NULL );

    /**
     *  The same as DoNotifyObservers( CEvent, CICloneable )
     *  except that this allows you to specify the sender yourself.
     *  You will basically be faking an event emitted at the given sender.
     *  
     *  Use with great care !!!
     *  Use of this version should be an exception and not standard practice
     *
     *  @return Alive state of the notifier, if false then the notifier died itself as a result of the notification.
     */
    bool DoNotifyObservers( CNotifier& sender             ,
                            const CEvent& eventid         ,
                            CICloneable* eventData = GUCEF_NULL );
    
    virtual const CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    /**
     *  Event callback member function.
     *  Implement this in your owner class to handle
     *  notification events. This template will proxy the 
     *  callback to the owner class.
     *
     *  @param notifier the notifier that sent the notification
     *  @param eventid the unique event id for an event
     *  @param eventdata optional notifier defined userdata
     */
    virtual void OnNotify( CNotifier* notifier                 ,
                           const CEvent& eventid               ,
                           CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;
    
    private:
    
    TOwnerClass* m_componentOwner;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template < class TOwnerClass >
CTObservingNotifierComponent< TOwnerClass >::CTObservingNotifierComponent( void )
    : CObservingNotifier()     ,
      m_componentOwner( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template < class TOwnerClass >
CTObservingNotifierComponent< TOwnerClass >::CTObservingNotifierComponent( const CTObservingNotifierComponent& src )
    : CObservingNotifier( src ) ,
      m_componentOwner( GUCEF_NULL )
{GUCEF_TRACE;
    
}

/*-------------------------------------------------------------------------*/

template < class TOwnerClass >
CTObservingNotifierComponent< TOwnerClass >::~CTObservingNotifierComponent( void )
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

template < class TOwnerClass >
CTObservingNotifierComponent< TOwnerClass >&
CTObservingNotifierComponent< TOwnerClass >::operator=( const CTObservingNotifierComponent& src )
{GUCEF_TRACE;

    CObservingNotifier::operator=( src );
    
    if ( &src != this )
    {
        /* don't do anything, leave the owner pointer as is */
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

template < class TOwnerClass >
void 
CTObservingNotifierComponent< TOwnerClass >::SetOwner( TOwnerClass* ownerInstance )
{GUCEF_TRACE;

    m_componentOwner = ownerInstance;
}

/*-------------------------------------------------------------------------*/

template < class TOwnerClass >
bool
CTObservingNotifierComponent< TOwnerClass >::DoNotifyObservers( void )
{GUCEF_TRACE;

    return NotifyObservers();
}

/*-------------------------------------------------------------------------*/

template < class TOwnerClass >
bool
CTObservingNotifierComponent< TOwnerClass >::DoNotifyObservers( const CEvent& eventid               ,
                                                                CICloneable* eventData /* = NULL */ )
{GUCEF_TRACE;

    return NotifyObservers( eventid   ,
                            eventData );
}

/*-------------------------------------------------------------------------*/

template < class TOwnerClass >
bool
CTObservingNotifierComponent< TOwnerClass >::DoNotifyObservers( CNotifier& sender                   ,
                                                                const CEvent& eventid               ,
                                                                CICloneable* eventData /* = NULL */ )
{GUCEF_TRACE;

    return NotifyObservers( sender    ,
                            eventid   ,
                            eventData );
}

/*-------------------------------------------------------------------------*/

template < class TOwnerClass >
void
CTObservingNotifierComponent< TOwnerClass >::OnNotify( CNotifier* notifier                 ,
                                                       const CEvent& eventid               ,
                                                       CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    /*
     *  If you crash here then you forgot to call SetOwner() in your
     *  class constructor.
     */
    assert( m_componentOwner );
    
    /*
     *  Simply proxy the call to the owner class.
     *  This is the purpose of this template 
     */
    m_componentOwner->OnNotify( notifier  ,
                                eventid   ,
                                eventdata );
}

/*-------------------------------------------------------------------------*/

template< class BaseClass >
const MT::CILockable* 
CTObservingNotifierComponent< BaseClass >::AsLockable( void ) const
{GUCEF_TRACE;

    assert( GUCEF_NULL != m_componentOwner );
    return m_componentOwner->AsLockable();
}

/*-------------------------------------------------------------------------*/

template< class BaseClass >
MT::TLockStatus
CTObservingNotifierComponent< BaseClass >::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    assert( GUCEF_NULL != m_componentOwner );
    return m_componentOwner->Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

template< class BaseClass >
MT::TLockStatus
CTObservingNotifierComponent< BaseClass >::Unlock( void ) const
{GUCEF_TRACE;

    assert( GUCEF_NULL != m_componentOwner );
    return m_componentOwner->Unlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTOBSERVINGNOTIFIERCOMPONENT_H  ? */
