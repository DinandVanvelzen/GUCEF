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
#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

#ifndef GUCEF_GUI_CUVECTOR_H
#include "gucefGUI_CUVector.h"
#define GUCEF_GUI_CUVECTOR_H
#endif /* GUCEF_GUI_CUVECTOR_H ? */

#include "gucefGUI_CWidgetTypeRegistry.h"

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

bool
IsInteger( const CString& text )
{
    CString trimmed = text.Trim( true ).Trim( false );
    if ( trimmed.IsNULLOrEmpty() )
        return false;
    char* end = GUCEF_NULL;
    strtol( trimmed.C_String(), &end, 10 );
    return '\0' == *end;
}

bool
IsNumber( const CString& text )
{
    CString trimmed = text.Trim( true ).Trim( false );
    if ( trimmed.IsNULLOrEmpty() )
        return false;
    char* end = GUCEF_NULL;
    strtod( trimmed.C_String(), &end );
    return '\0' == *end;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPropertyDescriptor::CPropertyDescriptor( void )
    : name()
    , id( EWidgetProperty_Unknown )
    , valueType( VALUETYPE_STRING )
    , defaultValue()
    , flags( FLAG_DEFAULT )
    , category()
    , description()
    , enumValues()
{
}

/*-------------------------------------------------------------------------*/

CPropertyDescriptor::CPropertyDescriptor( const CString& propertyName       ,
                                          TValueType type                   ,
                                          const CString& defaultVal         ,
                                          UInt32 propertyFlags              ,
                                          const CString& propertyCategory   ,
                                          const CString& propertyDescription )
    : name( propertyName )
    , id( StringToWidgetProperty( propertyName ) )
    , valueType( type )
    , defaultValue( defaultVal )
    , flags( propertyFlags )
    , category( propertyCategory )
    , description( propertyDescription )
    , enumValues()
{
}

/*-------------------------------------------------------------------------*/

CPropertyDescriptor::CPropertyDescriptor( EWidgetProperty property          ,
                                          TValueType type                   ,
                                          const CString& defaultVal         ,
                                          UInt32 propertyFlags              ,
                                          const CString& propertyCategory   ,
                                          const CString& propertyDescription )
    : name( WidgetPropertyToString( property ) )
    , id( property )
    , valueType( type )
    , defaultValue( defaultVal )
    , flags( propertyFlags )
    , category( propertyCategory )
    , description( propertyDescription )
    , enumValues()
{
}

/*-------------------------------------------------------------------------*/

bool
CPropertyDescriptor::HasFlag( UInt32 flag ) const
{
    return flag == ( flags & flag );
}

/*-------------------------------------------------------------------------*/

bool
CPropertyDescriptor::Validate( const CString& value, CString& normalized ) const
{
    switch ( valueType )
    {
        case VALUETYPE_BOOL:
        {
            CString lower = value.Trim( true ).Trim( false ).Lowercase();
            if ( lower == "true" || lower == "1" || lower == "yes" || lower == "on" )
            {
                normalized = "true";
                return true;
            }
            if ( lower == "false" || lower == "0" || lower == "no" || lower == "off" )
            {
                normalized = "false";
                return true;
            }
            return false;
        }
        case VALUETYPE_INT:
        {
            if ( !IsInteger( value ) )
                return false;
            normalized = CORE::Int32ToString( CORE::StringToInt32( value.Trim( true ).Trim( false ) ) );
            return true;
        }
        case VALUETYPE_FLOAT:
        {
            if ( !IsNumber( value ) )
                return false;
            normalized = value.Trim( true ).Trim( false );
            return true;
        }
        case VALUETYPE_UDIM:
        {
            CUDim dim;
            if ( !CUDim::FromString( value, dim ) )
                return false;
            normalized = dim.ToString();
            return true;
        }
        case VALUETYPE_UVECTOR:
        {
            CUVector vec;
            if ( !CUVector::FromString( value, vec ) )
                return false;
            normalized = vec.ToString();
            return true;
        }
        case VALUETYPE_COLOUR:
        {
            CColour colour;
            if ( !CColour::FromString( value, colour ) )
                return false;
            normalized = colour.ToString();
            return true;
        }
        case VALUETYPE_RECT:
        {
            CRect rect;
            if ( !CRect::FromString( value, rect ) )
                return false;
            normalized = rect.ToString();
            return true;
        }
        case VALUETYPE_ENUM:
        {
            for ( UInt32 i=0; i<(UInt32)enumValues.size(); ++i )
            {
                if ( enumValues[ i ].Equals( value, false ) )
                {
                    normalized = enumValues[ i ];
                    return true;
                }
            }
            return false;
        }
        case VALUETYPE_INTLIST:
        {
            if ( value.IsNULLOrEmpty() )
            {
                normalized = value;
                return true;
            }
            CString::StringVector parts = value.ParseElements( ',', true );
            CString result;
            for ( UInt32 i=0; i<(UInt32)parts.size(); ++i )
            {
                if ( !IsInteger( parts[ i ] ) )
                    return false;
                if ( i > 0 )
                    result += ',';
                result += CORE::Int32ToString( CORE::StringToInt32( parts[ i ].Trim( true ).Trim( false ) ) );
            }
            normalized = result;
            return true;
        }
        case VALUETYPE_STRING:
        case VALUETYPE_IMAGE:
        case VALUETYPE_FONT:
        case VALUETYPE_STRINGLIST:
        default:
        {
            normalized = value;
            return true;
        }
    }
}

/*-------------------------------------------------------------------------*/

CString
CPropertyDescriptor::ValueTypeToString( TValueType type )
{
    switch ( type )
    {
        case VALUETYPE_STRING: return "String";
        case VALUETYPE_BOOL: return "Bool";
        case VALUETYPE_INT: return "Int";
        case VALUETYPE_FLOAT: return "Float";
        case VALUETYPE_UDIM: return "UDim";
        case VALUETYPE_UVECTOR: return "UVector";
        case VALUETYPE_COLOUR: return "Colour";
        case VALUETYPE_RECT: return "Rect";
        case VALUETYPE_ENUM: return "Enum";
        case VALUETYPE_IMAGE: return "Image";
        case VALUETYPE_FONT: return "Font";
        case VALUETYPE_STRINGLIST: return "StringList";
        case VALUETYPE_INTLIST: return "IntList";
        default: return "Unknown";
    }
}

/*-------------------------------------------------------------------------*/

CWidgetTypeInfo::CWidgetTypeInfo( void )
    : typeName()
    , baseTypeName()
    , category()
    , description()
    , creator( GUCEF_NULL )
    , properties()
{
}

/*-------------------------------------------------------------------------*/

CWidgetTypeInfo&
CWidgetTypeInfo::AddProperty( const CPropertyDescriptor& descriptor )
{
    properties.push_back( descriptor );
    return *this;
}

/*-------------------------------------------------------------------------*/

namespace {

/** Gives every registry its own generations: a widget that remembers one never mistakes the resolved types of another registry for current */
UInt32
NextRegistryGeneration( void )
{
    static MT::CMutex lock;
    static UInt32 counter = 0;
    MT::CScopeMutex scopeLock( lock );
    return ++counter;
}

}

/*-------------------------------------------------------------------------*/

CResolvedWidgetType::CResolvedWidgetType( const CString& typeName, UInt32 generation )
    : m_typeName( typeName )
    , m_generation( generation )
    , m_chain()
    , m_descriptors()
    , m_defaults()
    , m_all()
    , m_custom()
{
    for ( UInt32 i=0; i<(UInt32)EWidgetProperty_Count; ++i )
    {
        m_byId[ i ] = GUCEF_NULL;
        m_defaultById[ i ] = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CResolvedWidgetType::GetTypeName( void ) const
{
    return m_typeName;
}

/*-------------------------------------------------------------------------*/

const CResolvedWidgetType::TStringVector&
CResolvedWidgetType::GetTypeChain( void ) const
{
    return m_chain;
}

/*-------------------------------------------------------------------------*/

bool
CResolvedWidgetType::IsA( const CString& baseTypeName ) const
{
    for ( UInt32 i=0; i<(UInt32)m_chain.size(); ++i )
        if ( m_chain[ i ] == baseTypeName )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CResolvedWidgetType::IsA( const char* baseTypeName ) const
{
    for ( UInt32 i=0; i<(UInt32)m_chain.size(); ++i )
        if ( 0 == strcmp( m_chain[ i ].C_String(), baseTypeName ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

const CResolvedWidgetType::TDescriptorVector&
CResolvedWidgetType::GetProperties( void ) const
{
    return m_all;
}

/*-------------------------------------------------------------------------*/

const CPropertyDescriptor*
CResolvedWidgetType::FindProperty( EWidgetProperty property ) const
{
    if ( (UInt32) property >= (UInt32) EWidgetProperty_Count )
        return GUCEF_NULL;
    return m_byId[ property ];
}

/*-------------------------------------------------------------------------*/

const CPropertyDescriptor*
CResolvedWidgetType::FindProperty( const CString& propertyName ) const
{
    EWidgetProperty property = StringToWidgetProperty( propertyName );
    if ( EWidgetProperty_Unknown != property )
        return m_byId[ property ];
    TDescriptorMap::const_iterator i = m_custom.find( propertyName );
    if ( i != m_custom.end() )
        return (*i).second;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CWidgetPropertyValue*
CResolvedWidgetType::FindDefault( EWidgetProperty property ) const
{
    if ( (UInt32) property >= (UInt32) EWidgetProperty_Count )
        return GUCEF_NULL;
    return m_defaultById[ property ];
}

/*-------------------------------------------------------------------------*/

UInt32
CResolvedWidgetType::GetGeneration( void ) const
{
    return m_generation;
}

/*-------------------------------------------------------------------------*/

CWidgetTypeRegistry::CWidgetTypeRegistry( void )
    : m_types()
    , m_resolved()
    , m_retired()
    , m_generation( NextRegistryGeneration() )
    , m_lock()
{
}

/*-------------------------------------------------------------------------*/

CWidgetTypeRegistry::~CWidgetTypeRegistry()
{
    ForgetResolvedTypes();
    for ( UInt32 i=0; i<(UInt32)m_retired.size(); ++i )
        delete m_retired[ i ];
    m_retired.clear();
}

/*-------------------------------------------------------------------------*/

void
CWidgetTypeRegistry::ForgetResolvedTypes( void )
{
    // widgets may still point to them: they are kept until the registry goes
    for ( TResolvedMap::iterator i=m_resolved.begin(); i!=m_resolved.end(); ++i )
        m_retired.push_back( (*i).second );
    m_resolved.clear();
    m_generation = NextRegistryGeneration();
}

/*-------------------------------------------------------------------------*/

UInt32
CWidgetTypeRegistry::GetGeneration( void ) const
{
    return m_generation;
}

/*-------------------------------------------------------------------------*/

bool
CWidgetTypeRegistry::RegisterType( const CWidgetTypeInfo& typeInfo )
{
    if ( typeInfo.typeName.IsNULLOrEmpty() )
        return false;

    MT::CScopeMutex lock( m_lock );
    m_types[ typeInfo.typeName ] = typeInfo;
    ForgetResolvedTypes();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWidgetTypeRegistry::UnregisterType( const CString& typeName )
{
    MT::CScopeMutex lock( m_lock );
    ForgetResolvedTypes();
    return m_types.erase( typeName ) > 0;
}

/*-------------------------------------------------------------------------*/

const CWidgetTypeInfo*
CWidgetTypeRegistry::GetTypeInfo( const CString& typeName ) const
{
    MT::CScopeMutex lock( m_lock );
    TTypeMap::const_iterator i = m_types.find( typeName );
    if ( i != m_types.end() )
        return &(*i).second;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CWidgetTypeRegistry::IsRegistered( const CString& typeName ) const
{
    return GUCEF_NULL != GetTypeInfo( typeName );
}

/*-------------------------------------------------------------------------*/

CWidgetTypeRegistry::TStringSet
CWidgetTypeRegistry::GetTypeNames( void ) const
{
    MT::CScopeMutex lock( m_lock );
    TStringSet names;
    TTypeMap::const_iterator i = m_types.begin();
    while ( i != m_types.end() )
    {
        names.insert( (*i).first );
        ++i;
    }
    return names;
}

/*-------------------------------------------------------------------------*/

GUCEF::vector< CString >
CWidgetTypeRegistry::GetTypeChain( const CString& typeName ) const
{
    const CResolvedWidgetType* resolved = ResolveType( typeName );
    if ( GUCEF_NULL == resolved )
        return GUCEF::vector< CString >();
    return resolved->GetTypeChain();
}

/*-------------------------------------------------------------------------*/

bool
CWidgetTypeRegistry::IsA( const CString& typeName, const CString& baseTypeName ) const
{
    const CResolvedWidgetType* resolved = ResolveType( typeName );
    return GUCEF_NULL != resolved && resolved->IsA( baseTypeName );
}

/*-------------------------------------------------------------------------*/

CWidgetTypeRegistry::TDescriptorVector
CWidgetTypeRegistry::GetProperties( const CString& typeName ) const
{
    const CResolvedWidgetType* resolved = ResolveType( typeName );
    if ( GUCEF_NULL == resolved )
        return TDescriptorVector();
    return resolved->GetProperties();
}

/*-------------------------------------------------------------------------*/

const CPropertyDescriptor*
CWidgetTypeRegistry::FindProperty( const CString& typeName, const CString& propertyName ) const
{
    const CResolvedWidgetType* resolved = ResolveType( typeName );
    return GUCEF_NULL == resolved ? GUCEF_NULL : resolved->FindProperty( propertyName );
}

/*-------------------------------------------------------------------------*/

const CPropertyDescriptor*
CWidgetTypeRegistry::FindProperty( const CString& typeName, EWidgetProperty property ) const
{
    const CResolvedWidgetType* resolved = ResolveType( typeName );
    return GUCEF_NULL == resolved ? GUCEF_NULL : resolved->FindProperty( property );
}

/*-------------------------------------------------------------------------*/

const CResolvedWidgetType*
CWidgetTypeRegistry::ResolveType( const CString& typeName ) const
{
    MT::CScopeMutex lock( m_lock );

    TResolvedMap::const_iterator known = m_resolved.find( typeName );
    if ( known != m_resolved.end() )
        return (*known).second;

    // the chain: the type and the registered types it extends
    GUCEF::vector< const CWidgetTypeInfo* > infos;
    CResolvedWidgetType* resolved = GUCEF_NEW CResolvedWidgetType( typeName, m_generation );
    CString current = typeName;
    while ( !current.IsNULLOrEmpty() && resolved->m_chain.size() < 64 )
    {
        TTypeMap::const_iterator i = m_types.find( current );
        if ( i == m_types.end() )
            break;
        resolved->m_chain.push_back( current );
        infos.push_back( &(*i).second );
        current = (*i).second.baseTypeName;
    }
    if ( infos.empty() )
    {
        delete resolved;
        return GUCEF_NULL;
    }

    // copies of every descriptor, base type first, so the pointers stay good for the life of the object
    UInt32 total = 0;
    for ( UInt32 c=0; c<(UInt32)infos.size(); ++c )
        total += (UInt32) infos[ c ]->properties.size();
    resolved->m_descriptors.reserve( total );
    resolved->m_defaults.reserve( total );
    GUCEF::vector< UInt32 > levelStart( infos.size(), 0 );
    for ( Int32 c=(Int32)infos.size()-1; c>=0; --c )
    {
        levelStart[ c ] = (UInt32) resolved->m_descriptors.size();
        for ( UInt32 p=0; p<(UInt32)infos[ c ]->properties.size(); ++p )
        {
            CPropertyDescriptor descriptor = infos[ c ]->properties[ p ];
            descriptor.id = StringToWidgetProperty( descriptor.name );
            resolved->m_descriptors.push_back( descriptor );
            resolved->m_defaults.push_back( CWidgetPropertyValue( descriptor.defaultValue ) );
            resolved->m_defaults.back().ReadAll();
        }
    }

    // every property, base properties first: a derived type may redefine a property (for example another default), the most derived wins
    for ( Int32 c=(Int32)infos.size()-1; c>=0; --c )
    {
        for ( UInt32 p=0; p<(UInt32)infos[ c ]->properties.size(); ++p )
        {
            const CPropertyDescriptor* descriptor = &resolved->m_descriptors[ levelStart[ c ] + p ];
            bool replaced = false;
            for ( UInt32 r=0; r<(UInt32)resolved->m_all.size(); ++r )
            {
                if ( resolved->m_all[ r ]->name == descriptor->name )
                {
                    resolved->m_all[ r ] = descriptor;
                    replaced = true;
                    break;
                }
            }
            if ( !replaced )
                resolved->m_all.push_back( descriptor );
        }
    }

    // by id and by name: the most derived type that has the property, its first mention
    for ( UInt32 c=0; c<(UInt32)infos.size(); ++c )
    {
        for ( UInt32 p=0; p<(UInt32)infos[ c ]->properties.size(); ++p )
        {
            UInt32 index = levelStart[ c ] + p;
            const CPropertyDescriptor* descriptor = &resolved->m_descriptors[ index ];
            if ( EWidgetProperty_Unknown != descriptor->id )
            {
                if ( GUCEF_NULL == resolved->m_byId[ descriptor->id ] )
                {
                    resolved->m_byId[ descriptor->id ] = descriptor;
                    resolved->m_defaultById[ descriptor->id ] = &resolved->m_defaults[ index ];
                }
            }
            else
            if ( resolved->m_custom.find( descriptor->name ) == resolved->m_custom.end() )
                resolved->m_custom[ descriptor->name ] = descriptor;
        }
    }

    m_resolved[ typeName ] = resolved;
    return resolved;
}

/*-------------------------------------------------------------------------*/

CWidget*
CWidgetTypeRegistry::CreateWidget( const CString& typeName ) const
{
    const CWidgetTypeInfo* info = GetTypeInfo( typeName );
    if ( GUCEF_NULL == info || GUCEF_NULL == info->creator )
        return GUCEF_NULL;
    return info->creator();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
