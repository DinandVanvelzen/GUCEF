/*
 *  gucef common header: provides header based platform wide facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_DLLINIT_H
#define GUCEF_DLLINIT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"      /* GUCEF platform build configuration */
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>

  #define GUCEF_DLL_INIT_FUNC( ns )                    \
                                                       \
        int WINAPI DllMain( HINSTANCE hinstDLL ,       \
                            DWORD fwdreason    ,       \
                            LPVOID lpvReserved )       \
        {                                              \
                return 1;                              \
        }

  #define GUCEF_DLL_INIT_FUNC_BEG( ns )                \
                                                       \
        int WINAPI DllMain( HINSTANCE hinstDLL ,       \
                            DWORD fwdreason    ,       \
                            LPVOID lpvReserved )       \
        {                                              \
                if ( fwdreason == DLL_PROCESS_ATTACH ) \
                {                                      \
                        if ( !ns::Load() )             \
                        {                              \
                                return 0;              \
                        }                              \
                }                                      \
                return 1;                              \
        }

  #define GUCEF_DLL_INIT_FUNC_END( ns )                 \
                                                        \
        int WINAPI DllMain( HINSTANCE hinstDLL ,        \
                            DWORD fwdreason    ,        \
                            LPVOID lpvReserved )        \
        {                                               \
                if ( fwdreason == DLL_PROCESS_DETACH )  \
                {                                       \
                        if ( !ns::Unload() )            \
                        {                               \
                                return 0;               \
                        }                               \
                }                                       \
                return 1;                               \
        }

  #define GUCEF_DLL_INIT_FUNC_BEG_END( ns )                    \
                                                               \
        int WINAPI DllMain( HINSTANCE hinstDLL ,               \
                            DWORD fwdreason    ,               \
                            LPVOID lpvReserved )               \
        {                                                      \
                if ( fwdreason == DLL_PROCESS_ATTACH )         \
                {                                              \
                        if ( !ns::Load() )                     \
                        {                                      \
                                return 0;                      \
                        }                                      \
                        if ( fwdreason == DLL_PROCESS_DETACH ) \
                        {                                      \
                                if ( !ns::Unload() )           \
                                {                              \
                                        return 0;              \
                                }                              \
                        }                                      \
                        return 1;                              \
                }                                              \
                return 1;                                      \
        }

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

#if ( GUCEF_COMPILER == GUCEF_COMPILER_GNUCfoobar )

  #define GUCEF_DLL_INIT_FUNC( ns )                               \
                                                                  \
        void __attribute__ ((constructor(65535))) _moduleInit( void )    \
        {                                                         \
                                                                  \
        }                                                         \
                                                                  \
        void __attribute__ ((destructor)) _moduleShutdown( void ) \
        {                                                         \
                                                                  \
        }


  #define GUCEF_DLL_INIT_FUNC_BEG( ns )                           \
                                                                  \
        void __attribute__ ((constructor(65535))) _moduleInit( void )    \
        {                                                         \
                if ( !ns::Load() )                                \
                {                                                 \
                        return;                                   \
                }                                                 \
                return;                                           \
        }                                                         \
                                                                  \
        void __attribute__ ((destructor)) _moduleShutdown( void ) \
        {                                                         \
                return;                                           \
        }


  #define GUCEF_DLL_INIT_FUNC_END( ns )                           \
                                                                  \
        void __attribute__ ((constructor(65535))) _moduleInit( void )    \
        {                                                         \
                return;                                           \
        }                                                         \
                                                                  \
        void __attribute__ ((destructor)) _moduleShutdown( void ) \
        {                                                         \
                if ( !ns::Unload() )                              \
                {                                                 \
                        return;                                   \
                }                                                 \
                return;                                           \
        }


  #define GUCEF_DLL_INIT_FUNC_BEG_END( ns )                       \
                                                                  \
        void __attribute__ ((constructor(65535))) _moduleInit( void )    \
        {                                                         \
                if ( !ns::Load() )                                \
                {                                                 \
                        return;                                   \
                }                                                 \
                return;                                           \
        }                                                         \
                                                                  \
        void __attribute__ ((destructor)) _moduleShutdown( void ) \
        {                                                         \
                if ( !ns::Unload() )                              \
                {                                                 \
                        return;                                   \
                }                                                 \
                return;                                           \
        }

#else

  #define GUCEF_DLL_INIT_FUNC( ns ) \
                                    \
        int _init( void )           \
        {                           \
                return 1;           \
        }                           \
                                    \
        int _fini( void )           \
        {                           \
                return 1;           \
        }


  #define GUCEF_DLL_INIT_FUNC_BEG( ns )   \
                                          \
        int _init( void )                 \
        {                                 \
                if ( !ns::Load() )        \
                {                         \
                        return 0;         \
                }                         \
                return 1;                 \
        }                                 \
                                          \
        int _fini( void )                 \
        {                                 \
                return 1;                 \
        }


  #define GUCEF_DLL_INIT_FUNC_END( ns )  \
                                         \
        int _init( void )                \
        {                                \
                return 1;                \
        }                                \
                                         \
        int _fini( void )                \
        {                                \
                if ( !ns::Unload() )     \
                {                        \
                        return 0;        \
                }                        \
                return 1;                \
        }


  #define GUCEF_DLL_INIT_FUNC_BEG_END( ns ) \
                                            \
        int _init( void )                   \
        {                                   \
                if ( !ns::Load() )          \
                {                           \
                        return 0;           \
                }                           \
                return 1;                   \
        }                                   \
                                            \
        int _fini( void )                   \
        {                                   \
                if ( !ns::Unload() )        \
                {                           \
                        return 0;           \
                }                           \
                return 1;                   \
        }

#endif

#else
  #error No DLL entry points available for the target platform
#endif

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DLLINIT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 12-02-2005 :
        - Initial implementation

-----------------------------------------------------------------------------*/
