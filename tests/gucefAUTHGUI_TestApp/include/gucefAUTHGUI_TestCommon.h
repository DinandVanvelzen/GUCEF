/*
 *  gucefAUTHGUI_TestApp: Application for testing gucefAUTHGUI
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_AUTHGUI_TESTCOMMON_H
#define GUCEF_AUTHGUI_TESTCOMMON_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
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

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

#ifndef GUCEF_AUTHGUI_H
#include "gucefAUTHGUI.h"
#define GUCEF_AUTHGUI_H
#endif /* GUCEF_AUTHGUI_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Loads a plugin from the directory of the test app, the debug or release build of it */
bool LoadTestPlugin( const char* moduleName, const char* pluginGroup );

/** A context of the null driver at 640 x 480, NULL when the driver cannot be loaded */
GUCEF::GUI::TGuiContextPtr CreateNullContext( void );

/** A pixel map of the given size filled with one colour */
GUCEF::IMAGE::TPixelMapPtr MakeSolidImage( GUCEF::UInt32 width, GUCEF::UInt32 height, GUCEF::UInt8 r, GUCEF::UInt8 g, GUCEF::UInt8 b, GUCEF::UInt8 a );

/** A bitmap font with fixed width glyphs for ASCII 32..126: each glyph is 'width' x 'height' */
GUCEF::GUI::TBitmapFontPtr MakeTestFont( GUCEF::Int32 width, GUCEF::Int32 height );

/** A context of the null driver at 1280 x 960 in the dark theme, with the fixed width test font */
GUCEF::GUI::TGuiContextPtr CreateFormContext( void );

/**
 *  A context of the null driver of the given size in the dark theme: with the font of the game user interfaces (Noto Sans at 18 pixels,
 *  through the FreeType plugin) when 'realFont' is set and the plugin is there, else with the fixed width test font
 */
GUCEF::GUI::TGuiContextPtr CreateSizedFormContext( GUCEF::UInt32 width, GUCEF::UInt32 height, bool realFont );

/** Whether the context draws with the real font */
bool HasRealFont( GUCEF::GUI::CGUIContext* context );

/** The last frame of a context as RGBA pixels at its size */
GUCEF::IMAGE::TPixelMapPtr CaptureFrame( GUCEF::GUI::CGUIContext* context );

/**
 *  Writes RGBA pixels as a PNG file. The directory is in the environment variable GUCEF_TEST_SHOT_DIR: without it nothing is
 *  written and false is returned (the shots are for people to look at, not part of what a test checks).
 */
bool WriteShot( const GUCEF::IMAGE::TPixelMapPtr& pixels, const GUCEF::CORE::CString& name );

/** The context of a context pointer as the forms want it */
GUCEF::GUI::CGUIContext* ContextOf( GUCEF::GUI::TGuiContextPtr& contextPtr );

/** A click of the user at a position of the context */
void ClickAt( GUCEF::GUI::CGUIContext* context, GUCEF::Int32 x, GUCEF::Int32 y, GUCEF::GUI::TMouseButton button = GUCEF::GUI::MOUSEBUTTON_LEFT );

/** A click of the user in the middle of a widget */
void ClickWidget( GUCEF::GUI::CGUIContext* context, GUCEF::GUI::CWidget* widget );

void PressKey( GUCEF::GUI::CGUIContext* context, GUCEF::INPUT::KeyCode key );

/** The user types text in a field, and Enter if asked */
void TypeInto( GUCEF::GUI::CGUIContext* context, GUCEF::GUI::CWidget* field, const GUCEF::CORE::CString& text, bool enter );

bool IsWidgetEnabled( GUCEF::GUI::CForm& form, const GUCEF::CORE::CString& name );

/** Pulses the global pulse generator from the test thread for the given time, for timers of the lobby doubles */
void PumpFor( GUCEF::UInt32 durationInMs );

/*-------------------------------------------------------------------------*/

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


void PerformAuthGuiGlobalTests( void );
void PerformLoginFormTests( void );
void PerformAccountSettingsFormTests( void );
void PerformFormLayoutTests( void );
void PerformAccountOfferTests( void );

/** The offers of the page of the accounts when the HTTP client has no https: runs before the plugin for https is loaded */
void PerformAccountOfferNoHttpsTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHGUI_TESTCOMMON_H ? */
