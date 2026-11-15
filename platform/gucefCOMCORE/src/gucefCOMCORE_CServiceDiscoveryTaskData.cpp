/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#include "gucefCOMCORE_CServiceDiscoveryTaskData.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CServiceDiscoveryTaskData::CServiceDiscoveryTaskData( void )
    : CORE::CICloneable()
    , settings()
    , candidates()
    , found( false )
    , source()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceDiscoveryTaskData::CServiceDiscoveryTaskData( const CServiceDiscoveryTaskData& src )
    : CORE::CICloneable( src )
    , settings( src.settings )
    , candidates( src.candidates )
    , found( src.found )
    , source( src.source )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceDiscoveryTaskData::~CServiceDiscoveryTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CServiceDiscoveryTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CServiceDiscoveryTaskData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CServiceDiscoveryTaskData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CServiceDiscoveryTaskData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COMCORE::CServiceDiscoveryTaskData";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
