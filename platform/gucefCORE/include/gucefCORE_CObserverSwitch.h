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

#ifndef GUCEF_CORE_COBSERVERSWITCH_H
#define GUCEF_CORE_COBSERVERSWITCH_H

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
 *  Class that forwards events as if they where send from the notifiers the switch
 *  is subscribed to for the active observer group.
 */
class GUCEF_CORE_PUBLIC_CPP CObserverSwitch : public CObservingNotifier
{
    public:

    CObserverSwitch( void );

    CObserverSwitch( const CObserverSwitch& src );

    virtual ~CObserverSwitch();

    CObserverSwitch& operator=( const CObserverSwitch& src );

    void SetActiveGroup( const CString& groupName );

    const CString& GetActiveGroup( void ) const;

    void AddObserverToGroup( const CString& groupName ,
                             CObserver& observer      );

    void RemoveObserverFromGroup( const CString& groupName ,
                                  CObserver& observer      );

    virtual const CString& GetClassTypeName( void ) const;

    protected:

    /**
     *  Event callback member function.
     *  Implement this in your descending class to handle
     *  notification events.
     *
     *  @param notifier the notifier that sent the notification
     *  @param eventid the unique event id for an event
     *  @param eventdata optional notifier defined userdata
     */
    virtual void OnNotify( CNotifier* notifier           ,
                           const CEvent& eventid         ,
                           CICloneable* eventdata = NULL );

    private:

    typedef GUCEF::set< CObserver* > TObserverSet;
    typedef GUCEF::map< CString, TObserverSet > TObserverGroupMap;

    TObserverGroupMap m_observerGroups;
    CString m_activeGroupName;
    TObserverSet* m_activeGroup;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_COBSERVERSWITCH_H ? */
