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

#ifndef GUCEF_VIDEO_CVIDEOCODECCPLUGINCODEC_H
#define GUCEF_VIDEO_CVIDEOCODECCPLUGINCODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VIDEO_CIVIDEOCODEC_H
#include "gucefVIDEO_CIVideoCodec.h"
#define GUCEF_VIDEO_CIVIDEOCODEC_H
#endif /* GUCEF_VIDEO_CIVIDEOCODEC_H ? */

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

class CVideoCodecCPluginInstance;

/*-------------------------------------------------------------------------*/

/**
 *  One codec type of a C codec plugin, as a CIVideoCodec. The plugin manager makes one per type the plugin
 *  offers and registers it in the codec registry. Sources and sinks it opens keep the plugin codec instance
 *  alive, and end (instead of failing badly) when the plugin is unloaded while they are open.
 */
class GUCEF_VIDEO_PUBLIC_CPP CVideoCodecCPluginCodec : public CIVideoCodec
{
    public:

    typedef CVideoCodecCPlugin::TVideoCodecCPluginPtr          TVideoCodecCPluginPtr;
    typedef CORE::CTSharedPtr< CVideoCodecCPluginInstance, MT::CMutex > TCodecInstancePtr;

    /** Creates the codec instance in the plugin. False when the plugin refuses the type */
    static bool Create( const TVideoCodecCPluginPtr& plugin, const CString& codecType, CORE::CTBasicSharedPtr< CIVideoCodec, MT::CMutex >& codec );

    CVideoCodecCPluginCodec( const CVideoCodecCPluginCodec& src );

    virtual ~CVideoCodecCPluginCodec() GUCEF_VIRTUAL_OVERRIDE;

    /** The format, length and audio tracks, from opening the video, plus whatever the plugin reads as tags */
    virtual bool DecodeMetaData( CORE::CIOAccess& encodedInput, CORE::CValueList& metaData ) GUCEF_VIRTUAL_OVERRIDE;

    /** True when the plugin has the sink functions */
    virtual bool CanEncode( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    const TVideoCodecCPluginPtr& GetPlugin( void ) const;

    protected:

    virtual bool OpenSourceImpl( CORE::CIOAccess* input, const TIOAccessPtr& owner, const CORE::CValueList& options, TVideoSourcePtr& source ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OpenSinkImpl( CORE::CIOAccess* output, const TIOAccessPtr& owner, const CVideoFormat& format, const TAudioFormatVector& audioTrackFormats, const CORE::CValueList& options, TVideoSinkPtr& sink ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CVideoCodecCPluginCodec( const TVideoCodecCPluginPtr& plugin, const CString& codecType, const TCodecInstancePtr& instance );

    CVideoCodecCPluginCodec& operator=( const CVideoCodecCPluginCodec& src );   /**< not implemented */

    private:

    TVideoCodecCPluginPtr m_plugin;
    CString m_codecType;
    TCodecInstancePtr m_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VIDEO_CVIDEOCODECCPLUGINCODEC_H ? */
