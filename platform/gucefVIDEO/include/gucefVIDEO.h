/*
 *  gucefVIDEO: GUCEF module providing video decoding and playback
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

#ifndef GUCEFVIDEO_H
#define GUCEFVIDEO_H

/*
 *      This is the main include header for the gucefVIDEO library.
 *      It is part of a framework called GUCEF.
 *      All classes in this library have to do with video: formats and frames, sources and codecs
 *      (codecs come as plugins), sharing a video between its pictures and its sound, and a player.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VIDEO_ETYPES_H
#include "gucefVIDEO_ETypes.h"
#define GUCEF_VIDEO_ETYPES_H
#endif /* GUCEF_VIDEO_ETYPES_H ? */

#ifndef GUCEF_VIDEO_CONFIG_H
#include "gucefVIDEO_config.h"
#define GUCEF_VIDEO_CONFIG_H
#endif /* GUCEF_VIDEO_CONFIG_H ? */

#ifndef GUCEF_VIDEO_MACROS_H
#include "gucefVIDEO_macros.h"
#define GUCEF_VIDEO_MACROS_H
#endif /* GUCEF_VIDEO_MACROS_H ? */

#ifndef GUCEF_VIDEO_CVIDEOFORMAT_H
#include "gucefVIDEO_CVideoFormat.h"
#define GUCEF_VIDEO_CVIDEOFORMAT_H
#endif /* GUCEF_VIDEO_CVIDEOFORMAT_H ? */

#ifndef GUCEF_VIDEO_CVIDEOFRAME_H
#include "gucefVIDEO_CVideoFrame.h"
#define GUCEF_VIDEO_CVIDEOFRAME_H
#endif /* GUCEF_VIDEO_CVIDEOFRAME_H ? */

#ifndef GUCEF_VIDEO_IVIDEOSOURCE_H
#include "gucefVIDEO_IVideoSource.h"
#define GUCEF_VIDEO_IVIDEOSOURCE_H
#endif /* GUCEF_VIDEO_IVIDEOSOURCE_H ? */

#ifndef GUCEF_VIDEO_IVIDEOSINK_H
#include "gucefVIDEO_IVideoSink.h"
#define GUCEF_VIDEO_IVIDEOSINK_H
#endif /* GUCEF_VIDEO_IVIDEOSINK_H ? */

#ifndef GUCEF_VIDEO_CIVIDEOCODEC_H
#include "gucefVIDEO_CIVideoCodec.h"
#define GUCEF_VIDEO_CIVIDEOCODEC_H
#endif /* GUCEF_VIDEO_CIVIDEOCODEC_H ? */

#ifndef GUCEF_VIDEO_CVIDEOCODECREGISTRY_H
#include "gucefVIDEO_CVideoCodecRegistry.h"
#define GUCEF_VIDEO_CVIDEOCODECREGISTRY_H
#endif /* GUCEF_VIDEO_CVIDEOCODECREGISTRY_H ? */

#ifndef GUCEF_VIDEO_CVIDEOCODECCPLUGIN_H
#include "gucefVIDEO_CVideoCodecCPlugin.h"
#define GUCEF_VIDEO_CVIDEOCODECCPLUGIN_H
#endif /* GUCEF_VIDEO_CVIDEOCODECCPLUGIN_H ? */

#ifndef GUCEF_VIDEO_CVIDEOCODECCPLUGINCODEC_H
#include "gucefVIDEO_CVideoCodecCPluginCodec.h"
#define GUCEF_VIDEO_CVIDEOCODECCPLUGINCODEC_H
#endif /* GUCEF_VIDEO_CVIDEOCODECCPLUGINCODEC_H ? */

#ifndef GUCEF_VIDEO_CVIDEOCODECPLUGINMANAGER_H
#include "gucefVIDEO_CVideoCodecPluginManager.h"
#define GUCEF_VIDEO_CVIDEOCODECPLUGINMANAGER_H
#endif /* GUCEF_VIDEO_CVIDEOCODECPLUGINMANAGER_H ? */

#ifndef GUCEF_VIDEO_CVIDEODEMUXER_H
#include "gucefVIDEO_CVideoDemuxer.h"
#define GUCEF_VIDEO_CVIDEODEMUXER_H
#endif /* GUCEF_VIDEO_CVIDEODEMUXER_H ? */

#ifndef GUCEF_VIDEO_CVIDEOPLAYER_H
#include "gucefVIDEO_CVideoPlayer.h"
#define GUCEF_VIDEO_CVIDEOPLAYER_H
#endif /* GUCEF_VIDEO_CVIDEOPLAYER_H ? */

#ifndef GUCEF_VIDEO_CVIDEOTRANSCODER_H
#include "gucefVIDEO_CVideoTranscoder.h"
#define GUCEF_VIDEO_CVIDEOTRANSCODER_H
#endif /* GUCEF_VIDEO_CVIDEOTRANSCODER_H ? */

#ifndef GUCEF_VIDEO_CVIDEOGLOBAL_H
#include "gucefVIDEO_CVideoGlobal.h"
#define GUCEF_VIDEO_CVIDEOGLOBAL_H
#endif /* GUCEF_VIDEO_CVIDEOGLOBAL_H ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEFVIDEO_H ? */
