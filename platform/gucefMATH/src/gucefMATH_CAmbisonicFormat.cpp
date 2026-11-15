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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#include "gucefMATH_CAmbisonicFormat.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** FuMa channel -> ACN index: W X Y Z R S T U V K L M N O P Q */
const UInt32 g_fumaToAcn[ 16 ] = { 0, 3, 1, 2, 6, 7, 5, 8, 4, 12, 13, 11, 14, 10, 15, 9 };

/** ACN index -> FuMa channel */
const UInt32 g_acnToFuma[ 16 ] = { 0, 2, 3, 1, 8, 6, 4, 5, 7, 15, 13, 11, 9, 10, 12, 14 };

/*-------------------------------------------------------------------------*/

template < typename T >
bool
ConvertBlockOf( const T* in, TAmbisonicChannelOrder inOrder, TShNormalisation inNorm,
                T* out, TAmbisonicChannelOrder outOrder, TShNormalisation outNorm, UInt32 order, UInt32 frameCount ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !CAmbisonicFormat::IsSupported( inOrder, inNorm, order ) || !CAmbisonicFormat::IsSupported( outOrder, outNorm, order ) )
    {
        GUCEF_MATH_ASSERT( false );
        return false;
    }
    // the ratios per channel once for the block
    const UInt32 count = ( order + 1 ) * ( order + 1 );
    T ratio[ CAmbisonicFormat::MAX_CHANNELS ];
    UInt32 source[ CAmbisonicFormat::MAX_CHANNELS ];
    for ( UInt32 acn=0; acn<count; ++acn )
    {
        const UInt32 target = CAmbisonicFormat::GetChannelIndex( acn, outOrder );
        ratio[ target ] = (T) ( (Float64) CTSphericalHarmonicsBasis< Float64 >::GetNormalisationFactor( acn, outNorm )
                              / (Float64) CTSphericalHarmonicsBasis< Float64 >::GetNormalisationFactor( acn, inNorm ) );
        source[ target ] = CAmbisonicFormat::GetChannelIndex( acn, inOrder );
    }
    T copy[ CAmbisonicFormat::MAX_CHANNELS ];
    for ( UInt32 frame=0; frame<frameCount; ++frame )
    {
        const T* frameIn = in + (size_t) frame * count;
        T* frameOut = out + (size_t) frame * count;
        for ( UInt32 c=0; c<count; ++c )
            copy[ c ] = frameIn[ c ];
        for ( UInt32 c=0; c<count; ++c )
            frameOut[ c ] = copy[ source[ c ] ] * ratio[ c ];
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
EncodeAmbiX( const T& x, const T& y, const T& z, UInt32 order, TShNormalisation norm, T* gains ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !CAmbisonicFormat::IsSupported( AMBIORDER_ACN, norm, order ) )
    {
        GUCEF_MATH_ASSERT( false );
        return false;
    }
    CTSphericalHarmonicsBasis< T >::Evaluate( x, y, z, order, norm, gains );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
EncodeAzimuthElevation( const T& listenerAzimuth, const T& elevation, UInt32 order, TShNormalisation norm, T* gains ) GUCEF_MATH_REALTIME_CONTEXT
{
    // listener azimuth positive right -> AmbiX azimuth positive left
    const Float64 azimuth = -(Float64) listenerAzimuth;
    const Float64 cosElevation = cos( (Float64) elevation );
    return EncodeAmbiX( (T) ( cosElevation * cos( azimuth ) ), (T) ( cosElevation * sin( azimuth ) ), (T) sin( (Float64) elevation ), order, norm, gains );
}

}  /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32
CAmbisonicFormat::GetChannelCount( UInt32 order ) GUCEF_MATH_REALTIME_CONTEXT
{
    return ( order + 1 ) * ( order + 1 );
}

/*-------------------------------------------------------------------------*/

bool
CAmbisonicFormat::IsSupported( TAmbisonicChannelOrder channelOrder, TShNormalisation norm, UInt32 order ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( order > (UInt32) MAX_ORDER )
        return false;
    if ( ( AMBIORDER_FUMA == channelOrder || SHNORM_FUMA == norm ) && order > (UInt32) FUMA_MAX_ORDER )
        return false;
    return AMBIORDER_ACN == channelOrder || AMBIORDER_FUMA == channelOrder;
}

/*-------------------------------------------------------------------------*/

UInt32
CAmbisonicFormat::GetAcnIndex( UInt32 channel, TAmbisonicChannelOrder channelOrder ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( AMBIORDER_FUMA == channelOrder && channel < 16 )
        return g_fumaToAcn[ channel ];
    return channel;
}

/*-------------------------------------------------------------------------*/

UInt32
CAmbisonicFormat::GetChannelIndex( UInt32 acn, TAmbisonicChannelOrder channelOrder ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( AMBIORDER_FUMA == channelOrder && acn < 16 )
        return g_acnToFuma[ acn ];
    return acn;
}

/*-------------------------------------------------------------------------*/

bool
CAmbisonicFormat::ConvertChannels( const Float32* in, TAmbisonicChannelOrder inOrder, TShNormalisation inNorm,
                                   Float32* out, TAmbisonicChannelOrder outOrder, TShNormalisation outNorm, UInt32 order ) GUCEF_MATH_REALTIME_CONTEXT
{
    return ConvertBlockOf( in, inOrder, inNorm, out, outOrder, outNorm, order, 1 );
}

/*-------------------------------------------------------------------------*/

bool
CAmbisonicFormat::ConvertChannels( const Float64* in, TAmbisonicChannelOrder inOrder, TShNormalisation inNorm,
                                   Float64* out, TAmbisonicChannelOrder outOrder, TShNormalisation outNorm, UInt32 order ) GUCEF_MATH_REALTIME_CONTEXT
{
    return ConvertBlockOf( in, inOrder, inNorm, out, outOrder, outNorm, order, 1 );
}

/*-------------------------------------------------------------------------*/

bool
CAmbisonicFormat::ConvertBlock( const Float32* in, TAmbisonicChannelOrder inOrder, TShNormalisation inNorm,
                                Float32* out, TAmbisonicChannelOrder outOrder, TShNormalisation outNorm, UInt32 order, UInt32 frameCount ) GUCEF_MATH_REALTIME_CONTEXT
{
    return ConvertBlockOf( in, inOrder, inNorm, out, outOrder, outNorm, order, frameCount );
}

/*-------------------------------------------------------------------------*/

bool
CAmbisonicFormat::ConvertBlock( const Float64* in, TAmbisonicChannelOrder inOrder, TShNormalisation inNorm,
                                Float64* out, TAmbisonicChannelOrder outOrder, TShNormalisation outNorm, UInt32 order, UInt32 frameCount ) GUCEF_MATH_REALTIME_CONTEXT
{
    return ConvertBlockOf( in, inOrder, inNorm, out, outOrder, outNorm, order, frameCount );
}

/*-------------------------------------------------------------------------*/

bool
CAmbisonicFormat::EncodeDirection( const CTRadian< Float32 >& listenerAzimuth, const CTRadian< Float32 >& elevation, UInt32 order, TShNormalisation norm, Float32* gains ) GUCEF_MATH_REALTIME_CONTEXT
{
    return EncodeAzimuthElevation( listenerAzimuth.GetValue(), elevation.GetValue(), order, norm, gains );
}

/*-------------------------------------------------------------------------*/

bool
CAmbisonicFormat::EncodeDirection( const CTRadian< Float64 >& listenerAzimuth, const CTRadian< Float64 >& elevation, UInt32 order, TShNormalisation norm, Float64* gains ) GUCEF_MATH_REALTIME_CONTEXT
{
    return EncodeAzimuthElevation( listenerAzimuth.GetValue(), elevation.GetValue(), order, norm, gains );
}

/*-------------------------------------------------------------------------*/

bool
CAmbisonicFormat::EncodeListenerDirection( const CTDirectionVector3D< Float32 >& direction, UInt32 order, TShNormalisation norm, Float32* gains ) GUCEF_MATH_REALTIME_CONTEXT
{
    return EncodeAmbiX( -direction.GetZ(), -direction.GetX(), direction.GetY(), order, norm, gains );
}

/*-------------------------------------------------------------------------*/

bool
CAmbisonicFormat::EncodeListenerDirection( const CTDirectionVector3D< Float64 >& direction, UInt32 order, TShNormalisation norm, Float64* gains ) GUCEF_MATH_REALTIME_CONTEXT
{
    return EncodeAmbiX( -direction.GetZ(), -direction.GetX(), direction.GetY(), order, norm, gains );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
