/*
 *  gucefGAMEGUI_TestApp: Application for testing gucefGAMEGUI
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

#ifndef GUCEF_GAMEGUI_TESTAPP_TESTASYNCLOBBYFORMS_H
#define GUCEF_GAMEGUI_TESTAPP_TESTASYNCLOBBYFORMS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the lobby forms of a game on a network with the capabilities of a lobby whose members answer slowly (a game by mail): the list of
 *  games without ping, lock and direct connect and with a settings button, the lobby with the presentation its lobby gives, and the form that
 *  creates a lobby with rows for the players to invite
 */
void
PerformAsyncLobbyFormTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_TESTAPP_TESTASYNCLOBBYFORMS_H ? */
