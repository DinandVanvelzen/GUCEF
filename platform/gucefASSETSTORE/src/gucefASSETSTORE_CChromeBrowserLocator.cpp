/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */
#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */
#ifndef GUCEF_ASSETSTORE_CCHROMEBROWSERLOCATOR_H
#include "gucefASSETSTORE_CChromeBrowserLocator.h"
#define GUCEF_ASSETSTORE_CCHROMEBROWSERLOCATOR_H
#endif /* GUCEF_ASSETSTORE_CCHROMEBROWSERLOCATOR_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

void
AddUnder( const CORE::CString& root, const char* relativePath, CORE::CString::StringVector& paths )
{
    if ( root.IsNULLOrEmpty() )
        return;
    CORE::CString path = root;
    CORE::AppendToPath( path, relativePath );
    paths.push_back( path );
}

}

/*-------------------------------------------------------------------------*/

void
CChromeBrowserLocator::GetCandidatePaths( CORE::CString::StringVector& paths )
{GUCEF_TRACE;

    paths.clear();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    const CORE::CString programFiles = CORE::GetEnv( "ProgramFiles" );
    const CORE::CString programFilesX86 = CORE::GetEnv( "ProgramFiles(x86)" );
    const CORE::CString localAppData = CORE::GetEnv( "LOCALAPPDATA" );
    const char* chrome = "Google\\Chrome\\Application\\chrome.exe";
    const char* chromium = "Chromium\\Application\\chrome.exe";
    const char* edge = "Microsoft\\Edge\\Application\\msedge.exe";
    const char* brave = "BraveSoftware\\Brave-Browser\\Application\\brave.exe";
    const char* names[] = { chrome, chromium, edge, brave };
    for ( UInt32 i=0; i<4; ++i )
    {
        AddUnder( programFiles, names[ i ], paths );
        AddUnder( programFilesX86, names[ i ], paths );
        AddUnder( localAppData, names[ i ], paths );
    }

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS )

    paths.push_back( "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome" );
    paths.push_back( "/Applications/Chromium.app/Contents/MacOS/Chromium" );
    paths.push_back( "/Applications/Microsoft Edge.app/Contents/MacOS/Microsoft Edge" );
    paths.push_back( "/Applications/Brave Browser.app/Contents/MacOS/Brave Browser" );

    #else

    // the binaries themselves before the wrapper scripts, so the process that is started is the browser
    paths.push_back( "/opt/google/chrome/chrome" );
    paths.push_back( "/usr/bin/google-chrome-stable" );
    paths.push_back( "/usr/bin/google-chrome" );
    paths.push_back( "/usr/lib/chromium/chromium" );
    paths.push_back( "/usr/lib/chromium-browser/chromium-browser" );
    paths.push_back( "/usr/bin/chromium" );
    paths.push_back( "/usr/bin/chromium-browser" );
    paths.push_back( "/snap/bin/chromium" );
    paths.push_back( "/opt/microsoft/msedge/msedge" );
    paths.push_back( "/usr/bin/microsoft-edge-stable" );
    paths.push_back( "/usr/bin/microsoft-edge" );
    paths.push_back( "/opt/brave.com/brave/brave" );
    paths.push_back( "/usr/bin/brave-browser" );
    paths.push_back( "/usr/bin/brave" );

    #endif
}

/*-------------------------------------------------------------------------*/

bool
CChromeBrowserLocator::FindBrowser( const CORE::CString& configuredPath, CORE::CString& executablePath )
{GUCEF_TRACE;

    if ( !configuredPath.IsNULLOrEmpty() )
    {
        const CORE::CString resolved = CORE::RelativePath( configuredPath );
        if ( CORE::FileExists( resolved ) )
        {
            executablePath = resolved;
            return true;
        }
        return false;
    }

    CORE::CString::StringVector candidates;
    GetCandidatePaths( candidates );
    for ( size_t i=0; i<candidates.size(); ++i )
    {
        if ( CORE::FileExists( candidates[ i ] ) )
        {
            executablePath = candidates[ i ];
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
