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

#ifndef GUCEF_CORE_CSTATEMACHINE_H
#define GUCEF_CORE_CSTATEMACHINE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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

class CErrorStateHandler;
class CMachineState;
class CStateHandler;

/*-------------------------------------------------------------------------*/

/**
 *      State machine implementation.
 *      Once set in motion the state machine will simply go trough the state handing
 *      path created by the user. The path can vary depending on the condition values
 *      that result from the handlers actions.
 *
 *      Everything works with integers for performance reasons.
 *      It is recommended to make an int enum of all your states and condition
 *      values and use those. The value 0 is reserved and should be considdered to have
 *      the following meaning:
 *              - condition value 0 : no condition
 *              - state value 0     : machine default non-state/invalid state
 *
 *      Note that the user remains responible for memory management of
 *      the CStateHandler derived objects. The state machine only uses them,
 *      it does not assume ownership.
 */
class GUCEF_CORE_PUBLIC_CPP CStateMachine
{
        public:

        CStateMachine( void );

        CStateMachine( const CStateMachine& src );

        ~CStateMachine();

        CStateMachine& operator=( const CStateMachine& src );

        bool Start( const UInt32 initialstate );

        void Stop( void );

        /**
         *      Resets the state machine to the initial state
         *      and re-starts the machine. Basicly a Stop() Start()
         */
        bool Reset( void );

        /**
         *
         *
         *      @param tickcount the tick count when the Update process commenced.
         *      @param deltaticks ticks since the last Update process commenced.
         *      @return wheter the handler encountered a fatal error
         */
        bool Update( UInt32 tickcount  ,
                     UInt32 deltaticks );

        bool AddState( UInt32 state           ,
                       CStateHandler* handler );

        bool DelState( UInt32 state );

        bool SetTransition( const UInt32 oldstate       ,
                            const UInt32 newstate       ,
                            const UInt32 conditionvalue );

        bool GetTransition( const UInt32 oldstateid     ,
                            const UInt32 conditionvalue ,
                            UInt32& newstate            );

        UInt32 GetCurrentState( void ) const;

        void SetErrorHandler( CErrorStateHandler* errorhandler );

        void Clear( void );

        private:
        friend class CMachineState;

        CMachineState* LookupStateObj( UInt32 state ) const;

        private:
        friend class CStateHandler;

        void OnStateHandled( CStateHandler* handler ,
                             UInt32 conditionvalue  );

        void OnStateFailure( UInt32 errorvalue          ,
                             const CString& description );

        private:
        UInt32 _initialstate;
        CMachineState* _curstate;
        bool _updatestate;
        GUCEF::vector< CMachineState* > _states;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSTATEMACHINE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 14-05-2005 :
        - Designed and implemented this class.

-----------------------------------------------------------------------------*/
