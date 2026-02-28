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

#ifndef GUCEF_CORE_CSTRING_H
#define GUCEF_CORE_CSTRING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CASCIISTRING_H
#include "gucefCORE_CAsciiString.h"
#define GUCEF_CORE_CASCIISTRING_H
#endif /* GUCEF_CORE_CASCIISTRING_H ? */

#ifndef GUCEF_CORE_CUTF8STRING_H
#include "gucefCORE_CUtf8String.h"
#define GUCEF_CORE_CUTF8STRING_H
#endif /* GUCEF_CORE_CUTF8STRING_H ? */

#ifndef GUCEF_CORE_CUTF16STRING_H
#include "gucefCORE_CUtf16String.h"
#define GUCEF_CORE_CUTF16STRING_H
#endif /* GUCEF_CORE_CUTF16STRING_H ? */

#ifndef GUCEF_CORE_CUTF32STRING_H
#include "gucefCORE_CUtf32String.h"
#define GUCEF_CORE_CUTF32STRING_H
#endif /* GUCEF_CORE_CUTF32STRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_ASCII_STRING )

typedef CAsciiString                          CString;
typedef CAsciiString::StringSet               CStringSet;
typedef CAsciiString::StringVector            CStringVector;
typedef CAsciiString::StringMap               CStringMap;
typedef CAsciiString::StringMapMap            CStringMapMap;
typedef CAsciiString::StringMapMapMap         CStringMapMapMap;
typedef CAsciiString::StringMapMapMapMap      CStringMapMapMapMap;
typedef CAsciiString::StringMapSet            CStringMapSet;
typedef CAsciiString::StringMapMapSet         CStringMapMapSet;
typedef CAsciiString::StringMapMapMapSet      CStringMapMapMapSet;
typedef CAsciiString::StringUMap              CStringUMap;
typedef CAsciiString::StringUMapUMap          CStringUMapUMap;
typedef CAsciiString::StringUMapUMapUMap      CStringUMapUMapUMap;
typedef CAsciiString::StringUMapUMapUMapUMap  CStringUMapUMapUMapUMap;
typedef CAsciiString::StringUMapSet           CStringUMapSet;
typedef CAsciiString::StringUMapUMapSet       CStringUMapUMapSet;
typedef CAsciiString::StringUMapUMapUMapSet   CStringUMapUMapUMapSet;

#define GUCEF_DATATYPE_DATETIME_ISO8601_STRING     GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING
#define GUCEF_DATATYPE_STRING                      GUCEF_DATATYPE_ASCII_STRING

#elif ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_UTF8_STRING )

typedef CUtf8String                          CString;
typedef CUtf8String::StringSet               CStringSet;
typedef CUtf8String::StringVector            CStringVector;
typedef CUtf8String::StringMap               CStringMap;
typedef CUtf8String::StringMapMap            CStringMapMap;
typedef CUtf8String::StringMapMapMap         CStringMapMapMap;
typedef CUtf8String::StringMapMapMapMap      CStringMapMapMapMap;
typedef CUtf8String::StringMapSet            CStringMapSet;
typedef CUtf8String::StringMapMapSet         CStringMapMapSet;
typedef CUtf8String::StringMapMapMapSet      CStringMapMapMapSet;
typedef CUtf8String::StringUMap              CStringUMap;
typedef CUtf8String::StringUMapUMap          CStringUMapUMap;
typedef CUtf8String::StringUMapUMapUMap      CStringUMapUMapUMap;
typedef CUtf8String::StringUMapUMapUMapUMap  CStringUMapUMapUMapUMap;
typedef CUtf8String::StringUMapSet           CStringUMapSet;
typedef CUtf8String::StringUMapUMapSet       CStringUMapUMapSet;
typedef CUtf8String::StringUMapUMapUMapSet   CStringUMapUMapUMapSet;

#define GUCEF_DATATYPE_DATETIME_ISO8601_STRING     GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING
#define GUCEF_DATATYPE_STRING                      GUCEF_DATATYPE_UTF8_STRING

#elif ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_UTF16_STRING )

/*
 *  UTF-16 string container typedefs — always available regardless of the CString default
 */
typedef CUtf16String                          CString;
typedef CUtf16String::StringSet               CStringSet;
typedef CUtf16String::StringVector            CStringVector;
typedef CUtf16String::StringMap               CStringMap;
typedef CUtf16String::StringMapMap            CStringMapMap;
typedef CUtf16String::StringMapSet            CStringMapSet;
typedef CUtf16String::StringMapMapSet         CStringMapMapSet;
typedef CUtf16String::StringUMap              CStringUMap;
typedef CUtf16String::StringUMapUMap          CStringUMapUMap;
typedef CUtf16String::StringUMapSet           CStringUMapSet;
typedef CUtf16String::StringUMapUMapSet       CStringUMapUMapSet;

#define GUCEF_DATATYPE_DATETIME_ISO8601_STRING     GUCEF_DATATYPE_DATETIME_ISO8601_UTF16_STRING
#define GUCEF_DATATYPE_STRING                      GUCEF_DATATYPE_UTF16_STRING

#elif ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_UTF32_STRING )

/*
 *  UTF-32 string container typedefs — always available regardless of the CString default
 */
typedef CUtf32String                          CString;
typedef CUtf32String::StringSet               CStringSet;
typedef CUtf32String::StringVector            CStringVector;
typedef CUtf32String::StringMap               CStringMap;
typedef CUtf32String::StringMapMap            CStringMapMap;
typedef CUtf32String::StringMapSet            CStringMapSet;
typedef CUtf32String::StringMapMapSet         CStringMapMapSet;
typedef CUtf32String::StringUMap              CStringUMap;
typedef CUtf32String::StringUMapUMap          CStringUMapUMap;
typedef CUtf32String::StringUMapSet           CStringUMapSet;
typedef CUtf32String::StringUMapUMapSet       CStringUMapUMapSet;

#define GUCEF_DATATYPE_DATETIME_ISO8601_STRING     GUCEF_DATATYPE_DATETIME_ISO8601_UTF32_STRING
#define GUCEF_DATATYPE_STRING                      GUCEF_DATATYPE_UTF32_STRING

#endif

/*
 *  When more UTF variants are supported "UtfString" will represent the default
 *  UTF implementation used on the targeted platform
 */
typedef CUtf8String                         CUtfString;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSTRING_H ? */
