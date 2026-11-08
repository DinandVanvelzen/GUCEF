/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#if ( defined( __SSE__ ) || defined( _M_X64 ) || ( defined( _M_IX86_FP ) && _M_IX86_FP >= 1 ) ) && ( defined( __i386__ ) || defined( __x86_64__ ) || defined( _M_IX86 ) || defined( _M_X64 ) )
  #define GUCEF_CORE_DENORMAL_X86_SSE
  #include <xmmintrin.h>
#elif defined( __aarch64__ ) && ( defined( __GNUC__ ) || defined( __clang__ ) )
  #define GUCEF_CORE_DENORMAL_AARCH64
#elif defined( __arm__ ) && ( defined( __GNUC__ ) || defined( __clang__ ) ) && ( defined( __VFP_FP__ ) || defined( __ARM_FP ) ) && !defined( __SOFTFP__ )
  #define GUCEF_CORE_DENORMAL_ARMV7
#endif

#ifndef GUCEF_CORE_CDENORMALFLUSHSCOPE_H
#include "gucefCORE_CDenormalFlushScope.h"
#define GUCEF_CORE_CDENORMALFLUSHSCOPE_H
#endif /* GUCEF_CORE_CDENORMALFLUSHSCOPE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if defined( GUCEF_CORE_DENORMAL_X86_SSE )

// MXCSR: bit 15 flush to zero, bit 6 denormals are zero
static const UInt32 flushMask = 0x8040;

static inline UInt32 ReadControl( void ) { return (UInt32) _mm_getcsr(); }
static inline void WriteControl( UInt32 value ) { _mm_setcsr( (unsigned int) value ); }

#elif defined( GUCEF_CORE_DENORMAL_AARCH64 )

// FPCR: bit 24 flush to zero
static const UInt32 flushMask = 0x01000000;

static inline UInt32 ReadControl( void )
{
    UInt64 value;
    __asm__ __volatile__( "mrs %0, fpcr" : "=r" ( value ) );
    return (UInt32) value;
}

static inline void WriteControl( UInt32 value )
{
    UInt64 wide = value;
    __asm__ __volatile__( "msr fpcr, %0" : : "r" ( wide ) );
}

#elif defined( GUCEF_CORE_DENORMAL_ARMV7 )

// FPSCR: bit 24 flush to zero
static const UInt32 flushMask = 0x01000000;

static inline UInt32 ReadControl( void )
{
    UInt32 value;
    __asm__ __volatile__( "vmrs %0, fpscr" : "=r" ( value ) );
    return value;
}

static inline void WriteControl( UInt32 value )
{
    __asm__ __volatile__( "vmsr fpscr, %0" : : "r" ( value ) );
}

#else

static const UInt32 flushMask = 0;

static inline UInt32 ReadControl( void ) { return 0; }
static inline void WriteControl( UInt32 ) {}

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDenormalFlushScope::CDenormalFlushScope( void )
    : m_previous( 0 )
{
    // no GUCEF_TRACE: used on real time threads

    if ( 0 != flushMask )
    {
        const UInt32 control = ReadControl();
        m_previous = control & flushMask;
        if ( m_previous != flushMask )
            WriteControl( control | flushMask );
    }
}

/*-------------------------------------------------------------------------*/

CDenormalFlushScope::~CDenormalFlushScope()
{
    if ( 0 != flushMask )
    {
        // only our bits are put back so that the exception flags raised in the scope survive
        const UInt32 control = ReadControl();
        if ( ( control & flushMask ) != m_previous )
            WriteControl( ( control & ~flushMask ) | m_previous );
    }
}

/*-------------------------------------------------------------------------*/

bool
CDenormalFlushScope::IsSupported( void )
{
    return 0 != flushMask;
}

/*-------------------------------------------------------------------------*/

bool
CDenormalFlushScope::IsActive( void )
{
    return 0 != flushMask && ( ReadControl() & flushMask ) == flushMask;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
