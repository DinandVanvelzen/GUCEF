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

#ifndef GUCEF_AUDIO_CAUDIOPARAMETERBLOCK_H
#define GUCEF_AUDIO_CAUDIOPARAMETERBLOCK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_AUDIO_CATOMICFLOAT_H
#include "gucefAUDIO_CAtomicFloat.h"
#define GUCEF_AUDIO_CATOMICFLOAT_H
#endif /* GUCEF_AUDIO_CATOMICFLOAT_H ? */

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

/**
 *  The parameter values of an effect, set by any thread and read by the audio thread without either waiting for the other.
 *
 *  Writers store the values as atomics and publish them through a sequence counter that is odd while a write is under
 *  way. The reader (one thread, the audio thread) calls Refresh() once per block: when a complete publication happened
 *  since its last refresh it copies all values into its snapshot in one consistent pass, and Read() serves the snapshot.
 *  When a writer is busy at that moment the reader keeps its previous snapshot and picks up the new values one block
 *  later; it never waits and never retries in a loop. Values written by one Set() call, including the multi-value form,
 *  are therefore always seen together.
 *
 *  Writers are serialised by a mutex of the block. That mutex is never taken by the reader.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioParameterBlock
{
    public:

    /** Room for 'count' values, all 0 until set. Allocates */
    explicit CAudioParameterBlock( UInt32 count );

    ~CAudioParameterBlock();

    UInt32 GetCount( void ) const;

    /** Any thread: sets one value and publishes it. Out of range indices are ignored */
    void Set( UInt32 index, Float32 value );

    /** Any thread: sets several values, published together */
    void Set( const UInt32* indices, const Float32* values, UInt32 count );

    /** Any thread: the value last set */
    Float32 Get( UInt32 index ) const;

    /** Any thread: the number of publications so far */
    UInt32 GetPublicationCount( void ) const;

    /**
     *  Reader thread: takes the values of the latest complete publication into the snapshot when there is a newer one
     *  than the snapshot holds. True when the snapshot changed. Never waits
     */
    bool Refresh( void );

    /** Reader thread: the value in the snapshot */
    inline Float32 Read( UInt32 index ) const { return index < m_count ? m_snapshot[ index ] : 0.0f; }

    /** Owner thread, while no reader runs (before the effect is processed): makes the snapshot hold the current values */
    void SynchronizeSnapshot( void );

    private:

    CAudioParameterBlock( const CAudioParameterBlock& src );            /**< not implemented */
    CAudioParameterBlock& operator=( const CAudioParameterBlock& src ); /**< not implemented */

    void BeginWrite( void );
    void EndWrite( void );

    private:

    UInt32 m_count;
    CAtomicFloat* m_values;
    MT::CTAtomic< UInt32 > m_sequence;      /**< odd while a writer is storing, + 2 per publication */
    MT::CMutex m_writeLock;
    UInt32 m_writeSequence;                 /**< the even value the sequence had when the current write began, under m_writeLock */

    // reader side
    Float32* m_snapshot;
    Float32* m_staging;                     /**< a copy is checked here before it becomes the snapshot */
    UInt32 m_snapshotSequence;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOPARAMETERBLOCK_H ? */
