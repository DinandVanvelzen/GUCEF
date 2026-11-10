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

#ifndef GUCEF_CORE_CDENORMALFLUSHSCOPE_H
#define GUCEF_CORE_CDENORMALFLUSHSCOPE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Scope in which the calling thread treats denormal (subnormal) floating point numbers as zero, the way
 *  CScopeMutex locks one:
 *
 *      {
 *          CDenormalFlushScope flushScope;
 *          <real time signal processing, feedback filters, anything that decays towards zero>
 *      }
 *
 *  Denormals are the tiny numbers between zero and the smallest normal number. Arithmetic on them is many times
 *  slower than on normal numbers on most processors, which makes a decaying feedback loop (a reverb tail, a filter
 *  that runs out of input) suddenly use many times the CPU it used a moment before. In the scope results that would
 *  be denormal are flushed to zero (FTZ) and denormal inputs are read as zero (DAZ, where the processor has it).
 *
 *  The setting is per thread: it is changed in the floating point control register of the calling thread, other running
 *  threads are not affected. A thread that is started inside the scope starts with the setting of its creator (the floating
 *  point environment is inherited at thread creation), which then stays in effect for that thread after the scope has ended.
 *  The register is:
 *  - x86 and x86-64: SSE MXCSR flush-to-zero and denormals-are-zero (the x87 unit is not affected)
 *  - AArch64: FPCR.FZ
 *  - ARMv7 with hardware floating point: FPSCR.FZ
 *  - Elsewhere (WebAssembly offers no such control, soft float, other processors) the scope does nothing,
 *    IsSupported() tells which. Code that needs the effect there must flush its own state.
 *
 *  The scope ends when the object is destroyed: at the end of the block, on a return from it and when an exception
 *  unwinds it. Only the flush bits are put back as they were, so scopes nest and the floating point exception flags
 *  that were raised inside the scope are kept. Objects of this class must end in the reverse order of their creation,
 *  which block scoping gives for free, and on the thread that created them.
 */
class GUCEF_CORE_PUBLIC_CPP CDenormalFlushScope
{
    public:

    /**
     *  Starts flushing denormals on the calling thread
     */
    CDenormalFlushScope( void );

    /**
     *  Puts the flush setting of the calling thread back as it was when the scope was opened
     */
    ~CDenormalFlushScope();

    /**
     *  True when this platform and build has a control for flushing denormals, false when the scope is a no-op
     */
    static bool IsSupported( void );

    /**
     *  True when the calling thread flushes denormals right now (always false when not supported)
     */
    static bool IsActive( void );

    private:

    CDenormalFlushScope( const CDenormalFlushScope& src );              /* Copying doesnt make sense */
    CDenormalFlushScope& operator=( const CDenormalFlushScope& src );   /* Copying doesnt make sense */

    UInt32 m_previous;      /**< the flush bits of the control register as they were */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDENORMALFLUSHSCOPE_H ? */
