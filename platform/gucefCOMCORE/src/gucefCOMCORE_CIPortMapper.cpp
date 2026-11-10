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

#include "gucefCOMCORE_CIPortMapper.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CIPortMapper::MappingCreatedEvent = "GUCEF::COMCORE::CIPortMapper::MappingCreatedEvent";
const CORE::CEvent CIPortMapper::MappingLostEvent = "GUCEF::COMCORE::CIPortMapper::MappingLostEvent";
const CORE::CEvent CIPortMapper::PortMapperErrorEvent = "GUCEF::COMCORE::CIPortMapper::PortMapperErrorEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CIPortMapper::RegisterEvents( void )
{GUCEF_TRACE;

    MappingCreatedEvent.Initialize();
    MappingLostEvent.Initialize();
    PortMapperErrorEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIPortMapper::CIPortMapper( void )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIPortMapper::~CIPortMapper()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
