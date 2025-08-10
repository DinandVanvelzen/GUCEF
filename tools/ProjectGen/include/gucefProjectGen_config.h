/*
 *  ProjectGen: GUCEF module providing the business logic to generate module/project files
 *
 *  Copyright (C) 1998 - 2025.  Dinand Vanvelzen
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

#ifndef GUCEF_PROJECTGEN_CONFIG_H
#define GUCEF_PROJECTGEN_CONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      BUILD DEFINES                                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      If we are building or using a DLL then it is sometimes desireable to
 *      export only C code or perhaps only C++ code. You can do this with the
 *      following defines
 */
#undef GUCEF_PROJECTGEN_PUBLIC_CPP_CODE        /* do not change this line */ 
#undef GUCEF_PROJECTGEN_PUBLIC_C_CODE          /* do not change this line */  
#define GUCEF_PROJECTGEN_PUBLIC_CPP_CODE /* do you want to enable the C++ exports ? */
#define GUCEF_PROJECTGEN_PUBLIC_C_CODE   /* do you want to enable the C exports ? */

/*-------------------------------------------------------------------------*/

/*
 *      Switches for setting the calling convention used by the gucefPROJECTGEN
 *      module. Please enable only 1, no more, no less.
 */
#undef USE_CALLSPEC_C                /* do not change this line */
#undef USE_CALLSPEC_STD              /* do not change this line */ 
#undef USE_CALLSPEC_PASCAL           /* do not change this line */ 
#undef USE_CALLSPEC_FAST             /* do not change this line */
#undef USE_PLUGIN_CALLSPEC_C         /* do not change this line */
#undef USE_PLUGIN_CALLSPEC_STD       /* do not change this line */ 
#undef USE_PLUGIN_CALLSPEC_PASCAL    /* do not change this line */ 
#undef USE_PLUGIN_CALLSPEC_FAST      /* do not change this line */
  
#define USE_CALLSPEC_C 
/* #define USE_CALLSPEC_STD */
/* #define USE_CALLSPEC_PASCAL */
/* #define USE_CALLSPEC_FAST */
/* #define USE_PLUGIN_CALLSPEC_C */
#define USE_PLUGIN_CALLSPEC_STD
/* #define USE_PLUGIN_CALLSPEC_PASCAL */
/* #define USE_PLUGIN_CALLSPEC_FAST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PROJECTGEN_CONFIG_H */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 16-10-2004 :
       - Dinand: Initial version of this file.

-----------------------------------------------------------------------------*/
