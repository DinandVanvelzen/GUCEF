/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_UTILS_H
#define GUCEF_GUI_UTILS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef GUCEF::vector< UInt32 > TCodePointVector;
typedef GUCEF::vector< CString > TStringVector;

/** The code points of UTF-8 text; a broken sequence yields U+FFFD and decoding goes on */
GUCEF_GUI_PUBLIC_CPP void DecodeUtf8( const CString& text, TCodePointVector& codePoints );

/** Joins list entries with '\n', the form of a StringList property value */
GUCEF_GUI_PUBLIC_CPP CString JoinStringList( const TStringVector& entries );

/** Splits a StringList property value; an empty value is an empty list */
GUCEF_GUI_PUBLIC_CPP TStringVector SplitStringList( const CString& value );

/** Splits an IntList property value ("1,4,7"); entries that are no number are skipped */
GUCEF_GUI_PUBLIC_CPP GUCEF::vector< Int32 > SplitIntList( const CString& value );

GUCEF_GUI_PUBLIC_CPP CString JoinIntList( const GUCEF::vector< Int32 >& values );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_UTILS_H ? */
