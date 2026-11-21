/*
 *  assetpluginUNREAL: Follows the parent chain of a material instance to its master material and collects the parameters
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALINSTANCEREADER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALINSTANCEREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H
#include "assetpluginUNREAL_CUnrealObjectPathResolver.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALRESOLVEDMATERIAL_H
#include "assetpluginUNREAL_CUnrealResolvedMaterial.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALRESOLVEDMATERIAL_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALRESOLVEDMATERIAL_H ? */

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
 *  Reads a material instance (class MaterialInstanceConstant) or a master material (class Material) and follows the
 *  parents: the instance values come first, a parent only supplies what is not set yet, and the master material supplies
 *  the defaults of its parameter expressions (scalar, vector, texture sample and static switch parameters) and its base
 *  properties. Parents that are not in the pack (engine materials) end the chain with a problem in the result; chains
 *  are limited in depth and cannot loop.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealMaterialInstanceReader
{
    public:

    static const UInt32 MaxChainDepth = 16;

    /** @param materialPath the VFS path of the package of the material; false when that package cannot be read at all */
    static bool Resolve( const CORE::CString& materialPath, const CUnrealObjectPathResolver& resolver, CUnrealResolvedMaterial& out );

    private:

    CUnrealMaterialInstanceReader( void );  /**< only static members */
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALINSTANCEREADER_H ? */
