/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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
#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */
#ifndef GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#include "gucefAUTH_CMemoryCredentialStore.h"
#define GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H ? */
#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */
#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H
#include "gucefASSETSTORE_CAssetStoreSafeNames.h"
#define GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETDIGEST_H
#include "gucefASSETSTORE_CAssetDigest.h"
#define GUCEF_ASSETSTORE_CASSETDIGEST_H
#endif /* GUCEF_ASSETSTORE_CASSETDIGEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETKEYWORDNORMALIZER_H
#include "gucefASSETSTORE_CAssetKeywordNormalizer.h"
#define GUCEF_ASSETSTORE_CASSETKEYWORDNORMALIZER_H
#endif /* GUCEF_ASSETSTORE_CASSETKEYWORDNORMALIZER_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETNAMEHINTS_H
#include "gucefASSETSTORE_CAssetNameHints.h"
#define GUCEF_ASSETSTORE_CASSETNAMEHINTS_H
#endif /* GUCEF_ASSETSTORE_CASSETNAMEHINTS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETVOCABULARY_H
#include "gucefASSETSTORE_CAssetVocabulary.h"
#define GUCEF_ASSETSTORE_CASSETVOCABULARY_H
#endif /* GUCEF_ASSETSTORE_CASSETVOCABULARY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREITEMNORMALIZER_H
#include "gucefASSETSTORE_CAssetStoreItemNormalizer.h"
#define GUCEF_ASSETSTORE_CASSETSTOREITEMNORMALIZER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREITEMNORMALIZER_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETDISKPACKNORMALIZER_H
#include "gucefASSETSTORE_CAssetDiskPackNormalizer.h"
#define GUCEF_ASSETSTORE_CASSETDISKPACKNORMALIZER_H
#endif /* GUCEF_ASSETSTORE_CASSETDISKPACKNORMALIZER_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETIMAGEHEADERPROBE_H
#include "gucefASSETSTORE_CAssetImageHeaderProbe.h"
#define GUCEF_ASSETSTORE_CASSETIMAGEHEADERPROBE_H
#endif /* GUCEF_ASSETSTORE_CASSETIMAGEHEADERPROBE_H ? */
#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H
#include "gucefASSETSTORE_CPrimaryAssetStoreSettings.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOB_H
#include "gucefASSETSTORE_CAssetStoreDownloadJob.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOB_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOB_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#include "gucefASSETSTORE_CAssetStoreItem.h"
#define GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREITEM_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSEARCHQUERY_H
#include "gucefASSETSTORE_CAssetSearchQuery.h"
#define GUCEF_ASSETSTORE_CASSETSEARCHQUERY_H
#endif /* GUCEF_ASSETSTORE_CASSETSEARCHQUERY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#include "gucefASSETSTORE_CAssetStoreJson.h"
#define GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREJSON_H ? */
#ifndef TESTASSETSTORESUPPORT_H
#include "TestAssetStoreSupport.h"
#define TESTASSETSTORESUPPORT_H
#endif /* TESTASSETSTORESUPPORT_H ? */
#include "TestNormalisation.h"

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace GUCEF::ASSETSTORE;
using namespace ASSETSTORETEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
HasKeyword( const TAssetKeywordVector& keywords, const CString& keyword )
{
    for ( size_t k=0; k<keywords.size(); ++k )
        if ( keywords[ k ].keyword == keyword )
            return true;
    return false;
}

bool
SameWords( const CString::StringVector& words, const char* expected )
{
    return CString( expected ).ParseElements( ' ', false ) == words;
}

CString
NormalizedQuery( const CAssetKeywordNormalizer& normalizer, const CString& text )
{
    CString::StringVector words;
    normalizer.NormalizeQuery( text, words );
    CString joined;
    for ( size_t w=0; w<words.size(); ++w )
        joined += ( w > 0 ? " " : "" ) + words[ w ];
    return joined;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      FUNCTIONS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformNormalisationTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "Normalisation" );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    vfs.MountModuleData( "gucefASSETSTORE" );
    CAssetVocabulary vocabulary;
    CString seedError;
    const bool seedLoaded = vocabulary.LoadSeed( "gucefASSETSTORE", seedError );

    GUCEF_TESTFW_TESTCASE( "Safe names: names that stay inside their folder on every platform pass, the others are refused" )
        try
        {
            ASSERT_TRUE( CAssetStoreSafeNames::IsSafeRelativeName( "Models/SM_Sword.fbx" ) );
            ASSERT_TRUE( CAssetStoreSafeNames::IsSafeRelativeName( "Textures\\T_Wall_D.png" ) );
            ASSERT_TRUE( CAssetStoreSafeNames::IsSafeRelativeName( "a..b/c" ) );
            ASSERT_FALSE( CAssetStoreSafeNames::IsSafeRelativeName( "" ) );
            ASSERT_FALSE( CAssetStoreSafeNames::IsSafeRelativeName( "../x" ) );
            ASSERT_FALSE( CAssetStoreSafeNames::IsSafeRelativeName( "a/../../x" ) );
            ASSERT_FALSE( CAssetStoreSafeNames::IsSafeRelativeName( "/etc/passwd" ) );
            ASSERT_FALSE( CAssetStoreSafeNames::IsSafeRelativeName( "\\\\server\\share" ) );
            ASSERT_FALSE( CAssetStoreSafeNames::IsSafeRelativeName( "C:/Windows/x.dll" ) );
            ASSERT_FALSE( CAssetStoreSafeNames::IsSafeRelativeName( "a/./b" ) );
            ASSERT_FALSE( CAssetStoreSafeNames::IsSafeRelativeName( "a//b" ) );
            ASSERT_FALSE( CAssetStoreSafeNames::IsSafeRelativeName( "CON" ) );
            ASSERT_FALSE( CAssetStoreSafeNames::IsSafeRelativeName( "dir/com1.txt" ) );
            ASSERT_FALSE( CAssetStoreSafeNames::IsSafeRelativeName( "dir/name." ) );
            ASSERT_FALSE( CAssetStoreSafeNames::IsSafeRelativeName( "a\tb" ) );
            ASSERT_FALSE( CAssetStoreSafeNames::IsSafeRelativeName( "what?.txt" ) );
            ASSERT_TRUE( CAssetStoreSafeNames::MakeSafePart( "Sword: The <Best>?" ) == "Sword_ The _Best__" );
            ASSERT_TRUE( CAssetStoreSafeNames::MakeSafePart( "..." ) == "_" );
            ASSERT_TRUE( CAssetStoreSafeNames::MakeSafePart( "nul" ) == "_nul" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Digests: SHA-1 and SHA-256 of the FIPS 180 vectors, in one pass and in pieces" )
        try
        {
            CAssetDigest sha256( CAssetDigest::Sha256 );
            sha256.Update( "abc", 3 );
            ASSERT_TRUE( sha256.FinalHex() == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" );
            CAssetDigest sha1( "SHA1" );
            sha1.Update( "a", 1 );
            sha1.Update( "bc", 2 );
            ASSERT_TRUE( sha1.FinalHex() == "a9993e364706816aba3e25717850c26c9cd0d89d" );
            ASSERT_TRUE( CAssetDigest::IsSupportedAlgorithm( "sha256" ) && !CAssetDigest::IsSupportedAlgorithm( "md5" ) );
            ASSERT_TRUE( CAssetDigest::AreEqual( "ABCDEF", "abcdef" ) && !CAssetDigest::AreEqual( "", "" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Keywords: folding, accents, separators, camel case and digits split, stop words, stemming and synonyms" )
        try
        {
            CString::StringVector words;
            CAssetKeywordNormalizer::SplitWords( "SM_Bld_Apartment_01", words );
            ASSERT_TRUE( SameWords( words, "SM Bld Apartment 01" ) );
            CAssetKeywordNormalizer::SplitWords( "metalCrateHTTPServer2x Wall01", words );
            ASSERT_TRUE( SameWords( words, "metal Crate HTTP Server 2 x Wall 01" ) );
            const CString folded = CAssetKeywordNormalizer::Fold( "\xC3\x89p\xC3\xA9" "e Gr\xC3\xB6\xC3\x9F" "e \xC3\x86ther" );
            if ( folded != "epee grosse aether" )
                GUCEF_LOG( LOGLEVEL_NORMAL, "Folded: \"" << folded << "\"" );
            ASSERT_TRUE( folded == "epee grosse aether" );
            ASSERT_TRUE( CAssetKeywordNormalizer::Stem( "crates" ) == "crate" && CAssetKeywordNormalizer::Stem( "boxes" ) == "box" );
            ASSERT_TRUE( CAssetKeywordNormalizer::Stem( "bodies" ) == "body" && CAssetKeywordNormalizer::Stem( "torches" ) == "torch" );
            ASSERT_TRUE( CAssetKeywordNormalizer::Stem( "glass" ) == "glass" && CAssetKeywordNormalizer::Stem( "bus" ) == "bus" && CAssetKeywordNormalizer::Stem( "axis" ) == "axis" );

            ASSERT_TRUE( seedLoaded );
            const CAssetKeywordNormalizer& normalizer = vocabulary.GetKeywordNormalizer();
            ASSERT_TRUE( NormalizedQuery( normalizer, "SM_Bld_Apartment_01" ) == "building apartment" );
            ASSERT_TRUE( NormalizedQuery( normalizer, "The Veh Pack of Swords" ) == "vehicle sword" );
            ASSERT_TRUE( NormalizedQuery( normalizer, "LowPoly lowpoly" ) == "low poly low-poly" );
            TAssetKeywordVector keywords;
            ASSERT_TRUE( 2 == normalizer.Normalize( "Rusty Barrels", "title", 10, keywords ) );
            ASSERT_TRUE( 0 == normalizer.Normalize( "barrel", "title", 3, keywords ) );
            ASSERT_TRUE( 1 == normalizer.Normalize( "barrel", "tag", 5, keywords ) );
            ASSERT_TRUE( 3 == keywords.size() && "rusty" == keywords[ 0 ].keyword && "barrel" == keywords[ 1 ].keyword && "Barrels" == keywords[ 1 ].original && 10 == keywords[ 1 ].weight );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Name hints: kinds, formats, texture roles, levels of detail and collision meshes from names" )
        try
        {
            // the archives of a pack that its content is read from: the packages of the engines and of the character tools too
            ASSERT_TRUE( CAssetNameHints::IsArchiveExtension( "zip" ) && CAssetNameHints::IsArchiveExtension( "UnityPackage" ) && CAssetNameHints::IsArchiveExtension( "MHPKG" ) );
            ASSERT_FALSE( CAssetNameHints::IsArchiveExtension( "fbx" ) || CAssetNameHints::IsArchiveExtension( CString::Empty ) );
            ASSERT_TRUE( CAssetNameHints::GuessKind( "Meshes/SK_Hero.fbx" ) == "skeletal-model" );
            ASSERT_TRUE( CAssetNameHints::GuessKind( "Meshes/SM_Rock.FBX" ) == "static-model" );
            ASSERT_TRUE( CAssetNameHints::GuessKind( "Anims/A_Hero_Run.fbx" ) == "animation" );
            ASSERT_TRUE( CAssetNameHints::GuessKind( "Audio/Music/theme.ogg" ) == "music" );
            ASSERT_TRUE( CAssetNameHints::GuessKind( "Audio/hit.wav" ) == "sound-effect" );
            ASSERT_TRUE( CAssetNameHints::GuessKind( "sky_hdri.exr" ) == "hdri" );
            ASSERT_TRUE( CAssetNameHints::GuessKind( "readme.txt" ) == "document" );
            ASSERT_TRUE( CAssetNameHints::FormatFromExtension( "glb" ) == "gltf" && CAssetNameHints::FormatFromExtension( "usdz" ) == "usd" );
            ASSERT_TRUE( CAssetNameHints::FormatFromExtension( "uasset" ) == "unreal" && CAssetNameHints::GetExtension( "a/b.c/T_X.PNG" ) == "png" );
            ASSERT_TRUE( CAssetNameHints::GuessTextureRole( "T_Wall_N" ) == "normal" && CAssetNameHints::GuessTextureRole( "T_Wall_BaseColor.png" ) == "albedo" );
            ASSERT_TRUE( CAssetNameHints::GuessTextureRole( "Wall_ORM" ) == "orm" && CAssetNameHints::GuessTextureRole( "logo" ).IsNULLOrEmpty() );
            ASSERT_TRUE( 2 == CAssetNameHints::GetLodIndex( "SM_Rock_LOD2" ) && -1 == CAssetNameHints::GetLodIndex( "SM_Rock" ) );
            ASSERT_TRUE( CAssetNameHints::IsCollisionName( "UCX_SM_Rock" ) && !CAssetNameHints::IsCollisionName( "SM_Rock" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Vocabulary: store categories map through their parents, unknown ones do not, rules classify by keyword, name and seller" )
        try
        {
            ASSERT_TRUE( seedLoaded );
            CAssetClassification classification;
            ASSERT_TRUE( vocabulary.MapStoreCategory( "fab", "3D / Weapons & Combat / Melee", classification ) );
            ASSERT_TRUE( 1 == classification.categories.size() && "weapons" == *classification.categories.begin() );
            CAssetClassification environment;
            ASSERT_TRUE( vocabulary.MapStoreCategory( "FAB", "3D/Environments/Fantasy", environment ) );
            ASSERT_TRUE( environment.themes.end() != environment.themes.find( "fantasy" ) );
            CAssetClassification audio;
            ASSERT_TRUE( vocabulary.MapStoreCategory( "unity", "Audio/Music", audio ) && "music" == audio.kind );
            CAssetClassification unknown;
            ASSERT_FALSE( vocabulary.MapStoreCategory( "fab", "Something Else", unknown ) );
            ASSERT_FALSE( vocabulary.MapStoreCategory( "nostore", "3D/Weapons & Combat", unknown ) );
            ASSERT_TRUE( vocabulary.GetMappingCount( "sketchfab" ) >= 14 && vocabulary.GetMappingCount( "godot" ) >= 7 && vocabulary.GetMappingCount( "cgtrader" ) >= 6 );

            CString::StringSet keywords;
            keywords.insert( "sword" );
            keywords.insert( "weapon" );
            CAssetClassification byKeyword;
            ASSERT_TRUE( vocabulary.ApplyRules( keywords, CString::StringVector(), CString::Empty, byKeyword ) );
            ASSERT_TRUE( 1 == byKeyword.categories.size() && "weapons/melee" == *byKeyword.categories.begin() );
            CString::StringVector names;
            names.push_back( "SM_Veh_Car_01" );
            CAssetClassification byName;
            ASSERT_TRUE( vocabulary.ApplyRules( CString::StringSet(), names, CString::Empty, byName ) && byName.categories.end() != byName.categories.find( "vehicles" ) );
            CAssetClassification bySeller;
            ASSERT_TRUE( vocabulary.ApplyRules( CString::StringSet(), CString::StringVector(), "Synty Studios", bySeller ) );
            ASSERT_TRUE( bySeller.artStyles.end() != bySeller.artStyles.find( "low-poly" ) && bySeller.artStyles.end() != bySeller.artStyles.find( "stylised" ) );
            CAssetClassification none;
            ASSERT_FALSE( vocabulary.ApplyRules( CString::StringSet(), CString::StringVector(), "Nobody", none ) );

            ASSERT_TRUE( vocabulary.IsKnownCategory( "buildings/modular/walls" ) && !vocabulary.IsKnownCategory( "kind:texture" ) );
            CString::StringVector categories;
            vocabulary.GetTerms( "category", categories );
            UInt32 topLevel = 0;
            for ( size_t c=0; c<categories.size(); ++c )
                if ( categories[ c ].HasChar( '/' ) < 0 )
                    ++topLevel;
            ASSERT_TRUE( 20 == topLevel );
            CString::StringVector kinds;
            vocabulary.GetTerms( "kind", kinds );
            ASSERT_TRUE( 22 == kinds.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Licences: classes from names, restriction flags from classes, rules on the text and what the store states" )
        try
        {
            CAssetLicence ccBy;
            ccBy.name = "CC BY 4.0";
            vocabulary.ClassifyLicence( ccBy );
            ASSERT_TRUE( "attribution" == ccBy.licenceClass && ccBy.HasRestriction( CAssetLicence::AttributionRequired ) && ccBy.AllowsMachineLearning() );
            ASSERT_TRUE( 0 == ccBy.licenceId.HasSubstr( "lic-" ) && 20 == ccBy.licenceId.Length() );
            CAssetLicence cc0;
            cc0.name = "CC0 1.0";
            vocabulary.ClassifyLicence( cc0 );
            ASSERT_TRUE( "public-domain" == cc0.licenceClass && cc0.restrictions.empty() );
            CAssetLicence nonCommercial;
            nonCommercial.name = "CC BY-NC 4.0";
            vocabulary.ClassifyLicence( nonCommercial );
            ASSERT_TRUE( "noncommercial" == nonCommercial.licenceClass && nonCommercial.HasRestriction( CAssetLicence::NoCommercialUse ) );
            CAssetLicence noTraining;
            noTraining.name = "Standard License";
            noTraining.text = "The assets may not be used for machine learning or to train any model.";
            vocabulary.ClassifyLicence( noTraining );
            ASSERT_TRUE( "royalty-free" == noTraining.licenceClass && !noTraining.AllowsMachineLearning() && noTraining.HasRestriction( CAssetLicence::NoRedistribution ) );
            CAssetLicence stated;
            stated.name = "Own Licence";
            stated.restrictions.insert( CAssetLicence::NoMachineLearning );
            vocabulary.ClassifyLicence( stated );
            ASSERT_TRUE( "unknown" == stated.licenceClass && !stated.AllowsMachineLearning() );
            CAssetLicence keptClass;
            keptClass.licenceClass = "editorial";
            keptClass.name = "CC0";
            vocabulary.ClassifyLicence( keptClass );
            ASSERT_TRUE( "editorial" == keptClass.licenceClass && keptClass.HasRestriction( CAssetLicence::EditorialOnly ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Normalisers: a store item and a pack from disk become pack records with categories, unmapped categories, keywords, licence and styles" )
        try
        {
            CAssetNormalizationInput input;
            input.sourceId = "fab";
            input.item.itemId = "fab-123";
            input.item.title = "Medieval Weapons Pack";
            input.item.seller = "Synty Studios";
            input.item.description = "Swords and shields for your game";
            input.item.licenceName = "Standard License";
            input.item.licenceRestrictions.insert( CAssetLicence::NoMachineLearning );
            input.item.categories.push_back( "3D/Weapons & Combat/Melee" );
            input.item.categories.push_back( "3D/Strange Things" );
            input.item.tags.push_back( "Sword" );
            input.item.tags.push_back( "lowpoly" );
            CAssetStoreItemNormalizer itemNormalizer;
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == itemNormalizer.Normalize( input, vocabulary, pack ) );
            ASSERT_TRUE( pack.title == "Medieval Weapons Pack" && pack.sourceItemId == "fab-123" && pack.normalizer == "storeItem/1" );
            ASSERT_TRUE( pack.categories.end() != pack.categories.find( "weapons" ) );
            ASSERT_TRUE( 1 == pack.unmappedCategories.size() && "3D/Strange Things" == *pack.unmappedCategories.begin() );
            ASSERT_TRUE( pack.themes.end() != pack.themes.find( "medieval" ) );
            ASSERT_TRUE( pack.artStyles.end() != pack.artStyles.find( "low-poly" ) && pack.artStyles.end() != pack.artStyles.find( "stylised" ) );
            ASSERT_TRUE( "royalty-free" == pack.licence.licenceClass && !pack.licence.AllowsMachineLearning() );
            bool titleWeight = false;
            for ( size_t k=0; k<pack.keywords.size(); ++k )
                if ( "weapon" == pack.keywords[ k ].keyword && "title" == pack.keywords[ k ].field && 10 == pack.keywords[ k ].weight )
                    titleWeight = true;
            ASSERT_TRUE( titleWeight && HasKeyword( pack.keywords, "sword" ) && HasKeyword( pack.keywords, "shield" ) );

            CAssetNormalizationInput disk;
            disk.sourceId = "disk";
            disk.packName = "Fantasy_Village-Pack";
            disk.fileNames.push_back( "Meshes/SM_Bld_House_01.fbx" );
            CAssetDiskPackNormalizer diskNormalizer;
            CAssetPackRecord diskPack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == diskNormalizer.Normalize( disk, vocabulary, diskPack ) );
            ASSERT_TRUE( "Fantasy Village Pack" == diskPack.title && "diskPack/1" == diskPack.normalizer );
            ASSERT_TRUE( diskPack.themes.end() != diskPack.themes.find( "fantasy" ) && diskPack.categories.end() != diskPack.categories.find( "buildings" ) );
            ASSERT_TRUE( "unknown" == diskPack.licence.licenceClass );

            disk.hasPackJson = true;
            ASSERT_TRUE( CAssetStoreJson::FromText( "{ \"title\": \"Haunted Crates\", \"author\": \"Somebody\", \"tags\": [ \"crate\", \"horror\" ], "
                                                    "\"licence\": { \"name\": \"CC BY 4.0\", \"restrictions\": [ \"noMachineLearning\" ] } }", disk.packJson ) );
            CAssetPackRecord jsonPack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == diskNormalizer.Normalize( disk, vocabulary, jsonPack ) );
            ASSERT_TRUE( "Haunted Crates" == jsonPack.title && "Somebody" == jsonPack.seller );
            ASSERT_TRUE( jsonPack.categories.end() != jsonPack.categories.find( "furniture-containers/containers" ) );
            ASSERT_TRUE( jsonPack.themes.end() != jsonPack.themes.find( "horror" ) );
            ASSERT_TRUE( "attribution" == jsonPack.licence.licenceClass && !jsonPack.licence.AllowsMachineLearning() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Image headers: size, channels, depth and compression of PNG, TGA, BMP, GIF, JPEG and DDS without decoding" )
        try
        {
            CAssetImageHeaderProbe::SImageInfo info;
            CDynamicBuffer png;
            ASSERT_TRUE( MakePng( 64, 32, false, png ) );
            ASSERT_TRUE( CAssetImageHeaderProbe::Probe( png.AsConstTypePtr< UInt8 >(), png.GetDataSize(), info ) );
            ASSERT_TRUE( 64 == info.width && 32 == info.height && 3 == info.channels && 8 == info.bitDepth && "png" == info.format );
            CDynamicBuffer tga;
            MakeTga( 128, 256, tga );
            ASSERT_TRUE( CAssetImageHeaderProbe::Probe( tga.AsConstTypePtr< UInt8 >(), tga.GetDataSize(), info ) );
            ASSERT_TRUE( 128 == info.width && 256 == info.height && 4 == info.channels && "tga" == info.format );

            UInt8 bmp[ 54 ] = { 'B', 'M' };
            bmp[ 18 ] = 0x00; bmp[ 19 ] = 0x02;                 // 512
            bmp[ 22 ] = 0x00; bmp[ 23 ] = 0x01;                 // 256
            bmp[ 28 ] = 24;
            ASSERT_TRUE( CAssetImageHeaderProbe::Probe( bmp, sizeof( bmp ), info ) && 512 == info.width && 256 == info.height && "bmp" == info.format );

            UInt8 gif[ 13 ] = { 'G', 'I', 'F', '8', '9', 'a', 0x20, 0x00, 0x10, 0x00 };
            ASSERT_TRUE( CAssetImageHeaderProbe::Probe( gif, sizeof( gif ), info ) && 32 == info.width && 16 == info.height );

            UInt8 jpeg[ 32 ] = { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x04, 0x00, 0x00, 0xFF, 0xC0, 0x00, 0x11, 0x08, 0x00, 0x30, 0x00, 0x40, 0x03 };
            ASSERT_TRUE( CAssetImageHeaderProbe::Probe( jpeg, sizeof( jpeg ), info ) );
            ASSERT_TRUE( 64 == info.width && 48 == info.height && 3 == info.channels && "jpeg" == info.format );

            UInt8 dds[ 148 ] = { 'D', 'D', 'S', ' ', 124 };
            dds[ 12 ] = 0x00; dds[ 13 ] = 0x04;                 // height 1024
            dds[ 16 ] = 0x00; dds[ 17 ] = 0x08;                 // width 2048
            dds[ 80 ] = 0x04;
            dds[ 84 ] = 'D'; dds[ 85 ] = 'X'; dds[ 86 ] = 'T'; dds[ 87 ] = '5';
            ASSERT_TRUE( CAssetImageHeaderProbe::Probe( dds, sizeof( dds ), info ) );
            ASSERT_TRUE( 2048 == info.width && 1024 == info.height && "bc3" == info.compression );
            dds[ 84 ] = 'D'; dds[ 85 ] = 'X'; dds[ 86 ] = '1'; dds[ 87 ] = '0';
            dds[ 128 ] = 98;
            ASSERT_TRUE( CAssetImageHeaderProbe::Probe( dds, sizeof( dds ), info ) && "bc7" == info.compression );

            const char notAnImage[] = "this is text";
            ASSERT_FALSE( CAssetImageHeaderProbe::Probe( (const UInt8*) notAnImage, sizeof( notAnImage ), info ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Settings: the password of the database is never saved; it comes from the credential store or a password file" )
        try
        {
            CPrimaryAssetStoreSettings settings;
            ASSERT_TRUE( 1 == settings.locations.size() && "default" == settings.locations[ 0 ].locationId );
            ASSERT_TRUE( "$MODULEDIR$/../../assets/" == settings.locations[ 0 ].fileSystemPath && "vfstables" == settings.database.backendName );
            ASSERT_TRUE( ASSETKEEP_BOTH == settings.GetPolicy( "fab" ).keep && ASSETINGEST_MODE_MANUAL == settings.GetPolicy( "fab" ).mode );

            settings.database.backendOptions.SetAllowMultipleValues( false );
            settings.database.backendOptions.Set( "password", CVariant( "do-not-save" ) );
            settings.database.backendOptions.Set( "schema", CVariant( "assets" ) );
            CAssetSourcePolicy automatic;
            automatic.sourceId = "FAB";
            automatic.mode = ASSETINGEST_MODE_AUTOMATIC;
            automatic.keep = ASSETKEEP_ORIGINAL;
            settings.SetPolicy( automatic );
            CDataNode saved;
            CDataNodeSerializableSettings serializerSettings;
            ASSERT_TRUE( settings.Serialize( saved, serializerSettings ) );
            CString json;
            ASSERT_TRUE( CAssetStoreJson::ToText( saved, json ) );
            ASSERT_TRUE( json.HasSubstr( "do-not-save" ) < 0 && json.HasSubstr( "password" ) < 0 && json.HasSubstr( "assets" ) >= 0 );
            CPrimaryAssetStoreSettings loaded;
            ASSERT_TRUE( loaded.Deserialize( saved, serializerSettings ) );
            ASSERT_TRUE( ASSETINGEST_MODE_AUTOMATIC == loaded.GetPolicy( "fab" ).mode && ASSETKEEP_ORIGINAL == loaded.GetPolicy( "fab" ).keep );

            COMCORE::CDatabaseClientSettings withPassword;
            CString error;
            AUTH::CMemoryCredentialStore credentials;
            loaded.credentialProvider = "gucefASSETSTORE.catalogue";
            loaded.credentialAccount = "assets";
            ASSERT_FALSE( loaded.GetDatabaseSettingsWithCredentials( &credentials, withPassword, error ) );
            ASSERT_TRUE( credentials.Store( "gucefASSETSTORE.catalogue", "assets", AUTH::CSecretString( CString( "from-the-store" ) ) ) );
            ASSERT_TRUE( loaded.GetDatabaseSettingsWithCredentials( &credentials, withPassword, error ) );
            ASSERT_TRUE( "from-the-store" == withPassword.backendOptions.GetValueAlways( CString( "password" ) ).AsString() );

            const CString passwordFile = CombinePath( TestDirectory( "settings" ), "password.txt" );
            CreateDirs( TestDirectory( "settings" ) );
            CDynamicBuffer passwordBytes;
            passwordBytes.CopyFrom( CString( "from-the-file\nsecond line" ), 0, false );
            ASSERT_TRUE( passwordBytes.WriteContentToFile( passwordFile, 0, true ) );
            loaded.credentialProvider.Clear();
            loaded.databasePasswordFile = passwordFile;
            ASSERT_TRUE( loaded.GetDatabaseSettingsWithCredentials( GUCEF_NULL, withPassword, error ) );
            ASSERT_TRUE( "from-the-file" == withPassword.backendOptions.GetValueAlways( CString( "password" ) ).AsString() );
            loaded.databasePasswordFile = CombinePath( TestDirectory( "settings" ), "missing.txt" );
            ASSERT_FALSE( loaded.GetDatabaseSettingsWithCredentials( GUCEF_NULL, withPassword, error ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Types: items, jobs and queries go to data trees and back" )
        try
        {
            CAssetStoreItem item;
            item.providerId = "fab";
            item.itemId = "x1";
            item.title = "Thing";
            item.licenceRestrictions.insert( CAssetLicence::NoMachineLearning );
            item.categories.push_back( "3D/Props" );
            item.storeAttributes.SetAttribute( "rating", (UInt32) 5 );
            CAssetStoreFormat format;
            format.formatId = "gltf";
            CAssetStoreFile file;
            file.fileId = "a";
            file.name = "a.glb";
            file.size = 12;
            file.digest = "ABC";
            file.digestAlgorithm = "SHA256";
            format.files.push_back( file );
            item.formats.push_back( format );
            CDataNode node;
            CDataNodeSerializableSettings serializerSettings;
            ASSERT_TRUE( item.Serialize( node, serializerSettings ) );
            CString json;
            ASSERT_TRUE( CAssetStoreJson::ToText( node, json ) );
            CDataNode parsed;
            ASSERT_TRUE( CAssetStoreJson::FromText( json, parsed ) );
            CAssetStoreItem back;
            ASSERT_TRUE( back.Deserialize( parsed, serializerSettings ) );
            ASSERT_TRUE( "x1" == back.itemId && 1 == back.formats.size() && 1 == back.formats[ 0 ].files.size() && "abc" == back.formats[ 0 ].files[ 0 ].digest );
            ASSERT_TRUE( 12 == back.formats[ 0 ].files[ 0 ].size && "sha256" == back.formats[ 0 ].files[ 0 ].digestAlgorithm );
            ASSERT_TRUE( 1 == back.licenceRestrictions.size() && 1 == back.categories.size() && 5 == back.storeAttributes.GetAttributeValue( "rating" ).AsUInt32() );

            CAssetSearchQuery query;
            query.text = "red barrel";
            query.kinds.insert( "static-model" );
            query.maxTriangles = 5000;
            query.minAnimationSeconds = 1.5;
            query.rigged = 1;
            CDataNode queryNode;
            ASSERT_TRUE( query.Serialize( queryNode, serializerSettings ) );
            CAssetSearchQuery queryBack;
            ASSERT_TRUE( queryBack.Deserialize( queryNode, serializerSettings ) );
            ASSERT_TRUE( "red barrel" == queryBack.text && 1 == queryBack.kinds.size() && 5000 == queryBack.maxTriangles && -1 == queryBack.minTriangles );
            ASSERT_TRUE( 1 == queryBack.rigged && -1 == queryBack.animated && queryBack.minAnimationSeconds > 1.4 && "assets" == queryBack.resultKind );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
