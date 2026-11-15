/*
 *  AssetConverterLib: the interface of the writers of the intermediate
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

#ifndef ASSETCONVERTERLIB_CIASSETEXPORTER_H
#define ASSETCONVERTERLIB_CIASSETEXPORTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTGLOBALONREGISTRY_H
#include "gucefCORE_CTGlobalONRegistry.h"
#define GUCEF_CORE_CTGLOBALONREGISTRY_H
#endif /* GUCEF_CORE_CTGLOBALONREGISTRY_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */


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

class CAssetScene;
class CConversionReport;

/*-------------------------------------------------------------------------*/

/**
 *  An exporter: writes a scene of the model as the intermediate, the glTF exporter being the one the pipeline uses by
 *  default ("gltf"). It writes the asset into the VFS folder it gets, named after CAssetScene::name, and reports every
 *  file it wrote (CConversionReport::AddOutputFile()) with the files that file references, such as the baked textures
 *  (CAssetTexture::bakedPath). The sidecar is written by the pipeline. Called from the tasks of the pipeline, several
 *  at the same time. GetVersion() is part of the cache key.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CIAssetExporter
{
    public:

    CIAssetExporter( void );

    virtual ~CIAssetExporter();

    virtual CORE::CString GetName( void ) const = 0;

    virtual UInt32 GetVersion( void ) const = 0;

    virtual bool Export( const CAssetScene& scene, const CORE::CString& vfsDir, CConversionReport& report ) = 0;

    private:

    CIAssetExporter( const CIAssetExporter& src );            /**< not implemented */
    CIAssetExporter& operator=( const CIAssetExporter& src ); /**< not implemented */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CIAssetExporter, MT::CMutex > TAssetExporterPtr;
typedef CORE::CTGlobalONRegistry< CIAssetExporter, MT::CMutex > TAssetExporterRegistry;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CIASSETEXPORTER_H ? */
