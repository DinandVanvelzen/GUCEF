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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_COM_CMCPSCHEMABUILDER_H
#include "gucefCOM_CMcpSchemaBuilder.h"
#define GUCEF_COM_CMCPSCHEMABUILDER_H
#endif /* GUCEF_COM_CMCPSCHEMABUILDER_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#include "gucefASSETSTORE_CAssetRootedPath.h"
#define GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#endif /* GUCEF_ASSETSTORE_CASSETROOTEDPATH_H ? */

#include "AssetStoreConverterHook_CPipelineOperationCall.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CPipelineOperationCall::ErrorPathRefused = "path-refused";
const CORE::CString CPipelineOperationCall::ErrorPackUnknown = "pack-unknown";
const CORE::CString CPipelineOperationCall::ErrorPackAmbiguous = "pack-ambiguous";
const CORE::CString CPipelineOperationCall::ErrorLicenceRefused = "licence-refused";
const CORE::CString CPipelineOperationCall::ErrorContentWithheld = "content-withheld";
const CORE::CString CPipelineOperationCall::ErrorChangeTokenInvalid = "change-token-invalid";
const CORE::CString CPipelineOperationCall::ErrorChangeConflict = "change-conflict";
const CORE::CString CPipelineOperationCall::ErrorBusy = "busy";
const CORE::CString CPipelineOperationCall::ErrorInvalidArguments = "invalid-arguments";
const CORE::CString CPipelineOperationCall::ErrorNotFound = "not-found";
const CORE::CString CPipelineOperationCall::ErrorUnsupported = "unsupported";
const CORE::CString CPipelineOperationCall::ErrorTooLarge = "too-large";
const CORE::CString CPipelineOperationCall::ErrorBackendUnavailable = "backend-unavailable";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const PageTokenPrefix = "p";

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPipelineOperationCall::CPipelineOperationCall( const ASSETSTORE::CAssetPipelineRequest& callRequest  ,
                                                ASSETSTORE::CIAssetConversionObserver* callObserver   ,
                                                ASSETSTORE::CAssetServiceDocument& callResult         ,
                                                const CPipelineOperationSettings& callSettings        )
    : request( callRequest )
    , observer( callObserver )
    , result( callResult )
    , settings( callSettings )
    , principal()
    , exposePurchaseDetails( false )
    , maxUntrustedTextChars( 500 )
{GUCEF_TRACE;

    principal = GetArgument( "principal" );
    exposePurchaseDetails = GetBoolArgument( "exposePurchaseDetails", false );
    const CORE::CVariant& chars = request.arguments.GetAttributeValueOrChildValueByName( "maxUntrustedTextChars" );
    if ( !chars.IsNULLOrEmpty() )
        maxUntrustedTextChars = chars.AsUInt32( maxUntrustedTextChars, true );
    if ( 0 == maxUntrustedTextChars || maxUntrustedTextChars > 2000 )
        maxUntrustedTextChars = 2000;
    result.document.SetName( "result" );
    result.document.SetNodeType( GUCEF_DATATYPE_OBJECT );
}

/*-------------------------------------------------------------------------*/

CORE::CDataNode&
CPipelineOperationCall::Document( void )
{GUCEF_TRACE;

    return result.document;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineOperationCall::StatusOfError( const CORE::CString& errorCode )
{GUCEF_TRACE;

    if ( ErrorInvalidArguments == errorCode || ErrorPathRefused == errorCode || ErrorTooLarge == errorCode )
        return ASSETSTORE::ASSETSTORE_STATUS_INVALID_ARGUMENT;
    if ( ErrorPackUnknown == errorCode || ErrorNotFound == errorCode )
        return ASSETSTORE::ASSETSTORE_STATUS_NOT_FOUND;
    if ( ErrorLicenceRefused == errorCode || ErrorContentWithheld == errorCode )
        return ASSETSTORE::ASSETSTORE_STATUS_REFUSED;
    if ( ErrorUnsupported == errorCode )
        return ASSETSTORE::ASSETSTORE_STATUS_UNSUPPORTED;
    if ( ErrorBackendUnavailable == errorCode )
        return ASSETSTORE::ASSETSTORE_STATUS_UNAVAILABLE;
    return ASSETSTORE::ASSETSTORE_STATUS_FAILED;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineOperationCall::Fail( const CORE::CString& errorCode, const CORE::CString& message )
{GUCEF_TRACE;

    result.status = StatusOfError( errorCode );
    result.message = message;
    result.document.SetAttribute( "errorCode", errorCode );
    result.document.SetAttribute( "message", message );
    return result.status;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineOperationCall::Succeed( const CORE::CString& message )
{GUCEF_TRACE;

    result.status = ASSETSTORE::ASSETSTORE_STATUS_OK;
    result.message = message;
    return result.status;
}

/*-------------------------------------------------------------------------*/

CORE::CDataNode
CPipelineOperationCall::Untrusted( const CORE::CString& name, const CORE::CString& source, const CORE::CString& text ) const
{GUCEF_TRACE;

    CORE::CDataNode value = COM::CMcpSchemaBuilder::MakeUntrustedTextValue( source, text, maxUntrustedTextChars );
    value.SetName( name );
    return value;
}

/*-------------------------------------------------------------------------*/

void
CPipelineOperationCall::SetRooted( CORE::CDataNode& node, const CORE::CString& member, const CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    CORE::CString rooted;
    if ( !vfsPath.IsNULLOrEmpty() && CConverterPack::ToRooted( settings.roots, vfsPath, rooted ) )
        node.SetAttribute( member, rooted );
    else
        node.SetAttribute( member, CORE::CVariant() );
}

/*-------------------------------------------------------------------------*/

void
CPipelineOperationCall::SetTextOrNull( CORE::CDataNode& node, const CORE::CString& member, const CORE::CString& text )
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() )
        node.SetAttribute( member, CORE::CVariant() );
    else
        node.SetAttribute( member, text );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPipelineOperationCall::GetArgument( const CORE::CString& name ) const
{GUCEF_TRACE;

    return request.arguments.GetAttributeValueOrChildValueByName( name ).AsString( CORE::CString::Empty, true );
}

/*-------------------------------------------------------------------------*/

bool
CPipelineOperationCall::GetBoolArgument( const CORE::CString& name, bool defaultValue ) const
{GUCEF_TRACE;

    const CORE::CVariant& value = request.arguments.GetAttributeValueOrChildValueByName( name );
    if ( value.IsNULLOrEmpty() )
        return defaultValue;
    return value.AsBool( defaultValue, true );
}

/*-------------------------------------------------------------------------*/

void
CPipelineOperationCall::GetListArgument( const CORE::CString& name, CORE::CString::StringVector& values ) const
{GUCEF_TRACE;

    values.clear();
    request.arguments.GetValuesOfChildByName( name, values );
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
CPipelineOperationCall::GetNodeArgument( const CORE::CString& name ) const
{GUCEF_TRACE;

    return request.arguments.FindChild( name );
}

/*-------------------------------------------------------------------------*/

void
CPipelineOperationCall::GetPacks( TConverterPackVector& packs ) const
{GUCEF_TRACE;

    CConverterPack::Discover( settings.roots, packs );
}

/*-------------------------------------------------------------------------*/

bool
CPipelineOperationCall::FindPack( const CORE::CString& packId, CConverterPack& pack )
{GUCEF_TRACE;

    if ( packId.IsNULLOrEmpty() )
    {
        Fail( ErrorInvalidArguments, "the request names no pack (packId)" );
        return false;
    }
    TConverterPackVector packs;
    GetPacks( packs );
    for ( size_t i=0; i<packs.size(); ++i )
    {
        if ( packs[ i ].packId != packId )
            continue;
        if ( packs[ i ].ambiguous )
        {
            CORE::CString names = packs[ i ].manifestRooted;
            for ( size_t o=0; o<packs[ i ].otherManifests.size(); ++o )
                names += " and " + packs[ i ].otherManifests[ o ];
            Fail( ErrorPackAmbiguous, "the pack id " + packId + " is used by more than one manifest: " + names + "; the owner has to give one of them another id" );
            return false;
        }
        pack = packs[ i ];
        return true;
    }
    Fail( ErrorPackUnknown, "there is no pack " + packId + "; pack_list gives the packs that are there" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPipelineOperationCall::FindRequestPack( CConverterPack& pack, bool runnable )
{GUCEF_TRACE;

    if ( !FindPack( request.packId, pack ) )
        return false;
    if ( runnable && !pack.runnable )
    {
        Fail( ErrorPathRefused, "the pack " + pack.packId + " cannot be run or changed through the tools: " + pack.runnableReason );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPipelineOperationCall::ResolveRootedPath( const CORE::CString& text, const CORE::CString& kinds, bool mustWrite, CORE::CString& vfsPath,
                                           const ASSETSTORE::CAssetToolRoot*& root, CORE::CString& rooted )
{GUCEF_TRACE;

    ASSETSTORE::CAssetRootedPath path;
    CORE::CString problem;
    if ( !ASSETSTORE::CAssetRootedPath::ParseAndResolve( text, settings.roots, kinds, path, root, vfsPath, problem ) )
    {
        Fail( ErrorPathRefused, "the path is refused: " + problem );
        return false;
    }
    if ( mustWrite && !root->AllowsWrites() )
    {
        Fail( ErrorPathRefused, "the root " + root->id + " cannot be written through the tools" );
        return false;
    }
    rooted = path.ToString();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPipelineOperationCall::CheckAssetPath( const CORE::CString& assetPath )
{GUCEF_TRACE;

    CORE::CString problem;
    if ( !ASSETSTORE::CAssetRootedPath::IsSafeRelativePath( assetPath, false, true, problem ) )
    {
        Fail( ErrorPathRefused, "the asset path is refused: " + problem );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPipelineOperationCall::GetPage( UInt32 defaultEntries, UInt32& first, UInt32& count )
{GUCEF_TRACE;

    first = 0;
    count = 0 == request.maxEntries ? ( 0 == defaultEntries ? settings.defaultPageEntries : defaultEntries ) : request.maxEntries;
    if ( count > settings.maxPageEntries )
        count = settings.maxPageEntries;
    if ( request.pageToken.IsNULLOrEmpty() )
        return true;
    const CORE::CString digits = request.pageToken.CutChars( 1, true );
    if ( 0 != request.pageToken.HasSubstr( PageTokenPrefix, true ) || digits.IsNULLOrEmpty() || digits.Length() > 9 )
    {
        Fail( ErrorInvalidArguments, "the page token is not one this operation gave" );
        return false;
    }
    for ( UInt32 i=0; i<digits.Length(); ++i )
    {
        if ( digits[ i ] < '0' || digits[ i ] > '9' )
        {
            Fail( ErrorInvalidArguments, "the page token is not one this operation gave" );
            return false;
        }
    }
    first = CORE::StringToUInt32( digits );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CPipelineOperationCall::FinishPage( UInt32 first, UInt32 count, UInt32 total )
{GUCEF_TRACE;

    result.document.SetAttribute( "total", total );
    if ( first + count < total )
        result.nextPageToken = CORE::CString( PageTokenPrefix ) + CORE::ToString( first + count );
    else
        result.nextPageToken.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CPipelineOperationCall::IsCancelRequested( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != observer && observer->IsCancelRequested();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
