/*
 *  AssetConverterLib: the common naming conventions of asset packs as data: prefix tokens and domain tokens
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

#ifndef ASSETCONVERTERLIB_CPACKNAMINGCONVENTIONS_H
#define ASSETCONVERTERLIB_CPACKNAMINGCONVENTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The conventions asset packs follow in the names of their files, as tables:
 *
 *  - prefix tokens (sm, sk, t, m, ...): the first token of a name that says what the file is, not what it shows. They give the
 *    kind hint of the store (static-model, skeletal-model, texture, material) and are left out of the display title and of the
 *    search for the domain token.
 *  - domain tokens (bld, prop, veh, sign, env, char, fx, ...): a token that says what the file shows; each maps to a sub-group id.
 *
 *  The tables are generic: they hold common abbreviations, no names of vendors, games or products. A pack whose convention is
 *  not in the tables states it in its profile (CPackGroupRule), which goes first.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackNamingConventions
{
    public:

    /** Splits a file name (without directory) into lower case tokens: separators (_ - space . ), camel case and digit runs split; digits only tokens are dropped unless keepDigits */
    static void SplitTokens( const CORE::CString& name, CORE::CString::StringVector& tokens, bool keepDigits = false );

    /** Whether the token is a prefix token (sm, sk, t, m, ...) */
    static bool IsPrefixToken( const CORE::CString& token );

    /** The sub-group id of a domain token; empty when the token is not in the table */
    static CORE::CString GetSubGroupOfToken( const CORE::CString& token );

    /** The vocabulary kind of the store the prefix token and the asset kind say (static-model, skeletal-model, texture, material); empty when they say none */
    static CORE::CString GetStoreKind( const CORE::CString& prefixToken, EAssetKind kind );

    /**
     *  The function group a prefix token says for a file whose kind says little (data, package, other: a compiled resource of an engine,
     *  a package asset): sm and mesh give static-models, sk and skm characters, t and tex textures, m, mi, mat and ma materials, a, anim
     *  and am animations, ps particle systems. Empty when the token or the kind say none.
     */
    static CORE::CString GetGroupOfPrefix( const CORE::CString& prefixToken, EAssetKind kind );

    /** Sub-group ids that say nothing (misc, other, common, ...): the name tokens may replace them */
    static bool IsGenericSubGroup( const CORE::CString& subGroupId );

    /** The display title of a file name: the prefix and the LOD suffix left out, separators as spaces, the first letter in upper case */
    static CORE::CString MakeTitle( const CORE::CString& name );

    /** The number of entries of the domain table (a change of the table is a change of CPackGroupRules::Version) */
    static UInt32 GetDomainTokenCount( void );

    private:

    CPackNamingConventions( void );
    ~CPackNamingConventions();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKNAMINGCONVENTIONS_H ? */

