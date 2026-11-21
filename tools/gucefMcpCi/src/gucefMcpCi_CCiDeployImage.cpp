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
#include "gucefMcpCi_CCiDeployImage.h"

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

static const char* DefaultRegistryOwner = "ci";
static const UInt32 MaxKubernetesNameLength = 253;
static const UInt32 MaxImageNameLength = 200;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
Attribute( const CORE::CDataNode& node, const char* name )
{
    return CCiUtils::Trimmed( node.GetAttributeValue( name ).AsString() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCiDeployImage::CTarget::CTarget( void )
    : name()
    , nameSpace()
    , deployment()
    , container()
    , allowScratch( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiDeployImage::CTarget::GetKey( void ) const
{GUCEF_TRACE;

    return nameSpace + "/" + deployment;
}

/*-------------------------------------------------------------------------*/

CCiDeployImage::CCiDeployImage( void )
    : name()
    , registryName()
    , registryOwner( DefaultRegistryOwner )
    , targets()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CCiDeployImage::IsKubernetesName( const CORE::CString& text )
{GUCEF_TRACE;

    const UInt32 length = text.Length();
    if ( 0 == length || length > MaxKubernetesNameLength )
        return false;
    const char* c = text.C_String();
    for ( UInt32 i=0; i<length; ++i )
    {
        const char ch = c[ i ];
        const bool alphaNumeric = ( ch >= 'a' && ch <= 'z' ) || ( ch >= '0' && ch <= '9' );
        if ( alphaNumeric )
            continue;
        if ( ( '-' == ch || '.' == ch ) && i > 0 && i+1 < length )
            continue;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCiDeployImage::IsImageName( const CORE::CString& text )
{GUCEF_TRACE;

    const UInt32 length = text.Length();
    if ( 0 == length || length > MaxImageNameLength )
        return false;
    const char* c = text.C_String();
    for ( UInt32 i=0; i<length; ++i )
    {
        const char ch = c[ i ];
        const bool ok = ( ch >= 'a' && ch <= 'z' ) || ( ch >= '0' && ch <= '9' ) || '-' == ch || '_' == ch || '.' == ch || '/' == ch;
        if ( !ok )
            return false;
    }
    return '/' != c[ 0 ] && '/' != c[ length-1 ] && '.' != c[ 0 ] && text.HasSubstr( ".." ) < 0 && text.HasSubstr( "//" ) < 0;
}

/*-------------------------------------------------------------------------*/

bool
CCiDeployImage::LoadConfig( const CORE::CDataNode& node, const CORE::CString& ciId, CORE::CString& problem )
{GUCEF_TRACE;

    name = Attribute( node, "name" );
    registryName = Attribute( node, "registryName" );
    registryOwner = Attribute( node, "registryOwner" );
    if ( registryName.IsNULLOrEmpty() )
        registryName = name;
    if ( registryOwner.IsNULLOrEmpty() )
        registryOwner = DefaultRegistryOwner;
    targets.clear();

    const CORE::CString where = "the deploy block of \"" + ciId + "\"";
    if ( !IsImageName( name ) )
    {
        problem = where + " has an image without a valid name (lower case letters, digits, - _ . /): \"" + name + "\"";
        return false;
    }
    if ( !IsImageName( registryName ) || !IsImageName( registryOwner ) )
    {
        problem = where + ": the image \"" + name + "\" has a registryName or registryOwner that is not a valid name";
        return false;
    }

    const CORE::CDataNode* targetsNode = node.FindChild( "targets" );
    if ( GUCEF_NULL == targetsNode || GUCEF_DATATYPE_OBJECT != targetsNode->GetNodeType() || 0 == targetsNode->GetNrOfChildNodes() )
    {
        problem = where + ": the image \"" + name + "\" has no targets object";
        return false;
    }
    for ( CORE::CDataNode::const_iterator i = targetsNode->ConstBegin(); i != targetsNode->ConstEnd(); ++i )
    {
        const CORE::CDataNode& entry = *(*i);
        CTarget target;
        target.name = CCiUtils::Trimmed( entry.GetName() );
        if ( target.name.IsNULLOrEmpty() || GUCEF_DATATYPE_OBJECT != entry.GetNodeType() )
        {
            problem = where + ": the image \"" + name + "\" has a target that is not an object with a name";
            return false;
        }
        target.nameSpace = Attribute( entry, "namespace" );
        target.deployment = Attribute( entry, "deployment" );
        target.container = Attribute( entry, "container" );
        target.allowScratch = entry.GetAttributeValue( "allowScratch" ).AsBool( false );
        const char* missing = GUCEF_NULL;
        if ( target.nameSpace.IsNULLOrEmpty() )
            missing = "namespace";
        else
        if ( target.deployment.IsNULLOrEmpty() )
            missing = "deployment";
        else
        if ( target.container.IsNULLOrEmpty() )
            missing = "container";
        if ( GUCEF_NULL != missing )
        {
            problem = where + ": the target \"" + target.name + "\" of the image \"" + name + "\" has no " + missing;
            return false;
        }
        if ( !IsKubernetesName( target.nameSpace ) || !IsKubernetesName( target.deployment ) || !IsKubernetesName( target.container ) )
        {
            problem = where + ": the target \"" + target.name + "\" of the image \"" + name + "\" has a namespace, deployment or container that is "
                      "not a Kubernetes name (lower case letters, digits, - and .)";
            return false;
        }
        if ( GUCEF_NULL != FindTarget( target.name ) )
        {
            problem = where + ": the image \"" + name + "\" names the target \"" + target.name + "\" twice";
            return false;
        }
        targets.push_back( target );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CCiDeployImage::CTarget*
CCiDeployImage::FindTarget( const CORE::CString& targetName ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<targets.size(); ++i )
        if ( targets[ i ].name == targetName )
            return &targets[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiDeployImage::ListTargets( void ) const
{GUCEF_TRACE;

    CORE::CString::StringVector names;
    for ( size_t i=0; i<targets.size(); ++i )
        names.push_back( targets[ i ].name );
    return CCiUtils::Join( names, ", " );
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
