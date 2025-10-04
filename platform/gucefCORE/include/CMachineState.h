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

#ifndef GUCEF_CORE_CMACHINESTATE_H
#define GUCEF_CORE_CMACHINESTATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTATEHANDLER_H
#include "CStateHandler.h"
#define GUCEF_CORE_CSTATEHANDLER_H
#endif /* GUCEF_CORE_CSTATEHANDLER_H ? */

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

struct SStateTransition;

/**
 *      Internally used class for the CStateMachine class.
 *      Represents a system state and stores the state transition 
 *      pathways.
 */
class CMachineState
{
        public:
        
        CMachineState( UInt32 state           ,
                       CStateHandler* handler );

        virtual ~CMachineState();
        
        UInt32 GetState( void ) const;
        
        CStateHandler* GetHandler( void ) const;
        
        bool AddTransition( UInt32 state          ,
                            UInt32 conditionvalue );
        
        /**
         *      Returns the new state if this state is handled
         *      with the given condition.
         *      In case of error 0 is returned.
         *
         *      @param conditionvalue the state handler condition
         *      @return the new state for the given condition
         */
        UInt32 GetTransition( UInt32 conditionvalue );        
        
        private:

        CMachineState( void );
        CMachineState( const CMachineState& src );    
        CMachineState& operator=( const CMachineState& src );    
                
        GUCEF::vector< struct SStateTransition* > _transitions;
        CStateHandler* _handler;
        UInt32 _state;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CMACHINESTATE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 14-05-2005 :
        - Designed and implemented this class.

-----------------------------------------------------------------------------*/
