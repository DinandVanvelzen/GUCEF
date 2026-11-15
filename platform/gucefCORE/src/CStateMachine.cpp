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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CMACHINESTATE_H
#include "CMachineState.h"
#define GUCEF_CORE_CMACHINESTATE_H
#endif /* GUCEF_CORE_CMACHINESTATE_H ? */

#ifndef GUCEF_CORE_CSTATEHANDLER_H
#include "CStateHandler.h"
#define GUCEF_CORE_CSTATEHANDLER_H
#endif /* GUCEF_CORE_CSTATEHANDLER_H ? */

#include "CStateMachine.h"

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

CStateMachine::CStateMachine( void )
        : _initialstate( 0 ) ,
          _curstate( 0 )     ,
          _updatestate( true )
{
}

/*-------------------------------------------------------------------------*/

CStateMachine::CStateMachine( const CStateMachine& src )
        : _initialstate( src._initialstate ) 
{
}

/*-------------------------------------------------------------------------*/

CStateMachine::~CStateMachine()
{
}

/*-------------------------------------------------------------------------*/

CStateMachine& 
CStateMachine::operator=( const CStateMachine& src )
{
        return *this;
}

/*-------------------------------------------------------------------------*/

bool 
CStateMachine::Start( const UInt32 initialstate )
{
        if ( !_curstate )
        {
                _initialstate = initialstate;
                
                /*
                 *      Notify each handler of the machine start event and make sure
                 *      that each is notified only once.
                 */                
                CStateHandler* handler;
                for ( UInt32 i=0; i<_states.size(); ++i )
                {
                        handler = _states[ i ]->GetHandler();
                        if ( !handler->_mstarted )
                        {
                                handler->_mstarted = true;
                                handler->OnMachineStart();
                        }
                }                
                
                /*
                 *      Now we get busy,.. enter the initial state.
                 *      The handlers will do the rest.
                 */
                _curstate = LookupStateObj( initialstate );
                if ( _curstate )
                {
                        if ( _curstate->GetHandler()->OnEnterState() )
                        {                        
                                return true;
                        }
                        Stop();
                        return false;
                }
                return false;                                         
        }
        return false;                                
}

/*-------------------------------------------------------------------------*/
        
void 
CStateMachine::Stop( void )
{
        if ( _curstate )
        {      
                CStateHandler* handler;
                for ( UInt32 i=0; i<_states.size(); ++i )
                {
                        handler = _states[ i ]->GetHandler();
                        if ( handler->_mstarted )
                        {
                                handler->_mstarted = false;
                                handler->OnMachineStop();
                        }
                }
                
                _curstate = GUCEF_NULL;
        }                
}

/*-------------------------------------------------------------------------*/

bool
CStateMachine::Reset( void )
{
        Stop();
        return Start( _initialstate );        
}

/*-------------------------------------------------------------------------*/                              
                            
UInt32 
CStateMachine::GetCurrentState( void ) const
{
        if ( _curstate )
        {
                return _curstate->GetState();
        }
        return 0;                
}

/*-------------------------------------------------------------------------*/
                            
void 
CStateMachine::SetErrorHandler( CErrorStateHandler* errorhandler )
{
}

/*-------------------------------------------------------------------------*/
                            
void 
CStateMachine::Clear( void )
{
        for ( UInt32 i=0; i<_states.size(); ++i )
        {
                GUCEF_DELETE _states[ i ];
        }
        _states.clear();
}

/*-------------------------------------------------------------------------*/
                              
bool 
CStateMachine::Update( UInt32 tickcount  ,
                       UInt32 deltaticks )
{
        if ( _curstate )
        {
        }
        return false;
}                       

/*-------------------------------------------------------------------------*/
                                    
void 
CStateMachine::OnStateHandled( CStateHandler* handler ,
                               UInt32 conditionvalue  )
{
        if ( handler == _curstate->GetHandler() )
        {
                handler->OnLeaveState();

                UInt32 newstate = _curstate->GetTransition( conditionvalue );
                if ( newstate )
                {
                        _curstate = LookupStateObj( newstate );
                        _curstate->GetHandler()->OnEnterState();
                        return;                
                }
                Stop();
                return;
        }                        
}

/*-------------------------------------------------------------------------*/                               
                             
void 
CStateMachine::OnStateFailure( UInt32 errorvalue          ,
                               const CString& description )
{
}

/*-------------------------------------------------------------------------*/

CMachineState* 
CStateMachine::LookupStateObj( UInt32 state ) const
{
        CMachineState* mstate;
        for ( UInt32 i=0; i<_states.size(); ++i )
        {       
                mstate = _states[ i ];
                if ( mstate->GetState() == state )
                {
                        return mstate;
                }                
        }
        return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool 
CStateMachine::AddState( UInt32 state           ,
                         CStateHandler* handler )
{
        if ( !_curstate && handler )
        {
                if ( !LookupStateObj( state ) )
                {
                        handler->_machine = this;
                        _states.push_back( GUCEF_NEW CMachineState( state, handler ) );
                        return true;
                }
        }                
        return false;                
}                         

/*-------------------------------------------------------------------------*/
        
bool 
CStateMachine::DelState( UInt32 state )
{
        CMachineState* mstate;
        for ( UInt32 i=0; i<_states.size(); ++i )
        {       
                mstate = _states[ i ];
                if ( mstate->GetState() == state )
                {
                    _states[ i ] = GUCEF_NULL;
                    GUCEF_DELETE mstate;
                    mstate = 0;
                    //_states.FillNULLGaps(); @TODO
                    return true;
                }                
        }
        return false;
}

/*-------------------------------------------------------------------------*/

bool 
CStateMachine::SetTransition( const UInt32 oldstate       ,
                              const UInt32 newstate       ,
                              const UInt32 conditionvalue )
{
        CMachineState* state( LookupStateObj( oldstate ) );
        if ( state )
        { 
                return state->AddTransition( newstate, conditionvalue );                
        }                
        return false;
}

/*-------------------------------------------------------------------------*/

bool 
CStateMachine::GetTransition( const UInt32 oldstate       ,
                              const UInt32 conditionvalue ,
                              UInt32& newstate            )
{
        CMachineState* state( LookupStateObj( oldstate ) );
        if ( state )
        { 
                newstate = state->GetTransition( conditionvalue );
                return true;
        }
        return false;         
}                           

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
                               