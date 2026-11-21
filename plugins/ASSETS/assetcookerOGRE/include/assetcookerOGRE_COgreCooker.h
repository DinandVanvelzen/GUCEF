/*
 *  assetcookerOGRE: GUCEF asset converter plugin: the cooker of the intermediate into Ogre 1.x runtime files
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

#ifndef GUCEF_ASSETCOOKER_OGRE_COGRECOOKER_H
#define GUCEF_ASSETCOOKER_OGRE_COGRECOOKER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CIASSETCOOKER_H
#include "AssetConverterLib_CIAssetCooker.h"
#define ASSETCONVERTERLIB_CIASSETCOOKER_H
#endif /* ASSETCONVERTERLIB_CIASSETCOOKER_H ? */

#ifndef GUCEF_ASSETCOOKER_OGRE_COGRECOOKENVIRONMENT_H
#include "assetcookerOGRE_COgreCookEnvironment.h"
#define GUCEF_ASSETCOOKER_OGRE_COGRECOOKENVIRONMENT_H
#endif /* GUCEF_ASSETCOOKER_OGRE_COGRECOOKENVIRONMENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCOOKER {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The cooker "ogre": the intermediate into the runtime files of Ogre 1.x (14.6.3).
 *
 *  Cook() reads every asset it is given back from its glTF intermediate with the glTF importer of the registry (so the
 *  mapping stays the converter's, not Codec_Assimp's) and its sidecar, and cooks it with CAssetCook into the folder of
 *  the cook profile. The textures are written once per call (CTextureCooker). Ogre runs headless (COgreCookEnvironment)
 *  on the calling thread. Problems are reported per asset; Cook() fails only when it cannot start (no Ogre, no glTF
 *  importer, an intermediate folder that cannot be read).
 *
 *  GetVersion() is part of the cache key: raise it whenever the files written for the same intermediate change.
 *  GetMaxBonesPerSubMesh() is 0 (no limit): the vertices of a submesh index their bones with 8 bits in the v1 format, so
 *  the mesh cooker splits a submesh that uses more than 256 bones by bone subsets (CBoneSubsetSplitter).
 */
class GUCEF_ASSETCOOKER_OGRE_EXPORT_CPP COgreCooker : public ASSETCONVERTER::CIAssetCooker
{
    public:

    static const CORE::CString TargetName;      /**< "ogre" */
    static const CORE::CString CookedAreaName;  /**< "gucef-ogre3d": the folder of the asset store that holds the output of this cooker, accepted as an alias of the target name */
    static const UInt32 Version;

    COgreCooker( void );

    virtual ~COgreCooker() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetTargetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetMaxBonesPerSubMesh( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Cook( const CORE::CString& intermediateDir                 ,
                       const CORE::CString& outDir                          ,
                       const ASSETCONVERTER::CCookSettings& settings        ,
                       ASSETCONVERTER::CConversionReport& report            ) GUCEF_VIRTUAL_OVERRIDE;

    /** Takes down what the cooker made of Ogre, before the plugin unloads */
    void Release( void );

    private:

    COgreCookEnvironment m_environment;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETCOOKER_OGRE_COGRECOOKER_H ? */
