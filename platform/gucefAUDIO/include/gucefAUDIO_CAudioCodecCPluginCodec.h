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

#ifndef GUCEF_AUDIO_CAUDIOCODECCPLUGINCODEC_H
#define GUCEF_AUDIO_CAUDIOCODECCPLUGINCODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CIAUDIOCODEC_H
#include "gucefAUDIO_CIAudioCodec.h"
#define GUCEF_AUDIO_CIAUDIOCODEC_H
#endif /* GUCEF_AUDIO_CIAUDIOCODEC_H ? */

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

class CAudioCodecCPluginInstance;

/*-------------------------------------------------------------------------*/

/**
 *  One codec type of a C codec plugin, as a CIAudioCodec. The plugin manager makes one per type the plugin
 *  offers and registers it in the codec registry. Sources it opens keep the plugin codec instance alive, and
 *  end (instead of failing badly) when the plugin is unloaded while they are open.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioCodecCPluginCodec : public CIAudioCodec
{
    public:

    typedef CAudioCodecCPlugin::TAudioCodecCPluginPtr          TAudioCodecCPluginPtr;
    typedef CORE::CTSharedPtr< CAudioCodecCPluginInstance, MT::CMutex > TCodecInstancePtr;

    /** Creates the codec instance in the plugin. False when the plugin refuses the type */
    static bool Create( const TAudioCodecCPluginPtr& plugin, const CString& codecType, CORE::CTBasicSharedPtr< CIAudioCodec, MT::CMutex >& codec );

    CAudioCodecCPluginCodec( const CAudioCodecCPluginCodec& src );

    virtual ~CAudioCodecCPluginCodec() GUCEF_VIRTUAL_OVERRIDE;

    /** The format and length, from opening the stream, plus whatever the plugin reads as tags */
    virtual bool DecodeMetaData( CORE::CIOAccess& encodedInput, CORE::CValueList& metaData ) GUCEF_VIRTUAL_OVERRIDE;

    /** Asks the plugin when it has a probe (AUDIOCODECPLUG_Probe). False when it has none, the input position is restored */
    virtual bool Probe( CORE::CIOAccess& encodedInput, UInt32& confidence ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CanEncode( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Encode( const CAudioBuffer& audio, CORE::CIOAccess& encodedOutput, const CORE::CValueList& options = CORE::CValueList() ) GUCEF_VIRTUAL_OVERRIDE;

    using CIAudioCodec::Encode;

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    const TAudioCodecCPluginPtr& GetPlugin( void ) const;

    protected:

    virtual bool OpenSourceImpl( CORE::CIOAccess* input, const TIOAccessPtr& owner, const CORE::CValueList& options, TAudioSourcePtr& source ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAudioCodecCPluginCodec( const TAudioCodecCPluginPtr& plugin, const CString& codecType, const TCodecInstancePtr& instance );

    CAudioCodecCPluginCodec& operator=( const CAudioCodecCPluginCodec& src );   /**< not implemented */

    private:

    TAudioCodecCPluginPtr m_plugin;
    CString m_codecType;
    TCodecInstancePtr m_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOCODECCPLUGINCODEC_H ? */
