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

#ifndef GUCEF_DRN_CDRNREQUESTGATE_H
#define GUCEF_DRN_CDRNREQUESTGATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

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

class CDRNNode;

/**
 *  A budget of calls that may be under way at once, handed out first come, first served: a counting gate with a limit
 *  that can change. Callers that find the budget used up queue in the order they came and are let in one by one as
 *  budget is given back; one that waits too long leaves the queue (Acquire() returns false). A limit of 0 is no
 *  maximum. Thread safe.
 *
 *  Used by CDRNNamespaceClient to stay within the limits a serving member announced (NS_HELLO), so that a requester that
 *  shares the member among several threads or mounts never runs into the member's BUSY refusals during normal use.
 */
class GUCEF_DRN_EXPORT_CPP CDRNRequestGate
{
    public:

    /** The budget to start with, 0 for no maximum */
    explicit CDRNRequestGate( UInt32 limit = 0 );

    ~CDRNRequestGate();

    /** Changes the limit (0 = no maximum); waiting callers are let in when it grew */
    void SetLimit( UInt32 limit );

    UInt32 GetLimit( void ) const;

    /**
     *  Takes one unit of the budget, waiting at most 'timeoutInMs' for its turn. False when the time ran out (the caller
     *  is no longer queued and holds nothing). A true has to be followed by Release().
     */
    bool Acquire( UInt32 timeoutInMs );

    /**
     *  As Acquire(), for a caller that may be the thread that pumps 'node' (CDRNNode::IsPumpThread()): such a thread
     *  works the node while it waits for its turn, as it does for every other wait on the node, because the callers that
     *  hold the budget may wait for answers that only that thread can bring in.
     */
    bool Acquire( UInt32 timeoutInMs, CDRNNode& node );

    /** Gives one unit back, to the first caller in the queue if there is one */
    void Release( void );

    /** Units taken now */
    UInt32 GetInFlight( void ) const;

    /** Callers waiting for their turn now */
    UInt32 GetWaiting( void ) const;

    /** The most units that were taken at once since the gate was made */
    UInt32 GetPeakInFlight( void ) const;

    private:

    class CTicket;
    class CTurnCondition;

    typedef GUCEF::vector< CTicket* > TTicketVector;

    CDRNRequestGate( const CDRNRequestGate& src );
    CDRNRequestGate& operator=( const CDRNRequestGate& src );

    bool Take( void );
    void GrantWaiting( void );
    CTicket* Enter( void );
    bool Leave( CTicket* ticket );

    mutable MT::CMutex m_lock;
    UInt32 m_limit;
    UInt32 m_inFlight;
    UInt32 m_peakInFlight;
    TTicketVector m_queue;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNREQUESTGATE_H ? */
