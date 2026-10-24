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

#ifndef GUCEF_DRN_CIDRNSESSIONCOMPONENT_H
#define GUCEF_DRN_CIDRNSESSIONCOMPONENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDRNSession;

/**
 *  What plugs into a session to replicate something over it: the data grids, the streams, bulk transfers.
 *  A component handles the messages of one message class (REPL, STREAM, BULK), is ticked with the session
 *  (Tick() of the session, every node pulse) and hears about members coming and going.
 *  The session hands a message to its component without a lookup.
 */
class GUCEF_DRN_EXPORT_CPP CIDRNSessionComponent
{
    public:

    CIDRNSessionComponent( void );

    virtual ~CIDRNSessionComponent();

    /** The message class (TDRNMessageClass) this component handles */
    virtual UInt8 GetMessageClass( void ) const = 0;

    /** A message of the component's class from the member at the other end of the link */
    virtual void OnSessionMessage( CDRNSession& session, CDRNPeerLink& link, UInt32 fromPeerId, const CDRNLinkMessage& message ) = 0;

    /** Flush what is due */
    virtual void OnSessionTick( CDRNSession& session, UInt64 nodeNow ) = 0;

    virtual void OnMemberJoined( CDRNSession& session, UInt32 peerId );

    virtual void OnMemberLeft( CDRNSession& session, UInt32 peerId );

    virtual void OnSessionClosed( CDRNSession& session );

    private:

    CIDRNSessionComponent( const CIDRNSessionComponent& src );
    CIDRNSessionComponent& operator=( const CIDRNSessionComponent& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CIDRNSESSIONCOMPONENT_H ? */
