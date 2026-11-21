/*
 *  assetpluginUNREAL: An entry of the import table of a package
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#include "assetpluginUNREAL_UnrealVersions.h"
#define GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTIMPORT_H
#include "assetpluginUNREAL_CUnrealObjectImport.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTIMPORT_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTIMPORT_H ? */

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

CUnrealObjectImport::CUnrealObjectImport( void )
    : classPackage()
    , className()
    , outerIndex()
    , objectName()
    , packageName()
    , isOptional( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CUnrealObjectImport::Read( CUnrealArchiveReader& reader, const CUnrealNameTable& names, const CUnrealVersionContext& versions, UInt32 packageFlags )
{GUCEF_TRACE;

    names.ReadName( reader, classPackage );
    names.ReadName( reader, className );
    outerIndex.index = reader.ReadInt32();
    names.ReadName( reader, objectName );
    if ( versions.IsAtLeast( UE4_NON_OUTER_PACKAGE_IMPORT ) && 0 == ( packageFlags & UE_PKG_FILTER_EDITOR_ONLY ) )
        names.ReadName( reader, packageName );
    if ( versions.IsAtLeast( UE5_OPTIONAL_RESOURCES ) )
        isOptional = reader.ReadBool32();
    return !reader.HasError();
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
