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

#ifndef GUCEF_ASSETSTORE_CASSETRENDERSHEETTOOL_H
#define GUCEF_ASSETSTORE_CASSETRENDERSHEETTOOL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETTOOLBASE_H
#include "gucefASSETSTORE_CAssetToolBase.h"
#define GUCEF_ASSETSTORE_CASSETTOOLBASE_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLBASE_H ? */

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
 *  render_sheet (section 7.3), and the renders of compare_routes and material_map_suggest: every target passes the
 *  content gate on its own BEFORE anything is asked of the service, so nothing of a withheld pack is even rendered;
 *  the targets that may be shown go to CIAssetStoreService::RenderSheet() (the render hook of the service). A service
 *  without a renderer answers UNSUPPORTED, which the result says per target ("supported": false). The pictures that
 *  come back are PNG image content, each tagged with the pack of its target for the final filter.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetRenderSheetTool : public CAssetToolBase
{
    public:

    static const UInt32 MaxImageBytes = 1048576;

    /** A target of a render */
    struct GUCEF_ASSETSTORE_PUBLIC_CPP STarget
    {
        CORE::CString label;
        CORE::CString packId;
        CORE::CString assetPath;
        CORE::CString stage;
        CORE::CString profile;
        CORE::CString storeAssetId;

        STarget( void );
    };
    typedef GUCEF::vector< STarget > TTargetVector;

    CAssetRenderSheetTool( CAssetToolContext& context, const CORE::CString& name );

    virtual ~CAssetRenderSheetTool() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Renders the targets: 'outcomes' gets an array entry per target ({ withheld, reason } or { failed, reason } or
     *  { sheet: n }), the pictures go into the result with their tags; 'supported' false when the service has no
     *  renderer
     */
    static void RenderTargets( CAssetToolContext& context                  ,
                               const TTargetVector& targets                ,
                               UInt32 tile                                 ,
                               COM::CMcpToolResult& result                 ,
                               CAssetToolContext::TContentTagVector& tags  ,
                               CORE::CDataNode& outcomes                   ,
                               bool& supported                             );

    protected:

    virtual COM::EMcpExecuteResult Compute( COM::CMcpRequestContext& context      ,
                                            const CORE::CDataNode& arguments      ,
                                            COM::CMcpToolResult& result           ,
                                            COM::CMcpInputRequired& inputRequired ,
                                            SCallState& state                     ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETRENDERSHEETTOOL_H ? */
