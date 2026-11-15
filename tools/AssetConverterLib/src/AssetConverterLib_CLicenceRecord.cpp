/*
 *  AssetConverterLib: the licence record of a pack, checked before conversion
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

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#include "AssetConverterLib_CLicenceRecord.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CLicenceRecord::NoMachineLearning = "noMachineLearning";
const CORE::CString CLicenceRecord::AttributionRequired = "attributionRequired";
const CORE::CString CLicenceRecord::NoCommercialUse = "noCommercialUse";
const CORE::CString CLicenceRecord::NoRedistribution = "noRedistribution";
const CORE::CString CLicenceRecord::EditorialOnly = "editorialOnly";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLicenceRecord::CLicenceRecord( void )
    : id()
    , store()
    , product()
    , orderReference()
    , purchaser()
    , licenceName()
    , licenceUrl()
    , checked()
    , seats( 0 )
    , ueOnly( false )
    , referenceOnly( false )
    , restrictedOrNonStandard( false )
    , humbleOneSeat( false )
    , reviewed( false )
    , notes()
    , restrictions()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CLicenceRecord::Load( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    CORE::CDataNode node;
    if ( !CCanonicalJson::Load( vfsPath, node ) )
        return false;
    return Deserialize( node );
}

/*-------------------------------------------------------------------------*/

bool
CLicenceRecord::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "id", id );
    node.SetAttribute( "store", store );
    node.SetAttribute( "product", product );
    node.SetAttribute( "order", orderReference );
    node.SetAttribute( "purchaser", purchaser );
    node.SetAttribute( "licence", licenceName );
    node.SetAttribute( "licenceUrl", licenceUrl );
    node.SetAttribute( "checked", checked );
    node.SetAttribute( "seats", seats );
    node.SetAttribute( "ueOnly", ueOnly );
    node.SetAttribute( "referenceOnly", referenceOnly );
    node.SetAttribute( "restrictedOrNonStandard", restrictedOrNonStandard );
    node.SetAttribute( "humbleOneSeat", humbleOneSeat );
    node.SetAttribute( "reviewed", reviewed );
    node.SetAttribute( "notes", notes );
    if ( !restrictions.empty() )
        node.SetValuesOfChildByName( "restrictions", restrictions );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLicenceRecord::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CLicenceRecord record;
    record.id = node.GetAttributeValueOrChildValueByName( "id" ).AsString();
    if ( record.id.IsNULLOrEmpty() )
        return false;
    record.store = node.GetAttributeValueOrChildValueByName( "store" ).AsString();
    record.product = node.GetAttributeValueOrChildValueByName( "product" ).AsString();
    record.orderReference = node.GetAttributeValueOrChildValueByName( "order" ).AsString();
    record.purchaser = node.GetAttributeValueOrChildValueByName( "purchaser" ).AsString();
    record.licenceName = node.GetAttributeValueOrChildValueByName( "licence" ).AsString();
    record.licenceUrl = node.GetAttributeValueOrChildValueByName( "licenceUrl" ).AsString();
    record.checked = node.GetAttributeValueOrChildValueByName( "checked" ).AsString();
    record.seats = node.GetAttributeValueOrChildValueByName( "seats" ).AsUInt32( 0 );
    record.ueOnly = node.GetAttributeValueOrChildValueByName( "ueOnly" ).AsBool( false );
    record.referenceOnly = node.GetAttributeValueOrChildValueByName( "referenceOnly" ).AsBool( false );
    record.restrictedOrNonStandard = node.GetAttributeValueOrChildValueByName( "restrictedOrNonStandard" ).AsBool( false );
    record.humbleOneSeat = node.GetAttributeValueOrChildValueByName( "humbleOneSeat" ).AsBool( false );
    record.reviewed = node.GetAttributeValueOrChildValueByName( "reviewed" ).AsBool( false );
    record.notes = node.GetAttributeValueOrChildValueByName( "notes" ).AsString();
    node.GetValuesOfChildByName( "restrictions", record.restrictions );
    *this = record;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLicenceRecord::HasRestriction( const CORE::CString& flag ) const
{GUCEF_TRACE;

    return restrictions.end() != restrictions.find( flag );
}

/*-------------------------------------------------------------------------*/

bool
CLicenceRecord::AllowsConversion( CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( ueOnly )
    {
        reason = "the licence record " + id + " marks the pack ueOnly: its content may only be used in Unreal Engine products";
        return false;
    }
    if ( referenceOnly )
    {
        reason = "the licence record " + id + " marks the pack referenceOnly: it gives no source format to convert";
        return false;
    }
    if ( restrictedOrNonStandard && !reviewed )
    {
        reason = "the licence record " + id + " marks the licence restricted or non-standard and it is not reviewed (reviewed: true)";
        return false;
    }
    reason.Clear();
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
