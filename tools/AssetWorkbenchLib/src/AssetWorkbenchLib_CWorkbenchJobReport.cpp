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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHJOBREPORT_H
#include "AssetWorkbenchLib_CWorkbenchJobReport.h"
#define ASSETWORKBENCHLIB_CWORKBENCHJOBREPORT_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHJOBREPORT_H ? */

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

UInt32
CWorkbenchJobReport::BuildEntries( const ASSETSTORE::CAssetServiceDocument& result, GUI::CReportViewForm::TEntryVector& entries )
{GUCEF_TRACE;

    entries.clear();
    const CORE::CDataNode& document = result.document;

    // what the run did, in one line
    CORE::CString summary;
    const CORE::CDataNode* counts = document.FindChild( "counts" );
    if ( GUCEF_NULL != counts )
    {
        static const char* const names[] = { "converted", "cached", "approximated", "failed", "refused", "skipped" };
        for ( UInt32 i=0; i<6; ++i )
        {
            const UInt32 count = counts->GetAttributeValueOrChildValueByName( names[ i ] ).AsUInt32( 0, true );
            if ( count > 0 )
                summary += ( summary.IsNULLOrEmpty() ? "" : ", " ) + CORE::UInt32ToString( count ) + " " + names[ i ];
        }
    }
    if ( summary.IsNULLOrEmpty() )
        summary = result.message.IsNULLOrEmpty() ? CORE::CString( "no assets were reported" ) : result.message;
    const UInt64 durationMs = document.GetAttributeValueOrChildValueByName( "durationMs" ).AsUInt64( 0, true );
    entries.push_back( GUI::CReportViewForm::TEntry( GUI::CReportViewForm::SEVERITY_INFO, CORE::CString::Empty, "Result: " + summary,
                                                    "Operation " + document.GetAttributeValueOrChildValueByName( "operation" ).AsString( "?" ) + ", pack " +
                                                    document.GetAttributeValueOrChildValueByName( "packId" ).AsString( "?" ) + ", " + CORE::UInt64ToString( durationMs ) + " ms" ) );

    UInt32 assetCount = 0;
    const CORE::CDataNode* assets = document.FindChild( "assets" );
    if ( GUCEF_NULL != assets )
    {
        CORE::CDataNode::const_iterator a = assets->ConstBegin();
        for ( ; a != assets->ConstEnd(); ++a )
        {
            const CORE::CString state = (*a)->GetAttributeValueOrChildValueByName( "state" ).AsString();
            GUI::CReportViewForm::ESeverity severity = GUI::CReportViewForm::SEVERITY_INFO;
            if ( "failed" == state || "refused" == state )
                severity = GUI::CReportViewForm::SEVERITY_FAILURE;
            else
            if ( "approximated" == state )
                severity = GUI::CReportViewForm::SEVERITY_WARNING;
            entries.push_back( GUI::CReportViewForm::TEntry( severity, (*a)->GetAttributeValueOrChildValueByName( "assetPath" ).AsString(), state,
                                                            CORE::UInt64ToString( (*a)->GetAttributeValueOrChildValueByName( "durationMs" ).AsUInt64( 0, true ) ) + " ms" ) );
            ++assetCount;
        }
    }
    static const char* const lists[] = { "failures", "approximations" };
    for ( UInt32 l=0; l<2; ++l )
    {
        const CORE::CDataNode* list = document.FindChild( lists[ l ] );
        if ( GUCEF_NULL == list )
            continue;
        CORE::CDataNode::const_iterator f = list->ConstBegin();
        for ( ; f != list->ConstEnd(); ++f )
        {
            const CORE::CVariant& message = (*f)->GetAttributeValueOrChildValueByName( "message" );
            entries.push_back( GUI::CReportViewForm::TEntry( 0 == l ? GUI::CReportViewForm::SEVERITY_FAILURE : GUI::CReportViewForm::SEVERITY_WARNING,
                                                            (*f)->GetAttributeValueOrChildValueByName( "assetPath" ).AsString(),
                                                            message.AsString(), (*f)->GetAttributeValueOrChildValueByName( "source" ).AsString() ) );
        }
    }
    return assetCount;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobReport::GetFailedAssets( const ASSETSTORE::CAssetServiceDocument& result, CORE::CString::StringVector& assetPaths )
{GUCEF_TRACE;

    const CORE::CDataNode* assets = result.document.FindChild( "assets" );
    if ( GUCEF_NULL == assets )
        return;
    CORE::CDataNode::const_iterator a = assets->ConstBegin();
    for ( ; a != assets->ConstEnd(); ++a )
    {
        if ( "failed" == (*a)->GetAttributeValueOrChildValueByName( "state" ).AsString() )
            assetPaths.push_back( (*a)->GetAttributeValueOrChildValueByName( "assetPath" ).AsString() );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
