/*
 *  assetpluginUNREAL: An entry of the export table of a package
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTEXPORT_H
#include "assetpluginUNREAL_CUnrealObjectExport.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTEXPORT_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTEXPORT_H ? */

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

CUnrealObjectExport::CUnrealObjectExport( void )
    : classIndex()
    , superIndex()
    , templateIndex()
    , outerIndex()
    , objectName()
    , objectFlags( 0 )
    , serialSize( 0 )
    , serialOffset( 0 )
    , forcedExport( false )
    , notForClient( false )
    , notForServer( false )
    , packageGuid()
    , isInheritedInstance( false )
    , packageFlags( 0 )
    , notAlwaysLoadedForEditorGame( false )
    , isAsset( false )
    , generatePublicHash( false )
    , firstExportDependency( -1 )
    , serializationBeforeSerializationDependencies( 0 )
    , createBeforeSerializationDependencies( 0 )
    , serializationBeforeCreateDependencies( 0 )
    , createBeforeCreateDependencies( 0 )
    , scriptSerializationStartOffset( 0 )
    , scriptSerializationEndOffset( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CUnrealObjectExport::Read( CUnrealArchiveReader& reader, const CUnrealNameTable& names, const CUnrealVersionContext& versions, UInt32 summaryPackageFlags )
{GUCEF_TRACE;

    classIndex.index = reader.ReadInt32();
    superIndex.index = reader.ReadInt32();
    if ( versions.IsAtLeast( UE4_TEMPLATE_INDEX_IN_COOKED_EXPORTS ) )
        templateIndex.index = reader.ReadInt32();
    outerIndex.index = reader.ReadInt32();
    names.ReadName( reader, objectName );
    objectFlags = reader.ReadUInt32();
    if ( versions.IsAtLeast( UE4_64BIT_EXPORTMAP_SERIALSIZES ) )
    {
        serialSize = reader.ReadInt64();
        serialOffset = reader.ReadInt64();
    }
    else
    {
        serialSize = reader.ReadInt32();
        serialOffset = reader.ReadInt32();
    }
    forcedExport = reader.ReadBool32();
    notForClient = reader.ReadBool32();
    notForServer = reader.ReadBool32();
    if ( !versions.IsAtLeast( UE5_REMOVE_OBJECT_EXPORT_PACKAGE_GUID ) )
        reader.ReadGuid( packageGuid );
    if ( versions.IsAtLeast( UE5_TRACK_OBJECT_EXPORT_IS_INHERITED ) )
        isInheritedInstance = reader.ReadBool32();
    packageFlags = reader.ReadUInt32();
    notAlwaysLoadedForEditorGame = reader.ReadBool32();
    if ( versions.IsAtLeast( UE4_COOKED_ASSETS_IN_EDITOR_SUPPORT ) )
        isAsset = reader.ReadBool32();
    if ( versions.IsAtLeast( UE5_OPTIONAL_RESOURCES ) )
        generatePublicHash = reader.ReadBool32();
    if ( versions.IsAtLeast( UE4_PRELOAD_DEPENDENCIES_IN_COOKED_EXPORTS ) )
    {
        firstExportDependency = reader.ReadInt32();
        serializationBeforeSerializationDependencies = reader.ReadInt32();
        createBeforeSerializationDependencies = reader.ReadInt32();
        serializationBeforeCreateDependencies = reader.ReadInt32();
        createBeforeCreateDependencies = reader.ReadInt32();
    }
    if ( versions.IsAtLeast( UE5_SCRIPT_SERIALIZATION_OFFSET ) && 0 == ( summaryPackageFlags & UE_PKG_UNVERSIONED_PROPERTIES ) )
    {
        scriptSerializationStartOffset = reader.ReadInt64();
        scriptSerializationEndOffset = reader.ReadInt64();
    }
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
