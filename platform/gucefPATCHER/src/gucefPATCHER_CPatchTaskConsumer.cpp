/*
 *  gucefPATCHER: GUCEF RAD module providing a patch delivery system
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */
 
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PATCHER_CPATCHTASKDATA_H
#include "gucefPATCHER_CPatchTaskData.h"
#define GUCEF_PATCHER_CPATCHTASKDATA_H
#endif /* GUCEF_PATCHER_CPATCHTASKDATA_H ? */

#ifndef GUCEF_PATCHER_CPATCHMANAGER_H
#include "gucefPATCHER_CPatchManager.h"
#define GUCEF_PATCHER_CPATCHMANAGER_H
#endif /* GUCEF_PATCHER_CPATCHMANAGER_H ? */

#include "gucefPATCHER_CPatchTaskConsumer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PATCHER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CPatchTaskConsumer::CPatchTaskConsumer( void )
    : CTaskConsumer()       ,
      m_patchEngine( NULL ) ,
      m_taskName()          
{GUCEF_TRACE;

    m_patchEngine = new CPatchEngine( GetPulseGenerator() );
}

/*-------------------------------------------------------------------------*/

CPatchTaskConsumer::~CPatchTaskConsumer()
{GUCEF_TRACE;

    delete m_patchEngine;
    m_patchEngine = NULL;
}

/*-------------------------------------------------------------------------*/

CString
CPatchTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return GetTypeString();
}

/*-------------------------------------------------------------------------*/

const CString&
CPatchTaskConsumer::GetTypeString( void )
{GUCEF_TRACE;

    static CString typeName = "GUCEF::PATCHER::CPatchTaskConsumer"; 
    return typeName;
}

/*-------------------------------------------------------------------------*/

bool
CPatchTaskConsumer::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

void
CPatchTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool forced )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
bool
CPatchTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;

    CORE::CICloneable* opaqueTaskData = task->GetTaskData();
    CPatchTaskData* ptData = static_cast< CPatchTaskData* >( opaqueTaskData );
    m_taskName = ptData->GetTaskName();    
    if ( m_patchEngine->GetConfig().LoadConfig( ptData->GetPatchEngineConfig() ) )
    {
        ptData->GetPatchManager().RegisterTask( this );        
        ptData->GetPatchManager().SubscribeTo( m_patchEngine ); 

        m_patchEngine->Start();

        ptData->GetPatchManager().UnsubscribeFrom( m_patchEngine );
        ptData->GetPatchManager().UnregisterTask( this );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CPatchTaskConsumer::GetTaskName( void ) const
{GUCEF_TRACE;

    return m_taskName;
}

/*-------------------------------------------------------------------------*/

const CString& 
CPatchTaskConsumer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::PATCHER::CPatchTaskConsumer";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CPatchEngine&
CPatchTaskConsumer::GetPatchEngine( void )
{GUCEF_TRACE;

    return *m_patchEngine;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PATCHER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
