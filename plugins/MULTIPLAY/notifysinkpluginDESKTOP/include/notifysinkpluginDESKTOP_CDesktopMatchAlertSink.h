/*
 *  notifysinkpluginDESKTOP: gucefMULTIPLAY match alert sink plugin for desktop alerts
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

#ifndef GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTSINK_H
#define GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTSINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CIMATCHALERTSINK_H
#include "gucefMULTIPLAY_CIMatchAlertSink.h"
#define GUCEF_MULTIPLAY_CIMATCHALERTSINK_H
#endif /* GUCEF_MULTIPLAY_CIMATCHALERTSINK_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_DESKTOP_INOTIFYLAUNCHER_H
#include "notifysinkpluginDESKTOP_INotifyLauncher.h"
#define GUCEF_MULTIPLAYPLUGIN_DESKTOP_INOTIFYLAUNCHER_H
#endif /* GUCEF_MULTIPLAYPLUGIN_DESKTOP_INOTIFYLAUNCHER_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTTEXT_H
#include "notifysinkpluginDESKTOP_CDesktopMatchAlertText.h"
#define GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTTEXT_H
#endif /* GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTTEXT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace DESKTOP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Shows a notification in the desktop of Linux with the "notify-send" program of libnotify, started through an
 *  INotifyLauncher. The sink interrupts the player, so the router (CMatchAlertRouter) applies quiet hours, the
 *  mode, coalescing and the minimum interval before it gets anything.
 *
 *  The arguments are: notify-send, --app-name=NAME, --urgency=normal, optionally --category=NAME and
 *  --expire-time=MS, then "--", the summary and the body (when there is one). Each is an entry of its own, so
 *  that text which looks like an option stays text. Summary and body are cleaned and cut by CDesktopMatchAlertText and
 *  made from the title of the match and the name of the previous mover only, the contents of a match never get here.
 *
 *  Clear() does nothing: notify-send cannot take a notification back without reading what it prints, and this sink
 *  reads nothing from the programs it starts. A notification goes away when the player or the desktop closes it.
 *
 *  At most GetMaxChildren() programs run at the same time, a notification for which there is no room is dropped
 *  and counted. Missing notify-send is logged once and nothing is shown. Deliver() never throws.
 *  Not thread safe: use it from the thread of the router.
 */
class GUCEF_MULTIPLAYPLUGIN_DESKTOP_EXPORT_CPP CDesktopMatchAlertSink : public MULTIPLAY::CIMatchAlertSink
{
    public:

    static const CORE::CString SinkName;                    /**< "desktop", also the name in the factory of the match alert sinks */
    static const UInt32 DefaultMaxChildren = 4;
    static const UInt32 MaxExpireTimeMs = 3600000;

    /** A sink with its own launcher for notify-send */
    CDesktopMatchAlertSink( void );

    /** A sink with the launcher given, which is not owned and has to outlive the sink; NULL shows nothing */
    explicit CDesktopMatchAlertSink( INotifyLauncher* launcher );

    virtual ~CDesktopMatchAlertSink();

    virtual void Deliver( const MULTIPLAY::CMatchAlert& alert ) GUCEF_VIRTUAL_OVERRIDE;

    /** Does nothing, see the class description */
    virtual void Clear( const CORE::CString& matchId ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsInterrupting( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetSinkName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Makes the arguments for the launcher: see the class description.
     *  @return false when there is nothing to show for the notification
     */
    bool BuildArguments( const MULTIPLAY::CMatchAlert& alert, INotifyLauncher::TArgumentVector& argv ) const;

    /** The launcher to use from now on, which is not owned; NULL shows nothing */
    void SetLauncher( INotifyLauncher* launcher );

    /** The name of the application the notifications come from, cleaned and cut to 64 bytes; "Game" when nothing is left */
    void SetAppName( const CORE::CString& appName );

    const CORE::CString& GetAppName( void ) const;

    /** The category of the notifications (see IsValidCategory of CDesktopMatchAlertText); anything else is refused and leaves it as it was; empty for none */
    bool SetCategory( const CORE::CString& category );

    const CORE::CString& GetCategory( void ) const;

    /** How long the notification is shown, in milliseconds up to MaxExpireTimeMs; 0 leaves it to the desktop */
    void SetExpireTimeMs( UInt32 expireTimeMs );

    UInt32 GetExpireTimeMs( void ) const;

    /** The number of programs that may run at the same time, 1 to 64 */
    void SetMaxChildren( UInt32 maxChildren );

    UInt32 GetMaxChildren( void ) const;

    void SetTexts( const CDesktopMatchAlertTexts& texts );

    const CDesktopMatchAlertTexts& GetTexts( void ) const;

    /** Notifications handed to the launcher */
    UInt32 GetLaunchedCount( void ) const;

    /** Notifications not shown because too many programs were running */
    UInt32 GetDroppedCount( void ) const;

    private:

    CDesktopMatchAlertSink( const CDesktopMatchAlertSink& src );
    CDesktopMatchAlertSink& operator=( const CDesktopMatchAlertSink& src );

    private:

    INotifyLauncher* m_launcher;
    INotifyLauncher* m_ownedLauncher;
    CORE::CString m_appName;
    CORE::CString m_category;
    UInt32 m_expireTimeMs;
    UInt32 m_maxChildren;
    CDesktopMatchAlertTexts m_texts;
    UInt32 m_launchedCount;
    UInt32 m_droppedCount;
    bool m_reportedUnavailable;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< MULTIPLAY::CIMatchAlertSink, CDesktopMatchAlertSink, MT::CMutex > TDesktopMatchAlertSinkFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DESKTOP */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTSINK_H ? */
