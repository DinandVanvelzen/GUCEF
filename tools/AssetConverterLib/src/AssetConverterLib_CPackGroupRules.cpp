/*
 *  AssetConverterLib: assigns the function group, sub-group and kind of the resources of a pack
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

#include <algorithm>

#ifndef ASSETCONVERTERLIB_CPACKGROUPVOCABULARY_H
#include "AssetConverterLib_CPackGroupVocabulary.h"
#define ASSETCONVERTERLIB_CPACKGROUPVOCABULARY_H
#endif /* ASSETCONVERTERLIB_CPACKGROUPVOCABULARY_H ? */

#ifndef ASSETCONVERTERLIB_CPACKNAMINGCONVENTIONS_H
#include "AssetConverterLib_CPackNamingConventions.h"
#define ASSETCONVERTERLIB_CPACKNAMINGCONVENTIONS_H
#endif /* ASSETCONVERTERLIB_CPACKNAMINGCONVENTIONS_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#include "AssetConverterLib_CPackIndexSerial.h"
#define ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXSERIAL_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetConverterLib_CPackGroupRules.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Folders that repeat the function or are engine roots or source folders: in normalised form, singular and plural */
const char* const StructuralSegments[] =
{
    "assets", "asset", "resources", "resource", "content", "contents", "res", "data", "source", "sources", "src", "raw", "export", "exports",
    "models", "model", "meshes", "mesh", "geometry", "textures", "texture", "tex", "materials", "material", "mats", "mat", "prefabs", "prefab",
    "animations", "animation", "anims", "anim", "audio", "sounds", "sound", "sfx", "music", "scenes", "scene", "shaders", "shader", "fonts", "font",
    "fbx", "obj", "gltf", "glb", "png", "jpg", "tga", "dds", "wav", "ogg", "mp3"
};

template< typename T, size_t N > size_t CountOf( T (&)[ N ] ) { return N; }

bool
StartsWithCI( const CORE::CString& lowerText, const CORE::CString& lowerPrefix )
{
    return lowerText.Length() >= lowerPrefix.Length() && lowerText.SubstrFromRange( 0, lowerPrefix.Length() ) == lowerPrefix;
}

bool
IsHiddenOrOs( const CORE::CString& path )
{
    // a folder or file whose name starts with a dot is hidden; the files the operating systems and tools leave behind
    CORE::CString::StringVector parts = path.ParseElements( '/', false );
    for ( size_t i=0; i<parts.size(); ++i )
    {
        if ( parts[ i ].Length() > 0 && '.' == parts[ i ][ 0 ] )
            return true;
        if ( parts[ i ].Length() > 0 && '~' == parts[ i ][ parts[ i ].Length()-1 ] && i+1 < parts.size() )
            return true;    // a folder that ends in '~' is ignored by the engines that use the convention
    }
    const CORE::CString name = parts.empty() ? CORE::CString::Empty : parts[ parts.size()-1 ].Lowercase();
    return "thumbs.db" == name || "desktop.ini" == name || "ehthumbs.db" == name || ( name.Length() > 1 && '~' == name[ 0 ] );
}

}

/*-------------------------------------------------------------------------*/

CPackGroupRules::CPackGroupRules( const CAssetKindRegistry& kinds )
    : m_kinds( kinds )
    , m_profile()
    , m_ruleOrder()
    , m_hintSegments()
    , m_rootSegments()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

namespace {

struct SRuleOrder
{
    const CPackGroupRule::TRuleVector* rules;
    bool operator()( size_t a, size_t b ) const
    {
        const Int32 pa = (*rules)[ a ].priority;
        const Int32 pb = (*rules)[ b ].priority;
        return pa != pb ? pa > pb : a < b;
    }
};

}

void
CPackGroupRules::SetProfile( const CPackIndexProfile& profile )
{GUCEF_TRACE;

    m_profile = profile;
    m_ruleOrder.clear();
    for ( size_t i=0; i<m_profile.groupRules.size(); ++i )
        m_ruleOrder.push_back( i );
    SRuleOrder order;
    order.rules = &m_profile.groupRules;
    std::stable_sort( m_ruleOrder.begin(), m_ruleOrder.end(), order );
    m_hintSegments.clear();
    for ( size_t h=0; h<m_profile.groupingHints.size(); ++h )
    {
        CORE::CString::StringVector parts = CVfsPaths::Normalize( m_profile.groupingHints[ h ] ).ParseElements( '/', false );
        for ( size_t p=0; p<parts.size(); ++p )
            if ( !parts[ p ].IsNULLOrEmpty() )
                m_hintSegments.insert( NormaliseSegment( parts[ p ] ) );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackGroupRules::NormaliseSegment( const CORE::CString& segment )
{GUCEF_TRACE;

    CORE::CString result;
    const UInt32 length = segment.Length();
    bool pendingDash = false;
    for ( UInt32 i=0; i<length; ++i )
    {
        const char c = segment[ i ];
        const bool upper = c >= 'A' && c <= 'Z';
        const bool lower = c >= 'a' && c <= 'z';
        const bool digit = c >= '0' && c <= '9';
        if ( !upper && !lower && !digit )
        {
            pendingDash = !result.IsNULLOrEmpty();
            continue;
        }
        if ( upper && i > 0 )
        {
            const char prev = segment[ i-1 ];
            const char next = i+1 < length ? segment[ i+1 ] : '\0';
            if ( ( prev >= 'a' && prev <= 'z' ) || ( prev >= '0' && prev <= '9' ) || ( prev >= 'A' && prev <= 'Z' && next >= 'a' && next <= 'z' ) )
                pendingDash = !result.IsNULLOrEmpty();
        }
        if ( pendingDash )
        {
            result += '-';
            pendingDash = false;
        }
        result += (char) ( upper ? c + ( 'a' - 'A' ) : c );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CPackGroupRules::IsStructuralSegment( const CORE::CString& normalisedSegment )
{GUCEF_TRACE;

    for ( size_t i=0; i<CountOf( StructuralSegments ); ++i )
        if ( normalisedSegment == StructuralSegments[ i ] )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CPackGroupRules::GetFolderSegments( const CORE::CString& path, CORE::CString::StringVector& segments ) const
{GUCEF_TRACE;

    segments.clear();
    const CORE::CString folder = CVfsPaths::FolderOf( path );
    if ( folder.IsNULLOrEmpty() )
        return;
    CORE::CString::StringVector parts = folder.ParseElements( '/', false );
    for ( size_t i=0; i<parts.size(); ++i )
    {
        if ( parts[ i ].IsNULLOrEmpty() )
            continue;
        const CORE::CString normalised = NormaliseSegment( parts[ i ] );
        if ( normalised.IsNULLOrEmpty() || IsStructuralSegment( normalised ) || m_hintSegments.find( normalised ) != m_hintSegments.end() )
            continue;
        segments.push_back( normalised );
    }
}

/*-------------------------------------------------------------------------*/

void
CPackGroupRules::Prepare( const CORE::CString::StringVector& paths )
{GUCEF_TRACE;

    m_rootSegments.clear();
    TSegmentLists lists;
    lists.reserve( paths.size() );
    for ( size_t i=0; i<paths.size(); ++i )
    {
        CORE::CString::StringVector segments;
        GetFolderSegments( paths[ i ], segments );
        lists.push_back( segments );
    }

    // the folder that holds nearly everything below the structural ones is the folder of the pack itself: no sub-group says anything with it
    for ( UInt32 depth=0; depth<3; ++depth )
    {
        typedef GUCEF::map< CORE::CString, UInt32 > TCountMap;
        TCountMap counts;
        UInt32 withSegment = 0;
        for ( size_t i=0; i<lists.size(); ++i )
        {
            if ( lists[ i ].size() > depth )
            {
                bool underRoots = true;
                for ( size_t r=0; r<m_rootSegments.size() && underRoots; ++r )
                    underRoots = lists[ i ][ r ] == m_rootSegments[ r ];
                if ( underRoots )
                {
                    ++counts[ lists[ i ][ depth ] ];
                    ++withSegment;
                }
            }
        }
        CORE::CString best;
        UInt32 bestCount = 0;
        TCountMap::const_iterator c = counts.begin();
        while ( c != counts.end() )
        {
            if ( (*c).second > bestCount )
            {
                best = (*c).first;
                bestCount = (*c).second;
            }
            ++c;
        }
        if ( bestCount < 2 || bestCount * 10 < withSegment * 9 )
            break;
        m_rootSegments.push_back( best );
    }
}

/*-------------------------------------------------------------------------*/

bool
CPackGroupRules::IsNoise( const CORE::CString& path ) const
{GUCEF_TRACE;

    if ( IsHiddenOrOs( path ) )
        return true;
    for ( size_t i=0; i<m_profile.noisePatterns.size(); ++i )
        if ( path.WildcardEquals( m_profile.noisePatterns[ i ], '*', false ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPackGroupRules::RuleMatches( const CPackGroupRule& rule, const CORE::CString& lowerPath, const CORE::CString::StringVector& tokens, const CORE::CString& kindId ) const
{GUCEF_TRACE;

    if ( !rule.kind.IsNULLOrEmpty() && rule.kind != kindId )
        return false;
    if ( !rule.glob.IsNULLOrEmpty() && !lowerPath.WildcardEquals( rule.glob.Lowercase(), '*', true ) )
        return false;
    if ( !rule.folder.IsNULLOrEmpty() )
    {
        CORE::CString folder = CVfsPaths::Normalize( rule.folder ).Lowercase();
        while ( folder.Length() > 0 && '/' == folder[ 0 ] )
            folder = folder.SubstrFromRange( 1, folder.Length() );
        while ( folder.Length() > 0 && '/' == folder[ folder.Length()-1 ] )
            folder = folder.SubstrFromRange( 0, folder.Length()-1 );
        if ( folder.IsNULLOrEmpty() )
            return false;
        if ( !StartsWithCI( lowerPath, folder + "/" ) && lowerPath.HasSubstr( "/" + folder + "/" ) < 0 )
            return false;
    }
    if ( !rule.token.IsNULLOrEmpty() )
    {
        const CORE::CString token = rule.token.Lowercase();
        if ( std::find( tokens.begin(), tokens.end(), token ) == tokens.end() )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CPackGroupRules::Assign( CPackIndexResource& resource ) const
{GUCEF_TRACE;

    resource.name = CVfsPaths::NameOf( resource.path );
    resource.format = CAssetKindRegistry::GetExtensionOfPath( resource.path ).Lowercase();
    const EAssetKind kind = m_kinds.GetKindOfPath( resource.path );
    resource.kind = CAssetKindRegistry::GetKindId( kind );
    resource.group.Clear();
    resource.subGroup.Clear();
    resource.groupRule.Clear();

    CORE::CString::StringVector tokens;
    CPackNamingConventions::SplitTokens( CVfsPaths::StemOf( resource.name ), tokens );
    resource.title = CPackNamingConventions::MakeTitle( resource.name );
    resource.storeKind = !tokens.empty() ? CPackNamingConventions::GetStoreKind( tokens[ 0 ], kind ) : CORE::CString::Empty;

    // P: the explicit rules of the profile, the highest priority first
    CORE::CString subSource;
    const CORE::CString lowerPath = resource.path.Lowercase();
    for ( size_t r=0; r<m_ruleOrder.size(); ++r )
    {
        const CPackGroupRule& rule = m_profile.groupRules[ m_ruleOrder[ r ] ];
        if ( !RuleMatches( rule, lowerPath, tokens, resource.kind ) )
            continue;
        if ( !rule.group.IsNULLOrEmpty() )
            resource.group = rule.group;
        if ( !rule.subGroup.IsNULLOrEmpty() )
        {
            resource.subGroup = rule.subGroup;
            subSource = "profile";
        }
        resource.groupRule = "profile:" + CORE::UInt32ToString( (UInt32) m_ruleOrder[ r ] );
        break;
    }

    // K: the kind registry
    if ( resource.group.IsNULLOrEmpty() )
    {
        const CORE::CString& group = CPackGroupVocabulary::GetGroupOfKind( kind );
        resource.group = group.IsNULLOrEmpty() ? CPackGroupVocabulary::Other : group;
        if ( resource.groupRule.IsNULLOrEmpty() )
            resource.groupRule = "kind";
    }

    // the prefix of a name (SM_, SK_, T_, MI_, A_, ...) says the function of a file whose kind says little: a compiled resource of an engine, a package asset
    if ( "kind" == resource.groupRule && tokens.size() > 1 && CPackNamingConventions::IsPrefixToken( tokens[ 0 ] ) )
    {
        const CORE::CString prefixGroup = CPackNamingConventions::GetGroupOfPrefix( tokens[ 0 ], kind );
        if ( !prefixGroup.IsNULLOrEmpty() )
        {
            resource.group = prefixGroup;
            resource.groupRule = "prefix";
        }
    }

    // F: the folders
    if ( resource.subGroup.IsNULLOrEmpty() )
    {
        CORE::CString::StringVector segments;
        GetFolderSegments( resource.path, segments );
        size_t start = 0;
        while ( start < segments.size() && start < m_rootSegments.size() && segments[ start ] == m_rootSegments[ start ] )
            ++start;
        CORE::CString id;
        for ( size_t s=start; s<segments.size() && s-start < MaxSubGroupDepth; ++s )
        {
            if ( !id.IsNULLOrEmpty() )
                id += '/';
            id += segments[ s ];
        }
        if ( !id.IsNULLOrEmpty() )
        {
            resource.subGroup = id;
            subSource = "folder";
        }
    }

    // T: the name tokens, when there is no sub-group or only a generic one
    if ( resource.subGroup.IsNULLOrEmpty() || CPackNamingConventions::IsGenericSubGroup( resource.subGroup ) )
    {
        size_t first = 0;
        while ( first < tokens.size() && CPackNamingConventions::IsPrefixToken( tokens[ first ] ) )
            ++first;
        for ( size_t t=first; t<tokens.size() && t<first+3; ++t )
        {
            const CORE::CString sub = CPackNamingConventions::GetSubGroupOfToken( tokens[ t ] );
            if ( !sub.IsNULLOrEmpty() )
            {
                resource.subGroup = sub;
                subSource = "token";
                break;
            }
        }
    }
    if ( !subSource.IsNULLOrEmpty() )
        resource.groupRule += "+" + subSource;
}

/*-------------------------------------------------------------------------*/

void
CPackGroupRules::Refine( CPackIndexResource& resource ) const
{GUCEF_TRACE;

    // only what the kind registry placed: an explicit rule of the profile is not overruled by a statistic
    if ( !resource.groupRule.WildcardEquals( "kind*", '*', true ) )
        return;
    UInt64 bones = 0;
    if ( "model" == resource.kind && resource.GetStat( "bones", bones ) && bones > 0 )
    {
        resource.group = CPackGroupVocabulary::Characters;
        resource.groupRule = "refine:skinned" + resource.groupRule.SubstrFromRange( 4, resource.groupRule.Length() );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackGroupRules::GetRulesHash( void ) const
{GUCEF_TRACE;

    CORE::CDataNode node;
    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "version", Version );
    node.SetAttribute( "domainTokens", CPackNamingConventions::GetDomainTokenCount() );
    CORE::CDataNode* rules = node.AddChild( "groupRules", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<m_profile.groupRules.size(); ++i )
        m_profile.groupRules[ i ].Serialize( *rules->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    CPackIndexSerial::SetList( node, "groupingHints", m_profile.groupingHints );
    return CCanonicalJson::Hash( node );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
