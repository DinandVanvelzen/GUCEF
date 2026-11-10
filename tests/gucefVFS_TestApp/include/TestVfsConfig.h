/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#ifndef GUCEF_VFS_TESTAPP_TESTVFSCONFIG_H
#define GUCEF_VFS_TESTAPP_TESTVFSCONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The config of the VFS: what is loaded is mounted, what is saved is what was configured (not what was mounted at run time) */
void
PerformVfsConfigTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_TESTAPP_TESTVFSCONFIG_H ? */
