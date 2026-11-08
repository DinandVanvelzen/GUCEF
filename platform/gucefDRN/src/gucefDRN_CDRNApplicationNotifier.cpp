/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#include "gucefDRN_CDRNApplicationNotifier.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNApplicationNotifier::CDRNApplicationNotifier( CDRNNode& node )
    : CORE::CTSGNotifier( CORE::PulseGeneratorPtr(), true, true )
    , m_applicationNode( &node )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNApplicationNotifier::~CDRNApplicationNotifier()
{GUCEF_TRACE;

    m_applicationNode->DiscardApplicationEvents( *this );
}

/*-------------------------------------------------------------------------*/

void
CDRNApplicationNotifier::EmitToApplication( const CORE::CEvent& eventid, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_applicationNode->IsRunningOwnThread() )
        m_applicationNode->PostApplicationEvent( *this, eventid, eventData );
    else
        NotifyObservers( eventid, eventData );
}

/*-------------------------------------------------------------------------*/

void
CDRNApplicationNotifier::DeliverApplicationEvent( const CORE::CEvent& eventid, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( eventid, eventData );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNApplicationNotifier::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNApplicationNotifier";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
