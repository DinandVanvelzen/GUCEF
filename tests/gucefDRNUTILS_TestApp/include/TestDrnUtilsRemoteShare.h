/*
 *  gucefDRNUTILS_TestApp: GUCEF test application
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

#ifndef GUCEF_TESTDRNUTILSREMOTESHARE_H
#define GUCEF_TESTDRNUTILSREMOTESHARE_H

/**
 *  The shares of a remote service mounted into the VFS: CRemoteShareMount over a paired connection to a service host with a share (mounted before the
 *  connection is up, online, a lost link, a new session, a name that is taken, stopping, the channel), and CRemoteShareMounts built from a configuration
 */
void PerformDrnUtilsRemoteShareTests( void );

/**
 *  The shares that a CRemoteServiceHost serves (RegisterShare(), the policy of the shares, the decision of a share), over a paired device that mounts them, the host
 *  restarted, the roles of the registry changed and a device revoked; and the registration of a session that several mounts of one connection hold together
 */
void PerformDrnUtilsRemoteShareHostTests( void );

#endif /* GUCEF_TESTDRNUTILSREMOTESHARE_H ? */
