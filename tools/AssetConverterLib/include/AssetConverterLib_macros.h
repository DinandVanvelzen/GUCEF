/*
 *  AssetConverterLib: conversion of asset packs into an engine-neutral intermediate and its cooking
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

#ifndef ASSETCONVERTERLIB_MACROS_H
#define ASSETCONVERTERLIB_MACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#ifndef ASSETCONVERTERLIB_CONFIG_H
#include "AssetConverterLib_config.h"
#define ASSETCONVERTERLIB_CONFIG_H
#endif /* ASSETCONVERTERLIB_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#undef ASSETCONVERTERLIB_EXPORTSPEC
#ifdef ASSETCONVERTERLIB_BUILD_MODULE
  #define ASSETCONVERTERLIB_EXPORTSPEC GUCEF_EXPORT
#else
  #define ASSETCONVERTERLIB_EXPORTSPEC GUCEF_IMPORT
#endif /* ASSETCONVERTERLIB_BUILD_MODULE ? */

#undef ASSETCONVERTERLIB_PUBLIC_CPP
#ifdef ASSETCONVERTERLIB_PUBLIC_CPP_CODE
  #define ASSETCONVERTERLIB_PUBLIC_CPP ASSETCONVERTERLIB_EXPORTSPEC
#else
  #define ASSETCONVERTERLIB_PUBLIC_CPP GUCEF_HIDDEN
#endif /* ASSETCONVERTERLIB_PUBLIC_CPP_CODE ? */

#define ASSETCONVERTERLIB_PRIVATE_CPP GUCEF_HIDDEN

#undef ASSETCONVERTERLIB_PUBLIC_C
#ifdef ASSETCONVERTERLIB_PUBLIC_C_CODE
  #define ASSETCONVERTERLIB_PUBLIC_C ASSETCONVERTERLIB_EXPORTSPEC
#else
  #define ASSETCONVERTERLIB_PUBLIC_C GUCEF_HIDDEN
#endif /* ASSETCONVERTERLIB_PUBLIC_C_CODE ? */

#define ASSETCONVERTERLIB_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_MACROS_H ? */
