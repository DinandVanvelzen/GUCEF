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

#ifndef GUCEF_CORE_CFORWARDINGNOTIFIER_H
#define GUCEF_CORE_CFORWARDINGNOTIFIER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <set>
#include <map>

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
 *  Adds a event forwarding mechanism to the observing notifier functionality
 */
class GUCEF_CORE_PUBLIC_CPP CForwardingNotifier : public CObservingNotifier
{
    public:

    enum EEventOriginFilter
    {
        EVENTORIGINFILTER_UNMODIFIED , /**< do not change the event emission origin */
        EVENTORIGINFILTER_TRANSFER     /**< turns the re-sender into the emitter of the event */
    };
    typedef enum EEventOriginFilter TEventOriginFilter;

    CForwardingNotifier( void );

    CForwardingNotifier( const CForwardingNotifier& src );

    virtual ~CForwardingNotifier();

    CForwardingNotifier& operator=( const CForwardingNotifier& src );

    bool SetForwardingActiveStateForAllEvents( CNotifier& notifier ,
                                               const bool enabled  );

    bool SetForwardingActiveStateForEvent( const CEvent& eventid ,
                                           CNotifier* notifier   ,
                                           const bool enabled    );

    /**
     *  This allows you to forward all events from one notifier
     *  trough this notifier. This is useful if you have a
     *  notifier who's derived class delegates tasks to
     *  notifying sub-systems. Using forwarding you can create
     *  a composite class and make it seem like a single system
     *  to the observer.
     *
     *  @param notifier the notifier for which you want all events forwarded
     *  @param originFilter allows you to mutate the sending notifier if so desired
     */
    void AddForwardingForAllEvents( CNotifier& notifier                   ,
                                    const TEventOriginFilter originFilter ,
                                    const bool enabled = true             );

    /**
     *  Adds the given event ID to the forwarding list.
     *
     *  This allows you to forward events from one notifier
     *  trough this notifier. This is useful if you have a
     *  notifier who's derived class delegates tasks to
     *  notifying sub-systems. Using forwarding you can create
     *  a composite class and make it seem like a single system
     *  to the observer.
     *
     *  Note that the instance of this class will become the sending
     *  notifier instead of the original notifier. As such it is recommended
     *  to create a event declaration interface from which both your exposed
     *  class and potential sub-system classes derive. This allows you
     *  to use forwarding without breaking event scoping rules.
     *
     *  Note that if "notifier" is NULL you must use the filter type
     *  EVENTORIGINFILTER_TRANSFER or the you wont be able to add the
     *  forwarding mapping.
     *
     *  @param eventid the ID specifying the event you wish to forward
     *  @param notifier if non-NULL only event's triggered from the
     *  @param notifier given notifier are forwarded, otherwise the
     *  @param notifier event is always forwarded
     */
    bool AddForwardingForEvent( const CEvent& eventid                 ,
                                const TEventOriginFilter originFilter ,
                                CNotifier* notifier = NULL            ,
                                const bool enabled = true             );

    /**
     *  Removes the given event ID to the forwarding list.
     *
     *  See AddEventForwarding() for more details
     *
     *  @param eventid the ID specifying the event you wish to remove from forwarding
     *  @param notifier if non-NULL only event's triggered from the
     *  @param notifier given notifier are removed from the forwarding list,
     *  @param notifier otherwise the event is always forwarded
     */
    void RemoveForwardingForEvent( const CEvent& eventid      ,
                                   CNotifier* notifier = NULL );

    /**
     *  Removes all references to the given notifier from our forwarding
     *  system
     */
    void RemoveForwardingForAllEvents( CNotifier& notifier );

    /**
     *  Removes all references to the given notifier from our forwarding
     *  system
     */
    void RemoveAllForwardingForNotifier( CNotifier& notifier );

    virtual const CString& GetClassTypeName( void ) const;

    protected:

    /**
     *  Event callback member function.
     *  Implement this in your descending class to handle
     *  notification events.
     *
     *  Note: Do NOT  forget to call this class's implementation
     *  from your descending class or you will break the event
     *  forwarding mechanism.
     *
     *  @param notifier the notifier that sent the notification
     *  @param eventid the unique event id for an event
     *  @param eventdata optional notifier defined userdata
     */
    virtual void OnNotify( CNotifier* notifier           ,
                           const CEvent& eventid         ,
                           CICloneable* eventdata = NULL );

    private:
    struct SForwardState
    {
        TEventOriginFilter filter;
        bool enabled;
    };
    typedef struct SForwardState TForwardState;

    typedef GUCEF::map< CNotifier*, TForwardState > TNotifierMap;
    typedef GUCEF::map< CEvent, TNotifierMap > TEventNotifierMap;
    typedef GUCEF::map< CEvent, TForwardState > TEventMap;

    TEventMap m_forwardAllList;
    TNotifierMap m_forwardAllForNotifierList;
    TEventNotifierMap m_eventNotifierMap;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CFORWARDINGNOTIFIER_H ? */
