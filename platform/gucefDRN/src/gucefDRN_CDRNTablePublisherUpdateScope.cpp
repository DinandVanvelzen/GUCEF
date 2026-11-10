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

#ifndef GUCEF_DRN_CDRNTABLEPUBLISHER_H
#include "gucefDRN_CDRNTablePublisher.h"
#define GUCEF_DRN_CDRNTABLEPUBLISHER_H
#endif /* GUCEF_DRN_CDRNTABLEPUBLISHER_H ? */

#include "gucefDRN_CDRNTablePublisherUpdateScope.h"

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

CDRNTablePublisherUpdateScope::CDRNTablePublisherUpdateScope( CDRNTablePublisher& publisher )
    : m_publisher( &publisher )
    , m_open( publisher.BeginUpdate() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNTablePublisherUpdateScope::~CDRNTablePublisherUpdateScope()
{GUCEF_TRACE;

    EarlyEnd();
}

/*-------------------------------------------------------------------------*/

bool
CDRNTablePublisherUpdateScope::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTablePublisherUpdateScope::EarlyEnd( void )
{GUCEF_TRACE;

    if ( !m_open )
        return false;
    m_open = false;
    return m_publisher->EndUpdate();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
