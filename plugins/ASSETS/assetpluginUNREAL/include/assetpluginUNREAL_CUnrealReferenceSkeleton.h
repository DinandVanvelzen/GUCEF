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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALREFERENCESKELETON_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALREFERENCESKELETON_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H
#include "assetpluginUNREAL_CUnrealArchiveReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALNAMETABLE_H
#include "assetpluginUNREAL_CUnrealNameTable.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALNAMETABLE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALNAMETABLE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H
#include "assetpluginUNREAL_CUnrealVersionContext.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H ? */

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
 *  The bones of a skeleton in parent first order: name, parent index (-1 for the root) and the local bind pose as
 *  rotation (quaternion x, y, z, w), translation and scale in the units and axes of the engine (centimetres, Z up,
 *  left handed). Read from the native data of a skeletal mesh or skeleton: the bone infos, the poses, and the table of
 *  bone names to indices.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealReferenceSkeleton
{
    public:

    struct SBone
    {
        CORE::CString name;
        Int32 parent;
        Float64 rotation[ 4 ];
        Float64 translation[ 3 ];
        Float64 scale[ 3 ];
    };
    typedef GUCEF::vector< SBone > TBoneVector;

    CUnrealReferenceSkeleton( void );

    void Clear( void );

    /**
     *  @param hasExportNames the bone infos carry the editor only export name string (the package does not filter editor data)
     *  @return false when the data is damaged or the tree is not valid: parents must come before their children
     */
    bool Read( CUnrealArchiveReader& reader, const CUnrealNameTable& names, const CUnrealVersionContext& versions, bool hasExportNames );

    const TBoneVector& GetBones( void ) const;

    /** Replaces the bones: for data that does not come from a package (tests, tools); the order must be parent first */
    void SetBones( const TBoneVector& bones );

    /** The index of the bone with the name, or -1 */
    Int32 FindBone( const CORE::CString& name ) const;

    private:

    TBoneVector m_bones;
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALREFERENCESKELETON_H ? */
