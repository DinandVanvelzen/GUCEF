/*
 *  gucefDRNUTILSGUI_TestApp: GUCEF test application for the gucefDRNUTILSGUI module
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

#ifndef GUCEF_DRNUTILSGUI_TESTAPP_TESTCOMMON_H
#define GUCEF_DRNUTILSGUI_TESTAPP_TESTCOMMON_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                    //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_H
#include "gucefGUI.h"
#define GUCEF_GUI_H
#endif /* GUCEF_GUI_H ? */

#ifndef GUCEF_DRNUTILS_H
#include "gucefDRNUTILS.h"
#define GUCEF_DRNUTILS_H
#endif /* GUCEF_DRNUTILS_H ? */

#ifndef GUCEF_DRNUTILSGUI_H
#include "gucefDRNUTILSGUI.h"
#define GUCEF_DRNUTILSGUI_H
#endif /* GUCEF_DRNUTILSGUI_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Loads a plugin from the directory of the test app, the debug or release build of it */
bool LoadTestPlugin( const char* moduleName, const char* pluginGroup );

/** A context of the null driver at 1280 x 960 in the dark theme with a fixed width test font, NULL when the driver cannot be loaded */
GUCEF::GUI::TGuiContextPtr CreateFormContext( void );

/** The context of a context pointer as the forms want it */
GUCEF::GUI::CGUIContext* ContextOf( GUCEF::GUI::TGuiContextPtr& contextPtr );

/** A click of the user in the middle of a widget */
void ClickWidget( GUCEF::GUI::CGUIContext* context, GUCEF::GUI::CWidget* widget );

/** Pulses the global pulse generator from the test thread for the given time, for the events of the notifiers that mail */
void PumpFor( GUCEF::UInt32 durationInMs );

/** Whether the widget of the form is there and enabled */
bool IsWidgetEnabled( GUCEF::GUI::CForm& form, const GUCEF::CORE::CString& name );

/** The text property of a widget of the form, empty when there is no such widget */
GUCEF::CORE::CString TextOfWidget( GUCEF::GUI::CForm& form, const GUCEF::CORE::CString& name );

/** The last frame of a context as RGBA pixels */
GUCEF::IMAGE::TPixelMapPtr CaptureFrame( GUCEF::GUI::CGUIContext* context );

/**
 *  Writes RGBA pixels as a PNG file named after 'name' in the directory of the environment variable GUCEF_TEST_SHOT_DIR: without
 *  it nothing is written and false is returned (the shots are for people to look at, not part of what a test checks)
 */
bool WriteShot( const GUCEF::IMAGE::TPixelMapPtr& pixels, const GUCEF::CORE::CString& name );

/** The instant of a UTC date and time in milliseconds since the Unix epoch */
GUCEF::Int64 UtcMs( GUCEF::Int32 year, GUCEF::Int32 month, GUCEF::Int32 day, GUCEF::Int32 hour, GUCEF::Int32 minute, GUCEF::Int32 second );

/** Counts one event of one notifier */
class CEventCount : public GUCEF::CORE::CObserver
{
    public:

    typedef GUCEF::CORE::CTEventHandlerFunctor< CEventCount > TEventCallback;

    GUCEF::UInt32 count;

    CEventCount( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& id )
        : GUCEF::CORE::CObserver(), count( 0 )
    {
        TEventCallback callback( this, &CEventCount::OnEvent );
        SubscribeTo( notifier, id, callback );
    }

    virtual ~CEventCount() { UnsubscribeAllFromObserver(); }

    void OnEvent( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& id, GUCEF::CORE::CICloneable* eventData )
    {
        ++count;
    }
};

/*-------------------------------------------------------------------------*/

void PerformControlConfirmFormTests( void );
void PerformTimeDisplayTests( void );
void PerformSchedulerFormTests( void );
void PerformJobRunHistoryFormTests( void );
void PerformTaskManagerFormTests( void );

/** A real scheduler, the DRNUTILS host, a DRN link, the client facade and the forms on the Null GUI driver */
void PerformEndToEndTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILSGUI_TESTAPP_TESTCOMMON_H ? */
