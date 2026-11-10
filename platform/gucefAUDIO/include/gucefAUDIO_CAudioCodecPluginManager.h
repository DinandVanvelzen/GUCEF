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

#ifndef GUCEF_AUDIO_CAUDIOCODECPLUGINMANAGER_H
#define GUCEF_AUDIO_CAUDIOCODECPLUGINMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CPLUGINMANAGER_H
#include "CPluginManager.h"
#define GUCEF_CORE_CPLUGINMANAGER_H
#endif /* GUCEF_CORE_CPLUGINMANAGER_H ? */

#ifndef GUCEF_AUDIO_CAUDIOCODECCPLUGIN_H
#include "gucefAUDIO_CAudioCodecCPlugin.h"
#define GUCEF_AUDIO_CAUDIOCODECCPLUGIN_H
#endif /* GUCEF_AUDIO_CAUDIOCODECCPLUGIN_H ? */

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

class CAudioGlobal;

/*-------------------------------------------------------------------------*/

/**
 *  Plugin manager for audio codec plugins with the C interface (plugin type "GucefAudioCodecPlugin").
 *  Each codec type of a loaded plugin is registered in the codec registry, so it is found the same way as the
 *  built in codecs. Types that are already taken (a built in "wav", another plugin) are left to their owner.
 *  Reach it through CAudioGlobal.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioCodecPluginManager : public CORE::CPluginManager
{
    public:

    typedef CAudioCodecCPlugin::TAudioCodecCPluginPtr TAudioCodecCPluginPtr;

    static const CString PluginType;

    virtual CString GetPluginType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The plugin that offers the given codec type, NULL when no loaded plugin does */
    TAudioCodecCPluginPtr GetPluginForCodecType( const CString& codecType ) const;

    protected:

    virtual CORE::TPluginPtr RegisterPlugin( void* modulePtr, CORE::TPluginMetaDataPtr pluginMetaData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool UnregisterPlugin( CORE::TPluginPtr plugin ) GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CAudioGlobal;

    CAudioCodecPluginManager( void );

    virtual ~CAudioCodecPluginManager() GUCEF_VIRTUAL_OVERRIDE;

    /** At shutdown: takes the codecs of all plugins out of the registry and unlinks the plugins */
    void UnlinkAll( void );

    void UnregisterCodecs( CAudioCodecCPlugin& plugin );

    CAudioCodecPluginManager( const CAudioCodecPluginManager& src );              /**< not implemented */
    CAudioCodecPluginManager& operator=( const CAudioCodecPluginManager& src );   /**< not implemented */

    private:

    typedef GUCEF::set< TAudioCodecCPluginPtr > TPluginSet;

    TPluginSet m_plugins;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOCODECPLUGINMANAGER_H ? */
