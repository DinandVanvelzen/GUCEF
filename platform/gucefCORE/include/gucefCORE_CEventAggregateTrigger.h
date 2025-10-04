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

#ifndef GUCEF_CORE_CEVENTAGGREGATETRIGGER_H
#define GUCEF_CORE_CEVENTAGGREGATETRIGGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"          
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

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
 * @class CEventAggregateTrigger
 * @brief This class is used to aggregate multiple events and trigger a single event when all criterea are met.
 * @note This class is thread-safe.
 */
class GUCEF_CORE_PUBLIC_CPP CEventAggregateTrigger : public CTSGNotifier
{
    public:

    static const CEvent AggregateTriggerEvent;

    static void RegisterEvents( void );

    bool AddEventToTriggerCriterea( const CEvent& eventId );

    bool SubscribeAndAddEventToTriggerCriterea( CINotifier* notifier, const CEvent& eventId );

    bool RemoveEventFromTriggerCriterea( const CEvent& eventId );

    void CheckForTriggerCriterea( void );

    void SetOnlyTriggerOnce( bool onlyTriggerOnce );

    bool GetOnlyTriggerOnce( void ) const;

    CEventAggregateTrigger( void );
    CEventAggregateTrigger( PulseGeneratorPtr pulseGenerator );
    CEventAggregateTrigger( const CEventAggregateTrigger& src );
    virtual ~CEventAggregateTrigger();

    private:

    void
    OnTriggerCritereaEvent( CNotifier* notifier    ,
                            const CEvent& eventId  ,
                            CICloneable* eventData );

    private:
    
    typedef GUCEF::map< CEvent, bool >                            TEventCritereaMap;
    typedef CTEventHandlerFunctor< CEventAggregateTrigger >     TEventCallback;

    TEventCritereaMap m_eventCritereaMap;
    bool m_isTriggered;
    bool m_onlyTriggerOnce;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CEVENTAGGREGATETRIGGER_H ? */
