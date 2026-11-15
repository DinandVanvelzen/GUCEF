/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
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

#ifndef GUCEF_AUTH_TESTAPP_TESTKEYRING_H
#define GUCEF_AUTH_TESTAPP_TESTKEYRING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      FUNCTIONS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the keyring code key mode of AUTH::CFileCredentialStore and AUTH::CKeyringSession: the "argon2id" codec against
 *  RFC 9106 and against Monocypher itself, the derivation and the file format, unlock with the right and a wrong code, a
 *  wrong code apart from tampering, the wait after wrong codes, the idle lock, a change of code, a store moved to another
 *  machine, reads while an unlock runs, that no code or key reaches a log or a file, forgetting the code and the config
 */
void
PerformKeyringTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_TESTAPP_TESTKEYRING_H ? */
