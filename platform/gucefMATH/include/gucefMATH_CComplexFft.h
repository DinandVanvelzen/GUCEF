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

#ifndef GUCEF_MATH_CCOMPLEXFFT_H
#define GUCEF_MATH_CCOMPLEXFFT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_MATH_CTCOMPLEX_H
#include "gucefMATH_CTComplex.h"
#define GUCEF_MATH_CTCOMPLEX_H
#endif /* GUCEF_MATH_CTCOMPLEX_H ? */

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
 *  Fast Fourier transform of complex signals: N complex values in, N complex values out, N a power of two from
 *  MIN_SIZE (2) to MAX_SIZE (2^20). The algorithm is the split-radix decimation in time of CRealFft.
 *
 *  Forward is unscaled, X[k] = sum over n of x[n] * exp( -2 pi i k n / N ). Inverse is scaled by 1/N, so the inverse of
 *  the forward transform is the original signal. The input and the output may be the same memory (in place).
 *
 *  Float32 and Float64 paths are prepared on request (Prepare()); calling a path that was not prepared does nothing
 *  and returns false. Prepare() allocates; nothing else does, and nothing locks, so Forward() and Inverse() are
 *  realtime safe. One thread at a time uses one object (it has work buffers).
 */
class GUCEF_MATH_PUBLIC_CPP CComplexFft
{
    public:

    enum
    {
        MIN_SIZE = 2 ,
        MAX_SIZE = 1048576
    };

    CComplexFft( void );

    ~CComplexFft();

    /**
     *  Builds the twiddle tables and work buffers. Allocates: not for the real time thread.
     *  A size outside MIN_SIZE to MAX_SIZE or not a power of two gives false and leaves the object as it was.
     */
    bool Prepare( UInt32 size, bool singlePrecision = true, bool doublePrecision = true );

    /** N, or 0 when not prepared */
    UInt32 GetSize( void ) const;

    /** 'input' and 'output': N complex values each; may be the same memory */
    bool Forward( const CTComplex< Float32 >* input, CTComplex< Float32 >* output );
    bool Forward( const CTComplex< Float64 >* input, CTComplex< Float64 >* output );

    /** 'input' and 'output': N complex values each, output scaled by 1/N; may be the same memory */
    bool Inverse( const CTComplex< Float32 >* input, CTComplex< Float32 >* output );
    bool Inverse( const CTComplex< Float64 >* input, CTComplex< Float64 >* output );

    private:

    CComplexFft( const CComplexFft& src );              /**< not implemented */
    CComplexFft& operator=( const CComplexFft& src );   /**< not implemented */

    private:

    UInt32 m_size;
    GUCEF::vector< Float32 > m_twiddles32;      /**< exp( -2 pi i j / N ) for j below N, real and imaginary part alternating */
    GUCEF::vector< Float32 > m_workA32;         /**< N complex numbers */
    GUCEF::vector< Float32 > m_workB32;         /**< N complex numbers */
    GUCEF::vector< Float64 > m_twiddles64;
    GUCEF::vector< Float64 > m_workA64;
    GUCEF::vector< Float64 > m_workB64;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MATH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CCOMPLEXFFT_H ? */
