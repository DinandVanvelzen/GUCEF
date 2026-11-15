/*
 *  AssetConverterLib: the built-in validator of triangle budgets per LOD level
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

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef GUCEF_CORE_CGLOBPATTERN_H
#include "gucefCORE_CGlobPattern.h"
#define GUCEF_CORE_CGLOBPATTERN_H
#endif /* GUCEF_CORE_CGLOBPATTERN_H ? */

#include "AssetConverterLib_CTriangleBudgetValidator.h"

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

const CORE::CString CTriangleBudgetValidator::TypeName = "TriangleBudget";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTriangleBudgetValidator::CTriangleBudgetValidator( void )
    : CIAssetValidator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTriangleBudgetValidator::~CTriangleBudgetValidator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CTriangleBudgetValidator::GetName( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CTriangleBudgetValidator::GetVersion( void ) const
{GUCEF_TRACE;

    return 1;
}

/*-------------------------------------------------------------------------*/

bool
CTriangleBudgetValidator::Validate( const CAssetScene& scene, const CORE::CDataNode& settings, CConversionReport& report )
{GUCEF_TRACE;

    const TReportItemKind severity = GetSeverity( settings, REPORTITEM_WARNING );
    const CORE::CDataNode* budgets = settings.FindChild( "budgets" );
    if ( GUCEF_NULL == budgets )
        return true;
    CORE::CDataNode::TVariantVector limits;
    bool found = false;
    CORE::CDataNode::const_iterator b = budgets->ConstBegin();
    while ( b != budgets->ConstEnd() && !found )
    {
        const CORE::CString match = (*b)->GetAttributeValueOrChildValueByName( "match" ).AsString( "**" );
        if ( CORE::CGlobPattern::IsMatch( match, scene.provenance.assetPath, false ) )
        {
            (*b)->GetValuesOfChildByName( "levels", limits );
            found = true;
        }
        ++b;
    }
    if ( !found || limits.empty() )
        return true;

    // the triangles per level
    GUCEF::vector< UInt32 > triangles;
    GUCEF::vector< bool > inLevel( scene.nodes.size(), false );
    for ( size_t g=0; g<scene.lodGroups.size(); ++g )
    {
        for ( size_t l=0; l<scene.lodGroups[ g ].levels.size(); ++l )
        {
            if ( triangles.size() <= l )
                triangles.resize( l+1, 0 );
            for ( size_t n=0; n<scene.nodes.size(); ++n )
            {
                if ( !scene.IsInSubtree( (UInt32) n, scene.lodGroups[ g ].levels[ l ].node ) )
                    continue;
                inLevel[ n ] = true;
                if ( scene.nodes[ n ].mesh >= 0 && (size_t) scene.nodes[ n ].mesh < scene.meshes.size() )
                    triangles[ l ] += scene.meshes[ scene.nodes[ n ].mesh ].GetTriangleCount();
            }
        }
    }
    if ( triangles.empty() )
        triangles.resize( 1, 0 );
    for ( size_t n=0; n<scene.nodes.size(); ++n )
        if ( !inLevel[ n ] && scene.nodes[ n ].mesh >= 0 && (size_t) scene.nodes[ n ].mesh < scene.meshes.size() )
            triangles[ 0 ] += scene.meshes[ scene.nodes[ n ].mesh ].GetTriangleCount();

    for ( size_t l=0; l<triangles.size() && l<limits.size(); ++l )
    {
        const UInt32 limit = limits[ l ].AsUInt32( 0 );
        if ( limit > 0 && triangles[ l ] > limit )
            report.AddItem( severity, TypeName, "LOD" + CORE::UInt32ToString( (UInt32) l ), CORE::UInt32ToString( triangles[ l ] ) + " triangles, the budget is " + CORE::UInt32ToString( limit ) );
    }
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
