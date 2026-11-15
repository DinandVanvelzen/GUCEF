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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "AssetStoreConverterHook_CConverterHookSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CConverterHookSettings::CConverterHookSettings( void )
    : CORE::CIDataNodeSerializable()
    , cookProfile( "desktop" )
    , cookTarget()
    , importer( "auto" )
    , exporter( "gltf" )
    , threadPoolName( "AssetConversion" )
    , threads( 0 )
    , convertibleKinds()
    , mainExtensions()
    , nonStandardLicenceClasses()
{GUCEF_TRACE;

    convertibleKinds.insert( "static-model" );
    convertibleKinds.insert( "skeletal-model" );
    convertibleKinds.insert( "prefab" );
    convertibleKinds.insert( "scene" );
    mainExtensions.push_back( "scene" );
    mainExtensions.push_back( "mesh" );
    mainExtensions.push_back( "gltf" );
    mainExtensions.push_back( "glb" );
    nonStandardLicenceClasses.insert( "unknown" );
    nonStandardLicenceClasses.insert( "proprietary" );
    nonStandardLicenceClasses.insert( "editorial" );
}

/*-------------------------------------------------------------------------*/

CConverterHookSettings::~CConverterHookSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CConverterHookSettings::Serialize( CORE::CDataNode& domRootNode                        ,
                                   const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = cookProfile.Serialize( *domRootNode.AddChild( "cookProfile", GUCEF_DATATYPE_OBJECT ) );
    success = domRootNode.SetAttribute( "cookTarget", cookTarget ) && success;
    success = domRootNode.SetAttribute( "importer", importer ) && success;
    success = domRootNode.SetAttribute( "exporter", exporter ) && success;
    success = domRootNode.SetAttribute( "threadPoolName", threadPoolName ) && success;
    success = domRootNode.SetAttribute( "threads", threads ) && success;
    success = domRootNode.SetValuesOfChildByName( "convertibleKinds", convertibleKinds ) && success;
    success = domRootNode.SetValuesOfChildByName( "mainExtensions", mainExtensions ) && success;
    success = domRootNode.SetValuesOfChildByName( "nonStandardLicenceClasses", nonStandardLicenceClasses ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CConverterHookSettings::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                     const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CConverterHookSettings defaults;
    *this = defaults;
    const CORE::CDataNode* profileNode = domRootNode.FindChild( "cookProfile" );
    if ( GUCEF_NULL != profileNode )
        cookProfile.Deserialize( *profileNode );
    if ( cookProfile.name.IsNULLOrEmpty() )
        cookProfile.name = defaults.cookProfile.name;
    cookTarget = domRootNode.GetAttributeValueOrChildValueByName( "cookTarget" ).AsString( defaults.cookTarget );
    importer = domRootNode.GetAttributeValueOrChildValueByName( "importer" ).AsString( defaults.importer );
    exporter = domRootNode.GetAttributeValueOrChildValueByName( "exporter" ).AsString( defaults.exporter );
    threadPoolName = domRootNode.GetAttributeValueOrChildValueByName( "threadPoolName" ).AsString( defaults.threadPoolName );
    threads = domRootNode.GetAttributeValueOrChildValueByName( "threads" ).AsUInt32( defaults.threads, true );
    if ( GUCEF_NULL != domRootNode.FindChild( "convertibleKinds" ) )
    {
        convertibleKinds.clear();
        domRootNode.GetValuesOfChildByName( "convertibleKinds", convertibleKinds );
    }
    if ( GUCEF_NULL != domRootNode.FindChild( "mainExtensions" ) )
    {
        mainExtensions.clear();
        domRootNode.GetValuesOfChildByName( "mainExtensions", mainExtensions );
    }
    if ( GUCEF_NULL != domRootNode.FindChild( "nonStandardLicenceClasses" ) )
    {
        nonStandardLicenceClasses.clear();
        domRootNode.GetValuesOfChildByName( "nonStandardLicenceClasses", nonStandardLicenceClasses );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CConverterHookSettings::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CConverterHookSettings( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CConverterHookSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORECONVERTER::CConverterHookSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
