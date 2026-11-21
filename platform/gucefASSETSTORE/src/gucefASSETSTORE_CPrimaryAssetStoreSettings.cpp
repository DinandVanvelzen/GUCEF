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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */
#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */
#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */
#include "gucefASSETSTORE_CPrimaryAssetStoreSettings.h"

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

const CORE::CString CPrimaryAssetStoreSettings::DefaultLocationId = "default";
const CORE::CString CPrimaryAssetStoreSettings::DiskSourceId = "disk";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPrimaryAssetStoreSettings::CPrimaryAssetStoreSettings( void )
    : CORE::CIDataNodeSerializable()
    , database()
    , credentialProvider()
    , credentialAccount()
    , databasePasswordFile()
    , locations()
    , defaultLocationId( DefaultLocationId )
    , sources()
    , defaultPolicy()
    , duplicatePolicy( ASSETDUPLICATE_SKIP )
    , copyDiskPacks( true )
    , scratchStagingPath()
    , seedDataPath( "gucefASSETSTORE" )
    , analysisConcurrency( 4 )
    , indexAfterIngest( false )
    , threadPoolName( "AssetStorePrimary" )
    , analysisThreadPoolName( "AssetStoreAnalysis" )
{GUCEF_TRACE;

    CAssetStorageLocation location;
    location.locationId = DefaultLocationId;
    location.name = "Default";
    location.vfsRoot = "assetstore/default";
    location.fileSystemPath = "$MODULEDIR$/../../assets/";
    location.writeable = true;
    locations.push_back( location );

    database.backendName = "vfstables";
    database.database = "assetstore/default/catalogue";

    defaultPolicy.sourceId = "*";
    defaultPolicy.mode = ASSETINGEST_MODE_MANUAL;
    defaultPolicy.keep = ASSETKEEP_BOTH;
}

/*-------------------------------------------------------------------------*/

CPrimaryAssetStoreSettings::~CPrimaryAssetStoreSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CAssetStorageLocation*
CPrimaryAssetStoreSettings::FindLocation( const CORE::CString& locationId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<locations.size(); ++i )
        if ( locations[ i ].locationId == locationId )
            return &locations[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CAssetSourcePolicy
CPrimaryAssetStoreSettings::GetPolicy( const CORE::CString& sourceId ) const
{GUCEF_TRACE;

    const CORE::CString id = sourceId.Lowercase();
    for ( size_t i=0; i<sources.size(); ++i )
        if ( sources[ i ].sourceId == id )
            return sources[ i ];
    CAssetSourcePolicy policy( defaultPolicy );
    policy.sourceId = id;
    return policy;
}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStoreSettings::SetPolicy( const CAssetSourcePolicy& policy )
{GUCEF_TRACE;

    for ( size_t i=0; i<sources.size(); ++i )
    {
        if ( sources[ i ].sourceId == policy.sourceId.Lowercase() )
        {
            sources[ i ] = policy;
            sources[ i ].sourceId = policy.sourceId.Lowercase();
            return;
        }
    }
    sources.push_back( policy );
    sources.back().sourceId = policy.sourceId.Lowercase();
}

/*-------------------------------------------------------------------------*/

bool
CPrimaryAssetStoreSettings::GetDatabaseSettingsWithCredentials( AUTH::ICredentialStore* credentialStore      ,
                                                                COMCORE::CDatabaseClientSettings& settings   ,
                                                                CORE::CString& error                         ) const
{GUCEF_TRACE;

    settings = database;
    if ( !credentialProvider.IsNULLOrEmpty() && !credentialAccount.IsNULLOrEmpty() )
    {
        if ( GUCEF_NULL == credentialStore )
        {
            error = "the password of the catalogue is in a credential store and none was given";
            return false;
        }
        AUTH::CSecretString secret;
        if ( !credentialStore->Load( credentialProvider, credentialAccount, secret ) || secret.IsEmpty() )
        {
            error = "the credential store has no password for the catalogue";
            return false;
        }
        settings.backendOptions.SetAllowMultipleValues( false );
        settings.backendOptions.Set( "password", CORE::CVariant( secret.Reveal() ) );
        secret.Clear();
        return true;
    }
    if ( !databasePasswordFile.IsNULLOrEmpty() )
    {
        const CORE::CString path = CORE::RelativePath( databasePasswordFile );
        CORE::CDynamicBuffer content;
        if ( !content.LoadContentFromFile( path ) )
        {
            error = "the password file of the catalogue cannot be read";
            return false;
        }
        CORE::CString text;
        text.Set( content.AsConstTypePtr< char >(), content.GetDataSize() );
        AUTH::CSecretString secret( text.SubstrToChar( '\n', true ).RemoveChar( '\r' ) );
        text.Clear();
        if ( secret.IsEmpty() )
        {
            error = "the password file of the catalogue is empty";
            return false;
        }
        settings.backendOptions.SetAllowMultipleValues( false );
        settings.backendOptions.Set( "password", CORE::CVariant( secret.Reveal() ) );
        secret.Clear();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPrimaryAssetStoreSettings::Serialize( CORE::CDataNode& domRootNode                        ,
                                       const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    // a password never goes into the settings that are saved
    COMCORE::CDatabaseClientSettings databaseToSave( database );
    databaseToSave.backendOptions.Delete( "password" );

    bool success = databaseToSave.Serialize( *domRootNode.AddChild( "database", GUCEF_DATATYPE_OBJECT ), settings );
    success = domRootNode.SetAttribute( "credentialProvider", credentialProvider ) && success;
    success = domRootNode.SetAttribute( "credentialAccount", credentialAccount ) && success;
    success = domRootNode.SetAttribute( "databasePasswordFile", databasePasswordFile ) && success;
    CORE::CDataNode* locationsNode = domRootNode.AddChild( "locations", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; GUCEF_NULL != locationsNode && i<locations.size(); ++i )
        success = locations[ i ].Serialize( *locationsNode->AddChild( "location", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    success = domRootNode.SetAttribute( "defaultLocationId", defaultLocationId ) && success;
    CORE::CDataNode* sourcesNode = domRootNode.AddChild( "sources", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; GUCEF_NULL != sourcesNode && i<sources.size(); ++i )
        success = sources[ i ].Serialize( *sourcesNode->AddChild( "source", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    success = defaultPolicy.Serialize( *domRootNode.AddChild( "defaultPolicy", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    success = domRootNode.SetAttribute( "duplicatePolicy", CORE::CString( AssetDuplicatePolicyToString( duplicatePolicy ) ) ) && success;
    success = domRootNode.SetAttribute( "copyDiskPacks", copyDiskPacks ) && success;
    success = domRootNode.SetAttribute( "seedDataPath", seedDataPath ) && success;
    if ( !scratchStagingPath.IsNULLOrEmpty() )
        success = domRootNode.SetAttribute( "scratchStagingPath", scratchStagingPath ) && success;
    success = domRootNode.SetAttribute( "analysisConcurrency", analysisConcurrency ) && success;
    success = domRootNode.SetAttribute( "indexAfterIngest", indexAfterIngest ) && success;
    success = domRootNode.SetAttribute( "threadPoolName", threadPoolName ) && success;
    success = domRootNode.SetAttribute( "analysisThreadPoolName", analysisThreadPoolName ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CPrimaryAssetStoreSettings::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                         const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CPrimaryAssetStoreSettings loaded;
    const CORE::CDataNode* databaseNode = domRootNode.FindChild( "database" );
    if ( GUCEF_NULL != databaseNode && !loaded.database.Deserialize( *databaseNode, settings ) )
        return false;
    loaded.credentialProvider = domRootNode.GetAttributeValueOrChildValueByName( "credentialProvider" ).AsString();
    loaded.credentialAccount = domRootNode.GetAttributeValueOrChildValueByName( "credentialAccount" ).AsString();
    loaded.databasePasswordFile = domRootNode.GetAttributeValueOrChildValueByName( "databasePasswordFile" ).AsString();
    const CORE::CDataNode* locationsNode = domRootNode.FindChild( "locations" );
    if ( GUCEF_NULL != locationsNode )
    {
        loaded.locations.clear();
        CORE::CDataNode::const_iterator i = locationsNode->ConstBegin();
        while ( i != locationsNode->ConstEnd() )
        {
            CAssetStorageLocation location;
            if ( location.Deserialize( *(*i), settings ) && !location.locationId.IsNULLOrEmpty() )
                loaded.locations.push_back( location );
            ++i;
        }
    }
    loaded.defaultLocationId = domRootNode.GetAttributeValueOrChildValueByName( "defaultLocationId" ).AsString( loaded.defaultLocationId );
    const CORE::CDataNode* sourcesNode = domRootNode.FindChild( "sources" );
    if ( GUCEF_NULL != sourcesNode )
    {
        CORE::CDataNode::const_iterator i = sourcesNode->ConstBegin();
        while ( i != sourcesNode->ConstEnd() )
        {
            CAssetSourcePolicy policy;
            if ( policy.Deserialize( *(*i), settings ) )
                loaded.sources.push_back( policy );
            ++i;
        }
    }
    const CORE::CDataNode* defaultPolicyNode = domRootNode.FindChild( "defaultPolicy" );
    if ( GUCEF_NULL != defaultPolicyNode )
        loaded.defaultPolicy.Deserialize( *defaultPolicyNode, settings );
    loaded.duplicatePolicy = AssetDuplicatePolicyFromString( domRootNode.GetAttributeValueOrChildValueByName( "duplicatePolicy" ).AsString(), ASSETDUPLICATE_SKIP );
    loaded.copyDiskPacks = domRootNode.GetAttributeValueOrChildValueByName( "copyDiskPacks" ).AsBool( true, true );
    loaded.seedDataPath = domRootNode.GetAttributeValueOrChildValueByName( "seedDataPath" ).AsString( loaded.seedDataPath );
    loaded.scratchStagingPath = domRootNode.GetAttributeValueOrChildValueByName( "scratchStagingPath" ).AsString( loaded.scratchStagingPath );
    loaded.analysisConcurrency = domRootNode.GetAttributeValueOrChildValueByName( "analysisConcurrency" ).AsUInt32( loaded.analysisConcurrency, true );
    loaded.indexAfterIngest = domRootNode.GetAttributeValueOrChildValueByName( "indexAfterIngest" ).AsBool( loaded.indexAfterIngest, true );
    loaded.threadPoolName = domRootNode.GetAttributeValueOrChildValueByName( "threadPoolName" ).AsString( loaded.threadPoolName );
    loaded.analysisThreadPoolName = domRootNode.GetAttributeValueOrChildValueByName( "analysisThreadPoolName" ).AsString( loaded.analysisThreadPoolName );
    if ( 0 == loaded.analysisConcurrency )
        loaded.analysisConcurrency = 1;
    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CPrimaryAssetStoreSettings::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CPrimaryAssetStoreSettings( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPrimaryAssetStoreSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CPrimaryAssetStoreSettings";
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
