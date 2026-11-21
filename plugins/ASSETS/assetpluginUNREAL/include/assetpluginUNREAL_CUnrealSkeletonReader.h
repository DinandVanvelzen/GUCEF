/*
 *  assetpluginUNREAL: Reads the reference skeleton and the sockets of a Skeleton asset
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETONREADER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETONREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads an export of the class Skeleton: after the tagged properties the object guid flag and the reference skeleton
 *  (the same layout as in a skeletal mesh) follow. The skeleton asset is the shared skeleton of the meshes and animations
 *  that use it; the reference skeleton of a skeletal mesh stays authoritative for the skinning of that mesh. The sockets
 *  and the number of virtual bones declared in the properties are counted, they are not read further at this step.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealSkeletonReader
{
    public:

    CUnrealSkeletonReader( void );

    bool Read( CUnrealPackage& package, UInt32 exportIndex, CUnrealReferenceSkeleton& skeleton );

    const CORE::CString& GetError( void ) const;

    UInt32 GetSocketCount( void ) const;

    UInt32 GetVirtualBoneCount( void ) const;

    private:

    CUnrealSkeletonReader( const CUnrealSkeletonReader& src );            /**< not implemented */
    CUnrealSkeletonReader& operator=( const CUnrealSkeletonReader& src ); /**< not implemented */

    CORE::CString m_error;
    UInt32 m_socketCount;
    UInt32 m_virtualBoneCount;
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETONREADER_H ? */
