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

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_COM_CMCPJOBINFO_H
#include "gucefCOM_CMcpJobInfo.h"
#define GUCEF_COM_CMCPJOBINFO_H
#endif /* GUCEF_COM_CMCPJOBINFO_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#include "gucefASSETSTORE_CAssetRootedPath.h"
#define GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#endif /* GUCEF_ASSETSTORE_CASSETROOTEDPATH_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#include "AssetConverterLib_CAssetConverterGlobal.h"
#define ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#include "AssetConverterLib_CAssetPluginRegistry.h"
#define ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#endif /* ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPIPELINE_H
#include "AssetConverterLib_CAssetPipeline.h"
#define ASSETCONVERTERLIB_CASSETPIPELINE_H
#endif /* ASSETCONVERTERLIB_CASSETPIPELINE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H
#include "AssetConverterLib_CAssetPipelineOptions.h"
#define ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H
#endif /* ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKPLAN_H
#include "AssetConverterLib_CAssetPackPlan.h"
#define ASSETCONVERTERLIB_CASSETPACKPLAN_H
#endif /* ASSETCONVERTERLIB_CASSETPACKPLAN_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#ifndef ASSETCONVERTERLIB_CCONTENTHASH_H
#include "AssetConverterLib_CContentHash.h"
#define ASSETCONVERTERLIB_CCONTENTHASH_H
#endif /* ASSETCONVERTERLIB_CCONTENTHASH_H ? */

#ifndef ASSETCONVERTERLIB_CIMPORTACTIVITYLOG_H
#include "AssetConverterLib_CImportActivityLog.h"
#define ASSETCONVERTERLIB_CIMPORTACTIVITYLOG_H
#endif /* ASSETCONVERTERLIB_CIMPORTACTIVITYLOG_H ? */

#ifndef ASSETCONVERTERLIB_CIMPORTSETTINGSBUILDER_H
#include "AssetConverterLib_CImportSettingsBuilder.h"
#define ASSETCONVERTERLIB_CIMPORTSETTINGSBUILDER_H
#endif /* ASSETCONVERTERLIB_CIMPORTSETTINGSBUILDER_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CTEXTLINEDIFF_H
#include "AssetStoreConverterHook_CTextLineDiff.h"
#define ASSETSTORECONVERTERHOOK_CTEXTLINEDIFF_H
#endif /* ASSETSTORECONVERTERHOOK_CTEXTLINEDIFF_H ? */

#include "AssetStoreConverterHook_CPipelineAuthoringOperations.h"

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

const char* const ManifestSuffix = ".assetpack.json";

/** The bytes of a file of the VFS as text; false when it is not there */
bool
ReadText( const CORE::CString& vfsPath, CORE::CString& text )
{GUCEF_TRACE;

    text.Clear();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.FileExists( vfsPath ) )
        return false;
    CORE::CDynamicBuffer buffer;
    if ( !vfs.LoadFile( buffer, vfsPath, "rb" ) )
        return false;
    if ( buffer.GetDataSize() > 0 )
        text.Set( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() );
    return true;
}

/** The new text with the line endings of the old one (CRLF when the old one has them) */
CORE::CString
WithLineEndingsOf( const CORE::CString& text, const CORE::CString& old )
{GUCEF_TRACE;

    CORE::CString lf = text.ReplaceSubstr( "\r\n", "\n" );
    if ( lf.Length() == 0 || '\n' != lf[ lf.Length()-1 ] )
        lf += '\n';
    if ( old.HasSubstr( "\r\n", true ) >= 0 )
        return lf.ReplaceSubstr( "\n", "\r\n" );
    return lf;
}

/** The digest of what a write is made of: the operation, the pack, the asset and the arguments that describe the change */
CORE::CString
ArgumentsDigest( const CPipelineOperationCall& call )
{GUCEF_TRACE;

    CORE::CDataNode digest( "change", GUCEF_DATATYPE_OBJECT );
    digest.SetAttribute( "operation", call.request.operation );
    digest.SetAttribute( "packId", call.request.packId );
    digest.SetAttribute( "assetPath", call.request.assetPath );
    CORE::CDataNode* arguments = digest.AddChild( call.request.arguments );
    arguments->SetName( "arguments" );
    arguments->DelAttribute( "principal" );
    arguments->DelAttribute( "maxUntrustedTextChars" );
    arguments->DelAttribute( "exposePurchaseDetails" );
    arguments->DelAttribute( "changeToken" );
    arguments->DelAttribute( "apply" );
    arguments->DelAttribute( "requestKey" );
    return ASSETCONVERTER::CCanonicalJson::Hash( digest );
}

/** Makes the folder and the folders above it */
bool
EnsureFolder( const CORE::CString& folder )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( folder.IsNULLOrEmpty() || vfs.DirExists( folder ) )
        return true;
    CORE::CString::StringVector missing;
    CORE::CString current = folder;
    while ( !current.IsNULLOrEmpty() && !vfs.DirExists( current ) )
    {
        missing.push_back( current );
        current = ASSETCONVERTER::CVfsPaths::FolderOf( current );
    }
    for ( size_t m=missing.size(); m>0; --m )
        vfs.CreateDir( missing[ m-1 ] );
    return vfs.DirExists( folder );
}

bool
StoreText( const CORE::CString& vfsPath, const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer;
    if ( text.Length() > 0 )
        buffer.CopyFrom( text.Length(), text.C_String() );
    return VFS::CVfsGlobal::Instance()->GetVfs().StoreAsFile( vfsPath, buffer, 0, true );
}

/** The preview of a change: the diff and a token, nothing written */
ASSETSTORE::TAssetStoreStatus
Preview( CPipelineOperationCall& call, CPendingChangeStore& changes, const CORE::CString& targetVfs, const CORE::CString& targetRooted,
         const CORE::CString& content, bool removes, const CORE::CString::StringVector& warnings )
{GUCEF_TRACE;

    CORE::CString old;
    const bool exists = ReadText( targetVfs, old );
    CPendingChange change;
    change.principal = call.principal;
    change.operation = call.request.operation;
    change.targetVfsPath = targetVfs;
    change.targetRooted = targetRooted;
    change.targetExisted = exists;
    change.baseDigest = exists ? ASSETCONVERTER::CContentHash::HashFile( targetVfs ) : CORE::CString::Empty;
    change.removes = removes;
    change.newContent = removes ? CORE::CString::Empty : WithLineEndingsOf( content, old );
    change.newDigest = removes ? CORE::CString::Empty : ASSETCONVERTER::CContentHash::HashText( change.newContent );
    change.argumentsDigest = ArgumentsDigest( call );
    if ( !changes.Add( change, call.settings.changeTokenTtlMs, call.settings.maxChangeTokensPerPrincipal ) )
        return call.Fail( CPipelineOperationCall::ErrorBackendUnavailable, "no change token could be made" );

    CORE::CDataNode& document = call.Document();
    document.SetAttribute( "target", targetRooted );
    document.SetAttribute( "exists", exists );
    CPipelineOperationCall::SetTextOrNull( document, "baseDigest", change.baseDigest );
    CPipelineOperationCall::SetTextOrNull( document, "newDigest", change.newDigest );
    bool truncated = false;
    document.SetAttribute( "diff", CTextLineDiff::Unified( old, change.newContent, "a/" + targetRooted, removes ? CORE::CString( "/dev/null" ) : "b/" + targetRooted,
                                                         call.settings.maxDiffLines, truncated ) );
    document.SetAttribute( "diffTruncated", truncated );
    document.SetValuesOfChildByName( "warnings", warnings );
    document.SetAttribute( "changeToken", change.token );
    document.SetAttribute( "expiresUtc", COM::CMcpJobInfo::UtcMsToText( change.expiresUtcMs ) );
    document.SetAttribute( "written", false );
    call.result.changeToken = change.token;
    return call.Succeed( "previewed; apply with the change token to write" );
}

/** The apply of a previewed change */
ASSETSTORE::TAssetStoreStatus
Apply( CPipelineOperationCall& call, CPendingChangeStore& changes, const CORE::CString& targetVfs )
{GUCEF_TRACE;

    if ( call.request.changeToken.IsNULLOrEmpty() )
        return call.Fail( CPipelineOperationCall::ErrorChangeTokenInvalid, "an apply needs the change token of its preview" );
    CPendingChange change;
    switch ( changes.Take( call.request.changeToken, call.principal, call.request.operation, ArgumentsDigest( call ), change ) )
    {
        case CPendingChangeStore::TAKE_OK: break;
        case CPendingChangeStore::TAKE_MISMATCH:
            return call.Fail( CPipelineOperationCall::ErrorChangeTokenInvalid, "the change token belongs to another principal, operation or set of arguments: preview the change again" );
        default:
            return call.Fail( CPipelineOperationCall::ErrorChangeTokenInvalid, "the change token is unknown, used or expired: preview the change again" );
    }
    if ( ASSETSTORE::CAssetRootedPath::NormalizeVfsPath( change.targetVfsPath ) != ASSETSTORE::CAssetRootedPath::NormalizeVfsPath( targetVfs ) )
        return call.Fail( CPipelineOperationCall::ErrorChangeTokenInvalid, "the change token is for another target" );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const bool exists = vfs.FileExists( targetVfs );
    const CORE::CString digest = exists ? ASSETCONVERTER::CContentHash::HashFile( targetVfs ) : CORE::CString::Empty;
    if ( exists != change.targetExisted || digest != change.baseDigest )
        return call.Fail( CPipelineOperationCall::ErrorChangeConflict, change.targetRooted + " changed since the preview: preview the change again" );

    CORE::CDataNode& document = call.Document();
    document.SetAttribute( "target", change.targetRooted );
    CORE::CString::StringVector warnings;

    // the earlier content goes to the history first
    if ( exists )
    {
        if ( call.settings.historyVfsPath.IsNULLOrEmpty() )
        {
            warnings.push_back( "no history folder is configured: the earlier content is not kept" );
            document.SetAttribute( "history", CORE::CVariant() );
        }
        else
        {
            ASSETSTORE::CAssetRootedPath rooted;
            CORE::CString problem;
            rooted.Parse( change.targetRooted, problem );
            CORE::CString stamp = ASSETCONVERTER::CImportActivityLog::GetUtcTimeText().RemoveChar( '-' ).RemoveChar( ':' ).RemoveChar( '.' );
            const CORE::CString folder = ASSETCONVERTER::CVfsPaths::Join( ASSETCONVERTER::CVfsPaths::Join( call.settings.historyVfsPath, rooted.rootId ), rooted.relativePath );
            const CORE::CString historyPath = ASSETCONVERTER::CVfsPaths::Join( folder, stamp + "-" + digest.SubstrToIndex( 8, true ) + ".json" );
            CORE::CString old;
            ReadText( targetVfs, old );
            if ( !EnsureFolder( folder ) || !StoreText( historyPath, old ) )
                return call.Fail( CPipelineOperationCall::ErrorBackendUnavailable, "the earlier content cannot be kept in the history: nothing is written" );
            // the history folder is no root: it is named by its path below the history folder
            CORE::CString historyRooted;
            CORE::CString relative;
            if ( !CConverterPack::ToRooted( call.settings.roots, historyPath, historyRooted ) &&
                 ASSETCONVERTER::CVfsPaths::MakeRelative( call.settings.historyVfsPath, historyPath, relative ) )
                historyRooted = "history:" + relative;
            CPipelineOperationCall::SetTextOrNull( document, "history", historyRooted );
        }
    }
    else
        document.SetAttribute( "history", CORE::CVariant() );

    if ( change.removes )
    {
        if ( !vfs.DeleteFile( targetVfs, true ) )
            return call.Fail( CPipelineOperationCall::ErrorBackendUnavailable, change.targetRooted + " cannot be removed" );
    }
    else
    {
        const CORE::CString folder = ASSETCONVERTER::CVfsPaths::FolderOf( targetVfs );
        const CORE::CString temporary = ASSETCONVERTER::CVfsPaths::Join( folder, "." + ASSETCONVERTER::CVfsPaths::NameOf( targetVfs ) + ".tmp-" + change.token.SubstrToIndex( 8, true ) );
        if ( !EnsureFolder( folder ) || !StoreText( temporary, change.newContent ) )
            return call.Fail( CPipelineOperationCall::ErrorBackendUnavailable, change.targetRooted + " cannot be written" );
        if ( !vfs.MoveFileAtomically( temporary, targetVfs, true ) )
        {
            vfs.DeleteFile( temporary, true );
            return call.Fail( CPipelineOperationCall::ErrorBackendUnavailable, change.targetRooted + " cannot be replaced atomically (is the root a folder of the disk?)" );
        }
    }
    document.SetAttribute( "written", true );
    document.SetAttribute( "removed", change.removes );
    CPipelineOperationCall::SetTextOrNull( document, "newDigest", change.newDigest );
    document.SetValuesOfChildByName( "warnings", warnings );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreConverterHook: " << call.request.operation << " wrote " << change.targetRooted << " for " << change.principal );
    return call.Succeed( change.removes ? "removed" : "written" );
}

/** The writable root of a VFS path of the kind, with its rooted path; fails the call with path-refused otherwise */
bool
ResolveVfsTarget( CPipelineOperationCall& call, const CORE::CString& vfsPath, const CORE::CString& kind, CORE::CString& rooted )
{GUCEF_TRACE;

    CORE::CString relative;
    const ASSETSTORE::CAssetToolRoot* root = ASSETSTORE::CAssetToolRoot::FindHolder( call.settings.roots, vfsPath, kind, relative );
    if ( GUCEF_NULL == root )
    {
        call.Fail( CPipelineOperationCall::ErrorPathRefused, "the file lies outside the roots of the kind " + kind );
        return false;
    }
    if ( !root->AllowsWrites() )
    {
        call.Fail( CPipelineOperationCall::ErrorPathRefused, "the root " + root->id + " cannot be written through the tools" );
        return false;
    }
    CORE::CString problem;
    if ( !ASSETSTORE::CAssetRootedPath::IsInsideRootOnDisk( *root, relative, problem ) )
    {
        call.Fail( CPipelineOperationCall::ErrorPathRefused, problem );
        return false;
    }
    rooted = root->id + ":" + relative;
    return true;
}

/** The manifest of a tree as the pipeline reads it, with its folder; 'problem' when it cannot be read */
bool
ReadManifestTree( const CORE::CDataNode& tree, const CORE::CString& manifestVfsPath, ASSETCONVERTER::CAssetPackManifest& manifest, CORE::CString& problem )
{GUCEF_TRACE;

    manifest = ASSETCONVERTER::CAssetPackManifest();
    if ( !manifest.Deserialize( tree ) )
    {
        problem = "the manifest cannot be read";
        return false;
    }
    manifest.manifestFolder = ASSETCONVERTER::CVfsPaths::FolderOf( manifestVfsPath );
    if ( manifest.id.IsNULLOrEmpty() )
    {
        problem = "the manifest has no id";
        return false;
    }
    return true;
}

/** Splits a JSON pointer in its tokens, unescaped */
bool
ParsePointer( const CORE::CString& pointer, CORE::CString::StringVector& tokens, CORE::CString& problem )
{GUCEF_TRACE;

    tokens.clear();
    if ( pointer.IsNULLOrEmpty() || '/' != pointer[ 0 ] )
    {
        problem = "the path \"" + pointer + "\" is no JSON pointer below the root";
        return false;
    }
    const CORE::CString::StringVector raw = pointer.CutChars( 1, true ).ParseElements( '/', true );
    for ( size_t i=0; i<raw.size(); ++i )
        tokens.push_back( raw[ i ].ReplaceSubstr( "~1", "/" ).ReplaceSubstr( "~0", "~" ) );
    if ( tokens.empty() || tokens[ 0 ].IsNULLOrEmpty() )
    {
        problem = "the path \"" + pointer + "\" names no member";
        return false;
    }
    return true;
}

bool
ParseIndex( const CORE::CString& token, UInt32& index )
{GUCEF_TRACE;

    if ( token.IsNULLOrEmpty() || token.Length() > 6 )
        return false;
    for ( UInt32 i=0; i<token.Length(); ++i )
        if ( token[ i ] < '0' || token[ i ] > '9' )
            return false;
    index = CORE::StringToUInt32( token );
    return true;
}

/** The node the tokens lead to; 'create' makes missing objects on the way */
CORE::CDataNode*
Walk( CORE::CDataNode& tree, const CORE::CString::StringVector& tokens, size_t count, bool create, CORE::CString& problem )
{GUCEF_TRACE;

    CORE::CDataNode* node = &tree;
    for ( size_t t=0; t<count; ++t )
    {
        if ( GUCEF_DATATYPE_ARRAY == node->GetNodeType() )
        {
            UInt32 index = 0;
            if ( !ParseIndex( tokens[ t ], index ) || index >= node->GetNrOfDirectChildNodes() )
            {
                problem = "the list has no element " + tokens[ t ];
                return GUCEF_NULL;
            }
            node = node->GetChildAtIndex( index );
            continue;
        }
        CORE::CDataNode* child = node->FindChild( tokens[ t ] );
        if ( GUCEF_NULL == child )
        {
            if ( !create || GUCEF_NULL != node->GetAttribute( tokens[ t ] ) )
            {
                problem = "there is no object " + tokens[ t ] + " to go into";
                return GUCEF_NULL;
            }
            child = node->AddChild( tokens[ t ], GUCEF_DATATYPE_OBJECT );
        }
        node = child;
    }
    return node;
}

/** Adds the value of an operation (its attribute or child "value") to a parent: as the member of the name, or as an element */
void
PutValue( CORE::CDataNode& parent, const CORE::CString& name, const CORE::CDataNode& operation )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* scalar = operation.GetAttribute( "value" );
    const CORE::CDataNode* tree = GUCEF_NULL == scalar ? operation.FindChild( "value" ) : GUCEF_NULL;
    if ( GUCEF_DATATYPE_ARRAY == parent.GetNodeType() )
    {
        if ( GUCEF_NULL != scalar )
            parent.AddValueAsChild( (*scalar).second );
        else
        if ( GUCEF_NULL != tree )
        {
            CORE::CDataNode* added = parent.AddChild( *tree );
            added->SetName( CORE::CString::Empty );
        }
        return;
    }
    if ( GUCEF_NULL != scalar )
        parent.SetAttribute( name, (*scalar).second );
    else
    if ( GUCEF_NULL != tree )
    {
        CORE::CDataNode* added = parent.AddChild( *tree );
        added->SetName( name );
    }
}

bool
HasValue( const CORE::CDataNode& operation )
{GUCEF_TRACE;

    return GUCEF_NULL != operation.GetAttribute( "value" ) || GUCEF_NULL != operation.FindChild( "value" );
}

/** A member of the top of the manifest as text, to see whether an edit changed it */
CORE::CString
TopMember( const CORE::CDataNode& tree, const char* name )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* value = tree.GetAttribute( name );
    if ( GUCEF_NULL != value )
        return (*value).second.AsString( CORE::CString::Empty, true );
    const CORE::CDataNode* child = tree.FindChild( name );
    return GUCEF_NULL == child ? CORE::CString::Empty : ASSETCONVERTER::CCanonicalJson::ToJson( *child );
}

/** The override file of the asset of the request and its rooted path; fails the call when there is none or it cannot be written */
bool
ResolveOverride( CPipelineOperationCall& call, const CConverterPack& pack, CORE::CString& vfsPath, CORE::CString& rooted )
{GUCEF_TRACE;

    if ( !call.CheckAssetPath( call.request.assetPath ) )
        return false;
    vfsPath = ASSETCONVERTER::CImportSettingsBuilder::GetOverridePath( pack.manifest, call.request.assetPath );
    if ( vfsPath.IsNULLOrEmpty() )
    {
        call.Fail( CPipelineOperationCall::ErrorInvalidArguments, "the manifest of the pack names no overrides folder: add one with manifest_edit first" );
        return false;
    }
    return ResolveVfsTarget( call, vfsPath, ASSETSTORE::CAssetToolRoot::KindOverrides, rooted );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CPipelineAuthoringOperations::ApplyOperations( CORE::CDataNode& tree, const CORE::CDataNode& operations, CORE::CString& problem )
{GUCEF_TRACE;

    UInt32 index = 0;
    CORE::CDataNode::const_iterator i = operations.ConstBegin();
    for ( ; i != operations.ConstEnd(); ++i, ++index )
    {
        const CORE::CDataNode& operation = *(*i);
        const CORE::CString op = operation.GetAttributeValueOrChildValueByName( "op" ).AsString( CORE::CString::Empty, true );
        const CORE::CString path = operation.GetAttributeValueOrChildValueByName( "path" ).AsString( CORE::CString::Empty, true );
        const CORE::CString prefix = "operation " + CORE::ToString( index + 1 ) + " (" + op + " " + path + "): ";
        CORE::CString::StringVector tokens;
        if ( !ParsePointer( path, tokens, problem ) )
        {
            problem = prefix + problem;
            return false;
        }
        if ( "set" == op )
        {
            if ( !HasValue( operation ) )
            {
                problem = prefix + "a set needs a value";
                return false;
            }
            CORE::CDataNode* parent = Walk( tree, tokens, tokens.size()-1, true, problem );
            if ( GUCEF_NULL == parent )
            {
                problem = prefix + problem;
                return false;
            }
            const CORE::CString& name = tokens.back();
            if ( GUCEF_DATATYPE_ARRAY == parent->GetNodeType() )
            {
                UInt32 at = 0;
                if ( !ParseIndex( name, at ) || at >= parent->GetNrOfDirectChildNodes() )
                {
                    problem = prefix + "the list has no element " + name + " (append adds one)";
                    return false;
                }
                // the element is replaced: the new value is added at the end, then the elements after the old one move behind it
                CORE::CDataNode copy( *parent );
                parent->DelSubTree();
                UInt32 position = 0;
                for ( CORE::CDataNode::const_iterator c=copy.ConstBegin(); c!=copy.ConstEnd(); ++c, ++position )
                {
                    if ( position == at )
                        PutValue( *parent, CORE::CString::Empty, operation );
                    else
                        parent->AddChild( *(*c) );
                }
                continue;
            }
            parent->DelAttribute( name );
            parent->DelChild( name );
            PutValue( *parent, name, operation );
            continue;
        }
        if ( "remove" == op )
        {
            CORE::CDataNode* parent = Walk( tree, tokens, tokens.size()-1, false, problem );
            if ( GUCEF_NULL == parent )
            {
                problem = prefix + problem;
                return false;
            }
            const CORE::CString& name = tokens.back();
            if ( GUCEF_DATATYPE_ARRAY == parent->GetNodeType() )
            {
                UInt32 at = 0;
                if ( !ParseIndex( name, at ) || at >= parent->GetNrOfDirectChildNodes() )
                {
                    problem = prefix + "the list has no element " + name;
                    return false;
                }
                parent->GetChildAtIndex( at )->Delete();
                continue;
            }
            if ( GUCEF_NULL == parent->GetAttribute( name ) && GUCEF_NULL == parent->FindChild( name ) )
            {
                problem = prefix + "there is no member " + name;
                return false;
            }
            parent->DelAttribute( name );
            parent->DelChild( name );
            continue;
        }
        if ( "append" == op )
        {
            if ( !HasValue( operation ) )
            {
                problem = prefix + "an append needs a value";
                return false;
            }
            CORE::CDataNode* target = Walk( tree, tokens, tokens.size(), false, problem );
            if ( GUCEF_NULL == target )
            {
                // a list that is not there yet is made
                CORE::CString ignored;
                CORE::CDataNode* parent = Walk( tree, tokens, tokens.size()-1, true, ignored );
                if ( GUCEF_NULL == parent || GUCEF_DATATYPE_ARRAY == parent->GetNodeType() || GUCEF_NULL != parent->GetAttribute( tokens.back() ) )
                {
                    problem = prefix + problem;
                    return false;
                }
                target = parent->AddChild( tokens.back(), GUCEF_DATATYPE_ARRAY );
            }
            if ( GUCEF_DATATYPE_ARRAY != target->GetNodeType() )
            {
                problem = prefix + "an append goes to a list";
                return false;
            }
            PutValue( *target, CORE::CString::Empty, operation );
            continue;
        }
        problem = prefix + "the op is set, remove or append";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineAuthoringOperations::ManifestCreate( CPipelineOperationCall& call, CPendingChangeStore& changes )
{GUCEF_TRACE;

    CORE::CString targetVfs;
    const ASSETSTORE::CAssetToolRoot* root = GUCEF_NULL;
    CORE::CString rooted;
    if ( !call.ResolveRootedPath( call.GetArgument( "target" ), ASSETSTORE::CAssetToolRoot::KindManifests, true, targetVfs, root, rooted ) )
        return call.result.status;
    const UInt32 suffixLength = CORE::CString( ManifestSuffix ).Length();
    if ( targetVfs.Length() <= suffixLength || targetVfs.CutChars( targetVfs.Length() - suffixLength, true ) != ManifestSuffix )
        return call.Fail( CPipelineOperationCall::ErrorPathRefused, "a manifest is a file <name>" + CORE::CString( ManifestSuffix ) );
    if ( call.request.apply )
        return Apply( call, changes, targetVfs );

    if ( VFS::CVfsGlobal::Instance()->GetVfs().FileExists( targetVfs ) )
        return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, rooted + " exists already: manifest_edit changes a manifest" );
    const CORE::CDataNode* tree = call.GetNodeArgument( "manifest" );
    if ( GUCEF_NULL == tree )
        return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, "manifest_create needs the manifest (an object)" );
    CORE::CDataNode content( *tree );
    content.SetName( CORE::CString::Empty );
    content.SetNodeType( GUCEF_DATATYPE_OBJECT );
    ASSETCONVERTER::CAssetPackManifest manifest;
    CORE::CString problem;
    if ( !ReadManifestTree( content, targetVfs, manifest, problem ) )
        return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, problem );
    TConverterPackVector packs;
    call.GetPacks( packs );
    for ( size_t p=0; p<packs.size(); ++p )
        if ( packs[ p ].packId == manifest.id )
            return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, "the pack id " + manifest.id + " is used by " + packs[ p ].manifestRooted + " already" );
    if ( !manifest.licenceRecord.IsNULLOrEmpty() && !VFS::CVfsGlobal::Instance()->GetVfs().FileExists( manifest.GetLicenceRecordPath() ) )
        return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, "the licence record " + manifest.licenceRecord + " the manifest names does not exist: the owner makes licence records" );
    if ( !CConverterPack::CheckManifestPaths( call.settings.roots, manifest, problem ) )
        return call.Fail( CPipelineOperationCall::ErrorPathRefused, problem );
    CORE::CString::StringVector warnings;
    if ( manifest.licenceRecord.IsNULLOrEmpty() )
        warnings.push_back( "the manifest names no licence record: the pipeline refuses the pack until the owner adds one" );
    return Preview( call, changes, targetVfs, rooted, ASSETCONVERTER::CCanonicalJson::ToJson( content ), false, warnings );
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineAuthoringOperations::ManifestEdit( CPipelineOperationCall& call, CPendingChangeStore& changes )
{GUCEF_TRACE;

    CConverterPack pack;
    if ( !call.FindRequestPack( pack, true ) )
        return call.result.status;
    CORE::CString targetVfs;
    const ASSETSTORE::CAssetToolRoot* root = GUCEF_NULL;
    CORE::CString rooted;
    if ( !call.ResolveRootedPath( pack.manifestRooted, ASSETSTORE::CAssetToolRoot::KindManifests, true, targetVfs, root, rooted ) )
        return call.result.status;
    if ( call.request.apply )
        return Apply( call, changes, targetVfs );

    const CORE::CDataNode* operations = call.GetNodeArgument( "operations" );
    if ( GUCEF_NULL == operations || 0 == operations->GetNrOfDirectChildNodes() || operations->GetNrOfDirectChildNodes() > 64 )
        return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, "operations is a list of 1 to 64 operations" );
    CORE::CDataNode tree;
    if ( !ASSETCONVERTER::CCanonicalJson::Load( targetVfs, tree ) )
        return call.Fail( CPipelineOperationCall::ErrorNotFound, "the manifest " + rooted + " cannot be read" );
    tree.SetName( CORE::CString::Empty );
    CORE::CDataNode edited( tree );
    CORE::CString problem;
    if ( !ApplyOperations( edited, *operations, problem ) )
        return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, problem );
    const char* locked[] = { "id", "licenceRecord", "licenceFolder" };
    for ( UInt32 l=0; l<3; ++l )
        if ( TopMember( tree, locked[ l ] ) != TopMember( edited, locked[ l ] ) )
            return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, CORE::CString( "the member " ) + locked[ l ] + " of a manifest is not changed through the tools" );
    ASSETCONVERTER::CAssetPackManifest manifest;
    if ( !ReadManifestTree( edited, targetVfs, manifest, problem ) )
        return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, problem );
    if ( !CConverterPack::CheckManifestPaths( call.settings.roots, manifest, problem ) )
        return call.Fail( CPipelineOperationCall::ErrorPathRefused, problem );
    CORE::CString::StringVector warnings;
    return Preview( call, changes, targetVfs, rooted, ASSETCONVERTER::CCanonicalJson::ToJson( edited ), false, warnings );
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineAuthoringOperations::OverrideWrite( CPipelineOperationCall& call, CPendingChangeStore& changes )
{GUCEF_TRACE;

    CConverterPack pack;
    if ( !call.FindRequestPack( pack, true ) )
        return call.result.status;
    CORE::CString targetVfs;
    CORE::CString rooted;
    if ( !ResolveOverride( call, pack, targetVfs, rooted ) )
        return call.result.status;
    if ( call.request.apply )
        return Apply( call, changes, targetVfs );

    const CORE::CDataNode* settings = call.GetNodeArgument( "settings" );
    if ( GUCEF_NULL == settings || GUCEF_DATATYPE_ARRAY == settings->GetNodeType() )
        return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, "override_write needs the settings (an object)" );
    CORE::CDataNode content( *settings );
    content.SetName( CORE::CString::Empty );
    content.SetNodeType( GUCEF_DATATYPE_OBJECT );

    // the asset as a run would take it now: its importer and its settings
    ASSETCONVERTER::CAssetPipelineOptions options;
    options.assetGlobs.push_back( call.request.assetPath );
    ASSETCONVERTER::CAssetPipeline pipeline;
    ASSETCONVERTER::CAssetPackPlan plan;
    pipeline.PlanPack( pack.manifest, options, plan );
    const ASSETCONVERTER::CAssetPlanEntry* entry = plan.FindEntry( call.request.assetPath );
    if ( GUCEF_NULL == entry )
        return call.Fail( CPipelineOperationCall::ErrorNotFound, "the asset " + call.request.assetPath + " is not one the pack converts" );
    ASSETCONVERTER::TAssetImporterPtr importer;
    CORE::CDataNode defaults( "settings", GUCEF_DATATYPE_OBJECT );
    if ( ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( entry->importer, importer ) )
        importer->GetDefaultSettings( defaults );

    CORE::CString::StringVector warnings;
    CORE::CString::StringVector unknown;
    for ( CORE::CDataNode::TAttributeMap::const_iterator a=content.AttributeBegin(); a!=content.AttributeEnd(); ++a )
        if ( GUCEF_NULL == defaults.GetAttribute( (*a).first ) && GUCEF_NULL == defaults.FindChild( (*a).first ) )
            unknown.push_back( (*a).first );
    for ( CORE::CDataNode::const_iterator c=content.ConstBegin(); c!=content.ConstEnd(); ++c )
        if ( GUCEF_NULL == defaults.GetAttribute( (*c)->GetName() ) && GUCEF_NULL == defaults.FindChild( (*c)->GetName() ) )
            unknown.push_back( (*c)->GetName() );
    for ( size_t u=0; u<unknown.size(); ++u )
        warnings.push_back( "the importer " + entry->importer + " has no setting " + unknown[ u ] + " (it may be the settings of a post-processor or validator)" );

    // the settings after the change: the layers without the override, then the new override over them
    ASSETCONVERTER::CAssetPackManifest withoutOverrides = pack.manifest;
    withoutOverrides.overrides.Clear();
    CORE::CDataNode effective( "settings", GUCEF_DATATYPE_OBJECT );
    CORE::CString::StringVector layers;
    ASSETCONVERTER::CImportSettingsBuilder::Build( defaults, withoutOverrides, call.request.assetPath, entry->importer, effective, layers );
    ASSETCONVERTER::CImportSettingsBuilder::Merge( effective, content );

    const ASSETSTORE::TAssetStoreStatus status = Preview( call, changes, targetVfs, rooted, ASSETCONVERTER::CCanonicalJson::ToJson( content ), false, warnings );
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status )
        return status;
    CORE::CDataNode& document = call.Document();
    document.SetValuesOfChildByName( "unknownKeys", unknown );
    CORE::CDataNode* after = document.AddChild( effective );
    after->SetName( "effectiveSettings" );
    after->SetNodeType( GUCEF_DATATYPE_OBJECT );
    document.SetAttribute( "wouldReimport", ASSETCONVERTER::CCanonicalJson::Hash( effective ) != ASSETCONVERTER::CCanonicalJson::Hash( entry->settings ) );
    return status;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineAuthoringOperations::OverrideRemove( CPipelineOperationCall& call, CPendingChangeStore& changes )
{GUCEF_TRACE;

    CConverterPack pack;
    if ( !call.FindRequestPack( pack, true ) )
        return call.result.status;
    CORE::CString targetVfs;
    CORE::CString rooted;
    if ( !ResolveOverride( call, pack, targetVfs, rooted ) )
        return call.result.status;
    if ( call.request.apply )
        return Apply( call, changes, targetVfs );
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().FileExists( targetVfs ) )
        return call.Fail( CPipelineOperationCall::ErrorNotFound, "the asset " + call.request.assetPath + " has no override" );
    CORE::CString::StringVector warnings;
    return Preview( call, changes, targetVfs, rooted, CORE::CString::Empty, true, warnings );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
