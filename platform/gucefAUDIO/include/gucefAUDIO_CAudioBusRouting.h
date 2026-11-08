/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#ifndef GUCEF_AUDIO_CAUDIOBUSROUTING_H
#define GUCEF_AUDIO_CAUDIOBUSROUTING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CAudioBus;

/*-------------------------------------------------------------------------*/

/**
 *  Who feeds whom among the buses of a mixer: every bus feeds its parent, and a bus with a send feeds the target of
 *  the send. Kept on the owner side; the audio thread only ever sees the processing order computed from it.
 *
 *  A connection that would make a bus feed itself, directly or through other buses, is refused: a send to the bus
 *  itself, or to any bus that already feeds it (one of its children, or a bus that sends to it or to one of those).
 *  Sends to an ancestor are fine: the audio then reaches the ancestor twice, which is no cycle.
 *
 *  ComputeOrder() gives an order in which every bus comes after all buses that feed it (Kahn's topological sort).
 *  Among the buses that are ready at a step, the one created last goes first, so a plain tree is processed in reverse
 *  creation order: the order the mixer has always used, which keeps the summation order and therefore the output of
 *  existing bus trees bit for bit the same.
 *
 *  Not thread safe: one thread at a time (the engine guards it with its lock).
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioBusRouting
{
    public:

    explicit CAudioBusRouting( UInt32 maxBuses = 64 );

    ~CAudioBusRouting();

    /** Forgets all buses and connections */
    void Clear( void );

    /**
     *  Adds a bus below its parent. The first bus is the root (the master bus) and has no parent; every later one needs a
     *  parent that was added before. False when full, when the bus is known already or the parent is not
     */
    bool AddBus( CAudioBus* bus, CAudioBus* parent );

    /** Takes a bus out again, only one that nothing feeds and that sends nowhere (to undo an AddBus()). The root stays */
    bool RemoveBus( const CAudioBus* bus );

    bool HasBus( const CAudioBus* bus ) const;

    UInt32 GetBusCount( void ) const;

    /** True when audio of 'from' reaches 'to' through parents and sends (a bus does not reach itself unless through a cycle) */
    bool Reaches( const CAudioBus* from, const CAudioBus* to ) const;

    /** True when a send from 'from' to 'to' would be acceptable: both known, not the same, and no cycle would result */
    bool CanAddSend( const CAudioBus* from, const CAudioBus* to ) const;

    /** Adds a send connection. False when CanAddSend() is false. The same pair may be connected more than once */
    bool AddSend( CAudioBus* from, CAudioBus* to );

    /** Removes one send connection between the pair. False when there is none */
    bool RemoveSend( const CAudioBus* from, const CAudioBus* to );

    /**
     *  Writes the processing order into 'order' (room for 'capacity' buses): feeders first, the root last.
     *  False when the room is too small
     */
    bool ComputeOrder( CAudioBus** order, UInt32 capacity, UInt32& count ) const;

    private:

    CAudioBusRouting( const CAudioBusRouting& src );            /**< not implemented */
    CAudioBusRouting& operator=( const CAudioBusRouting& src ); /**< not implemented */

    Int32 IndexOf( const CAudioBus* bus ) const;

    struct SEdge
    {
        UInt32 from;
        UInt32 to;
    };

    private:

    UInt32 m_maxBuses;
    GUCEF::vector< CAudioBus* > m_buses;        /**< in creation order */
    GUCEF::vector< Int32 > m_parents;           /**< index of the parent of each bus, -1 for the root */
    GUCEF::vector< SEdge > m_sends;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOBUSROUTING_H ? */
