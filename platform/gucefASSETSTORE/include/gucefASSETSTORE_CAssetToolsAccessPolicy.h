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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLSACCESSPOLICY_H
#define GUCEF_ASSETSTORE_CASSETTOOLSACCESSPOLICY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CIMCPACCESSPOLICY_H
#include "gucefCOM_CIMcpAccessPolicy.h"
#define GUCEF_COM_CIMCPACCESSPOLICY_H
#endif /* GUCEF_COM_CIMCPACCESSPOLICY_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLCONTEXT_H
#include "gucefASSETSTORE_CAssetToolContext.h"
#define GUCEF_ASSETSTORE_CASSETTOOLCONTEXT_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLCONTEXT_H ? */

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
 *  The access policy of the asset tools (section 11.4 of the asset tools design), on the exposure in force:
 *
 *  - mcpEnabled false hides every tool, resource and prompt;
 *  - the tools of the group "read" are visible; a tool of the groups convert, author or store only when the exposure
 *    enables its group or names it in allowedWriteTools; deniedTools hides a tool whatever the rest says; a tool that
 *    is not of the catalogue (contributed) is visible when it is read-only, or when allowedWriteTools names it;
 *  - job_cancel is visible with convert or store, and its call is allowed only when the group of the job's kind is
 *    enabled (a conversion with convert, an ingest with store);
 *  - content resources (store/assets/{id}/thumbnail, sheets/...) of a pack whose licence withholds content are denied
 *    with -31003 and the reason, not hidden, so the model can tell the owner why; metadata resources are allowed.
 *
 *  Visibility does not depend on the principal (lists stay cacheable for a single-user stdio server).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetToolsAccessPolicy : public COM::CIMcpAccessPolicy
{
    public:

    explicit CAssetToolsAccessPolicy( CAssetToolContext& context );

    virtual ~CAssetToolsAccessPolicy() GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the tool of the name with the effect is visible under the exposure */
    static bool IsToolVisible( const CAssetToolsExposure& exposure, const CORE::CString& name, COM::EMcpToolEffect effect );

    virtual bool IsVisibilityPrincipalDependent( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsToolVisible( const COM::CMcpPrincipal& principal, const COM::CMcpToolEntry& tool ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual COM::EMcpAccessDecision AuthorizeToolCall( const COM::CMcpRequestContext& context ,
                                                       const COM::CMcpToolEntry& tool          ,
                                                       const CORE::CDataNode& arguments        ,
                                                       CORE::CString& reason                   ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsResourceVisible( const COM::CMcpPrincipal& principal, const CORE::CString& uri ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual COM::EMcpAccessDecision AuthorizeResourceRead( const COM::CMcpRequestContext& context, const CORE::CString& uri, CORE::CString& reason ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsPromptVisible( const COM::CMcpPrincipal& principal, const COM::CMcpPromptEntry& prompt ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual COM::EMcpAccessDecision AuthorizePromptGet( const COM::CMcpRequestContext& context, const COM::CMcpPromptEntry& prompt, CORE::CString& reason ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual COM::EMcpAccessDecision AuthorizeSubscription( const COM::CMcpRequestContext& context, CORE::CString& reason ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAssetToolsAccessPolicy( const CAssetToolsAccessPolicy& src );              /**< not implemented */
    CAssetToolsAccessPolicy& operator=( const CAssetToolsAccessPolicy& src );   /**< not implemented */

    CAssetToolContext& m_context;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETTOOLSACCESSPOLICY_H ? */
