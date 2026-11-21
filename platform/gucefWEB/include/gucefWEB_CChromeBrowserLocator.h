/*
 *  gucefWEB: GUCEF module providing Web application functionality
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

#ifndef GUCEF_WEB_CCHROMEBROWSERLOCATOR_H
#define GUCEF_WEB_CCHROMEBROWSERLOCATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Finds a browser that speaks the Chrome DevTools protocol for a web session sign in: Google Chrome, Chromium,
 *  Microsoft Edge and Brave, in that order, at the places they install to on Linux, Windows and macOS. A configured
 *  path always comes first (CWebSignInBrowserSettings::executablePath).
 */
class GUCEF_WEB_PUBLIC_CPP CChromeBrowserLocator
{
    public:

    /** The executables to try on this platform, best first */
    static void GetCandidatePaths( CORE::CString::StringVector& paths );

    /**
     *  The configured path when it is a file, else the first candidate that exists
     *  @return false when none was found
     */
    static bool FindBrowser( const CORE::CString& configuredPath, CORE::CString& executablePath );

    private:

    CChromeBrowserLocator( void );      /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CCHROMEBROWSERLOCATOR_H ? */
