/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefDRNUTILS_CRemoteShareMountSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CRemoteShareMountSettings::AutoChannel = GUCEF_UINT32MAX;

/*-------------------------------------------------------------------------*/

CRemoteShareMountSettings::CRemoteShareMountSettings( void )
    : CORE::CIConfigurable()
    , service()
    , share()
    , mountPoint()
    , channel( AutoChannel )
    , bridgeSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteShareMountSettings::CRemoteShareMountSettings( const CRemoteShareMountSettings& src )
    : CORE::CIConfigurable( src )
    , service( src.service )
    , share( src.share )
    , mountPoint( src.mountPoint )
    , channel( src.channel )
    , bridgeSettings( src.bridgeSettings )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteShareMountSettings::~CRemoteShareMountSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteShareMountSettings&
CRemoteShareMountSettings::operator=( const CRemoteShareMountSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        service = src.service;
        share = src.share;
        mountPoint = src.mountPoint;
        channel = src.channel;
        bridgeSettings = src.bridgeSettings;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteShareMountSettings::IsValid( CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( share.IsNULLOrEmpty() )
    {
        reason = "no share is named";
        return false;
    }
    if ( mountPoint.IsNULLOrEmpty() )
    {
        reason = "no mount point is given";
        return false;
    }
    if ( bridgeSettings.GetNrOfChildNodes() > 0 )
    {
        reason = "the bridge settings are attributes, not children";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteShareMountSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "service", service );
    config.SetAttribute( "share", share );
    config.SetAttribute( "mountPoint", mountPoint );
    if ( AutoChannel == channel )
        config.SetAttribute( "channel", CORE::CString( "auto" ) );
    else
        config.SetAttribute( "channel", channel );
    CORE::CDataNode* bridge = config.AddChild( "BridgeSettings", GUCEF_DATATYPE_OBJECT );
    for ( CORE::CDataNode::TAttributeMap::const_iterator a = bridgeSettings.AttributeBegin(); a != bridgeSettings.AttributeEnd(); ++a )
        bridge->SetAttribute( (*a).first, (*a).second.AsString() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteShareMountSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    // a key that is not a setting is a mistake of the configuration: refused, nothing is taken
    for ( CORE::CDataNode::TAttributeMap::const_iterator a = config.AttributeBegin(); a != config.AttributeEnd(); ++a )
    {
        const CORE::CString& key = (*a).first;
        if ( key != "service" && key != "share" && key != "mountPoint" && key != "channel" )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteShareMountSettings: the setting \"" << key << "\" is not known" );
            return false;
        }
    }
    for ( CORE::CDataNode::const_iterator c = config.ConstBegin(); c != config.ConstEnd(); ++c )
    {
        if ( (*c)->GetName() != "BridgeSettings" )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteShareMountSettings: the setting \"" << (*c)->GetName() << "\" is not known" );
            return false;
        }
        if ( (*c)->GetNrOfChildNodes() > 0 )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteShareMountSettings: the BridgeSettings are attributes, they have no children" );
            return false;
        }
    }

    UInt32 loadedChannel = channel;
    const CORE::CVariant channelValue = config.GetAttributeValueOrChildValueByName( "channel" );
    if ( !channelValue.IsNULLOrEmpty() )
    {
        const CORE::CString text = channelValue.AsString().Trim( true ).Trim( false );
        if ( text.Equals( "auto", false ) )
            loadedChannel = AutoChannel;
        else
        {
            // a number: all digits
            bool digits = !text.IsNULLOrEmpty();
            for ( UInt32 i=0; i<text.Length() && digits; ++i )
                digits = text[ i ] >= '0' && text[ i ] <= '9';
            if ( !digits || text.Length() > 4 )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteShareMountSettings: the channel \"" << text << "\" is neither \"auto\" nor the index of a channel" );
                return false;
            }
            loadedChannel = CORE::StringToUInt32( text );
        }
    }

    service = config.GetAttributeValueOrChildValueByName( "service" ).AsString( service, true );
    share = config.GetAttributeValueOrChildValueByName( "share" ).AsString( share, true );
    mountPoint = config.GetAttributeValueOrChildValueByName( "mountPoint" ).AsString( mountPoint, true );
    channel = loadedChannel;
    const CORE::CDataNode* bridge = config.FindChild( "BridgeSettings" );
    if ( GUCEF_NULL != bridge )
    {
        bridgeSettings.ClearAttributes();
        for ( CORE::CDataNode::TAttributeMap::const_iterator a = bridge->AttributeBegin(); a != bridge->AttributeEnd(); ++a )
            bridgeSettings.SetAttribute( (*a).first, (*a).second.AsString() );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteShareMountSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRNUTILS::CRemoteShareMountSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
