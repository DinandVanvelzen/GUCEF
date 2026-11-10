/*
 *  gucefIMAGE: GUCEF module providing image utilities
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

#ifndef GUCEF_IMAGE_CASYNCIMAGEUPSCALE_H
#define GUCEF_IMAGE_CASYNCIMAGEUPSCALE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What CImageUpscaler::UpscaleAsync() asks a thread of a pool to do, and what came of it */
class GUCEF_HIDDEN CImageUpscaleTaskData : public CORE::CICloneable
{
    public:

    CORE::CString upscalerName;     /**< the upscaler, looked up in the registry of CImageGlobal when the task runs */
    UInt32 factor;
    TPixelMapPtr source;            /**< shared with the caller, not copied */
    TPixelMapPtr result;            /**< set when the upscale succeeded */
    bool success;

    CImageUpscaleTaskData( void );

    CImageUpscaleTaskData( const CImageUpscaleTaskData& src );

    virtual ~CImageUpscaleTaskData();

    CImageUpscaleTaskData& operator=( const CImageUpscaleTaskData& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** The task that runs CImageUpscaler::Upscale() for CImageUpscaler::UpscaleAsync() */
class GUCEF_HIDDEN CAsyncImageUpscale : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    CAsyncImageUpscale( void );

    virtual ~CAsyncImageUpscale() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    /** The task data stays: it has the result for CImageUpscaler::GetAsyncResult() */
    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAsyncImageUpscale( const CAsyncImageUpscale& src );
    CAsyncImageUpscale& operator=( const CAsyncImageUpscale& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CORE::CTaskConsumer, CAsyncImageUpscale, MT::CMutex > TAsyncImageUpscaleTaskFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CASYNCIMAGEUPSCALE_H ? */
