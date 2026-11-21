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

#include "gucefDRN_CDRNNamespaceShareInfo.h"

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

CDRNNamespaceShareInfo::CDRNNamespaceShareInfo( void )
    : name()
    , capabilities( 0 )
    , flags( 0 )
    , maxStatPaths( 0 )
    , maxPageEntries( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceShareInfo::CanList( void ) const
{GUCEF_TRACE;

    return 0 != ( capabilities & DRN_NS_CAP_LIST );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceShareInfo::CanStat( void ) const
{GUCEF_TRACE;

    return 0 != ( capabilities & DRN_NS_CAP_STAT );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceShareInfo::IsReadOnly( void ) const
{GUCEF_TRACE;

    return 0 != ( flags & DRN_NS_SHAREFLAG_READ_ONLY );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
