/*
 *  gucefIMAGE: GUCEF module providing image utilities
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

#include <string.h>

#include "gucefIMAGE_CMmpxImageUpscaler.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The picture as whole RGBA pixels, read with the edges repeated */
class CPixels
{
    public:

    CPixels( const UInt8* data, Int32 width, Int32 height )
        : m_data( data ), m_width( width ), m_height( height )
    {
    }

    UInt32 At( Int32 x, Int32 y ) const
    {
        x = x < 0 ? 0 : ( x >= m_width ? m_width - 1 : x );
        y = y < 0 ? 0 : ( y >= m_height ? m_height - 1 : y );
        UInt32 pixel;
        memcpy( &pixel, m_data + ( (size_t) y * m_width + x ) * 4, 4 );
        return pixel;
    }

    private:

    const UInt8* m_data;
    Int32 m_width;
    Int32 m_height;
};

/*-------------------------------------------------------------------------*/

inline Float32
Luma( UInt32 pixel )
{
    const UInt8* c = reinterpret_cast< const UInt8* >( &pixel );
    return 0.2126f * c[ 0 ] + 0.7152f * c[ 1 ] + 0.0722f * c[ 2 ];
}

inline bool AllEq2( UInt32 B, UInt32 A0, UInt32 A1 ) { return B == A0 && B == A1; }
inline bool AllEq3( UInt32 B, UInt32 A0, UInt32 A1, UInt32 A2 ) { return B == A0 && B == A1 && B == A2; }
inline bool AllEq4( UInt32 B, UInt32 A0, UInt32 A1, UInt32 A2, UInt32 A3 ) { return B == A0 && B == A1 && B == A2 && B == A3; }
inline bool AnyEq3( UInt32 B, UInt32 A0, UInt32 A1, UInt32 A2 ) { return B == A0 || B == A1 || B == A2; }
inline bool NoneEq2( UInt32 B, UInt32 A0, UInt32 A1 ) { return B != A0 && B != A1; }
inline bool NoneEq4( UInt32 B, UInt32 A0, UInt32 A1, UInt32 A2, UInt32 A3 ) { return B != A0 && B != A1 && B != A2 && B != A3; }

/*-------------------------------------------------------------------------*/

/** The four output pixels of one source pixel: J K on top, L M below */
void
MmpxPixel( const CPixels& src, Int32 x, Int32 y, UInt32& J, UInt32& K, UInt32& L, UInt32& M )
{
    const UInt32 E = src.At( x, y );
    const UInt32 A = src.At( x - 1, y - 1 ), B = src.At( x, y - 1 ), C = src.At( x + 1, y - 1 );
    const UInt32 D = src.At( x - 1, y ), F = src.At( x + 1, y );
    const UInt32 G = src.At( x - 1, y + 1 ), H = src.At( x, y + 1 ), I = src.At( x + 1, y + 1 );

    // nearest by default, constant 3x3 centres stay that way
    J = K = L = M = E;
    if ( E == A && E == B && E == C && E == D && E == F && E == G && E == H && E == I )
        return;

    const UInt32 P = src.At( x, y - 2 ), Q = src.At( x - 2, y ), R = src.At( x + 2, y ), S = src.At( x, y + 2 );
    const Float32 Bl = Luma( B ), Dl = Luma( D ), El = Luma( E ), Fl = Luma( F ), Hl = Luma( H );

    // round some corners and fill in 1:1 slopes, but keep sharp right angles
    if ( ( D == B && D != H && D != F ) && ( El >= Dl || E == A ) && AnyEq3( E, A, C, G ) && ( El < Dl || A != D || E != P || E != Q ) ) J = D;
    if ( ( B == F && B != D && B != H ) && ( El >= Bl || E == C ) && AnyEq3( E, A, C, I ) && ( El < Bl || C != B || E != P || E != R ) ) K = B;
    if ( ( H == D && H != F && H != B ) && ( El >= Hl || E == G ) && AnyEq3( E, A, G, I ) && ( El < Hl || G != H || E != S || E != Q ) ) L = H;
    if ( ( F == H && F != B && F != D ) && ( El >= Fl || E == I ) && AnyEq3( E, C, G, I ) && ( El < Fl || I != H || E != R || E != S ) ) M = F;

    // clean up disconnected line intersections
    if ( ( E != F && AllEq4( E, C, I, D, Q ) && AllEq2( F, B, H ) ) && F != src.At( x + 3, y ) ) K = M = F;
    if ( ( E != D && AllEq4( E, A, G, F, R ) && AllEq2( D, B, H ) ) && D != src.At( x - 3, y ) ) J = L = D;
    if ( ( E != H && AllEq4( E, G, I, B, P ) && AllEq2( H, D, F ) ) && H != src.At( x, y + 3 ) ) L = M = H;
    if ( ( E != B && AllEq4( E, A, C, H, S ) && AllEq2( B, D, F ) ) && B != src.At( x, y - 3 ) ) J = K = B;

    // remove the tips of bright triangles on dark backgrounds
    if ( Bl < El && AllEq4( E, G, H, I, S ) && NoneEq4( E, A, D, C, F ) ) J = K = B;
    if ( Hl < El && AllEq4( E, A, B, C, P ) && NoneEq4( E, D, G, I, F ) ) L = M = H;
    if ( Fl < El && AllEq4( E, A, D, G, Q ) && NoneEq4( E, B, C, I, H ) ) K = M = F;
    if ( Dl < El && AllEq4( E, C, F, I, R ) && NoneEq4( E, B, A, G, H ) ) J = L = D;

    // 2:1 and 1:2 slopes of constant colour
    if ( H != B )
    {
        if ( H != A && H != E && H != C )
        {
            if ( AllEq3( H, G, F, R ) && NoneEq2( H, D, src.At( x + 2, y - 1 ) ) ) L = M;
            if ( AllEq3( H, I, D, Q ) && NoneEq2( H, F, src.At( x - 2, y - 1 ) ) ) M = L;
        }
        if ( B != I && B != G && B != E )
        {
            if ( AllEq3( B, A, F, R ) && NoneEq2( B, D, src.At( x + 2, y + 1 ) ) ) J = K;
            if ( AllEq3( B, C, D, Q ) && NoneEq2( B, F, src.At( x - 2, y + 1 ) ) ) K = J;
        }
    }
    if ( F != D )
    {
        if ( D != I && D != E && D != C )
        {
            if ( AllEq3( D, A, H, S ) && NoneEq2( D, B, src.At( x + 1, y + 2 ) ) ) J = L;
            if ( AllEq3( D, G, B, P ) && NoneEq2( D, H, src.At( x + 1, y - 2 ) ) ) L = J;
        }
        if ( F != E && F != A && F != G )
        {
            if ( AllEq3( F, C, H, S ) && NoneEq2( F, B, src.At( x - 1, y + 2 ) ) ) K = M;
            if ( AllEq3( F, I, B, P ) && NoneEq2( F, H, src.At( x - 1, y - 2 ) ) ) M = K;
        }
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMmpxImageUpscaler::CMmpxImageUpscaler( void )
    : CImageUpscaler()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMmpxImageUpscaler::~CMmpxImageUpscaler()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CMmpxImageUpscaler::GetName( void ) const
{GUCEF_TRACE;

    return "mmpx";
}

/*-------------------------------------------------------------------------*/

CImageUpscaler::TFactorVector
CMmpxImageUpscaler::GetFactors( void ) const
{GUCEF_TRACE;

    TFactorVector factors;
    factors.push_back( 2 );
    factors.push_back( 4 );
    return factors;
}

/*-------------------------------------------------------------------------*/

bool
CMmpxImageUpscaler::Double( const CPixelMap& source, TPixelMapPtr& result, CIUpscaleProgress* progress, Float32 progressFrom, Float32 progressTo )
{GUCEF_TRACE;

    const Int32 width = (Int32) source.GetWidthInPixels();
    const Int32 height = (Int32) source.GetHeightInPixels();
    if ( width <= 0 || height <= 0 || PSF_RGBA != source.GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != source.GetPixelComponentDataType() )
        return false;

    TPixelMapPtr doubled( GUCEF_NEW CPixelMap( GUCEF_NULL, (UInt32) width * 2, (UInt32) height * 2, PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    CPixels src( static_cast< const UInt8* >( source.GetDataPtr() ), width, height );
    UInt8* out = static_cast< UInt8* >( doubled->GetDataPtr() );
    const size_t outRow = (size_t) width * 2 * 4;
    for ( Int32 y=0; y<height; ++y )
    {
        if ( GUCEF_NULL != progress && 0 == ( y % 16 ) && !progress->OnUpscaleProgress( progressFrom + ( progressTo - progressFrom ) * y / height ) )
            return false;
        UInt8* top = out + (size_t) y * 2 * outRow;
        UInt8* bottom = top + outRow;
        for ( Int32 x=0; x<width; ++x )
        {
            UInt32 J, K, L, M;
            MmpxPixel( src, x, y, J, K, L, M );
            memcpy( top + x * 8, &J, 4 );
            memcpy( top + x * 8 + 4, &K, 4 );
            memcpy( bottom + x * 8, &L, 4 );
            memcpy( bottom + x * 8 + 4, &M, 4 );
        }
    }
    result = doubled;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMmpxImageUpscaler::Upscale( const CPixelMap& source, UInt32 factor, TPixelMapPtr& result, CIUpscaleProgress* progress )
{GUCEF_TRACE;

    if ( 2 != factor && 4 != factor )
        return false;

    // RGBA with 8 bits per channel, the picture of the caller stays as it is
    const CPixelMap* rgba = &source;
    CPixelMap converted( source );
    if ( PSF_RGBA != source.GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != source.GetPixelComponentDataType() )
    {
        if ( !converted.ConvertFormatTo( PSF_RGBA, GUCEF::DATATYPE_UINT8 ) )
            return false;
        rgba = &converted;
    }

    TPixelMapPtr twice;
    if ( !Double( *rgba, twice, progress, 0.0f, 2 == factor ? 1.0f : 0.2f ) )
        return false;
    if ( 4 == factor && !Double( *twice, twice, progress, 0.2f, 1.0f ) )
        return false;
    if ( GUCEF_NULL != progress && !progress->OnUpscaleProgress( 1.0f ) )
        return false;
    result = twice;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
