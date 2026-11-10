/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

#include "gucefDRN_CDRNPublishedColumn.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNPublishedColumn::CDRNPublishedColumn( void )
    : id()
    , def()
    , title()
    , width( 0 )
    , hidden( false )
    , hints()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNPublishedColumn::CDRNPublishedColumn( const CORE::CString& columnId, const CORE::CDataTableColumnDef& columnDef )
    : id( columnId )
    , def( columnDef )
    , title()
    , width( 0 )
    , hidden( false )
    , hints()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedColumn::Encode( CORE::CVariantStream& stream ) const
{GUCEF_TRACE;

    stream << id << def.name << def.GetTypeText() << def.nullable << def.description << title << width << hidden;
    UInt32 hintCount = hints.GetKeyCount();
    stream << hintCount;
    for ( UInt32 i=0; i<hintCount; ++i )
    {
        const CORE::CVariant& key = hints.GetKey( i );
        const CORE::CValueList::TVariantVector& values = hints.GetValueVector( i );
        stream << key.AsString() << ( values.empty() ? CORE::CString::Empty : values.front().AsString() );
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedColumn::Decode( CORE::CVariantStream& stream )
{GUCEF_TRACE;

    CORE::CString typeText;
    UInt32 hintCount = 0;
    stream >> id >> def.name >> typeText >> def.nullable >> def.description >> title >> width >> hidden >> hintCount;
    if ( !stream.IsValid() || !def.SetTypeFromText( typeText ) || hintCount > 4096 )
        return false;
    hints.DeleteAll();
    for ( UInt32 i=0; i<hintCount; ++i )
    {
        CORE::CString name;
        CORE::CString value;
        stream >> name >> value;
        if ( !stream.IsValid() )
            return false;
        hints.Set( name, CORE::CVariant( value ) );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedColumn::operator==( const CDRNPublishedColumn& other ) const
{GUCEF_TRACE;

    if ( id != other.id || def != other.def || title != other.title || width != other.width || hidden != other.hidden ||
         hints.GetKeyCount() != other.hints.GetKeyCount() )
        return false;
    for ( UInt32 i=0; i<hints.GetKeyCount(); ++i )
    {
        CORE::CString key = hints.GetKey( i ).AsString();
        if ( !other.hints.HasKey( key ) || other.hints.GetValue( key ).AsString() != hints.GetValue( key ).AsString() )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedColumn::operator!=( const CDRNPublishedColumn& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
