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

#ifndef GUCEF_LABLINK_CONFIGFILE_H
#define GUCEF_LABLINK_CONFIGFILE_H

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

/** The largest configuration file that is read */
static const GUCEF::UInt32 MaxConfigFileSize = 65536;

/*-------------------------------------------------------------------------*/

/**
 *  The options of a configuration file (lablink.conf) as command line arguments, for the same option parser as the
 *  command line. One option per line, with the name of the command line option, the leading "--" optional:
 *      name = value        or   name value        a value; quotes around it keep spaces at its ends: name = "a b"
 *      name                                        an option without a value (no-mdns, permanent, ...)
 *      # text   or   ; text                       a comment line; empty lines are skipped
 *  A line gives "--name", followed by the value as an argument of its own when there is one, so a value never
 *  combines with the next line. Refused, with the line number in 'problem': a name with other characters than
 *  letters, digits and '-', "name =" without a value, an unterminated quote, a line longer than 4096 bytes, and the
 *  options "config" and "console", which belong on the command line. Whether a name is an option, whether it takes a
 *  value and whether it is a command (which does not belong in the file) is for the option parser to judge.
 *  A UTF-8 byte order mark and CR line ends are accepted.
 *  @param sourceName   the file, for 'problem'
 */
bool ParseConfigText( const GUCEF::CORE::CString& text                ,
                      const GUCEF::CORE::CString& sourceName          ,
                      GUCEF::CORE::CString::StringVector& arguments   ,
                      GUCEF::CORE::CString& problem                   );

/*-------------------------------------------------------------------------*/

/**
 *  Reads and parses a configuration file. The file decides what the service runs as an administrator, so only its
 *  administrators may be able to change it:
 *  POSIX: a regular file (no symbolic link), owned by root or by the effective user, not writable by the group or
 *  others, at most MaxConfigFileSize bytes. Windows: CORE::ReadOwnerOnlyFile(): owned by the user or the Administrators,
 *  access for the user, SYSTEM and the Administrators only.
 *  @param missing  set to whether the file does not exist (then the result is false, 'problem' says so)
 */
bool ReadConfigFile( const GUCEF::CORE::CString& path                 ,
                     GUCEF::CORE::CString::StringVector& arguments    ,
                     bool& missing                                    ,
                     GUCEF::CORE::CString& problem                    );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_CONFIGFILE_H ? */
