/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#ifndef GUCEF_CORE_ETYPES_H
#define GUCEF_CORE_ETYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_ETYPES_H
#include "gucefMT_ETypes.h"
#define GUCEF_MT_ETYPES_H
#endif /* GUCEF_MT_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Maximum and minimal values for the simple types which we are about to
 *      define.
 */
#define GUCEFCORE_INT8MAX     GUCEFMT_INT8MAX
#define GUCEFCORE_INT8MIN     GUCEFMT_INT8MIN
#define GUCEFCORE_UINT8MAX    GUCEFMT_UINT8MAX
#define GUCEFCORE_UINT8MIN    GUCEFMT_UINT8MIN
#define GUCEFCORE_INT16MAX    GUCEFMT_INT16MAX
#define GUCEFCORE_INT16MIN    GUCEFMT_INT16MIN
#define GUCEFCORE_UINT16MAX   GUCEFMT_UINT16MAX
#define GUCEFCORE_UINT16MIN   GUCEFMT_UINT16MIN
#define GUCEFCORE_INT32MAX    GUCEFMT_INT32MAX
#define GUCEFCORE_INT32MIN    GUCEFMT_INT32MIN
#define GUCEFCORE_UINT32MAX   GUCEFMT_UINT32MAX
#define GUCEFCORE_UINT32MIN   GUCEFMT_UINT32MIN
#define GUCEFCORE_FLOAT32MAX  GUCEFMT_FLOAT32MAX
#define GUCEFCORE_FLOAT32MIN  GUCEFMT_FLOAT32MIN

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

/**
 *  Enumerated type for special directories
 */
enum ESpecialDirs
{
    SPECIALDIR_UNSPECIFIED = 0,

    SPECIALDIR_CURRENT_WORK_DIR ,
    SPECIALDIR_UP_DIR           ,
    SPECIALDIR_MODULE_DIR       ,
    SPECIALDIR_TEMP_DIR         ,
    SPECIALDIR_HOME_DIR         ,
    SPECIALDIR_SYSTEM_DIR       ,
    SPECIALDIR_USER_DIR         ,
    SPECIALDIR_APPDATA_DIR      ,
    SPECIALDIR_COMMON_APPDATA_DIR,
    SPECIALDIR_DESKTOP_DIR      ,
    SPECIALDIR_FAVORITES_DIR    ,
    SPECIALDIR_STARTMENU_DIR    ,
    SPECIALDIR_PROGRAMS_DIR     ,
    SPECIALDIR_X86_PROGRAMS_DIR ,
    SPECIALDIR_X64_PROGRAMS_DIR ,
    SPECIALDIR_RECENT_DIR       ,
    SPECIALDIR_TEMPLATES_DIR    ,
    SPECIALDIR_LOCAL_APPDATA_DIR,
    SPECIALDIR_INTERNET_CACHE_DIR,
    SPECIALDIR_COOKIES_DIR      ,
    SPECIALDIR_HISTORY_DIR      ,
    SPECIALDIR_PROFILE_DIR      ,
    SPECIALDIR_WINDOWS_DIR      ,
    SPECIALDIR_SYSTEM32_DIR     ,
    SPECIALDIR_PROGRAM_FILES_DIR ,
    SPECIALDIR_COMMON_FILES_DIR  ,
    SPECIALDIR_MY_DOCUMENTS_DIR  ,
    SPECIALDIR_MY_PICTURES_DIR   ,
    SPECIALDIR_MY_MUSIC_DIR      ,
    SPECIALDIR_MY_VIDEO_DIR      ,
    SPECIALDIR_MY_GAMES_DIR      ,
    SPECIALDIR_MY_RECENT_DIR     ,
    SPECIALDIR_MY_SEARCHES_DIR   ,
    SPECIALDIR_MY_FAVORITES_DIR  ,
    SPECIALDIR_MY_DOWNLOADS_DIR  ,
    SPECIALDIR_CACHE_DIR         ,  /**< per user data that can be made again: Local AppData on Windows, $XDG_CACHE_HOME (~/.cache) on Linux */
    SPECIALDIR_APP_CONFIG_DIR       /**< per user settings of applications: Roaming AppData (%APPDATA%) on Windows, $XDG_CONFIG_HOME (~/.config) on Linux */
};
typedef enum ESpecialDirs TSpecialDirs;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace CORE */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

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
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

// the below is legacy and deprecated. these are in the root GUCEF namespace now

typedef MT::UInt8 UInt8;    /* 1 byte, unsigned */
typedef MT::Int8 Int8;      /* 1 byte, signed */

typedef MT::UInt16 UInt16;   /* 2 bytes, unsigned */
typedef MT::Int16 Int16;     /* 2 bytes, signed */

typedef MT::UInt32 UInt32;   /* 4 bytes, unsigned */
typedef MT::Int32  Int32;    /* 4 bytes, signed */

typedef MT::Int64 Int64;     /* 8 bytes, signed */
typedef MT::UInt64 UInt64;   /* 8 bytes, unsigned */

typedef MT::Float32 Float32;  /* 4 bytes, signed, decimal */
typedef MT::Float64 Float64;  /* 8 bytes, signed, decimal */

typedef MT::TBuildinDataType TBuildinDataType;
typedef MT::TBasicDataType TBasicDataType;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* __cplusplus ? */

#endif /* GUCEF_CORE_ETYPES_H ? */
