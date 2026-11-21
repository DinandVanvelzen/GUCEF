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

#ifndef GUCEF_LABLINK_WINDOWSHOST_H
#define GUCEF_LABLINK_WINDOWSHOST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Windows only (src/mswin/gucefLabLink_WindowsHost.cpp); callers use them behind GUCEF_PLATFORM_MSWIN.
 *
 *  The service runs as LocalSystem and the command line (--set-secret, --approve, ...) as an administrator, so the
 *  files of the state directory belong to both: owner the Administrators group, access for SYSTEM and the
 *  Administrators only, protected from what the parent directory passes on. CORE::ReadOwnerOnlyFile() accepts such
 *  files for both. CORE::CreateOwnerOnlyFile() would give the file to the one user that made it, which the other
 *  could then not read.
 */

/** Makes the directory when it does not exist and gives it the access above, passed on to what is made in it */
bool ProtectStateDirectory( const GUCEF::CORE::CString& directory, GUCEF::CORE::CString& problem );

/**
 *  Replaces or makes a file of the state directory with the access above: the data goes to a new temporary file next
 *  to it, made with that access (CREATE_NEW, so nothing that exists is used), flushed and moved over the file.
 *  A state directory that does not exist yet is made with ProtectStateDirectory().
 */
bool WriteProtectedStateFile( const GUCEF::CORE::CString& stateDir    ,
                              const GUCEF::CORE::CString& path        ,
                              const void* data                        ,
                              GUCEF::UInt32 size                      ,
                              GUCEF::CORE::CString& problem           );

/**
 *  The program is a Windows application (WinMain), which has no console: for --console and the commands its standard
 *  output, error and input go to the console of the command prompt it was started from, or to a console of its own.
 *  Streams that were redirected (to a file or a pipe) stay as they are.
 */
void AttachToConsole( void );

/** The PROCESSOR_ARCHITECTURE_ value of the machine (GetNativeSystemInfo()) */
GUCEF::UInt32 NativeProcessorArchitecture( void );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_WINDOWSHOST_H ? */
