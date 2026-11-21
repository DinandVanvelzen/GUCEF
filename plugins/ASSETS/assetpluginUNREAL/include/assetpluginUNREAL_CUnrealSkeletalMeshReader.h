/*
 *  assetpluginUNREAL: Reads the native data of a skeletal mesh export: material slots, reference skeleton and editor LOD models
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHREADER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESH_H
#include "assetpluginUNREAL_CUnrealSkeletalMesh.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESH_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESH_H ? */

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
 *  Reads a skeletal mesh export. The tagged properties give the LOD infos (screen sizes), the skeleton asset and the
 *  counts of morph targets and sockets. The native data that follows them is read in the order: object guid flag, strip
 *  flags, imported bounds, material slots, reference skeleton and the editor LOD models (sections with their vertices, the
 *  indices of the LOD, the bone lists). The editor models hold everything inline and uncompressed; the engine releases
 *  differ in a few fields (the texture streaming data of the material slots, the ray tracing flag of sections, the precision of bone weights,
 *  the strip flags of the model), which the reader finds by trying the combinations and checking each against the
 *  structure of the data (counts that must agree, the bone tree). Anything not understood is a clean error, never a crash.
 *  Meshes that hold cloth mapping data, imported mesh infos or skin weight profiles are reported as not supported.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealSkeletalMeshReader
{
    public:

    CUnrealSkeletalMeshReader( void );

    /** @param exportIndex the export of the class SkeletalMesh */
    bool Read( CUnrealPackage& package, UInt32 exportIndex, CUnrealSkeletalMesh& mesh );

    /** The reason of the last failed Read() */
    const CORE::CString& GetError( void ) const;

    /** What the last Read() noted: the layout variant that matched, features that were left out */
    const CORE::CString::StringVector& GetNotes( void ) const;

    private:

    struct SVariant
    {
        UInt32 materialSlotWords;   /**< 32 bit words of texture streaming data behind the imported name of each material slot */
        bool sectionRayTracingFlag; /**< sections and their source user data have the visible in ray tracing flag */
        bool weights16;             /**< bone weights are 16 bit, else 8 bit */
        bool modelStripFlags;       /**< strip flags in front of the LOD model array */
    };

    bool ReadHeader( CUnrealPackage& package, CUnrealArchiveReader& reader, const SVariant& variant, CUnrealSkeletalMesh& mesh );
    bool ReadLods( CUnrealArchiveReader& reader, const SVariant& variant, CUnrealSkeletalMesh& mesh );
    bool ReadLod( CUnrealArchiveReader& reader, const SVariant& variant, CUnrealSkeletalMesh::SLod& lod );
    bool ReadSection( CUnrealArchiveReader& reader, const SVariant& variant, CUnrealSkeletalMesh::SSection& section );
    void ReadProperties( const CORE::CDataNode& root, CUnrealSkeletalMesh& mesh );
    bool Fail( const CUnrealArchiveReader& reader, UInt32 line );

    CUnrealSkeletalMeshReader( const CUnrealSkeletalMeshReader& src );            /**< not implemented */
    CUnrealSkeletalMeshReader& operator=( const CUnrealSkeletalMeshReader& src ); /**< not implemented */

    const CUnrealPackage* m_package;
    UInt64 m_bestPosition;
    UInt32 m_bestLine;
    CORE::CString m_error;
    CORE::CString::StringVector m_notes;
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHREADER_H ? */
