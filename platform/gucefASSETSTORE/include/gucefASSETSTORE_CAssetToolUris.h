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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLURIS_H
#define GUCEF_ASSETSTORE_CASSETTOOLURIS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The URIs of the resources of the asset tools (section 8 of the asset tools design), scheme "assettools": made by
 *  whoever links to a resource (the tools, the converter hook) and taken apart by whoever serves it. A segment that
 *  comes from data (a pack id, an asset path, a profile) is percent-encoded when the URI is made, because the URI normal
 *  form of the MCP server refuses spaces and other characters raw; Split() decodes the segments again. An asset path
 *  keeps its '/' separators in the URI ({+assetPath} of the templates).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetToolUris
{
    public:

    static const CORE::CString Scheme;          /**< "assettools" */
    static const CORE::CString Prefix;          /**< "assettools://" */

    /** Percent-encodes every byte that is not unreserved (RFC 3986: A-Z a-z 0-9 - . _ ~); '/' too unless keepSlashes */
    static CORE::CString Encode( const CORE::CString& text, bool keepSlashes );

    /** Decodes percent-encoded bytes; false for a broken escape */
    static bool Decode( const CORE::CString& text, CORE::CString& decoded );

    /** "assettools://packs/<packId>/<member>", member for example "manifest", "report", "activity" */
    static CORE::CString ForPack( const CORE::CString& packId, const CORE::CString& member );

    /** "assettools://packs/<packId>/assets/<assetPath>/<member>", member "report", "sidecar" or "override" */
    static CORE::CString ForAsset( const CORE::CString& packId, const CORE::CString& assetPath, const CORE::CString& member );

    /** "assettools://packs/<packId>/cook/<profile>/<member>", member "references" or "sizes" */
    static CORE::CString ForCookProfile( const CORE::CString& packId, const CORE::CString& profile, const CORE::CString& member );

    /** "assettools://docs/<name>" ("manifest-reference") */
    static CORE::CString ForDoc( const CORE::CString& name );

    /** "assettools://docs/import-settings/<importer>" */
    static CORE::CString ForImportSettings( const CORE::CString& importer );

    /** "assettools://jobs/<jobId>/result" */
    static CORE::CString ForJobResult( const CORE::CString& jobId );

    /** "assettools://store/<kind>/<id>" (kind "assets", "packs") and "assettools://store/licences/<id>/text" */
    static CORE::CString ForStoreRecord( const CORE::CString& kind, const CORE::CString& id );
    static CORE::CString ForStoreLicenceText( const CORE::CString& licenceId );

    /**
     *  The segments of an assettools URI after the scheme, decoded: "assettools://packs/a%20b/report" gives "packs",
     *  "a b", "report". False when the URI has another scheme, a query or fragment, an empty segment or a broken escape
     */
    static bool Split( const CORE::CString& uri, CORE::CString::StringVector& segments );

    private:

    CAssetToolUris( void );     /**< not implemented: static functions only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETTOOLURIS_H ? */
