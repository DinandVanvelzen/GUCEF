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

#ifndef GUCEF_LABLINK_WEBCOMMANDS_H
#define GUCEF_LABLINK_WEBCOMMANDS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefLabLink_WebSettings.h"
#include "gucefLabLink_Store.h"

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

/**
 *  The command line of the website, apart from the rest of the main program:
 *
 *  Service options (cluster):
 *    --web                    serve the website of the hub
 *    --web-port N             default 47813
 *    --web-listen ADDR        a dotted IPv4 address; default every address. Peers outside --allow are refused
 *    --web-cert PATH, --web-key PATH   the certificate (chain) and key of the site, PEM; default tls/tls.crt and
 *                             tls/tls.key of the MCP directory when both are there
 *    --web-allow-plain-http   serve over plain HTTP when there is no certificate (bring-up only)
 *    --web-token-file PATH    the bearer token of the site, made for the owner when the file is not there
 *
 *  Commands (each does its work and ends; the service reads the users file again, so they apply while it runs):
 *    --add-user NAME --role admin|viewer   the password is asked on the terminal without echo, or read from the
 *                             first line of standard input; never from the command line
 *    --set-password NAME, --disable-user NAME, --enable-user NAME, --remove-user NAME, --list-users
 */
class CWebCommands
{
    public:

    /** Whether the option takes no value */
    static bool IsFlag( const GUCEF::CORE::CString& arg );

    /**
     *  Takes a website option or command. @param command and commandArgument set for a command
     *  @return false when the option is not one of the website
     */
    static bool ParseOption( const GUCEF::CORE::CString& arg, const GUCEF::CORE::CString& value, SWebSettings& settings,
                             GUCEF::CORE::CString& command, GUCEF::CORE::CString& commandArgument );

    static bool IsCommand( const GUCEF::CORE::CString& command );

    /**
     *  Runs a user command on the users file of the state directory.
     *  @param role the value of --role, for --add-user
     *  @return the exit code of the program
     */
    static int Run( CStore& store, const SWebSettings& settings, const GUCEF::CORE::CString& command, const GUCEF::CORE::CString& name,
                    const GUCEF::CORE::CString& role );

    /** The lines of the usage text */
    static const char* Usage( void );

    /**
     *  A new password: on a terminal asked twice without echo, else the first line of standard input.
     *  @return false when there is none, the two differ, or it is shorter than minLength; problem says which
     */
    static bool ReadNewPassword( const GUCEF::CORE::CString& user, GUCEF::UInt32 minLength, GUCEF::CORE::CString& password,
                                 GUCEF::CORE::CString& problem );

    private:

    CWebCommands( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_WEBCOMMANDS_H ? */
