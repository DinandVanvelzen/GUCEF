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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "CDataNode.h"  /* definition of the class implemented here */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataNode::CDataNode( int nodeType )
    : CIEnumerable()
    , m_nodeType( nodeType )
    , _pparent( NULL ) 
    , m_children()     
    , _pnext( NULL )   
    , _pprev( NULL )   
    , _name()          
    , m_value()
    , m_associatedData( GUCEF_NULL )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CDataNode::CDataNode( const CString& name, int nodeType )
    : CIEnumerable()
    , m_nodeType( nodeType )
    , _name( name )    
    , m_value()      
    , _pparent( NULL ) 
    , m_children()     
    , _pnext( NULL )   
    , _pprev( NULL )
    , m_associatedData( GUCEF_NULL )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CDataNode::CDataNode( const CDataNode& src )
    : CIEnumerable()
    , m_nodeType( src.m_nodeType )
    , _name( src._name )       
    , m_value( src.m_value ) 
    , _atts( src._atts )       
    , _pparent( src._pparent ) 
    , _pnext( src._pnext )     
    , _pprev( src._pprev )     
    , m_children()
    , m_associatedData( GUCEF_NULL )
{GUCEF_TRACE;
    
    if ( GUCEF_NULL != src.m_associatedData )
        m_associatedData = src.m_associatedData->Clone();    
    
    TDataNodeList::const_iterator n = src.m_children.cbegin();
    while ( n != src.m_children.cend() )
    {
        AddChild( *(*n) );
        ++n;
    }             
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED

CDataNode::CDataNode( CDataNode&& src ) GUCEF_NOEXCEPT
    : CIEnumerable()
    , m_nodeType( src.m_nodeType )
    , _name( GUCEF_MOVE( src._name ) )
    , m_value( GUCEF_MOVE( src.m_value ) )
    , _atts( GUCEF_MOVE( src._atts ) )
    , _pparent( src._pparent ) 
    , _pnext( src._pnext )     
    , _pprev( src._pprev )     
    , m_children( GUCEF_MOVE( src.m_children ) )
    , m_associatedData( src.m_associatedData )
{GUCEF_TRACE;

    // reset source to default constructor values
    src.m_nodeType = GUCEF_DATATYPE_OBJECT;
    src._pparent = GUCEF_NULL;
    src._pnext = GUCEF_NULL;
    src._pprev = GUCEF_NULL;    
    src.m_associatedData = GUCEF_NULL;
}

#endif /* GUCEF_RVALUE_REFERENCES_SUPPORTED ? */

/*-------------------------------------------------------------------------*/
        
CDataNode::~CDataNode()
{GUCEF_TRACE;
        
    GUCEF_DELETE m_associatedData;
    m_associatedData = GUCEF_NULL; 
    
    Clear();
    Detach();    
}

/*-------------------------------------------------------------------------*/

void
CDataNode::Clear( void )
{GUCEF_TRACE;

    m_nodeType = GUCEF_DATATYPE_OBJECT;
    _name.Clear();
    m_value.Clear();
    ClearAttributes();

    DelSubTree();    
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::IsEmpty( void ) const
{GUCEF_TRACE;

    return ( m_nodeType == GUCEF_DATATYPE_OBJECT || m_nodeType == GUCEF_DATATYPE_UNKNOWN ) &&
           _name.IsNULLOrEmpty() &&
           m_value.IsNULLOrEmpty() &&
           _atts.empty() &&
           _pparent == GUCEF_NULL &&
           m_children.empty() &&
           _pnext == GUCEF_NULL &&
           _pprev == GUCEF_NULL &&
           m_associatedData == GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void       
CDataNode::Detach( void )
{GUCEF_TRACE;

    // Detach from parent, if any
    // this can be a very expensive operation of there are a lot of children
    if ( GUCEF_NULL != _pparent )
    {
        _pparent->DetachChild( this );                        
    }

    // Detach from siblings, if any, and repair the chain
    if ( GUCEF_NULL != _pnext )
    {
        _pnext->_pprev = _pprev;
    }
    if ( GUCEF_NULL != _pprev )
    {
        _pprev->_pnext = _pnext;
    } 

    _pnext = GUCEF_NULL;
    _pprev = GUCEF_NULL;
    _pparent = GUCEF_NULL;     
}

/*-------------------------------------------------------------------------*/

void
CDataNode::DetachChild( CDataNode* child )
{GUCEF_TRACE;       

    if ( GUCEF_NULL == child ) 
        return;
    m_children.remove( child );
}

/*-------------------------------------------------------------------------*/

CDataNode& 
CDataNode::operator=( const CDataNode& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        Copy( src );
    }               
    return *this;
}

/*-------------------------------------------------------------------------*/

void 
CDataNode::Copy( const CDataNode& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        Clear();
        
        m_nodeType = src.m_nodeType;
        _name = src._name;
        m_value = src.m_value;
        _atts = src._atts;

        SetAssociatedData( src.m_associatedData );

        CopySubTree( src );    
    }
}

/*-------------------------------------------------------------------------*/
        
void 
CDataNode::SetName( const CString& name )
{GUCEF_TRACE;

    _name = name;
}

/*-------------------------------------------------------------------------*/
        
const CString&
CDataNode::GetName( void ) const
{GUCEF_TRACE;

    return _name;
}

/*-------------------------------------------------------------------------*/

void
CDataNode::SetValue( const CVariant& value )
{GUCEF_TRACE;

    m_value = value;
}

/*-------------------------------------------------------------------------*/

void
CDataNode::SetValue( const TVariantData& value )
{GUCEF_TRACE;

    m_value = value;
}

/*-------------------------------------------------------------------------*/

const CVariant& 
CDataNode::GetValue( void ) const
{
    return m_value;
}

/*-------------------------------------------------------------------------*/

CVariant& 
CDataNode::GetValue( void )
{
    return m_value;
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::HasValue( void ) const
{GUCEF_TRACE;
    
    return m_value.IsInitialized();
}

/*-------------------------------------------------------------------------*/

int 
CDataNode::GetValueType( void ) const
{GUCEF_TRACE;

    return (int) m_value.GetTypeId();
}

/*-------------------------------------------------------------------------*/

void
CDataNode::SetNodeType( Int32 nodeType )
{GUCEF_TRACE;

    m_nodeType = nodeType;
}

/*-------------------------------------------------------------------------*/

Int32 
CDataNode::GetNodeType( void ) const
{GUCEF_TRACE;

    return m_nodeType;
}

/*-------------------------------------------------------------------------*/
        
bool 
CDataNode::IsAttribute( const CString& name ) const
{GUCEF_TRACE;

    return _atts.end() != _atts.find( name );
}

/*-------------------------------------------------------------------------*/
        
const CDataNode::TKeyValuePair* 
CDataNode::operator[]( UInt32 index ) const
{GUCEF_TRACE;
        
    UInt32 n = 0;
    TAttributeMap::const_iterator i = _atts.begin();
    while ( i != _atts.end() )
    {
        if ( n == index )
        {
            return const_cast< const CDataNode::TKeyValuePair* >( &(*i) );
        }
        ++n;        
        ++i;
    }
    return 0;        
}

/*-------------------------------------------------------------------------*/
        
CDataNode::TKeyValuePair* 
CDataNode::operator[]( UInt32 index )
{GUCEF_TRACE;

    UInt32 n = 0;
    TAttributeMap::iterator i = _atts.begin();
    while ( i != _atts.end() )
    {
        if ( n == index )
        {
            return &(*i);
        }
        ++n;        
        ++i;
    }
    return 0;  
}

/*-------------------------------------------------------------------------*/
        
const CDataNode::TKeyValuePair* 
CDataNode::operator[]( const CString& name ) const
{GUCEF_TRACE;

    TAttributeMap::const_iterator i = _atts.find( name );
    if ( i != _atts.end() )
    {
        return &(*i);
    }
    return 0;
}

/*-------------------------------------------------------------------------*/
        
CDataNode::TKeyValuePair* 
CDataNode::operator[]( const CString& name )
{GUCEF_TRACE;

    TAttributeMap::iterator i = _atts.find( name );
    if ( i != _atts.end() )
    {
        return &(*i);
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

void
CDataNode::ClearAttributes( void )
{GUCEF_TRACE;

    _atts.clear();
}

/*-------------------------------------------------------------------------*/
        
const CDataNode::TKeyValuePair* 
CDataNode::GetAttribute( UInt32 index ) const
{
    return this->operator[]( index );      
}

/*-------------------------------------------------------------------------*/
        
CDataNode::TKeyValuePair* 
CDataNode::GetAttribute( UInt32 index )
{GUCEF_TRACE;

    return this->operator[]( index ); 
}

/*-------------------------------------------------------------------------*/
        
const CDataNode::TKeyValuePair* 
CDataNode::GetAttribute( const CString& name ) const
{GUCEF_TRACE;

    return this->operator[]( name );
}

/*-------------------------------------------------------------------------*/

const CVariant &
CDataNode::GetAttributeValue( const CString& name ) const
{GUCEF_TRACE;

    return GetAttributeValue( name, CVariant::Empty, true );
}

/*-------------------------------------------------------------------------*/

const CVariant&
CDataNode::GetAttributeValue( const CString& name          , 
                              const CVariant& defaultValue ,
                              const bool caseSensitive     ) const
{GUCEF_TRACE;

    if ( caseSensitive )
    {
        TAttributeMap::const_iterator i = _atts.find( name );
        if ( i != _atts.end() )
        {
            return (*i).second;
        }
        return defaultValue;
    }
    else
    {
        TAttributeMap::const_iterator i = _atts.begin();
        while ( i != _atts.end() )
        {
            if ( (*i).first.Equals( name, caseSensitive ) )
            {
                return (*i).second;
            }
            ++i;
        }
        return defaultValue;
    }
}

/*-------------------------------------------------------------------------*/
        
CDataNode::TKeyValuePair* 
CDataNode::GetAttribute( const CString& name )
{GUCEF_TRACE;

    TAttributeMap::iterator i = _atts.find( name );
    if ( i != _atts.end() )
    {
        return &(*i);
    }
    return 0;
}

/*-------------------------------------------------------------------------*/
   
bool
CDataNode::AddAttribute( const TKeyValuePair& att )
{GUCEF_TRACE;

    TAttributeMap::iterator i = _atts.find( att.first );
    if ( i == _atts.end() )
    {
        _atts.insert( att );
        return true;
    }
    return false;     
}

/*-------------------------------------------------------------------------*/   
        
bool 
CDataNode::AddAttribute( const CString& name  ,
                         const CString& value ,
                         int typeOfValue      )
{GUCEF_TRACE;

    TAttributeMap::iterator i = _atts.find( name );
    if ( i == _atts.end() )
    {
        return _atts[ name ].SetFromString( (UInt8) typeOfValue, value ); 
    }
    return false;                       
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::SetAttribute( const CString& name  ,
                         const CString& value ,
                         int typeOfValue      )
{GUCEF_TRACE;

    return _atts[ name ].SetFromString( (UInt8) typeOfValue, value );                       
}

/*-------------------------------------------------------------------------*/

bool
CDataNode::SetAttribute( const char* name  ,
                         const char* value ,
                         int typeOfValue   )
{GUCEF_TRACE;

    return _atts[ name ].SetFromString( (UInt8) typeOfValue, value );                     
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::SetAttribute( const CString& name ,
                         bool value          )
{GUCEF_TRACE;

    _atts[ name ] = value;
    return true;                      
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::SetAttribute( const CString& name ,
                         UInt16 value        )
{GUCEF_TRACE;

    _atts[ name ] = value;
    return true;                      
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::SetAttribute( const CString& name ,
                         Int16 value         )
{GUCEF_TRACE;

    _atts[ name ] = value;
    return true;                      
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::SetAttribute( const CString& name ,
                         UInt32 value        )
{GUCEF_TRACE;

    _atts[ name ] = value;
    return true;                      
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::SetAttribute( const CString& name ,
                         Int32 value         )
{GUCEF_TRACE;

    _atts[ name ] = value;
    return true;                      
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::SetAttribute( const CString& name ,
                         Int64 value         )
{GUCEF_TRACE;

    _atts[ name ] = value;
    return true;                      
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::SetAttribute( const CString& name ,
                         UInt64 value        )
{GUCEF_TRACE;

    _atts[ name ] = value;
    return true;                      
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::SetAttribute( const CString& name    ,
                         const CVariant& value  ,
                         bool linkWherePossible )
{GUCEF_TRACE;

    if ( linkWherePossible )
        _atts[ name ].LinkTo( value );
    else
        _atts[ name ] = value;
    return true;                      
}

/*-------------------------------------------------------------------------*/

bool
CDataNode::SetAttribute( const CString& name       , 
                         const TVariantData& value , 
                         bool linkWherePossible    )
{GUCEF_TRACE;

    if ( linkWherePossible )
        _atts[ name ].LinkTo( value );
    else
        _atts[ name ] = value;
    return true;                      
}

/*-------------------------------------------------------------------------*/

bool
CDataNode::SetAttribute( const TVariantData& name  , 
                         const TVariantData& value , 
                         bool linkWherePossible    )
{GUCEF_TRACE;

    if ( linkWherePossible )
        _atts[ CVariant( name, true ) ].LinkTo( value );
    else
        _atts[ CVariant( name, true ) ] = value;
    return true;                      
}

/*-------------------------------------------------------------------------*/

//bool 
//CDataNode::SetAttribute( const CVariant& name   ,
//                         const CVariant& value  ,
//                         bool linkWherePossible )
//{GUCEF_TRACE;
//
//    if ( linkWherePossible )
//        _atts[ name ].LinkTo( value );
//    else
//        _atts[ name ] = value;
//    return true;                      
//}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::SetAttribute( const CString& name ,
                         Float32 value       )
{GUCEF_TRACE;

    _atts[ name ] = value;
    return true;                      
}

/*-------------------------------------------------------------------------*/                         
        
UInt32 
CDataNode::GetAttCount( void ) const
{GUCEF_TRACE;

    return (UInt32) _atts.size();
}

/*-------------------------------------------------------------------------*/

void 
CDataNode::CopySubTree( const CDataNode& root )
{GUCEF_TRACE;

    if ( this != &root )
    {
        TDataNodeList::const_iterator n = root.m_children.cbegin();
        while ( n != root.m_children.cend() )
        {
            AddChild( *(*n) );
            ++n;
        }                                
    }
}

/*-------------------------------------------------------------------------*/

void 
CDataNode::DelAttribute( const CString& name )
{GUCEF_TRACE;

    _atts.erase( name );      
}

/*-------------------------------------------------------------------------*/

CDataNode* 
CDataNode::FindRoot( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == _pparent )
    {
        return (CDataNode*) this;
    }        
    return _pparent->FindRoot();
}

/*-------------------------------------------------------------------------*/

CDataNode* 
CDataNode::FindChild( const CString& name, bool caseSensitive ) const
{GUCEF_TRACE;
        
    TDataNodeList::const_iterator i = m_children.cbegin();
    while ( i != m_children.end() )
    {
        if ( (*i)->_name.Equals( name, caseSensitive ) )
            return (*i);
        ++i;
    }
    return nullptr;
}

/*-------------------------------------------------------------------------*/

CDataNode::TConstDataNodeSet
CDataNode::FindChildrenOfType( const CString& name      ,
                               const bool recursive     ,
                               const bool caseSensitive ) const
{GUCEF_TRACE;

    TConstDataNodeSet children;
    TDataNodeList::const_iterator m = m_children.cbegin();
    while ( m != m_children.cend() )
    {
        const CDataNode* child = (*m);
        if ( child->_name.Equals( name, caseSensitive ) )
        {
            children.insert( child );            
        }
        if ( recursive )
        {
            
            TConstDataNodeSet subSet( child->FindChildrenOfType( name, recursive, caseSensitive ) );
            TConstDataNodeSet::const_iterator i = subSet.begin();
            while ( i != subSet.end() )
            {
                children.insert( (*i) );
                ++i;
            }
        }        
        ++m;
    }
    return children;
}

/*-------------------------------------------------------------------------*/

CDataNode::TConstDataNodeSet
CDataNode::FindNodesOfType( const CString& name  ,
                            const bool recursive ) const
{GUCEF_TRACE;

    TConstDataNodeSet children = FindChildrenOfType( name, recursive );
    if ( _name == name )
    {
        children.insert( this );            
    }
    return children;
}

/*-------------------------------------------------------------------------*/

CDataNode::TDataNodeSet
CDataNode::FindNodesOfType( const CString& name  ,
                            const bool recursive )
{GUCEF_TRACE;

    TDataNodeSet children = FindChildrenOfType( name, recursive );
    if ( _name == name )
    {
        children.insert( this );            
    }
    return children;
}

/*-------------------------------------------------------------------------*/

CDataNode::TDataNodeSet
CDataNode::FindChildrenOfType( const CString& name      ,
                               const bool recursive     ,
                               const bool caseSensitive )
{GUCEF_TRACE;

    TDataNodeSet children;
    TDataNodeList::iterator m = m_children.begin();
    while ( m != m_children.end() )
    {
        if ( (*m)->_name.Equals( name, caseSensitive ) )
        {
            children.insert( (*m) );            
        }
        if ( recursive )
        {
            TDataNodeSet subSet( (*m)->FindChildrenOfType( name, recursive, caseSensitive ) );
            TDataNodeSet::const_iterator i = subSet.begin();
            while ( i != subSet.end() )
            {
                children.insert( (*i) );
                ++i;
            }
        }
        ++m;
    }
    return children;
}

/*-------------------------------------------------------------------------*/

CDataNode*
CDataNode::FindChild( const CString& name         ,
                      const CString& attribName   ,
                      const CVariant& attribValue )
{GUCEF_TRACE;

    TDataNodeList::iterator i = m_children.begin();
    while ( i != m_children.end() )
    {
        // Check if the name matches
        if ( (*i)->_name == name )
        {
            // See if the node has the attribute we want
            TKeyValuePair* att = (*i)->GetAttribute( attribName );
            if ( att != NULL )
            {
                // See if the node attribute has the value we want
                if ( att->second == attribValue )
                {
                    return (*i);
                }
            }
        }
        ++i;
    }  
    return nullptr;    
}

/*-------------------------------------------------------------------------*/

CDataNode*
CDataNode::FindOrAddChild( const CString& name, int typeOfNode )
{GUCEF_TRACE;

    CDataNode* child = FindChild( name );
    if ( GUCEF_NULL != child )
        return child;
    return AddChild( name, typeOfNode );    
}

/*-------------------------------------------------------------------------*/

bool
CDataNode::Compare( const CDataNode& other        ,
                    const TStringSet& excludeList ) const
{GUCEF_TRACE;

    // identical nodes should of course have identical names
    if ( _name == other._name )
    {
        const TKeyValuePair* att;
        const TKeyValuePair* att2;
        const UInt32 count = (UInt32) other._atts.size();
        
        // if one has a bigger list then the other then they cannot be the same
        if ( _atts.size() == count )
        {
            // same number of entries in the lists, we will have to compare each item
            for ( UInt32 i=0; i<count; ++i )
            {
                    att = other[ i ];
                    if ( att )
                    {
                        if ( excludeList.end() == excludeList.find( att->first ) )
                        {
                            att2 = GetAttribute( att->first );
                            if ( att2 )
                            {
                                if ( att->second != att2->second )
                                {
                                        return false;
                                }
                            }                            
                        }
                    }                                                        
            }
            
            // Everything checked out,.. nodes are identical content wise when taking
            // the exclusion list into account.
            return true;
        }
        return false;             
    }
    return false;    
}

/*-------------------------------------------------------------------------*/

const CDataNode* 
CDataNode::CompareChild( const CDataNode& other        ,
                         const TStringSet& excludeList ) const
{GUCEF_TRACE;

    const_iterator i( ConstBegin() );
    while ( i != ConstEnd() )
    {
        if ( (*i)->Compare( other       , 
                            excludeList ) )
        {
            return (*i);
        }                             
        
        ++i;
    }
    return NULL;   
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::operator==( const CDataNode& other ) const
{GUCEF_TRACE;
    
    return Compare( other, TStringSet() );
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::operator!=( const CDataNode& other ) const
{GUCEF_TRACE;
    return !( (*this) == other );
}

/*-------------------------------------------------------------------------*/

const CDataNode* 
CDataNode::FindSibling( const CString& name ) const
{GUCEF_TRACE;

    if ( name == _name )
    {
            return this;
    }
    
    CDataNode* n = _pnext;
    while ( n )
    {
            if ( n->_name == name )
            {       
                    return n;
            }
            n = n->_pnext;
    }
    n = _pprev;
    while ( n )
    {
            if ( n->_name == name )
            {       
                    return n;
            }
            n = n->_pprev;
    }        
    return NULL;        
}

/*-------------------------------------------------------------------------*/

CDataNode* 
CDataNode::FindSibling( const CString& name )
{GUCEF_TRACE;

    if ( name == _name )
    {
            return this;
    }
    
    CDataNode* n = _pnext;
    while ( n )
    {
            if ( n->_name == name )
            {       
                    return n;
            }
            n = n->_pnext;
    }
    n = _pprev;
    while ( n )
    {
            if ( n->_name == name )
            {       
                    return n;
            }
            n = n->_pprev;
    }        
    return NULL;        
}

/*-------------------------------------------------------------------------*/

CDataNode::TAttributeMap::const_iterator
CDataNode::AttributeBegin( void ) const
{GUCEF_TRACE;

    return _atts.begin();
}

/*-------------------------------------------------------------------------*/
    
CDataNode::TAttributeMap::const_iterator
CDataNode::AttributeEnd( void ) const
{GUCEF_TRACE;
    
    return _atts.end();
}

/*-------------------------------------------------------------------------*/

CString
CDataNode::GetPathToRoot( char nodeNameSeperator , 
                          bool includeThisNode   ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != _pparent )
    {
        return _pparent->GetPathToRoot( nodeNameSeperator, includeThisNode ) + nodeNameSeperator + _name;
    }
    return _name;
}

/*-------------------------------------------------------------------------*/

CDataNode* 
CDataNode::Find( const CString& name )
{GUCEF_TRACE;    

    CDataNode* n = FindSibling( name );
    if ( GUCEF_NULL != n ) 
    {
        return n;
    }                

    TDataNodeList::iterator m = m_children.begin();
    while ( m != m_children.end() )
    {
        n = (*m)->Find( name );
        if ( GUCEF_NULL != n )
        {
            return n;
        }
        ++m;
    }
    return GUCEF_NULL;       
}

/*-------------------------------------------------------------------------*/

CDataNode* 
CDataNode::FindOrAdd( const CString& name, int typeOfNode )
{GUCEF_TRACE;    

    CDataNode* n = FindSibling( name );
    if ( GUCEF_NULL != n ) 
    {
        return n;
    }                

    TDataNodeList::iterator m = m_children.begin();
    while ( m != m_children.end() )
    {
        n = (*m)->Find( name );
        if ( GUCEF_NULL != n )
        {
            return n;
        }
        ++m;
    }

    // nothing find so add
    return AddChild( name, typeOfNode );       
}

/*-------------------------------------------------------------------------*/

const CDataNode* 
CDataNode::Find( const CString& name ) const
{GUCEF_TRACE;    

    const CDataNode* n = FindSibling( name );
    if ( GUCEF_NULL != n ) 
    {
        return n;
    }                

    TDataNodeList::const_iterator m = m_children.cbegin();
    while ( m != m_children.cend() )
    {
        n = (*m)->Find( name );
        if ( GUCEF_NULL != n )
        {
            return n;
        }
        ++m;
    }
    return GUCEF_NULL;       
}

/*-------------------------------------------------------------------------*/

UInt32
CDataNode::GetNrOfDirectChildNodes( void ) const
{GUCEF_TRACE;

    return (UInt32) m_children.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CDataNode::GetNrOfChildNodes( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    TDataNodeList::const_iterator i = m_children.cbegin();
    while ( i != m_children.cend() )
    {
        count += (*i)->GetNrOfChildNodes()+1;
        ++i;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataNode::GetNrOfTreeNodes( void ) const
{GUCEF_TRACE;

    return FindRoot()->GetNrOfChildNodes()+1;    
}

/*-------------------------------------------------------------------------*/

CDataNode* 
CDataNode::Search( const CString& query     ,
                   char seperator           ,
                   bool fromcurrent         ,
                   bool treatChildAsCurrent ,
                   bool doWildcardMatching  ,
                   char wildcardChar        ) const
{GUCEF_TRACE;

    if ( fromcurrent )
    {
        CString thisname( query.SubstrToChar( seperator, true ) );
        if ( thisname == _name )
        {
            CString remnant( query.CutChars( thisname.Length()+1, true ) );
            if ( remnant.Length() > 0 )
            {            
                CString leftover;
                TDataNodeVector results = WalkTree( remnant            ,
                                                    seperator          ,
                                                    leftover           ,
                                                    doWildcardMatching ,
                                                    wildcardChar       );
                if ( 0 == leftover.Length() && !results.empty() )
                {       
                    return (*results.begin());
                }                                 
            }
            else
            {   
                // this node is the leaf node we are searching for
                return const_cast< CDataNode* >( this );
            }
        }

        if ( treatChildAsCurrent )
        {
            TDataNodeList::const_iterator i = m_children.begin();
            while ( i != m_children.end()  )
            {
                CDataNode* result = (*i)->Search( query              ,
                                                  seperator          ,
                                                  fromcurrent        ,
                                                  false              ,
                                                  doWildcardMatching ,
                                                  wildcardChar       );                
                if ( GUCEF_NULL != result )
                    return result;
                ++i;
            }
        }
        return GUCEF_NULL;                                            
    }
    else
    {
        CString leftover;
        TDataNodeVector results = WalkTree( query              ,
                                            seperator          ,
                                            leftover           ,
                                            doWildcardMatching ,
                                            wildcardChar       );
        if ( 0 == leftover.Length() && !results.empty() )
        {       
            return *results.begin();
        }
        return GUCEF_NULL;
    }                
}

/*-------------------------------------------------------------------------*/

CDataNode::TDataNodeVector 
CDataNode::SearchForAll( const CString& query     ,
                         char seperator           ,
                         bool fromcurrent         ,
                         bool treatChildAsCurrent ) const
{GUCEF_TRACE;

    if ( fromcurrent )
    {
        CString thisname( query.SubstrToChar( seperator, true ) );
        if ( thisname == _name )
        {
            CString remnant( query.CutChars( thisname.Length()+1, true ) );
            if ( remnant.Length() > 0 )
            {            
                CString leftover;
                TDataNodeVector results = WalkTree( remnant   ,
                                                    seperator ,
                                                    leftover  );
                if ( 0 == leftover.Length() )
                {       
                    return results;
                }                                 
            }
            else
            {   
                // this node is the leaf node we are searching for
                TDataNodeVector result;
                result.push_back( const_cast< CDataNode* >( this ) );
                return result;
            }
        }
        else
        if ( treatChildAsCurrent )
        {
            TDataNodeVector results;
            TDataNodeList::const_iterator i = m_children.begin();
            while ( i != m_children.end()  )
            {
                TDataNodeVector childResults = (*i)->SearchForAll( query       ,
                                                                   seperator   ,
                                                                   fromcurrent ,
                                                                   false       );                
                TDataNodeVector::iterator n = childResults.begin();
                while ( n != childResults.end() )
                {
                    results.push_back( (*n) );
                    ++n;
                }
                ++i;
            }
            return results;
        }
        return TDataNodeVector();                                            
    }
    else
    {
        CString leftover;
        TDataNodeVector results = WalkTree( query     ,
                                            seperator ,
                                            leftover  );
        if ( 0 == leftover.Length() )
        {       
            return results;
        }
        return TDataNodeVector();
    }
}

/*-------------------------------------------------------------------------*/

CDataNode::TDataNodeVector
CDataNode::WalkTree( const CString& sequence ,
                     char seperator          ,
                     CString& sleftover      ,
                     bool doWildcardMatching ,
                     char wildcardChar       ) const
{GUCEF_TRACE;

    sleftover = sequence;
        
    return WalkTreeImp( sleftover          ,
                        seperator          ,
                        doWildcardMatching ,
                        wildcardChar       );
}                     

/*-------------------------------------------------------------------------*/

CDataNode::TDataNodeVector
CDataNode::WalkTreeImp( CString& sleftover      ,
                        char seperator          ,
                        bool doWildcardMatching ,
                        char wildcardChar       ) const
{GUCEF_TRACE;
          
    if ( !m_children.empty() )
    {
        CString searchseg( sleftover.SubstrToChar( seperator, true ) );
        CString left( sleftover.CutChars( searchseg.Length()+1, true ) );
        
        TDataNodeVector resultSet;                        
        CDataNode* sn = (CDataNode*) this;       
        CString bestMatchLeftover( left );
                                                  
        CDataNode* n = m_children.front();                                                                                    
        while ( GUCEF_NULL != n )
        {
            bool nodeMatch = !doWildcardMatching ? n->_name == searchseg : searchseg.WildcardEquals( n->_name, wildcardChar, true, true );
            
            // Are we looking for more nesting or a leaf node?
            if ( 0 == left.Length() )
            {
                // We are looking for a leaf node. Check if the current child matches
                if ( nodeMatch )
                {
                    // nothing left to search for, so
                    // no point in continuing
                    resultSet.push_back( n );
                }        
            }
            else
            {            
                // check if this node could be a link in the search chain
                if ( nodeMatch )
                {
                    // search the tree for our leftover
                    CString childLeftover = left;
                    TDataNodeVector childResultSet = n->WalkTreeImp( childLeftover, seperator, doWildcardMatching, wildcardChar );
                        
                    // if what we found is better then what we found so far then
                    // substitute the current deepest nodes with the new deeper nodes.
                    if ( bestMatchLeftover.Length() > childLeftover.Length() )                  
                    {
                        // We found a better match, switch to that one
                        resultSet = childResultSet;
                        bestMatchLeftover = childLeftover;
                    }
                    else
                    if ( bestMatchLeftover.Length() == childLeftover.Length() )
                    {
                        if ( childResultSet.empty() )
                        {
                            // The child match itself is currently the best match
                            resultSet.push_back( n );
                        }
                        else
                        {
                            // We found more equally good matches deeper down the child tree
                            TDataNodeVector::iterator m = childResultSet.begin();
                            while ( m != childResultSet.end() )
                            {
                                resultSet.push_back( (*m) );
                                ++m;
                            }
                        }
                    }
                }
            }
            n = n->_pnext;
        }

        if ( !resultSet.empty() )
            sleftover = bestMatchLeftover;
        return resultSet;
    }
    
    TDataNodeVector resultSet;
    resultSet.push_back( const_cast<CDataNode*>( this ) );
    return resultSet;                                                                          
}                     

/*-------------------------------------------------------------------------*/

CDataNode* 
CDataNode::Structure( const CString& sequence ,
                      char seperator          )
{GUCEF_TRACE;

    // walk the tree
    CString buildseg;
    TDataNodeVector walkResults = WalkTree( sequence  ,
                                            seperator ,
                                            buildseg  );
                 
    // do we need to add nodes ?
    if ( 0 < buildseg.Length() )
    {
        CDataNode* parentNode = this;
        if ( !walkResults.empty() )
            parentNode = *walkResults.begin(); 
        
        CDataNode child;
        CString name;
        while ( buildseg.Length() )
        {
            name = buildseg.SubstrToChar( seperator, true );
            child.SetName( name );                        
            buildseg = buildseg.CutChars( name.Length()+1, true );                        
            parentNode = parentNode->AddChild( child );
        }
        return parentNode;
    }
    
    if ( walkResults.empty() )
        return nullptr;
    return *walkResults.begin();        
}                      

/*-------------------------------------------------------------------------*/

CDataNode::TDataNodeSet
CDataNode::StructureMultiple( const CString& sequence      ,
                              char seperator               ,
                              bool deletePreExistingLeaves ,
                              UInt32 minNrOfNodes          ,
                              UInt32 maxNrOfNodes          ,
                              int nodeType                 )
{GUCEF_TRACE;

    // Only the leaf node handling is different in this variant
    // everything up to that point can use the regular Structure()
    CString leafNodeName;
    CDataNode* leafNodesParent = this;
    Int32 lastSepCharIndex = sequence.HasChar( seperator, false );
    if ( lastSepCharIndex > 0 )
    {
        CString pathToLeaf = sequence.SubstrToIndex( (UInt32) lastSepCharIndex, true );
        leafNodesParent = Structure( pathToLeaf, seperator );
        leafNodeName = sequence.SubstrToIndex( (UInt32) lastSepCharIndex, true );
    }
    else
    {
        if ( 0 == lastSepCharIndex )
            leafNodeName = sequence.CutChars( 1, true, 0 );
        else
            leafNodeName = sequence;
    }

    // basic input sanity check
    if ( maxNrOfNodes < minNrOfNodes )
    {
        UInt32 temp = minNrOfNodes;
        minNrOfNodes = maxNrOfNodes;
        maxNrOfNodes = temp;
    }
    
    // Now make sure we have the desired number of leaf nodes of the given type
    // also apply retention if needed
    
    CDataNode::TDataNodeSet leaves = leafNodesParent->FindChildrenOfType( leafNodeName );
    if ( deletePreExistingLeaves )
    {
        CDataNode::TDataNodeSet::iterator i = leaves.begin();
        while ( i != leaves.end() )
        {
            CDataNode* leaf = (*i);
            leafNodesParent->DetachChild( leaf );
            GUCEF_DELETE leaf;
            ++i;
        }
        leaves.clear();
    }

    // If there are leaves pre-existing we mandate that the node type matches
    CDataNode::TDataNodeSet::iterator i = leaves.begin();
    while ( i != leaves.end() )
    {
        CDataNode* leaf = (*i);
        if ( nodeType != leaf->GetNodeType() )
        {
            leafNodesParent->DetachChild( leaf );
            GUCEF_DELETE leaf;
            leaves.erase( i );
            i = leaves.begin();
        }
        else
            ++i;
    }
    
    // Now we apply the desired min/max leaf node count
    
    if ( maxNrOfNodes < (UInt32) leaves.size() )
    {
        // All desired nr of nodes already exist
        // too many of them actually so we have to reduce the count
        
        UInt32 delta = ( (UInt32) leaves.size() ) - maxNrOfNodes;
        i = leaves.begin();
        while ( delta > 0 )
        {
            CDataNode* leaf = (*i);
            leafNodesParent->DetachChild( leaf );
            GUCEF_DELETE leaf;
            
            leaves.erase( i );
            --delta;
            i = leaves.begin();
        }     
    }
    else
    {
        if ( minNrOfNodes > (UInt32) leaves.size() )
        {
            // Not enough nodes exist yet, we need to add more
            
            UInt32 delta = minNrOfNodes - ( (UInt32) leaves.size() );
            for ( UInt32 n=0; n<delta; ++n )
            {
                leaves.insert( leafNodesParent->AddChild( leafNodeName, nodeType ) );
            }
        }
    }

    return leaves;
}

/*-------------------------------------------------------------------------*/

CDataNode*
CDataNode::Structure( const CString& nodeName       ,
                      const CString& attribName     ,
                      const CString& attribSequence ,
                      const char seperator          )
{GUCEF_TRACE;

    // Prepare some variables for the first loop iteration
    CDataNode* node = this;
    CString attSeqRemnant = attribSequence;
    CVariant attValue = attSeqRemnant.SubstrToChar( seperator );
    bool childExists = true;
    CDataNode* childNode = NULL;
    
    do
    {
        // First we check if we can skip the search for a child node
        // This is a minor optimization        
        if ( childExists )
        {
            // See if there already is a node of the given type
            childNode = node->FindChild( nodeName   ,
                                         attribName ,
                                         attValue   );
            if ( childNode == NULL )
            {
                childExists = false;
            }
        }
        
        // Check if we have to create a new node
        if ( childNode == NULL )
        {
            // No such node exists, we will create it
            CDataNode newChild( nodeName );
            newChild.AddAttribute( attribName, attValue );
            childNode = node->AddChild( newChild );
        }
        
        node = childNode;
        
        // Get the next segment
        attSeqRemnant = attSeqRemnant.CutChars( attValue.AsString().Length()+1, true );
        attValue = attSeqRemnant.SubstrToChar( seperator );
        
    } while ( attSeqRemnant.Length() > 0 );
    
    return childNode;
}

/*-------------------------------------------------------------------------*/

void 
CDataNode::DelSubTree( void )
{GUCEF_TRACE;

    TDataNodeList::iterator i = m_children.begin();
    while ( !m_children.empty() )
    {
        CDataNode* child = m_children.front();
        m_children.pop_front();
        
        // Don't bother fixing the parent administration if the parent is being destroyed
        // This is an optimization since this can be an expensive operation if there are a lot
        // of child nodes since the list would have to be traversed for every single child.
        // This prevents Detach() in the child destructor from reaching out to this parent again
        // which is pointless since we are deleting the entire sub tree
        child->_pparent = GUCEF_NULL;
        
        GUCEF_DELETE child;
    } 
}        

/*-------------------------------------------------------------------------*/

void 
CDataNode::DelSubTree( const CString& path, char seperator )
{GUCEF_TRACE;
    
    CDataNode* pathLeafNode = Search( path, seperator, true, true, false, '*' );
    if ( GUCEF_NULL != pathLeafNode )
    {
        pathLeafNode->DelSubTree();
    }
}

/*-------------------------------------------------------------------------*/
      
CDataNode* 
CDataNode::AddChild( const CDataNode& newnode )
{GUCEF_TRACE;

    CDataNode* n = GUCEF_NEW CDataNode( newnode );
    n->_pprev = GUCEF_NULL;
    n->_pnext = GUCEF_NULL;
    n->_pparent = this;        

    if ( !m_children.empty() )
    {
        CDataNode* last = m_children.back();
        last->_pnext = n;
        n->_pprev = last;
    }

    m_children.push_back( n );
    return n;                                  
}

/*-------------------------------------------------------------------------*/

CDataNode*
CDataNode::AddChild( const CString& nodeName, int nodeType )
{GUCEF_TRACE;

    CDataNode newNode( nodeName, nodeType );
    return AddChild( newNode );
}

/*-------------------------------------------------------------------------*/

CDataNode*
CDataNode::AddChild( void )
{GUCEF_TRACE;

    CDataNode* n = GUCEF_NEW CDataNode( GUCEF_DATATYPE_UNKNOWN );
    n->_pparent = this;

    if ( !m_children.empty() )
    {
        CDataNode* last = m_children.back();
        last->_pnext = n;
        n->_pprev = last;
    }

    m_children.push_back( n );
    return n;
}

/*-------------------------------------------------------------------------*/

CDataNode*
CDataNode::AddChildWithValue( const CString& nodeName   , 
                              const CVariant& nodeValue ,
                              int nodeType              )
{GUCEF_TRACE;

    CDataNode newNode( nodeName, nodeType == GUCEF_DATATYPE_UNKNOWN ? nodeValue.GetTypeId() : nodeType );
    newNode.SetValue( nodeValue );
    return AddChild( newNode );
}

/*-------------------------------------------------------------------------*/

CDataNode*
CDataNode::AddChildWithValue( const CString& nodeName       , 
                              const TVariantData& nodeValue ,
                              int nodeType                  )
{GUCEF_TRACE;

    CDataNode newNode( nodeName, nodeType == GUCEF_DATATYPE_UNKNOWN ? nodeValue.containedType : nodeType );
    newNode.SetValue( nodeValue );
    return AddChild( newNode );
}

/*-------------------------------------------------------------------------*/

CDataNode*
CDataNode::AddValueAsChild( const CVariant& nodeValue ,
                            bool linkIfPossible       ,
                            const CString& childName   )
{GUCEF_TRACE;

    CDataNode* newNode = AddChild( childName, nodeValue.GetTypeId() );
    if ( linkIfPossible )
        newNode->GetValue().LinkTo( nodeValue );
    else
        newNode->SetValue( nodeValue );
    return newNode;
}

/*-------------------------------------------------------------------------*/

CDataNode*
CDataNode::AddValueAsChild( const TVariantData& nodeValue ,
                            bool linkIfPossible           ,
                            const CString& childName      )
{GUCEF_TRACE;

    CDataNode* newNode = AddChild( childName, nodeValue.containedType );
    if ( linkIfPossible )
        newNode->GetValue().LinkTo( nodeValue );
    else
        newNode->SetValue( nodeValue );
    return newNode;
}

/*-------------------------------------------------------------------------*/

CDataNode*
CDataNode::AddValueAsChild( const CString& nodeValue ,
                            bool linkIfPossible      ,
                            const CString& childName )
{GUCEF_TRACE;

    CDataNode* newNode = AddChild( childName, GUCEF_DATATYPE_STRING );
    if ( linkIfPossible )
        newNode->GetValue().LinkTo( nodeValue );
    else
        newNode->SetValue( nodeValue );
    return newNode;
}

/*-------------------------------------------------------------------------*/

CDataNode*
CDataNode::AddValueAsChild( const char* nodeValue    ,
                            int valueType            ,
                            bool linkIfPossible      ,
                            const CString& childName )
{GUCEF_TRACE;

    CDataNode* newNode = AddChild( childName, valueType );
    if ( linkIfPossible )
        newNode->GetValue().LinkTo( nodeValue );
    else
        newNode->SetValue( nodeValue );
    return newNode;
}

/*-------------------------------------------------------------------------*/

const CVariant&
CDataNode::GetChildValueByName( const CString& name          ,
                                const CVariant& defaultValue ,
                                const bool caseSensitive     ) const
{GUCEF_TRACE;

    CDataNode* childNode = FindChild( name, caseSensitive );
    if ( GUCEF_NULL != childNode )
    {
        return childNode->GetValue();
    }
    return defaultValue;
}

/*-------------------------------------------------------------------------*/

const CVariant&
CDataNode::GetAttributeValueOrChildValueByName( const CString& name          , 
                                                const CVariant& defaultValue ,
                                                const bool caseSensitive     ) const
{GUCEF_TRACE;

    const CVariant& attValue = GetAttributeValue( name, CVariant::Empty, caseSensitive );
    if ( !attValue.IsInitialized() )
    {
        const CVariant& childValue = GetChildValueByName( name, CVariant::Empty, caseSensitive );
        if ( !childValue.IsInitialized() )
        {
            return defaultValue;
        }
        return childValue;
    }
    return attValue;
}

/*-------------------------------------------------------------------------*/

CDataNode::TVariantVector
CDataNode::GetAttributeValueOrChildValuesByName( const CString& name ) const
{GUCEF_TRACE;

    TVariantVector results = GetChildrenValuesByName( name );
    CVariant attValue = GetAttributeValue( name );
    if ( attValue.IsInitialized() )
    {
        results.push_back( attValue );
    }
    return results;
}

/*-------------------------------------------------------------------------*/

CDataNode::TVariantVector
CDataNode::GetChildrenValuesByName( const CString& name ) const
{GUCEF_TRACE;

    TVariantVector results;
    
    TConstDataNodeSet childNodes = FindChildrenOfType( name, false );
    TConstDataNodeSet::iterator i = childNodes.begin();
    while ( i != childNodes.end() )
    {
        switch ( (*i)->GetNodeType() )
        {
            case GUCEF_DATATYPE_OBJECT:
            case GUCEF_DATATYPE_ARRAY:
            {
                // Complex types cannot be obtained this way, skip
                break;
            }
            default:
            {
                const CVariant& childValue = (*i)->GetValue();
                if ( childValue.IsInitialized() )
                {
                    results.push_back( childValue );
                }
                break;
            }
        }
        ++i;
    }

    return results;
}

/*-------------------------------------------------------------------------*/

CDataNode::TVariantVector
CDataNode::GetChildrenValues( void ) const
{GUCEF_TRACE;

    TVariantVector results;
    
    TDataNodeList::const_iterator i = m_children.begin();
    while ( i != m_children.end() )
    {
        switch ( (*i)->GetNodeType() )
        {
            case GUCEF_DATATYPE_OBJECT:
            case GUCEF_DATATYPE_ARRAY:
            {
                // Complex types cannot be obtained this way, skip
                break;
            }
            default:
            {
                const CVariant& childValue = (*i)->GetValue();
                if ( childValue.IsInitialized() )
                {
                    results.push_back( childValue );
                }
                break;
            }
        }
        ++i;
    }

    return results;
}

/*-------------------------------------------------------------------------*/

bool
CDataNode::SetValuesOfChildByName( const CString& name                     , 
                                   const CDataNode::TVariantVector& values ,
                                   bool linkIfPossible                     ,
                                   const CString& childName                )
{GUCEF_TRACE;

    CDataNode* collectionNode = FindOrAddChild( name, GUCEF_DATATYPE_ARRAY );
    if GUCEF_PREDICT_TRUE( GUCEF_NULL != collectionNode )
    {
        CDataNode::TVariantVector::const_iterator i = values.begin();
        while ( i != values.end() )
        {
            const CVariant& var = (*i);
            collectionNode->AddValueAsChild( var, linkIfPossible );
            ++i;
        }

        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataNode::SetValuesOfChildByName( const CString& name              , 
                                   const CString::StringSet& values ,
                                   bool linkIfPossible              ,
                                   const CString& childName         )
{GUCEF_TRACE;

    CDataNode* collectionNode = FindOrAddChild( name, GUCEF_DATATYPE_ARRAY );
    if GUCEF_PREDICT_TRUE( GUCEF_NULL != collectionNode )
    {
        CString::StringSet::const_iterator i = values.begin();
        while ( i != values.end() )
        {
            const CString& str = (*i);
            collectionNode->AddValueAsChild( str, linkIfPossible );
            ++i;
        }

        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataNode::GetValuesOfChildByName( const CString& name                  , 
                                   CDataNode::TVariantVector& outValues ,
                                   bool linkIfPossible                  ) const
{GUCEF_TRACE;

    CDataNode* collectionNode = FindChild( name );
    if ( GUCEF_NULL != collectionNode )
    {
        TDataNodeList::const_iterator i = collectionNode->m_children.begin();
        while ( i != collectionNode->m_children.end() )
        {
            switch ( (*i)->GetNodeType() )
            {
                case GUCEF_DATATYPE_ARRAY:
                {
                    // Complex types cannot be obtained this way, skip
                    break;
                }
                default:
                {
                    const CVariant& childValue = (*i)->GetValue();
                    if ( childValue.IsInitialized() )
                    {
                        if ( linkIfPossible )
                        {
                            outValues.push_back( CVariant() );
                            outValues.back().LinkTo( childValue );
                        }
                        else
                        {
                            outValues.push_back( childValue );
                        }
                    }
                    break;
                }
            }
            ++i;
        }

        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataNode::GetValuesOfChildByName( const CString& name           , 
                                   CString::StringSet& outValues ) const
{GUCEF_TRACE;

    TVariantVector varValues;
    if ( GetValuesOfChildByName( name, varValues, true ) )
    {
        outValues = ToStringSet( varValues );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataNode::SetMappedValuesOfChildByName( const CString& name                  ,
                                         const CDataNode::TVariantMap& values ,
                                         bool linkIfPossible                  )
{GUCEF_TRACE;

    CDataNode* collectionNode = FindOrAddChild( name );
    if ( GUCEF_NULL != collectionNode )
    {
        CDataNode::TVariantMap::const_iterator v = values.begin();
        while ( v != values.end() )
        {
            CDataNode* childNode = collectionNode->FindOrAddChild( v->first );
            if ( GUCEF_NULL == childNode )
            {
                if ( linkIfPossible )
                {
                    CVariant& valueVar = childNode->GetValue();
                    valueVar.LinkTo( v->second );
                }
                else
                {
                    childNode->SetValue( v->second );
                }
            }

            ++v;
        }

        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::SetMappedValuesOfChildByName( const CString& name              ,
                                         const CString::StringMap& values )
{GUCEF_TRACE;

    CDataNode* collectionNode = FindOrAddChild( name );
    if ( GUCEF_NULL != collectionNode )
    {
        CString::StringMap::const_iterator v = values.begin();
        while ( v != values.end() )
        {
            CDataNode* childNode = collectionNode->FindOrAddChild( v->first );
            if ( GUCEF_NULL == childNode )
            {
                childNode->SetValue( v->second );
            }

            ++v;
        }

        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataNode::GetMappedValuesOfChildByName( const CString& name               ,
                                         CDataNode::TVariantMap& outValues ,
                                         bool linkIfPossible               ) const
{GUCEF_TRACE;

    CDataNode* collectionNode = FindChild( name );
    if ( GUCEF_NULL != collectionNode )
    {
        UInt64 childIndex = 1;
        TDataNodeList::const_iterator i = collectionNode->m_children.begin();
        while ( i != collectionNode->m_children.end() )
        {
            switch ( (*i)->GetNodeType() )
            {
                case GUCEF_DATATYPE_SET:
                case GUCEF_DATATYPE_ARRAY:
                {
                    // Complex types cannot be obtained this way, skip
                    break;
                }
                default:
                {
                    const CString& childName = (*i)->GetName();
                    const CVariant& childValue = (*i)->GetValue();
                    if ( childValue.IsInitialized() )
                    {
                        if ( childName.IsNULLOrEmpty() )
                        {
                            if ( linkIfPossible )
                            {
                                CVariant& valueVar = outValues[ childIndex ];
                                valueVar.LinkTo( childValue );
                            }
                            else
                            {
                                outValues[ childIndex ] = childValue;
                            }
                        }
                        else
                        {                        
                            if ( linkIfPossible )
                            {
                                CVariant& valueVar = outValues[ childName ];
                                valueVar.LinkTo( childValue );
                            }
                            else
                            {
                                outValues[ childName ] = childValue;
                            }
                        }
                    }
                    break;
                }
            }

            ++childIndex;
            ++i;
        }

        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::GetMappedValuesOfChildByName( const CString& name           ,
                                        CString::StringMap& outValues ) const
{GUCEF_TRACE;

    TVariantMap varValues;
    if ( GetMappedValuesOfChildByName( name, varValues, true ) )
    {
        outValues = ToStringMap( varValues );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void 
CDataNode::Delete( void )
{GUCEF_TRACE;

    Detach();
    GUCEF_DELETE this;             
}

/*-------------------------------------------------------------------------*/

CICloneable* 
CDataNode::GetAssociatedData( void ) const
{GUCEF_TRACE;

    return m_associatedData;
}

/*-------------------------------------------------------------------------*/

void 
CDataNode::SetAssociatedData( CICloneable* associatedData )
{GUCEF_TRACE;
    
    GUCEF_DELETE m_associatedData;
    m_associatedData = GUCEF_NULL;
    
    if ( GUCEF_NULL != associatedData )    
        m_associatedData = associatedData->Clone();
}

/*-------------------------------------------------------------------------*/
        
bool 
CDataNode::DelChild( const CString& name )
{GUCEF_TRACE;

    CDataNode* n = FindChild( name );
    if ( n )
    {       
        GUCEF_DELETE n;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataNode::HasChildren( void ) const
{GUCEF_TRACE;

    return !m_children.empty();
}

/*-------------------------------------------------------------------------*/

CDataNode* 
CDataNode::GetParent( void ) const
{GUCEF_TRACE;

    return _pparent;
}

/*-------------------------------------------------------------------------*/
        
CDataNode* 
CDataNode::GetFirstChild( void ) const
{GUCEF_TRACE;

    if ( !m_children.empty() )
        return m_children.front();
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CDataNode* 
CDataNode::GetLastChild( void ) const
{GUCEF_TRACE;

    if ( !m_children.empty() )
        return m_children.back();
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/
        
CDataNode* 
CDataNode::GetNext( void ) const
{GUCEF_TRACE;

    return _pnext;
}

/*-------------------------------------------------------------------------*/
        
CDataNode* 
CDataNode::GetPrevious( void ) const
{GUCEF_TRACE;

    return _pprev;
}

/*-------------------------------------------------------------------------*/

CDataNode* 
CDataNode::GetChildAtIndex( UInt32 index ) const
{GUCEF_TRACE;

    if ( !m_children.empty() )
    {
        CDataNode* firstChild = GetFirstChild();
        CDataNode* child = firstChild;

        UInt32 i=0;
        while ( firstChild != child && i < index )
        {
            child = child->_pnext;
            ++i;
        }
        return child;

    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CDataNode::iterator 
CDataNode::Begin( void )
{GUCEF_TRACE;

    iterator i( this );
    return i;
}

/*-------------------------------------------------------------------------*/

CDataNode::iterator 
CDataNode::End( void )
{GUCEF_TRACE;

    iterator i( this );
    i._pos = NULL;
    i._atend = true;      
    return i;
}

/*-------------------------------------------------------------------------*/
        
CDataNode::const_iterator 
CDataNode::Begin( void ) const
{
        GUCEF_BEGIN;
        const_iterator i( this );
        GUCEF_END;
        return i;
}

/*-------------------------------------------------------------------------*/

CDataNode::const_iterator 
CDataNode::End( void ) const
{
        GUCEF_BEGIN;
        const_iterator i( this );
        i._pos = NULL;
        i._atend = true;
        i._atstart = false;
        GUCEF_END;       
        return i;
}

/*-------------------------------------------------------------------------*/

CDataNode::const_iterator 
CDataNode::ConstBegin( void ) const
{                
        GUCEF_BEGIN;
        const_iterator i( this );
        GUCEF_END;        
        return i;
}

/*-------------------------------------------------------------------------*/

CDataNode::const_iterator 
CDataNode::ConstEnd( void ) const
{
        GUCEF_BEGIN;
        const_iterator i( this );
        i._pos = NULL;
        i._atend = true;
        i._atstart = false;
        GUCEF_END;        
        return i;
}

/*-------------------------------------------------------------------------*/

CDataNode::const_iterator::const_iterator( const CDataNode* parent )
        : _pfchild( NULL )  ,
          _pos( NULL )      ,
          _atend( true )    ,
          _atstart( false )
{
        GUCEF_BEGIN;
        if ( parent )
        {
                if ( !parent->m_children.empty() )
                {
                        _pfchild = parent->m_children.front();
                        _pos = _pfchild;
                        _atstart = true;
                        _atend = false;
                }
        }
        GUCEF_END;
}

/*-------------------------------------------------------------------------*/

CDataNode::const_iterator::const_iterator( const const_iterator& src )
        : _pfchild( src._pfchild ) ,
          _pos( src._pos )         ,
          _atend( src._atend )     ,
          _atstart( src._atstart )
{
        GUCEF_BEGIN;
        GUCEF_END;
}

/*-------------------------------------------------------------------------*/

CDataNode::const_iterator::~const_iterator()
{
        GUCEF_BEGIN;
        GUCEF_END;
}

/*-------------------------------------------------------------------------*/

CDataNode::const_iterator& 
CDataNode::const_iterator::operator=( const const_iterator& src )
{
        GUCEF_BEGIN;
        if ( this != &src )
        {
                _pos = src._pos;
                _pfchild = src._pfchild;
                _atend = src._atend;
                _atstart = src._atstart;
                GUCEF_END;
                return *this;
        }
        GUCEF_END;
        return *this;
}

/*-------------------------------------------------------------------------*/

const CDataNode* 
CDataNode::const_iterator::operator*( void ) const
{
        GUCEF_BEGIN;
        GUCEF_END;
        return _pos;
}

/*-------------------------------------------------------------------------*/

CDataNode::const_iterator& 
CDataNode::const_iterator::operator++( void )
{
        GUCEF_BEGIN;
        if ( _pos )
        {
                if ( _pos->_pnext )
                {
                        _pos = _pos->_pnext;
                        GUCEF_END;
                        return *this;
                }
                _pos = NULL;
                _atend = true;
                _atstart = false;
                GUCEF_END;
                return *this;
        }
        GUCEF_END;
        return *this;                
}

/*-------------------------------------------------------------------------*/

CDataNode::const_iterator& 
CDataNode::const_iterator::operator--( void )
{
        GUCEF_BEGIN;
        if ( _pfchild )
        {
                if ( _pos == _pfchild )
                {
                        _pos = NULL;
                        _atstart = true;
                        _atend = false;
                        GUCEF_END;
                        return *this;
                }
                _pos = _pos->_pprev;
                GUCEF_END;
                return *this;              
        }
        GUCEF_END;                
        return *this;
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::const_iterator::operator!=( const const_iterator& src ) const
{
        GUCEF_BEGIN;
        GUCEF_END;
        return !( ( _pos == src._pos ) && 
                  ( _atend == src._atend ) &&
                  ( _atstart == src._atstart ) ); 
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::const_iterator::operator==( const const_iterator& src ) const
{
        GUCEF_BEGIN;
        GUCEF_END;
        return ( ( _pos == src._pos ) && 
                 ( _atend == src._atend ) &&
                 ( _atstart == src._atstart ) ); 
}

/*-------------------------------------------------------------------------*/

CDataNode::iterator::iterator( CDataNode* parent )
        : _pfchild( NULL )  ,
          _pos( NULL )      ,
          _atend( true )    ,
          _atstart( false )
{
        GUCEF_BEGIN;
        if ( parent )
        {
                if ( !parent->m_children.empty() )
                {
                        _pos = parent->m_children.front();
                        _pfchild = _pos;
                        _atstart = true;
                        _atend = false;                        
                }
        }
        GUCEF_END;
}

/*-------------------------------------------------------------------------*/

CDataNode::iterator::iterator( const iterator& src )
        : _pfchild( src._pfchild ) ,
          _pos( src._pos )         ,
          _atend( src._atend )     ,
          _atstart( src._atstart )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataNode::iterator::~iterator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataNode::iterator& 
CDataNode::iterator::operator=( const iterator& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        _pos = src._pos;
        _pfchild = src._pfchild;
        _atend = src._atend;
        _atstart = src._atstart;
        return *this;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CDataNode* 
CDataNode::iterator::operator*( void ) const
{GUCEF_TRACE;

    return _pos;
}

/*-------------------------------------------------------------------------*/

CDataNode::iterator& 
CDataNode::iterator::operator++( void )
{GUCEF_TRACE;

    if ( _pos )
    {
        if ( _pos->_pnext )
        {
            _pos = _pos->_pnext;
            return *this;
        }
        _pos = NULL;
        _atend = true;
        _atstart = false;
        return *this;
    }
    return *this;                
}

/*-------------------------------------------------------------------------*/

CDataNode::iterator& 
CDataNode::iterator::operator--( void )
{GUCEF_TRACE;

    if ( _pfchild )
    {
        if ( _pos == _pfchild )
        {
            _pos = NULL;
            _atstart = true;
            _atend = false;
            return *this;
        }
        _pos = _pos->_pprev;
        return *this;              
    }             
    return *this;
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::iterator::operator!=( const iterator& src ) const
{GUCEF_TRACE;

    return !( ( _pos == src._pos ) && 
              ( _atend == src._atend ) &&
              ( _atstart == src._atstart ) ); 
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::iterator::operator==( const iterator& src ) const
{GUCEF_TRACE;

    return ( ( _pos == src._pos ) && 
             ( _atend == src._atend ) &&
             ( _atstart == src._atstart ) ); 
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::GetEnumerator( CEnumerator& enumerator )
{GUCEF_TRACE;

    Int64 index = -1;
    if ( GUCEF_DATATYPE_UNKNOWN != m_value.GetTypeId() )
        index = 0;
    else
    if ( !_atts.empty() || !m_children.empty() )
        index = 1;

    enumerator = CEnumerator( this, CVariant( index ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::GetEnumerator( CConstEnumerator& enumerator ) const
{GUCEF_TRACE;

    Int64 index = -1;
    if ( GUCEF_DATATYPE_UNKNOWN != m_value.GetTypeId() )
        index = 0;
    else
    if ( !_atts.empty() || !m_children.empty() )
        index = 1;

    enumerator = CConstEnumerator( this, CVariant( index ) );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt8
CDataNode::GetTypeOfCurrent( CVariant& enumeratorData ) const
{GUCEF_TRACE;

    if ( enumeratorData.GetTypeId() == GUCEF_DATATYPE_INT64 )
    {
        Int64 index = enumeratorData.AsInt64();
        if ( 0 < index )
        {
            return GUCEF_DATATYPE_UNKNOWN;
        }
        else
        if ( 0 == index )
        {
            return m_value.GetTypeId();
        }
        else
        if ( (size_t)index < _atts.size() )
        {
            const TKeyValuePair* kvPair = GetAttribute( (UInt32) index-1 );
            if ( GUCEF_NULL != kvPair )
            {
                return kvPair->second.GetTypeId();
            }
        }
        else
        if ( !m_children.empty() )
        {
            index -= (Int64) _atts.size(); 
            CDataNode* child = GetChildAtIndex( (UInt32) index );
            if ( GUCEF_NULL != child )
            {
                return (UInt8) child->GetNodeType();
            }
        }
    }
    return GUCEF_DATATYPE_UNKNOWN;    
}

/*-------------------------------------------------------------------------*/
    
bool
CDataNode::GetCurrent( CVariant& enumeratorData , 
                       CVariant& value          , 
                       bool linkIfPossible      )
{GUCEF_TRACE;

    if ( enumeratorData.GetTypeId() == GUCEF_DATATYPE_INT64 )
    {
        Int64 index = enumeratorData.AsInt64();
        if ( 0 < index )
        {
            // you are at 'end' thus no name
            return false;
        }
        if ( 0 == index )
        {
            linkIfPossible ? value.LinkTo( m_value ) : value = m_value;
            return true;
        }
        if ( (size_t)index < _atts.size() )
        {
            TKeyValuePair* kvPair = GetAttribute( (UInt32) index-1 );
            if ( GUCEF_NULL != kvPair )
            {
                linkIfPossible ? value.LinkTo( kvPair->second ) : value = kvPair->second;
                return true;
            }
        }
        // else: 
        //    you cannot get an entire child node as a variant
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataNode::GetCurrent( CVariant& enumeratorData        ,
                       const CIEnumerable** enumerable )
{GUCEF_TRACE;

    if ( enumeratorData.GetTypeId() == GUCEF_DATATYPE_INT64 && GUCEF_NULL != enumerable )
    {
        Int64 index = enumeratorData.AsInt64();
        if ( index > (Int64) _atts.size() )
        {
            index = index - (Int64) _atts.size();
            if ( index < (Int64) m_children.size() )
            {
                // the child node is also an enumerable
                CDataNode* child = GetChildAtIndex( (UInt32) index );
                if ( GUCEF_NULL != child )
                {
                    *enumerable = child;
                    return true;
                }
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataNode::GetCurrent( CVariant& enumeratorData  ,
                       CIEnumerable** enumerable )
{GUCEF_TRACE;

    if ( enumeratorData.GetTypeId() == GUCEF_DATATYPE_INT64 )
    {
        Int64 index = enumeratorData.AsInt64();
        if ( index > (Int64) _atts.size() )
        {
            index = index - (Int64) _atts.size();
            if ( index < (Int64) m_children.size() )
            {
                // the child node is also an enumerable
                CDataNode* child = GetChildAtIndex( (UInt32) index );
                if ( GUCEF_NULL != child )
                {
                    *enumerable = child;
                    return true;
                }
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataNode::GetIdOfCurrent( CVariant& enumeratorData ,
                           CVariant& value          ,  
                           bool linkIfPossible      )
{GUCEF_TRACE;

    // not supported
    return false; 
}

/*-------------------------------------------------------------------------*/

bool 
CDataNode::GetNameOfCurrent( CVariant& enumeratorData ,
                             CVariant& value          ,  
                             bool linkIfPossible      )
{GUCEF_TRACE;

    if ( enumeratorData.GetTypeId() == GUCEF_DATATYPE_INT64 )
    {
        Int64 index = enumeratorData.AsInt64();
        if ( 0 < index )
        {
            // you are at 'end' thus no name
            return false;
        }
        else
        if ( 0 == index )
        {
            // the node value uses the node name as its name
            linkIfPossible ? value.LinkTo( _name ) : value = _name;
            return true;
        }
        else
        if ( (size_t)index < _atts.size() )
        {
            TKeyValuePair* kvPair = GetAttribute( (UInt32) index-1 );
            if ( GUCEF_NULL != kvPair )
            {
                linkIfPossible ? value.LinkTo( kvPair->first ) : value = kvPair->first;
                return true;
            }
        }
        else
        {
            index = index - (Int64) _atts.size();
            if ( index < (Int64) m_children.size() )
            {
                CDataNode* child = GetChildAtIndex( (UInt32) index );
                if ( GUCEF_NULL != child )
                {
                    linkIfPossible ? value.LinkTo( child->_name ) : value = child->_name;
                    return true;
                }
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataNode::CanEnumerateForward( CVariant& enumeratorData ) const
{GUCEF_TRACE;

    if ( enumeratorData.GetTypeId() == GUCEF_DATATYPE_INT64 )
    {
        Int64 index = enumeratorData.AsInt64();
        if ( index >= 0 && (size_t) index < _atts.size() + m_children.size() )
        {
            return true;
        }
    }
    return false;    
}

/*-------------------------------------------------------------------------*/

bool
CDataNode::CanEnumerateBackward( CVariant& enumeratorData ) const
{GUCEF_TRACE;
    
    if ( enumeratorData.GetTypeId() == GUCEF_DATATYPE_INT64 )
    {
        Int64 index = enumeratorData.AsInt64();
        if ( index > 0 )
        {
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataNode::MoveNext( CVariant& enumeratorData )
{GUCEF_TRACE;

    if ( enumeratorData.GetTypeId() == GUCEF_DATATYPE_INT64 )
    {
        Int64 index = enumeratorData.AsInt64();
        if ( index >= 0 && (size_t) index < _atts.size() + m_children.size() )
        {
            enumeratorData = ++index;
            return true;
        }
    }
    return false; 
}

/*-------------------------------------------------------------------------*/
    
bool
CDataNode::MovePrev( CVariant& enumeratorData )
{GUCEF_TRACE;

    if ( enumeratorData.GetTypeId() == GUCEF_DATATYPE_INT64 )
    {
        Int64 index = enumeratorData.AsInt64();
        if ( index > 0 )
        {
            enumeratorData = --index;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataNode::IsAtEnd( CVariant& enumeratorData ) const
{GUCEF_TRACE;

    if ( enumeratorData.GetTypeId() == GUCEF_DATATYPE_INT64 )
    {
        Int64 index = enumeratorData.AsInt64();
        return index < 0;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
CDataNode::Compare( CVariant& enumeratorData            ,
                    const CVariant& otherEnumeratorData ) const
{GUCEF_TRACE;

    if ( enumeratorData.GetTypeId() == GUCEF_DATATYPE_INT64 )
    {
        Int64 index = enumeratorData.AsInt64();
        return index < 0;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
