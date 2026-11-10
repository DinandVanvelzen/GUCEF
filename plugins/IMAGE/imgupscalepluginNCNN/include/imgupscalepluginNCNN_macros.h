/*
 *  imgupscalepluginNCNN: GUCEF image plugin that upscales pictures with Real-ESRGAN neural networks running on ncnn
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

#ifndef GUCEF_IMGUPSCALEPLUGINNCNN_MACROS_H
#define GUCEF_IMGUPSCALEPLUGINNCNN_MACROS_H

#ifndef GUCEF_IMAGE_MACROS_H
#include "gucefIMAGE_macros.h"
#define GUCEF_IMAGE_MACROS_H
#endif /* GUCEF_IMAGE_MACROS_H ? */

#ifndef GUCEF_IMGUPSCALEPLUGINNCNN_CONFIG_H
#include "imgupscalepluginNCNN_config.h"
#define GUCEF_IMGUPSCALEPLUGINNCNN_CONFIG_H
#endif /* GUCEF_IMGUPSCALEPLUGINNCNN_CONFIG_H ? */

#undef IMGUPSCALEPLUGINNCNN_EXPORTSPEC
#ifdef IMGUPSCALEPLUGINNCNN_BUILD_MODULE
  #define IMGUPSCALEPLUGINNCNN_EXPORTSPEC GUCEF_EXPORT
#else
  #define IMGUPSCALEPLUGINNCNN_EXPORTSPEC GUCEF_IMPORT
#endif /* IMGUPSCALEPLUGINNCNN_BUILD_MODULE ? */

#undef IMGUPSCALEPLUGINNCNN_PUBLIC_CPP
#ifdef IMGUPSCALEPLUGINNCNN_PUBLIC_CPP_CODE
  #define IMGUPSCALEPLUGINNCNN_PUBLIC_CPP IMGUPSCALEPLUGINNCNN_EXPORTSPEC
#else
  #define IMGUPSCALEPLUGINNCNN_PUBLIC_CPP GUCEF_HIDDEN
#endif /* IMGUPSCALEPLUGINNCNN_PUBLIC_CPP_CODE ? */

#undef IMGUPSCALEPLUGINNCNN_PUBLIC_C
#ifdef IMGUPSCALEPLUGINNCNN_PUBLIC_C_CODE
  #define IMGUPSCALEPLUGINNCNN_PUBLIC_C IMGUPSCALEPLUGINNCNN_EXPORTSPEC
#else
  #define IMGUPSCALEPLUGINNCNN_PUBLIC_C GUCEF_HIDDEN
#endif /* IMGUPSCALEPLUGINNCNN_PUBLIC_C_CODE ? */

#endif /* GUCEF_IMGUPSCALEPLUGINNCNN_MACROS_H ? */
