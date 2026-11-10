/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#ifndef GUCEF_VFS_TESTAPP_TESTCUEBINARCHIVE_H
#define GUCEF_VFS_TESTAPP_TESTCUEBINARCHIVE_H

/**
 *  Tests the CUE/BIN plugin on CD images that the test builds itself: raw and cooked data tracks read through the
 *  ISO 9660 plugin, audio tracks as WAVE files, image files found by the name of the sheet, and sheets that are refused
 */
void
PerformCueBinArchiveTests( void );

#endif /* GUCEF_VFS_TESTAPP_TESTCUEBINARCHIVE_H ? */
