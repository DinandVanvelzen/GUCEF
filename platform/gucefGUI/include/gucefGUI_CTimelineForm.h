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
#ifndef GUCEF_GUI_CTIMELINEFORM_H
#define GUCEF_GUI_CTIMELINEFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

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
 *  A time line control: a list of clips (a name and a length each), play, pause, step back and step forward, a slider for
 *  the time, a speed choice, a loop check box and a readout of the time. The form keeps the current time of the selected
 *  clip and advances it while playing: Update() does it with the time that passed, and the form calls it on every
 *  FrameStartedEvent of its context, so a form in a context that renders frames plays on its own. A test drives it with a
 *  manual clock by calling Update().
 *
 *  Times are in milliseconds. The form knows nothing of what a clip is: the application gives names and lengths and reacts
 *  to the events.
 *
 *  Events (notified by the form, the changes of the user and those made through the functions alike):
 *      ClipSelectedEvent           TCloneableInt32 with the index of the clip, -1 when none is selected
 *      TimeChangedEvent            TTimeChangedEventData: the time and whether the user scrubbed (slider, keys of the slider)
 *      PlayStateChangedEvent       TCloneableBool: playing or not
 *
 *  Keys: Space plays and pauses while the clips, the slider or the form itself has the focus; the list and the slider keep
 *  their own keys (Up and Down choose a clip, Left and Right move the slider); the buttons and the check box are activated
 *  as every button and check box is. A clip of length 0 plays nothing: Play() refuses.
 *
 *  The form builds its layout in code, as a window (Build()) or in the place of a widget of another form (Build( placeholder )).
 */
class GUCEF_GUI_PUBLIC_CPP CTimelineForm : public CFormEx
{
    public:

    static const CORE::CEvent ClipSelectedEvent;
    static const CORE::CEvent TimeChangedEvent;
    static const CORE::CEvent PlayStateChangedEvent;

    static void RegisterEvents( void );

    /** A clip of the application: shown as its name and length */
    struct GUCEF_GUI_PUBLIC_CPP SClip
    {
        CString name;
        UInt32 lengthInMs;

        SClip( void );
        SClip( const CString& clipName, UInt32 clipLengthInMs );
    };
    typedef struct SClip TClip;
    typedef GUCEF::vector< TClip > TClipVector;

    /** The data of TimeChangedEvent */
    struct STimeChange
    {
        UInt32 timeInMs;
        bool scrubbed;          /**< the user moved the slider (else playing, a step or a function made the change) */
    };
    typedef struct STimeChange TTimeChange;
    typedef CORE::CTCloneableObj< TTimeChange > TTimeChangedEventData;

    /** The speeds the speed choice offers, in the order it lists them */
    enum ESpeedChoices
    {
        SpeedCount = 5
    };

    CTimelineForm( void );

    explicit CTimelineForm( CIGUIContext* context );

    virtual ~CTimelineForm();

    /** Makes the form as a window of the context; false when it has no context */
    bool Build( const CString& title = "Time line" );

    /** Makes the form in the place of 'placeholder', which it fills (as a settings page is built in its pane) */
    bool Build( CWidget* placeholder );

    /*
     *  Clips
     */

    void SetClips( const TClipVector& clips );

    void AddClip( const CString& name, UInt32 lengthInMs );

    void ClearClips( void );

    const TClipVector& GetClips( void ) const;

    /** Selects a clip (-1: none); the time goes to 0 and playing goes on; false for an index that is no clip */
    bool SelectClip( Int32 index );

    Int32 GetSelectedClip( void ) const;

    /** The length of the selected clip, 0 when none is selected */
    UInt32 GetClipLength( void ) const;

    /*
     *  Time and playing
     */

    /** Moves to a time, kept within the clip; false without a selected clip */
    bool SetTime( UInt32 timeInMs );

    UInt32 GetTime( void ) const;

    /** Plays the selected clip; from the start when it was at its end. False without a clip or for a clip of length 0 */
    bool Play( void );

    void Pause( void );

    bool IsPlaying( void ) const;

    /** A step of the step buttons, 33 ms by default (about a frame at 30 per second) */
    void SetStepInMs( UInt32 stepInMs );

    UInt32 GetStepInMs( void ) const;

    bool StepForward( void );

    bool StepBack( void );

    /** The speed that time passes at while playing; the choice of the speed list sets it too. Any value above 0 */
    bool SetSpeed( Float32 speed );

    Float32 GetSpeed( void ) const;

    void SetLooping( bool loop );

    bool IsLooping( void ) const;

    /** Lets time pass: advances the time of a clip that plays by 'deltaInMs' times the speed; at the end it loops or pauses */
    void Update( UInt32 deltaInMs );

    /** The readout text for a time and a length, "m:ss.mmm / m:ss.mmm" */
    static CString FormatTime( UInt32 timeInMs, UInt32 lengthInMs );

    /** The speed of an entry of the speed choice */
    static Float32 GetSpeedOfChoice( UInt32 index );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CTimelineForm > TEventCallback;

    void OnPlayActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPauseActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnStepBackActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnStepForwardActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnClipsSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTimeValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSpeedValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnLoopChecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnLoopUnchecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title );
    void UpdateWidgets( void );
    void ShowClips( void );
    void ChangeTime( Float64 timeInMs, bool scrubbed );
    void ChangePlayState( bool playing );

    CTimelineForm( const CTimelineForm& src );
    CTimelineForm& operator=( const CTimelineForm& src );

    private:

    TClipVector m_clips;
    Int32 m_selectedClip;
    Float64 m_time;
    bool m_playing;
    Float32 m_speed;
    bool m_loop;
    UInt32 m_stepInMs;
    bool m_updatingWidgets;
    bool m_frameSubscribed;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CTIMELINEFORM_H ? */
