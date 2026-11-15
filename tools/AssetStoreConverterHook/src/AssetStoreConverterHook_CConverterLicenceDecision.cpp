/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_COM_CMCPSCHEMABUILDER_H
#include "gucefCOM_CMcpSchemaBuilder.h"
#define GUCEF_COM_CMCPSCHEMABUILDER_H
#endif /* GUCEF_COM_CMCPSCHEMABUILDER_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetStoreConverterHook_CConverterLicenceDecision.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CConverterLicenceDecision::ExposureAllowed = "allowed";
const CORE::CString CConverterLicenceDecision::ExposureWithheld = "withheld";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CConverterLicenceDecision::CConverterLicenceDecision( void )
    : recordId()
    , recordFound( false )
    , record()
    , conversionAllowed( false )
    , reason()
    , contentExposure( ExposureWithheld )
    , exposureReason()
    , recordVfsPath()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CConverterLicenceDecision::Evaluate( const ASSETCONVERTER::CAssetPackManifest& manifest )
{GUCEF_TRACE;

    *this = CConverterLicenceDecision();
    recordId = manifest.licenceRecord;
    if ( recordId.IsNULLOrEmpty() )
    {
        reason = "the manifest names no licence record";
        Decide();
        return;
    }
    EvaluateRecord( manifest.licenceRecord, manifest.GetLicenceRecordPath() );
}

/*-------------------------------------------------------------------------*/

void
CConverterLicenceDecision::EvaluateRecord( const CORE::CString& id, const CORE::CString& vfsPath )
{GUCEF_TRACE;

    *this = CConverterLicenceDecision();
    recordId = id;
    recordVfsPath = vfsPath;
    const CORE::CString fileName = ASSETCONVERTER::CVfsPaths::NameOf( vfsPath );
    if ( vfsPath.IsNULLOrEmpty() || !VFS::CVfsGlobal::Instance()->GetVfs().FileExists( vfsPath ) || !record.Load( vfsPath ) )
    {
        record = ASSETCONVERTER::CLicenceRecord();
        reason = "there is no readable licence record " + ( fileName.IsNULLOrEmpty() ? id + ".licence.json" : fileName ) + " in the licence folder of the pack";
        Decide();
        return;
    }
    recordFound = true;
    if ( record.id != id )
    {
        reason = "the licence record " + fileName + " has the identifier " + record.id + ", not " + id;
        Decide();
        return;
    }
    record.AllowsConversion( reason );
    Decide();
}

/*-------------------------------------------------------------------------*/

void
CConverterLicenceDecision::Decide( void )
{GUCEF_TRACE;

    conversionAllowed = reason.IsNULLOrEmpty();
    if ( !recordFound )
    {
        contentExposure = ExposureWithheld;
        exposureReason = recordId.IsNULLOrEmpty() ? "the pack has no licence record" : "the licence record of the pack cannot be read";
        return;
    }
    if ( record.HasRestriction( ASSETCONVERTER::CLicenceRecord::NoMachineLearning ) )
    {
        contentExposure = ExposureWithheld;
        exposureReason = "the licence record carries " + ASSETCONVERTER::CLicenceRecord::NoMachineLearning;
        return;
    }
    contentExposure = ExposureAllowed;
    exposureReason.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CConverterLicenceDecision::IsStageWithheld( const CORE::CString& stage, CORE::CString& why ) const
{GUCEF_TRACE;

    if ( ExposureWithheld == contentExposure )
    {
        why = exposureReason;
        return true;
    }
    if ( "source" == stage && !conversionAllowed )
    {
        why = "the conversion of the pack is refused (" + reason + "), so its sources are not shown";
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CConverterLicenceDecision::GetMissingFields( CORE::CString::StringVector& fields ) const
{GUCEF_TRACE;

    fields.clear();
    if ( !recordFound )
    {
        fields.push_back( "record" );
        return;
    }
    if ( record.store.IsNULLOrEmpty() )
        fields.push_back( "store" );
    if ( record.product.IsNULLOrEmpty() )
        fields.push_back( "product" );
    if ( record.licenceName.IsNULLOrEmpty() )
        fields.push_back( "licenceName" );
    if ( record.licenceUrl.IsNULLOrEmpty() )
        fields.push_back( "licenceUrl" );
    if ( record.checked.IsNULLOrEmpty() )
        fields.push_back( "checked" );
    if ( 0 == record.seats )
        fields.push_back( "seats" );
}

/*-------------------------------------------------------------------------*/

void
CConverterLicenceDecision::ToDataNode( CORE::CDataNode& node, bool details, bool exposePurchaseDetails, UInt32 maxUntrustedTextChars ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    if ( recordId.IsNULLOrEmpty() )
        node.SetAttribute( "recordId", CORE::CVariant() );
    else
        node.SetAttribute( "recordId", recordId );
    node.SetAttribute( "recordFound", recordFound );
    node.SetAttribute( "conversionAllowed", conversionAllowed );
    if ( reason.IsNULLOrEmpty() )
        node.SetAttribute( "reason", CORE::CVariant() );
    else
        node.SetAttribute( "reason", reason );
    CORE::CDataNode* list = node.AddChild( "restrictions", GUCEF_DATATYPE_ARRAY );
    if ( recordFound )
        for ( CORE::CString::StringSet::const_iterator r=record.restrictions.begin(); r!=record.restrictions.end(); ++r )
            list->AddValueAsChild( (*r) );
    node.SetAttribute( "contentExposure", contentExposure );
    if ( exposureReason.IsNULLOrEmpty() )
        node.SetAttribute( "exposureReason", CORE::CVariant() );
    else
        node.SetAttribute( "exposureReason", exposureReason );
    if ( !details )
        return;
    node.SetAttribute( "licenceName", recordFound ? record.licenceName : CORE::CString::Empty );
    node.SetAttribute( "licenceUrl", recordFound ? record.licenceUrl : CORE::CString::Empty );
    node.SetAttribute( "checked", recordFound ? record.checked : CORE::CString::Empty );
    node.SetAttribute( "store", recordFound ? record.store : CORE::CString::Empty );
    node.SetAttribute( "product", recordFound ? record.product : CORE::CString::Empty );
    node.SetAttribute( "seats", recordFound ? record.seats : (UInt32) 0 );
    node.SetAttribute( "ueOnly", recordFound && record.ueOnly );
    node.SetAttribute( "referenceOnly", recordFound && record.referenceOnly );
    node.SetAttribute( "restrictedOrNonStandard", recordFound && record.restrictedOrNonStandard );
    node.SetAttribute( "humbleOneSeat", recordFound && record.humbleOneSeat );
    node.SetAttribute( "reviewed", recordFound && record.reviewed );
    if ( recordFound && !record.notes.IsNULLOrEmpty() )
    {
        CORE::CDataNode notes = COM::CMcpSchemaBuilder::MakeUntrustedTextValue( "licence-text", record.notes, maxUntrustedTextChars );
        notes.SetName( "notes" );
        node.AddChild( notes );
    }
    else
        node.SetAttribute( "notes", CORE::CVariant() );
    if ( exposePurchaseDetails && recordFound )
    {
        node.SetAttribute( "purchaser", record.purchaser );
        node.SetAttribute( "orderReference", record.orderReference );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
