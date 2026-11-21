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

#ifndef GUCEF_CORE_EFILESYSTEMACCESSRESULT_H
#define GUCEF_CORE_EFILESYSTEMACCESSRESULT_H

/*-------------------------------------------------------------------------*/

/*
 *      The result codes of file system operations. This header can be read by C code as well as C++ code: the enum is in the
 *      GUCEF::CORE namespace only for C++, the values carry a prefix and fixed numbers.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"     /* often used gucef macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace CORE {
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

enum EFileSystemAccessResult
{
    FILESYSTEMACCESS_SUCCESS                = 0  ,    /**< the operation was done */
    FILESYSTEMACCESS_PARTIAL_SUCCESS        = 1  ,    /**< an operation on several entries did some of them */

    FILESYSTEMACCESS_NOT_FOUND              = 10 ,    /**< the entry is not there */
    FILESYSTEMACCESS_PARENT_NOT_FOUND       = 11 ,    /**< the directory the entry should be in is not there */
    FILESYSTEMACCESS_ALREADY_EXISTS         = 12 ,    /**< the target exists and may not be replaced */
    FILESYSTEMACCESS_NOT_A_DIRECTORY        = 13 ,    /**< a directory was expected */
    FILESYSTEMACCESS_NOT_A_FILE             = 14 ,    /**< a file was expected */
    FILESYSTEMACCESS_DIRECTORY_NOT_EMPTY    = 15 ,    /**< a directory to delete has contents and deleting them was not asked */

    FILESYSTEMACCESS_ACCESS_DENIED          = 20 ,    /**< the entry or its directory is read-only for the user */
    FILESYSTEMACCESS_READ_ONLY              = 21 ,    /**< the place cannot be written at all, for example a mount that is not writeable */

    FILESYSTEMACCESS_INVALID_PATH           = 30 ,    /**< the path cannot be used */
    FILESYSTEMACCESS_INVALID_NAME           = 31 ,    /**< a new name is empty or has a directory separator */
    FILESYSTEMACCESS_SAME_PATH              = 32 ,    /**< source and target are the same */
    FILESYSTEMACCESS_CROSS_DEVICE           = 33 ,    /**< source and target are on devices the operation cannot cross */
    FILESYSTEMACCESS_IN_USE                 = 34 ,    /**< the entry is in use */
    FILESYSTEMACCESS_NO_SPACE               = 35 ,    /**< there is no room left */
    FILESYSTEMACCESS_TOO_LONG               = 36 ,    /**< the path or name is longer than the file system allows */

    FILESYSTEMACCESS_NOT_SUPPORTED          = 40 ,    /**< the file system cannot do this */

    FILESYSTEMACCESS_QUEUEING_FAILED        = 50 ,    /**< an async operation could not be queued */
    FILESYSTEMACCESS_CANCELLED              = 51 ,    /**< an async operation was cancelled */
    FILESYSTEMACCESS_TIMEOUT                = 52 ,    /**< an async operation did not end in time */

    FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING   = 60 ,    /**< the place is remote and there is no usable link to it right now: connecting, reconnecting or offline. A retry later can work */
    FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED     = 61 ,    /**< the remote resource changed since it was opened or listed: opening it again gives the new version */
    FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER = 62 ,    /**< the remote side is at a capacity limit (requests in flight, open handles, listings): try again later */
    FILESYSTEMACCESS_RESOURCE_CORRUPT            = 63 ,    /**< the data did not pass verification (for example a digest mismatch) */
    FILESYSTEMACCESS_REMOTE_PROTOCOL_ERROR       = 64 ,    /**< the remote side did not follow the protocol or gave an answer that makes no sense */
    FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE    = 65 ,    /**< the place is remote, the link may be fine, the data is not held locally and the calling thread must not wait for it: a background fetch was started (or was already under way) and an arrival or failure event for the path follows */

    FILESYSTEMACCESS_FAILED                 = 99      /**< the operation failed for a reason that cannot be told */
};
typedef enum EFileSystemAccessResult TFileSystemAccessResult;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus ? */

/**
 *  The name of a result code, for example "FILESYSTEMACCESS_NOT_FOUND". Never NULL; unknown values give
 *  "FILESYSTEMACCESS_UNKNOWN_RESULT".
 */
GUCEF_CORE_PUBLIC_C const char*
FileSystemAccessResultToString( TFileSystemAccessResult result );

#ifdef __cplusplus
}
#endif /* __cplusplus ? */

#ifdef __cplusplus

/** Whether a result means the operation did what was asked (some of it, for PARTIAL_SUCCESS: not counted as succeeded) */
inline bool
FileSystemAccessSucceeded( TFileSystemAccessResult result ) { return FILESYSTEMACCESS_SUCCESS == result; }

#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace CORE */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_EFILESYSTEMACCESSRESULT_H ? */
