/*
 *  gucefMT: GUCEF module providing multithreading solutions
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

#ifndef GUCEF_MT_THREADNAME_H
#define GUCEF_MT_THREADNAME_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_ETYPES_H
#include "gucefMT_ETypes.h"
#define GUCEF_MT_ETYPES_H
#endif /* GUCEF_MT_ETYPES_H */

#ifndef GUCEF_MT_MACROS_H
#include "gucefMT_macros.h"
#define GUCEF_MT_MACROS_H
#endif /* GUCEF_MT_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The most bytes of a thread name the operating system keeps on Linux and Android (pthread_setname_np(): 16 bytes
 *  including the terminating zero). Longer names are cut to this many bytes there; Windows keeps the whole name.
 */
#define GUCEF_MT_MAX_OS_THREAD_NAME_BYTES     15

/*-------------------------------------------------------------------------*/

/**
 *  Gives the calling thread a name at the level of the operating system, as debuggers, profilers, `top -H` and
 *  /proc/<pid>/task/<tid>/comm show it. Meant to be called once, when a thread starts.
 *
 *  Linux and Android: pthread_setname_np(), the name cut to GUCEF_MT_MAX_OS_THREAD_NAME_BYTES bytes (never in the
 *  middle of a UTF-8 sequence). Apple: pthread_setname_np() of the calling thread. Windows: SetThreadDescription()
 *  where the system has it (Windows 10 1607 and later, looked up at run time), else nothing.
 *  Other platforms (Emscripten): nothing.
 *
 *  @return whether the operating system took the name; false where the platform has no thread names
 */
GUCEF_MT_PUBLIC_CPP bool
SetCurrentThreadName( const char* name );

/*-------------------------------------------------------------------------*/

/**
 *  The name the operating system has for the calling thread, as set with SetCurrentThreadName() or by the system,
 *  written to buffer with a terminating zero (cut to bufferSize - 1 bytes).
 *
 *  @return false where the platform has no thread names or the name cannot be read; buffer then holds an empty text
 */
GUCEF_MT_PUBLIC_CPP bool
GetCurrentThreadName( char* buffer, UInt32 bufferSize );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MT_THREADNAME_H ? */
