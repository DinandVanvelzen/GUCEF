/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef TESTASSETSTOREREMOTESERVICE_H
#define TESTASSETSTOREREMOTESERVICE_H

/** The pieces of the remote service on their own: contracts, the records of the operations, settings, pairing */
void PerformAssetStoreRemoteUnitTests( void );

/** The equivalence suite against a remote client of a host on the loopback, the server side on the vfstables backend */
void PerformAssetStoreRemoteServiceVfsTablesTests( void );

/** The same with the server side on PostgreSQL, when GUCEF_TEST_POSTGRESQL names a test database; skipped otherwise */
void PerformAssetStoreRemoteServicePostgreSqlTests( void );

/** Faults of the network and the service: a cut during a listing, a restart that replays a mutation, an upload cut and resumed, three clients */
void PerformAssetStoreRemoteFaultTests( void );

/** A machine caller (the role mcp) is refused the sign in and the keyring, and gets no prompts: a world of its own */
void PerformAssetStoreRemoteSignInRoleTests( void );

/** Against a running service (GUCEF_TEST_ASSETSTORE_REMOTE_ENDPOINT and GUCEF_TEST_ASSETSTORE_PAIRING_CODE): pairing, a search, an upload read back; skipped without them */
void PerformAssetStoreRemoteLiveSmokeTests( void );

#endif /* TESTASSETSTOREREMOTESERVICE_H ? */
