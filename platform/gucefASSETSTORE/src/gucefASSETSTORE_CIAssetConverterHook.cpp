/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#include "gucefASSETSTORE_CIAssetConverterHook.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIAssetConverterHook::CIAssetConverterHook( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIAssetConverterHook::~CIAssetConverterHook()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIAssetConverterHook::SupportsPipelineOperation( const CORE::CString& operation ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CIAssetConverterHook::RunPipelineOperation( const CAssetPipelineRequest& request    ,
                                            CIAssetConversionObserver* observer     ,
                                            CAssetServiceDocument& result           )
{GUCEF_TRACE;

    result.Clear();
    result.status = ASSETSTORE_STATUS_UNSUPPORTED;
    result.message = "The converter " + GetName() + " does not carry out the operation \"" + request.operation + "\"";
    return result.status;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
