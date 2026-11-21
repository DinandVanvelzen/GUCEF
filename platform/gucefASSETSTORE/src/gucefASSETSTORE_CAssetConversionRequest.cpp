/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#include "gucefASSETSTORE_CAssetConversionRequest.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetConversionRequest::CAssetConversionRequest( void )
    : pack()
    , sourceVfsPath()
    , sourcePrefix()
    , convertedVfsFolder()
    , convertedPathPrefix()
    , cookedVfsFolder()
    , cookedPathPrefix()
    , observer( GUCEF_NULL )
    , reconvert( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetConversionRequest::~CAssetConversionRequest()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetConversionRequest::GetSourceRelativePath( const CAssetRecord& asset ) const
{GUCEF_TRACE;

    CORE::CString relative = asset.originalPath.ReplaceChar( '\\', '/' );
    if ( !sourcePrefix.IsNULLOrEmpty() )
    {
        if ( 0 != relative.HasSubstr( sourcePrefix, true ) )
            return CORE::CString::Empty;
        relative = relative.CutChars( sourcePrefix.Length(), true );
    }
    return relative;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
