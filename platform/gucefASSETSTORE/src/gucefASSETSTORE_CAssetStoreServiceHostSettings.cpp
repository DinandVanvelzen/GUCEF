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

#include "gucefASSETSTORE_CAssetStoreServiceHostSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const KnownKeys[] = { "roles", "signInRoles", "learningRoles", "stagingDirectory", "stagingVfsRoot", "maxUploadBytesPerSubject", "maxUploadBytesTotal",
                                  "maxUploadsPerSubject", "uploadExpiryHours", "uploadLocationId", "commandJournalDirectory",
                                  "commandJournalKeepDays", "eventBarrierMs", "jobProgressIntervalMs", "maxReadBlockBytes" };

bool
IsKnownKey( const CORE::CString& key )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<sizeof( KnownKeys ) / sizeof( KnownKeys[ 0 ] ); ++i )
        if ( key == KnownKeys[ i ] )
            return true;
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetStoreServiceHostSettings::CAssetStoreServiceHostSettings( void )
    : CORE::CIConfigurable()
    , roles()
    , signInRoles()
    , learningRoles()
    , stagingDirectory()
    , stagingVfsRoot( "assetstore-service/staging" )
    , maxUploadBytesPerSubject( 50ULL * 1024ULL * 1024ULL * 1024ULL )
    , maxUploadBytesTotal( 0 )
    , maxUploadsPerSubject( 2 )
    , uploadExpiryHours( 48 )
    , uploadLocationId()
    , commandJournalDirectory()
    , commandJournalKeepDays( 7 )
    , eventBarrierMs( 1000 )
    , jobProgressIntervalMs( 500 )
    , maxReadBlockBytes( 262144 )
{GUCEF_TRACE;

    roles.insert( "owner" );
    signInRoles.insert( "owner" );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreServiceHostSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    bool success = true;
    success = config.SetValuesOfChildByName( "roles", roles ) && success;
    success = config.SetValuesOfChildByName( "signInRoles", signInRoles ) && success;
    success = config.SetValuesOfChildByName( "learningRoles", learningRoles ) && success;
    success = config.SetAttribute( "stagingDirectory", stagingDirectory ) && success;
    success = config.SetAttribute( "stagingVfsRoot", stagingVfsRoot ) && success;
    success = config.SetAttribute( "maxUploadBytesPerSubject", maxUploadBytesPerSubject ) && success;
    success = config.SetAttribute( "maxUploadBytesTotal", maxUploadBytesTotal ) && success;
    success = config.SetAttribute( "maxUploadsPerSubject", maxUploadsPerSubject ) && success;
    success = config.SetAttribute( "uploadExpiryHours", uploadExpiryHours ) && success;
    success = config.SetAttribute( "uploadLocationId", uploadLocationId ) && success;
    success = config.SetAttribute( "commandJournalDirectory", commandJournalDirectory ) && success;
    success = config.SetAttribute( "commandJournalKeepDays", commandJournalKeepDays ) && success;
    success = config.SetAttribute( "eventBarrierMs", eventBarrierMs ) && success;
    success = config.SetAttribute( "jobProgressIntervalMs", jobProgressIntervalMs ) && success;
    success = config.SetAttribute( "maxReadBlockBytes", maxReadBlockBytes ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreServiceHostSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    for ( CORE::CDataNode::TAttributeMap::const_iterator a = config.AttributeBegin(); a != config.AttributeEnd(); ++a )
    {
        if ( !IsKnownKey( (*a).first ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreServiceHostSettings: the setting \"" << (*a).first << "\" is not known" );
            return false;
        }
    }
    for ( CORE::CDataNode::const_iterator c = config.ConstBegin(); c != config.ConstEnd(); ++c )
    {
        if ( !IsKnownKey( (*c)->GetName() ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreServiceHostSettings: the setting \"" << (*c)->GetName() << "\" is not known" );
            return false;
        }
    }
    CORE::CString::StringVector roleList;
    if ( config.GetValuesOfChildByName( "roles", roleList ) || GUCEF_NULL != config.GetAttribute( "roles" ) )
    {
        if ( roleList.empty() )
            roleList = config.GetAttributeValueOrChildValueByName( "roles" ).AsString().ParseElements( ',', false );
        roles.clear();
        for ( size_t i=0; i<roleList.size(); ++i )
            if ( !roleList[ i ].Trim( true ).IsNULLOrEmpty() )
                roles.insert( roleList[ i ].Trim( true ) );
    }
    CORE::CString::StringVector signInList;
    if ( config.GetValuesOfChildByName( "signInRoles", signInList ) || GUCEF_NULL != config.GetAttribute( "signInRoles" ) )
    {
        if ( signInList.empty() )
            signInList = config.GetAttributeValueOrChildValueByName( "signInRoles" ).AsString().ParseElements( ',', false );
        CORE::CString::StringSet wanted;
        for ( size_t i=0; i<signInList.size(); ++i )
            if ( !signInList[ i ].Trim( true ).IsNULLOrEmpty() )
                wanted.insert( signInList[ i ].Trim( true ) );
        if ( wanted.find( "mcp" ) != wanted.end() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreServiceHostSettings: signInRoles may not hold the role \"mcp\": a machine caller never drives a sign in or the keyring" );
            return false;
        }
        signInRoles = wanted;
    }
    CORE::CString::StringVector learningList;
    if ( config.GetValuesOfChildByName( "learningRoles", learningList ) || GUCEF_NULL != config.GetAttribute( "learningRoles" ) )
    {
        if ( learningList.empty() )
            learningList = config.GetAttributeValueOrChildValueByName( "learningRoles" ).AsString().ParseElements( ',', false );
        learningRoles.clear();
        for ( size_t i=0; i<learningList.size(); ++i )
            if ( !learningList[ i ].Trim( true ).IsNULLOrEmpty() )
                learningRoles.insert( learningList[ i ].Trim( true ) );
    }
    stagingDirectory = config.GetAttributeValueOrChildValueByName( "stagingDirectory" ).AsString( stagingDirectory, true );
    stagingVfsRoot = config.GetAttributeValueOrChildValueByName( "stagingVfsRoot" ).AsString( stagingVfsRoot, true );
    maxUploadBytesPerSubject = config.GetAttributeValueOrChildValueByName( "maxUploadBytesPerSubject" ).AsUInt64( maxUploadBytesPerSubject, true );
    maxUploadBytesTotal = config.GetAttributeValueOrChildValueByName( "maxUploadBytesTotal" ).AsUInt64( maxUploadBytesTotal, true );
    maxUploadsPerSubject = config.GetAttributeValueOrChildValueByName( "maxUploadsPerSubject" ).AsUInt32( maxUploadsPerSubject, true );
    uploadExpiryHours = config.GetAttributeValueOrChildValueByName( "uploadExpiryHours" ).AsUInt32( uploadExpiryHours, true );
    uploadLocationId = config.GetAttributeValueOrChildValueByName( "uploadLocationId" ).AsString( uploadLocationId, true );
    commandJournalDirectory = config.GetAttributeValueOrChildValueByName( "commandJournalDirectory" ).AsString( commandJournalDirectory, true );
    commandJournalKeepDays = config.GetAttributeValueOrChildValueByName( "commandJournalKeepDays" ).AsUInt32( commandJournalKeepDays, true );
    eventBarrierMs = config.GetAttributeValueOrChildValueByName( "eventBarrierMs" ).AsUInt32( eventBarrierMs, true );
    jobProgressIntervalMs = config.GetAttributeValueOrChildValueByName( "jobProgressIntervalMs" ).AsUInt32( jobProgressIntervalMs, true );
    maxReadBlockBytes = config.GetAttributeValueOrChildValueByName( "maxReadBlockBytes" ).AsUInt32( maxReadBlockBytes, true );
    if ( 0 == maxReadBlockBytes || maxReadBlockBytes > 1048576 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreServiceHostSettings: maxReadBlockBytes must be 1 to 1048576" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreServiceHostSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStoreServiceHostSettings";
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
