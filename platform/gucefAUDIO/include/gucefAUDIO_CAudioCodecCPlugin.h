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

#ifndef GUCEF_AUDIO_CAUDIOCODECCPLUGIN_H
#define GUCEF_AUDIO_CAUDIOCODECCPLUGIN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CIPLUGIN_H
#include "CIPlugin.h"
#define GUCEF_CORE_CIPLUGIN_H
#endif /* GUCEF_CORE_CIPLUGIN_H ? */

#ifndef GUCEF_CORE_CPLUGINMETADATA_H
#include "gucefCORE_CPluginMetaData.h"
#define GUCEF_CORE_CPLUGINMETADATA_H
#endif /* GUCEF_CORE_CPLUGINMETADATA_H ? */

#ifndef GUCEF_AUDIO_C_AUDIOCODEC_PLUGIN_API_H
#include "gucefAUDIO_c_audiocodec_plugin_api.h"
#define GUCEF_AUDIO_C_AUDIOCODEC_PLUGIN_API_H
#endif /* GUCEF_AUDIO_C_AUDIOCODEC_PLUGIN_API_H ? */

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

class CAudioCodecPluginManager;

/*-------------------------------------------------------------------------*/

/**
 *  Something that holds state of a C codec plugin (a codec instance, an open stream) and has to give it back
 *  when the plugin goes away while the holder still lives.
 */
class GUCEF_AUDIO_PRIVATE_CPP CIAudioCodecCPluginClient
{
    public:

    virtual ~CIAudioCodecCPluginClient();

    /** The plugin is being unlinked: release the plugin state now. Called with the call lock of the plugin held */
    virtual void OnPluginUnlink( const TGucefAudioCodecPluginApi& api, void** plugdata ) = 0;
};

/*-------------------------------------------------------------------------*/

/**
 *  The host side of an audio codec plugin with the C interface of gucefAUDIO_c_audiocodec_plugin_api.h.
 *  Finds the functions of the module, initializes it and offers its codec types; the plugin manager registers
 *  a CIAudioCodec per type in the codec registry so plugin codecs look like the built in ones.
 *
 *  Every call into the module goes through the call lock, so the module sees one thread at a time and can be
 *  unlinked while streams are still open: unlinking closes the open streams and codec instances first, after
 *  which they act as ended instead of calling into a module that is gone.
 *
 *  A module that declares GUCEF_AUDIOCODEC_THREADING_PER_SOURCE (AUDIOCODECPLUG_Threading_Model) is not called
 *  under the call lock for the calls of open streams (reading, seeking, tags): the stream takes its own lock,
 *  so streams of that module decode at the same time. Everything else still goes through the call lock.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioCodecCPlugin : public CORE::CIPlugin
{
    public:

    typedef CORE::CTSharedPtr< CAudioCodecCPlugin, MT::CMutex > TAudioCodecCPluginPtr;
    typedef GUCEF::vector< CString > TStringVector;

    CAudioCodecCPlugin( void );

    virtual ~CAudioCodecCPlugin() GUCEF_VIRTUAL_OVERRIDE;

    /** Finds the functions of the module and initializes it. False when the module is not an audio codec plugin */
    bool Link( void* modulePtr, CORE::TPluginMetaDataPtr pluginMetaData );

    /** Releases everything that still uses the module and shuts the plugin down */
    bool Unlink( void );

    bool IsLinked( void ) const;

    virtual CORE::TPluginMetaDataPtr GetMetaData( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void* GetModulePointer( void ) GUCEF_VIRTUAL_OVERRIDE;

    CString GetName( void ) const;

    CString GetCopyright( void ) const;

    CORE::TVersion GetVersion( void ) const;

    /** The codec types the plugin offers ("au", "ogg", ...) */
    TStringVector GetCodecTypes( void ) const;

    /** GUCEF_AUDIOCODEC_THREADING_PLUGIN_LOCK or GUCEF_AUDIOCODEC_THREADING_PER_SOURCE, as the module declared. Fixed once linked */
    UInt32 GetThreadingModel( void ) const;

    /** The codec types that ended up in the codec registry */
    const TStringVector& GetRegisteredCodecTypes( void ) const;

    /** Held around every call into the module */
    const MT::CMutex& GetCallLock( void ) const;

    /** The functions of the module, NULL once unlinked. Only use while holding the call lock */
    const TGucefAudioCodecPluginApi* GetLinkedApi( void ) const;

    /** The plugin state to pass to every call. Only use while holding the call lock */
    void** GetPlugData( void );

    /** Codec instances and open streams register so they can be released when the plugin goes. Call with the lock held */
    void RegisterClient( CIAudioCodecCPluginClient* client, bool isStream );

    void UnregisterClient( CIAudioCodecCPluginClient* client );

    private:
    friend class CAudioCodecPluginManager;

    void SetRegisteredCodecTypes( const TStringVector& codecTypes );

    CAudioCodecCPlugin( const CAudioCodecCPlugin& src );              /**< not implemented */
    CAudioCodecCPlugin& operator=( const CAudioCodecCPlugin& src );   /**< not implemented */

    private:

    typedef GUCEF::set< CIAudioCodecCPluginClient* > TClientSet;

    TGucefAudioCodecPluginApi m_api;
    void* m_moduleHandle;
    void* m_plugdata;
    bool m_isLinked;
    UInt32 m_threadingModel;
    CORE::TPluginMetaDataStoragePtr m_metaData;
    TStringVector m_registeredCodecTypes;
    TClientSet m_streams;
    TClientSet m_codecInstances;
    MT::CMutex m_callLock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOCODECCPLUGIN_H ? */
