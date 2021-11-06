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

#ifndef GUCEF_CORE_CDATANODE_H
#define GUCEF_CORE_CDATANODE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <vector>
#include <set>
#include <map>
#include <list>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

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

/**
 *      Simple datanode class.
 *      Can be used to build a tree of data with each tree node
 *      having an optional number of attributes.
 *      Note that CDataNode objects always remain owner of all their
 *      children so do not delete any node or node attribute directly yourself.
 */
class GUCEF_CORE_PUBLIC_CPP CDataNode
{
    public:

    typedef std::vector< CString > TStringVector;
    typedef std::set< CString > TStringSet;
    typedef std::pair< const CString, CVariant > TKeyValuePair;
    typedef std::set< CDataNode* > TDataNodeSet;
    typedef std::list< CDataNode* > TDataNodeList;
    typedef std::vector< CDataNode* > TDataNodeVector;
    typedef std::set< const CDataNode* > TConstDataNodeSet;
    typedef std::map< CString, CVariant > TAttributeMap;
    typedef CVariant::VariantVector TVariantVector;
    typedef CVariant::VariantSet TVariantSet;

    CDataNode( int nodeType = GUCEF_DATATYPE_OBJECT );

    /**
     *      Constructs a datanode with the given name
     *      The constructed node has no attributes and is not part of a tree
     *
     *      @param name The name of the node
     */
    CDataNode( const CString& name                  ,
               int nodeType = GUCEF_DATATYPE_OBJECT );

    /**
     *      Constructs a datanode tree that is an excact duplicate
     *      of the given tree with src as the tree root.
     *
     *      @param src The tree that is to serve as an template
     */
    CDataNode( const CDataNode& src );

    /**
     *      Cleans up all attributes and children
     */
    ~CDataNode();

    /**
     *      Turns the given node and it's sub-tree into an replica
     *      of the given node and it's sub-tree.
     *
     *      @param src tree you wish to replicate
     *      @return the modified node
     */
    CDataNode& operator=( const CDataNode& src );

    bool operator==( const CDataNode& other ) const;

    bool operator!=( const CDataNode& other ) const;

    bool Compare( const CDataNode& other        ,
                  const TStringSet& excludeList ) const;

    /**
     *  Checks the immediate children using Compare()
     *  If no "identical" child node can be found NULL
     *  will be returned.
     */
    const CDataNode* CompareChild( const CDataNode& other        ,
                                   const TStringSet& excludeList ) const;

    /**
     *      Sets the new node name
     *
     *      @param name the new node name
     */
    void SetName( const CString& name );

    /**
     *      Gets the current node name
     *
     *      @return the current node name
     */
    const CString& GetName( void ) const;

    /**
     *      Sets the new node simplistic value
     *
     *      @param name the new node name
     */
    void SetValue( const CVariant& value );

    /**
     *      Gets the current node value
     *
     *      @return the current simplistic node value
     */
    const CVariant& GetValue( void ) const;

    bool HasValue( void ) const;

    int GetValueType( void ) const;
    
    void SetNodeType( int nodeType );
    
    int GetNodeType( void ) const;

    bool IsAttribute( const CString& name ) const;

    const TKeyValuePair* operator[]( UInt32 index ) const;

    TKeyValuePair* operator[]( UInt32 index );

    const TKeyValuePair* operator[]( const CString& name ) const;

    TKeyValuePair* operator[]( const CString& name );

    const TKeyValuePair* GetAttribute( UInt32 index ) const;

    TKeyValuePair* GetAttribute( UInt32 index );

    const TKeyValuePair* GetAttribute( const CString& name ) const;

    TKeyValuePair* GetAttribute( const CString& name );

    const CVariant& GetAttributeValue( const CString& name ) const;
    
    const CVariant& GetAttributeValue( const CString& name, const CVariant& defaultValue ) const;

    bool SetAttribute( const CString& name                     ,
                       const CString& value                    ,
                       int typeOfValue = GUCEF_DATATYPE_STRING );

    bool SetAttribute( const char* name                        ,
                       const char* value                       ,
                       int typeOfValue = GUCEF_DATATYPE_STRING );

    bool SetAttribute( const CString& name, bool value );
    bool SetAttribute( const CString& name, UInt32 value );
    bool SetAttribute( const CString& name, Int32 value );
    bool SetAttribute( const CString& name, Int64 value );
    bool SetAttribute( const CString& name, const CVariant& value );

    void DelAttribute( const CString& name );

    void ClearAttributes( void );

    void Clear( void );

    UInt32 GetAttCount( void ) const;

    /**
     *  Attempts to locate a named value which can come from either a attribute on the current node
     *  or from a value field of a child node with the given name and return its value. 
     *  If value can be found using either method then an empty string is returned.
     *
     *  Note that attributes have priority over child value fields. 
     *  Also note that if multiple children exist with the given name the first located one's value will be used.
     *  Only use this function if you expect the key/value combo to have a uniqueness constraint.
     *
     *  @param name the name of the attribute or child node
     *  @return the value located using the given name
     */
    const CVariant& GetAttributeValueOrChildValueByName( const CString& name, const CVariant& defaultValue = CVariant::Empty ) const;

    TVariantVector GetAttributeValueOrChildValuesByName( const CString& name ) const;

    /**
     *  Attempts to locate a child node with the given name
     *  and return its value. If no such child node exists then
     *  an empty string is returned. Also note that if multiple children exist
     *  with the given name the first located one's value will be used.
     *  In such a case you may wish to use GetChildValuesByName() instead.
     *
     *  @param name the name of the child node
     *  @return the value of the first child located with the given name
     */
    const CVariant& GetChildValueByName( const CString& name ) const;

    /**
     *  Attempts to locate a child node with the given name
     *  and return its value. If no such child node exists then
     *  an empty string is returned. Also note that if multiple children exist
     *  with the given name the first located one's value will be used.
     *  In such a case you may wish to use GetChildValuesByName() instead.
     *
     *  @param name the name of the child node
     *  @return the value of the first child located with the given name
     */
    TVariantVector GetChildrenValuesByName( const CString& name ) const;

    /**
     *  Retrieves the "value: property of each child node if set and aggregates them
     *
     *  @return the values of the child nodes
     */
    TVariantVector GetChildrenValues( void ) const;

    CDataNode* FindRoot( void ) const;

    CDataNode* FindChild( const CString& name ) const;

    CDataNode* FindOrAddChild( const CString& name );

    TConstDataNodeSet FindChildrenOfType( const CString& name          ,
                                          const bool recursive = false ) const;

    TDataNodeSet FindChildrenOfType( const CString& name          ,
                                     const bool recursive = false );

    TDataNodeSet FindNodesOfType( const CString& name  ,
                                  const bool recursive );

    CDataNode* FindChild( const CString& name         ,
                          const CString& attribName   ,
                          const CVariant& attribValue );

    CDataNode* FindSibling( const CString& name );

    const CDataNode* FindSibling( const CString& name ) const;

    /**
     *      Searches the entire tree starting at the current level
     *      for an node with the given name
     */
    const CDataNode* Find( const CString& name ) const;

    /**
     *      Searches the entire tree starting at the current level
     *      for an node with the given name
     */
    CDataNode* Find( const CString& name );

    /**
     *      If the given node structure does not exists starting
     *      as a child node of the current node then the nodes
     *      will be created. This ensures that you have the
     *      requested tree structure. A pointer to the last
     *      mentioned structure node is returned.
     *
     *      @param sequence the node structure sequence you wish to create
     *      @param seperator the char you used to seperate node names
     *      @return last node in the structure
     */
    CDataNode* Structure( const CString& sequence ,
                          char seperator          );

    /**
     *  If the given node structure does not exists starting
     *  as a child node of the current node then the nodes
     *  will be created. This ensures that you have the
     *  requested tree structure. A collection of pointers to
     *  the specified number of leaf nodes is returned.
     *
     *  @param sequence the node structure sequence you wish to create
     *  @param seperator the char you used to seperate node names
     *  @param deletePreExistingLeaves whether to delete pre-existing leaf nodes and generate new ones exclusively
     *  @param minNrOfNodes the minimum nr of leaf nodes that should exist following the given structure
     *  @param maxNrOfNodes the maximum nr of leaf nodes that should exist following the given structure
     *  @return all leaf nodes created in the structure
     */
    TDataNodeSet StructureMultiple( const CString& sequence              ,
                                    char seperator                       ,
                                    bool deletePreExistingLeaves         ,
                                    UInt32 minNrOfNodes                  ,
                                    UInt32 maxNrOfNodes                  ,
                                    int nodeType = GUCEF_DATATYPE_OBJECT );

    /**
     *      Builds a structure if where each node is named
     *      according to 'nodeName' and where each of those nodes is given
     *      an attribute with 'attribName' using a section from 'attribSequence'
     *      as the value for the attribute with 'seperator' used to separate
     *      the segments
     *
     *      @param nodeName name used for all the nodes in the structure
     *      @param attribName name of the attribute you want to use to store the attrib value
     *      @param attribSequence the attribute values you wish to set
     *      @param seperator the char you used to separate attribute values
     *      @return last node in the structure
     */
    CDataNode* Structure( const CString& nodeName       ,
                          const CString& attribName     ,
                          const CString& attribSequence ,
                          const char seperator          );

    /**
     *      Searches for the structure provided in the query.
     *      If the structure is found a pointer to the last node
     *      in the structure will be provided.
     *
     *      @param sequence the node structure sequence you wish to search for
     *      @param seperator the char you used to seperate node names
     *      @param fromcurrent wheter the requested structure must start at the current node.
     *      @return last node in the search structure
     */
    CDataNode* Search( const CString& query             ,
                       char seperator                   ,
                       bool fromCurrent                 ,
                       bool treatChildAsCurrent = false ,
                       bool doWildcardMatching = false  ,
                       char wildcardChar = '*'          ) const;

    /**
     *      Searches for the structure provided in the query.
     *      If the structure is found an entry pointer to the last node is added to the result list
     *
     *      @param sequence the node structure sequence you wish to search for
     *      @param seperator the char you used to seperate node names
     *      @param fromcurrent wheter the requested structure must start at the current node.
     *      @return last node in the search structure
     */
    TDataNodeVector SearchForAll( const CString& query             ,
                                  char seperator                   ,
                                  bool fromCurrent                 ,
                                  bool treatChildAsCurrent = false ) const;

    /**
     *  'Walks' the data node tree to find the sequence
     *  of nodes given. The algorith will attempt to get as 'deep'
     *  as possible using the specified node sequence.
     *  If no nodes where found that have an name as specified in the sequence
     *  at the correct location then that part of the sequence is considdered to
     *  be a leftover. Pointers to the 'deepest' nodes are returned.
     *
     *  @param sequence the sequence of node names you are looking for, each name seperated with a seperator char
     *  @param seperator char that is used to seperate the different node names
     *  @param sleftover outputvar: the section of the sequence that could not be resolved.
     *  @return pointers to the deepest nodes found. Equally good matches.
     */
    TDataNodeVector WalkTree( const CString& sequence         ,
                              char seperator                  ,
                              CString& sleftover              ,
                              bool doWildcardMatching = false ,
                              char wildcardChar = '*'         ) const;

    /**
     *  Returns wheter the current node has any children
     *
     *  @return whether the current node has any children
     */
    bool HasChildren( void ) const;

    /**
     *  Adds an child node to the given node using the
     *  given newnode as a template.
     *
     *  @param newnode node that is to be used as an template
     *  @return pointer to the new child.
     */
    CDataNode* AddChild( const CDataNode& newnode );

    /**
     *  Adds an child node to the given node using the
     *  given name as the child node's name
     *
     *  @param nodeName name of the new child node
     *  @return pointer to the new child.
     */
    CDataNode* AddChild( const CString& nodeName              , 
                         int nodeType = GUCEF_DATATYPE_OBJECT );

    CDataNode* AddChildWithValue( const CString& nodeName   , 
                                  const CVariant& nodeValue );
    
    bool DelChild( const CString& name );

    void Copy( const CDataNode& root );

    void CopySubTree( const CDataNode& root );

    /**
     *  Deletes the entire sub-tree from the current node.
     *  So in essense the child nodes and their children ect.
     *  are deleted.
     */
    void DelSubTree( void );

    /**
     *  Deletes the entire sub-tree for after the given path
     *  with the path root equaling the node where this member function is invoked.
     */
    void DelSubTree( const CString& path, char seperator );

    /**
     *  Counts all the child nodes for the node the 
     *  request is made on.
     */
    UInt32 GetNrOfChildNodes( void ) const;
    
    /**
     *  Counts all the nodes in the data tree
     *  This includes nodes above and below the node the
     *  request is made on.
     */
    UInt32 GetNrOfTreeNodes( void ) const;
    
    /**
     *  Creates a string representation of the 'path' to the root node
     *  consisting of all the node names encountered up to the root node
     *  seperared by 'nodeNameSeperator'
     */
    CString GetPathToRoot( char nodeNameSeperator ,
                           bool includeThisNode   ) const;

    /**
     *  Attempts to delete this node from the tree.
     */
    void Delete( void );

    /**
     *  Provides access to the associated data if any
     *  Note that the associated is owned as a clone by the data node 
     */
    CICloneable* GetAssociatedData( void ) const;

    /**
     *  Sets the associated data if any
     *  Note that the associated given is clones and the clone is subsequently owned by the data node 
     */
    void SetAssociatedData( CICloneable* associatedData );

    CDataNode* GetParent( void ) const;

    CDataNode* GetFirstChild( void ) const;

    CDataNode* GetLastChild( void ) const;

    CDataNode* GetNext( void ) const;

    CDataNode* GetPrevious( void ) const;

    class GUCEF_CORE_PUBLIC_CPP iterator
    {
        public:

        iterator( CDataNode* parent );

        iterator( const iterator& src );

        ~iterator();

        iterator& operator=( const iterator& src );

        CDataNode* operator*( void ) const;

        iterator& operator++( void );

        iterator& operator--( void );

        bool operator!=( const iterator& src ) const;

        bool operator==( const iterator& src ) const;

        private:
        friend class CDataNode;

        const CDataNode* _pfchild;   /**< first child node */
        CDataNode* _pos;             /**< current child node */
        bool _atend;                 /**< has the iterator reached the right end pos */
        bool _atstart;               /**< has the iterator reached the left end pos */
    };

    class GUCEF_CORE_PUBLIC_CPP const_iterator
    {
        public:

        const_iterator( const CDataNode* parent );

        const_iterator( const const_iterator& src );

        ~const_iterator();

        const_iterator& operator=( const const_iterator& src );

        const CDataNode* operator*( void ) const;

        const_iterator& operator++( void );

        const_iterator& operator--( void );

        bool operator!=( const const_iterator& src ) const;

        bool operator==( const const_iterator& src ) const;

        private:
        friend class CDataNode;

        const CDataNode* _pfchild;   /**< first child node */
        const CDataNode* _pos;       /**< current child node */
        bool _atend;                 /**< has the iterator reached the right end pos */
        bool _atstart;               /**< has the iterator reached the left end pos */
    };

    iterator Begin( void );

    iterator End( void );

    const_iterator Begin( void ) const;

    const_iterator End( void ) const;

    const_iterator ConstBegin( void ) const;

    const_iterator ConstEnd( void ) const;
    
    TAttributeMap::const_iterator AttributeBegin( void ) const;
    
    TAttributeMap::const_iterator AttributeEnd( void ) const;

    private:
    friend class iterator;
    friend class const_iterator;

    bool AddAttribute( const TKeyValuePair& att );

    bool AddAttribute( const CString& name                     ,
                       const CString& value                    ,
                       int typeOfValue = GUCEF_DATATYPE_STRING );

    TDataNodeVector WalkTreeImp( CString& sleftover              ,
                                 char seperator                  ,
                                 bool doWildcardMatching = false ,
                                 char wildcardChar = '*'         ) const;

    void Detach( void );      /**< detaches the node from the tree */
    void DetachChild( CDataNode* child ); /**< detaches the given child node */

    
    int m_nodeType;       /**< metadata encoding the type of the node */
    CString _name;        /**< name of the node */
    CVariant m_value;     /**< simplistic value field of the node */
    TAttributeMap _atts;  /**< list of node attributes */
    CDataNode* _pparent;  /**< parent node */
    TDataNodeList m_children; /**< child nodes */
    CDataNode* _pnext;    /**< next sibling node */
    CDataNode* _pprev;    /**< previous sibling node */
    CICloneable* m_associatedData; /**< externally managed data */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATANODE_H ? */
