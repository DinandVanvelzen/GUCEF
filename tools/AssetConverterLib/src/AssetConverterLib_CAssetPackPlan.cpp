/*
 *  AssetConverterLib: the plan of a run: what it would convert, keep, cook or refuse, and why, without writing anything
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

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#include "AssetConverterLib_CAssetPackPlan.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetPackPlan::CAssetPackPlan( void )
    : packId()
    , mode( PIPELINEMODE_FULL )
    , licenceRecordId()
    , conversionAllowed( false )
    , licenceReason()
    , restrictions()
    , assetFilter()
    , cookTargets()
    , intermediateDir()
    , lockHolder()
    , indexNote()
    , filteredOut( 0 )
    , problems()
    , entries()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CAssetPackPlan::GetCount( TPlanAction action ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<entries.size(); ++i )
        if ( entries[ i ].action == action )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

const CAssetPlanEntry*
CAssetPackPlan::FindEntry( const CORE::CString& assetPath ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<entries.size(); ++i )
        if ( entries[ i ].assetPath == assetPath )
            return &entries[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackPlan::IsRunnable( void ) const
{GUCEF_TRACE;

    return conversionAllowed && problems.empty();
}

/*-------------------------------------------------------------------------*/

void
CAssetPackPlan::Clear( void )
{GUCEF_TRACE;

    *this = CAssetPackPlan();
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackPlan::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "pack", packId );
    node.SetAttribute( "mode", ToString( mode ) );
    node.SetAttribute( "licenceRecord", licenceRecordId );
    node.SetAttribute( "conversionAllowed", conversionAllowed );
    if ( !licenceReason.IsNULLOrEmpty() )
        node.SetAttribute( "licenceReason", licenceReason );
    node.SetValuesOfChildByName( "restrictions", restrictions );
    node.SetValuesOfChildByName( "assetFilter", assetFilter );
    node.SetValuesOfChildByName( "cookTargets", cookTargets );
    node.SetAttribute( "intermediate", intermediateDir );
    if ( !lockHolder.IsNULLOrEmpty() )
        node.SetAttribute( "lockHolder", lockHolder );
    if ( !indexNote.IsNULLOrEmpty() )
        node.SetAttribute( "indexNote", indexNote );
    node.SetAttribute( "filteredOut", filteredOut );
    node.SetValuesOfChildByName( "problems", problems );
    CORE::CDataNode* counts = node.AddChild( "counts", GUCEF_DATATYPE_OBJECT );
    for ( Int32 a=0; a<PLANACTION_COUNT; ++a )
        counts->SetAttribute( ToString( (TPlanAction) a ), GetCount( (TPlanAction) a ) );
    CORE::CDataNode* assets = node.AddChild( "assets", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<entries.size(); ++i )
        entries[ i ].Serialize( *assets->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackPlan::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CAssetPackPlan plan;
    plan.packId = node.GetAttributeValueOrChildValueByName( "pack" ).AsString();
    FromString( node.GetAttributeValueOrChildValueByName( "mode" ).AsString(), plan.mode );
    plan.licenceRecordId = node.GetAttributeValueOrChildValueByName( "licenceRecord" ).AsString();
    plan.conversionAllowed = node.GetAttributeValueOrChildValueByName( "conversionAllowed" ).AsBool( false );
    plan.licenceReason = node.GetAttributeValueOrChildValueByName( "licenceReason" ).AsString();
    node.GetValuesOfChildByName( "restrictions", plan.restrictions );
    node.GetValuesOfChildByName( "assetFilter", plan.assetFilter );
    node.GetValuesOfChildByName( "cookTargets", plan.cookTargets );
    plan.intermediateDir = node.GetAttributeValueOrChildValueByName( "intermediate" ).AsString();
    plan.lockHolder = node.GetAttributeValueOrChildValueByName( "lockHolder" ).AsString();
    plan.indexNote = node.GetAttributeValueOrChildValueByName( "indexNote" ).AsString();
    plan.filteredOut = node.GetAttributeValueOrChildValueByName( "filteredOut" ).AsUInt32( 0 );
    node.GetValuesOfChildByName( "problems", plan.problems );
    const CORE::CDataNode* assets = node.FindChild( "assets" );
    if ( GUCEF_NULL != assets )
    {
        CORE::CDataNode::const_iterator i = assets->ConstBegin();
        while ( i != assets->ConstEnd() )
        {
            CAssetPlanEntry entry;
            if ( entry.Deserialize( *(*i) ) )
                plan.entries.push_back( entry );
            ++i;
        }
    }
    *this = plan;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackPlan::SaveJson( const CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    CORE::CDataNode node( "plan", GUCEF_DATATYPE_OBJECT );
    Serialize( node );
    return CCanonicalJson::Store( vfsPath, node );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackPlan::LoadJson( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    CORE::CDataNode node;
    if ( !CCanonicalJson::Load( vfsPath, node ) )
        return false;
    return Deserialize( node );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetPackPlan::ToSummaryText( void ) const
{GUCEF_TRACE;

    CORE::CString text = "Plan of pack " + packId + " (" + ToString( mode ) + ")\n";
    text += "Licence record: " + ( licenceRecordId.IsNULLOrEmpty() ? CORE::CString( "(none)" ) : licenceRecordId );
    text += conversionAllowed ? CORE::CString( ", conversion allowed\n" ) : ", REFUSED: " + licenceReason + "\n";
    if ( !restrictions.empty() )
    {
        text += "Restrictions:";
        for ( CORE::CString::StringSet::const_iterator r = restrictions.begin(); r != restrictions.end(); ++r )
            text += " " + (*r);
        text += "\n";
    }
    if ( !assetFilter.empty() )
        text += "Assets restricted to " + CORE::ToString( (UInt32) assetFilter.size() ) + " globs; " + CORE::ToString( filteredOut ) + " assets left out\n";
    if ( !lockHolder.IsNULLOrEmpty() )
        text += "The intermediate is locked by " + lockHolder + "\n";
    if ( !indexNote.IsNULLOrEmpty() )
        text += "Pack index: " + indexNote + "\n";
    for ( size_t p=0; p<problems.size(); ++p )
        text += "PROBLEM: " + problems[ p ] + "\n";
    for ( Int32 a=0; a<PLANACTION_COUNT; ++a )
        text += ToString( (TPlanAction) a ) + ": " + CORE::ToString( GetCount( (TPlanAction) a ) ) + "\n";
    for ( size_t i=0; i<entries.size(); ++i )
    {
        const CAssetPlanEntry& entry = entries[ i ];
        if ( PLANACTION_CACHED == entry.action )
            continue;
        text += "  " + entry.assetPath + " [" + entry.importer + "]: " + ToString( entry.action ) + " (" + ToString( entry.reason );
        if ( !entry.detail.IsNULLOrEmpty() )
            text += ": " + entry.detail;
        text += ")\n";
    }
    return text;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
