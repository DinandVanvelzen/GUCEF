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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "AssetStoreConverterHook_CPipelineOperationSettings.h"

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

CPipelineOperationSettings::CPipelineOperationSettings( void )
    : CORE::CIDataNodeSerializable()
    , roots()
    , historyVfsPath()
    , changeTokenTtlMs( 600000 )
    , maxChangeTokensPerPrincipal( 32 )
    , defaultPageEntries( 25 )
    , maxPageEntries( 100 )
    , planSampleEntries( 50 )
    , maxResourceBytes( 1048576 )
    , maxDiffLines( 400 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPipelineOperationSettings::~CPipelineOperationSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPipelineOperationSettings::IsValid( CORE::CString& problem ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<roots.size(); ++i )
    {
        if ( !roots[ i ].IsValid( problem ) )
            return false;
        for ( size_t j=0; j<i; ++j )
        {
            if ( roots[ j ].id == roots[ i ].id )
            {
                problem = "the root id " + roots[ i ].id + " is used twice";
                return false;
            }
        }
    }
    if ( 0 == changeTokenTtlMs || 0 == maxChangeTokensPerPrincipal || 0 == defaultPageEntries || 0 == maxPageEntries ||
         0 == planSampleEntries || 0 == maxResourceBytes || 0 == maxDiffLines )
    {
        problem = "a limit of the pipeline operations is 0";
        return false;
    }
    if ( defaultPageEntries > maxPageEntries )
    {
        problem = "defaultPageEntries is larger than maxPageEntries";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPipelineOperationSettings::Serialize( CORE::CDataNode& domRootNode                        ,
                                       const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    // SaveList() clears the node it writes to, its name too: the name is set after it
    CORE::CDataNode* rootList = domRootNode.AddChild( "roots", GUCEF_DATATYPE_ARRAY );
    ASSETSTORE::CAssetToolRoot::SaveList( roots, *rootList );
    rootList->SetName( "roots" );
    bool success = domRootNode.SetAttribute( "historyVfsPath", historyVfsPath );
    success = domRootNode.SetAttribute( "changeTokenTtlMs", changeTokenTtlMs ) && success;
    success = domRootNode.SetAttribute( "maxChangeTokensPerPrincipal", maxChangeTokensPerPrincipal ) && success;
    success = domRootNode.SetAttribute( "defaultPageEntries", defaultPageEntries ) && success;
    success = domRootNode.SetAttribute( "maxPageEntries", maxPageEntries ) && success;
    success = domRootNode.SetAttribute( "planSampleEntries", planSampleEntries ) && success;
    success = domRootNode.SetAttribute( "maxResourceBytes", maxResourceBytes ) && success;
    success = domRootNode.SetAttribute( "maxDiffLines", maxDiffLines ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CPipelineOperationSettings::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                         const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    const CPipelineOperationSettings defaults;
    *this = defaults;
    const CORE::CDataNode* rootList = domRootNode.FindChild( "roots" );
    if ( GUCEF_NULL != rootList )
    {
        CORE::CString problem;
        if ( !ASSETSTORE::CAssetToolRoot::LoadList( *rootList, roots, problem ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CPipelineOperationSettings: The roots cannot be read: " << problem );
            return false;
        }
    }
    historyVfsPath = domRootNode.GetAttributeValueOrChildValueByName( "historyVfsPath" ).AsString( defaults.historyVfsPath, true );
    changeTokenTtlMs = domRootNode.GetAttributeValueOrChildValueByName( "changeTokenTtlMs" ).AsUInt32( defaults.changeTokenTtlMs, true );
    maxChangeTokensPerPrincipal = domRootNode.GetAttributeValueOrChildValueByName( "maxChangeTokensPerPrincipal" ).AsUInt32( defaults.maxChangeTokensPerPrincipal, true );
    defaultPageEntries = domRootNode.GetAttributeValueOrChildValueByName( "defaultPageEntries" ).AsUInt32( defaults.defaultPageEntries, true );
    maxPageEntries = domRootNode.GetAttributeValueOrChildValueByName( "maxPageEntries" ).AsUInt32( defaults.maxPageEntries, true );
    planSampleEntries = domRootNode.GetAttributeValueOrChildValueByName( "planSampleEntries" ).AsUInt32( defaults.planSampleEntries, true );
    maxResourceBytes = domRootNode.GetAttributeValueOrChildValueByName( "maxResourceBytes" ).AsUInt32( defaults.maxResourceBytes, true );
    maxDiffLines = domRootNode.GetAttributeValueOrChildValueByName( "maxDiffLines" ).AsUInt32( defaults.maxDiffLines, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CPipelineOperationSettings::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CPipelineOperationSettings( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPipelineOperationSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORECONVERTER::CPipelineOperationSettings";
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
