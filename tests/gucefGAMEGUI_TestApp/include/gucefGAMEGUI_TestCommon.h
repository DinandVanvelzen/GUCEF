/*
 *  gucefGAMEGUI_TestApp: Application for testing gucefGAMEGUI
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

#ifndef GUCEF_GAMEGUI_TESTCOMMON_H
#define GUCEF_GAMEGUI_TESTCOMMON_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_GUI_H
#include "gucefGUI.h"
#define GUCEF_GUI_H
#endif /* GUCEF_GUI_H ? */

#ifndef GUCEF_MULTIPLAY_H
#include "gucefMULTIPLAY.h"
#define GUCEF_MULTIPLAY_H
#endif /* GUCEF_MULTIPLAY_H ? */

#ifndef GUCEF_GAMEGUI_H
#include "gucefGAMEGUI.h"
#define GUCEF_GAMEGUI_H
#endif /* GUCEF_GAMEGUI_H ? */

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

/** The context of a context pointer as the forms want it */
GUCEF::GUI::CGUIContext* ContextOf( GUCEF::GUI::TGuiContextPtr& contextPtr );

/** A click of the user at a position of the context */
void ClickAt( GUCEF::GUI::CGUIContext* context, GUCEF::Int32 x, GUCEF::Int32 y, GUCEF::GUI::TMouseButton button = GUCEF::GUI::MOUSEBUTTON_LEFT );

/** A click of the user in the middle of a widget */
void ClickWidget( GUCEF::GUI::CGUIContext* context, GUCEF::GUI::CWidget* widget );

/** The middle of a data row of a grid (row 0 is below the header) */
void RowPoint( GUCEF::GUI::CGUIContext* context, GUCEF::GUI::CGridView* grid, GUCEF::Int32 row, GUCEF::Int32& x, GUCEF::Int32& y );

void ClickRow( GUCEF::GUI::CGUIContext* context, GUCEF::GUI::CGridView* grid, GUCEF::Int32 row, GUCEF::GUI::TMouseButton button = GUCEF::GUI::MOUSEBUTTON_LEFT );

void DoubleClickRow( GUCEF::GUI::CGUIContext* context, GUCEF::GUI::CGridView* grid, GUCEF::Int32 row );

/** A click on a column title of a grid */
void ClickColumnTitle( GUCEF::GUI::CGUIContext* context, GUCEF::GUI::CGridView* grid, GUCEF::Int32 column );

void PressKey( GUCEF::GUI::CGUIContext* context, GUCEF::INPUT::KeyCode key );

/** The user types text in a field, and Enter if asked */
void TypeInto( GUCEF::GUI::CGUIContext* context, GUCEF::GUI::CWidget* field, const GUCEF::CORE::CString& text, bool enter );

/** The user opens the list of a combo box and clicks an entry */
bool PickItem( GUCEF::GUI::CGUIContext* context, GUCEF::GUI::CWidget* combobox, GUCEF::Int32 index );

bool Contains( const GUCEF::CORE::CString::StringVector& lines, const GUCEF::CORE::CString& text );

/** Whether one of the lines has the text in it */
bool AnyLineHas( const GUCEF::CORE::CString::StringVector& lines, const GUCEF::CORE::CString& text );

bool IsWidgetEnabled( GUCEF::GUI::CForm& form, const GUCEF::CORE::CString& name );

/** Pulses the global pulse generator from the test thread for the given time, for timers of the lobby doubles */
void PumpFor( GUCEF::UInt32 durationInMs );

/*-------------------------------------------------------------------------*/

/** Counts one event of one notifier */
class CEventCount : public GUCEF::CORE::CObserver
{
    public:

    GUCEF::CORE::CEvent eventId;
    GUCEF::UInt32 count;

    CEventCount( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& id )
        : GUCEF::CORE::CObserver(), eventId( id ), count( 0 )
    {
        SubscribeTo( notifier, id );
    }

    virtual ~CEventCount() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& id, GUCEF::CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( id == eventId )
            ++count;
    }
};

/*-------------------------------------------------------------------------*/

void PerformGameGuiGlobalTests( void );
void PerformServerBrowserFormTests( void );
void PerformCreateLobbyFormTests( void );
void PerformLobbySettingsPanelFormTests( void );
void PerformLobbyFormTests( void );
void PerformChatFormTests( void );
void PerformVotePromptFormTests( void );
void PerformConnectingFormTests( void );
void PerformDirectConnectFormTests( void );
void PerformPlayerProfileFormTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_TESTCOMMON_H ? */
