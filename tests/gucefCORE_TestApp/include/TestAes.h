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

#ifndef GUCEF_CORE_TESTAPP_TESTAES_H
#define GUCEF_CORE_TESTAPP_TESTAES_H

/**
 *  Tests CORE::CAes against the example vectors of FIPS-197 Appendix C (AES-128, AES-192, AES-256) and the CBC vectors of
 *  NIST SP 800-38A F.2.1 to F.2.6, the PKCS#7 padding of CbcEncrypt/CbcDecrypt and the refusals (key size, partial block,
 *  padding that is not valid).
 */
void
PerformAesTests( void );

#endif /* GUCEF_CORE_TESTAPP_TESTAES_H ? */
