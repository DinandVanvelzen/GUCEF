/*
 *  AssetConverterLib: the report of one asset
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

#include "AssetConverterLib_CConversionReportEntry.h"

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

CConversionReportEntry::CConversionReportEntry( void )
    : assetPath()
    , importer()
    , items()
    , files()
    , cached( false )
    , refused( false )
    , skipped( false )
    , cookBlocked( false )
    , durationInMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

TAssetStatus
CConversionReportEntry::GetStatus( void ) const
{GUCEF_TRACE;

    if ( refused )
        return ASSETSTATUS_REFUSED;
    if ( cached )
        return ASSETSTATUS_CACHED;
    if ( HasErrors() )
        return ASSETSTATUS_FAILED;
    if ( skipped )
        return ASSETSTATUS_SKIPPED;
    if ( GetItemCount( REPORTITEM_APPROXIMATION ) > 0 )
        return ASSETSTATUS_APPROXIMATED;
    return ASSETSTATUS_CONVERTED;
}

/*-------------------------------------------------------------------------*/

UInt32
CConversionReportEntry::GetItemCount( TReportItemKind kind ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<items.size(); ++i )
        if ( items[ i ].kind == kind )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

bool
CConversionReportEntry::HasErrors( void ) const
{GUCEF_TRACE;

    return GetItemCount( REPORTITEM_ERROR ) > 0;
}

/*-------------------------------------------------------------------------*/

void
CConversionReportEntry::AddFile( const CConversionReportFile& file )
{GUCEF_TRACE;

    for ( size_t i=0; i<files.size(); ++i )
    {
        if ( files[ i ].path == file.path && files[ i ].profile == file.profile )
        {
            for ( size_t r=0; r<file.references.size(); ++r )
            {
                bool known = false;
                for ( size_t k=0; k<files[ i ].references.size() && !known; ++k )
                    known = files[ i ].references[ k ] == file.references[ r ];
                if ( !known )
                    files[ i ].references.push_back( file.references[ r ] );
            }
            return;
        }
    }
    files.push_back( file );
}

/*-------------------------------------------------------------------------*/

bool
CConversionReportEntry::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "asset", assetPath );
    node.SetAttribute( "importer", importer );
    node.SetAttribute( "status", ToString( GetStatus() ) );
    if ( cookBlocked )
        node.SetAttribute( "cookBlocked", true );
    CORE::CDataNode* itemList = node.AddChild( "items", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<items.size(); ++i )
        items[ i ].Serialize( *itemList->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    CORE::CDataNode* fileList = node.AddChild( "files", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<files.size(); ++i )
        files[ i ].Serialize( *fileList->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CConversionReportEntry::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CConversionReportEntry entry;
    entry.assetPath = node.GetAttributeValueOrChildValueByName( "asset" ).AsString();
    entry.importer = node.GetAttributeValueOrChildValueByName( "importer" ).AsString();
    TAssetStatus status = ASSETSTATUS_CONVERTED;
    FromString( node.GetAttributeValueOrChildValueByName( "status" ).AsString(), status );
    entry.cached = ASSETSTATUS_CACHED == status;
    entry.refused = ASSETSTATUS_REFUSED == status;
    entry.skipped = ASSETSTATUS_SKIPPED == status;
    entry.cookBlocked = node.GetAttributeValueOrChildValueByName( "cookBlocked" ).AsBool( false );
    const CORE::CDataNode* itemList = node.FindChild( "items" );
    if ( GUCEF_NULL != itemList )
    {
        CORE::CDataNode::const_iterator i = itemList->ConstBegin();
        while ( i != itemList->ConstEnd() )
        {
            CConversionReportItem item;
            if ( item.Deserialize( *(*i) ) )
                entry.items.push_back( item );
            ++i;
        }
    }
    const CORE::CDataNode* fileList = node.FindChild( "files" );
    if ( GUCEF_NULL != fileList )
    {
        CORE::CDataNode::const_iterator i = fileList->ConstBegin();
        while ( i != fileList->ConstEnd() )
        {
            CConversionReportFile file;
            if ( file.Deserialize( *(*i) ) )
                entry.files.push_back( file );
            ++i;
        }
    }
    *this = entry;
    return !assetPath.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
