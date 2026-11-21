/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefGUI_CActivityScope.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CActivityScope::CActivityScope( CActivityService* service       ,
                                const CORE::CString& label      ,
                                UInt32 expectedDurationInMs     ,
                                bool determinate                ,
                                bool cancellable                )
    : m_service( service )
    , m_id( 0 )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_service )
        m_id = m_service->Begin( label, expectedDurationInMs, determinate, cancellable );
}

/*-------------------------------------------------------------------------*/

CActivityScope::~CActivityScope()
{GUCEF_TRACE;

    Finish();
}

/*-------------------------------------------------------------------------*/

UInt32
CActivityScope::GetId( void ) const
{GUCEF_TRACE;

    return m_id;
}

/*-------------------------------------------------------------------------*/

void
CActivityScope::SetProgress( Float32 fraction )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_service && 0 != m_id )
        m_service->SetProgress( m_id, fraction );
}

/*-------------------------------------------------------------------------*/

void
CActivityScope::SetLabel( const CORE::CString& label )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_service && 0 != m_id )
        m_service->SetLabel( m_id, label );
}

/*-------------------------------------------------------------------------*/

bool
CActivityScope::IsCancelRequested( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_service && 0 != m_id && m_service->IsCancelRequested( m_id );
}

/*-------------------------------------------------------------------------*/

void
CActivityScope::Finish( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_service && 0 != m_id )
        m_service->End( m_id );
    m_id = 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
