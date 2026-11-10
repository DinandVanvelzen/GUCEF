/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefCORE_EFileSystemAccessResult.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

extern "C" {

const char*
FileSystemAccessResultToString( TFileSystemAccessResult result )
{
    switch ( result )
    {
        case FILESYSTEMACCESS_SUCCESS:              return "FILESYSTEMACCESS_SUCCESS";
        case FILESYSTEMACCESS_PARTIAL_SUCCESS:      return "FILESYSTEMACCESS_PARTIAL_SUCCESS";
        case FILESYSTEMACCESS_NOT_FOUND:            return "FILESYSTEMACCESS_NOT_FOUND";
        case FILESYSTEMACCESS_PARENT_NOT_FOUND:     return "FILESYSTEMACCESS_PARENT_NOT_FOUND";
        case FILESYSTEMACCESS_ALREADY_EXISTS:       return "FILESYSTEMACCESS_ALREADY_EXISTS";
        case FILESYSTEMACCESS_NOT_A_DIRECTORY:      return "FILESYSTEMACCESS_NOT_A_DIRECTORY";
        case FILESYSTEMACCESS_NOT_A_FILE:           return "FILESYSTEMACCESS_NOT_A_FILE";
        case FILESYSTEMACCESS_DIRECTORY_NOT_EMPTY:  return "FILESYSTEMACCESS_DIRECTORY_NOT_EMPTY";
        case FILESYSTEMACCESS_ACCESS_DENIED:        return "FILESYSTEMACCESS_ACCESS_DENIED";
        case FILESYSTEMACCESS_READ_ONLY:            return "FILESYSTEMACCESS_READ_ONLY";
        case FILESYSTEMACCESS_INVALID_PATH:         return "FILESYSTEMACCESS_INVALID_PATH";
        case FILESYSTEMACCESS_INVALID_NAME:         return "FILESYSTEMACCESS_INVALID_NAME";
        case FILESYSTEMACCESS_SAME_PATH:            return "FILESYSTEMACCESS_SAME_PATH";
        case FILESYSTEMACCESS_CROSS_DEVICE:         return "FILESYSTEMACCESS_CROSS_DEVICE";
        case FILESYSTEMACCESS_IN_USE:               return "FILESYSTEMACCESS_IN_USE";
        case FILESYSTEMACCESS_NO_SPACE:             return "FILESYSTEMACCESS_NO_SPACE";
        case FILESYSTEMACCESS_TOO_LONG:             return "FILESYSTEMACCESS_TOO_LONG";
        case FILESYSTEMACCESS_NOT_SUPPORTED:        return "FILESYSTEMACCESS_NOT_SUPPORTED";
        case FILESYSTEMACCESS_QUEUEING_FAILED:      return "FILESYSTEMACCESS_QUEUEING_FAILED";
        case FILESYSTEMACCESS_CANCELLED:            return "FILESYSTEMACCESS_CANCELLED";
        case FILESYSTEMACCESS_TIMEOUT:              return "FILESYSTEMACCESS_TIMEOUT";
        case FILESYSTEMACCESS_FAILED:               return "FILESYSTEMACCESS_FAILED";
        default:                                    return "FILESYSTEMACCESS_UNKNOWN_RESULT";
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
