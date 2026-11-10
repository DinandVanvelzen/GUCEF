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

#ifndef GUCEF_PUBSUB_PUBSUBROUTETYPES_H
#define GUCEF_PUBSUB_PUBSUBROUTETYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PUBSUB_MACROS_H
#include "gucefPUBSUB_macros.h"    
#define GUCEF_PUBSUB_MACROS_H
#endif /* GUCEF_PUBSUB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

enum RouteType : CORE::Int32
{
    Disabled        = 0 , /**< not a runtime active route */
    Active          = 1 , /**< the active route this varies between the available routes and the logic applied */

    Primary         = 2 , /**< regular flow under normal conditions */
    Failover        = 3 , /**< if the primary flow fails traffic would be rerouted here as a equivelant */ 
    SpilloverBuffer = 4 , /**< if the primary is unhealthy or is a slow consumer the spill over acts as buffer for the primary route */
    DeadLetter      = 5 , /**< unable to publish on configured channels and remedies exhausted messages go here */
    Persistence     = 6   /**< always receives a copy of all messages flowing through the route for durable storage and replay support */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_PUBSUB_EXPORT_CPP
CORE::CString RouteTypeToString( RouteType routeType );

inline CORE::CString ToString( RouteType routeType ) { return RouteTypeToString( routeType ); }

GUCEF_PUBSUB_EXPORT_CPP RouteType 
StringToRouteType( const CORE::CString& routeTypeStr );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PUBSUB_PUBSUBROUTETYPES_H ? */

