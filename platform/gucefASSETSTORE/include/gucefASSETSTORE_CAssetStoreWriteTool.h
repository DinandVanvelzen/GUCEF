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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREWRITETOOL_H
#define GUCEF_ASSETSTORE_CASSETSTOREWRITETOOL_H

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
 *  The write tools of the primary store (group "store", section 7.5): store_save_search (SaveSearch()),
 *  store_delete_search (DeleteSavedSearch()), store_classify (SetClassification() per item, source "llm-assisted"),
 *  store_edit_rules (SetCategoryRule(), RemoveCategoryRule(), SetCategoryMapping(), RemoveCategoryMapping(), with
 *  CountRuleMatches() in the preview) and store_reclassify (Reclassify()). Every write takes one of the hourly quota
 *  of the principal and its requestKey is the command id of the service. store_classify and store_edit_rules preview
 *  first: the preview lists the current and the new values and gives a single-use change token kept by the tool set
 *  (bound to the principal and the arguments); the apply needs it (and the confirmation of confirmWrites). What the
 *  service cannot do (the in-process service has no SetClassification() yet) is answered unsupported per item.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreWriteTool : public CAssetToolBase
{
    public:

    CAssetStoreWriteTool( CAssetToolContext& context, const CORE::CString& name );

    virtual ~CAssetStoreWriteTool() GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual COM::EMcpExecuteResult Compute( COM::CMcpRequestContext& context      ,
                                            const CORE::CDataNode& arguments      ,
                                            COM::CMcpToolResult& result           ,
                                            COM::CMcpInputRequired& inputRequired ,
                                            SCallState& state                     ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    COM::EMcpExecuteResult Preview( const COM::CMcpPrincipal& principal, const CORE::CDataNode& arguments, COM::CMcpToolResult& result );
    void ApplyClassify( const CORE::CString& commandId, const CORE::CDataNode& payload, CORE::CDataNode& document );
    void ApplyRules( const CORE::CString& commandId, const CORE::CDataNode& payload, CORE::CDataNode& document );

    /** The digest that binds a preview to its apply: the arguments without apply, changeToken, requestKey */
    static CORE::CString DigestOfChange( const CORE::CDataNode& arguments );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTOREWRITETOOL_H ? */
