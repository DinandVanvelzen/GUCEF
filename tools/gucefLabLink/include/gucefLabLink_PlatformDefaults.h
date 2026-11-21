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

#ifndef GUCEF_LABLINK_PLATFORMDEFAULTS_H
#define GUCEF_LABLINK_PLATFORMDEFAULTS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefLabLink_Settings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Where the service keeps its files on a platform */
struct SPlatformPaths
{
    GUCEF::CORE::CString stateDir;              /**< the default of --state-dir */
    GUCEF::CORE::CString configFile;            /**< the configuration file read without --config; empty: none */
    GUCEF::CORE::CString mcpDir;                /**< the default of --mcp-dir */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The paths of a platform, a pure function so that both are tested everywhere.
 *  Windows: under 'programData' (the value of %ProgramData%, "C:\ProgramData" when it is empty): gucef-lablink,
 *  gucef-lablink\lablink.conf and gucef-mcp. Elsewhere: the defaults of SSettings and no configuration file.
 */
SPlatformPaths PlatformPathsFor( bool windows, const GUCEF::CORE::CString& programData );

/** The paths of the platform this runs on */
SPlatformPaths ThisPlatformPaths( void );

/** Sets the defaults of the platform in settings made by SSettings(): on Windows the state and MCP directories; elsewhere nothing */
void ApplyPlatformDefaults( SSettings& settings );

/** The directory of the log file of a Windows service: "logs" in the state directory */
GUCEF::CORE::CString LogDirFor( const GUCEF::CORE::CString& stateDir );

/** The role without --role: cluster on a k3s server node (Linux, its systemd unit exists), machine elsewhere and always on Windows */
GUCEF::CORE::CString DefaultRoleFor( bool windows, bool k3sServerUnitExists );

/** DefaultRoleFor() for this machine */
GUCEF::CORE::CString DefaultRole( void );

/** The architecture name of a PROCESSOR_ARCHITECTURE_ value of Windows, as uname -m gives it on Linux: x86_64, aarch64, ... */
GUCEF::CORE::CString WindowsArchName( GUCEF::UInt32 processorArchitecture );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_PLATFORMDEFAULTS_H ? */
