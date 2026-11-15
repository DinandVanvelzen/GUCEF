/*
 *  assetcookerOGRE: GUCEF asset converter plugin: the cooker of the intermediate into Ogre 1.x runtime files
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

#include <algorithm>
#include <math.h>

#include "Ogre.h"
#include "OgreRenderOperation.h"
#include "OgrePixelCountLodStrategy.h"
#include "OgreMeshLodGenerator.h"
#include "OgreLodConfig.h"

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "assetcookerOGRE_CLodCooker.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCOOKER {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const SourceName = "ogre";
static const Float64 QuarterPi = 0.78539816339744830962;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
IsNearer( const CLodCooker::SLevel& a, const CLodCooker::SLevel& b )
{
    return a.screenRelativeHeight > b.screenRelativeHeight;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLodCooker::SLevel::SLevel( void )
    : screenRelativeHeight( 0.0f )
    , manualMeshName()
    , reduction( 0.0f )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Float32
CLodCooker::ToStrategyValue( Float32 screenRelativeHeight, Float32 lodBias )
{GUCEF_TRACE;

    const Float64 height = (Float64) screenRelativeHeight * pow( 2.0, -(Float64) lodBias );
    return (Float32) ( QuarterPi * height * height );
}

/*-------------------------------------------------------------------------*/

void
CLodCooker::ReadReductionTable( const CORE::CDataNode& lodSettings, TLevelVector& levels, CORE::CString& method )
{GUCEF_TRACE;

    levels.clear();
    method = lodSettings.GetAttributeValueOrChildValueByName( "method" ).AsString();
    if ( method.IsNULLOrEmpty() )
        method = "proportional";
    const CORE::CDataNode* table = lodSettings.FindChild( "levels" );
    if ( GUCEF_NULL != table )
    {
        CORE::CDataNode::const_iterator i = table->ConstBegin();
        while ( i != table->ConstEnd() )
        {
            SLevel level;
            level.screenRelativeHeight = (*i)->GetAttributeValueOrChildValueByName( "screenRelativeHeight" ).AsFloat32( 0.0f );
            level.reduction = (*i)->GetAttributeValueOrChildValueByName( "reduction" ).AsFloat32( 0.0f );
            if ( level.screenRelativeHeight > 0.0f && level.reduction > 0.0f )
                levels.push_back( level );
            ++i;
        }
    }
    if ( levels.empty() )
    {
        SLevel level;
        level.screenRelativeHeight = 0.25f;
        level.reduction = 0.5f;
        levels.push_back( level );
        level.screenRelativeHeight = 0.1f;
        level.reduction = 0.8f;
        levels.push_back( level );
    }
    std::stable_sort( levels.begin(), levels.end(), IsNearer );
}

/*-------------------------------------------------------------------------*/

UInt32
CLodCooker::ApplyManualLevels( Ogre::MeshPtr& mesh, const TLevelVector& sourceLevels, Float32 lodBias, ASSETCONVERTER::CConversionReport& report )
{GUCEF_TRACE;

    if ( !mesh || sourceLevels.empty() )
        return 0;
    TLevelVector levels( sourceLevels );
    std::stable_sort( levels.begin(), levels.end(), IsNearer );
    try
    {
        Ogre::LodConfig config( mesh, Ogre::ScreenRatioPixelCountLodStrategy::getSingletonPtr() );
        for ( size_t l=0; l<levels.size(); ++l )
            config.createManualLodLevel( ToStrategyValue( levels[ l ].screenRelativeHeight, lodBias ), levels[ l ].manualMeshName.STL_String() );
        Ogre::MeshLodGenerator::getSingleton().generateLodLevels( config );
        return (UInt32) mesh->getNumLodLevels() - 1;
    }
    catch ( const Ogre::Exception& e )
    {
        report.AddError( SourceName, CORE::CString( mesh->getName() ), "the manual levels of detail cannot be set: " + CORE::CString( e.getDescription() ) );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CLodCooker::GenerateLevels( Ogre::MeshPtr& mesh, const CORE::CDataNode& lodSettings, Float32 lodBias, ASSETCONVERTER::CConversionReport& report )
{GUCEF_TRACE;

    if ( !mesh )
        return 0;
    TLevelVector levels;
    CORE::CString method;
    ReadReductionTable( lodSettings, levels, method );
    Ogre::LodLevel::VertexReductionMethod reduction = Ogre::LodLevel::VRM_PROPORTIONAL;
    if ( method.Equals( "constant", false ) )
        reduction = Ogre::LodLevel::VRM_CONSTANT;
    else
    if ( method.Equals( "collapseCost", false ) )
        reduction = Ogre::LodLevel::VRM_COLLAPSE_COST;
    else
    if ( !method.Equals( "proportional", false ) )
        report.AddWarning( SourceName, CORE::CString( mesh->getName() ), "the LOD reduction method " + method + " is not known: proportional is used" );

    try
    {
        Ogre::LodConfig config( mesh, Ogre::ScreenRatioPixelCountLodStrategy::getSingletonPtr() );
        config.advanced.useBackgroundQueue = false;
        for ( size_t l=0; l<levels.size(); ++l )
            config.createGeneratedLodLevel( ToStrategyValue( levels[ l ].screenRelativeHeight, lodBias ), levels[ l ].reduction, reduction );
        Ogre::MeshLodGenerator::getSingleton().generateLodLevels( config );
        const UInt32 made = (UInt32) mesh->getNumLodLevels() - 1;
        if ( made < levels.size() )
            report.AddInfo( SourceName, CORE::CString( mesh->getName() ), CORE::UInt32ToString( (UInt32) levels.size() - made ) + " generated level(s) skipped: they would not have fewer vertices" );
        return made;
    }
    catch ( const Ogre::Exception& e )
    {
        report.AddError( SourceName, CORE::CString( mesh->getName() ), "the levels of detail cannot be generated: " + CORE::CString( e.getDescription() ) );
    }
    return 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
