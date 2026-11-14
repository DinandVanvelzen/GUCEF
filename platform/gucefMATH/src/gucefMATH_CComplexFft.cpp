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

#include <string.h>

#include "gucefMATH_FftKernels.h"

#include "gucefMATH_CComplexFft.h"

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

GUCEF_MATH_STATIC_ASSERT( sizeof( CTComplex< Float32 > ) == 2 * sizeof( Float32 ), CTComplex_Float32_is_two_floats );
GUCEF_MATH_STATIC_ASSERT( sizeof( CTComplex< Float64 > ) == 2 * sizeof( Float64 ), CTComplex_Float64_is_two_doubles );

template< typename T >
void
ForwardTransform( const T* input, T* output, UInt32 size, const T* twiddles, T* work ) GUCEF_MATH_REALTIME_CONTEXT
{
    FftSplitRadix( input, 1, work, size, twiddles, size );
    memcpy( output, work, 2 * (size_t) size * sizeof( T ) );
}

/*-------------------------------------------------------------------------*/

/** The inverse is the conjugate of the forward transform of the conjugate, scaled by 1/N */
template< typename T >
void
InverseTransform( const T* input, T* output, UInt32 size, const T* twiddles, T* workIn, T* workOut ) GUCEF_MATH_REALTIME_CONTEXT
{
    for ( UInt32 n=0; n<size; ++n )
    {
        workIn[ 2 * (size_t) n ] = input[ 2 * (size_t) n ];
        workIn[ 2 * (size_t) n + 1 ] = -input[ 2 * (size_t) n + 1 ];
    }

    FftSplitRadix( workIn, 1, workOut, size, twiddles, size );

    const T scale = (T) 1 / (T) size;
    for ( UInt32 n=0; n<size; ++n )
    {
        output[ 2 * (size_t) n ] = workOut[ 2 * (size_t) n ] * scale;
        output[ 2 * (size_t) n + 1 ] = -workOut[ 2 * (size_t) n + 1 ] * scale;
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CComplexFft::CComplexFft( void )
    : m_size( 0 )
    , m_twiddles32()
    , m_workA32()
    , m_workB32()
    , m_twiddles64()
    , m_workA64()
    , m_workB64()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComplexFft::~CComplexFft()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CComplexFft::Prepare( UInt32 size, bool singlePrecision, bool doublePrecision )
{GUCEF_TRACE;

    if ( !FftIsValidSize( size, (UInt32) MIN_SIZE, (UInt32) MAX_SIZE ) )
        return false;

    m_size = 0;
    m_twiddles32.clear(); m_workA32.clear(); m_workB32.clear();
    m_twiddles64.clear(); m_workA64.clear(); m_workB64.clear();

    if ( singlePrecision )
    {
        FftFillTwiddles( m_twiddles32, size );
        m_workA32.assign( 2 * (size_t) size, 0.0f );
        m_workB32.assign( 2 * (size_t) size, 0.0f );
    }
    if ( doublePrecision )
    {
        FftFillTwiddles( m_twiddles64, size );
        m_workA64.assign( 2 * (size_t) size, 0.0 );
        m_workB64.assign( 2 * (size_t) size, 0.0 );
    }
    m_size = size;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CComplexFft::GetSize( void ) const
{
    // no trace macro: realtime safe

    return m_size;
}

/*-------------------------------------------------------------------------*/

bool
CComplexFft::Forward( const CTComplex< Float32 >* input, CTComplex< Float32 >* output )
{
    // no trace macro: realtime safe

    if ( m_twiddles32.empty() || GUCEF_NULL == input || GUCEF_NULL == output )
        return false;
    ForwardTransform( &input[ 0 ].re, &output[ 0 ].re, m_size, &m_twiddles32[ 0 ], &m_workA32[ 0 ] );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CComplexFft::Forward( const CTComplex< Float64 >* input, CTComplex< Float64 >* output )
{
    // no trace macro: realtime safe

    if ( m_twiddles64.empty() || GUCEF_NULL == input || GUCEF_NULL == output )
        return false;
    ForwardTransform( &input[ 0 ].re, &output[ 0 ].re, m_size, &m_twiddles64[ 0 ], &m_workA64[ 0 ] );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CComplexFft::Inverse( const CTComplex< Float32 >* input, CTComplex< Float32 >* output )
{
    // no trace macro: realtime safe

    if ( m_twiddles32.empty() || GUCEF_NULL == input || GUCEF_NULL == output )
        return false;
    InverseTransform( &input[ 0 ].re, &output[ 0 ].re, m_size, &m_twiddles32[ 0 ], &m_workA32[ 0 ], &m_workB32[ 0 ] );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CComplexFft::Inverse( const CTComplex< Float64 >* input, CTComplex< Float64 >* output )
{
    // no trace macro: realtime safe

    if ( m_twiddles64.empty() || GUCEF_NULL == input || GUCEF_NULL == output )
        return false;
    InverseTransform( &input[ 0 ].re, &output[ 0 ].re, m_size, &m_twiddles64[ 0 ], &m_workA64[ 0 ], &m_workB64[ 0 ] );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MATH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
