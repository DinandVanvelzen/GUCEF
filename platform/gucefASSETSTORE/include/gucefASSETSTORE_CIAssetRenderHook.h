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

#ifndef GUCEF_ASSETSTORE_CIASSETRENDERHOOK_H
#define GUCEF_ASSETSTORE_CIASSETRENDERHOOK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#include "gucefASSETSTORE_CAssetPipelineRequest.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H
#include "gucefASSETSTORE_CAssetServiceDocument.h"
#define GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The render path of the asset store service: contact sheets and renders of assets for the tools of the service
 *  (CIAssetStoreService::RenderSheet(), operation CAssetPipelineOperations::RenderSheet). The store does not render: an
 *  application with a render system (the viewer, a render worker) implements the hook and gives it to the service. The
 *  request names the targets (request.arguments "targets": objects with packId, assetPath, stage and profile, or
 *  storeAssetId) and the tile size; the result has a document per target and the pictures in result.images.
 *
 *  Render() runs on the calling thread and blocks until the pictures are made; an implementation that has to render on
 *  another thread (the thread of its render system) hands the work over and waits for it, without pumping a pulse
 *  generator that is not its own.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CIAssetRenderHook
{
    public:

    CIAssetRenderHook( void );
    virtual ~CIAssetRenderHook();

    virtual CORE::CString GetName( void ) const = 0;

    virtual TAssetStoreStatus Render( const CAssetPipelineRequest& request, CAssetServiceDocument& result ) = 0;

    private:

    CIAssetRenderHook( const CIAssetRenderHook& src );              /**< not implemented */
    CIAssetRenderHook& operator=( const CIAssetRenderHook& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CIASSETRENDERHOOK_H ? */
