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

#ifndef GUCEF_MATH_CTCOMPENSATEDSUM_H
#define GUCEF_MATH_CTCOMPENSATEDSUM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTNUMERICTRAITS_H
#include "gucefMATH_CTNumericTraits.h"
#define GUCEF_MATH_CTNUMERICTRAITS_H
#endif /* GUCEF_MATH_CTNUMERICTRAITS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Compensated summation (Neumaier's improvement of Kahan's algorithm): the rounding error of every addition is kept
 *  in a separate term, so the sum of 1, 1e100, 1, -1e100 is 2 instead of 0. For floating point kinds; for exact kinds it
 *  is a plain sum. Requires value semantics without contraction (the module builds with -ffp-contract=off) and
 *  without -ffast-math. Realtime-safe, no allocation.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTCompensatedSum
{
    public:

    typedef CTNumericTraits< T > TTraits;

    CTCompensatedSum( void )
        : m_sum( TTraits::Zero() )
        , m_compensation( TTraits::Zero() )
    {
    }

    /** Adds one term (precision-sensitive, section 5.10) */
    void Add( const T& value ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The compensated sum of all terms added since construction or Reset */
    T GetSum( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    inline void Reset( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        m_sum = TTraits::Zero();
        m_compensation = TTraits::Zero();
    }

    private:

    T m_sum;
    T m_compensation;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE void
CTCompensatedSum< T >::Add( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    T t = m_sum + value;
    if ( TTraits::Abs( value ) <= TTraits::Abs( m_sum ) )
        m_compensation = m_compensation + ( ( m_sum - t ) + value );
    else
        m_compensation = m_compensation + ( ( value - t ) + m_sum );
    m_sum = t;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTCompensatedSum< T >::GetSum( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return m_sum + m_compensation;
}

/*-------------------------------------------------------------------------*/

/* The Float32 and Float64 instantiations are compiled in the module (gucefMATH_CTCompensatedSum.cpp), section 5.10 */
GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTCompensatedSum< Float32 > )
GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTCompensatedSum< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTCOMPENSATEDSUM_H ? */
