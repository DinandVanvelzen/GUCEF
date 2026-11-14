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

#ifndef GUCEF_VFS_ETYPES_H
#define GUCEF_VFS_ETYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Maximum and minimal values for the simple types which we are about to
 *      define.
 */
#define GUCEFVFS_INT8MAX     GUCEFCORE_INT8MAX
#define GUCEFVFS_INT8MIN     GUCEFCORE_INT8MIN
#define GUCEFVFS_UINT8MAX    GUCEFCORE_UINT8MAX
#define GUCEFVFS_UINT8MIN    GUCEFCORE_UINT8MIN
#define GUCEFVFS_INT16MAX    GUCEFCORE_INT16MAX
#define GUCEFVFS_INT16MIN    GUCEFCORE_INT16MIN
#define GUCEFVFS_UINT16MAX   GUCEFCORE_UINT16MAX
#define GUCEFVFS_UINT16MIN   GUCEFCORE_UINT16MIN
#define GUCEFVFS_INT32MAX    GUCEFCORE_INT32MAX
#define GUCEFVFS_INT32MIN    GUCEFCORE_INT32MIN
#define GUCEFVFS_UINT32MAX   GUCEFCORE_UINT32MAX
#define GUCEFVFS_UINT32MIN   GUCEFCORE_UINT32MIN
#define GUCEFVFS_FLOAT32MAX  GUCEFCORE_FLOAT32MAX
#define GUCEFVFS_FLOAT32MIN  GUCEFCORE_FLOAT32MIN

/*--------------------------------------------------------------------------*/

#define GUCEF_VFS_FILE_PROPERTY_UNKNOWN             0
#define GUCEF_VFS_FILE_PROPERTY_NAME                1
#define GUCEF_VFS_FILE_PROPERTY_BYTESIZE            2
#define GUCEF_VFS_FILE_PROPERTY_CREATIONDATETIME    3
#define GUCEF_VFS_FILE_PROPERTY_MODIFIEDDATETIME    4
#define GUCEF_VFS_FILE_PROPERTY_HASH                5

/*--------------------------------------------------------------------------*/

/*
 *  We only have to define types when using C++ due to namespacing 
 *  For C the gucefMT versions are automaticly used in the global namespace
 */
#ifdef __cplusplus
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::UInt8 UInt8;    /* 1 byte, unsigned */
typedef CORE::Int8 Int8;      /* 1 byte, signed */

typedef CORE::UInt16 UInt16;   /* 2 bytes, unsigned */
typedef CORE::Int16 Int16;     /* 2 bytes, signed */

typedef CORE::UInt32 UInt32;   /* 4 bytes, unsigned */
typedef CORE::Int32  Int32;    /* 4 bytes, signed */

typedef CORE::Int64 Int64;     /* 8 bytes, signed */
typedef CORE::UInt64 UInt64;   /* 8 bytes, unsigned */

typedef CORE::Float32 Float32;  /* 4 bytes, signed, decimal */
typedef CORE::Float64 Float64;  /* 8 bytes, signed, decimal */

typedef CORE::CString CString;

/*--------------------------------------------------------------------------*/

enum EVfsFilePropertyTypes GUCEF_ENUM_BASE( Int32 )
{
    Unknown             = GUCEF_VFS_FILE_PROPERTY_UNKNOWN,
    Name                = GUCEF_VFS_FILE_PROPERTY_NAME,
    ByteSize            = GUCEF_VFS_FILE_PROPERTY_BYTESIZE,
    CreationDateTime    = GUCEF_VFS_FILE_PROPERTY_CREATIONDATETIME,
    ModifiedDateTime    = GUCEF_VFS_FILE_PROPERTY_MODIFIEDDATETIME,
    Hash                = GUCEF_VFS_FILE_PROPERTY_HASH
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* __cplusplus ? */

#endif /* GUCEF_VFS_ETYPES_H ? */
