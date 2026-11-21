/*
 *  AssetWorkbenchLib: views and controllers of the asset workbench
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETWORKBENCHLIB_CBULKCONVERSIONPLAN_H
#include "AssetWorkbenchLib_CBulkConversionPlan.h"
#define ASSETWORKBENCHLIB_CBULKCONVERSIONPLAN_H
#endif /* ASSETWORKBENCHLIB_CBULKCONVERSIONPLAN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CBulkConversionPlan::SItem::SItem( void )
    : assetPath()
    , action()
    , reason()
    , detail()
    , importer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CBulkConversionPlan::CBulkConversionPlan( void )
    : packId()
    , valid( false )
    , conversionAllowed( true )
    , refusalReason()
    , restrictions()
    , problems()
    , cookTargets()
    , lockHolder()
    , importCount( 0 )
    , cachedCount( 0 )
    , cookCount( 0 )
    , refusedCount( 0 )
    , failedCount( 0 )
    , validateCount( 0 )
    , noIntermediateCount( 0 )
    , filteredOutCount( 0 )
    , lastRunAssetTimeMs( 0 )
    , totalItems( 0 )
    , items()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CBulkConversionPlan::Clear( void )
{GUCEF_TRACE;

    *this = CBulkConversionPlan();
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionPlan::Parse( const ASSETSTORE::CAssetServiceDocument& answer )
{GUCEF_TRACE;

    Clear();
    const CORE::CDataNode& document = answer.document;
    const CORE::CDataNode* counts = document.FindChild( "counts" );
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != answer.status || GUCEF_NULL == counts )
    {
        // a refusal of the pipeline (the licence, a lock) is a plan with the reason
        refusalReason = answer.message;
        conversionAllowed = answer.message.IsNULLOrEmpty();
        return false;
    }
    valid = true;
    packId = document.GetAttributeValueOrChildValueByName( "packId" ).AsString();
    const CORE::CDataNode* licence = document.FindChild( "licence" );
    if ( GUCEF_NULL != licence )
    {
        conversionAllowed = licence->GetAttributeValueOrChildValueByName( "conversionAllowed" ).AsBool( true, true );
        refusalReason = licence->GetAttributeValueOrChildValueByName( "reason" ).AsString();
        licence->GetValuesOfChildByName( "restrictions", restrictions );
    }
    importCount = counts->GetAttributeValueOrChildValueByName( "import" ).AsUInt32( 0, true );
    cachedCount = counts->GetAttributeValueOrChildValueByName( "cached" ).AsUInt32( 0, true );
    cookCount = counts->GetAttributeValueOrChildValueByName( "cook" ).AsUInt32( 0, true );
    refusedCount = counts->GetAttributeValueOrChildValueByName( "refused" ).AsUInt32( 0, true );
    failedCount = counts->GetAttributeValueOrChildValueByName( "failed" ).AsUInt32( 0, true );
    validateCount = counts->GetAttributeValueOrChildValueByName( "validate" ).AsUInt32( 0, true );
    noIntermediateCount = counts->GetAttributeValueOrChildValueByName( "noIntermediate" ).AsUInt32( 0, true );
    filteredOutCount = counts->GetAttributeValueOrChildValueByName( "filteredOut" ).AsUInt32( 0, true );
    lockHolder = document.GetAttributeValueOrChildValueByName( "lockHolder" ).AsString();
    document.GetValuesOfChildByName( "problems", problems );
    document.GetValuesOfChildByName( "cookTargets", cookTargets );
    lastRunAssetTimeMs = document.GetAttributeValueOrChildValueByName( "lastRunAssetTimeMs" ).AsUInt64( 0, true );
    const CORE::CDataNode* list = document.FindChild( "items" );
    if ( GUCEF_NULL != list )
    {
        CORE::CDataNode::const_iterator i = list->ConstBegin();
        for ( ; i != list->ConstEnd(); ++i )
        {
            SItem item;
            item.assetPath = (*i)->GetAttributeValueOrChildValueByName( "assetPath" ).AsString();
            item.action = (*i)->GetAttributeValueOrChildValueByName( "action" ).AsString();
            item.reason = (*i)->GetAttributeValueOrChildValueByName( "reason" ).AsString();
            item.detail = (*i)->GetAttributeValueOrChildValueByName( "detail" ).AsString();
            item.importer = (*i)->GetAttributeValueOrChildValueByName( "importer" ).AsString();
            items.push_back( item );
        }
    }
    totalItems = importCount + cachedCount + cookCount + refusedCount + failedCount + validateCount + noIntermediateCount;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CBulkConversionPlan::GetWorkCount( void ) const
{GUCEF_TRACE;

    return importCount + cookCount + validateCount;
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionPlan::CanStart( void ) const
{GUCEF_TRACE;

    return valid && conversionAllowed && lockHolder.IsNULLOrEmpty() && GetWorkCount() > 0;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CBulkConversionPlan::ToSummaryText( void ) const
{GUCEF_TRACE;

    if ( !conversionAllowed )
        return "Refused: " + ( refusalReason.IsNULLOrEmpty() ? CORE::CString( "the licence does not allow the conversion" ) : refusalReason );
    if ( !valid )
        return refusalReason.IsNULLOrEmpty() ? CORE::CString( "No plan yet" ) : "No plan: " + refusalReason;
    CORE::CString text = CORE::UInt32ToString( importCount ) + " to import, " + CORE::UInt32ToString( cookCount ) + " to cook, " +
                         CORE::UInt32ToString( cachedCount ) + " up to date, " + CORE::UInt32ToString( refusedCount ) + " refused, " +
                         CORE::UInt32ToString( failedCount ) + " failed";
    if ( validateCount > 0 )
        text += ", " + CORE::UInt32ToString( validateCount ) + " to validate";
    if ( filteredOutCount > 0 )
        text += ", " + CORE::UInt32ToString( filteredOutCount ) + " outside the scope";
    if ( lastRunAssetTimeMs > 0 )
        text += "; the last run of the pack took " + CORE::UInt64ToString( lastRunAssetTimeMs / 1000 ) + " s of asset time";
    if ( !lockHolder.IsNULLOrEmpty() )
        text += "; the intermediate is in use by " + lockHolder;
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CBulkConversionPlan::DescribeWork( const SItem& item )
{GUCEF_TRACE;

    if ( "import" == item.action )
        return "import and export";
    if ( "cook" == item.action )
        return "cook";
    if ( "validate" == item.action )
        return "validate";
    if ( "cached" == item.action )
        return "none (up to date)";
    if ( "refused" == item.action )
        return "refused" + ( item.detail.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + item.detail );
    if ( "failed" == item.action )
        return "fails" + ( item.detail.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + item.detail );
    if ( "no intermediate" == item.action )
        return "none (no intermediate to cook from)";
    return item.action;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
