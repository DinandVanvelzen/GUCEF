/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CREMOTESESSIONEVENTRING_H
#define GUCEF_DRNUTILS_CREMOTESESSIONEVENTRING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CRPCMESSAGE_H
#include "gucefCOMCORE_CRpcMessage.h"
#define GUCEF_COMCORE_CRPCMESSAGE_H
#endif /* GUCEF_COMCORE_CRPCMESSAGE_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The event ring of a client session (gucef.drnutils.session Event messages): the last events with their sequence
 *  numbers (1, 2, ...), bounded by a count and by the encoded bytes, the oldest dropped first. Append() sets the
 *  event's eventSeq (field 1) and atUtcMs (field 3). Not thread safe: the owner (CRemoteServiceHost) holds its lock.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CRemoteSessionEventRing
{
    public:

    static const UInt32 EventSeqField = 1;
    static const UInt32 KindField = 2;
    static const UInt32 AtUtcMsField = 3;

    struct SEvent
    {
        UInt64 eventSeq;
        COMCORE::CRpcMessage message;
        UInt32 bytes;
    };
    typedef GUCEF::vector< SEvent > TEventVector;

    CRemoteSessionEventRing( void );

    /** 0 = no bound for that dimension; applied at the next Append() */
    void SetLimits( UInt32 maxEvents, UInt64 maxBytes );

    /** Gives the event its sequence and time, keeps it, drops the oldest beyond the limits; the sequence */
    UInt64 Append( COMCORE::CRpcMessage& event, UInt64 nowUtcMs );

    /** The sequence the next event gets (the last one given is this minus one); 1 for a new ring */
    UInt64 GetNextSeq( void ) const;

    /** The first sequence still held (GetNextSeq() when empty) */
    UInt64 GetRetainedFrom( void ) const;

    UInt32 GetRetainedCount( void ) const;
    UInt64 GetRetainedBytes( void ) const;

    /** Whether every event from the sequence on is still held (a stream may start there) */
    bool Covers( UInt64 fromSeq ) const;

    /** The event of the sequence; GUCEF_NULL when it is not held */
    const SEvent* Find( UInt64 eventSeq ) const;

    /** Drops every event; the numbering goes on */
    void Clear( void );

    private:

    TEventVector m_events;                  /**< oldest first; a vector with a head index keeps appends cheap */
    UInt32 m_head;
    UInt32 m_maxEvents;
    UInt64 m_maxBytes;
    UInt64 m_bytes;
    UInt64 m_nextSeq;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CREMOTESESSIONEVENTRING_H ? */
