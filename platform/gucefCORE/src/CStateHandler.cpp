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

#include "CStateHandler.h"

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

CStateHandler::CStateHandler( void )
        : _machine( GUCEF_NULL )
{

}

/*-------------------------------------------------------------------------*/
                          
CStateHandler::CStateHandler( const CStateHandler& src )
        : _machine( src._machine )
{
        
}

/*-------------------------------------------------------------------------*/

CStateHandler::~CStateHandler()
{
}

/*-------------------------------------------------------------------------*/

CStateHandler& 
CStateHandler::operator=( const CStateHandler& src )
{
        if ( this != &src )
        {

        }                
        return *this;
}

/*-------------------------------------------------------------------------*/
        
bool 
CStateHandler::OnMachineStart( void )
{
        return true;
}

/*-------------------------------------------------------------------------*/
        
void 
CStateHandler::OnMachineStop( void )
{

}

/*-------------------------------------------------------------------------*/

void 
CStateHandler::OnLeaveState( void )
{
        
}

/*-------------------------------------------------------------------------*/

bool 
CStateHandler::OnUpdate( UInt32 tickcount  ,
                         UInt32 deltaticks )
{
        SignalStateHandled( 0 );
        return true;
}                         

/*-------------------------------------------------------------------------*/
        
void 
CStateHandler::SignalStateHandled( UInt32 conditionvalue )
{
        _machine->OnStateHandled( this, conditionvalue );
}

/*-------------------------------------------------------------------------*/
        
void 
CStateHandler::SignalStateFailure( UInt32 errorvalue          ,
                                   const CString& description )
{
}

/*-------------------------------------------------------------------------*/

UInt32
CStateHandler::GetState( void ) const
{
        return _machine->GetCurrentState();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
