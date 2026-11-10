/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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


#ifndef GUCEF_COM_CIMAPENVELOPE_H
#include "gucefCOM_CImapEnvelope.h"
#define GUCEF_COM_CIMAPENVELOPE_H
#endif /* GUCEF_COM_CIMAPENVELOPE_H ? */

#ifndef GUCEF_COM_CIMAPBODYSTRUCTURE_H
#include "gucefCOM_CImapBodyStructure.h"
#define GUCEF_COM_CIMAPBODYSTRUCTURE_H
#endif /* GUCEF_COM_CIMAPBODYSTRUCTURE_H ? */

#ifndef GUCEF_COM_CIMAPFETCHSECTION_H
#include "gucefCOM_CImapFetchSection.h"
#define GUCEF_COM_CIMAPFETCHSECTION_H
#endif /* GUCEF_COM_CIMAPFETCHSECTION_H ? */

#include "gucefCOM_CImapFetchedItem.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

inline UInt32
ByteLength( const CORE::CString& text )
{
    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImapFetchedItem::CImapFetchedItem( void )
    : sequence( 0 )
    , hasUid( false )
    , uid( 0 )
    , hasFlags( false )
    , flags()
    , hasInternalDate( false )
    , internalDate()
    , hasSize( false )
    , size( 0 )
    , hasEnvelope( false )
    , envelope()
    , hasBodyStructure( false )
    , bodyStructure()
    , sections()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CImapFetchSection*
CImapFetchedItem::FindSection( const CORE::CString& name ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<sections.size(); ++i )
        if ( sections[ i ].name.Equals( name, false ) )
            return &sections[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CImapFetchedItem::HasFlag( const CORE::CString& flag ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<flags.size(); ++i )
        if ( flags[ i ].Equals( flag, false ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CImapFetchedItem::FromResponse( const CImapResponse& response, CImapFetchedItem& item )
{GUCEF_TRACE;

    item = CImapFetchedItem();
    if ( CImapResponse::KIND_UNTAGGED != response.kind || !response.hasNumber || response.keyword != "FETCH" )
        return false;
    item.sequence = response.number;

    const CImapValue& items = response.data.GetElement( 0 );
    if ( !items.IsList() )
        return false;

    for ( UInt32 i=0; i+1<items.GetListSize(); i+=2 )
    {
        const CImapValue& nameValue = items.GetElement( i );
        const CImapValue& value = items.GetElement( i + 1 );
        if ( !nameValue.IsAtom() )
            return false;
        const CORE::CString name = nameValue.GetString().Uppercase();

        if ( name == "UID" )
        {
            item.hasUid = value.IsNumber() && value.GetNumber() <= 0xFFFFFFFFull;
            item.uid = (UInt32) value.GetNumber();
        }
        else
        if ( name == "FLAGS" )
        {
            item.hasFlags = true;
            for ( UInt32 f=0; f<value.GetListSize() && f<200; ++f )
                item.flags.push_back( value.GetElement( f ).GetString() );
        }
        else
        if ( name == "INTERNALDATE" )
        {
            item.hasInternalDate = true;
            item.internalDate = value.GetString();
        }
        else
        if ( name == "RFC822.SIZE" )
        {
            item.hasSize = true;
            item.size = (UInt32)( value.GetNumber() > 0xFFFFFFFFull ? 0xFFFFFFFFull : value.GetNumber() );
        }
        else
        if ( name == "ENVELOPE" )
            item.hasEnvelope = CImapEnvelope::FromValue( value, item.envelope );
        else
        if ( name == "BODYSTRUCTURE" || ( name == "BODY" && value.IsList() ) )
            item.hasBodyStructure = CImapBodyStructure::FromValue( value, item.bodyStructure );
        else
        if ( name == "RFC822" || name == "RFC822.HEADER" || name == "RFC822.TEXT" || name.StartsWith( "BODY[" ) || name.StartsWith( "BODY.PEEK[" ) )
        {
            CImapFetchSection section;
            if ( name == "RFC822" ) section.name = "";
            else if ( name == "RFC822.HEADER" ) section.name = "HEADER";
            else if ( name == "RFC822.TEXT" ) section.name = "TEXT";
            else
            {
                // BODY[section]<origin>
                const Int32 open = nameValue.GetString().HasChar( '[' );
                const Int32 close = nameValue.GetString().HasChar( ']', false );
                if ( open < 0 || close < open )
                    return false;
                section.name = nameValue.GetString().SubstrFromRange( (UInt32) open + 1, (UInt32) close ).Uppercase();
                const CORE::CString after = nameValue.GetString().SubstrFromRange( (UInt32) close + 1, nameValue.GetString().Length() );
                if ( after.StartsWith( "<" ) )
                {
                    section.hasOrigin = true;
                    section.origin = (UInt32) CORE::StringToUInt32( after.SubstrFromRange( 1, after.Length() ).SubstrToChar( '>', true ) );
                }
            }
            section.isNil = value.IsNil();
            if ( value.IsString() )
                section.data = value.GetBytes();
            else
            if ( value.IsAtom() )
                section.data.CopyFrom( ByteLength( value.GetString() ), value.GetString().C_String() );
            item.sections.push_back( section );
        }
        // BINARY, X-GM-MSGID and other things that were not asked for are skipped
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
