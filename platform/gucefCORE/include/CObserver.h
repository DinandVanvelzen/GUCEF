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

#ifndef GUCEF_CORE_COBSERVER_H
#define GUCEF_CORE_COBSERVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <set>

#include "CICloneable.h"

#ifndef GUCEF_CORE_CEVENT_H
#include "CEvent.h"
#define GUCEF_CORE_CEVENT_H
#endif /* GUCEF_CORE_CEVENT_H ? */

#ifndef GUCEF_CORE_CIOBSERVER_H
#include "gucefCORE_CIObserver.h"
#define GUCEF_CORE_CIOBSERVER_H
#endif /* GUCEF_CORE_CIOBSERVER_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CNotifier;
class CNotifierImplementor;

/*-------------------------------------------------------------------------*/

/**
 *  Implementation of the observation component in the notifier-observer
 *  design pattern.
 *
 *  Note that you should unsubscribe the descending class before deallocating it.
 *  This can be done by calling UnsubscribeAll() from the destructor of your
 *  descending class. Failure to do this will not cause any problems but should be
 *  considered to be dirty coding.
 *
 *  Note that in general it is recommended not to delete a notifier while a
 *  notification is being handled, ie during the OnNotify() callback. It is
 *  possible to do this but it is a rather involved process for both the observer
 *  and the notifier. The recommended method for deleting a notifier that is
 *  notifying you is to schedule the notifier for deletion after the notification
 *  finishes.
 */
class GUCEF_CORE_PUBLIC_CPP CObserver : public CIObserver
{
    public:

    CObserver( void );

    CObserver( const CObserver& src );

    virtual ~CObserver();

    CObserver& operator=( const CObserver& src );

    void SubscribeTo( CNotifier* notifier );

    void SubscribeTo( CNotifier* notifier   ,
                      const CEvent& eventid );

    void SubscribeTo( CNotifier* notifier                 ,
                      const CEvent& eventid               ,
                      CIEventHandlerFunctorBase& callback );

    /**
     *  Unsubscribe's the observer from all notifiers.
     */
    void UnsubscribeAllFromObserver( void );

    /**
     *  Detaches from the given notifier.
     *  All the observers subscriptions will be cancelled
     *  This includes both standard notifier events as well
     *  as custom events.
     */
    void UnsubscribeFrom( CNotifier& notifier );

    UInt32 GetSubscriptionCount( void );

    UInt32 GetNotifierCount( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:
    friend class CNotifierImplementor;

    /**
     *  Event callback member function.
     *  Implement this in your descending class to handle
     *  notification events.
     *
     *  @param notifier the notifier that sent the notification
     *  @param eventid the unique event id for an event
     *  @param eventdata optional notifier defined userdata
     */
    virtual void OnNotify( CNotifier* notifier                 ,
                           const CEvent& eventid               ,
                           CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void SignalUpcomingObserverDestruction( void );

    private:

    /*
     *  Simply updates the observer administration, nothing more
     */
    void LinkTo( CNotifier* notifier );

    /*
     *  Simply updates the observer administration, nothing more
     */
    void UnlinkFrom( CNotifier* notifier );

    void SubscribeToImp( CNotifier* notifier                 ,
                         const CEvent& eventid               ,
                         CIEventHandlerFunctorBase* callback );

    private:

    typedef GUCEF::set< CNotifier*, std::less< CNotifier* >, gucef_allocator< CNotifier* > > TNotifierList;

    TNotifierList m_notifiers;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_COBSERVER_H ? */
