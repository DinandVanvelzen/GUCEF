/*
 *  AssetWorkbenchLib: the resources of a pack, grouped by function, as a file system for the content browser
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

#ifndef ASSETWORKBENCHLIB_CRESOURCEVIEWFILESYSTEMACCESS_H
#define ASSETWORKBENCHLIB_CRESOURCEVIEWFILESYSTEMACCESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CICONTENTITEMORDER_H
#include "gucefGUI_CIContentItemOrder.h"
#define GUCEF_GUI_CICONTENTITEMORDER_H
#endif /* GUCEF_GUI_CICONTENTITEMORDER_H ? */

#ifndef GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#include "gucefCORE_CFileSystemEntryListIterator.h"
#define GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#endif /* GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H ? */

#ifndef GUCEF_CORE_IFILESYSTEMACCESS_H
#include "gucefCORE_IFileSystemAccess.h"
#define GUCEF_CORE_IFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_IFILESYSTEMACCESS_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#include "AssetConverterLib_CPackIndexDocument.h"
#define ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The pack index (AssetConverterLib, CPackIndexDocument) as a file system that nothing can be written to, so that a
 *  GUI::CContentBrowserForm shows the resources of a pack by function, as it shows folders (F17, the resource view):
 *
 *      /                               the function groups with their counts: "Static models (2,432)"
 *      /<group>                        its sub-groups with counts, then the resources that have no sub-group
 *      /<group>/<sub-group>[/<more>]   the next level of the sub-group id, then its resources
 *      /<group>/<sub-group>/<name>     a resource, a file with the size of the file of the pack
 *
 *  Nothing is copied or converted: a path of this file system maps back to the real path of the resource in the pack (see
 *  TryResolve() and GetVfsPathOf()), which is what select and open use. The files the index calls noise are not in it.
 *
 *  Lazy: the first level comes from the manifest alone (the counts of the groups). The records of a group are read when a
 *  folder of that group is listed for the first time (one shard at a time with a sharded index), and a folder listing is made
 *  when the folder is opened and kept while it is the same. A resource whose name another resource of the same folder has
 *  too is listed with the folder of the pack in brackets behind its name, so that every path is one resource.
 *
 *  Search: with a search text the root lists the resources of all groups whose name has the text (a pattern with * or ? is matched as
 *  a pattern), flat, each with its group behind its name; the groups and their counts show the matches only. An empty text is the
 *  whole pack again.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CResourceViewFileSystemAccess : public CORE::IFileSystemAccess, public GUI::CIContentItemOrder
{
    public:

    /** A folder with more files than this lists them in pages "Items 0001 - 0500" (folders of the file system), so that a folder of ten thousand resources is never built at once */
    static const UInt32 PageSize = 500;

    /** The most records an adopted index keeps in memory */
    static const UInt32 MaxResidentResources = 30000;

    explicit CResourceViewFileSystemAccess( const CORE::CString& displayName = "Resources" );
    virtual ~CResourceViewFileSystemAccess() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The pack to show: the manifest of its index (with the records inline for a small pack, the shards of 'indexFolder' for a large one)
     *  and the VFS folder the paths of the records are relative to. Replaces what was shown; the form shows it after its Refresh().
     */
    void SetPack( const ASSETCONVERTER::CPackIndexDocument& document, const CORE::CString& indexFolder, const CORE::CString& packVfsRoot );

    /**
     *  As SetPack(), for an index that the caller has read whole and does not need any more: the records are taken from the document (it is left
     *  without them) when there are not more than MaxResidentResources of them, so that no shard has to be read to list a group; with more the
     *  manifest alone is kept and the groups read their shards when they are opened.
     */
    void AdoptPack( ASSETCONVERTER::CPackIndexDocument& document, const CORE::CString& indexFolder, const CORE::CString& packVfsRoot );

    /** Shows nothing */
    void Clear( void );

    bool HasPack( void ) const;
    const ASSETCONVERTER::CPackIndexDocument& GetManifest( void ) const;
    const CORE::CString& GetIndexFolder( void ) const;
    const CORE::CString& GetPackVfsRoot( void ) const;

    /** The search text (see above); empty for none */
    void SetSearch( const CORE::CString& text );
    const CORE::CString& GetSearch( void ) const;

    /** The path of a group in this file system */
    static CORE::CString MakeGroupPath( const CORE::CString& groupId );

    /** The group a path is in ("/static-models/props/Table.fbx" gives "static-models"); 'isGroupFolder' says the path is the folder of the group itself; false for the root */
    static bool GetGroupOfPath( const CORE::CString& path, CORE::CString& groupId, bool& isGroupFolder );

    /** The record a path of a file names; false when the path is a folder or nothing */
    bool TryResolve( const CORE::CString& path, ASSETCONVERTER::CPackIndexResource& resource ) const;

    /** The path of the resource in the VFS (where the viewer opens it) */
    CORE::CString GetVfsPathOf( const ASSETCONVERTER::CPackIndexResource& resource ) const;

    /** The path of the resource in this file system (the leaf may carry the folder when the name is not unique); empty when it is not in the pack */
    CORE::CString GetViewPathOf( const ASSETCONVERTER::CPackIndexResource& resource ) const;

    /** What a folder of this file system holds: its text for people, the number of resources and their bytes; false when the path is no folder */
    bool GetFolderSummary( const CORE::CString& path, CORE::CString& text, UInt32& count, UInt64& bytes ) const;

    /** The path of the resource in the pack and the group path for people: "Static models / Polygon shops / Props" */
    CORE::CString GetGroupTextOf( const ASSETCONVERTER::CPackIndexResource& resource ) const;

    // what the tests count: the work that was done
    UInt32 GetMaterialisedGroupCount( void ) const;     /**< groups whose records were made ready */
    UInt32 GetShardReadCount( void ) const;             /**< shards read from the disk */
    UInt32 GetListingCount( void ) const;               /**< folder listings made (not those served from the kept one) */
    UInt32 GetLoadedResourceCount( void ) const;        /**< records held for the groups that were opened */

    /** The order of the groups: the fixed order of the vocabulary (scenes, characters, static models, ...), not the alphabet; the rest by name */
    virtual Int32 GetRank( const CORE::CString& path, bool isDirectory ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetDisplayName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetRootPath( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsWriteable( const CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryCreateDirectory( const CORE::CString& path, bool createParents ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryDeleteDirectory( const CORE::CString& path, bool deleteContents ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryMoveDirectory( const CORE::CString& oldPath, const CORE::CString& newPath ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryMoveFile( const CORE::CString& oldPath, const CORE::CString& newPath, bool overwrite ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryMoveFiles( const CORE::CString::StringVector& filePaths     ,
                                                        const CORE::CString& destinationDir             ,
                                                        bool overwrite                                  ,
                                                        TFileSystemAccessResultVector* perFileResults   ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryRenameFile( const CORE::CString& path, const CORE::CString& newName ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryDeleteFile( const CORE::CString& path ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryGetMetaData( const CORE::CString& path, CORE::CResourceMetaData& metaData ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryGetEntryType( const CORE::CString& path, CORE::TFileSystemEntryType& entryType ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryGetFileSystemIterator( const CORE::CString& dirPath, CORE::TFileSystemIteratorPtr& iterator ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult ListAvailableSpecialDirs( CORE::TSpecialDirSet& specialDirs ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryResolveSpecialDir( CORE::TSpecialDirs specialDir, CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CResourceViewFileSystemAccess( const CResourceViewFileSystemAccess& src );
    CResourceViewFileSystemAccess& operator=( const CResourceViewFileSystemAccess& src );

    /** The records of a group, in the order of the index, and what the listings need of them */
    struct SGroupData
    {
        bool loaded;
        GUCEF::vector< const ASSETCONVERTER::CPackIndexResource* > resources;     /**< in the order of the index; they point into the manifest or into the shards that were read */
        GUCEF::map< CORE::CString, UInt32 > subCounts;      /**< sub-group id to the number of records with it, for the records that match the search */
        SGroupData( void );
    };

    /** A folder as listed: the folders below it and the files in it, with the leaf name of each file */
    struct SListing
    {
        struct SFolder
        {
            CORE::CString segment;
            UInt32 count;
        };
        GUCEF::vector< SFolder > folders;
        GUCEF::vector< UInt32 > files;                      /**< indices into the records of the group */
        CORE::CString::StringVector leafNames;              /**< of the files, unique in the folder */
        GUCEF::map< CORE::CString, UInt32 > leafToFile;     /**< leaf name to position in 'files' */
        UInt32 pageCount;                                   /**< 0 for a folder that lists its files itself; else the number of pages of PageSize files that it lists instead */
        SListing( void ) : pageCount( 0 ) {}
    };

    typedef GUCEF::map< CORE::CString, SGroupData > TGroupMap;
    typedef GUCEF::map< CORE::CString, SListing > TListingMap;

    SGroupData* Materialise( const CORE::CString& groupId ) const;
    const SListing* GetListing( const CORE::CString& groupId, const CORE::CString& subPath ) const;
    bool Matches( const ASSETCONVERTER::CPackIndexResource& resource ) const;
    struct SLocation
    {
        Int32 type;                 /**< 0 nothing, 1 a folder, 2 a file */
        CORE::CString group;
        CORE::CString sub;
        UInt32 page;                /**< 0 for the folder itself, else the page of a paged folder */
        CORE::CString leaf;
        UInt32 position;            /**< of a file in the files of its listing */
        SLocation( void ) : type( 0 ), group(), sub(), page( 0 ), leaf(), position( 0 ) {}
    };
    bool Locate( const CORE::CString& path, SLocation& location ) const;
    void ReleaseStore( void );
    void AddFileEntries( const SGroupData& data, const SListing& listing, const CORE::CString& dirPath, UInt32 from, UInt32 to, bool withGroupText, CORE::CFileSystemEntryListIterator::TEntryVector& entries ) const;
    void RecountGroup( SGroupData& data ) const;
    static CORE::CString DisplayOfSegment( const CORE::CString& segment );
    static CORE::CString FormatCount( UInt32 count );
    static CORE::CString FolderTagOf( const CORE::CString& packPath );

    private:

    CORE::CString m_displayName;
    ASSETCONVERTER::CPackIndexDocument m_manifest;
    CORE::CString m_indexFolder;
    CORE::CString m_packVfsRoot;
    CORE::CString m_search;
    mutable TGroupMap m_groups;
    mutable TListingMap m_listings;
    mutable GUCEF::vector< ASSETCONVERTER::CPackIndexResource::TResourceVector* > m_shardStore;   /**< what was read from the shards: the records of the groups point into it */
    mutable UInt32 m_materialised;
    mutable UInt32 m_shardReads;
    mutable UInt32 m_listingCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CRESOURCEVIEWFILESYSTEMACCESS_H ? */

