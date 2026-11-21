/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#include "gucefLabLink_PlatformDefaults.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

SPlatformPaths
PlatformPathsFor( bool windows, const CORE::CString& programData )
{GUCEF_TRACE;

    SPlatformPaths paths;
    if ( !windows )
    {
        const SSettings defaults;
        paths.stateDir = defaults.stateDir;
        paths.mcpDir = defaults.mcpDir;
        return paths;
    }

    CORE::CString base = programData.IsNULLOrEmpty() ? CORE::CString( "C:\\ProgramData" ) : programData;
    while ( base.Length() > 3 && ( base.EndsWith( "\\" ) || base.EndsWith( "/" ) ) )
        base = base.CutChars( 1, false );
    if ( !base.EndsWith( "\\" ) )
        base += "\\";
    paths.stateDir = base + "gucef-lablink";
    paths.configFile = paths.stateDir + "\\lablink.conf";
    paths.mcpDir = base + "gucef-mcp";
    return paths;
}

/*-------------------------------------------------------------------------*/

SPlatformPaths
ThisPlatformPaths( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return PlatformPathsFor( true, CORE::GetEnv( "ProgramData" ) );
    #else
    return PlatformPathsFor( false, CORE::CString::Empty );
    #endif
}

/*-------------------------------------------------------------------------*/

void
ApplyPlatformDefaults( SSettings& settings )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    const SPlatformPaths paths = ThisPlatformPaths();
    settings.stateDir = paths.stateDir;
    settings.mcpDir = paths.mcpDir;
    #endif
}

/*-------------------------------------------------------------------------*/

CORE::CString
LogDirFor( const CORE::CString& stateDir )
{GUCEF_TRACE;

    return CORE::CombinePath( stateDir, "logs" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
DefaultRoleFor( bool windows, bool k3sServerUnitExists )
{GUCEF_TRACE;

    // a k3s server runs on Linux only
    return ( !windows && k3sServerUnitExists ) ? "cluster" : "machine";
}

/*-------------------------------------------------------------------------*/

CORE::CString
DefaultRole( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return DefaultRoleFor( true, false );
    #else
    return DefaultRoleFor( false, CORE::FileExists( "/etc/systemd/system/k3s.service" ) );
    #endif
}

/*-------------------------------------------------------------------------*/

CORE::CString
WindowsArchName( UInt32 processorArchitecture )
{GUCEF_TRACE;

    // the PROCESSOR_ARCHITECTURE_ values of winnt.h
    switch ( processorArchitecture )
    {
        case 0:  return "x86";          // PROCESSOR_ARCHITECTURE_INTEL
        case 5:  return "arm";          // PROCESSOR_ARCHITECTURE_ARM
        case 6:  return "ia64";         // PROCESSOR_ARCHITECTURE_IA64
        case 9:  return "x86_64";       // PROCESSOR_ARCHITECTURE_AMD64
        case 12: return "aarch64";      // PROCESSOR_ARCHITECTURE_ARM64
        default: return "unknown";
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
