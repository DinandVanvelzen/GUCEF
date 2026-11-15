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

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTRICTCONFIGREADER_H
#include "gucefASSETSTORE_CAssetStrictConfigReader.h"
#define GUCEF_ASSETSTORE_CASSETSTRICTCONFIGREADER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTRICTCONFIGREADER_H ? */

#include "gucefASSETSTORE_CAssetToolsExposure.h"

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

const CORE::CString CAssetToolsExposure::FormatName = "GUCEF.AssetTools.McpExposure";
const CORE::CString CAssetToolsExposure::GroupRead = "read";
const CORE::CString CAssetToolsExposure::GroupConvert = "convert";
const CORE::CString CAssetToolsExposure::GroupAuthor = "author";
const CORE::CString CAssetToolsExposure::GroupStore = "store";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
IsInList( const CORE::CString::StringVector& list, const CORE::CString& name )
{GUCEF_TRACE;

    for ( size_t i=0; i<list.size(); ++i )
        if ( list[ i ] == name )
            return true;
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetToolsExposure::CAssetToolsExposure( void )
    : mcpEnabled( true )
    , convertEnabled( false )
    , authorEnabled( false )
    , storeEnabled( false )
    , allowedWriteTools()
    , deniedTools()
    , exposePurchaseDetails( false )
    , maxWritesPerHour( 60 )
    , maxJobsPerDay( 50 )
    , maxRows( 100 )
    , maxUntrustedTextChars( 500 )
    , origin( "defaults" )
    , problem()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetToolsExposure::IsGroupEnabled( const CORE::CString& group ) const
{GUCEF_TRACE;

    if ( GroupRead == group )
        return true;
    if ( GroupConvert == group )
        return convertEnabled;
    if ( GroupAuthor == group )
        return authorEnabled;
    if ( GroupStore == group )
        return storeEnabled;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolsExposure::IsDenied( const CORE::CString& toolName ) const
{GUCEF_TRACE;

    return IsInList( deniedTools, toolName );
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolsExposure::IsAllowedWriteTool( const CORE::CString& toolName ) const
{GUCEF_TRACE;

    return IsInList( allowedWriteTools, toolName );
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolsExposure::LoadStrict( const CORE::CDataNode& node, CORE::CString& problemText )
{GUCEF_TRACE;

    CAssetToolsExposure loaded;
    CAssetStrictConfigReader reader( node, "the exposure file" );
    CORE::CString format;
    reader.ReadString( "format", format );
    UInt32 version = 0;
    reader.ReadUInt32( "version", 0, 1000, version );
    if ( !reader.HasProblem() && FormatName != format )
        reader.Fail( "\"format\" must be \"" + FormatName + "\"" );
    if ( !reader.HasProblem() && FormatVersion != version )
        reader.Fail( "\"version\" must be 1" );
    reader.ReadBool( "mcpEnabled", loaded.mcpEnabled );
    const CORE::CDataNode* groups = reader.ReadObject( "toolGroups" );
    if ( GUCEF_NULL != groups && !reader.HasProblem() )
    {
        CAssetStrictConfigReader groupReader( *groups, "toolGroups" );
        groupReader.ReadBool( GroupConvert, loaded.convertEnabled );
        groupReader.ReadBool( GroupAuthor, loaded.authorEnabled );
        groupReader.ReadBool( GroupStore, loaded.storeEnabled );
        CORE::CString groupProblem;
        if ( !groupReader.Finish( groupProblem ) )
            reader.Fail( groupProblem );
    }
    reader.ReadStringList( "allowedWriteTools", loaded.allowedWriteTools, 256 );
    reader.ReadStringList( "deniedTools", loaded.deniedTools, 256 );
    reader.ReadBool( "exposePurchaseDetails", loaded.exposePurchaseDetails );
    reader.ReadUInt32( "maxWritesPerHour", 0, HardMaxWritesPerHour, loaded.maxWritesPerHour );
    reader.ReadUInt32( "maxJobsPerDay", 0, HardMaxJobsPerDay, loaded.maxJobsPerDay );
    reader.ReadUInt32( "maxRows", 1, HardMaxRows, loaded.maxRows );
    reader.ReadUInt32( "maxUntrustedTextChars", 16, HardMaxUntrustedTextChars, loaded.maxUntrustedTextChars );
    if ( !reader.Finish( problemText ) )
        return false;
    loaded.origin = "file";
    loaded.problem.Clear();
    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolsExposure::LoadFromFile( const CORE::CString& path, CORE::CString& problemText, bool& missing )
{GUCEF_TRACE;

    missing = false;
    if ( path.IsNULLOrEmpty() || !CORE::FileExists( path ) )
    {
        missing = true;
        problemText = "there is no exposure file";
        return false;
    }
    CORE::CDynamicBuffer content;
    if ( !content.LoadContentFromFile( path, 0, 1048576 ) )
    {
        problemText = "the exposure file cannot be read";
        return false;
    }
    CORE::CString text;
    text.Set( (const char*) content.GetConstBufferPtr(), content.GetDataSize() );
    CORE::CDataNode root;
    if ( !COM::CJsonRpcEndpoint::ParseJsonText( text, root ) )
    {
        problemText = "the exposure file is no JSON object";
        return false;
    }
    return LoadStrict( root, problemText );
}

/*-------------------------------------------------------------------------*/

void
CAssetToolsExposure::ToDataNode( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.Clear();
    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "mcpEnabled", mcpEnabled );
    CORE::CDataNode* groups = node.AddChild( "toolGroups", GUCEF_DATATYPE_OBJECT );
    groups->SetAttribute( GroupRead, true );
    groups->SetAttribute( GroupConvert, convertEnabled );
    groups->SetAttribute( GroupAuthor, authorEnabled );
    groups->SetAttribute( GroupStore, storeEnabled );
    node.SetValuesOfChildByName( "allowedWriteTools", allowedWriteTools );
    node.SetValuesOfChildByName( "deniedTools", deniedTools );
    node.SetAttribute( "exposePurchaseDetails", exposePurchaseDetails );
    node.SetAttribute( "maxWritesPerHour", maxWritesPerHour );
    node.SetAttribute( "maxJobsPerDay", maxJobsPerDay );
    node.SetAttribute( "maxRows", maxRows );
    node.SetAttribute( "maxUntrustedTextChars", maxUntrustedTextChars );
    node.SetAttribute( "origin", origin );
    if ( problem.IsNULLOrEmpty() )
        node.SetAttribute( "problem", CORE::CVariant() );
    else
        node.SetAttribute( "problem", problem );
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolsExposure::operator==( const CAssetToolsExposure& other ) const
{GUCEF_TRACE;

    return mcpEnabled == other.mcpEnabled && convertEnabled == other.convertEnabled && authorEnabled == other.authorEnabled &&
           storeEnabled == other.storeEnabled && allowedWriteTools == other.allowedWriteTools && deniedTools == other.deniedTools &&
           exposePurchaseDetails == other.exposePurchaseDetails && maxWritesPerHour == other.maxWritesPerHour &&
           maxJobsPerDay == other.maxJobsPerDay && maxRows == other.maxRows && maxUntrustedTextChars == other.maxUntrustedTextChars &&
           origin == other.origin && problem == other.problem;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolsExposure::operator!=( const CAssetToolsExposure& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
