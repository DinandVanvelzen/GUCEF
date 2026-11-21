/*
 *  AssetConverterLib: a file extension that a plugin or the built-in table says is of a kind of asset
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "AssetConverterLib_CAssetFileKind.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetFileKind::CAssetFileKind( void )
    : extension()
    , kind( ASSETKIND_OTHER )
    , ownerExtensions()
    , source()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetFileKind::CAssetFileKind( const CORE::CString& fileExtension, EAssetKind fileKind )
    : extension( fileExtension.Lowercase() )
    , kind( fileKind )
    , ownerExtensions()
    , source()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetFileKind::CAssetFileKind( const CORE::CString& fileExtension, EAssetKind fileKind, const CORE::CString& owners )
    : extension( fileExtension.Lowercase() )
    , kind( fileKind )
    , ownerExtensions( owners.Lowercase().ParseElements( ' ', false ) )
    , source()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
