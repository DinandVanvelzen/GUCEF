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

typedef CAsciiString                        CString;
typedef CAsciiString::StringSet             CStringSet;
typedef CAsciiString::StringMap             CStringMap;
typedef CAsciiString::StringMapMap          CStringMapMap;
typedef CAsciiString::StringMapMapMap       CStringMapMapMap;
typedef CAsciiString::StringMapMapMapMap    CStringMapMapMapMap;
typedef CAsciiString::StringMapSet          CStringMapSet;
typedef CAsciiString::StringMapMapSet       CStringMapMapSet;
typedef CAsciiString::StringMapMapMapSet    CStringMapMapMapSet;

#define GUCEF_DATATYPE_DATETIME_ISO8601_STRING     GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING
#define GUCEF_DATATYPE_STRING                      GUCEF_DATATYPE_ASCII_STRING

#else

typedef CUtf8String                         CString;
typedef CUtf8String::StringSet              CStringSet;
typedef CUtf8String::StringMap              CStringMap;
typedef CUtf8String::StringMapMap           CStringMapMap;
typedef CUtf8String::StringMapMapMap        CStringMapMapMap;
typedef CUtf8String::StringMapMapMapMap     CStringMapMapMapMap;
typedef CUtf8String::StringMapSet           CStringMapSet;
typedef CUtf8String::StringMapMapSet        CStringMapMapSet;
typedef CUtf8String::StringMapMapMapSet     CStringMapMapMapSet;

#define GUCEF_DATATYPE_DATETIME_ISO8601_STRING     GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING
#define GUCEF_DATATYPE_STRING                      GUCEF_DATATYPE_UTF8_STRING

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
