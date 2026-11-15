/*
 *  AssetConverterLib: a file written for an asset, with the files it references
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

#include "AssetConverterLib_CConversionReportFile.h"

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

CConversionReportFile::CConversionReportFile( void )
    : path()
    , profile()
    , references()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CConversionReportFile::IsCooked( void ) const
{GUCEF_TRACE;

    return !profile.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CConversionReportFile::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "path", path );
    if ( !profile.IsNULLOrEmpty() )
        node.SetAttribute( "profile", profile );
    node.SetValuesOfChildByName( "references", references );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CConversionReportFile::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    path = node.GetAttributeValueOrChildValueByName( "path" ).AsString();
    profile = node.GetAttributeValueOrChildValueByName( "profile" ).AsString();
    node.GetValuesOfChildByName( "references", references );
    return !path.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
