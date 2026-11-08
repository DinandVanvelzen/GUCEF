/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_TESTAPP_TESTPBKDF2HMACSHA256_H
#define GUCEF_CORE_TESTAPP_TESTPBKDF2HMACSHA256_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests CORE::CPbkdf2HmacSha256 against the PBKDF2-HMAC-SHA256 vectors of RFC 7914 section 11 and the
 *  SHA-256 counterparts of the RFC 6070 inputs: one and several output blocks, a partial last block, NUL bytes in
 *  password and salt, empty password and salt, and the refused arguments
 */
void
PerformPbkdf2HmacSha256Tests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTPBKDF2HMACSHA256_H ? */
