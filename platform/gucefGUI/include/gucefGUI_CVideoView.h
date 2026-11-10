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

#ifndef GUCEF_GUI_CVIDEOVIEW_H
#define GUCEF_GUI_CVIDEOVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VIDEO_CVIDEOPLAYER_H
#include "gucefVIDEO_CVideoPlayer.h"
#define GUCEF_VIDEO_CVIDEOPLAYER_H
#endif /* GUCEF_VIDEO_CVIDEOPLAYER_H ? */

#ifndef GUCEF_VFS_CVFSHANDLE_H
#include "gucefVFS_CVFSHandle.h"
#define GUCEF_VFS_CVFSHANDLE_H
#endif /* GUCEF_VFS_CVFSHANDLE_H ? */

#ifndef GUCEF_GUI_CIMAGEFRAME_H
#include "gucefGUI_CImageFrame.h"
#define GUCEF_GUI_CIMAGEFRAME_H
#endif /* GUCEF_GUI_CIMAGEFRAME_H ? */

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
 *  Plays a movie: the frames are decoded by gucefVIDEO into an image of the frame (so a driver only has to
 *  show an image) and the sound goes to the audio engine and bus given with SetAudioOutput().
 *  Properties: Source (a VFS path), Loop, SkipOnInput (a click or a key ends the movie early), AutoPlay
 *  (start when Source is set), plus those of an image frame. Runtime: Playing.
 */
class GUCEF_GUI_PUBLIC_CPP CVideoView : public CImageFrame
{
    public:

    static const CORE::CEvent VideoEndedEvent;      /**< played to the end (not raised for a loop) */
    static const CORE::CEvent VideoSkippedEvent;    /**< ended early by the user (SkipOnInput) */

    static void RegisterEvents( void );

    public:

    CVideoView( void );

    virtual ~CVideoView();

    /** Where the sound goes; NULL for no sound. Takes effect for the next Play() */
    void SetAudioOutput( AUDIO::CAudioEngine* engine, AUDIO::CAudioBus* bus );

    /** Plays the movie of the Source property */
    bool Play( void );

    /** Plays a movie from an open file; the codec is chosen by 'nameForCodec' (a file name) */
    bool Play( const VFS::TBasicVfsResourcePtr& file, const CString& nameForCodec );

    /** Plays a source the application opened itself; such a movie does not loop */
    bool Play( const VIDEO::TVideoSourcePtr& source );

    /** Ends the movie, no event */
    void Stop( void );

    /** Ends the movie as the user would: VideoSkippedEvent */
    void Skip( void );

    bool IsPlaying( void ) const;

    const VIDEO::CVideoPlayer& GetPlayer( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static CWidget* Create( void );

    protected:

    virtual void OnUpdate( UInt32 deltaInMs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool NeedsUpdate( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    bool Start( void );
    void ShowFrame( void );

    CVideoView( const CVideoView& src );
    CVideoView& operator=( const CVideoView& src );

    private:

    VIDEO::CVideoPlayer m_player;
    VFS::TBasicVfsResourcePtr m_file;
    CString m_codecName;
    AUDIO::CAudioEngine* m_engine;
    AUDIO::CAudioBus* m_bus;
    IMAGE::TPixelMapPtr m_pixels;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CVIDEOVIEW_H ? */
