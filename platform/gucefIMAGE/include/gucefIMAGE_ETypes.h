/*
 *  gucefIMAGE: GUCEF module providing image utilities
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#ifndef GUCEF_IMAGE_ETYPES_H
#define GUCEF_IMAGE_ETYPES_H

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

/*
 *      Maximum and minimal values for the simple types which we are about to
 *      define.
 */
#define GUCEFIMAGE_INT8MAX     GUCEFCORE_INT8MAX
#define GUCEFIMAGE_INT8MIN     GUCEFCORE_INT8MIN
#define GUCEFIMAGE_UINT8MAX    GUCEFCORE_UINT8MAX
#define GUCEFIMAGE_UINT8MIN    GUCEFCORE_UINT8MIN
#define GUCEFIMAGE_INT16MAX    GUCEFCORE_INT16MAX
#define GUCEFIMAGE_INT16MIN    GUCEFCORE_INT16MIN
#define GUCEFIMAGE_UINT16MAX   GUCEFCORE_UINT16MAX
#define GUCEFIMAGE_UINT16MIN   GUCEFCORE_UINT16MIN
#define GUCEFIMAGE_INT32MAX    GUCEFCORE_INT32MAX
#define GUCEFIMAGE_INT32MIN    GUCEFCORE_INT32MIN
#define GUCEFIMAGE_UINT32MAX   GUCEFCORE_UINT32MAX
#define GUCEFIMAGE_UINT32MIN   GUCEFCORE_UINT32MIN
#define GUCEFIMAGE_FLOAT32MAX  GUCEFCORE_FLOAT32MAX
#define GUCEFIMAGE_FLOAT32MIN  GUCEFCORE_FLOAT32MIN

/*--------------------------------------------------------------------------*/

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
namespace IMAGE {

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

typedef CORE::CString          CString;
typedef CORE::TBuildinDataType TBuildinDataType;

/*-------------------------------------------------------------------------//
//                                                                         //
//      PROCESSING TYPES                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The colour space the colour channels of a picture are stored in */
enum EColourSpace
{
    COLOURSPACE_LINEAR = 0 ,
    COLOURSPACE_SRGB
};
typedef enum EColourSpace TColourSpace;

/** The filter that weighs the source pixels when a picture is resampled */
enum EResampleFilter
{
    RESAMPLEFILTER_BOX = 0 ,
    RESAMPLEFILTER_TRIANGLE ,
    RESAMPLEFILTER_KAISER ,
    RESAMPLEFILTER_LANCZOS3
};
typedef enum EResampleFilter TResampleFilter;

/** What a filter does with the pixels beyond the edge of the picture */
enum EWrapMode
{
    WRAPMODE_CLAMP = 0 ,
    WRAPMODE_REPEAT ,
    WRAPMODE_MIRROR
};
typedef enum EWrapMode TWrapMode;

/** What the pixels of a picture mean, which decides how they are filtered */
enum EImageContent
{
    IMAGECONTENT_COLOUR = 0 ,
    IMAGECONTENT_NORMALMAP ,
    IMAGECONTENT_DATA          /**< masks, roughness, metallic, occlusion: linear, never treated as colour */
};
typedef enum EImageContent TImageContent;

/*-------------------------------------------------------------------------*/

/** The names of the values, as used by settings files: "linear", "srgb" */
inline CString ToString( TColourSpace value )
{
    return COLOURSPACE_SRGB == value ? CString( "srgb" ) : CString( "linear" );
}

inline bool FromString( const CString& name, TColourSpace& value )
{
    if ( name.Equals( "linear", false ) ) { value = COLOURSPACE_LINEAR; return true; }
    if ( name.Equals( "srgb", false ) ) { value = COLOURSPACE_SRGB; return true; }
    return false;
}

/** "box", "triangle", "kaiser", "lanczos3" */
inline CString ToString( TResampleFilter value )
{
    switch ( value )
    {
        case RESAMPLEFILTER_BOX : return CString( "box" );
        case RESAMPLEFILTER_TRIANGLE : return CString( "triangle" );
        case RESAMPLEFILTER_LANCZOS3 : return CString( "lanczos3" );
        default : return CString( "kaiser" );
    }
}

inline bool FromString( const CString& name, TResampleFilter& value )
{
    if ( name.Equals( "box", false ) ) { value = RESAMPLEFILTER_BOX; return true; }
    if ( name.Equals( "triangle", false ) ) { value = RESAMPLEFILTER_TRIANGLE; return true; }
    if ( name.Equals( "kaiser", false ) ) { value = RESAMPLEFILTER_KAISER; return true; }
    if ( name.Equals( "lanczos3", false ) ) { value = RESAMPLEFILTER_LANCZOS3; return true; }
    return false;
}

/** "clamp", "repeat", "mirror" */
inline CString ToString( TWrapMode value )
{
    switch ( value )
    {
        case WRAPMODE_REPEAT : return CString( "repeat" );
        case WRAPMODE_MIRROR : return CString( "mirror" );
        default : return CString( "clamp" );
    }
}

inline bool FromString( const CString& name, TWrapMode& value )
{
    if ( name.Equals( "clamp", false ) ) { value = WRAPMODE_CLAMP; return true; }
    if ( name.Equals( "repeat", false ) ) { value = WRAPMODE_REPEAT; return true; }
    if ( name.Equals( "mirror", false ) ) { value = WRAPMODE_MIRROR; return true; }
    return false;
}

/** "colour", "normalmap", "data" */
inline CString ToString( TImageContent value )
{
    switch ( value )
    {
        case IMAGECONTENT_NORMALMAP : return CString( "normalmap" );
        case IMAGECONTENT_DATA : return CString( "data" );
        default : return CString( "colour" );
    }
}

inline bool FromString( const CString& name, TImageContent& value )
{
    if ( name.Equals( "colour", false ) || name.Equals( "color", false ) ) { value = IMAGECONTENT_COLOUR; return true; }
    if ( name.Equals( "normalmap", false ) ) { value = IMAGECONTENT_NORMALMAP; return true; }
    if ( name.Equals( "data", false ) ) { value = IMAGECONTENT_DATA; return true; }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* __cplusplus ? */

#endif /* GUCEF_IMAGE_ETYPES_H ? */