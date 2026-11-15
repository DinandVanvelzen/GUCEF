/*
 *  AssetConverterLib: the built-in post-processor that removes nodes that hold nothing
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

#ifndef ASSETCONVERTERLIB_CPRUNEEMPTYNODESPOSTPROCESSOR_H
#define ASSETCONVERTERLIB_CPRUNEEMPTYNODESPOSTPROCESSOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CIASSETPOSTPROCESSOR_H
#include "AssetConverterLib_CIAssetPostProcessor.h"
#define ASSETCONVERTERLIB_CIASSETPOSTPROCESSOR_H
#endif /* ASSETCONVERTERLIB_CIASSETPOSTPROCESSOR_H ? */


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

/**
 *  "PruneEmptyNodes": removes nodes that hold nothing and have no children, over and over until none is left. A node
 *  stays when it has a mesh, skin, light, particle system, LOD group, collider, prefab instance or extras, or when it is a joint, the
 *  target of an animation, a LOD level or a prefab node.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPruneEmptyNodesPostProcessor : public CIAssetPostProcessor
{
    public:

    static const CORE::CString TypeName;    /**< "PruneEmptyNodes" */

    CPruneEmptyNodesPostProcessor( void );

    virtual ~CPruneEmptyNodesPostProcessor() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Process( CAssetScene& scene, const CORE::CDataNode& settings, CConversionReport& report ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPRUNEEMPTYNODESPOSTPROCESSOR_H ? */
