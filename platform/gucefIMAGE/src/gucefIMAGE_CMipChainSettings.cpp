/*
 *  gucefIMAGE: GUCEF module providing image utilities
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefIMAGE_CMipChainSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CMipChainSettings::ConfigSectionName = "MipChainSettings";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMipChainSettings::CMipChainSettings( void )
    : CORE::CIConfigurable()
    , filter( RESAMPLEFILTER_KAISER )
    , colourSpace( COLOURSPACE_SRGB )
    , content( IMAGECONTENT_COLOUR )
    , wrapMode( WRAPMODE_REPEAT )
    , alphaCoverageCutoff( -1.0f )
    , minimumSize( 1 )
    , normalMapSigned( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMipChainSettings::CMipChainSettings( const CMipChainSettings& src )
    : CORE::CIConfigurable( src )
    , filter( src.filter )
    , colourSpace( src.colourSpace )
    , content( src.content )
    , wrapMode( src.wrapMode )
    , alphaCoverageCutoff( src.alphaCoverageCutoff )
    , minimumSize( src.minimumSize )
    , normalMapSigned( src.normalMapSigned )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMipChainSettings::~CMipChainSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMipChainSettings&
CMipChainSettings::operator=( const CMipChainSettings& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CORE::CIConfigurable::operator=( src );
        filter = src.filter;
        colourSpace = src.colourSpace;
        content = src.content;
        wrapMode = src.wrapMode;
        alphaCoverageCutoff = src.alphaCoverageCutoff;
        minimumSize = src.minimumSize;
        normalMapSigned = src.normalMapSigned;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CMipChainSettings::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = tree.Structure( ConfigSectionName, '/' );
    if ( GUCEF_NULL == section )
        return false;

    section->SetAttribute( "filter", ToString( filter ) );
    section->SetAttribute( "colourSpace", ToString( colourSpace ) );
    section->SetAttribute( "content", ToString( content ) );
    section->SetAttribute( "wrapMode", ToString( wrapMode ) );
    section->SetAttribute( "alphaCoverageCutoff", alphaCoverageCutoff );
    section->SetAttribute( "minimumSize", minimumSize );
    section->SetAttribute( "normalMapSigned", normalMapSigned );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMipChainSettings::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &treeroot;
    if ( treeroot.GetName() != ConfigSectionName )
        section = treeroot.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return false;

    // A name that is not known leaves the setting as it is
    FromString( section->GetAttributeValueOrChildValueByName( "filter" ).AsString( CORE::CString::Empty, true ), filter );
    FromString( section->GetAttributeValueOrChildValueByName( "colourSpace" ).AsString( CORE::CString::Empty, true ), colourSpace );
    FromString( section->GetAttributeValueOrChildValueByName( "content" ).AsString( CORE::CString::Empty, true ), content );
    FromString( section->GetAttributeValueOrChildValueByName( "wrapMode" ).AsString( CORE::CString::Empty, true ), wrapMode );
    alphaCoverageCutoff = section->GetAttributeValueOrChildValueByName( "alphaCoverageCutoff" ).AsFloat32( alphaCoverageCutoff, true );
    minimumSize = section->GetAttributeValueOrChildValueByName( "minimumSize" ).AsUInt32( minimumSize, true );
    normalMapSigned = section->GetAttributeValueOrChildValueByName( "normalMapSigned" ).AsBool( normalMapSigned, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMipChainSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::IMAGE::CMipChainSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
