/*
 *  gucefPUBSUB: GUCEF module providing pub-sub communication facilities
 *
 *  Copyright (C) 1998 - 2022.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefPUBSUB_PubSubRouteTypes.h"    

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
RouteTypeToString( RouteType routeType )
{GUCEF_TRACE;

    switch ( routeType )
    {
        case Disabled:          return "Disabled";
        case Active:            return "Active";
        case Primary:           return "Primary";
        case Failover:          return "Failover";
        case SpilloverBuffer:   return "SpilloverBuffer";
        case DeadLetter:        return "DeadLetter";
        case Persistence:       return "Persistence";
        default:                return CString::Empty;
    }
}

/*-------------------------------------------------------------------------*/

RouteType 
StringToRouteType( const CORE::CString& routeTypeStr )
{GUCEF_TRACE;

    CORE::CString routeTypeStrLc = routeTypeStr.Lowercase();
    if ( "disabled" == routeTypeStrLc ) return Disabled;
    if ( "active" == routeTypeStrLc ) return Active;
    if ( "primary" == routeTypeStrLc ) return Primary;
    if ( "failover" == routeTypeStrLc ) return Failover;
    if ( "spilloverbuffer" == routeTypeStrLc ) return SpilloverBuffer;
    if ( "deadletter" == routeTypeStrLc ) return DeadLetter;
    if ( "persistence" == routeTypeStrLc ) return Persistence;

    // If we get here treat it as an int
    return (RouteType) CORE::StringToInt32( routeTypeStrLc, Disabled );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
