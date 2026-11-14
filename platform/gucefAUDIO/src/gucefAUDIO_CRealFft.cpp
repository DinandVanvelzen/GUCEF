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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CREALFFT_H
#include "gucefMATH_CRealFft.h"
#define GUCEF_MATH_CREALFFT_H
#endif /* GUCEF_MATH_CREALFFT_H ? */

#include "gucefAUDIO_CRealFft.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRealFft::CRealFft( void )
    : m_fft( new MATH::CRealFft() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRealFft::~CRealFft()
{GUCEF_TRACE;

    delete m_fft;
    m_fft = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CRealFft::Prepare( UInt32 size, bool singlePrecision, bool doublePrecision )
{GUCEF_TRACE;

    if ( size < MIN_SIZE || size > MAX_SIZE )
        return false;

    return m_fft->Prepare( size, singlePrecision, doublePrecision );
}

/*-------------------------------------------------------------------------*/

UInt32
CRealFft::GetSize( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return m_fft->GetSize();
}

/*-------------------------------------------------------------------------*/

UInt32
CRealFft::GetBinCount( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return m_fft->GetBinCount();
}

/*-------------------------------------------------------------------------*/

UInt32
CRealFft::GetSpectrumLength( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return m_fft->GetSpectrumLength();
}

/*-------------------------------------------------------------------------*/

bool
CRealFft::Forward( const Float32* input, Float32* spectrum )
{
    // no trace macro: this runs where the audio is mixed

    return m_fft->Forward( input, spectrum );
}

/*-------------------------------------------------------------------------*/

bool
CRealFft::Forward( const Float64* input, Float64* spectrum )
{
    // no trace macro: this runs where the audio is mixed

    return m_fft->Forward( input, spectrum );
}

/*-------------------------------------------------------------------------*/

bool
CRealFft::Inverse( const Float32* spectrum, Float32* output )
{
    // no trace macro: this runs where the audio is mixed

    return m_fft->Inverse( spectrum, output );
}

/*-------------------------------------------------------------------------*/

bool
CRealFft::Inverse( const Float64* spectrum, Float64* output )
{
    // no trace macro: this runs where the audio is mixed

    return m_fft->Inverse( spectrum, output );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
