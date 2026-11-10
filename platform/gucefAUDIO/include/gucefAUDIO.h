/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#ifndef GUCEFAUDIO_H
#define GUCEFAUDIO_H

/*
 *      This is the main include header for the gucefAUDIO library.
 *      It is part of a framework called GUCEF.
 *      All classes in this library have to do with audio: formats and buffers, conversion,
 *      sources, codecs, audio devices through drivers and the engine that mixes and plays.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_ETYPES_H
#include "gucefAUDIO_ETypes.h"
#define GUCEF_AUDIO_ETYPES_H
#endif /* GUCEF_AUDIO_ETYPES_H ? */

#ifndef GUCEF_AUDIO_CONFIG_H
#include "gucefAUDIO_config.h"
#define GUCEF_AUDIO_CONFIG_H
#endif /* GUCEF_AUDIO_CONFIG_H ? */

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

#ifndef GUCEF_AUDIO_CAUDIOFORMAT_H
#include "gucefAUDIO_CAudioFormat.h"
#define GUCEF_AUDIO_CAUDIOFORMAT_H
#endif /* GUCEF_AUDIO_CAUDIOFORMAT_H ? */

#ifndef GUCEF_AUDIO_CAUDIOBUFFER_H
#include "gucefAUDIO_CAudioBuffer.h"
#define GUCEF_AUDIO_CAUDIOBUFFER_H
#endif /* GUCEF_AUDIO_CAUDIOBUFFER_H ? */

#ifndef GUCEF_AUDIO_IAUDIORESAMPLER_H
#include "gucefAUDIO_IAudioResampler.h"
#define GUCEF_AUDIO_IAUDIORESAMPLER_H
#endif /* GUCEF_AUDIO_IAUDIORESAMPLER_H ? */

#ifndef GUCEF_AUDIO_CLINEARRESAMPLER_H
#include "gucefAUDIO_CLinearResampler.h"
#define GUCEF_AUDIO_CLINEARRESAMPLER_H
#endif /* GUCEF_AUDIO_CLINEARRESAMPLER_H ? */

#ifndef GUCEF_AUDIO_CAUDIOCONVERTER_H
#include "gucefAUDIO_CAudioConverter.h"
#define GUCEF_AUDIO_CAUDIOCONVERTER_H
#endif /* GUCEF_AUDIO_CAUDIOCONVERTER_H ? */

#ifndef GUCEF_AUDIO_IAUDIOSOURCE_H
#include "gucefAUDIO_IAudioSource.h"
#define GUCEF_AUDIO_IAUDIOSOURCE_H
#endif /* GUCEF_AUDIO_IAUDIOSOURCE_H ? */

#ifndef GUCEF_AUDIO_CAUDIOCLIP_H
#include "gucefAUDIO_CAudioClip.h"
#define GUCEF_AUDIO_CAUDIOCLIP_H
#endif /* GUCEF_AUDIO_CAUDIOCLIP_H ? */

#ifndef GUCEF_AUDIO_CPCMSTREAMSOURCE_H
#include "gucefAUDIO_CPcmStreamSource.h"
#define GUCEF_AUDIO_CPCMSTREAMSOURCE_H
#endif /* GUCEF_AUDIO_CPCMSTREAMSOURCE_H ? */

#ifndef GUCEF_AUDIO_CIAUDIOCODEC_H
#include "gucefAUDIO_CIAudioCodec.h"
#define GUCEF_AUDIO_CIAUDIOCODEC_H
#endif /* GUCEF_AUDIO_CIAUDIOCODEC_H ? */

#ifndef GUCEF_AUDIO_CAUDIOCODECREGISTRY_H
#include "gucefAUDIO_CAudioCodecRegistry.h"
#define GUCEF_AUDIO_CAUDIOCODECREGISTRY_H
#endif /* GUCEF_AUDIO_CAUDIOCODECREGISTRY_H ? */

#ifndef GUCEF_AUDIO_CWAVAUDIOCODEC_H
#include "gucefAUDIO_CWavAudioCodec.h"
#define GUCEF_AUDIO_CWAVAUDIOCODEC_H
#endif /* GUCEF_AUDIO_CWAVAUDIOCODEC_H ? */

#ifndef GUCEF_AUDIO_CRAWAUDIOCODEC_H
#include "gucefAUDIO_CRawAudioCodec.h"
#define GUCEF_AUDIO_CRAWAUDIOCODEC_H
#endif /* GUCEF_AUDIO_CRAWAUDIOCODEC_H ? */

#ifndef GUCEF_AUDIO_CAUDIOCODECCPLUGIN_H
#include "gucefAUDIO_CAudioCodecCPlugin.h"
#define GUCEF_AUDIO_CAUDIOCODECCPLUGIN_H
#endif /* GUCEF_AUDIO_CAUDIOCODECCPLUGIN_H ? */

#ifndef GUCEF_AUDIO_CAUDIOCODECCPLUGINCODEC_H
#include "gucefAUDIO_CAudioCodecCPluginCodec.h"
#define GUCEF_AUDIO_CAUDIOCODECCPLUGINCODEC_H
#endif /* GUCEF_AUDIO_CAUDIOCODECCPLUGINCODEC_H ? */

#ifndef GUCEF_AUDIO_CAUDIOCODECPLUGINMANAGER_H
#include "gucefAUDIO_CAudioCodecPluginManager.h"
#define GUCEF_AUDIO_CAUDIOCODECPLUGINMANAGER_H
#endif /* GUCEF_AUDIO_CAUDIOCODECPLUGINMANAGER_H ? */

#ifndef GUCEF_AUDIO_CAUDIODEVICEINFO_H
#include "gucefAUDIO_CAudioDeviceInfo.h"
#define GUCEF_AUDIO_CAUDIODEVICEINFO_H
#endif /* GUCEF_AUDIO_CAUDIODEVICEINFO_H ? */

#ifndef GUCEF_AUDIO_CAUDIODEVICEPARAMS_H
#include "gucefAUDIO_CAudioDeviceParams.h"
#define GUCEF_AUDIO_CAUDIODEVICEPARAMS_H
#endif /* GUCEF_AUDIO_CAUDIODEVICEPARAMS_H ? */

#ifndef GUCEF_AUDIO_IAUDIODEVICECALLBACK_H
#include "gucefAUDIO_IAudioDeviceCallback.h"
#define GUCEF_AUDIO_IAUDIODEVICECALLBACK_H
#endif /* GUCEF_AUDIO_IAUDIODEVICECALLBACK_H ? */

#ifndef GUCEF_AUDIO_CAUDIODEVICE_H
#include "gucefAUDIO_CAudioDevice.h"
#define GUCEF_AUDIO_CAUDIODEVICE_H
#endif /* GUCEF_AUDIO_CAUDIODEVICE_H ? */

#ifndef GUCEF_AUDIO_CAUDIODRIVER_H
#include "gucefAUDIO_CAudioDriver.h"
#define GUCEF_AUDIO_CAUDIODRIVER_H
#endif /* GUCEF_AUDIO_CAUDIODRIVER_H ? */

#ifndef GUCEF_AUDIO_CAUDIODRIVERREGISTRY_H
#include "gucefAUDIO_CAudioDriverRegistry.h"
#define GUCEF_AUDIO_CAUDIODRIVERREGISTRY_H
#endif /* GUCEF_AUDIO_CAUDIODRIVERREGISTRY_H ? */

#ifndef GUCEF_AUDIO_CNULLAUDIODRIVER_H
#include "gucefAUDIO_CNullAudioDriver.h"
#define GUCEF_AUDIO_CNULLAUDIODRIVER_H
#endif /* GUCEF_AUDIO_CNULLAUDIODRIVER_H ? */

#ifndef GUCEF_AUDIO_CAUDIOPUSHDEVICE_H
#include "gucefAUDIO_CAudioPushDevice.h"
#define GUCEF_AUDIO_CAUDIOPUSHDEVICE_H
#endif /* GUCEF_AUDIO_CAUDIOPUSHDEVICE_H ? */

#ifndef GUCEF_AUDIO_CAUDIOSETTINGS_H
#include "gucefAUDIO_CAudioSettings.h"
#define GUCEF_AUDIO_CAUDIOSETTINGS_H
#endif /* GUCEF_AUDIO_CAUDIOSETTINGS_H ? */

#ifndef GUCEF_AUDIO_CAUDIOBUSLAYOUT_H
#include "gucefAUDIO_CAudioBusLayout.h"
#define GUCEF_AUDIO_CAUDIOBUSLAYOUT_H
#endif /* GUCEF_AUDIO_CAUDIOBUSLAYOUT_H ? */

#ifndef GUCEF_AUDIO_CAUDIOSTREAM_H
#include "gucefAUDIO_CAudioStream.h"
#define GUCEF_AUDIO_CAUDIOSTREAM_H
#endif /* GUCEF_AUDIO_CAUDIOSTREAM_H ? */

#ifndef GUCEF_AUDIO_CTONESEQUENCESOURCE_H
#include "gucefAUDIO_CToneSequenceSource.h"
#define GUCEF_AUDIO_CTONESEQUENCESOURCE_H
#endif /* GUCEF_AUDIO_CTONESEQUENCESOURCE_H ? */

#ifndef GUCEF_AUDIO_CATOMICFLOAT_H
#include "gucefAUDIO_CAtomicFloat.h"
#define GUCEF_AUDIO_CATOMICFLOAT_H
#endif /* GUCEF_AUDIO_CATOMICFLOAT_H ? */

#ifndef GUCEF_AUDIO_CAUDIOBUS_H
#include "gucefAUDIO_CAudioBus.h"
#define GUCEF_AUDIO_CAUDIOBUS_H
#endif /* GUCEF_AUDIO_CAUDIOBUS_H ? */

#ifndef GUCEF_AUDIO_CAUDIOVOICE_H
#include "gucefAUDIO_CAudioVoice.h"
#define GUCEF_AUDIO_CAUDIOVOICE_H
#endif /* GUCEF_AUDIO_CAUDIOVOICE_H ? */

#ifndef GUCEF_AUDIO_CAUDIOMIXER_H
#include "gucefAUDIO_CAudioMixer.h"
#define GUCEF_AUDIO_CAUDIOMIXER_H
#endif /* GUCEF_AUDIO_CAUDIOMIXER_H ? */

#ifndef GUCEF_AUDIO_CAUDIOENGINEPARAMS_H
#include "gucefAUDIO_CAudioEngineParams.h"
#define GUCEF_AUDIO_CAUDIOENGINEPARAMS_H
#endif /* GUCEF_AUDIO_CAUDIOENGINEPARAMS_H ? */

#ifndef GUCEF_AUDIO_CAUDIOENGINE_H
#include "gucefAUDIO_CAudioEngine.h"
#define GUCEF_AUDIO_CAUDIOENGINE_H
#endif /* GUCEF_AUDIO_CAUDIOENGINE_H ? */

#ifndef GUCEF_AUDIO_CAUDIOGLOBAL_H
#include "gucefAUDIO_CAudioGlobal.h"
#define GUCEF_AUDIO_CAUDIOGLOBAL_H
#endif /* GUCEF_AUDIO_CAUDIOGLOBAL_H ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEFAUDIO_H ? */
