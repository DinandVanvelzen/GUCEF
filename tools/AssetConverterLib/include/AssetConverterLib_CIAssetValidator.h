/*
 *  AssetConverterLib: the interface of the checks of an asset before it is cooked
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

#ifndef ASSETCONVERTERLIB_CIASSETVALIDATOR_H
#define ASSETCONVERTERLIB_CIASSETVALIDATOR_H

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

#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */


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
 *  A validator: checks an asset after the post-processors and reports each result as info, warning or error
 *  (CConversionReport::AddItem() with its name as the source). An error stops the cooking of the asset unless the run
 *  is forced. The manifest lists the validators with their settings ("validators"); 'settings' are those of its entry,
 *  with the child of the effective settings of the asset that has its name over them. Most validators let the setting
 *  "severity" choose the kind of their results. Called from the tasks of the pipeline, several at the same time.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CIAssetValidator
{
    public:

    CIAssetValidator( void );

    virtual ~CIAssetValidator();

    virtual CORE::CString GetName( void ) const = 0;

    virtual UInt32 GetVersion( void ) const = 0;

    /** False when the validator could not run; what it finds goes to the report */
    virtual bool Validate( const CAssetScene& scene, const CORE::CDataNode& settings, CConversionReport& report ) = 0;

    /** The severity the settings name ("info", "warning", "error"), or the default */
    static TReportItemKind GetSeverity( const CORE::CDataNode& settings, TReportItemKind defaultSeverity );

    private:

    CIAssetValidator( const CIAssetValidator& src );            /**< not implemented */
    CIAssetValidator& operator=( const CIAssetValidator& src ); /**< not implemented */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CIAssetValidator, MT::CMutex > TAssetValidatorPtr;
typedef CORE::CTGlobalONRegistry< CIAssetValidator, MT::CMutex > TAssetValidatorRegistry;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CIASSETVALIDATOR_H ? */
