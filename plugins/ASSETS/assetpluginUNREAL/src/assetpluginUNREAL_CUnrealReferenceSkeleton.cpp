/*
 *  assetpluginUNREAL: The reference skeleton of a skeletal mesh or skeleton: bones, parents and the bind pose
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALREFERENCESKELETON_H
#include "assetpluginUNREAL_CUnrealReferenceSkeleton.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALREFERENCESKELETON_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALREFERENCESKELETON_H ? */

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

CUnrealReferenceSkeleton::CUnrealReferenceSkeleton( void )
    : m_bones()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CUnrealReferenceSkeleton::Clear( void )
{GUCEF_TRACE;

    m_bones.clear();
}

/*-------------------------------------------------------------------------*/

bool
CUnrealReferenceSkeleton::Read( CUnrealArchiveReader& reader, const CUnrealNameTable& names, const CUnrealVersionContext& versions, bool hasExportNames )
{GUCEF_TRACE;

    Clear();

    // the bone infos: name, parent and in editor packages the export name
    const Int32 boneCount = reader.ReadInt32();
    if ( reader.HasError() || boneCount < 0 || !reader.CheckCount( (UInt64) boneCount, hasExportNames ? 16 : 12, "reference skeleton bones" ) )
        return false;
    m_bones.resize( (size_t) boneCount );
    for ( Int32 i=0; i<boneCount; ++i )
    {
        SBone& bone = m_bones[ (size_t) i ];
        if ( !names.ReadName( reader, bone.name ) )
            return false;
        bone.parent = reader.ReadInt32();
        if ( hasExportNames )
        {
            CORE::CString exportName;
            if ( !reader.ReadString( exportName ) )
                return false;
        }
        if ( reader.HasError() )
            return false;
        if ( bone.parent >= i || bone.parent < -1 || ( 0 == i && bone.parent != -1 ) )
            return reader.SetError( "Bone " + bone.name + " has the parent " + CORE::Int32ToString( bone.parent ) + ": the bones are not in parent first order" );
    }

    // the local bind poses, doubles from the large world coordinates versions on
    const bool doubles = versions.IsAtLeast( UE5_LARGE_WORLD_COORDINATES );
    const Int32 poseCount = reader.ReadInt32();
    if ( reader.HasError() || poseCount != boneCount || !reader.CheckCount( (UInt64) poseCount, doubles ? 80 : 40, "reference skeleton poses" ) )
        return reader.HasError() ? false : reader.SetError( "The reference skeleton has " + CORE::Int32ToString( poseCount ) + " poses for " + CORE::Int32ToString( boneCount ) + " bones" );
    for ( Int32 i=0; i<poseCount; ++i )
    {
        SBone& bone = m_bones[ (size_t) i ];
        for ( UInt32 c=0; c<4; ++c )
            bone.rotation[ c ] = doubles ? reader.ReadFloat64() : (Float64) reader.ReadFloat32();
        for ( UInt32 c=0; c<3; ++c )
            bone.translation[ c ] = doubles ? reader.ReadFloat64() : (Float64) reader.ReadFloat32();
        for ( UInt32 c=0; c<3; ++c )
            bone.scale[ c ] = doubles ? reader.ReadFloat64() : (Float64) reader.ReadFloat32();
    }

    // the table of names to indices: read to stay in step, the order of the bones is authoritative
    const Int32 mapCount = reader.ReadInt32();
    if ( reader.HasError() || mapCount < 0 || !reader.CheckCount( (UInt64) mapCount, 12, "reference skeleton name map" ) )
        return false;
    for ( Int32 i=0; i<mapCount; ++i )
    {
        CORE::CString name;
        if ( !names.ReadName( reader, name ) )
            return false;
        reader.ReadInt32();
    }
    return !reader.HasError();
}

/*-------------------------------------------------------------------------*/

const CUnrealReferenceSkeleton::TBoneVector&
CUnrealReferenceSkeleton::GetBones( void ) const
{GUCEF_TRACE;

    return m_bones;
}

/*-------------------------------------------------------------------------*/

void
CUnrealReferenceSkeleton::SetBones( const TBoneVector& bones )
{GUCEF_TRACE;

    m_bones = bones;
}

/*-------------------------------------------------------------------------*/

Int32
CUnrealReferenceSkeleton::FindBone( const CORE::CString& name ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_bones.size(); ++i )
        if ( m_bones[ i ].name == name )
            return (Int32) i;
    return -1;
}

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
