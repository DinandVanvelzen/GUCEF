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

#ifndef GUCEF_AUTHGUI_TESTAPP_TESTRECORDINGBROWSER_H
#define GUCEF_AUTHGUI_TESTAPP_TESTRECORDINGBROWSER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_BROWSERLAUNCHER_H
#include "gucefCORE_BrowserLauncher.h"
#define GUCEF_CORE_BROWSERLAUNCHER_H
#endif /* GUCEF_CORE_BROWSERLAUNCHER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Takes the place of the browser: remembers the urls instead of opening them */
class CRecordingBrowser : public GUCEF::CORE::CIBrowserLauncherHandler
{
    public:

    GUCEF::CORE::CString::StringVector urls;
    bool accept;
    GUCEF::CORE::CIBrowserLauncherHandler* previous;

    CRecordingBrowser( void ) : GUCEF::CORE::CIBrowserLauncherHandler(), urls(), accept( true ), previous( GUCEF_NULL )
    {
        previous = GUCEF::CORE::SetBrowserLauncherHandler( this );
    }

    virtual ~CRecordingBrowser()
    {
        GUCEF::CORE::SetBrowserLauncherHandler( previous );
    }

    virtual bool OpenUrl( const GUCEF::CORE::CString& url ) GUCEF_VIRTUAL_OVERRIDE
    {
        urls.push_back( url );
        return accept;
    }
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHGUI_TESTAPP_TESTRECORDINGBROWSER_H ? */
