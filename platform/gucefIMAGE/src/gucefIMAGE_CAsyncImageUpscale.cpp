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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H
#include "gucefIMAGE_CImageGlobal.h"
#define GUCEF_IMAGE_CIMAGEGLOBAL_H
#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */

#ifndef GUCEF_IMAGE_CIMAGEUPSCALERREGISTRY_H
#include "gucefIMAGE_CImageUpscalerRegistry.h"
#define GUCEF_IMAGE_CIMAGEUPSCALERREGISTRY_H
#endif /* GUCEF_IMAGE_CIMAGEUPSCALERREGISTRY_H ? */

#ifndef GUCEF_IMAGE_CIMAGEUPSCALER_H
#include "gucefIMAGE_CImageUpscaler.h"
#define GUCEF_IMAGE_CIMAGEUPSCALER_H
#endif /* GUCEF_IMAGE_CIMAGEUPSCALER_H ? */

#include "gucefIMAGE_CAsyncImageUpscale.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAsyncImageUpscale::TaskType = "AsyncImageUpscale";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Stops the upscale when the task is asked to cancel */
class CTaskCancellationProgress : public CIUpscaleProgress
{
    public:

    explicit CTaskCancellationProgress( CORE::CTaskPtr task )
        : m_task( task )
    {GUCEF_TRACE;

    }

    virtual bool OnUpscaleProgress( Float32 fraction ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return !m_task->IsCancellationRequested();
    }

    private:

    CORE::CTaskPtr m_task;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImageUpscaleTaskData::CImageUpscaleTaskData( void )
    : CORE::CICloneable()
    , upscalerName()
    , factor( 0 )
    , source()
    , result()
    , success( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageUpscaleTaskData::CImageUpscaleTaskData( const CImageUpscaleTaskData& src )
    : CORE::CICloneable( src )
    , upscalerName( src.upscalerName )
    , factor( src.factor )
    , source( src.source )
    , result( src.result )
    , success( src.success )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageUpscaleTaskData::~CImageUpscaleTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageUpscaleTaskData&
CImageUpscaleTaskData::operator=( const CImageUpscaleTaskData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        upscalerName = src.upscalerName;
        factor = src.factor;
        source = src.source;
        result = src.result;
        success = src.success;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CImageUpscaleTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CImageUpscaleTaskData( *this );
}

/*-------------------------------------------------------------------------*/

CAsyncImageUpscale::CAsyncImageUpscale( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAsyncImageUpscale::~CAsyncImageUpscale()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAsyncImageUpscale::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncImageUpscale::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CImageUpscaleTaskData* data = static_cast< CImageUpscaleTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AsyncImageUpscale: No task data to work on" );
        return true;
    }

    data->success = false;
    CImageUpscaler* upscaler = CImageGlobal::Instance()->GetImageUpscalerRegistry().Get( data->upscalerName );
    if ( GUCEF_NULL == upscaler )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AsyncImageUpscale: There is no image upscaler \"" << data->upscalerName << "\"" );
        return true;
    }
    if ( data->source.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AsyncImageUpscale: There is no picture to upscale" );
        return true;
    }

    CTaskCancellationProgress progress( task );
    data->success = upscaler->Upscale( *data->source, data->factor, data->result, &progress );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAsyncImageUpscale::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
