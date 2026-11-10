/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#ifndef ANCESTRYBACKUP_CMEDIAINVENTORY_H
#define ANCESTRYBACKUP_CMEDIAINVENTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include <string>
#include <vector>

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** One photo, document, story, recording or video of the tree */
struct SMediaItem
{
    std::string id;                 /**< the id of the item in the tree */
    std::string lookupId;           /**< the id of the file in the media service, which is not the same */
    std::string namespaceId;        /**< the media service keeps files in numbered collections */
    std::string extension;          /**< jpg, png, pdf... */
    std::string category;           /**< photo, document, story, audio, video */
    std::string title;
    std::string description;
    std::string date;
    std::string location;
    std::string createdDate;
    std::string width;
    std::string height;
    std::string siteUrl;            /**< the address of the file as the site's own list gives it, relative to the site */
    std::string json;               /**< the whole object as the site gave it, nothing is dropped */

    SMediaItem( void ) : id(), lookupId(), namespaceId(), extension(), category(), title(), description(), date(), location(), createdDate(), width(), height(), siteUrl(), json() {}
};

/** A page of the media list of a tree */
struct SMediaPage
{
    std::vector< SMediaItem > items;
    UInt32 totalCount;              /**< how many items the list has in all, over its pages */

    SMediaPage( void ) : items(), totalCount( 0 ) {}
};

/**
 *  The media list of a tree, which the site pages through 28 at a time, and what is needed to fetch the files from it.
 */
class CMediaInventory
{
    public:

    /** The kinds of media the site lists. Each is a list of its own. */
    static std::vector< std::string > GetFilters( void );

    static std::string BuildListUrl( const std::string& siteTreeId, const std::string& filter, UInt32 page, UInt32 rows );

    /**
     *  @param serialize    turns an object into JSON, so that it can be kept whole
     */
    static bool ParsePage( const CORE::CDataNode& document, SMediaPage& page, bool ( *serialize )( const CORE::CDataNode&, CORE::CString& ) );

    /** The address of the file: the full size version of what the viewer of the site shows */
    static std::string BuildDownloadUrl( const SMediaItem& item );

    /** The address the site's list itself gives for the file, as an absolute address. Empty if it has none. */
    static std::string BuildSiteUrl( const SMediaItem& item );

    /** Can the file of this kind of item be fetched with BuildDownloadUrl? Pictures and documents can, recordings and stories need their own handling. */
    static bool IsDownloadable( const SMediaItem& item );

    /** The name the file gets in the data directory: the id of the item and an extension that is safe to use in a file name */
    static std::string FileNameOf( const SMediaItem& item );

    /** One line for the inventory file */
    static std::string ToInventoryLine( const SMediaItem& item );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CMEDIAINVENTORY_H ? */
