/*
 *  AssetConverterLib: the interface of the processors that run on an imported asset
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

#ifndef ASSETCONVERTERLIB_CIASSETPOSTPROCESSOR_H
#define ASSETCONVERTERLIB_CIASSETPOSTPROCESSOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTGLOBALONREGISTRY_H
#include "gucefCORE_CTGlobalONRegistry.h"
#define GUCEF_CORE_CTGLOBALONREGISTRY_H
#endif /* GUCEF_CORE_CTGLOBALONREGISTRY_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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
 *  A post-processor: changes an imported asset after the import and the normalisation and before the export, so its
 *  result is part of the intermediate. The manifest names the stack per source format ("postProcessors"); without an
 *  entry the default stack of the importer runs. A processor runs only on the source formats AppliesTo() accepts.
 *
 *  'settings' are its own: the settings of its entry in the stack, with the child of the effective settings of the
 *  asset that has its name over them (so presets and overrides can change them per asset). The whole effective
 *  settings are in CAssetScene::settings. Called from the tasks of the pipeline, several at the same time.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CIAssetPostProcessor
{
    public:

    CIAssetPostProcessor( void );

    virtual ~CIAssetPostProcessor();

    virtual CORE::CString GetName( void ) const = 0;

    virtual UInt32 GetVersion( void ) const = 0;

    /** Whether it runs on assets of the source format (CAssetProvenance::sourceFormat); the default: every format */
    virtual bool AppliesTo( const CORE::CString& sourceFormat ) const;

    virtual bool Process( CAssetScene& scene, const CORE::CDataNode& settings, CConversionReport& report ) = 0;

    private:

    CIAssetPostProcessor( const CIAssetPostProcessor& src );            /**< not implemented */
    CIAssetPostProcessor& operator=( const CIAssetPostProcessor& src ); /**< not implemented */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CIAssetPostProcessor, MT::CMutex > TAssetPostProcessorPtr;
typedef CORE::CTGlobalONRegistry< CIAssetPostProcessor, MT::CMutex > TAssetPostProcessorRegistry;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CIASSETPOSTPROCESSOR_H ? */
