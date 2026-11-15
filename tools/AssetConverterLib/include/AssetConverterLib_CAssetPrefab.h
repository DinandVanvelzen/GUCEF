/*
 *  AssetConverterLib: a prefab of the intermediate model
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

#ifndef ASSETCONVERTERLIB_CASSETPREFAB_H
#define ASSETCONVERTERLIB_CASSETPREFAB_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETPREFABINSTANCE_H
#include "AssetConverterLib_CAssetPrefabInstance.h"
#define ASSETCONVERTERLIB_CASSETPREFABINSTANCE_H
#endif /* ASSETCONVERTERLIB_CASSETPREFABINSTANCE_H ? */


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

/** A prefab: its root node and the prefab instances nested in it */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetPrefab
{
    public:

    typedef GUCEF::vector< CAssetPrefabInstance > TInstanceVector;

    CORE::CString name;
    UInt32 rootNode;
    TInstanceVector instances;

    CAssetPrefab( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETPREFAB_H ? */
