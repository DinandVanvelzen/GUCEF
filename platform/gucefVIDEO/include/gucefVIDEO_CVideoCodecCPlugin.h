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

#ifndef GUCEF_VIDEO_CVIDEOCODECCPLUGIN_H
#define GUCEF_VIDEO_CVIDEOCODECCPLUGIN_H

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

#ifndef GUCEF_VIDEO_C_VIDEOCODEC_PLUGIN_API_H
#include "gucefVIDEO_c_videocodec_plugin_api.h"
#define GUCEF_VIDEO_C_VIDEOCODEC_PLUGIN_API_H
#endif /* GUCEF_VIDEO_C_VIDEOCODEC_PLUGIN_API_H ? */

#ifndef GUCEF_VIDEO_MACROS_H
#include "gucefVIDEO_macros.h"
#define GUCEF_VIDEO_MACROS_H
#endif /* GUCEF_VIDEO_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VIDEO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CVideoCodecPluginManager;

/*-------------------------------------------------------------------------*/

/**
 *  Something that holds state of a C codec plugin (a codec instance, an open video) and has to give it back
 *  when the plugin goes away while the holder still lives.
 */
class GUCEF_VIDEO_PRIVATE_CPP CIVideoCodecCPluginClient
{
    public:

    virtual ~CIVideoCodecCPluginClient();

    /** The plugin is being unlinked: release the plugin state now. Called with the call lock of the plugin held */
    virtual void OnPluginUnlink( const TGucefVideoCodecPluginApi& api, void** plugdata ) = 0;
};

/*-------------------------------------------------------------------------*/

/**
 *  The host side of an video codec plugin with the C interface of gucefVIDEO_c_videocodec_plugin_api.h.
 *  Finds the functions of the module, initializes it and offers its codec types; the plugin manager registers
 *  a CIVideoCodec per type in the codec registry so plugin codecs look like any other codec.
 *
 *  Every call into the module goes through the call lock, so the module sees one thread at a time and can be
 *  unlinked while sources are still open: unlinking closes the open sources and codec instances first, after
 *  which they act as ended instead of calling into a module that is gone.
 */
class GUCEF_VIDEO_PUBLIC_CPP CVideoCodecCPlugin : public CORE::CIPlugin
{
    public:

    typedef CORE::CTSharedPtr< CVideoCodecCPlugin, MT::CMutex > TVideoCodecCPluginPtr;
    typedef GUCEF::vector< CString > TStringVector;

    CVideoCodecCPlugin( void );

    virtual ~CVideoCodecCPlugin() GUCEF_VIRTUAL_OVERRIDE;

    /** Finds the functions of the module and initializes it. False when the module is not an video codec plugin */
    bool Link( void* modulePtr, CORE::TPluginMetaDataPtr pluginMetaData );

    /** Releases everything that still uses the module and shuts the plugin down */
    bool Unlink( void );

    bool IsLinked( void ) const;

    virtual CORE::TPluginMetaDataPtr GetMetaData( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void* GetModulePointer( void ) GUCEF_VIRTUAL_OVERRIDE;

    CString GetName( void ) const;

    CString GetCopyright( void ) const;

    CORE::TVersion GetVersion( void ) const;

    /** The codec types the plugin offers ("smk", ...) */
    TStringVector GetCodecTypes( void ) const;

    /** The codec types that ended up in the codec registry */
    const TStringVector& GetRegisteredCodecTypes( void ) const;

    /** Held around every call into the module */
    const MT::CMutex& GetCallLock( void ) const;

    /** The functions of the module, NULL once unlinked. Only use while holding the call lock */
    const TGucefVideoCodecPluginApi* GetLinkedApi( void ) const;

    /** The plugin state to pass to every call. Only use while holding the call lock */
    void** GetPlugData( void );

    /** Codec instances and open sources register so they can be released when the plugin goes. Call with the lock held */
    void RegisterClient( CIVideoCodecCPluginClient* client, bool isSource );

    void UnregisterClient( CIVideoCodecCPluginClient* client );

    private:
    friend class CVideoCodecPluginManager;

    void SetRegisteredCodecTypes( const TStringVector& codecTypes );

    CVideoCodecCPlugin( const CVideoCodecCPlugin& src );              /**< not implemented */
    CVideoCodecCPlugin& operator=( const CVideoCodecCPlugin& src );   /**< not implemented */

    private:

    typedef GUCEF::set< CIVideoCodecCPluginClient* > TClientSet;

    TGucefVideoCodecPluginApi m_api;
    void* m_moduleHandle;
    void* m_plugdata;
    bool m_isLinked;
    CORE::TPluginMetaDataStoragePtr m_metaData;
    TStringVector m_registeredCodecTypes;
    TClientSet m_sources;
    TClientSet m_codecInstances;
    MT::CMutex m_callLock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VIDEO_CVIDEOCODECCPLUGIN_H ? */
