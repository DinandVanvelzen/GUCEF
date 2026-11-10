/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

/*
 *  Compiled as C: the header of the file system access results must be readable by C code.
 */

#include <string.h>

#include "gucefCORE_EFileSystemAccessResult.h"

/*-------------------------------------------------------------------------*/

/** 1 when the values and their names are as C sees them */
int
FileSystemAccessResultHeaderWorksInC( void )
{
    TFileSystemAccessResult result = FILESYSTEMACCESS_NOT_FOUND;
    enum EFileSystemAccessResult other = FILESYSTEMACCESS_SUCCESS;
    if ( 10 != (int) result || 0 != (int) other )
        return 0;
    return 0 == strcmp( FileSystemAccessResultToString( result ), "FILESYSTEMACCESS_NOT_FOUND" ) ? 1 : 0;
}

/*-------------------------------------------------------------------------*/
