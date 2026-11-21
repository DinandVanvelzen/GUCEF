/*
 *  assetpluginUNREAL: Decoders for the structs that are stored in binary form inside tagged properties
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALNATIVESTRUCTS_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALNATIVESTRUCTS_H

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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSOFTOBJECTPATH_H
#include "assetpluginUNREAL_CUnrealSoftObjectPath.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSOFTOBJECTPATH_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSOFTOBJECTPATH_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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
 *  Structs with a native binary serialisation: the vector family, rotator, quaternion, colours, integer points and
 *  vectors, box, plane, matrix, transform, GUID, date and time, frame number and soft object paths. Vector types are
 *  stored as floats before UE5 file version 1004 and as doubles from then on, except the types that name their width
 *  (Vector3f, Vector3d, ...) and LinearColor, which is always float. In the result tree a struct is a node whose
 *  attributes hold the components (x, y, z, w, r, g, b, a, ...) and whose children hold sub structs.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealNativeStructs
{
    public:

    /** Whether a struct of this name is binary in the classic tag layout, and decoded here */
    static bool IsNative( const CORE::CString& structName );

    /** The size in bytes of one struct of this name with the versions, 0 when its size is not fixed or the struct is not known */
    static UInt32 GetFixedSize( const CORE::CString& structName, const CUnrealVersionContext& versions );

    /**
     *  Reads one struct. declaredSize is the size from the tag when the struct is the whole value of a property and
     *  -1 inside a container; with a size the width of the vector family is taken from it when the version is not
     *  decisive. On false the reader is in the error state.
     */
    static bool Read( CUnrealArchiveReader& reader, const CORE::CString& structName, const CUnrealNameTable& names, const CUnrealVersionContext& versions, const CUnrealSoftObjectPath::TSoftObjectPathVector* softPaths, Int64 declaredSize, CORE::CDataNode& node );
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALNATIVESTRUCTS_H ? */
