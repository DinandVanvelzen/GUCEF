/*
 *  audiocodecpluginFLAC: gucefAUDIO codec plugin for FLAC audio
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

#ifndef GUCEF_AUDIOCODECPLUGIN_FLAC_INTERNAL_H
#define GUCEF_AUDIOCODECPLUGIN_FLAC_INTERNAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_C_AUDIOCODEC_PLUGIN_API_H
#include "gucefAUDIO_c_audiocodec_plugin_api.h"     /* the interface this plugin implements */
#define GUCEF_AUDIO_C_AUDIOCODEC_PLUGIN_API_H
#endif /* GUCEF_AUDIO_C_AUDIOCODEC_PLUGIN_API_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  Writes the frames as a FLAC file: fixed predictors of order 0 to 4 with Rice coding, constant and verbatim
 *  sub frames where those are smaller, independent channels, up to 8 channels and 24 bits per sample.
 *  Options (text): "blocksize" frames per FLAC frame (16 to 65535, default 4096), "bits" bits per sample for 32 bit
 *  and float input (8 to 24, default 24 for 32 bit integers and 16 for float), "tags" VORBIS_COMMENT entries as
 *  lines of NAME=value separated by line feeds. A channel layout other than the default for the channel count is
 *  stored as the WAVEFORMATEXTENSIBLE_CHANNEL_MASK tag.
 */
UInt32
FlacEncode( const TGucefCoreCApi* libApi, const TAudioFormatInfo* format, const void* frames, UInt64 frameCount, TIOAccess* output, const TVariantMapApi* options );

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIOCODECPLUGIN_FLAC_INTERNAL_H ? */
