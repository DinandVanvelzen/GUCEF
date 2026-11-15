/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

#ifndef GUCEF_MATH_CAMBISONICFORMAT_H
#define GUCEF_MATH_CAMBISONICFORMAT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTSPHERICALHARMONICS_H
#include "gucefMATH_CTSphericalHarmonics.h"
#define GUCEF_MATH_CTSPHERICALHARMONICS_H
#endif /* GUCEF_MATH_CTSPHERICALHARMONICS_H ? */

#ifndef GUCEF_MATH_CTRADIAN_H
#include "gucefMATH_CTRadian.h"
#define GUCEF_MATH_CTRADIAN_H
#endif /* GUCEF_MATH_CTRADIAN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The order of the channels of an ambisonic signal */
enum EAmbisonicChannelOrder
{
    AMBIORDER_ACN = 0 ,     /**< Ambisonic Channel Number: channel l^2 + l + m (AmbiX) */
    AMBIORDER_FUMA          /**< Furse-Malham: W X Y Z R S T U V K L M N O P Q, up to order 3 */
};
typedef enum EAmbisonicChannelOrder TAmbisonicChannelOrder;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Ambisonic channel orders, normalisations and the encoding of a direction.
 *
 *  - Frame and basis: the spherical harmonics of CTSphericalHarmonicsBasis in the AmbiX frame: x front, y left, z up,
 *    azimuth positive to the left (counter-clockwise seen from above), elevation positive up. ACN 1 is Y (left), ACN 2
 *    Z, ACN 3 X. FuMa X, Y, Z are ACN 3, 1, 2; FuMa W is SN3D W / sqrt( 2 ).
 *  - Listener conventions of GUCEF (gucefAUDIO: right-handed, +Y up, -Z forward, azimuth +90 degrees = right) are
 *    converted explicitly: EncodeDirection takes the listener azimuth (positive right) and negates it into the AmbiX
 *    azimuth; EncodeListenerDirection takes a direction in listener space (x right, y up, -z forward).
 *  - Orders up to GUCEF_MATH_SH_MAX_ORDER (15); FuMa (channel order or normalisation) up to order 3. An unsupported
 *    combination asserts in debug builds and returns false without writing.
 *  - Float32 and Float64 overloads; no state. Every function is realtime-safe (no allocation; the conversions use a
 *    stack buffer of at most 256 values, so input and output may be the same memory).
 */
class GUCEF_MATH_PUBLIC_CPP CAmbisonicFormat
{
    public:

    enum
    {
        MAX_ORDER = GUCEF_MATH_SH_MAX_ORDER ,
        FUMA_MAX_ORDER = GUCEF_MATH_SH_FUMA_MAX_ORDER ,
        MAX_CHANNELS = ( GUCEF_MATH_SH_MAX_ORDER + 1 ) * ( GUCEF_MATH_SH_MAX_ORDER + 1 )
    };

    /** ( order + 1 )^2 */
    static UInt32 GetChannelCount( UInt32 order ) GUCEF_MATH_REALTIME_CONTEXT;

    /** True when 'order' is supported with the given channel order and normalisation */
    static bool IsSupported( TAmbisonicChannelOrder channelOrder, TShNormalisation norm, UInt32 order ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The ACN index of channel 'channel' in 'channelOrder'; the channel itself for ACN or beyond the FuMa channels */
    static UInt32 GetAcnIndex( UInt32 channel, TAmbisonicChannelOrder channelOrder ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The channel of ACN index 'acn' in 'channelOrder' (the inverse of GetAcnIndex) */
    static UInt32 GetChannelIndex( UInt32 acn, TAmbisonicChannelOrder channelOrder ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  Converts one frame of ( order + 1 )^2 channels between channel orders and normalisations; in and out may be the
     *  same memory. False (out unchanged) for an unsupported combination.
     */
    static bool ConvertChannels( const Float32* in, TAmbisonicChannelOrder inOrder, TShNormalisation inNorm,
                                 Float32* out, TAmbisonicChannelOrder outOrder, TShNormalisation outNorm, UInt32 order ) GUCEF_MATH_REALTIME_CONTEXT;

    static bool ConvertChannels( const Float64* in, TAmbisonicChannelOrder inOrder, TShNormalisation inNorm,
                                 Float64* out, TAmbisonicChannelOrder outOrder, TShNormalisation outNorm, UInt32 order ) GUCEF_MATH_REALTIME_CONTEXT;

    /** ConvertChannels for 'frameCount' interleaved frames; in and out may be the same memory */
    static bool ConvertBlock( const Float32* in, TAmbisonicChannelOrder inOrder, TShNormalisation inNorm,
                              Float32* out, TAmbisonicChannelOrder outOrder, TShNormalisation outNorm, UInt32 order, UInt32 frameCount ) GUCEF_MATH_REALTIME_CONTEXT;

    static bool ConvertBlock( const Float64* in, TAmbisonicChannelOrder inOrder, TShNormalisation inNorm,
                              Float64* out, TAmbisonicChannelOrder outOrder, TShNormalisation outNorm, UInt32 order, UInt32 frameCount ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The ( order + 1 )^2 encoding gains, in ACN order, of a plane wave from the listener azimuth (positive to the
     *  right, the GUCEF listener convention) and elevation (positive up). The azimuth is negated into the AmbiX azimuth
     *  (positive left): +90 degrees gives a negative ACN 1 (Y). False (gains unchanged) for an unsupported order or
     *  normalisation.
     */
    static bool EncodeDirection( const CTRadian< Float32 >& listenerAzimuth, const CTRadian< Float32 >& elevation, UInt32 order, TShNormalisation norm, Float32* gains ) GUCEF_MATH_REALTIME_CONTEXT;

    static bool EncodeDirection( const CTRadian< Float64 >& listenerAzimuth, const CTRadian< Float64 >& elevation, UInt32 order, TShNormalisation norm, Float64* gains ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The encoding gains, in ACN order, of a plane wave from the unit direction 'direction' in listener space (x right,
     *  y up, -z forward): AmbiX ( x, y, z ) = ( -direction.z, -direction.x, direction.y ).
     */
    static bool EncodeListenerDirection( const CTDirectionVector3D< Float32 >& direction, UInt32 order, TShNormalisation norm, Float32* gains ) GUCEF_MATH_REALTIME_CONTEXT;

    static bool EncodeListenerDirection( const CTDirectionVector3D< Float64 >& direction, UInt32 order, TShNormalisation norm, Float64* gains ) GUCEF_MATH_REALTIME_CONTEXT;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CAMBISONICFORMAT_H ? */
