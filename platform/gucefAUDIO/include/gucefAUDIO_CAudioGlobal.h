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

#ifndef GUCEF_AUDIO_CAUDIOGLOBAL_H
#define GUCEF_AUDIO_CAUDIOGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CGUCEFAUDIOModule;
class CAudioCodecRegistry;
class CAudioDriverRegistry;
class CAudioCodecPluginManager;
class CAudioEffectRegistry;

/*-------------------------------------------------------------------------*/

/**
 *  Singular singleton providing access to all global Audio systems.
 *  The registries for audio drivers and audio codecs are added here as they come into existence.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioGlobal
{
    public:

    static CAudioGlobal* Instance( void );

    /** All audio codecs by type name. The codecs that are part of the library are registered when the module starts */
    CAudioCodecRegistry& GetAudioCodecRegistry( void );

    /** All audio drivers by name. The null driver is always there, drivers of plugins are added when the plugin loads */
    CAudioDriverRegistry& GetAudioDriverRegistry( void );

    /** Loads audio codec plugins with the C interface; their codec types end up in the codec registry */
    CAudioCodecPluginManager& GetAudioCodecPluginManager( void );

    /** All audio effect types by type name. Built-in effects are registered when the module starts, effect plugins add theirs */
    CAudioEffectRegistry& GetAudioEffectRegistry( void );

    private:
    friend class CGUCEFAUDIOModule;

    static void Deinstance( void );

    private:

    CAudioGlobal( void );

    ~CAudioGlobal();

    void Initialize( void );

    CAudioGlobal( const CAudioGlobal& src );            /**< not implemented */
    CAudioGlobal& operator=( const CAudioGlobal& src ); /**< not implemented */

    private:

    CAudioCodecRegistry* m_audioCodecRegistry;
    CAudioDriverRegistry* m_audioDriverRegistry;
    CAudioCodecPluginManager* m_audioCodecPluginManager;
    CAudioEffectRegistry* m_audioEffectRegistry;

    static CAudioGlobal* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOGLOBAL_H ? */
