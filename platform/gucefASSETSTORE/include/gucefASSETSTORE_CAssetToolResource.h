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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLRESOURCE_H
#define GUCEF_ASSETSTORE_CASSETTOOLRESOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  The fixed resources of the asset tools: assettools://server/status (the service and its implementation, the
 *  exposure in force with its groups, the ids and kinds of the roots, the limits, whether a renderer and a converter
 *  are there; no secret, no path of the disk) and assettools://docs/manifest-reference (the manifest format, served by
 *  the converter hook through RunPipelineQuery( read_resource )).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetToolResource : public COM::CIMcpResource
{
    public:

    enum EKind
    {
        KIND_STATUS = 0         ,
        KIND_MANIFEST_REFERENCE
    };

    CAssetToolResource( CAssetToolContext& context, EKind kind );

    virtual ~CAssetToolResource() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetDescriptor( COM::CMcpResourceDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual COM::EMcpExecuteResult Read( COM::CMcpRequestContext& context      ,
                                         COM::CMcpResourceContents& contents   ,
                                         COM::CMcpInputRequired& inputRequired ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetCacheHints( UInt32& ttlMs, COM::EMcpCacheScope& scope ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The status document */
    static void BuildStatus( CAssetToolContext& context, CORE::CDataNode& status );

    private:

    CAssetToolResource( const CAssetToolResource& src );              /**< not implemented */
    CAssetToolResource& operator=( const CAssetToolResource& src );   /**< not implemented */

    CAssetToolContext& m_context;
    EKind m_kind;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETTOOLRESOURCE_H ? */
