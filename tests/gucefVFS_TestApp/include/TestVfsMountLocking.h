/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#ifndef GUCEF_VFS_TESTAPP_TESTVFSMOUNTLOCKING_H
#define GUCEF_VFS_TESTAPP_TESTVFSMOUNTLOCKING_H

/**
 *  Tests that a mount does not hold the lock of the VFS while its archive is made: the directory watcher of a file system archive
 *  subscribes to the pulse generator when it is made, so a mount on a worker thread waits for the lock of the pulse generator. The
 *  thread that holds that lock (the pulse thread, while it notifies) must still be able to use the VFS meanwhile, else the two
 *  wait for each other until the lock timeouts.
 */
void
PerformVfsMountLockingTests( void );

#endif /* GUCEF_VFS_TESTAPP_TESTVFSMOUNTLOCKING_H ? */
