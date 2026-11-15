/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#include "gucefASSETSTORE_CAssetRootedPath.h"
#define GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#endif /* GUCEF_ASSETSTORE_CASSETROOTEDPATH_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetStoreConverterHook_CPipelineLicenceOperations.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** An id for a record made of a name: lower case letters, digits and '-' */
CORE::CString
MakeRecordId( const CORE::CString& name )
{GUCEF_TRACE;

    CORE::CString id;
    bool dash = false;
    for ( UInt32 i=0; i<name.Length() && id.Length()<64; ++i )
    {
        char c = name[ i ];
        if ( c >= 'A' && c <= 'Z' )
            c = (char) ( c - 'A' + 'a' );
        if ( ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) )
        {
            id += c;
            dash = false;
        }
        else
        if ( !dash && id.Length() > 0 )
        {
            id += '-';
            dash = true;
        }
    }
    while ( id.Length() > 0 && '-' == id[ id.Length()-1 ] )
        id = id.CutChars( 1, false );
    return id.IsNULLOrEmpty() ? CORE::CString( "pack" ) : id;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CPipelineLicenceOperations::MakeId( const CORE::CString& name )
{GUCEF_TRACE;

    return MakeRecordId( name );
}

/*-------------------------------------------------------------------------*/

bool
CPipelineLicenceOperations::FindPackOfRequest( CPipelineOperationCall& call, CConverterPack& pack, bool& found, CORE::CString& sourceVfsPath )
{GUCEF_TRACE;

    found = false;
    sourceVfsPath.Clear();
    if ( !call.request.packId.IsNULLOrEmpty() )
    {
        if ( !call.FindPack( call.request.packId, pack ) )
            return false;
        found = true;
        sourceVfsPath = pack.manifest.GetSourcePath();
        return true;
    }
    if ( call.request.sourcePath.IsNULLOrEmpty() )
    {
        call.Fail( CPipelineOperationCall::ErrorInvalidArguments, "give packId or sourcePath" );
        return false;
    }
    const ASSETSTORE::CAssetToolRoot* root = GUCEF_NULL;
    CORE::CString rooted;
    if ( !call.ResolveRootedPath( call.request.sourcePath, "source|staging", false, sourceVfsPath, root, rooted ) )
        return false;
    TConverterPackVector packs;
    call.GetPacks( packs );
    const CORE::CString wanted = ASSETSTORE::CAssetRootedPath::NormalizeVfsPath( sourceVfsPath );
    for ( size_t i=0; i<packs.size(); ++i )
    {
        if ( ASSETSTORE::CAssetRootedPath::NormalizeVfsPath( packs[ i ].manifest.GetSourcePath() ) == wanted )
        {
            if ( packs[ i ].ambiguous )
                return call.FindPack( packs[ i ].packId, pack );
            pack = packs[ i ];
            found = true;
            return true;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineLicenceOperations::LicenceCheck( CPipelineOperationCall& call )
{GUCEF_TRACE;

    CConverterPack pack;
    bool found = false;
    CORE::CString sourceVfsPath;
    if ( !FindPackOfRequest( call, pack, found, sourceVfsPath ) )
        return call.result.status;

    CORE::CDataNode& document = call.Document();
    CORE::CString::StringVector missing;
    if ( found )
    {
        document.SetAttribute( "packId", pack.packId );
        pack.licence.ToDataNode( document, false, false, call.maxUntrustedTextChars );
        document.SetName( "result" );
        pack.licence.GetMissingFields( missing );
    }
    else
    {
        document.SetAttribute( "packId", CORE::CVariant() );
        CConverterLicenceDecision none;
        none.reason = "no manifest names this source, so no licence record is known for it";
        none.conversionAllowed = false;
        none.exposureReason = "the source has no licence record";
        none.ToDataNode( document, false, false, call.maxUntrustedTextChars );
        document.SetName( "result" );
        missing.push_back( "record" );
    }
    document.SetValuesOfChildByName( "missingFields", missing );
    return call.Succeed();
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineLicenceOperations::LicenceDraftRecord( CPipelineOperationCall& call )
{GUCEF_TRACE;

    CConverterPack pack;
    bool found = false;
    CORE::CString sourceVfsPath;
    if ( !FindPackOfRequest( call, pack, found, sourceVfsPath ) )
        return call.result.status;

    CORE::CString id;
    if ( found )
        id = pack.manifest.licenceRecord.IsNULLOrEmpty() ? MakeRecordId( pack.packId ) : pack.manifest.licenceRecord;
    else
        id = MakeRecordId( ASSETCONVERTER::CVfsPaths::StemOf( sourceVfsPath ) );

    // the folder the owner would save it in: the licence folder of the pack when it lies in a licences root, else the first licences root
    CORE::CString targetVfs;
    if ( found && !pack.manifest.licenceRecord.IsNULLOrEmpty() )
    {
        CORE::CString relative;
        const CORE::CString folder = ASSETCONVERTER::CVfsPaths::FolderOf( pack.manifest.GetLicenceRecordPath() );
        if ( GUCEF_NULL != ASSETSTORE::CAssetToolRoot::FindHolder( call.settings.roots, folder, ASSETSTORE::CAssetToolRoot::KindLicences, relative ) )
            targetVfs = ASSETCONVERTER::CVfsPaths::Join( folder, id + ".licence.json" );
    }
    if ( targetVfs.IsNULLOrEmpty() )
    {
        const ASSETSTORE::CAssetToolRoot* licences = ASSETSTORE::CAssetToolRoot::FindFirstOfKind( call.settings.roots, ASSETSTORE::CAssetToolRoot::KindLicences );
        if ( GUCEF_NULL != licences )
            targetVfs = ASSETCONVERTER::CVfsPaths::Join( licences->vfsPath, id + ".licence.json" );
    }

    const CORE::CDataNode* storeItem = call.GetNodeArgument( "storeItem" );
    CORE::CString title = found ? pack.manifest.title : CORE::CString::Empty;
    CORE::CString store = found ? pack.manifest.store : CORE::CString::Empty;
    CORE::CString licenceName;
    CORE::CString licenceUrl;
    CORE::CString seller;
    CORE::CString::StringVector suggested;
    if ( GUCEF_NULL != storeItem )
    {
        const CORE::CString itemTitle = storeItem->GetAttributeValueOrChildValueByName( "title" ).AsString( CORE::CString::Empty, true );
        if ( !itemTitle.IsNULLOrEmpty() )
            title = itemTitle;
        seller = storeItem->GetAttributeValueOrChildValueByName( "seller" ).AsString( CORE::CString::Empty, true );
        licenceName = storeItem->GetAttributeValueOrChildValueByName( "licenceName" ).AsString( CORE::CString::Empty, true );
        licenceUrl = storeItem->GetAttributeValueOrChildValueByName( "licenceUrl" ).AsString( CORE::CString::Empty, true );
        storeItem->GetValuesOfChildByName( "restrictions", suggested );
    }
    if ( !call.request.storeId.IsNULLOrEmpty() )
        store = call.request.storeId;

    CORE::CDataNode& document = call.Document();
    CORE::CDataNode* draft = document.AddChild( "draft", GUCEF_DATATYPE_OBJECT );
    draft->SetAttribute( "id", id );
    draft->SetAttribute( "store", store );
    draft->SetAttribute( "product", title );
    draft->SetAttribute( "licenceName", licenceName );
    draft->SetAttribute( "licenceUrl", licenceUrl );
    if ( !call.request.itemId.IsNULLOrEmpty() )
        draft->SetAttribute( "orderReference", CORE::CString::Empty );
    if ( seller.IsNULLOrEmpty() )
        document.SetAttribute( "seller", CORE::CVariant() );
    else
        document.SetAttribute( "seller", seller );
    call.SetRooted( document, "target", targetVfs );
    CORE::CString::StringVector decisions;
    decisions.push_back( "ueOnly" );
    decisions.push_back( "referenceOnly" );
    decisions.push_back( "restrictedOrNonStandard" );
    decisions.push_back( "humbleOneSeat" );
    decisions.push_back( "reviewed" );
    decisions.push_back( "restrictions" );
    decisions.push_back( "seats" );
    decisions.push_back( "checked" );
    document.SetValuesOfChildByName( "decisions", decisions );
    document.SetValuesOfChildByName( "suggestedRestrictions", suggested );
    document.SetAttribute( "recordExists", !targetVfs.IsNULLOrEmpty() && VFS::CVfsGlobal::Instance()->GetVfs().FileExists( targetVfs ) );
    document.SetAttribute( "packId", found ? CORE::CVariant( pack.packId ) : CORE::CVariant() );
    return call.Succeed();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
