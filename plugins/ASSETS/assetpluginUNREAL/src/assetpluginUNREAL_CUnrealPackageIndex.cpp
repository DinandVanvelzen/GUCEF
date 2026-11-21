/*
 *  assetpluginUNREAL: A reference to an import or an export of a package, or null
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGEINDEX_H
#include "assetpluginUNREAL_CUnrealPackageIndex.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGEINDEX_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGEINDEX_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnrealPackageIndex::CUnrealPackageIndex( void )
    : index( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnrealPackageIndex::CUnrealPackageIndex( Int32 value )
    : index( value )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackageIndex::IsNull( void ) const
{GUCEF_TRACE;

    return 0 == index;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackageIndex::IsImport( void ) const
{GUCEF_TRACE;

    return index < 0;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackageIndex::IsExport( void ) const
{GUCEF_TRACE;

    return index > 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealPackageIndex::ToImportIndex( void ) const
{GUCEF_TRACE;

    return index < 0 ? (UInt32) ( -(Int64) index - 1 ) : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealPackageIndex::ToExportIndex( void ) const
{GUCEF_TRACE;

    return index > 0 ? (UInt32) ( index - 1 ) : 0;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackageIndex::operator==( const CUnrealPackageIndex& other ) const
{GUCEF_TRACE;

    return index == other.index;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
