/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_GAMEGUI_CTURNWAITOVERLAYFORM_H
#define GUCEF_GAMEGUI_CTURNWAITOVERLAYFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_GAMEGUI_MACROS_H
#include "gucefGAMEGUI_macros.h"
#define GUCEF_GAMEGUI_MACROS_H
#endif /* GUCEF_GAMEGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GAMEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A box in the middle of the context that tells the player what is waited for ("Waiting on player Bob to complete
 *  their turn") with the time that is left beneath it, as "m:ss" (FormatCountdown()). It is not modal: Show() puts it
 *  on top of the roots of the context, which is where it stops the input of what is below it.
 *
 *  The time left counts down with the frames of the context (CGUIContext::FrameStartedEvent), and is set again with
 *  SetRemainingMs() when the application learns the time better, for example from a clock it shares with the other
 *  players. Once the time left is at or below the warning threshold (5000 ms unless SetWarningThresholdMs() says
 *  otherwise) the clock text and the box flash red, twice a second: red for the first 250 ms from the threshold, then
 *  the normal colours for 250 ms, and so on, with the phase taken from the time left. The properties of the widgets are
 *  only set when the phase changes. At 0 TimeExpiredEvent is notified once; the overlay stays open, the application
 *  decides what happens. A new time left above 0 arms the event again.
 *
 *  Without a clock (showClock false) the overlay is a message only: the clock is hidden, nothing counts down, the
 *  event is never sent.
 *
 *  The layout is built in code and made for BuildDarkTheme().
 */
class GUCEF_GAMEGUI_EXPORT_CPP CTurnWaitOverlayForm : public GUI::CFormEx
{
    public:

    static const CORE::CEvent TimeExpiredEvent;     /**< no data: the time left reached 0, notified once per countdown */

    static void RegisterEvents( void );

    static const UInt32 DefaultWarningThresholdInMs;        /**< 5000 */
    static const UInt32 FlashHalfPeriodInMs;                /**< 250: a toggle every 250 ms is 2 Hz */

    CTurnWaitOverlayForm( void );

    CTurnWaitOverlayForm( GUI::CIGUIContext* context );

    virtual ~CTurnWaitOverlayForm();

    /** Makes the box, hidden */
    bool Build( void );

    /** Shows the box with the message, and the clock when asked; 'remainingInMs' is what the clock counts down from */
    bool Open( const CORE::CString& message, Int64 remainingInMs, bool showClock );

    /** Sets the time left, below 0 is 0; for an open overlay with a clock */
    void SetRemainingMs( Int64 remainingInMs );

    Int64 GetRemainingMs( void ) const;

    void SetMessage( const CORE::CString& message );

    const CORE::CString& GetMessage( void ) const;

    void Close( void );

    bool IsOpen( void ) const;

    /** Whether the clock is shown and counts down */
    bool IsClockShown( void ) const;

    /** The time left at or below which the clock and the box flash red, 0 for never */
    void SetWarningThresholdMs( UInt32 thresholdInMs );

    UInt32 GetWarningThresholdMs( void ) const;

    /** Whether the time left is within the warning threshold */
    bool IsWarning( void ) const;

    /** Whether the box and the clock are in their red colours now: a warning, in the red half of the flash period */
    bool IsRed( void ) const;

    /** Lets time pass, done on every frame of the context with the time of the frame */
    void Update( UInt32 deltaInMs );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CTurnWaitOverlayForm > TEventCallback;

    void BuildLayout( CORE::CDataNode& layout );
    void ShowTime( void );
    void ShowPhase( void );
    void SetLabelText( const char* widgetName, const CORE::CString& text );
    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CTurnWaitOverlayForm( const CTurnWaitOverlayForm& src );
    CTurnWaitOverlayForm& operator=( const CTurnWaitOverlayForm& src );

    private:

    CORE::CString m_message;
    Int64 m_remainingInMs;
    UInt32 m_warningThresholdInMs;
    Int64 m_shownSeconds;                   /**< the seconds the clock text is for, -1 when it shows none */
    bool m_showClock;
    bool m_open;
    bool m_expired;
    bool m_red;                             /**< the red colours are the ones set on the widgets */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_CTURNWAITOVERLAYFORM_H ? */
