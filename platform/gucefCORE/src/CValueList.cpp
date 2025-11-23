/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <assert.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "CValueList.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_IMPLEMENT_MSGEXCEPTION( CValueList, EUnknownKey );
GUCEF_IMPLEMENT_MSGEXCEPTION( CValueList, EIndexOutOfRange );

/*-------------------------------------------------------------------------*/

CValueList::CValueList( void )
    : CIConfigurable()
    , m_list()
    , m_allowDuplicates( false )
    , m_allowMultipleValues( true )
    , m_configNamespace()
    , m_configKeyNamespace()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CValueList::CValueList( const CValueList& src )
    : CIConfigurable()
    , m_list( src.m_list )
    , m_allowDuplicates( src.m_allowDuplicates )
    , m_allowMultipleValues( src.m_allowMultipleValues )
    , m_configNamespace( src.m_configNamespace )
    , m_configKeyNamespace( src.m_configKeyNamespace )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CValueList::~CValueList()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CValueList&
CValueList::operator=( const CValueList& src )
{GUCEF_TRACE;

     if ( this != &src )
     {
        m_list = src.m_list;
        m_allowDuplicates = src.m_allowDuplicates;
        m_allowMultipleValues = src.m_allowMultipleValues;
        m_configNamespace = src.m_configNamespace;
        m_configKeyNamespace = src.m_configKeyNamespace;
     }
     return *this;
}

/*-------------------------------------------------------------------------*/

CVariant&
CValueList::operator[]( const CString& key )
{GUCEF_TRACE;

    TVariantVector& values = m_list[ key ];
    if ( values.empty() )
    {
        values.push_back( CVariant::Empty );
    }
    return (*values.begin());
}

/*-------------------------------------------------------------------------*/

const CVariant&
CValueList::operator[]( const CString& key ) const
{GUCEF_TRACE;

    return GetValue( key );
}

/*-------------------------------------------------------------------------*/

void 
CValueList::SetMultiple( const CValueList& src    ,
                         const CString& keyPrefix )
{GUCEF_TRACE;

    TValueMap::const_iterator i = src.m_list.begin();
    while ( i != src.m_list.end() )
    {
        const CString& key = (*i).first;
        const TVariantVector& values = (*i).second;

        TVariantVector::const_iterator n = values.begin();
        while ( n != values.end() )
        {
            // We will just use the 'set' function
            // the settings on this value list instance will ensure proper merge strategy
            Set( keyPrefix + key, (*n) );
            ++n;
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CValueList::SetMultiple( int argc    ,
                         char** argv )
{GUCEF_TRACE;

    for ( int i=0; i<argc; ++i )
    {
        SetUsingKvCombo( argv[ i ], '=', GUCEF_NULL );
    }
}

/*-------------------------------------------------------------------------*/

void
CValueList::SetConfigNamespace( const CString& configNamespace )
{GUCEF_TRACE;

    m_configNamespace = configNamespace;
}

/*-------------------------------------------------------------------------*/

const CString&
CValueList::GetConfigNamespace( void ) const
{GUCEF_TRACE;

    return m_configNamespace;
}

/*-------------------------------------------------------------------------*/

void
CValueList::SetConfigKeyNamespace( const CString& configKeyNamespace )
{GUCEF_TRACE;

    m_configKeyNamespace = configKeyNamespace;
}

/*-------------------------------------------------------------------------*/

const CString&
CValueList::GetConfigKeyNamespace( void ) const
{GUCEF_TRACE;

    return m_configKeyNamespace;
}

/*-------------------------------------------------------------------------*/

bool
CValueList::SaveConfig( CDataNode& tree ) const
{GUCEF_TRACE;

    CDataNode* nodeNamespaceRoot = tree.Structure( m_configNamespace, '/' );
    if ( GUCEF_NULL != nodeNamespaceRoot )
    {
        if ( m_allowDuplicates || m_allowMultipleValues )
        {
            TValueMap::const_iterator i = m_list.begin();
            while ( i != m_list.end() )
            {
                const TVariantVector& values = (*i).second;
                TVariantVector::const_iterator n = values.begin();
                while ( n != values.end() )
                {
                    CDataNode* child = nodeNamespaceRoot->AddChild( m_configKeyNamespace + (*i).first );
                    child->SetValue( (*n) );
                    ++n;
                }
                ++i;
            }
        }
        else
        {
            TValueMap::const_iterator i = m_list.begin();
            while ( i != m_list.end() )
            {
                const TVariantVector& values = (*i).second;
                if ( !values.empty() )
                    nodeNamespaceRoot->SetAttribute( m_configKeyNamespace + (*i).first, values.front() );
                ++i;
            }
        }
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CValueList::LoadConfig( const CDataNode& treeroot )
{GUCEF_TRACE;

    const CDataNode* nodeNamespaceRoot = treeroot.Search( m_configNamespace, '/', true, true );
    if ( GUCEF_NULL != nodeNamespaceRoot )
    {
        // Get the key-value combos from the attributes
        CDataNode::TAttributeMap::const_iterator i = nodeNamespaceRoot->AttributeBegin();
        while ( i != nodeNamespaceRoot->AttributeEnd() )
        {
            if ( m_configKeyNamespace.IsNULLOrEmpty() )
            {
                Set( (*i).first, (*i).second );
            }
            else
            {
                if ( 0 == (*i).first.HasSubstr( m_configKeyNamespace ) )
                {
                    CString keyname = (*i).first.CutChars( m_configKeyNamespace.Length(), true, 0 );
                    Set( keyname, (*i).second );
                }
            }
            ++i;
        }

        // Also get the key-value combo's of Child nodes names + default value
        CDataNode::const_iterator n = nodeNamespaceRoot->Begin();
        while ( n != nodeNamespaceRoot->End() )
        {
            CString value = (*n)->GetValue().AsString();
            if ( !value.IsNULLOrEmpty() )
            {
                if ( m_configKeyNamespace.IsNULLOrEmpty() )
                {
                    Set( (*n)->GetName(), value );
                }
                else
                {
                    if ( 0 == (*n)->GetName().HasSubstr( m_configKeyNamespace ) )
                    {
                        CString keyname = (*n)->GetName().CutChars( m_configKeyNamespace.Length(), true, 0 );
                        Set( keyname, value );
                    }
                }
            }
            ++n;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CValueList::Serialize( CDataNode& domRootNode                        ,
                       const CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    if ( CDataNodeSerializableSettings::DataNodeSerializableLod_KeyOnly == settings.levelOfDetail )
    {
        domRootNode.SetNodeType( GUCEF_DATATYPE_ARRAY );        
        TValueMap::const_iterator i = m_list.begin();
        while ( i != m_list.end() )
        {
            domRootNode.AddValueAsChild( (*i).first );
            ++i;
        }
        return true;
    }
    else
    {
        // For now just use SaveConfig until we disentangle this
        return SaveConfig( domRootNode );
    }
}

/*-------------------------------------------------------------------------*/

bool
CValueList::Deserialize( const CDataNode& domRootNode                  ,
                         const CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    // For now just use LoadConfig until we disentangle this
    return LoadConfig( domRootNode );

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CValueList::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::CORE::CValueList";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CValueList::SetMultiple( const CString& keyandvalue       ,
                         const char pairSeparator         ,
                         const char kvSeperator           ,
                         const CString* optionalKeyPrefix )
{GUCEF_TRACE;

    CString::StringVector kvPairs = keyandvalue.ParseElements( pairSeparator, false ); 
    CString::StringVector::iterator i = kvPairs.begin();
    while ( i != kvPairs.end() )
    {
        SetUsingKvCombo( (*i), kvSeperator, optionalKeyPrefix );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CValueList::SetAllowDuplicates( const bool allowDuplicates )
{GUCEF_TRACE;

    m_allowDuplicates = allowDuplicates;
}

/*-------------------------------------------------------------------------*/

bool
CValueList::GetAllowDuplicates( void ) const
{GUCEF_TRACE;

    return m_allowDuplicates;
}

/*-------------------------------------------------------------------------*/

void
CValueList::SetAllowMultipleValues( const bool allowMultipleValues )
{GUCEF_TRACE;

    m_allowMultipleValues = allowMultipleValues;
}

/*-------------------------------------------------------------------------*/

bool
CValueList::GetAllowMultipleValues( void ) const
{GUCEF_TRACE;

    return m_allowMultipleValues;
}

/*-------------------------------------------------------------------------*/

void
CValueList::SetUsingKvCombo( const CString& keyAndValue       , 
                             const char kvSeperator           ,
                             const CString* optionalKeyPrefix ,
                             UInt8 valueType                  )
{GUCEF_TRACE;

    CString key( keyAndValue.SubstrToChar( kvSeperator, true ) );
    CVariant value( valueType, keyAndValue.SubstrToChar( kvSeperator, false ), CVariant::Empty );
    
    if ( GUCEF_NULL == optionalKeyPrefix )
    {
        Set( key, value );
    }
    else
    {
        Set( *optionalKeyPrefix + key, value );
    }
}

/*-------------------------------------------------------------------------*/

void
CValueList::Set( const CVariant& key   ,
                 const CVariant& value )
{GUCEF_TRACE;

    if ( !key.IsNULLOrEmpty() )
    {
        TVariantVector& values = m_list[ key ];
        if ( m_allowDuplicates )
        {
            values.push_back( value );
        }
        else
        {
            // Duplicates are not allowed so if an identical
            // value is already in the list we should not add it
            TVariantVector::iterator n = values.begin();
            while ( n != values.end() )
            {
                if ( (*n) == value )
                {
                    // The given value is already in the list
                    return;
                }
                ++n;
            }

            if ( !m_allowMultipleValues )
            {
                values.clear();
            }
            values.push_back( value );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CValueList::Set( const char* key       ,
                 const CVariant& value )
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );
    
    Set( keyVar, value );
}

/*-------------------------------------------------------------------------*/

void
CValueList::Set( const CString& key   ,
                 const CVariant& value )
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );
    
    Set( keyVar, value );
}

/*-------------------------------------------------------------------------*/

CValueList::TVariantVector
CValueList::GetValueVectorAlways( const CString& key ) const
{GUCEF_TRACE;

    TVariantVector results;
    TValueMap::const_iterator i = m_list.find( key );
    if ( i != m_list.end() )
    {
        results = (*i).second;
    }
    return results;
}

/*-------------------------------------------------------------------------*/

CValueList::TVariantVector
CValueList::GetValueVectorAlways( const CVariant& key                 ,
                                  char valueSepChar                   ,
                                  const TVariantVector& defaultValues ) const
{GUCEF_TRACE;

    const TVariantVector* firstPass = GUCEF_NULL;
    TVariantVector results;
    TValueMap::const_iterator i = m_list.find( key );
    if ( i != m_list.end() )
    {
        firstPass = &(*i).second;
        TVariantVector::const_iterator n = firstPass->begin();
        while ( n != firstPass->end() )
        {
            TStringVector subSetResults = (*n).AsString().ParseElements( valueSepChar,  false );
            TStringVector::iterator m = subSetResults.begin();
            while ( m != subSetResults.end() )
            {
                results.push_back( (*m) );
                ++m;
            }
            ++n;
        }
    }

    if ( results.empty() )
    {
        results = defaultValues;
    }
    return results;
}

/*-------------------------------------------------------------------------*/

CValueList::TVariantVector
CValueList::GetValueVectorAlways( const CString& key                  ,
                                  char valueSepChar                   ,
                                  const TVariantVector& defaultValues ) const
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );

    return GetValueVectorAlways( keyVar, valueSepChar, defaultValues );
}

/*-------------------------------------------------------------------------*/

CValueList::TVariantVector
CValueList::GetValueVectorAlways( const CString& key, char valueSepChar ) const
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );

    return GetValueVectorAlways( keyVar, valueSepChar );
}

/*-------------------------------------------------------------------------*/

CValueList::TVariantVector
CValueList::GetValueVectorAlways( const CVariant& key, char valueSepChar ) const
{GUCEF_TRACE;

    TVariantVector emptyDefault;
    return GetValueVectorAlways( key, valueSepChar, emptyDefault );
}

/*-------------------------------------------------------------------------*/

CVariant
CValueList::GetValueAlways( const CVariant& key      ,
                            const char* defaultValue ) const
{GUCEF_TRACE;

    CVariant defaultValueVar;
    defaultValueVar.LinkTo( defaultValue );
    return GetValueAlways( key, defaultValueVar );
}

/*-------------------------------------------------------------------------*/

CVariant
CValueList::GetValueAlways( const CString& key          ,
                            const CString& defaultValue ) const
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );
    CVariant defaultValueVar;
    defaultValueVar.LinkTo( defaultValue );

    return GetValueAlways( keyVar, defaultValueVar );
}

/*-------------------------------------------------------------------------*/

CVariant
CValueList::GetValueAlways( const char* key              ,
                            const CVariant& defaultValue ) const
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );

    return GetValueAlways( keyVar, defaultValue );
}

/*-------------------------------------------------------------------------*/

CVariant
CValueList::GetValueAlways( const char* key          ,
                            const char* defaultValue ) const
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );
    CVariant defaultValueVar;
    defaultValueVar.LinkTo( defaultValue );

    return GetValueAlways( keyVar, defaultValueVar );
}

/*-------------------------------------------------------------------------*/

CVariant
CValueList::GetValueAlways( const char* key             , 
                            const CString& defaultValue ) const
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );
    CVariant defaultValueVar;
    defaultValueVar.LinkTo( defaultValue );

    return GetValueAlways( keyVar, defaultValueVar );
}

/*-------------------------------------------------------------------------*/

CVariant
CValueList::GetValueAlways( const CString& key       , 
                            const char* defaultValue ) const
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );
    CVariant defaultValueVar;
    defaultValueVar.LinkTo( defaultValue );

    return GetValueAlways( keyVar, defaultValueVar );
}

/*-------------------------------------------------------------------------*/

CVariant
CValueList::GetValueAlways( const CString& key           ,
                            const CVariant& defaultValue ) const
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );
    return GetValueAlways( keyVar, defaultValue );
}

/*-------------------------------------------------------------------------*/

const CVariant&
CValueList::GetValueAlways( const CVariant& key          ,
                            const CVariant& defaultValue ) const
{GUCEF_TRACE;

    TValueMap::const_iterator i = m_list.find( key );
    if ( i != m_list.end() )
    {
        return (*i).second[ 0 ];
    }

    return defaultValue;
}

/*-------------------------------------------------------------------------*/

const CVariant&
CValueList::GetValue( const CString& key ) const
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );

    return GetValue( keyVar );
}

/*-------------------------------------------------------------------------*/

const CVariant&
CValueList::GetValue( const CVariant& key ) const
{GUCEF_TRACE;

    TValueMap::const_iterator i = m_list.find( key );
    if ( i != m_list.end() )
    {
        return (*i).second[ 0 ];
    }

    GUCEF_EMSGTHROW( EUnknownKey, CString( "CValueList::GetValue(): The given key \"" + key.AsString() + "\" is not found" ).C_String() );
}

/*-------------------------------------------------------------------------*/

const CVariant&
CValueList::GetValue( const char* key ) const
{GUCEF_TRACE;

    CORE::CVariant charBuffer;
    charBuffer.LinkTo( key, (UInt8) GUCEF_DATATYPE_STRING );
    return GetValue( charBuffer );
}

/*-------------------------------------------------------------------------*/

CVariant&
CValueList::GetValue( const CString& key )
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );

    return GetValue( keyVar );
}

/*-------------------------------------------------------------------------*/

CVariant&
CValueList::GetValue( const CVariant& key )
{GUCEF_TRACE;

    TValueMap::iterator i = m_list.find( key );
    if ( i != m_list.end() )
    {
        return (*i).second[ 0 ];
    }

    GUCEF_EMSGTHROW( EUnknownKey, CString( "CValueList::GetValue(): The given key \"" + key.AsString() + "\" is not found" ).C_String() );
}

/*-------------------------------------------------------------------------*/

bool
CValueList::TryGetValue( const CString& key, CVariant& outValue ) const
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );

    return TryGetValue( keyVar, outValue );
}

/*-------------------------------------------------------------------------*/

bool
CValueList::TryGetValue( const char* key, CVariant& outValue ) const
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );

    return TryGetValue( keyVar, outValue );
}

/*-------------------------------------------------------------------------*/

bool
CValueList::TryGetValue( const CVariant& key, CVariant& outValue ) const
{GUCEF_TRACE;

    TValueMap::const_iterator i = m_list.find( key );
    if ( i != m_list.end() )
    {
        outValue = (*i).second[ 0 ];
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CValueList::TryGetValue( const char* key, CVariant& outValue, bool linkIfPossible )
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );

    return TryGetValue( keyVar, outValue, linkIfPossible );
}

/*-------------------------------------------------------------------------*/

bool
CValueList::TryGetValue( const CString& key, CVariant& outValue, bool linkIfPossible )
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );

    return TryGetValue( keyVar, outValue, linkIfPossible );
}

/*-------------------------------------------------------------------------*/

bool
CValueList::TryGetValue( const CVariant& key, CVariant& outValue, bool linkIfPossible )
{GUCEF_TRACE;

    TValueMap::iterator i = m_list.find( key );
    if ( i != m_list.end() )
    {
        if ( linkIfPossible )
        {
            outValue.LinkTo( (*i).second[ 0 ] );
        }
        else
        {
            outValue = (*i).second[ 0 ];
        }
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CValueList::TryGetValueVector( const CString& key, TVariantVector& outValue, bool linkIfPossible )
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );

    return TryGetValueVector( keyVar, outValue, linkIfPossible );
}

/*-------------------------------------------------------------------------*/

bool 
CValueList::TryGetValueVector( const char* key, TVariantVector& outValue, bool linkIfPossible )
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );

    return TryGetValueVector( keyVar, outValue, linkIfPossible );
}

/*-------------------------------------------------------------------------*/

bool 
CValueList::TryGetValueVector( const CVariant& key, TVariantVector& outValue, bool linkIfPossible )
{GUCEF_TRACE;

    TValueMap::iterator i = m_list.find( key );
    if ( i != m_list.end() )
    {
        if ( linkIfPossible )
        {
            TVariantVector& values = (*i).second;
            outValue.reserve( values.size() );
            TVariantVector::iterator n = values.begin();
            while ( n != values.end() )
            {
                CVariant value;
                outValue.push_back( value );
                outValue.back().LinkTo( (*n) );
                ++n;
            }
        }
        else
        {
            outValue = (*i).second;
        }
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CValueList::TryGetValueVector( const char* key, TVariantVector& outValue ) const
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );

    return TryGetValueVector( keyVar, outValue );
}

/*-------------------------------------------------------------------------*/

bool 
CValueList::TryGetValueVector( const CString& key, TVariantVector& outValue ) const
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );

    return TryGetValueVector( keyVar, outValue );
}

/*-------------------------------------------------------------------------*/

bool 
CValueList::TryGetValueVector( const CVariant& key, TVariantVector& outValue ) const
{GUCEF_TRACE;

    TValueMap::const_iterator i = m_list.find( key );
    if ( i != m_list.end() )
    {
        outValue = (*i).second;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CValueList::TVariantVector&
CValueList::GetValueVector( const CString& key )
{GUCEF_TRACE;

    TValueMap::iterator i = m_list.find( key );
    if ( i != m_list.end() )
    {
        return (*i).second;
    }

    GUCEF_EMSGTHROW( EUnknownKey, "CValueList::GetValue(): The given key is not found" );
}

/*-------------------------------------------------------------------------*/

const CValueList::TVariantVector&
CValueList::GetValueVector( const CString& key ) const
{GUCEF_TRACE;

    TValueMap::const_iterator i = m_list.find( key );
    if ( i != m_list.end() )
    {
        return (*i).second;
    }

    GUCEF_EMSGTHROW( EUnknownKey, "CValueList::GetValue(): The given key is not found" );
}

/*-------------------------------------------------------------------------*/

CValueList::TStringVector
CValueList::GetValueStringVector( const CString& key ) const
{GUCEF_TRACE;

    TStringVector result;
    TValueMap::const_iterator i = m_list.find( key );
    if ( i != m_list.end() )
    {
        return ToStringVector( (*i).second );
    }

    GUCEF_EMSGTHROW( EUnknownKey, "CValueList::GetValue(): The given key is not found" );
}

/*-------------------------------------------------------------------------*/

const CVariant&
CValueList::GetValue( const UInt32 index ) const
{GUCEF_TRACE;

    if ( index < m_list.size() )
    {
        TValueMap::const_iterator i = m_list.begin();
        for ( UInt32 n=0; n<index; ++n ) { ++i; }

        return (*i).second[ 0 ];
    }

    GUCEF_EMSGTHROW( EIndexOutOfRange, "CValueList::GetValue( index ): The given index is invalid" );
}

/*-------------------------------------------------------------------------*/

CVariant&
CValueList::GetValue( const UInt32 index )
{GUCEF_TRACE;

    if ( index < m_list.size() )
    {
        TValueMap::iterator i = m_list.begin();
        for ( UInt32 n=0; n<index; ++n ) { ++i; }

        return (*i).second[ 0 ];
    }

    GUCEF_EMSGTHROW( EIndexOutOfRange, "CValueList::GetValue( index ): The given index is invalid" );
}

/*-------------------------------------------------------------------------*/

CString
CValueList::GetPair( const UInt32 index ) const
{GUCEF_TRACE;

    const CString& key = GetKey( index );
    const CVariant& value = GetValue( index );

    // if no exception was thrown we can now proceed to build the pair
    CString pair = key + '=' + value.AsString();
    return pair;
}

/*-------------------------------------------------------------------------*/

CString
CValueList::GetAllPairs( const UInt32 index          ,
                         const CString& seperatorStr ) const
{GUCEF_TRACE;

    const CString& key = GetKey( index );
    const TVariantVector& values = GetValueVector( index );

    CString resultStr;
    bool first = true;
    for ( UInt32 i=0; i<values.size(); ++i )
    {
        if ( !first )
        {
            resultStr += seperatorStr;
        }
        else
        {
            first = false;
        }

        resultStr += key + '=' + values[ i ];
    }

    return resultStr;
}

/*-------------------------------------------------------------------------*/

CString
CValueList::GetAllPairs( const CString& keyValueSeperatorStr ,
                         const CString& kvPairSeperatorStr   ,
                         bool envelopElements                ,
                         const CString& envelopStr           ) const
{GUCEF_TRACE;

    CString resultStr;
    bool first = true;
    TValueMap::const_iterator i = m_list.begin();
    while ( i != m_list.end() )
    {
        const CString& key = (*i).first;
        const TVariantVector& values = (*i).second;

        TVariantVector::const_iterator n = values.begin();
        while ( n != values.end() )
        {
            if ( !first )
            {
                resultStr += kvPairSeperatorStr;
            }
            else
            {
                first = false;
            }

            if ( envelopElements )
                resultStr += envelopStr + key + envelopStr + keyValueSeperatorStr + envelopStr + (*n) + envelopStr;
            else
                resultStr += key + keyValueSeperatorStr + (*n);
            ++n;
        }
        ++i;
    }
    return resultStr;
}

/*-------------------------------------------------------------------------*/

CString
CValueList::GetPair( const CString& key ) const
{GUCEF_TRACE;

    const CVariant& value = GetValue( key );

    // if no exception was thrown we can now proceed to build the pair
    CString pair = key + '=' + value.AsString();
    return pair;
}

/*-------------------------------------------------------------------------*/

const CVariant&
CValueList::GetKey( const UInt32 index ) const
{GUCEF_TRACE;

    if ( index < m_list.size() )
    {
        TValueMap::const_iterator i = m_list.begin();
        for ( UInt32 n=0; n<index; ++n ) { ++i; }

        return (*i).first;
    }

    GUCEF_EMSGTHROW( EIndexOutOfRange, "CValueList::GetKey( index ): The given index is invalid" );
}

/*-------------------------------------------------------------------------*/

bool
CValueList::HasKey( const char* key ) const
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );

    return HasKey( keyVar );
}

/*-------------------------------------------------------------------------*/

bool
CValueList::HasKey( const CString& key ) const
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );

    return HasKey( keyVar );
}

/*-------------------------------------------------------------------------*/

bool
CValueList::HasKey( const CVariant& key ) const
{GUCEF_TRACE;

    return m_list.end() != m_list.find( key );
}

/*-------------------------------------------------------------------------*/

bool 
CValueList::HasKeyAndValue( const CString& key   , 
                            const CString& value ) const
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );
    CVariant valueVar;
    valueVar.LinkTo( value );

    return HasKeyAndValue( keyVar, valueVar );
}

/*-------------------------------------------------------------------------*/

bool 
CValueList::HasKeyAndValue( const CVariant& key   , 
                            const CVariant& value ) const
{GUCEF_TRACE;

    TValueMap::const_iterator i = m_list.find( key );
    if ( m_list.end() != i )
    {
        const TVariantVector& values = (*i).second;
        TVariantVector::const_iterator n = values.begin();
        while ( n != values.end() )
        {
            if ( (*n) == value )
                return true;
            ++n;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CValueList::TStringVector
CValueList::GetKeysWithWildcardKeyMatch( const CString& searchStr ,
                                         char wildCardChar        ,
                                         bool caseSensitive       ) const
{GUCEF_TRACE;

    TStringVector keys;
    TValueMap::const_iterator i = m_list.begin();
    while ( i != m_list.end() )
    {
        const CString& key = (*i).first;
        if ( key.WildcardEquals( searchStr, wildCardChar, caseSensitive ) )
        {
            keys.push_back( key );
        }
        ++i;
    }
    return keys;
}

/*-------------------------------------------------------------------------*/

void
CValueList::Delete( const char* key )
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );

    Delete( keyVar );
}

/*-------------------------------------------------------------------------*/

void
CValueList::Delete( const CString& key )
{GUCEF_TRACE;

    CVariant keyVar;
    keyVar.LinkTo( key );

    Delete( keyVar );
}

/*-------------------------------------------------------------------------*/

void
CValueList::Delete( const CVariant& key )
{GUCEF_TRACE;

    m_list.erase( key );
}

/*-------------------------------------------------------------------------*/

void
CValueList::DeleteAll( void )
{GUCEF_TRACE;

    m_list.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CValueList::GetKeyCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_list.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CValueList::GetValueCount( void ) const
{GUCEF_TRACE;

    size_t totalValues = 0;
    TValueMap::const_iterator i = m_list.begin();
    while ( i != m_list.end() )
    {
        totalValues += (*i).second.size();
        ++i;
    }

    return (UInt32) totalValues;
}

/*-------------------------------------------------------------------------*/

const CValueList::TVariantVector&
CValueList::GetValueVector( const UInt32 index ) const
{GUCEF_TRACE;

    if ( index < m_list.size() )
    {
        TValueMap::const_iterator i = m_list.begin();
        for ( UInt32 n=0; n<index; ++n ) { ++i; }

        return (*i).second;
    }

    GUCEF_EMSGTHROW( EIndexOutOfRange, "CValueList::GetValueVector( index ): The given index is invalid" )
}

/*-------------------------------------------------------------------------*/

CValueList::TValueMap::const_iterator
CValueList::GetDataBeginIterator( void ) const
{GUCEF_TRACE;

    return m_list.begin();
}

/*-------------------------------------------------------------------------*/

CValueList::TValueMap::const_iterator
CValueList::GetDataEndIterator( void ) const
{GUCEF_TRACE;

    return m_list.end();
}

/*-------------------------------------------------------------------------*/

void
CValueList::Clear( void )
{GUCEF_TRACE;

    m_list.clear();
    m_allowDuplicates = false;
    m_allowMultipleValues = true;
}

/*-------------------------------------------------------------------------*/

void 
CValueList::CopySettingsFrom( const CValueList& other )
{GUCEF_TRACE;

    m_allowDuplicates = other.m_allowDuplicates;
    m_allowMultipleValues = other.m_allowMultipleValues;
    m_configNamespace = other.m_configNamespace;
    m_configKeyNamespace = other.m_configKeyNamespace;    
}

/*-------------------------------------------------------------------------*/

CICloneable* 
CValueList::Clone( void ) const
{GUCEF_TRACE;

    return new CValueList( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
