/*
 *  AssetConverterLib: the interface of the importers of source formats
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

#include "AssetConverterLib_CIAssetImporter.h"

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

CIAssetImporter::CIAssetImporter( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIAssetImporter::~CIAssetImporter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIAssetImporter::IndexPack( CAssetPackContext& /* context */ )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

void
CIAssetImporter::GetDefaultSettings( CORE::CDataNode& /* settings */ ) const
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CIAssetImporter::GetDefaultPostProcessors( CORE::CString::StringVector& names ) const
{GUCEF_TRACE;

    names.clear();
    names.push_back( "NameHints" );
    names.push_back( "GenerateTangents" );
    names.push_back( "PruneEmptyNodes" );
    names.push_back( "MergeIdenticalMaterials" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
