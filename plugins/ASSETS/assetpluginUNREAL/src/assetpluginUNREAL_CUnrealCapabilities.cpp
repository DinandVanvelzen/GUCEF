/*
 *  assetpluginUNREAL: What the reader can and cannot do on this machine (the Oodle tool)
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

#include <stdlib.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCAPABILITIES_H
#include "assetpluginUNREAL_CUnrealCapabilities.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCAPABILITIES_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCAPABILITIES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* const CUnrealCapabilities::OodleToolPathVariable = "GUCEF_OOZ_EXTRACT_PATH";
const char* const CUnrealCapabilities::NeedsOodleProvider = "needs the oodle provider: tool not found";

static MT::CMutex g_notedLock;
static bool g_oodleNoted = false;

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealCapabilities::GetExpectedOodleToolPath( void )
{GUCEF_TRACE;

    const char* override = getenv( OodleToolPathVariable );
    if ( GUCEF_NULL != override && '\0' != override[ 0 ] )
        return CORE::CString( override );

    CORE::CString path = CORE::ModuleDir();
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    CORE::AppendToPath( path, "ooz_extract.exe" );
    #else
    CORE::AppendToPath( path, "ooz_extract" );
    #endif
    return path;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealCapabilities::FindOodleTool( CORE::CString& path )
{GUCEF_TRACE;

    path = GetExpectedOodleToolPath();
    return CORE::FileExists( path );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealCapabilities::GetOodleStatus( void )
{GUCEF_TRACE;

    CORE::CString path;
    if ( FindOodleTool( path ) )
        return "oodle: available (" + path + ")";
    return "oodle: unavailable (tool not found at " + path + "): Unreal 5 textures and static meshes unreadable";
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CUnrealCapabilities::GetCapabilityList( void )
{GUCEF_TRACE;

    CORE::CString::StringVector list;
    list.push_back( GetOodleStatus() );
    return list;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealCapabilities::GetNeedsOodleText( void )
{GUCEF_TRACE;

    return CORE::CString( NeedsOodleProvider ) + " at " + GetExpectedOodleToolPath();
}

/*-------------------------------------------------------------------------*/

void
CUnrealCapabilities::NoteOodleStatusOnce( void )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( g_notedLock );
        if ( g_oodleNoted )
            return;
        g_oodleNoted = true;
    }
    CORE::CString path;
    if ( FindOodleTool( path ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "assetpluginUNREAL: " << GetOodleStatus() );
    }
    else
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "assetpluginUNREAL: " << GetOodleStatus() );
    }
}

/*-------------------------------------------------------------------------*/

void
CUnrealCapabilities::ResetNotedState( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_notedLock );
    g_oodleNoted = false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
