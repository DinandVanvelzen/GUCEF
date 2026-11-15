/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#include "gucefGUI_CGridPreferences.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CGridPreferences::CurrentVersion = 1;
const CString CGridPreferences::RecordNodeName = "GridPreferences";

namespace {

const CString ColumnNodeName = "Column";

/*-------------------------------------------------------------------------*/

CString
AttributeText( const CORE::CDataNode& node, const char* name )
{
    const CORE::CDataNode::TKeyValuePair* attribute = node.GetAttribute( name );
    return GUCEF_NULL != attribute ? attribute->second.AsString() : CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
AttributeBool( const CORE::CDataNode& node, const char* name, bool defaultValue )
{
    const CORE::CDataNode::TKeyValuePair* attribute = node.GetAttribute( name );
    return GUCEF_NULL != attribute ? attribute->second.AsBool( defaultValue ) : defaultValue;
}

/*-------------------------------------------------------------------------*/

Int32
AttributeInt32( const CORE::CDataNode& node, const char* name, Int32 defaultValue )
{
    const CORE::CDataNode::TKeyValuePair* attribute = node.GetAttribute( name );
    return GUCEF_NULL != attribute ? attribute->second.AsInt32( defaultValue ) : defaultValue;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGridPreferences::SColumn::SColumn( void )
    : key()
    , shown( true )
    , width( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGridPreferences::SColumn::SColumn( const CString& columnKey, bool isShown, Int32 columnWidth )
    : key( columnKey )
    , shown( isShown )
    , width( columnWidth )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CGridPreferences::SColumn::operator==( const SColumn& other ) const
{GUCEF_TRACE;

    return key == other.key && shown == other.shown && width == other.width;
}

/*-------------------------------------------------------------------------*/

CGridPreferences::CGridPreferences( void )
    : version( CurrentVersion )
    , gridId()
    , columns()
    , hasWidths( false )
    , hasSort( false )
    , sortKey()
    , sortAscending( true )
    , exportRoot()
    , exportDirectory()
    , defaultsRevision()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CGridPreferences::Clear( void )
{GUCEF_TRACE;

    *this = CGridPreferences();
}

/*-------------------------------------------------------------------------*/

bool
CGridPreferences::IsEmpty( void ) const
{GUCEF_TRACE;

    return columns.empty() && !hasSort && exportRoot.IsNULLOrEmpty() && exportDirectory.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

Int32
CGridPreferences::FindColumn( const CString& key ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)columns.size(); ++i )
        if ( columns[ i ].key == key )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

bool
CGridPreferences::SaveTo( CORE::CDataNode& parent ) const
{GUCEF_TRACE;

    CORE::CDataNode* record = parent.AddChild( RecordNodeName );
    if ( GUCEF_NULL == record )
        return false;
    record->SetAttribute( "version", CurrentVersion );
    record->SetAttribute( "gridId", gridId );
    record->SetAttribute( "hasWidths", hasWidths );
    record->SetAttribute( "hasSort", hasSort );
    if ( hasSort )
    {
        record->SetAttribute( "sortKey", sortKey );
        record->SetAttribute( "sortAscending", sortAscending );
    }
    if ( !exportRoot.IsNULLOrEmpty() )
        record->SetAttribute( "exportRoot", exportRoot );
    if ( !exportDirectory.IsNULLOrEmpty() )
        record->SetAttribute( "exportDirectory", exportDirectory );
    if ( !defaultsRevision.IsNULLOrEmpty() )
        record->SetAttribute( "defaultsRevision", defaultsRevision );
    for ( UInt32 i=0; i<(UInt32)columns.size(); ++i )
    {
        // three attributes always: a node of one attribute would fold into its parent in some codecs
        CORE::CDataNode* column = record->AddChild( ColumnNodeName );
        if ( GUCEF_NULL == column )
            return false;
        column->SetAttribute( "key", columns[ i ].key );
        column->SetAttribute( "shown", columns[ i ].shown );
        column->SetAttribute( "width", columns[ i ].width );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGridPreferences::LoadFrom( const CORE::CDataNode& node )
{GUCEF_TRACE;

    Clear();
    const CORE::CDataNode* record = &node;
    if ( RecordNodeName != node.GetName() )
        record = node.FindChild( RecordNodeName );
    if ( GUCEF_NULL == record )
        return false;

    const Int32 recordVersion = AttributeInt32( *record, "version", 0 );
    if ( recordVersion <= 0 )
        return false;
    if ( (UInt32) recordVersion > CurrentVersion )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "GridPreferences: The record of \"" << AttributeText( *record, "gridId" ) << "\" has version " << recordVersion
            << ", newer than " << CurrentVersion << ": what this version knows is read" );
    }

    version = (UInt32) recordVersion;
    gridId = AttributeText( *record, "gridId" );
    hasWidths = AttributeBool( *record, "hasWidths", false );
    hasSort = AttributeBool( *record, "hasSort", false );
    sortKey = AttributeText( *record, "sortKey" );
    sortAscending = AttributeBool( *record, "sortAscending", true );
    if ( sortKey.IsNULLOrEmpty() )
        hasSort = false;
    exportRoot = AttributeText( *record, "exportRoot" );
    exportDirectory = AttributeText( *record, "exportDirectory" );
    defaultsRevision = AttributeText( *record, "defaultsRevision" );

    CORE::CDataNode::const_iterator i = record->ConstBegin();
    while ( i != record->ConstEnd() )
    {
        const CORE::CDataNode* child = (*i);
        ++i;
        if ( GUCEF_NULL == child || ColumnNodeName != child->GetName() )
            continue;
        const CString key = AttributeText( *child, "key" );
        if ( key.IsNULLOrEmpty() || FindColumn( key ) >= 0 )
            continue;     // a column without a key cannot be matched, a repeated key counts once
        Int32 width = AttributeInt32( *child, "width", 0 );
        columns.push_back( SColumn( key, AttributeBool( *child, "shown", true ), width < 0 ? 0 : width ) );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGridPreferences::operator==( const CGridPreferences& other ) const
{GUCEF_TRACE;

    if ( columns.size() != other.columns.size() )
        return false;
    for ( UInt32 i=0; i<(UInt32)columns.size(); ++i )
        if ( !( columns[ i ] == other.columns[ i ] ) )
            return false;
    return version == other.version && gridId == other.gridId && hasWidths == other.hasWidths && hasSort == other.hasSort &&
           ( !hasSort || ( sortKey == other.sortKey && sortAscending == other.sortAscending ) ) &&
           exportRoot == other.exportRoot && exportDirectory == other.exportDirectory && defaultsRevision == other.defaultsRevision;
}

/*-------------------------------------------------------------------------*/

bool
CGridPreferences::operator!=( const CGridPreferences& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
