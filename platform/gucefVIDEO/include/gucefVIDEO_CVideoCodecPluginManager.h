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

#ifndef GUCEF_VIDEO_CVIDEOCODECPLUGINMANAGER_H
#define GUCEF_VIDEO_CVIDEOCODECPLUGINMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CPLUGINMANAGER_H
#include "CPluginManager.h"
#define GUCEF_CORE_CPLUGINMANAGER_H
#endif /* GUCEF_CORE_CPLUGINMANAGER_H ? */

#ifndef GUCEF_VIDEO_CVIDEOCODECCPLUGIN_H
#include "gucefVIDEO_CVideoCodecCPlugin.h"
#define GUCEF_VIDEO_CVIDEOCODECCPLUGIN_H
#endif /* GUCEF_VIDEO_CVIDEOCODECCPLUGIN_H ? */

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

class CVideoGlobal;

/*-------------------------------------------------------------------------*/

/**
 *  Plugin manager for video codec plugins with the C interface (plugin type "GucefVideoCodecPlugin").
 *  Each codec type of a loaded plugin is registered in the codec registry, where it is found by its type.
 *  Types that are already taken (by another plugin) are left to their owner.
 *  Reach it through CVideoGlobal.
 */
class GUCEF_VIDEO_PUBLIC_CPP CVideoCodecPluginManager : public CORE::CPluginManager
{
    public:

    typedef CVideoCodecCPlugin::TVideoCodecCPluginPtr TVideoCodecCPluginPtr;

    static const CString PluginType;

    virtual CString GetPluginType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The plugin that offers the given codec type, NULL when no loaded plugin does */
    TVideoCodecCPluginPtr GetPluginForCodecType( const CString& codecType ) const;

    protected:

    virtual CORE::TPluginPtr RegisterPlugin( void* modulePtr, CORE::TPluginMetaDataPtr pluginMetaData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool UnregisterPlugin( CORE::TPluginPtr plugin ) GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CVideoGlobal;

    CVideoCodecPluginManager( void );

    virtual ~CVideoCodecPluginManager() GUCEF_VIRTUAL_OVERRIDE;

    /** At shutdown: takes the codecs of all plugins out of the registry and unlinks the plugins */
    void UnlinkAll( void );

    void UnregisterCodecs( CVideoCodecCPlugin& plugin );

    CVideoCodecPluginManager( const CVideoCodecPluginManager& src );              /**< not implemented */
    CVideoCodecPluginManager& operator=( const CVideoCodecPluginManager& src );   /**< not implemented */

    private:

    typedef GUCEF::set< TVideoCodecCPluginPtr > TPluginSet;

    TPluginSet m_plugins;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VIDEO_CVIDEOCODECPLUGINMANAGER_H ? */
