/*
 *  AssetConverterLib: the files of a folder or an archive, for a pack recogniser
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

#ifndef ASSETCONVERTERLIB_CASSETPACKLISTING_H
#define ASSETCONVERTERLIB_CASSETPACKLISTING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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
 *  What a pack recogniser looks at: the relative paths of the files of a folder or archive ('/' as separator), and a way to read
 *  the start of small text files (project.godot, ProjectVersion.txt, a .uproject). A listing is filled from a folder or mounted
 *  archive of the VFS with LoadFromVfs(), or by hand with AddFile() when the files are not at hand (a listing of an archive
 *  that is not mounted, the tests): a file added with text is read from that text.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetPackListing
{
    public:

    CAssetPackListing( void );

    void Clear( void );

    /** Lists the files below a VFS folder (recursively, without folders); files are read from there. False when the folder lists nothing */
    bool LoadFromVfs( const CORE::CString& vfsFolder, UInt32 maxFiles = 200000 );

    /** Adds a file by its path relative to the root of the listing */
    void AddFile( const CORE::CString& path );

    /** Adds a file whose start is this text: it is read from the text, not from the VFS */
    void AddFile( const CORE::CString& path, const CORE::CString& text );

    const CORE::CString::StringVector& GetFiles( void ) const;

    UInt32 GetFileCount( void ) const;

    /** Whether the file is listed; the letter case is ignored when the exact path is not there */
    bool HasFile( const CORE::CString& path ) const;

    /** The listed path of a file, with the letter case it has in the listing; false when it is not listed */
    bool FindFile( const CORE::CString& path, CORE::CString& listedPath ) const;

    /** The files that match a wildcard pattern ('*', letter case ignored), in listing order; returns the number found */
    UInt32 FindFiles( const CORE::CString& pattern, CORE::CString::StringVector& found ) const;

    /**
     *  The files that match a wildcard pattern, the best candidates first: more of the keywords in the file name (letter case ignored)
     *  is better, then fewer folders, then the order of the listing; at most maxCount. Returns the number found
     */
    UInt32 FindFilesRanked( const CORE::CString& pattern, const CORE::CString::StringVector& keywords, CORE::CString::StringVector& found, UInt32 maxCount ) const;

    /**
     *  The files that match a wildcard pattern, the ones of the pack itself first, then the ones that look like a showcase: the order is
     *  1. the folders that carry the words of the name of the pack (GetPackName(): "POLYGON_Shops_Unity_2022_3_v1_6_6" has polygon and shops; version
     *     numbers, the engine and words like pack, lite or source say nothing) score by how many of the words they have, so the folder of the pack
     *     comes before the folder of another pack that is bundled in the same package;
     *  2. the first of the keywords that the file name has (the keywords are in the order of preference: demo, sample, showcase, overview);
     *     a file name without any comes last;
     *  3. the shorter file name (Demo before Demo_Lite), then fewer folders, then the order of the listing.
     *  The same listing always gives the same order. At most maxCount; returns the number found
     */
    UInt32 FindFilesRankedForPack( const CORE::CString& pattern, const CORE::CString::StringVector& keywords, CORE::CString::StringVector& found, UInt32 maxCount ) const;

    /** The name of the pack the listing is of: what SetPackName() gave, else the last part of the VFS folder without its extension ("POLYGON_Shops_Unity_2022_3_v1_6_6") */
    CORE::CString GetPackName( void ) const;
    void SetPackName( const CORE::CString& name );

    /** Sets the folder the files are read from (ReadText(), sizes) for a listing that was built with AddFile() from a list the pipeline already holds */
    void SetVfsRoot( const CORE::CString& vfsRoot );

    /** The words of a name in lower case, split at separators, at a lower to upper case change and at the end of an upper case run: "PolygonShops_01" gives polygon, shops, 01 */
    static void SplitWords( const CORE::CString& name, CORE::CString::StringVector& words );

    /** The folders directly below a folder (empty for the root) that hold files, as paths, sorted: "Assets/Models", "Assets/Scenes" */
    void GetSubFolders( const CORE::CString& folder, CORE::CString::StringVector& folders ) const;

    /** Reads the first bytes (at most maxBytes) of a listed file as text; false when it cannot be read */
    bool ReadText( const CORE::CString& path, CORE::CString& text, UInt32 maxBytes = 65536 ) const;

    /** The text without the white space (space, tab, carriage return, line feed) at both ends; the Trim() of the string stops at spaces and tabs */
    static CORE::CString TrimText( const CORE::CString& text );

    /** The VFS folder the files are read from; empty for a listing that was built by hand */
    const CORE::CString& GetVfsRoot( void ) const;

    private:

    typedef GUCEF::map< CORE::CString, CORE::CString > TTextMap;
    typedef GUCEF::map< CORE::CString, CORE::CString > TLowerMap;

    CORE::CString m_vfsRoot;
    CORE::CString m_packName;
    CORE::CString::StringVector m_files;
    TLowerMap m_lowerToListed;
    TTextMap m_texts;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETPACKLISTING_H ? */
