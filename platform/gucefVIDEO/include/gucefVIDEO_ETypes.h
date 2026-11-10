/*
 *  gucefVIDEO: GUCEF module providing video decoding and playback
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

#ifndef GUCEF_VIDEO_ETYPES_H
#define GUCEF_VIDEO_ETYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifdef __cplusplus
#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Frame count of a video that does not tell how long it is */
#define GUCEF_VIDEO_FRAMECOUNT_UNKNOWN      GUCEF_UINT64MAX

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace VIDEO {
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
typedef CORE::CString CString;
#endif /* __cplusplus ? */

/**
 *  Why a playing video stopped. The values are stable, new ones are only ever appended.
 */
enum EVideoEndReason
{
    VIDEOEND_NONE = 0   ,       /**< it did not end (yet) */
    VIDEOEND_FINISHED   ,       /**< the video played to its end */
    VIDEOEND_STOPPED    ,       /**< it was stopped (skipped) before its end */
    VIDEOEND_FAILED             /**< the data could not be decoded */
};
typedef enum EVideoEndReason TVideoEndReason;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace VIDEO */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VIDEO_ETYPES_H ? */
