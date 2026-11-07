/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CMCPSUBSCRIPTION_H
#define GUCEF_COM_CMCPSUBSCRIPTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COM_CMCPPRINCIPAL_H
#include "gucefCOM_CMcpPrincipal.h"
#define GUCEF_COM_CMCPPRINCIPAL_H
#endif /* GUCEF_COM_CMCPPRINCIPAL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One subscriptions/listen request: the notifications the server agreed to send. Its id is the JSON-RPC id of the
 *  listen request, which every notification of the subscription carries in _meta. The state is scoped to the request:
 *  it ends with the request, the channel or the server. Lives on the protocol thread of the server only.
 */
class GUCEF_COM_PUBLIC_CPP CMcpSubscription
{
    public:

    CORE::CVariant id;                              /**< the id of the listen request: a string or an integer */
    CORE::CString idKey;                            /**< the id as a key that tells "1" and 1 apart */
    UInt64 channelId;
    CMcpPrincipal principal;
    bool toolsListChanged;
    bool promptsListChanged;
    bool resourcesListChanged;
    CORE::CString::StringSet resourceSubscriptions;
    bool hadResourceSubscriptions;                  /**< the request asked for resource updates */

    CMcpSubscription( void );

    /** Writes the SubscriptionFilter the server honours, for notifications/subscriptions/acknowledged */
    void FilterToDataNode( CORE::CDataNode& filter ) const;

    bool IsSubscribedToResource( const CORE::CString& uri ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPSUBSCRIPTION_H ? */
