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

#include "gucefMcpCi_CCiReleaseStatus.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCiReleaseStatus::CEntry::CEntry( void )
    : image()
    , nameSpace()
    , deployment()
    , container()
    , allowScratch( false )
    , imageReference()
    , digest()
    , imagePullPolicy()
    , revision()
    , changeCause()
    , strategy()
    , replicas()
    , history()
    , problem()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCiReleaseStatus::CCiReleaseStatus( void )
    : ciId()
    , target()
    , entries()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CCiReleaseStatus::SaveToJson( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "ci", ciId );
    node.SetAttribute( "target", target );
    CORE::CDataNode* list = node.AddChild( "entries", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<entries.size(); ++i )
    {
        const CEntry& entry = entries[ i ];
        CORE::CDataNode* item = list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        item->SetAttribute( "image", entry.image );
        item->SetAttribute( "namespace", entry.nameSpace );
        item->SetAttribute( "deployment", entry.deployment );
        item->SetAttribute( "container", entry.container );
        item->SetAttribute( "allowScratch", entry.allowScratch );
        item->SetAttribute( "imageReference", entry.imageReference );
        item->SetAttribute( "digest", entry.digest );
        item->SetAttribute( "imagePullPolicy", entry.imagePullPolicy );
        item->SetAttribute( "revision", entry.revision );
        item->SetAttribute( "changeCause", entry.changeCause );
        item->SetAttribute( "strategy", entry.strategy );
        item->SetAttribute( "replicas", entry.replicas );
        item->SetAttribute( "history", entry.history );
        item->SetAttribute( "problem", entry.problem );
    }
}

/*-------------------------------------------------------------------------*/

void
CCiReleaseStatus::LoadFromJson( const CORE::CDataNode& node )
{GUCEF_TRACE;

    *this = CCiReleaseStatus();
    ciId = node.GetAttributeValue( "ci" ).AsString();
    target = node.GetAttributeValue( "target" ).AsString();
    const CORE::CDataNode* list = node.FindChild( "entries" );
    if ( GUCEF_NULL == list )
        return;
    for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd(); ++i )
    {
        const CORE::CDataNode& item = *(*i);
        CEntry entry;
        entry.image = item.GetAttributeValue( "image" ).AsString();
        entry.nameSpace = item.GetAttributeValue( "namespace" ).AsString();
        entry.deployment = item.GetAttributeValue( "deployment" ).AsString();
        entry.container = item.GetAttributeValue( "container" ).AsString();
        entry.allowScratch = item.GetAttributeValue( "allowScratch" ).AsBool( false );
        entry.imageReference = item.GetAttributeValue( "imageReference" ).AsString();
        entry.digest = item.GetAttributeValue( "digest" ).AsString();
        entry.imagePullPolicy = item.GetAttributeValue( "imagePullPolicy" ).AsString();
        entry.revision = item.GetAttributeValue( "revision" ).AsString();
        entry.changeCause = item.GetAttributeValue( "changeCause" ).AsString();
        entry.strategy = item.GetAttributeValue( "strategy" ).AsString();
        entry.replicas = item.GetAttributeValue( "replicas" ).AsString();
        entry.history = item.GetAttributeValue( "history" ).AsString();
        entry.problem = item.GetAttributeValue( "problem" ).AsString();
        entries.push_back( entry );
    }
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
