/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include "gucefCOMCORE_CIDiscoveryClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CIDiscoveryClient::EntryDiscoveredEvent = "GUCEF::COMCORE::CIDiscoveryClient::EntryDiscoveredEvent";
const CORE::CEvent CIDiscoveryClient::EntryUpdatedEvent = "GUCEF::COMCORE::CIDiscoveryClient::EntryUpdatedEvent";
const CORE::CEvent CIDiscoveryClient::EntryLostEvent = "GUCEF::COMCORE::CIDiscoveryClient::EntryLostEvent";
const CORE::CEvent CIDiscoveryClient::BrowseStartedEvent = "GUCEF::COMCORE::CIDiscoveryClient::BrowseStartedEvent";
const CORE::CEvent CIDiscoveryClient::BrowseStoppedEvent = "GUCEF::COMCORE::CIDiscoveryClient::BrowseStoppedEvent";
const CORE::CEvent CIDiscoveryClient::DiscoveryErrorEvent = "GUCEF::COMCORE::CIDiscoveryClient::DiscoveryErrorEvent";

const CORE::CString CIDiscoveryClient::AnyServiceType = "*";

/*-------------------------------------------------------------------------*/

void
CIDiscoveryClient::RegisterEvents( void )
{GUCEF_TRACE;

    EntryDiscoveredEvent.Initialize();
    EntryUpdatedEvent.Initialize();
    EntryLostEvent.Initialize();
    BrowseStartedEvent.Initialize();
    BrowseStoppedEvent.Initialize();
    DiscoveryErrorEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIDiscoveryClient::CIDiscoveryClient( void )
    : CORE::CITypeNamed()
    , CORE::CObservingNotifier()
    , CORE::CIConfigurable()
    , CIDiscovery()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIDiscoveryClient::CIDiscoveryClient( const CIDiscoveryClient& src )
    : CORE::CITypeNamed( src )
    , CORE::CObservingNotifier( src )
    , CORE::CIConfigurable( src )
    , CIDiscovery( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIDiscoveryClient::Register( CIDiscoveryEntryPtr entry )
{GUCEF_TRACE;

    return Advertise( entry );
}

/*-------------------------------------------------------------------------*/

bool
CIDiscoveryClient::Unregister( CIDiscoveryEntryPtr entry )
{GUCEF_TRACE;

    return Withdraw( entry );
}

/*-------------------------------------------------------------------------*/

CIDiscoveryClient::~CIDiscoveryClient()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CIDiscoveryClient& 
CIDiscoveryClient::operator=( const CIDiscoveryClient& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CObservingNotifier::operator=( src );
    }
    return *this;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
