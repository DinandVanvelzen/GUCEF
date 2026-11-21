/*
 *  AssetConverterLib: what a pack recogniser knows about a recognised pack: engine, entry points, grouping and noise
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

#include "AssetConverterLib_CAssetPackProfile.h"

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

CAssetPackProfile::CAssetPackProfile( void )
    : packType()
    , packTypeName()
    , engine()
    , engineVersion()
    , root()
    , recognisedBy()
    , confidence( 0 )
    , showcaseCandidates()
    , groupingHints()
    , noisePatterns()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetPackProfile::Clear( void )
{GUCEF_TRACE;

    packType.Clear();
    packTypeName.Clear();
    engine.Clear();
    engineVersion.Clear();
    root.Clear();
    recognisedBy.Clear();
    confidence = 0;
    showcaseCandidates.clear();
    groupingHints.clear();
    noisePatterns.clear();
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackProfile::IsRecognised( void ) const
{GUCEF_TRACE;

    return !packType.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackProfile::IsNoise( const CORE::CString& packPath ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<noisePatterns.size(); ++i )
        if ( packPath.WildcardEquals( noisePatterns[ i ], '*', false ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
