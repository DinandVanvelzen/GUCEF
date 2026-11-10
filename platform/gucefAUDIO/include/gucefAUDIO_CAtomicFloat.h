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

#ifndef GUCEF_AUDIO_CATOMICFLOAT_H
#define GUCEF_AUDIO_CATOMICFLOAT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

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

/**
 *  A float that one thread can set while another reads it, as used for the settings of sounds (volume, pan)
 *  that the application changes and the audio thread reads. Reading and writing are single atomic operations
 *  and cost no more than a plain access on the platforms GUCEF runs on.
 */
class CAtomicFloat
{
    public:

    explicit CAtomicFloat( Float32 value = 0.0f ) : m_bits( ToBits( value ) ) {}

    Float32 Load( void ) const                      { return FromBits( m_bits.Load( MT::MEMORDER_RELAXED ) ); }
    void Store( Float32 value )                     { m_bits.Store( ToBits( value ), MT::MEMORDER_RELAXED ); }

    /** With an explicit memory order, for values that are published together with other data (see CAudioParameterBlock) */
    Float32 Load( MT::TMemoryOrder order ) const    { return FromBits( m_bits.Load( order ) ); }
    void Store( Float32 value, MT::TMemoryOrder order ) { m_bits.Store( ToBits( value ), order ); }

    private:

    static UInt32 ToBits( Float32 value )           { UInt32 bits; memcpy( &bits, &value, sizeof( bits ) ); return bits; }
    static Float32 FromBits( UInt32 bits )          { Float32 value; memcpy( &value, &bits, sizeof( value ) ); return value; }

    CAtomicFloat( const CAtomicFloat& src );            /**< not implemented */
    CAtomicFloat& operator=( const CAtomicFloat& src ); /**< not implemented */

    MT::CTAtomic< UInt32 > m_bits;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CATOMICFLOAT_H ? */
