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

#include "gucefMcpCi_CCiDeployResult.h"

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

CCiDeployResult::CCiDeployResult( void )
    : action()
    , ciId()
    , image()
    , target()
    , nameSpace()
    , deployment()
    , container()
    , runId()
    , commit()
    , digest()
    , imageReference()
    , previousImage()
    , previousDigest()
    , revision()
    , replicas()
    , strategy()
    , outcome()
    , code()
    , message()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CCiDeployResult::IsRefused( void ) const
{GUCEF_TRACE;

    return outcome == "refused";
}

/*-------------------------------------------------------------------------*/

bool
CCiDeployResult::IsSuccess( void ) const
{GUCEF_TRACE;

    return outcome == "deployed" || outcome == "unchanged" || outcome == "rolled_back";
}

/*-------------------------------------------------------------------------*/

void
CCiDeployResult::SaveToJson( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "action", action );
    node.SetAttribute( "ci", ciId );
    node.SetAttribute( "image", image );
    node.SetAttribute( "target", target );
    node.SetAttribute( "namespace", nameSpace );
    node.SetAttribute( "deployment", deployment );
    node.SetAttribute( "container", container );
    node.SetAttribute( "run", runId );
    node.SetAttribute( "commit", commit );
    node.SetAttribute( "digest", digest );
    node.SetAttribute( "imageReference", imageReference );
    node.SetAttribute( "previousImage", previousImage );
    node.SetAttribute( "oldDigest", previousDigest );
    node.SetAttribute( "revision", revision );
    node.SetAttribute( "replicas", replicas );
    node.SetAttribute( "strategy", strategy );
    node.SetAttribute( "result", outcome );
    node.SetAttribute( "code", code );
    node.SetAttribute( "reason", message );
}

/*-------------------------------------------------------------------------*/

void
CCiDeployResult::LoadFromJson( const CORE::CDataNode& node )
{GUCEF_TRACE;

    action = node.GetAttributeValue( "action" ).AsString();
    ciId = node.GetAttributeValue( "ci" ).AsString();
    image = node.GetAttributeValue( "image" ).AsString();
    target = node.GetAttributeValue( "target" ).AsString();
    nameSpace = node.GetAttributeValue( "namespace" ).AsString();
    deployment = node.GetAttributeValue( "deployment" ).AsString();
    container = node.GetAttributeValue( "container" ).AsString();
    runId = node.GetAttributeValue( "run" ).AsString();
    commit = node.GetAttributeValue( "commit" ).AsString();
    digest = node.GetAttributeValue( "digest" ).AsString();
    imageReference = node.GetAttributeValue( "imageReference" ).AsString();
    previousImage = node.GetAttributeValue( "previousImage" ).AsString();
    previousDigest = node.GetAttributeValue( "oldDigest" ).AsString();
    revision = node.GetAttributeValue( "revision" ).AsString();
    replicas = node.GetAttributeValue( "replicas" ).AsString();
    strategy = node.GetAttributeValue( "strategy" ).AsString();
    outcome = node.GetAttributeValue( "result" ).AsString();
    code = node.GetAttributeValue( "code" ).AsString();
    message = node.GetAttributeValue( "reason" ).AsString();
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
