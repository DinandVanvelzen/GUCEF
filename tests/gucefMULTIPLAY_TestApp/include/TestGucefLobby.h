/*
 *  gucefMULTIPLAY_TestApp: Application for testing gucefMULTIPLAY
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

#ifndef GUCEF_MULTIPLAY_TESTAPP_TESTGUCEFLOBBY_H
#define GUCEF_MULTIPLAY_TESTAPP_TESTGUCEFLOBBY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the built-in `gucef` lobby service against an embedded CGucefLobbyServer on loopback (tcp and the
 *  in-process mem channel): the directory, the whole flow of a lobby, the launch descriptions and the DRN game
 *  sessions made from them, the `lobby` chat and the standalone server app
 */
void
PerformGucefLobbyTests( void );

/**
 *  Tests the accounts at external identity providers in the lobby protocol: the link proofs of the handshake, what a
 *  server does with proofs that do not hold, requireLinkedProvider, bans and reports by account
 */
void
PerformGucefLobbyAuthTests( void );

/** The argument with which the test app only starts a CGucefLobbyServer, followed by the endpoint to listen on */
#define GUCEF_MULTIPLAY_TESTAPP_LOBBYSERVER_ONLY_ARG    "--lobby-server-only"

/** Remembers the executable of the test app, the tests start it again as a child process */
void
SetTestAppExecutable( const char* argv0 );

/**
 *  Starts a CGucefLobbyServer listening at the given endpoint and nothing else, as a fresh process that uses only
 *  the lobby server would
 *  @return the exit code: 0 when the server started
 */
int
RunLobbyServerOnly( const char* listenEndpoint );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_TESTAPP_TESTGUCEFLOBBY_H ? */
