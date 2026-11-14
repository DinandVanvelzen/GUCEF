/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

#ifndef GUCEF_MATH_ETYPES_H
#define GUCEF_MATH_ETYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Maximum and minimal values for the simple types which we are about to
 *      define.
 */
#define GUCEF_MATH_INT8MAX     GUCEFCORE_INT8MAX
#define GUCEF_MATH_INT8MIN     GUCEFCORE_INT8MIN
#define GUCEF_MATH_UINT8MAX    GUCEFCORE_UINT8MAX
#define GUCEF_MATH_UINT8MIN    GUCEFCORE_UINT8MIN
#define GUCEF_MATH_INT16MAX    GUCEFCORE_INT16MAX
#define GUCEF_MATH_INT16MIN    GUCEFCORE_INT16MIN
#define GUCEF_MATH_UINT16MAX   GUCEFCORE_UINT16MAX
#define GUCEF_MATH_UINT16MIN   GUCEFCORE_UINT16MIN
#define GUCEF_MATH_INT32MAX    GUCEFCORE_INT32MAX
#define GUCEF_MATH_INT32MIN    GUCEFCORE_INT32MIN
#define GUCEF_MATH_UINT32MAX   GUCEFCORE_UINT32MAX
#define GUCEF_MATH_UINT32MIN   GUCEFCORE_UINT32MIN
#define GUCEF_MATH_FLOAT32MAX  GUCEFCORE_FLOAT32MAX
#define GUCEF_MATH_FLOAT32MIN  GUCEFCORE_FLOAT32MIN

/*-------------------------------------------------------------------------*/

/*
 *  When using C++ we should use namespacing
 */
#ifdef __cplusplus

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

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

typedef CORE::TBuildinDataType TBuildinDataType;

/*-------------------------------------------------------------------------*/

/**
 *  Order of the rotations of Euler angles. The values are stable: new orders are only ever appended.
 */
enum EEulerOrder
{
    EULERORDER_XYZ = 0 ,
    EULERORDER_XZY     ,
    EULERORDER_YXZ     ,
    EULERORDER_YZX     ,
    EULERORDER_ZXY     ,
    EULERORDER_ZYX     ,
    EULERORDER_XYX     ,
    EULERORDER_XZX     ,
    EULERORDER_YXY     ,
    EULERORDER_YZY     ,
    EULERORDER_ZXZ     ,
    EULERORDER_ZYZ
};
typedef enum EEulerOrder TEulerOrder;

/** Whether the rotations of Euler angles are about the axes of the rotating body or about the fixed axes */
enum EEulerFrame
{
    EULERFRAME_INTRINSIC = 0 ,
    EULERFRAME_EXTRINSIC
};
typedef enum EEulerFrame TEulerFrame;

/** Handedness of a coordinate system */
enum EHandedness
{
    HANDEDNESS_RIGHT = 0 ,
    HANDEDNESS_LEFT
};
typedef enum EHandedness THandedness;

/** An axis of a coordinate system with its direction */
enum ESignedAxis
{
    SIGNEDAXIS_POSITIVE_X = 0 ,
    SIGNEDAXIS_NEGATIVE_X     ,
    SIGNEDAXIS_POSITIVE_Y     ,
    SIGNEDAXIS_NEGATIVE_Y     ,
    SIGNEDAXIS_POSITIVE_Z     ,
    SIGNEDAXIS_NEGATIVE_Z
};
typedef enum ESignedAxis TSignedAxis;

/** The range of depth values of clip space */
enum EDepthRange
{
    DEPTHRANGE_ZERO_TO_ONE = 0 ,
    DEPTHRANGE_MINUS_ONE_TO_ONE
};
typedef enum EDepthRange TDepthRange;

/** Normalisation of spherical harmonics. FuMa is defined up to order 3 */
enum EShNormalisation
{
    SHNORM_ORTHONORMAL = 0 ,
    SHNORM_N3D             ,
    SHNORM_SN3D            ,
    SHNORM_FUMA
};
typedef enum EShNormalisation TShNormalisation;

/** Distance attenuation models, the names are those of OpenAL 1.1 plus a curve */
enum EDistanceModel
{
    DISTANCEMODEL_NONE = 0         ,
    DISTANCEMODEL_INVERSE          ,
    DISTANCEMODEL_INVERSE_CLAMPED  ,
    DISTANCEMODEL_LINEAR           ,
    DISTANCEMODEL_LINEAR_CLAMPED   ,
    DISTANCEMODEL_EXPONENT         ,
    DISTANCEMODEL_EXPONENT_CLAMPED ,
    DISTANCEMODEL_CURVE
};
typedef enum EDistanceModel TDistanceModel;

/** The easing curves of Penner: linear and the ten families, each in an in, out and in-out form */
enum EEasingFunction
{
    EASING_LINEAR = 0 ,
    EASING_QUAD_IN    , EASING_QUAD_OUT    , EASING_QUAD_INOUT    ,
    EASING_CUBIC_IN   , EASING_CUBIC_OUT   , EASING_CUBIC_INOUT   ,
    EASING_QUART_IN   , EASING_QUART_OUT   , EASING_QUART_INOUT   ,
    EASING_QUINT_IN   , EASING_QUINT_OUT   , EASING_QUINT_INOUT   ,
    EASING_SINE_IN    , EASING_SINE_OUT    , EASING_SINE_INOUT    ,
    EASING_EXPO_IN    , EASING_EXPO_OUT    , EASING_EXPO_INOUT    ,
    EASING_CIRC_IN    , EASING_CIRC_OUT    , EASING_CIRC_INOUT    ,
    EASING_BACK_IN    , EASING_BACK_OUT    , EASING_BACK_INOUT    ,
    EASING_ELASTIC_IN , EASING_ELASTIC_OUT , EASING_ELASTIC_INOUT ,
    EASING_BOUNCE_IN  , EASING_BOUNCE_OUT  , EASING_BOUNCE_INOUT
};
typedef enum EEasingFunction TEasingFunction;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* __cplusplus ? */

#endif /* GUCEF_MATH_ETYPES_H ? */
