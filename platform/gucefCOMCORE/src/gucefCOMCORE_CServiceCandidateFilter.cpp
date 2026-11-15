/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#include "gucefCOMCORE_CServiceCandidateFilter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CServiceCandidateFilter::CServiceCandidateFilter( void )
    : CORE::CIConfigurable()
    , instanceId()
    , protocolMajor()
    , keyFingerprint()
    , requiredProperties()
    , requireEndpoint( true )
{GUCEF_TRACE;

    requiredProperties.SetAllowMultipleValues( false );
}

/*-------------------------------------------------------------------------*/

CServiceCandidateFilter::CServiceCandidateFilter( const CServiceCandidateFilter& src )
    : CORE::CIConfigurable( src )
    , instanceId( src.instanceId )
    , protocolMajor( src.protocolMajor )
    , keyFingerprint( src.keyFingerprint )
    , requiredProperties( src.requiredProperties )
    , requireEndpoint( src.requireEndpoint )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceCandidateFilter::~CServiceCandidateFilter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceCandidateFilter&
CServiceCandidateFilter::operator=( const CServiceCandidateFilter& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        instanceId = src.instanceId;
        protocolMajor = src.protocolMajor;
        keyFingerprint = src.keyFingerprint;
        requiredProperties = src.requiredProperties;
        requireEndpoint = src.requireEndpoint;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CServiceCandidateFilter::IsMatch( const CServiceCandidate& candidate ) const
{GUCEF_TRACE;

    const CServiceAnnouncement& announcement = candidate.announcement;
    if ( !instanceId.IsNULLOrEmpty() && instanceId != announcement.instanceId )
        return false;
    if ( !protocolMajor.IsNULLOrEmpty() && protocolMajor != announcement.GetProtocolMajor() )
        return false;
    if ( !keyFingerprint.IsNULLOrEmpty() && !keyFingerprint.Equals( announcement.keyFingerprint, false ) )
        return false;
    if ( requireEndpoint && announcement.endpoints.empty() )
        return false;
    UInt32 keyCount = requiredProperties.GetKeyCount();
    for ( UInt32 i=0; i<keyCount; ++i )
    {
        CORE::CString key = requiredProperties.GetKey( i ).AsString();
        if ( !announcement.properties.HasKeyAndValue( key, requiredProperties.GetValue( i ).AsString() ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CServiceCandidateFilter::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "instanceId", instanceId );
    config.SetAttribute( "protocolMajor", protocolMajor );
    config.SetAttribute( "keyFingerprint", keyFingerprint );
    config.SetAttribute( "requireEndpoint", requireEndpoint );
    CORE::CDataNode* props = config.AddChild( "requiredProperties", GUCEF_DATATYPE_OBJECT );
    UInt32 keyCount = requiredProperties.GetKeyCount();
    for ( UInt32 i=0; i<keyCount; ++i )
        props->SetAttribute( requiredProperties.GetKey( i ).AsString(), requiredProperties.GetValue( i ).AsString() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CServiceCandidateFilter::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    instanceId = config.GetAttributeValueOrChildValueByName( "instanceId" ).AsString( instanceId, true );
    protocolMajor = config.GetAttributeValueOrChildValueByName( "protocolMajor" ).AsString( protocolMajor, true );
    keyFingerprint = config.GetAttributeValueOrChildValueByName( "keyFingerprint" ).AsString( keyFingerprint, true );
    requireEndpoint = config.GetAttributeValueOrChildValueByName( "requireEndpoint" ).AsBool( requireEndpoint, true );
    const CORE::CDataNode* props = config.FindChild( "requiredProperties" );
    if ( GUCEF_NULL != props )
    {
        requiredProperties.DeleteAll();
        requiredProperties.SetAllowMultipleValues( false );
        for ( CORE::CDataNode::TAttributeMap::const_iterator a = props->AttributeBegin(); a != props->AttributeEnd(); ++a )
            requiredProperties.Set( (*a).first, (*a).second.AsString() );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CServiceCandidateFilter::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CServiceCandidateFilter";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
