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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLRESOURCETEMPLATE_H
#define GUCEF_ASSETSTORE_CASSETTOOLRESOURCETEMPLATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CIMCPRESOURCETEMPLATE_H
#include "gucefCOM_CIMcpResourceTemplate.h"
#define GUCEF_COM_CIMCPRESOURCETEMPLATE_H
#endif /* GUCEF_COM_CIMCPRESOURCETEMPLATE_H ? */

#ifndef GUCEF_COM_CIMCPRESOURCE_H
#include "gucefCOM_CIMcpResource.h"
#define GUCEF_COM_CIMCPRESOURCE_H
#endif /* GUCEF_COM_CIMCPRESOURCE_H ? */

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
 *  The resources of the asset tools (section 8 of the asset tools design), scheme assettools, as resource templates
 *  (and the fixed resources server/status and docs/manifest-reference through the same class, see CAssetToolResource):
 *
 *  - pipeline: docs/import-settings/{importer}, packs/{packId}/manifest|licence|report|report-summary|activity|plan,
 *    packs/{packId}/assets/{+assetPath}/report|sidecar|override, packs/{packId}/cook/{profile}/references|sizes:
 *    RunPipelineQuery( read_resource ) with the URI; the converter hook serves the text;
 *  - store: store/assets/{storeAssetId} (GetAsset()), store/packs/{storePackId} (GetPack()),
 *    store/licences/{licenceId}/text (GetLicence()), store/assets/{storeAssetId}/thumbnail (GetThumbnails(): content,
 *    gated; the in-process service has no thumbnails yet, R5, so it answers unsupported);
 *  - jobs/{jobId}/result: the result document of a job of the caller (principal bound through the job bridge).
 *
 *  Not found is -32602, a refused path -32602, content of a withheld pack -31003 (the access policy refuses it before),
 *  a resource above maxResourceBytes and unsupported reads -32603 with the reason. Every text passes the redaction.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetToolResourceTemplate : public COM::CIMcpResourceTemplate
{
    public:

    enum EKind
    {
        KIND_PIPELINE = 0   ,
        KIND_STORE_ASSET    ,
        KIND_STORE_THUMBNAIL,
        KIND_STORE_PACK     ,
        KIND_LICENCE_TEXT   ,
        KIND_JOB_RESULT
    };

    CAssetToolResourceTemplate( CAssetToolContext& context, EKind kind, const CORE::CString& name, const CORE::CString& uriTemplate,
                                const CORE::CString& mimeType, const CORE::CString& description );

    virtual ~CAssetToolResourceTemplate() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetDescriptor( COM::CMcpResourceTemplateDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual COM::EMcpExecuteResult Read( COM::CMcpRequestContext& context      ,
                                         const CORE::CString& uri              ,
                                         const CORE::CValueList& variables     ,
                                         COM::CMcpResourceContents& contents   ,
                                         COM::CMcpInputRequired& inputRequired ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetCacheHints( const CORE::CString& uri, UInt32& ttlMs, COM::EMcpCacheScope& scope ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The read of an assettools URI by any of the kinds; used by the fixed resources too */
    static COM::EMcpExecuteResult ReadUri( CAssetToolContext& context              ,
                                           EKind kind                              ,
                                           COM::CMcpRequestContext& request        ,
                                           const CORE::CString& uri                ,
                                           COM::CMcpResourceContents& contents     );

    private:

    CAssetToolResourceTemplate( const CAssetToolResourceTemplate& src );              /**< not implemented */
    CAssetToolResourceTemplate& operator=( const CAssetToolResourceTemplate& src );   /**< not implemented */

    CAssetToolContext& m_context;
    EKind m_kind;
    CORE::CString m_name;
    CORE::CString m_uriTemplate;
    CORE::CString m_mimeType;
    CORE::CString m_description;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETTOOLRESOURCETEMPLATE_H ? */
