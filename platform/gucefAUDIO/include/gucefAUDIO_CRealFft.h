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

#ifndef GUCEF_AUDIO_CREALFFT_H
#define GUCEF_AUDIO_CREALFFT_H

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

/**
 *  Fast Fourier transform of real signals, for spectral processing (noise suppression, echo cancellation, analysis).
 *  A DSP block of gucefAUDIO until gucefMATH has an FFT of its own, at which point this class forwards to it.
 *
 *  The size N is a power of two from MIN_SIZE (64) to MAX_SIZE (4096). The transform is done as a complex
 *  transform of size N/2 (split-radix, decimation in time, recursive with the tables of Prepare()), with the
 *  split into the spectra of the even and the odd samples afterwards.
 *
 *  Spectrum layout: N/2 + 1 bins (bin k is the frequency k / N of the sample rate, DC to Nyquist) of two values each,
 *  real part then imaginary part: 'N + 2' values. The imaginary parts of bin 0 and bin N/2 are zero for a real signal;
 *  they are written as zero and ignored by the inverse.
 *
 *  Scaling: the forward transform is unscaled, X[k] = sum over n of x[n] * exp( -2 pi i k n / N ), so a sine of
 *  amplitude A on a bin gives a bin magnitude of A * N / 2. The inverse transform is scaled by 1/N, so the inverse of the
 *  forward transform is the original signal.
 *
 *  Float32 and Float64 paths: the Float32 path computes with single precision tables and sums, accurate to about
 *  1e-6 of the largest value for the sizes above; the Float64 path to about 1e-14. Prepare() builds the tables for the
 *  paths asked for; calling a path that was not prepared does nothing and returns false.
 *
 *  Out-of-place and in-place: the input and the output may be the same memory (the buffer is then 'N + 2' values
 *  long, the time signal in the first N), results are the same as for separate buffers.
 *
 *  Prepare() allocates; nothing else does, and nothing locks, so Forward() and Inverse() are fit for the audio thread.
 *  One thread at a time uses one object (it has work buffers); use one object per thread or per stream.
 */
class GUCEF_AUDIO_PUBLIC_CPP CRealFft
{
    public:

    enum
    {
        MIN_SIZE = 64 ,
        MAX_SIZE = 4096
    };

    CRealFft( void );

    ~CRealFft();

    /**
     *  Builds the twiddle tables and work buffers. Allocates: not for the real time thread.
     *  @param size             N, a power of two from MIN_SIZE to MAX_SIZE
     *  @param singlePrecision  prepare the Float32 path
     *  @param doublePrecision  prepare the Float64 path
     */
    bool Prepare( UInt32 size, bool singlePrecision = true, bool doublePrecision = true );

    /** N, or 0 when not prepared */
    UInt32 GetSize( void ) const;

    /** N/2 + 1 */
    UInt32 GetBinCount( void ) const;

    /** Values in a spectrum buffer: 2 * ( N/2 + 1 ) = N + 2 */
    UInt32 GetSpectrumLength( void ) const;

    /** 'input': N reals. 'spectrum': N + 2 values (see the class description). May be the same memory */
    bool Forward( const Float32* input, Float32* spectrum );
    bool Forward( const Float64* input, Float64* spectrum );

    /** 'spectrum': N + 2 values. 'output': N reals, scaled by 1/N. May be the same memory */
    bool Inverse( const Float32* spectrum, Float32* output );
    bool Inverse( const Float64* spectrum, Float64* output );

    private:

    CRealFft( const CRealFft& src );            /**< not implemented */
    CRealFft& operator=( const CRealFft& src ); /**< not implemented */

    private:

    UInt32 m_size;
    GUCEF::vector< Float32 > m_twiddles32;      /**< exp( -2 pi i j / N ) for j below N, real and imaginary part alternating */
    GUCEF::vector< Float32 > m_workA32;         /**< N/2 complex numbers */
    GUCEF::vector< Float32 > m_workB32;         /**< N/2 complex numbers */
    GUCEF::vector< Float64 > m_twiddles64;
    GUCEF::vector< Float64 > m_workA64;
    GUCEF::vector< Float64 > m_workB64;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CREALFFT_H ? */
