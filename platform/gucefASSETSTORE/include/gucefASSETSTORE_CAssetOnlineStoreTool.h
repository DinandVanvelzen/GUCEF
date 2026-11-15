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

#ifndef GUCEF_ASSETSTORE_CASSETONLINESTORETOOL_H
#define GUCEF_ASSETSTORE_CASSETONLINESTORETOOL_H

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
 *  The read-only tools of the online stores and the transfer queue (section 7.6): online_stores (ListStores(), the
 *  sign-in state as a boolean only), online_library (QueryLibrary()), online_item (GetStoreItem(): formats and files,
 *  licence, categories and description as untrusted text) and transfer_queue (ListJobs()). No listing pictures, no
 *  downloads, no sign-in: a store that is not signed in answers not-signed-in, which asks the owner to sign in
 *  through the viewer.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetOnlineStoreTool : public CAssetToolBase
{
    public:

    CAssetOnlineStoreTool( CAssetToolContext& context, const CORE::CString& name );

    virtual ~CAssetOnlineStoreTool() GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual COM::EMcpExecuteResult Compute( COM::CMcpRequestContext& context      ,
                                            const CORE::CDataNode& arguments      ,
                                            COM::CMcpToolResult& result           ,
                                            COM::CMcpInputRequired& inputRequired ,
                                            SCallState& state                     ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    void SetStoreError( TAssetStoreStatus status, const CORE::CString& storeId, COM::CMcpToolResult& result );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETONLINESTORETOOL_H ? */
