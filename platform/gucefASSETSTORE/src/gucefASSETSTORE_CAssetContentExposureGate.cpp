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

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */

#include "gucefASSETSTORE_CAssetContentExposureGate.h"

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

CAssetContentExposureGate::CAssetContentExposureGate( CIAssetStoreService* service )
    : m_service( service )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetContentExposureGate::SetService( CIAssetStoreService* service )
{GUCEF_TRACE;

    m_service = service;
}

/*-------------------------------------------------------------------------*/

bool
CAssetContentExposureGate::IsConverterPackAllowed( const CORE::CString& packId, const CORE::CString& stage, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_service )
    {
        reason = "the licence cannot be checked: no service";
        return false;
    }
    CAssetPipelineRequest request;
    request.operation = CAssetPipelineOperations::LicenceCheck;
    request.packId = packId;
    CAssetServiceDocument answer;
    const TAssetStoreStatus status = m_service->RunPipelineQuery( request, answer );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        reason = "the licence of the pack cannot be checked (" + CORE::CString( AssetStoreStatusToString( status ) ) + ")";
        return false;
    }
    const CORE::CString exposure = answer.document.GetAttributeValueOrChildValueByName( "contentExposure" ).AsString( CORE::CString::Empty, true );
    if ( "allowed" != exposure )
    {
        reason = answer.document.GetAttributeValueOrChildValueByName( "exposureReason" ).AsString( CORE::CString::Empty, true );
        if ( reason.IsNULLOrEmpty() )
            reason = "the licence of the pack does not allow content to be shown";
        return false;
    }
    if ( "source" == stage && !answer.document.GetAttributeValueOrChildValueByName( "conversionAllowed" ).AsBool( false, true ) )
    {
        reason = "the licence refuses the conversion of the pack, so its source files are not shown";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetContentExposureGate::IsStorePackAllowed( const CORE::CString& storePackId, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_service )
    {
        reason = "the licence cannot be checked: no service";
        return false;
    }
    CAssetLicence licence;
    const TAssetStoreStatus status = m_service->GetLicence( storePackId, licence );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        reason = "the licence of the store pack cannot be checked (" + CORE::CString( AssetStoreStatusToString( status ) ) + ")";
        return false;
    }
    if ( licence.licenceClass.IsNULLOrEmpty() || "unknown" == licence.licenceClass )
    {
        reason = "the licence of the store pack is unknown";
        return false;
    }
    if ( licence.restrictions.end() != licence.restrictions.find( CAssetLicence::NoMachineLearning ) )
    {
        reason = "the licence carries noMachineLearning";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetContentExposureGate::IsStoreAssetAllowed( const CORE::CString& storeAssetId, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_service )
    {
        reason = "the licence cannot be checked: no service";
        return false;
    }
    CAssetRecord asset;
    const TAssetStoreStatus status = m_service->GetAsset( storeAssetId, asset );
    if ( ASSETSTORE_STATUS_OK != status || asset.packId.IsNULLOrEmpty() )
    {
        reason = "the pack of the asset cannot be found";
        return false;
    }
    return IsStorePackAllowed( asset.packId, reason );
}

/*-------------------------------------------------------------------------*/

bool
CAssetContentExposureGate::IsAllowed( const CORE::CString& converterPackId ,
                                      const CORE::CString& storePackId     ,
                                      const CORE::CString& storeAssetId    ,
                                      const CORE::CString& stage           ,
                                      CORE::CString& reason                ) const
{GUCEF_TRACE;

    if ( converterPackId.IsNULLOrEmpty() && storePackId.IsNULLOrEmpty() && storeAssetId.IsNULLOrEmpty() )
    {
        reason = "the content names no pack, so its licence cannot be checked";
        return false;
    }
    if ( !converterPackId.IsNULLOrEmpty() && !IsConverterPackAllowed( converterPackId, stage, reason ) )
        return false;
    if ( !storePackId.IsNULLOrEmpty() && !IsStorePackAllowed( storePackId, reason ) )
        return false;
    if ( !storeAssetId.IsNULLOrEmpty() && !IsStoreAssetAllowed( storeAssetId, reason ) )
        return false;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
