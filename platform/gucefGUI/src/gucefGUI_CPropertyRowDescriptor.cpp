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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefGUI_CPropertyRowDescriptor.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CString
Trimmed( const CString& text )
{
    return text.Trim( true ).Trim( false );
}

/** Reads the whole text as a number; false for anything else, also an empty text */
bool
ParseNumber( const CString& text, Float64& value )
{
    const CString trimmed = Trimmed( text );
    if ( trimmed.IsNULLOrEmpty() )
        return false;
    const char* begin = trimmed.C_String();
    char* end = GUCEF_NULL;
    const double parsed = strtod( begin, &end );
    if ( GUCEF_NULL == end || end == begin || '\0' != *end )
        return false;
    value = (Float64) parsed;
    return true;
}

/** A number as text: with the digits asked for, else the shortest text that holds the value */
CString
FormatNumber( Float64 value, Int32 decimals )
{
    char buffer[ 64 ];
    if ( decimals >= 0 )
        sprintf( buffer, "%.*f", (int) decimals, (double) value );
    else
        sprintf( buffer, "%.12g", (double) value );
    return buffer;
}

CString
ShortNumber( Float64 value )
{
    return FormatNumber( value, -1 );
}

bool
ParseBool( const CString& text, bool& value )
{
    const CString lower = Trimmed( text ).Lowercase();
    if ( "true" == lower || "1" == lower || "yes" == lower || "on" == lower )
    {
        value = true;
        return true;
    }
    if ( "false" == lower || "0" == lower || "no" == lower || "off" == lower )
    {
        value = false;
        return true;
    }
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPropertyRowDescriptor::CPropertyRowDescriptor( void )
    : type( VALUETYPE_TEXT )
    , label()
    , tooltip()
    , unit()
    , readOnly( false )
    , hasRange( false )
    , minimum( 0.0 )
    , maximum( 0.0 )
    , decimals( -1 )
    , choices()
    , componentCount( 3 )
    , pathIsDirectory( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPropertyRowDescriptor
CPropertyRowDescriptor::Text( void )
{GUCEF_TRACE;

    return CPropertyRowDescriptor();
}

/*-------------------------------------------------------------------------*/

CPropertyRowDescriptor
CPropertyRowDescriptor::Number( Int32 numberOfDecimals )
{GUCEF_TRACE;

    CPropertyRowDescriptor descriptor;
    descriptor.type = VALUETYPE_NUMBER;
    descriptor.decimals = numberOfDecimals;
    return descriptor;
}

/*-------------------------------------------------------------------------*/

CPropertyRowDescriptor
CPropertyRowDescriptor::NumberInRange( Float64 lowest, Float64 highest, Int32 numberOfDecimals )
{GUCEF_TRACE;

    CPropertyRowDescriptor descriptor = Number( numberOfDecimals );
    descriptor.hasRange = true;
    descriptor.minimum = lowest;
    descriptor.maximum = highest;
    return descriptor;
}

/*-------------------------------------------------------------------------*/

CPropertyRowDescriptor
CPropertyRowDescriptor::Boolean( void )
{GUCEF_TRACE;

    CPropertyRowDescriptor descriptor;
    descriptor.type = VALUETYPE_BOOL;
    return descriptor;
}

/*-------------------------------------------------------------------------*/

CPropertyRowDescriptor
CPropertyRowDescriptor::Choice( const CString::StringVector& options )
{GUCEF_TRACE;

    CPropertyRowDescriptor descriptor;
    descriptor.type = VALUETYPE_CHOICE;
    descriptor.choices = options;
    return descriptor;
}

/*-------------------------------------------------------------------------*/

CPropertyRowDescriptor
CPropertyRowDescriptor::Colour( void )
{GUCEF_TRACE;

    CPropertyRowDescriptor descriptor;
    descriptor.type = VALUETYPE_COLOUR;
    return descriptor;
}

/*-------------------------------------------------------------------------*/

CPropertyRowDescriptor
CPropertyRowDescriptor::Vector( UInt32 components )
{GUCEF_TRACE;

    CPropertyRowDescriptor descriptor;
    descriptor.type = VALUETYPE_VECTOR;
    descriptor.componentCount = components < 2 ? 2 : ( components > 4 ? 4 : components );
    return descriptor;
}

/*-------------------------------------------------------------------------*/

CPropertyRowDescriptor
CPropertyRowDescriptor::Path( bool isDirectory )
{GUCEF_TRACE;

    CPropertyRowDescriptor descriptor;
    descriptor.type = VALUETYPE_PATH;
    descriptor.pathIsDirectory = isDirectory;
    return descriptor;
}

/*-------------------------------------------------------------------------*/

CPropertyRowDescriptor
CPropertyRowDescriptor::AsReadOnly( void ) const
{GUCEF_TRACE;

    CPropertyRowDescriptor descriptor = *this;
    descriptor.readOnly = true;
    return descriptor;
}

/*-------------------------------------------------------------------------*/

CPropertyRowDescriptor
CPropertyRowDescriptor::WithTooltip( const CString& text ) const
{GUCEF_TRACE;

    CPropertyRowDescriptor descriptor = *this;
    descriptor.tooltip = text;
    return descriptor;
}

/*-------------------------------------------------------------------------*/

CPropertyRowDescriptor
CPropertyRowDescriptor::WithUnit( const CString& text ) const
{GUCEF_TRACE;

    CPropertyRowDescriptor descriptor = *this;
    descriptor.unit = text;
    return descriptor;
}

/*-------------------------------------------------------------------------*/

CPropertyRowDescriptor
CPropertyRowDescriptor::WithLabel( const CString& text ) const
{GUCEF_TRACE;

    CPropertyRowDescriptor descriptor = *this;
    descriptor.label = text;
    return descriptor;
}

/*-------------------------------------------------------------------------*/

bool
CPropertyRowDescriptor::Validate( const CString& text, CString& problem ) const
{GUCEF_TRACE;

    problem.Clear();
    switch ( type )
    {
        case VALUETYPE_NUMBER:
        {
            CString number = Trimmed( text );
            if ( !unit.IsNULLOrEmpty() && number.Length() > unit.Length() && number.SubstrFromRange( number.Length() - unit.Length(), number.Length() ) == unit )
                number = Trimmed( number.SubstrToIndex( number.Length() - unit.Length(), true ) );
            Float64 value = 0.0;
            if ( !ParseNumber( number, value ) )
            {
                problem = "This is no number";
                return false;
            }
            if ( 0 == decimals && value != floor( value ) )
            {
                problem = "A whole number is needed";
                return false;
            }
            if ( hasRange && value < minimum )
            {
                problem = "The value is at least " + ShortNumber( minimum );
                return false;
            }
            if ( hasRange && value > maximum )
            {
                problem = "The value is at most " + ShortNumber( maximum );
                return false;
            }
            return true;
        }
        case VALUETYPE_BOOL:
        {
            bool value = false;
            if ( !ParseBool( text, value ) )
            {
                problem = "Use true or false";
                return false;
            }
            return true;
        }
        case VALUETYPE_CHOICE:
        {
            for ( UInt32 i=0; i<(UInt32)choices.size(); ++i )
                if ( choices[ i ] == text )
                    return true;
            problem = "This is not one of the choices";
            return false;
        }
        case VALUETYPE_COLOUR:
        {
            CColour colour;
            if ( !CColour::FromString( Trimmed( text ), colour ) )
            {
                problem = "Use r,g,b,a with numbers up to 255, or #rrggbb";
                return false;
            }
            return true;
        }
        case VALUETYPE_VECTOR:
        {
            const CString::StringVector parts = Trimmed( text ).ParseElements( ',', true );
            if ( parts.size() != (size_t) componentCount )
            {
                problem = "Give " + CORE::UInt32ToString( componentCount ) + " numbers separated by commas";
                return false;
            }
            for ( UInt32 i=0; i<(UInt32)parts.size(); ++i )
            {
                Float64 value = 0.0;
                if ( !ParseNumber( parts[ i ], value ) )
                {
                    problem = "\"" + Trimmed( parts[ i ] ) + "\" is no number";
                    return false;
                }
                if ( 0 == decimals && value != floor( value ) )
                {
                    problem = "Whole numbers are needed";
                    return false;
                }
                if ( hasRange && ( value < minimum || value > maximum ) )
                {
                    problem = "Each number is between " + ShortNumber( minimum ) + " and " + ShortNumber( maximum );
                    return false;
                }
            }
            return true;
        }
        case VALUETYPE_PATH:
        case VALUETYPE_TEXT:
        default:
            return true;
    }
}

/*-------------------------------------------------------------------------*/

CString
CPropertyRowDescriptor::Normalize( const CString& text ) const
{GUCEF_TRACE;

    CString problem;
    if ( !Validate( text, problem ) )
        return text;
    switch ( type )
    {
        case VALUETYPE_NUMBER:
        {
            CString number = Trimmed( text );
            if ( !unit.IsNULLOrEmpty() && number.Length() > unit.Length() && number.SubstrFromRange( number.Length() - unit.Length(), number.Length() ) == unit )
                number = Trimmed( number.SubstrToIndex( number.Length() - unit.Length(), true ) );
            Float64 value = 0.0;
            ParseNumber( number, value );
            return FormatNumber( value, decimals );
        }
        case VALUETYPE_BOOL:
        {
            bool value = false;
            ParseBool( text, value );
            return value ? "true" : "false";
        }
        case VALUETYPE_COLOUR:
        {
            CColour colour;
            CColour::FromString( Trimmed( text ), colour );
            return colour.ToString();
        }
        case VALUETYPE_VECTOR:
        {
            const CString::StringVector parts = Trimmed( text ).ParseElements( ',', true );
            CString result;
            for ( UInt32 i=0; i<(UInt32)parts.size(); ++i )
            {
                Float64 value = 0.0;
                ParseNumber( parts[ i ], value );
                result += ( i > 0 ? ", " : "" ) + FormatNumber( value, decimals );
            }
            return result;
        }
        default:
            return text;
    }
}

/*-------------------------------------------------------------------------*/

CString
CPropertyRowDescriptor::FormatForDisplay( const CString& text ) const
{GUCEF_TRACE;

    const CString shown = Normalize( text );
    if ( VALUETYPE_NUMBER == type && !unit.IsNULLOrEmpty() && !shown.IsNULLOrEmpty() )
        return shown + " " + unit;
    return shown;
}

/*-------------------------------------------------------------------------*/

CString
CPropertyRowDescriptor::NumberToText( Float64 value, Int32 numberOfDecimals )
{GUCEF_TRACE;

    return FormatNumber( value, numberOfDecimals );
}

/*-------------------------------------------------------------------------*/

bool
CPropertyRowDescriptor::TextToNumber( const CString& text, Float64& value )
{GUCEF_TRACE;

    return ParseNumber( text, value );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
