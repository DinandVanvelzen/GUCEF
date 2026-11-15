/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETSTRICTCONFIGREADER_H
#include "gucefASSETSTORE_CAssetStrictConfigReader.h"
#define GUCEF_ASSETSTORE_CASSETSTRICTCONFIGREADER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTRICTCONFIGREADER_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#include "gucefASSETSTORE_CAssetRootedPath.h"
#define GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#endif /* GUCEF_ASSETSTORE_CASSETROOTEDPATH_H ? */

#include "gucefASSETSTORE_CAssetToolSetSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetToolSetSettings::SectionName = "AssetToolSet";
const CORE::CString CAssetToolSetSettings::ConfirmAuto = "auto";
const CORE::CString CAssetToolSetSettings::ConfirmAlways = "always";
const CORE::CString CAssetToolSetSettings::ConfirmNever = "never";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetToolSetSettings::CAssetToolSetSettings( void )
    : CORE::CIDataNodeSerializable()
    , name( "assets" )
    , roots()
    , exposureFile()
    , exposureCheckIntervalMs( 2000 )
    , jobStateFile()
    , confirmWrites( ConfirmAuto )
    , defaultLimit( 25 )
    , maxLimit( 100 )
    , maxResultBytes( 32768 )
    , maxResourceBytes( 1048576 )
    , maxUntrustedTextChars( 500 )
    , defaultWaitMs( 20000 )
    , maxWaitMs( 60000 )
    , jobPollIntervalMs( 100 )
    , maxQueuedPerPrincipal( 8 )
    , resultTtlHours( 168 )
    , requestKeyTtlMs( 86400000 )
    , changeTokenTtlMs( 600000 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetToolSetSettings::~CAssetToolSetSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetToolSetSettings::IsValid( CORE::CString& problem ) const
{GUCEF_TRACE;

    if ( !CAssetRootedPath::IsValidRootId( name ) )
    {
        problem = "AssetToolSet: \"name\" must match [a-z][a-z0-9-]{0,31}";
        return false;
    }
    if ( defaultLimit > maxLimit )
    {
        problem = "AssetToolSet: \"defaultLimit\" is above \"maxLimit\"";
        return false;
    }
    if ( defaultWaitMs > maxWaitMs )
    {
        problem = "AssetToolSet: \"defaultWaitMs\" is above \"maxWaitMs\"";
        return false;
    }
    for ( size_t i=0; i<roots.size(); ++i )
        if ( !roots[ i ].IsValid( problem ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolSetSettings::LoadStrict( const CORE::CDataNode& node, CORE::CString& problem )
{GUCEF_TRACE;

    CAssetToolSetSettings loaded( *this );
    CAssetStrictConfigReader reader( node, SectionName );
    reader.ReadString( "name", loaded.name, 32 );
    const CORE::CDataNode* rootList = reader.ReadArray( "roots" );
    if ( GUCEF_NULL != rootList )
    {
        CORE::CDataNode::const_iterator i = rootList->ConstBegin();
        while ( i != rootList->ConstEnd() && !reader.HasProblem() )
        {
            CAssetStrictConfigReader rootReader( *(*i), SectionName + " root" );
            CAssetToolRoot root;
            rootReader.ReadString( "id", root.id, 32 );
            rootReader.ReadString( "vfsPath", root.vfsPath );
            rootReader.ReadString( "diskPath", root.diskPath );
            rootReader.ReadString( "kind", root.kind );
            rootReader.ReadBool( "writable", root.writable );
            CORE::CString rootProblem;
            if ( !rootReader.Finish( rootProblem ) )
                reader.Fail( rootProblem );
            ++i;
        }
        TAssetToolRootVector list;
        if ( !reader.HasProblem() )
        {
            CORE::CString rootProblem;
            if ( !CAssetToolRoot::LoadList( *rootList, list, rootProblem ) )
                reader.Fail( rootProblem );
            else
                loaded.roots = list;
        }
    }
    reader.ReadString( "exposureFile", loaded.exposureFile );
    reader.ReadUInt32( "exposureCheckIntervalMs", 100, 600000, loaded.exposureCheckIntervalMs );
    reader.ReadString( "jobStateFile", loaded.jobStateFile );
    reader.ReadChoice( "confirmWrites", "auto|always|never", loaded.confirmWrites );
    reader.ReadUInt32( "defaultLimit", 1, 100, loaded.defaultLimit );
    reader.ReadUInt32( "maxLimit", 1, 100, loaded.maxLimit );
    reader.ReadUInt32( "maxResultBytes", 4096, 1048576, loaded.maxResultBytes );
    reader.ReadUInt32( "maxResourceBytes", 4096, 16777216, loaded.maxResourceBytes );
    reader.ReadUInt32( "maxUntrustedTextChars", 16, 2000, loaded.maxUntrustedTextChars );
    reader.ReadUInt32( "defaultWaitMs", 0, 600000, loaded.defaultWaitMs );
    reader.ReadUInt32( "maxWaitMs", 0, 600000, loaded.maxWaitMs );
    reader.ReadUInt32( "jobPollIntervalMs", 5, 10000, loaded.jobPollIntervalMs );
    reader.ReadUInt32( "maxQueuedPerPrincipal", 1, 1000, loaded.maxQueuedPerPrincipal );
    reader.ReadUInt32( "resultTtlHours", 1, 8760, loaded.resultTtlHours );
    reader.ReadUInt64( "requestKeyTtlMs", 1000, (UInt64) 2592000000u, loaded.requestKeyTtlMs );
    reader.ReadUInt32( "changeTokenTtlMs", 1000, 86400000, loaded.changeTokenTtlMs );
    if ( !reader.Finish( problem ) )
        return false;
    if ( !loaded.IsValid( problem ) )
        return false;
    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolSetSettings::Serialize( CORE::CDataNode& domRootNode                        ,
                                  const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "name", name ) && success;
    CORE::CDataNode* rootList = domRootNode.AddChild( "roots", GUCEF_DATATYPE_ARRAY );
    CAssetToolRoot::SaveList( roots, *rootList );
    success = domRootNode.SetAttribute( "exposureFile", exposureFile ) && success;
    success = domRootNode.SetAttribute( "exposureCheckIntervalMs", exposureCheckIntervalMs ) && success;
    success = domRootNode.SetAttribute( "jobStateFile", jobStateFile ) && success;
    success = domRootNode.SetAttribute( "confirmWrites", confirmWrites ) && success;
    success = domRootNode.SetAttribute( "defaultLimit", defaultLimit ) && success;
    success = domRootNode.SetAttribute( "maxLimit", maxLimit ) && success;
    success = domRootNode.SetAttribute( "maxResultBytes", maxResultBytes ) && success;
    success = domRootNode.SetAttribute( "maxResourceBytes", maxResourceBytes ) && success;
    success = domRootNode.SetAttribute( "maxUntrustedTextChars", maxUntrustedTextChars ) && success;
    success = domRootNode.SetAttribute( "defaultWaitMs", defaultWaitMs ) && success;
    success = domRootNode.SetAttribute( "maxWaitMs", maxWaitMs ) && success;
    success = domRootNode.SetAttribute( "jobPollIntervalMs", jobPollIntervalMs ) && success;
    success = domRootNode.SetAttribute( "maxQueuedPerPrincipal", maxQueuedPerPrincipal ) && success;
    success = domRootNode.SetAttribute( "resultTtlHours", resultTtlHours ) && success;
    success = domRootNode.SetAttribute( "requestKeyTtlMs", requestKeyTtlMs ) && success;
    success = domRootNode.SetAttribute( "changeTokenTtlMs", changeTokenTtlMs ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolSetSettings::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                    const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    name = domRootNode.GetAttributeValueOrChildValueByName( "name" ).AsString( name, true );
    const CORE::CDataNode* rootList = domRootNode.FindChild( "roots" );
    if ( GUCEF_NULL != rootList )
    {
        CORE::CString problem;
        TAssetToolRootVector list;
        if ( CAssetToolRoot::LoadList( *rootList, list, problem ) )
            roots = list;
    }
    exposureFile = domRootNode.GetAttributeValueOrChildValueByName( "exposureFile" ).AsString( exposureFile, true );
    exposureCheckIntervalMs = domRootNode.GetAttributeValueOrChildValueByName( "exposureCheckIntervalMs" ).AsUInt32( exposureCheckIntervalMs, true );
    jobStateFile = domRootNode.GetAttributeValueOrChildValueByName( "jobStateFile" ).AsString( jobStateFile, true );
    confirmWrites = domRootNode.GetAttributeValueOrChildValueByName( "confirmWrites" ).AsString( confirmWrites, true );
    defaultLimit = domRootNode.GetAttributeValueOrChildValueByName( "defaultLimit" ).AsUInt32( defaultLimit, true );
    maxLimit = domRootNode.GetAttributeValueOrChildValueByName( "maxLimit" ).AsUInt32( maxLimit, true );
    maxResultBytes = domRootNode.GetAttributeValueOrChildValueByName( "maxResultBytes" ).AsUInt32( maxResultBytes, true );
    maxResourceBytes = domRootNode.GetAttributeValueOrChildValueByName( "maxResourceBytes" ).AsUInt32( maxResourceBytes, true );
    maxUntrustedTextChars = domRootNode.GetAttributeValueOrChildValueByName( "maxUntrustedTextChars" ).AsUInt32( maxUntrustedTextChars, true );
    defaultWaitMs = domRootNode.GetAttributeValueOrChildValueByName( "defaultWaitMs" ).AsUInt32( defaultWaitMs, true );
    maxWaitMs = domRootNode.GetAttributeValueOrChildValueByName( "maxWaitMs" ).AsUInt32( maxWaitMs, true );
    jobPollIntervalMs = domRootNode.GetAttributeValueOrChildValueByName( "jobPollIntervalMs" ).AsUInt32( jobPollIntervalMs, true );
    maxQueuedPerPrincipal = domRootNode.GetAttributeValueOrChildValueByName( "maxQueuedPerPrincipal" ).AsUInt32( maxQueuedPerPrincipal, true );
    resultTtlHours = domRootNode.GetAttributeValueOrChildValueByName( "resultTtlHours" ).AsUInt32( resultTtlHours, true );
    requestKeyTtlMs = domRootNode.GetAttributeValueOrChildValueByName( "requestKeyTtlMs" ).AsUInt64( requestKeyTtlMs, true );
    changeTokenTtlMs = domRootNode.GetAttributeValueOrChildValueByName( "changeTokenTtlMs" ).AsUInt32( changeTokenTtlMs, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetToolSetSettings::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetToolSetSettings( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetToolSetSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetToolSetSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
