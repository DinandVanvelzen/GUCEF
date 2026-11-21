/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CCiImageReport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 MaxImages = 64;
static const UInt32 MaxLogTailChars = 8000;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
Text( const CORE::CDataNode& node, const char* name )
{
    return CCiUtils::Trimmed( node.GetAttributeValue( name ).AsString() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCiImageReport::CImage::CImage( void )
    : name()
    , registryName()
    , tag()
    , alsoTags()
    , digest()
    , status()
    , seconds( 0 )
    , logTail()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CCiImageReport::CImage::IsOk( void ) const
{GUCEF_TRACE;

    return status == "ok" && IsDigest( digest );
}

/*-------------------------------------------------------------------------*/

CCiImageReport::CCiImageReport( void )
    : runId()
    , schema( 1 )
    , run()
    , commit()
    , ref()
    , scratch( true )
    , status()
    , images()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CCiImageReport::IsImagesJob( const CORE::CString& jobId )
{GUCEF_TRACE;

    const CORE::CString lower = CCiUtils::Trimmed( jobId ).Lowercase();
    return lower == "images" || lower.StartsWith( "images " ) || lower.StartsWith( "images-" ) || lower.StartsWith( "images(" );
}

/*-------------------------------------------------------------------------*/

bool
CCiImageReport::IsDigest( const CORE::CString& text )
{GUCEF_TRACE;

    if ( 71 != text.Length() || !text.StartsWith( "sha256:" ) )
        return false;
    const char* c = text.C_String() + 7;
    for ( UInt32 i=0; i<64; ++i )
        if ( !( ( c[ i ] >= '0' && c[ i ] <= '9' ) || ( c[ i ] >= 'a' && c[ i ] <= 'f' ) ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCiImageReport::LoadFromJson( const CORE::CDataNode& node, CORE::CString& problem )
{GUCEF_TRACE;

    *this = CCiImageReport();
    if ( GUCEF_DATATYPE_OBJECT != node.GetNodeType() )
    {
        problem = "the image report is not a JSON object";
        return false;
    }
    runId = Text( node, "runId" );
    schema = node.GetAttributeValue( "schema" ).AsUInt32( 1 );
    run = Text( node, "run" );
    commit = Text( node, "commit" );
    ref = Text( node, "ref" );
    // what does not say it is published is taken as a scratch build
    scratch = node.GetAttributeValue( "scratch" ).AsBool( true );
    status = Text( node, "status" ).Lowercase();
    const CORE::CDataNode* list = node.FindChild( "images" );
    if ( GUCEF_NULL == list )
    {
        problem = "the image report has no images list";
        return false;
    }
    for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd() && images.size() < MaxImages; ++i )
    {
        const CORE::CDataNode& entry = *(*i);
        if ( GUCEF_DATATYPE_OBJECT != entry.GetNodeType() )
            continue;
        CImage image;
        image.name = Text( entry, "name" );
        image.registryName = Text( entry, "registryName" );
        image.tag = Text( entry, "tag" );
        image.alsoTags = CCiUtils::GetStringArray( entry, "alsoTags" );
        image.digest = Text( entry, "digest" );
        image.status = Text( entry, "status" ).Lowercase();
        image.seconds = entry.GetAttributeValue( "seconds" ).AsUInt64( 0 );
        image.logTail = CCiUtils::Bound( entry.GetAttributeValue( "logTail" ).AsString(), MaxLogTailChars );
        if ( image.registryName.IsNULLOrEmpty() )
            image.registryName = image.name;
        if ( !image.name.IsNULLOrEmpty() )
            images.push_back( image );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCiImageReport::SaveToJson( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "schema", schema );
    if ( !runId.IsNULLOrEmpty() )
        node.SetAttribute( "runId", runId );
    node.SetAttribute( "run", run );
    node.SetAttribute( "commit", commit );
    node.SetAttribute( "ref", ref );
    node.SetAttribute( "scratch", scratch );
    node.SetAttribute( "status", status );
    CORE::CDataNode* list = node.AddChild( "images", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<images.size(); ++i )
    {
        const CImage& image = images[ i ];
        CORE::CDataNode* entry = list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        entry->SetAttribute( "name", image.name );
        entry->SetAttribute( "registryName", image.registryName );
        entry->SetAttribute( "tag", image.tag );
        CORE::CDataNode* tags = entry->AddChild( "alsoTags", GUCEF_DATATYPE_ARRAY );
        for ( size_t t=0; t<image.alsoTags.size(); ++t )
            tags->AddValueAsChild( CORE::CVariant( image.alsoTags[ t ] ) );
        entry->SetAttribute( "digest", image.digest );
        entry->SetAttribute( "status", image.status );
        entry->SetAttribute( "seconds", image.seconds );
        entry->SetAttribute( "logTail", image.logTail );
    }
}

/*-------------------------------------------------------------------------*/

const CCiImageReport::CImage*
CCiImageReport::FindImage( const CORE::CString& name ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<images.size(); ++i )
        if ( images[ i ].name == name )
            return &images[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
