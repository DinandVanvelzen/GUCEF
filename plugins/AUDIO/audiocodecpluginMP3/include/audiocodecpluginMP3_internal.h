/*
 *  audiocodecpluginMP3: gucefAUDIO codec plugin for MP3 audio
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

#ifndef GUCEF_AUDIOCODECPLUGIN_MP3_INTERNAL_H
#define GUCEF_AUDIOCODECPLUGIN_MP3_INTERNAL_H

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
 *  Puts the text frames of an ID3v2 tag ('data' is the whole tag, header included, 'size' its size) into a map of
 *  the host as lower case names: title, artist, album, albumartist, composer, track, disc, date and genre, and the
 *  user text frames (TXXX) under their own description. Versions 2.2, 2.3 and 2.4. Text is delivered as UTF-8.
 *  Returns the number of entries it added.
 */
UInt32
Mp3AddId3v2Tags( TVariantMapApi* map, const UInt8* data, UInt32 size );

/**
 *  Same for an ID3v1 tag, which is always 128 bytes: title, artist, album, date, track and a comment
 */
UInt32
Mp3AddId3v1Tags( TVariantMapApi* map, const UInt8* data );

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIOCODECPLUGIN_MP3_INTERNAL_H ? */
