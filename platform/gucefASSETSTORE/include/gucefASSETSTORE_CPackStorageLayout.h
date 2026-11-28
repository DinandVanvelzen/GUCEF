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

#ifndef GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#define GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H

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
  *  Where the files of a pack are on disk, the one place that knows (section 8 of the design). Inside a storage
 *  location the pack has three areas, each with the folder of the pack below it:
 *
 *    <location>/original/<pack folder>/...          the files of the pack as ingested
 *    <location>/intermediate/<pack folder>/...      the converter's intermediate files, its report, licence record and manifest
 *                                                   and the pack index
 *    <location>/gucef-ogre3d/<pack folder>/...      the cooked runtime files, one folder per cook profile
 *    <location>/staging/<job>/...                   not part of a pack: work in progress of the ingests and downloads
 *    <location>/generations/<pack folder>/previous  the previous generation of the regenerated intermediate and cooked outputs of
 *                                                   the pack, kept for a rollback (CAssetPackGenerations)
 *
 *  The pack folder has two parts, "<site folder>/<pack name>": the folder of the web site the pack came from ("fab.com", the
 *  site name of the store provider, "disk" for packs from a disk) and then the folder of the pack, so the packs of two stores
 *  never meet and the origin of a pack shows on the disk ("original/fab.com/Asian Rice Terrace").
 *
 *  The pack name is the name of the pack's source (the file or folder name including its extension) made safe for
 *  every file system, computed once when the pack is placed and kept in the root path of the pack record: after that
 *  it is never derived from the name again, so nothing moves when a source is renamed. A name that another pack of
 *  the location has already gets ' [<short id>]' appended; the first pack keeps the clean name.
 *
 *  The paths in the rows of the catalogue are logical: they start with the name of the area ("original/...",
 *  "intermediate/...", "gucef-ogre3d/<profile>/...") and Resolve() turns them into VFS paths. The root path of a pack
 *  of the earlier layout (version 1) is a folder of the location that holds "original/" and "converted/<profile>/";
 *  its paths resolve below that folder. The root path of a pack of version 2 is "layout2:<pack folder>". Everything
 *  that needs a path of a pack asks this class.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CPackStorageLayout
{
    public:

    static const UInt32 LayoutFolders = 1;              /**< version 1: packs/<store>/<seller>/<title> [<id>]/original and converted */
    static const UInt32 LayoutAreas = 2;                /**< version 2: the three areas of the location */
    static const UInt32 MaxFolderNameBytes = 120;
    static const UInt32 MaxSiteFolderNameBytes = 60;
    static const UInt32 PackFolderParts = 2;            /**< the pack folder is "<site folder>/<pack name>": two parts below an area */

    static const CORE::CString OriginalArea;            /**< "original" */
    static const CORE::CString IntermediateArea;        /**< "intermediate" */
    static const CORE::CString CookedArea;              /**< "gucef-ogre3d" */
    static const CORE::CString StagingArea;             /**< "staging" */
    static const CORE::CString GenerationsArea;         /**< "generations": the previous generation of the regenerated outputs of a pack (CAssetPackGenerations), not one of the three areas */
    static const CORE::CString RootPathMarker;          /**< "layout2:" */

    // naming of the pack folder

    /** The folder name for the name of a source: unsafe characters replaced, no leading or trailing dots or blanks, no device names of Windows, at most MaxFolderNameBytes with the extension kept; never empty */
    static CORE::CString MakePackFolderName( const CORE::CString& sourceName );

    /** The first eight hexadecimal digits of the id, all of the id when it has fewer */
    static CORE::CString MakeShortId( const CORE::CString& packId );

    /**
     *  The folder for a pack: the clean name when 'takenLowercase' (the folders of the other packs of the location, in
     *  lower case: the shares are not case sensitive everywhere) does not hold it, else 'name [short id]', else
     *  'name [id]'. The same inputs give the same answer.
     */
    static CORE::CString ChoosePackFolderName( const CORE::CString& cleanName, const CORE::CString& packId, const CORE::CString::StringSet& takenLowercase );

    /** The folder of the site of a store: the site name (CIAssetStoreProvider::GetSiteName()) in lower case made safe for every file system; "unknown" when there is none */
    static CORE::CString MakeSiteFolderName( const CORE::CString& siteName );

    /** The pack folder for the folder of a site and the folder name of a pack: "<site folder>/<pack name>" */
    static CORE::CString MakePackFolder( const CORE::CString& siteFolder, const CORE::CString& packName );

    /**
     *  Splits a path relative to the root of a location ("original/fab.com/My Pack/a/b.fbx") into the area, the pack folder
     *  ("fab.com/My Pack") and what is below it ("a/b.fbx"). False when the path does not start with an area name followed by the
     *  parts of a pack folder; 'rest' is empty when the path ends at the pack folder.
     */
    static bool SplitAreaPath( const CORE::CString& relativePath, CORE::CString& area, CORE::CString& packFolder, CORE::CString& rest );

    /** The root path to store in the pack record for a folder of version 2 */
    static CORE::CString MakeRootPath( const CORE::CString& packFolder );

    // the layout of a stored pack

    CPackStorageLayout( void );

    /** From the root path of a pack record: "layout2:<folder>" is version 2, anything else version 1 */
    explicit CPackStorageLayout( const CORE::CString& storedRootPath );

    UInt32 GetVersion( void ) const;
    bool IsAreaLayout( void ) const;

    /** The pack folder of version 2, "<site folder>/<pack name>"; empty for version 1 */
    const CORE::CString& GetPackFolder( void ) const;

    /** The first part of the pack folder: the folder of the site the pack came from ("fab.com"); empty for version 1 */
    CORE::CString GetSiteFolder( void ) const;

    /** The last part of the pack folder: the folder name of the pack itself; empty for version 1 */
    CORE::CString GetPackName( void ) const;

    /** The root path as it is stored in the pack record */
    CORE::CString GetStoredRootPath( void ) const;

    /** Version 1: the folder of the pack in the location; version 2: empty */
    const CORE::CString& GetLegacyRoot( void ) const;

    /** The VFS path of a logical path of the catalogue ("original/a/b.fbx") in the location */
    CORE::CString Resolve( const CORE::CString& locationRoot, const CORE::CString& logicalPath ) const;

    /** The VFS folders (for the cook profile; version 1 has one folder for the intermediate and the cooked files) */
    CORE::CString GetOriginalFolder( const CORE::CString& locationRoot ) const;
    CORE::CString GetIntermediateFolder( const CORE::CString& locationRoot, const CORE::CString& profile ) const;
    CORE::CString GetCookedFolder( const CORE::CString& locationRoot, const CORE::CString& profile ) const;

    /** Version 2: the VFS folder "<location>/generations/<pack folder>" that holds the previous generation of the regenerated outputs; version 1: empty */
    CORE::CString GetGenerationsFolder( const CORE::CString& locationRoot ) const;

    /** The logical path of the same folders: what 'Resolve' takes ("original", "intermediate", "converted/desktop") */
    CORE::CString GetOriginalPrefix( void ) const;
    CORE::CString GetIntermediatePrefix( const CORE::CString& profile ) const;
    CORE::CString GetCookedPrefix( const CORE::CString& profile ) const;

    /** The folder to show for the pack: the folder of the pack in layout 1, the original folder of the pack in layout 2 */
    CORE::CString GetBrowseFolder( const CORE::CString& locationRoot ) const;

    /** The VFS path of the pack index document of the pack */
    CORE::CString GetPackIndexPath( const CORE::CString& locationRoot, const CORE::CString& profile ) const;

    /** Every folder that only this pack owns, for removing it; empty when the pack is not inside a folder of the store */
    void GetOwnedFolders( const CORE::CString& locationRoot, CORE::CString::StringVector& folders ) const;

    /** Whether the name is one of the three areas of a pack */
    static bool IsAreaName( const CORE::CString& name );

    /** The VFS paths joined with one '/', both normalised */
    static CORE::CString Join( const CORE::CString& left, const CORE::CString& right );

    private:

    UInt32 m_version;
    CORE::CString m_packFolder;
    CORE::CString m_legacyRoot;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H ? */
