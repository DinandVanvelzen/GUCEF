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

#ifndef GUCEF_CORE_CSTATEMACHINECOMPONENT_H
#define GUCEF_CORE_CSTATEMACHINECOMPONENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>
#include "CObservingNotifier.h"

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
 *  One of the essential building blocks for realtime applications.
 *  This class implements the 'state memory' and the decending class the
 *  transition handling based on a defined state transition table that
 *  can be copied from an STD (State Transition Diagram).
 */
class GUCEF_CORE_PUBLIC_CPP CStateMachineComponent : public CObservingNotifier
{
    public:

    explicit CStateMachineComponent( const UInt32 initialState );

    explicit CStateMachineComponent( const CStateMachineComponent& src );

    CStateMachineComponent& operator=( const CStateMachineComponent& src );

    virtual ~CStateMachineComponent();

    UInt32 GetIDOfCurrentState( void ) const;

    protected:

    virtual void OnStateTransition( const UInt32 oldStateID       ,
                                    const UInt32 newStateID       ,
                                    const UInt32 triggerEventID   ,
                                    CNotifier* triggerNotifier    ,
                                    CICloneable* eventdata = NULL ) = 0;

    void AddTransition( const UInt32 currentStateID ,
                        const UInt32 newStateID     ,
                        const UInt32 triggerEventID );

    bool DeleteTransition( const UInt32 currentStateID ,
                           const UInt32 triggerEventID );

    /**
     *  Event notification handler. Do not override.
     *  It processes the event id's and based on the contents of the
     *  transition table either ignores the event or changes the state.
     *  If the case of a state change OnStateTransition() is called with
     *  all relevant information.
     *
     *  @param notifier the notifier that sent the notification
     *  @param eventid the unique event id for an event
     *  @param eventdata optional notifier defined userdata
     */
    virtual void OnNotify( CNotifier* notifier           ,
                           const UInt32 eventid          ,
                           CICloneable* eventdata = NULL );

    private:

    CStateMachineComponent( void );

    private:
    typedef GUCEF::map< UInt32, UInt32 > TEventTransitions;
    typedef GUCEF::map< UInt32, TEventTransitions > TTransitionTable;

    TTransitionTable m_transitions;
    UInt32 m_currentState;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSTATEMACHINECOMPONENT_H  ? */
