/*
 *  AssetConverterLib: a mesh of the intermediate model
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

#ifndef ASSETCONVERTERLIB_CASSETMESH_H
#define ASSETCONVERTERLIB_CASSETMESH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETSUBMESH_H
#include "AssetConverterLib_CAssetSubMesh.h"
#define ASSETCONVERTERLIB_CASSETSUBMESH_H
#endif /* ASSETCONVERTERLIB_CASSETSUBMESH_H ? */


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A mesh: a name and its submeshes, one per material */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetMesh
{
    public:

    typedef GUCEF::vector< CAssetSubMesh > TSubMeshVector;

    CORE::CString name;
    TSubMeshVector subMeshes;

    CAssetMesh( void );

    UInt32 GetVertexCount( void ) const;
    UInt32 GetTriangleCount( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETMESH_H ? */
