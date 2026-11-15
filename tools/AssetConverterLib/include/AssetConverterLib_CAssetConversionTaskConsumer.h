/*
 *  AssetConverterLib: the task that converts one asset on a thread pool
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

#ifndef ASSETCONVERTERLIB_CASSETCONVERSIONTASKCONSUMER_H
#define ASSETCONVERTERLIB_CASSETCONVERSIONTASKCONSUMER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

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

/**
 *  The task consumer of CAssetPipeline::ConvertAssetAsync(): runs CAssetPipeline::ConvertAsset() on its CAssetConversionJob,
 *  on a thread of the pool, and leaves the job with its results as the task data for the future.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetConversionTaskConsumer : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;    /**< "AssetConversion" */

    CAssetConversionTaskConsumer( void );

    virtual ~CAssetConversionTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    /** The job stays: it has the results for the future */
    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAssetConversionTaskConsumer( const CAssetConversionTaskConsumer& src );            /**< not implemented */
    CAssetConversionTaskConsumer& operator=( const CAssetConversionTaskConsumer& src ); /**< not implemented */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CORE::CTaskConsumer, CAssetConversionTaskConsumer, MT::CMutex > TAssetConversionTaskConsumerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETCONVERSIONTASKCONSUMER_H ? */
