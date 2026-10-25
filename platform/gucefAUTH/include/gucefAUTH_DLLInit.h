/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_DLLINIT_H
#define GUCEF_AUTH_DLLINIT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTH_MACROS_H
#include "gucefAUTH_macros.h"         /* often used macros */
#define GUCEF_AUTH_MACROS_H
#endif /* GUCEF_AUTH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_MSWIN_BUILD
  #include <windows.h>

  #define DLL_INIT_FUNC( ns )                          \
                                                       \
        int WINAPI DllMain( HINSTANCE hinstDLL ,       \
                            DWORD fwdreason    ,       \
                            LPVOID lpvReserved )       \
        {                                              \
                return 1;                              \
        }                                              \                

  #define DLL_INIT_FUNC_BEG( ns )                      \
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

  #define DLL_INIT_FUNC_END( ns )                       \
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

  #define DLL_INIT_FUNC_BEG_END( ns )                          \
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
                  
        
#elif GUCEF_LINUX_BUILD

  #define DLL_INIT_FUNC( ns )   \
                                \ 
        int _init( void )       \
        {                       \
                return 1;       \
        }                       \
                                \
        int _fini( void )       \
        {                       \
                return 1;       \
        }                       
  
        
  #define DLL_INIT_FUNC_BEG( ns )   \
                                    \
        int _init( void )           \
        {                           \
                if ( !ns::Load() )  \
                {                   \
                        return 0;   \
                }                   \
                return 1;           \
        }                           \
                                    \
        int _fini( void )           \
        {                           \
                return 1;           \
        }                           
          
      
  #define DLL_INIT_FUNC_END( ns )     \
                                      \
        int _init( void )             \
        {                             \
                return 1;             \
        }                             \
                                      \
        int _fini( void )             \
        {                             \
                if ( !ns::Unload() )  \
                {                     \
                        return 0;     \
                }                     \
                return 1;             \
        }                             
          
        
  #define DLL_INIT_FUNC_BEG_END( ns ) \
                                      \
        int _init( void )             \
        {                             \
                if ( !ns::Load() )    \
                {                     \
                        return 0;     \
                }                     \
                return 1;             \
        }                             \
                                      \
        int _fini( void )             \
        {                             \
                if ( !ns::Unload() )  \
                {                     \
                        return 0;     \
                }                     \
                return 1;             \
        }                             
          
        
#else
  #error No DLL entry points available for the target platform
#endif

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_AUTH_DLLINIT_H ? */

