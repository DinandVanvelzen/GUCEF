/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CAUDIOPLAYER_H
#define GUCEF_GUI_CAUDIOPLAYER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CAUDIOENGINE_H
#include "gucefAUDIO_CAudioEngine.h"
#define GUCEF_AUDIO_CAUDIOENGINE_H
#endif /* GUCEF_AUDIO_CAUDIOENGINE_H ? */

#ifndef GUCEF_VFS_CVFSHANDLE_H
#include "gucefVFS_CVFSHandle.h"
#define GUCEF_VFS_CVFSHANDLE_H
#endif /* GUCEF_VFS_CVFSHANDLE_H ? */

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Plays a sound or music file through gucefAUDIO and shows a play / pause control with the time played.
 *  A click toggles play and pause. Properties: Source (a VFS path), Volume (0 to 1), Loop, AutoPlay, Font,
 *  TextColour, BackgroundColour, BarColour. Runtime: Playing, PositionInMs.
 */
class GUCEF_GUI_PUBLIC_CPP CAudioPlayer : public CWidget
{
    public:

    static const CORE::CEvent AudioEndedEvent;

    static void RegisterEvents( void );

    public:

    CAudioPlayer( void );

    virtual ~CAudioPlayer();

    /** Where the sound goes; it plays only with an engine */
    void SetAudioOutput( AUDIO::CAudioEngine* engine, AUDIO::CAudioBus* bus );

    /** Plays the file of the Source property */
    bool Play( void );

    /** Plays a source the application made itself (a tone, a decoded stream ...) */
    bool Play( const AUDIO::TAudioSourcePtr& source );

    bool Pause( void );

    bool Resume( void );

    void Stop( void );

    bool IsPlaying( void ) const;

    bool IsPaused( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "AudioPlayer"; }

    static CWidget* Create( void );

    protected:

    virtual void OnUpdate( UInt32 deltaInMs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool NeedsUpdate( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsActivatable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAudioPlayer( const CAudioPlayer& src );
    CAudioPlayer& operator=( const CAudioPlayer& src );

    private:

    AUDIO::CAudioEngine* m_engine;
    AUDIO::CAudioBus* m_bus;
    AUDIO::TAudioVoicePtr m_voice;
    VFS::TBasicVfsResourcePtr m_file;
    bool m_paused;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CAUDIOPLAYER_H ? */
