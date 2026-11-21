/*
 *  assetpluginUNREAL: The StaticMesh export: material slots, bounds and the mesh descriptions of its source models
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHREADER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMESHDESCRIPTION_H
#include "assetpluginUNREAL_CUnrealMeshDescription.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMESHDESCRIPTION_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMESHDESCRIPTION_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESH_H
#include "assetpluginUNREAL_CUnrealStaticMesh.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESH_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESH_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CIUNREALOODLEPROVIDER_H
#include "assetpluginUNREAL_CIUnrealOodleProvider.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CIUNREALOODLEPROVIDER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CIUNREALOODLEPROVIDER_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads a StaticMesh export of an editor package. The tagged properties give the material slots (StaticMaterials), the bounds
 *  and the source models. In UE 5 each source model refers to an object of the class StaticMeshDescriptionBulkData in the same
 *  package; its native data is an editor bulk data record whose payload is the mesh description of the model (compressed with
 *  Oodle in all samples), which CUnrealMeshDescription reads. A source model without a payload (the levels the engine generates
 *  itself) is left out and noted.
 *
 *  In UE 4 (4.25 to 4.27, UE4 file versions 518 to 522) the native data of the mesh itself holds per source model a flag and
 *  the legacy bulk data of its mesh description (compressed with zlib in chunks, CUnrealLegacyPayloadReader), which
 *  CUnrealMeshDescription::ReadUe4Layout() reads.
 *
 *  Not read: the raw mesh form of the engines before 4.25, the collision of the body setup, sockets, the render data of cooked
 *  packages (an editor package has none). A mesh whose source models hold no data has no geometry in the package: that is told
 *  by IsWithoutGeometry(), so that it can be reported as skipped rather than failed.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealStaticMeshReader
{
    public:

    CUnrealStaticMeshReader( void );

    /** @param provider reads the Oodle compressed payloads; may be NULL (such a mesh then fails with IsProviderMissing()) */
    bool Read( CUnrealPackage& package, UInt32 exportIndex, CIUnrealOodleProvider* provider, const ASSETCONVERTER::CConversionReport* cancellation, CUnrealStaticMesh& out );

    const CORE::CString& GetError( void ) const;

    /** The mesh could not be read because the oodle tool or the zlib codec is not there */
    bool IsProviderMissing( void ) const;

    /** The mesh was not read because the package holds no geometry for it: no source model has data (stripped or empty) */
    bool IsWithoutGeometry( void ) const;

    bool IsCancelled( void ) const;

    Int32 GetToolCode( void ) const;

    /** What the reader noticed on the way: levels left out, UV sets over four */
    const CORE::CString::StringVector& GetNotes( void ) const;

    /** Builds the levels of a mesh description (Vertices, VertexInstances, Triangles, PolygonGroups, UVs); exposed for tests */
    static bool BuildLod( const CUnrealMeshDescription& description, const CUnrealSkeletalMesh::TMaterialSlotVector& slots, CUnrealStaticMesh::SLod& lod, CORE::CString& error );

    private:
    bool Fail( const CORE::CString& reason );

    /** The source models of a UE 4 mesh, from the native data after the properties at the position of the reader */
    bool ReadUe4SourceModels( CUnrealPackage& package, CUnrealArchiveReader& reader, const CUnrealSkeletalMesh::TMaterialSlotVector& slots, UInt32 modelCount, CUnrealStaticMesh& out );

    private:
    CORE::CString m_error;
    bool m_providerMissing;
    bool m_withoutGeometry;
    bool m_cancelled;
    Int32 m_toolCode;
    CORE::CString::StringVector m_notes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHREADER_H ? */
