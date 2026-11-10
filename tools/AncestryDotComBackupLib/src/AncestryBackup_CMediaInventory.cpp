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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ANCESTRYBACKUP_CURLTEMPLATE_H
#include "AncestryBackup_CUrlTemplate.h"
#define ANCESTRYBACKUP_CURLTEMPLATE_H
#endif /* ANCESTRYBACKUP_CURLTEMPLATE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "AncestryBackup_CMediaInventory.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

std::vector< std::string >
CMediaInventory::GetFilters( void )
{GUCEF_TRACE;

    std::vector< std::string > filters;
    filters.push_back( "Photo" );
    filters.push_back( "Container" );
    filters.push_back( "Audio" );
    filters.push_back( "Video" );
    filters.push_back( "Story" );
    return filters;
}

/*-------------------------------------------------------------------------*/

std::string
CMediaInventory::BuildListUrl( const std::string& siteTreeId, const std::string& filter, UInt32 page, UInt32 rows )
{GUCEF_TRACE;

    return "https://www.ancestry.com/api/media/viewer/v1/trees/" + siteTreeId + "?client=ugcgallery-fe&excludeInlineStories=1&collectionId=1030&page=" + std::to_string( (unsigned long long) page ) +
           "&rows=" + std::to_string( (unsigned long long) rows ) + "&sort=-created&withTranscription=true&filter=" + filter;
}

/*-------------------------------------------------------------------------*/

static std::string
Text( const CORE::CDataNode& node, const char* name )
{
    return node.GetAttributeValue( name ).AsString().STL_String();
}

/*-------------------------------------------------------------------------*/

bool
CMediaInventory::ParsePage( const CORE::CDataNode& document, SMediaPage& page, bool ( *serialize )( const CORE::CDataNode&, CORE::CString& ) )
{GUCEF_TRACE;

    page = SMediaPage();
    const CORE::CDataNode* objects = document.Find( "objects" );
    if ( GUCEF_NULL == objects )
        return false;
    page.totalCount = (UInt32) document.GetAttributeValue( "mediaCount" ).AsUInt64( 0 );

    for ( CORE::CDataNode::const_iterator i = objects->ConstBegin(); i != objects->ConstEnd(); ++i )
    {
        const CORE::CDataNode& node = *(*i);
        SMediaItem item;
        item.id = Text( node, "id" );
        if ( item.id.empty() )
            continue;
        item.lookupId = Text( node, "ms_lookup_id" );
        item.namespaceId = Text( node, "collectionId" );
        item.extension = Text( node, "ext" );
        item.category = Text( node, "category" );
        item.title = Text( node, "title" );
        item.description = Text( node, "description" );
        item.date = Text( node, "date" );
        item.location = Text( node, "location" );
        item.createdDate = Text( node, "createdDate" );
        item.width = Text( node, "width" );
        item.siteUrl = Text( node, "url" );
        item.height = Text( node, "height" );
        if ( GUCEF_NULL != serialize )
        {
            CORE::CString json;
            if ( serialize( node, json ) )
                item.json = json.STL_String();
        }
        page.items.push_back( item );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

std::string
CMediaInventory::BuildSiteUrl( const SMediaItem& item )
{GUCEF_TRACE;

    if ( item.siteUrl.empty() || item.siteUrl[ 0 ] != '/' || item.siteUrl.compare( 0, 2, "//" ) == 0 )
        return std::string();
    return "https://www.ancestry.com" + item.siteUrl;
}

/*-------------------------------------------------------------------------*/

bool
CMediaInventory::IsDownloadable( const SMediaItem& item )
{GUCEF_TRACE;

    if ( item.lookupId.empty() || item.namespaceId.empty() )
        return false;
    if ( item.namespaceId.find_first_not_of( "0123456789" ) != std::string::npos )
        return false;
    // the viewer of the site shows pictures and scanned documents like this, recordings and stories are not pictures
    return item.category == "photo" || item.category == "document" || item.category == "image";
}

/*-------------------------------------------------------------------------*/

std::string
CMediaInventory::BuildDownloadUrl( const SMediaItem& item )
{GUCEF_TRACE;

    return "https://www.ancestry.com/api/media/retrieval/v2/image/namespaces/" + item.namespaceId + "/media/" + item.lookupId + "?client=trees-mediaservice&imageQuality=hq";
}

/*-------------------------------------------------------------------------*/

std::string
CMediaInventory::FileNameOf( const SMediaItem& item )
{GUCEF_TRACE;

    std::string id;
    for ( size_t i=0; i<item.id.size(); ++i )
    {
        const char c = item.id[ i ];
        if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || c == '-' || c == '_' )
            id += c;
    }
    std::string ext;
    for ( size_t i=0; i<item.extension.size() && ext.size() < 8; ++i )
    {
        const char c = (char) tolower( item.extension[ i ] );
        if ( ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) )
            ext += c;
    }
    if ( ext == "jpeg" )
        ext = "jpg";
    if ( ext.empty() )
        ext = "bin";
    return id + "." + ext;
}

/*-------------------------------------------------------------------------*/

std::string
CMediaInventory::ToInventoryLine( const SMediaItem& item )
{GUCEF_TRACE;

    return "{\"id\":\"" + CUrlTemplate::EscapeJson( item.id.c_str() ).STL_String() + "\",\"lookupId\":\"" + CUrlTemplate::EscapeJson( item.lookupId.c_str() ).STL_String() +
           "\",\"namespace\":\"" + CUrlTemplate::EscapeJson( item.namespaceId.c_str() ).STL_String() + "\",\"category\":\"" + CUrlTemplate::EscapeJson( item.category.c_str() ).STL_String() +
           "\",\"file\":\"" + CUrlTemplate::EscapeJson( FileNameOf( item ).c_str() ).STL_String() + "\",\"title\":\"" + CUrlTemplate::EscapeJson( item.title.c_str() ).STL_String() +
           "\",\"createdDate\":\"" + CUrlTemplate::EscapeJson( item.createdDate.c_str() ).STL_String() + "\",\"item\":" + ( item.json.empty() ? std::string( "null" ) : item.json ) + "}";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
