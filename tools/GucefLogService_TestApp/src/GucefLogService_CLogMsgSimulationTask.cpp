/*
 *  GucefLogService_TestApp: Test application for the logging service for GUCEF
 *
 *  Copyright (C) 2002 - 2009.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "GucefLogService_CLogMsgSimulationTask.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace LOGSERVICE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CLogMsgSimulationTask::TaskType = "LogMsgSimulation";

/*---------------------------------------------------------------------------*/

CLogMsgSimulationTask::CLogMsgSimulationTask( void )
    : CORE::CTaskConsumer() ,
      m_cycleCounter( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLogMsgSimulationTask::~CLogMsgSimulationTask()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CLogMsgSimulationTask::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLogMsgSimulationTask::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::LOGSERVICE::CLogMsgSimulationTask";    
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CLogMsgSimulationTask::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_CRITICAL, "LogMsgSimulationTask: starting task" );

    m_cycleCounter = 0;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLogMsgSimulationTask::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    CORE::CString cycleCountStr = CORE::UInt32ToString( m_cycleCounter+1 );
    
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_EVERYTHING, "LogMsgSimulationTask: GUCEF_ERROR_LOG CORE::LOGLEVEL_EVERYTHING " + cycleCountStr );
    GUCEF_LOG( CORE::LOGLEVEL_EVERYTHING, "LogMsgSimulationTask: GUCEF_LOG CORE::LOGLEVEL_EVERYTHING " + cycleCountStr );
    GUCEF_STANDARD_LOG( CORE::LOGLEVEL_EVERYTHING, "LogMsgSimulationTask: GUCEF_STANDARD_LOG CORE::LOGLEVEL_EVERYTHING " + cycleCountStr );
    GUCEF_USER_LOG( CORE::LOGLEVEL_EVERYTHING, "LogMsgSimulationTask: GUCEF_USER_LOG CORE::LOGLEVEL_EVERYTHING " + cycleCountStr );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_EVERYTHING, "LogMsgSimulationTask: GUCEF_SYSTEM_LOG CORE::LOGLEVEL_EVERYTHING " + cycleCountStr );
    GUCEF_DEV_LOG( CORE::LOGLEVEL_EVERYTHING, "LogMsgSimulationTask: GUCEF_DEV_LOG CORE::LOGLEVEL_EVERYTHING " + cycleCountStr );
    GUCEF_SERVICE_LOG( CORE::LOGLEVEL_EVERYTHING, "LogMsgSimulationTask: GUCEF_SERVICE_LOG CORE::LOGLEVEL_EVERYTHING " + cycleCountStr );
    GUCEF_PROTECTED_LOG( CORE::LOGLEVEL_EVERYTHING, "LogMsgSimulationTask: GUCEF_PROTECTED_LOG CORE::LOGLEVEL_EVERYTHING " + cycleCountStr );
    GUCEF_CALLSTACK_LOG( CORE::LOGLEVEL_EVERYTHING, "LogMsgSimulationTask: GUCEF_CALLSTACK_LOG CORE::LOGLEVEL_EVERYTHING " + cycleCountStr );
    GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_EVERYTHING, "LogMsgSimulationTask: GUCEF_EXCEPTION_LOG CORE::LOGLEVEL_EVERYTHING " + cycleCountStr );
    GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_EVERYTHING, "LogMsgSimulationTask: GUCEF_CONSOLE_LOG CORE::LOGLEVEL_EVERYTHING " + cycleCountStr );
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_EVERYTHING, "LogMsgSimulationTask: GUCEF_WARNING_LOG CORE::LOGLEVEL_EVERYTHING " + cycleCountStr );
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "LogMsgSimulationTask: GUCEF_DEBUG_LOG CORE::LOGLEVEL_EVERYTHING " + cycleCountStr );

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "LogMsgSimulationTask: GUCEF_ERROR_LOG CORE::LOGLEVEL_BELOW_NORMAL " + cycleCountStr );
    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "LogMsgSimulationTask: GUCEF_LOG CORE::LOGLEVEL_BELOW_NORMAL " + cycleCountStr );
    GUCEF_STANDARD_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "LogMsgSimulationTask: GUCEF_STANDARD_LOG CORE::LOGLEVEL_BELOW_NORMAL " + cycleCountStr );
    GUCEF_USER_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "LogMsgSimulationTask: GUCEF_USER_LOG CORE::LOGLEVEL_BELOW_NORMAL " + cycleCountStr );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "LogMsgSimulationTask: GUCEF_SYSTEM_LOG CORE::LOGLEVEL_BELOW_NORMAL " + cycleCountStr );
    GUCEF_DEV_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "LogMsgSimulationTask: GUCEF_DEV_LOG CORE::LOGLEVEL_BELOW_NORMAL " + cycleCountStr );
    GUCEF_SERVICE_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "LogMsgSimulationTask: GUCEF_SERVICE_LOG CORE::LOGLEVEL_BELOW_NORMAL " + cycleCountStr );
    GUCEF_PROTECTED_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "LogMsgSimulationTask: GUCEF_PROTECTED_LOG CORE::LOGLEVEL_BELOW_NORMAL " + cycleCountStr );
    GUCEF_CALLSTACK_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "LogMsgSimulationTask: GUCEF_CALLSTACK_LOG CORE::LOGLEVEL_BELOW_NORMAL " + cycleCountStr );
    GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "LogMsgSimulationTask: GUCEF_EXCEPTION_LOG CORE::LOGLEVEL_BELOW_NORMAL " + cycleCountStr );
    GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "LogMsgSimulationTask: GUCEF_CONSOLE_LOG CORE::LOGLEVEL_BELOW_NORMAL " + cycleCountStr );
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "LogMsgSimulationTask: GUCEF_WARNING_LOG CORE::LOGLEVEL_BELOW_NORMAL " + cycleCountStr );
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "LogMsgSimulationTask: GUCEF_DEBUG_LOG CORE::LOGLEVEL_BELOW_NORMAL " + cycleCountStr );

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LogMsgSimulationTask: GUCEF_ERROR_LOG CORE::LOGLEVEL_NORMAL " + cycleCountStr );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LogMsgSimulationTask: GUCEF_LOG CORE::LOGLEVEL_NORMAL " + cycleCountStr );
    GUCEF_STANDARD_LOG( CORE::LOGLEVEL_NORMAL, "LogMsgSimulationTask: GUCEF_STANDARD_LOG CORE::LOGLEVEL_NORMAL " + cycleCountStr );
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "LogMsgSimulationTask: GUCEF_USER_LOG CORE::LOGLEVEL_NORMAL " + cycleCountStr );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "LogMsgSimulationTask: GUCEF_SYSTEM_LOG CORE::LOGLEVEL_NORMAL " + cycleCountStr );
    GUCEF_DEV_LOG( CORE::LOGLEVEL_NORMAL, "LogMsgSimulationTask: GUCEF_DEV_LOG CORE::LOGLEVEL_NORMAL " + cycleCountStr );
    GUCEF_SERVICE_LOG( CORE::LOGLEVEL_NORMAL, "LogMsgSimulationTask: GUCEF_SERVICE_LOG CORE::LOGLEVEL_NORMAL " + cycleCountStr );
    GUCEF_PROTECTED_LOG( CORE::LOGLEVEL_NORMAL, "LogMsgSimulationTask: GUCEF_PROTECTED_LOG CORE::LOGLEVEL_NORMAL " + cycleCountStr );
    GUCEF_CALLSTACK_LOG( CORE::LOGLEVEL_NORMAL, "LogMsgSimulationTask: GUCEF_CALLSTACK_LOG CORE::LOGLEVEL_NORMAL " + cycleCountStr );
    GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "LogMsgSimulationTask: GUCEF_EXCEPTION_LOG CORE::LOGLEVEL_NORMAL " + cycleCountStr );
    GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_NORMAL, "LogMsgSimulationTask: GUCEF_CONSOLE_LOG CORE::LOGLEVEL_NORMAL " + cycleCountStr );
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LogMsgSimulationTask: GUCEF_WARNING_LOG CORE::LOGLEVEL_NORMAL " + cycleCountStr );
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "LogMsgSimulationTask: GUCEF_DEBUG_LOG CORE::LOGLEVEL_NORMAL " + cycleCountStr );    

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "LogMsgSimulationTask: GUCEF_ERROR_LOG CORE::LOGLEVEL_IMPORTANT " + cycleCountStr );
    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "LogMsgSimulationTask: GUCEF_LOG CORE::LOGLEVEL_IMPORTANT " + cycleCountStr );
    GUCEF_STANDARD_LOG( CORE::LOGLEVEL_IMPORTANT, "LogMsgSimulationTask: GUCEF_STANDARD_LOG CORE::LOGLEVEL_IMPORTANT " + cycleCountStr );
    GUCEF_USER_LOG( CORE::LOGLEVEL_IMPORTANT, "LogMsgSimulationTask: GUCEF_USER_LOG CORE::LOGLEVEL_IMPORTANT " + cycleCountStr );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_IMPORTANT, "LogMsgSimulationTask: GUCEF_SYSTEM_LOG CORE::LOGLEVEL_IMPORTANT " + cycleCountStr );
    GUCEF_DEV_LOG( CORE::LOGLEVEL_IMPORTANT, "LogMsgSimulationTask: GUCEF_DEV_LOG CORE::LOGLEVEL_IMPORTANT " + cycleCountStr );
    GUCEF_SERVICE_LOG( CORE::LOGLEVEL_IMPORTANT, "LogMsgSimulationTask: GUCEF_SERVICE_LOG CORE::LOGLEVEL_IMPORTANT " + cycleCountStr );
    GUCEF_PROTECTED_LOG( CORE::LOGLEVEL_IMPORTANT, "LogMsgSimulationTask: GUCEF_PROTECTED_LOG CORE::LOGLEVEL_IMPORTANT " + cycleCountStr );
    GUCEF_CALLSTACK_LOG( CORE::LOGLEVEL_IMPORTANT, "LogMsgSimulationTask: GUCEF_CALLSTACK_LOG CORE::LOGLEVEL_IMPORTANT " + cycleCountStr );
    GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "LogMsgSimulationTask: GUCEF_EXCEPTION_LOG CORE::LOGLEVEL_IMPORTANT " + cycleCountStr );
    GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, "LogMsgSimulationTask: GUCEF_CONSOLE_LOG CORE::LOGLEVEL_IMPORTANT " + cycleCountStr );
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "LogMsgSimulationTask: GUCEF_WARNING_LOG CORE::LOGLEVEL_IMPORTANT " + cycleCountStr );
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_IMPORTANT, "LogMsgSimulationTask: GUCEF_DEBUG_LOG CORE::LOGLEVEL_IMPORTANT " + cycleCountStr ); 

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_VERY_IMPORTANT, "LogMsgSimulationTask: GUCEF_ERROR_LOG CORE::LOGLEVEL_VERY_IMPORTANT " + cycleCountStr );
    GUCEF_LOG( CORE::LOGLEVEL_VERY_IMPORTANT, "LogMsgSimulationTask: GUCEF_LOG CORE::LOGLEVEL_VERY_IMPORTANT " + cycleCountStr );
    GUCEF_STANDARD_LOG( CORE::LOGLEVEL_VERY_IMPORTANT, "LogMsgSimulationTask: GUCEF_STANDARD_LOG CORE::LOGLEVEL_VERY_IMPORTANT " + cycleCountStr );
    GUCEF_USER_LOG( CORE::LOGLEVEL_VERY_IMPORTANT, "LogMsgSimulationTask: GUCEF_USER_LOG CORE::LOGLEVEL_VERY_IMPORTANT " + cycleCountStr );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_VERY_IMPORTANT, "LogMsgSimulationTask: GUCEF_SYSTEM_LOG CORE::LOGLEVEL_VERY_IMPORTANT " + cycleCountStr );
    GUCEF_DEV_LOG( CORE::LOGLEVEL_VERY_IMPORTANT, "LogMsgSimulationTask: GUCEF_DEV_LOG CORE::LOGLEVEL_VERY_IMPORTANT " + cycleCountStr );
    GUCEF_SERVICE_LOG( CORE::LOGLEVEL_VERY_IMPORTANT, "LogMsgSimulationTask: GUCEF_SERVICE_LOG CORE::LOGLEVEL_VERY_IMPORTANT " + cycleCountStr );
    GUCEF_PROTECTED_LOG( CORE::LOGLEVEL_VERY_IMPORTANT, "LogMsgSimulationTask: GUCEF_PROTECTED_LOG CORE::LOGLEVEL_VERY_IMPORTANT " + cycleCountStr );
    GUCEF_CALLSTACK_LOG( CORE::LOGLEVEL_VERY_IMPORTANT, "LogMsgSimulationTask: GUCEF_CALLSTACK_LOG CORE::LOGLEVEL_VERY_IMPORTANT " + cycleCountStr );
    GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_VERY_IMPORTANT, "LogMsgSimulationTask: GUCEF_EXCEPTION_LOG CORE::LOGLEVEL_VERY_IMPORTANT " + cycleCountStr );
    GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_VERY_IMPORTANT, "LogMsgSimulationTask: GUCEF_CONSOLE_LOG CORE::LOGLEVEL_VERY_IMPORTANT " + cycleCountStr );
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_VERY_IMPORTANT, "LogMsgSimulationTask: GUCEF_WARNING_LOG CORE::LOGLEVEL_VERY_IMPORTANT " + cycleCountStr );
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_VERY_IMPORTANT, "LogMsgSimulationTask: GUCEF_DEBUG_LOG CORE::LOGLEVEL_VERY_IMPORTANT " + cycleCountStr );

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "LogMsgSimulationTask: GUCEF_ERROR_LOG CORE::LOGLEVEL_CRITICAL " + cycleCountStr );
    GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "LogMsgSimulationTask: GUCEF_LOG CORE::LOGLEVEL_CRITICAL " + cycleCountStr );
    GUCEF_STANDARD_LOG( CORE::LOGLEVEL_CRITICAL, "LogMsgSimulationTask: GUCEF_STANDARD_LOG CORE::LOGLEVEL_CRITICAL " + cycleCountStr );
    GUCEF_USER_LOG( CORE::LOGLEVEL_CRITICAL, "LogMsgSimulationTask: GUCEF_USER_LOG CORE::LOGLEVEL_CRITICAL " + cycleCountStr );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_CRITICAL, "LogMsgSimulationTask: GUCEF_SYSTEM_LOG CORE::LOGLEVEL_CRITICAL " + cycleCountStr );
    GUCEF_DEV_LOG( CORE::LOGLEVEL_CRITICAL, "LogMsgSimulationTask: GUCEF_DEV_LOG CORE::LOGLEVEL_CRITICAL " + cycleCountStr );
    GUCEF_SERVICE_LOG( CORE::LOGLEVEL_CRITICAL, "LogMsgSimulationTask: GUCEF_SERVICE_LOG CORE::LOGLEVEL_CRITICAL " + cycleCountStr );
    GUCEF_PROTECTED_LOG( CORE::LOGLEVEL_CRITICAL, "LogMsgSimulationTask: GUCEF_PROTECTED_LOG CORE::LOGLEVEL_CRITICAL " + cycleCountStr );
    GUCEF_CALLSTACK_LOG( CORE::LOGLEVEL_CRITICAL, "LogMsgSimulationTask: GUCEF_CALLSTACK_LOG CORE::LOGLEVEL_CRITICAL " + cycleCountStr );
    GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, "LogMsgSimulationTask: GUCEF_EXCEPTION_LOG CORE::LOGLEVEL_CRITICAL " + cycleCountStr );
    GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_CRITICAL, "LogMsgSimulationTask: GUCEF_CONSOLE_LOG CORE::LOGLEVEL_CRITICAL " + cycleCountStr );
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_CRITICAL, "LogMsgSimulationTask: GUCEF_WARNING_LOG CORE::LOGLEVEL_CRITICAL " + cycleCountStr );
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_CRITICAL, "LogMsgSimulationTask: GUCEF_DEBUG_LOG CORE::LOGLEVEL_CRITICAL " + cycleCountStr );

    ++m_cycleCounter;

    // This is an infinate task so we always return false
    //  which indicates we are not done.
    return false;
}

/*-------------------------------------------------------------------------*/

void
CLogMsgSimulationTask::OnTaskEnded( CORE::CTaskPtr task  ,
                                      bool wasForced )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_CRITICAL, "LogMsgSimulationTask: ending task" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace LOGSERVICE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/