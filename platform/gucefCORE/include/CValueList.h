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

#ifndef GUCEF_CORE_CVALUELIST_H
#define GUCEF_CORE_CVALUELIST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>
#include <vector>

#ifndef GUCEF_CORE_EXCEPTIONCLASSMACROS_H
#include "ExceptionClassMacros.h"
#define GUCEF_CORE_EXCEPTIONCLASSMACROS_H
#endif /* GUCEF_CORE_EXCEPTIONCLASSMACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"     /* often used gucef macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

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

class GUCEF_CORE_PUBLIC_CPP CValueList : public CIConfigurable          ,
                                         public CIDataNodeSerializable
{
    public:

    typedef CVariant::VariantVector                         TVariantVector;
    typedef CString::StringVector                           TStringVector;
    typedef GUCEF::map< CVariant, TVariantVector >   TValueMap;

    CValueList( void );

    CValueList( const CValueList& src );

    CValueList& operator=( const CValueList& src );

    virtual ~CValueList() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Returns the first value associated with the
     *  given key.
     *
     *  Will create a new key entry if no such key entry yet exists with an empty string value
     *  thus allowing for valuelist[ "myKey" ] = "example"
     */
    CVariant& operator[]( const CString& key );

    /**
     *  Returns the first value associated with the
     *  given key.
     *
     *  @exception EUnknownKey thrown if the given key is unknown
     */
    const CVariant& operator[]( const CString& key ) const;

    /**
     *  Sets key/value pairs based on the source valuelist
     *  Like assignment in that new keys will be added and existing key will be overwritten if matched
     *  However contrary to assignment keys that have no match will remain
     */
    void SetMultiple( const CValueList& src                     ,
                      const CString& keyPrefix = CString::Empty );

    void SetMultiple( int argc    ,
                      char** argv );

    void SetMultiple( const CString& keyandvalue                    ,
                      const char pairSeparator                      ,
                      const char kvSeperator = '='                  ,
                      const CString* optionalKeyPrefix = GUCEF_NULL );

    void SetUsingKvCombo( const CString& keyAndValue                    ,
                          const char kvSeperator = '='                  ,
                          const CString* optionalKeyPrefix = GUCEF_NULL ,
                          UInt8 valueType = GUCEF_DATATYPE_STRING       );

    void Set( const char* key       ,
              const CVariant& value );

    void Set( const CString& key    ,
              const CVariant& value );

    void Set( const CVariant& key   ,
              const CVariant& value );

    /**
     *  Returns the first value associated with the
     *  given key.
     *
     *  @exception EUnknownKey thrown if the given key is unknown
     */
    CVariant& GetValue( const CString& key );

    /**
     *  Returns the first value associated with the
     *  given key.
     *
     *  @exception EUnknownKey thrown if the given key is unknown
     */
    CVariant& GetValue( const CVariant& key );

    /**
     *  Returns the first value associated with the
     *  given key.
     *
     *  @exception EUnknownKey thrown if the given key is unknown
     */
    const CVariant& GetValue( const CString& key ) const;

    /**
     *  Returns the first value associated with the
     *  given key.
     *
     *  @exception EUnknownKey thrown if the given key is unknown
     */
    const CVariant& GetValue( const CVariant& key ) const;

    /**
     *  Attempt to assign or link the first value associated with the given key.
     *  Note that this non-const version will be potentially more performant since its able to link data vs a deep copy
     */
    bool TryGetValue( const CString& key, CVariant& outValue, bool linkIfPossible = true );

    /**
     *  Attempt to assign or link the first value associated with the given key.
     *  Note that this non-const version will be potentially more performant since its able to link data vs a deep copy
     */
    bool TryGetValue( const CVariant& key, CVariant& outValue, bool linkIfPossible = true );

    /**
     *  Attempt to assign or link the first value associated with the given key.
     *  Note that this non-const version will be potentially more performant since its able to link data vs a deep copy
     */
    bool TryGetValue( const char* key, CVariant& outValue, bool linkIfPossible = true );

    /**
     *  Attempt to assign or link the first value associated with the given key.
     */
    bool TryGetValue( const char* key, CVariant& outValue ) const;

    /**
     *  Attempt to assign or link the first value associated with the given key.
     */
    bool TryGetValue( const CString& key, CVariant& outValue ) const;

    /**
     *  Attempt to assign or link the first value associated with the given key.
     */
    bool TryGetValue( const CVariant& key, CVariant& outValue ) const;

    /**
     *  Returns the first value associated with the
     *  given key. This GetValue version differs from the others in
     *  that no exception will be thrown if the key does not exist
     *  instead it will return an empty variant.
     */
    CVariant GetValueAlways( const CVariant& key      ,
                             const char* defaultValue ) const;

    /**
     *  Returns the first value associated with the
     *  given key. This GetValue version differs from the others in
     *  that no exception will be thrown if the key does not exist
     *  instead it will return an empty variant.
     */
    CVariant GetValueAlways( const CString& key       ,
                             const char* defaultValue ) const;

    /**
     *  Returns the first value associated with the
     *  given key. This GetValue version differs from the others in
     *  that no exception will be thrown if the key does not exist
     *  instead it will return an empty variant.
     */
    CVariant GetValueAlways( const char* key          ,
                             const char* defaultValue ) const;

    /**
     *  Returns the first value associated with the
     *  given key. This GetValue version differs from the others in
     *  that no exception will be thrown if the key does not exist
     *  instead it will return an empty variant.
     */
    CVariant GetValueAlways( const char* key              ,
                             const CVariant& defaultValue ) const;

    /**
     *  Returns the first value associated with the
     *  given key. This GetValue version differs from the others in
     *  that no exception will be thrown if the key does not exist
     *  instead it will return an empty variant.
     */
    CVariant GetValueAlways( const char* key             ,
                             const CString& defaultValue ) const;

    /**
     *  Returns the first value associated with the
     *  given key. This GetValue version differs from the others in
     *  that no exception will be thrown if the key does not exist
     *  instead it will return an empty variant.
     */
    CVariant GetValueAlways( const CString& key           ,
                             const CVariant& defaultValue ) const;

    /**
     *  Returns the first value associated with the
     *  given key. This GetValue version differs from the others in
     *  that no exception will be thrown if the key does not exist
     *  instead it will return an empty variant.
     */
    CVariant GetValueAlways( const CString& key          ,
                             const CString& defaultValue ) const;

    /**
     *  Returns the first value associated with the
     *  given key. This GetValue version differs from the others in
     *  that no exception will be thrown if the key does not exist
     *  instead it will return an empty variant.
     */
    const CVariant& GetValueAlways( const CVariant& key                            ,
                                    const CVariant& defaultValue = CVariant::Empty ) const;

    /**
     *  Returns the first value associated with the
     *  given key index.
     *
     *  @exception EIndexOutOfRange thrown if the given key index is invalid
     */
    CVariant& GetValue( const UInt32 index );

    /**
     *  Returns the first value associated with the
     *  given key index.
     *
     *  @exception EIndexOutOfRange thrown if the given key index is invalid
     */
    const CVariant& GetValue( const UInt32 index ) const;

    /**
     *  Returns the value vector associated with the
     *  given key if any. Otherwise returns an empty list and will not throw an exception
     */
    TVariantVector GetValueVectorAlways( const CString& key ) const;

    /**
     *  Returns the value vector associated with the
     *  given key if any. Otherwise returns an empty list and will not throw an exception
     *  Aside from returning the multiple entries for the same key (if permitted) it will also
     *  perform sub string parsing for values per value entry
     */
    TVariantVector GetValueVectorAlways( const CVariant& key, char valueSepChar ) const;
    TVariantVector GetValueVectorAlways( const CString& key, char valueSepChar ) const;

    TVariantVector GetValueVectorAlways( const CVariant& key, char valueSepChar, const TVariantVector& defaultValues ) const;
    TVariantVector GetValueVectorAlways( const CString& key, char valueSepChar, const TVariantVector& defaultValues ) const;

    /**
     *  Returns all keys matching the key wildcard search
     *  Will not throw an exception
     */
    TStringVector GetKeysWithWildcardKeyMatch( const CString& searchStr  ,
                                               char wildCardChar = '*'   ,
                                               bool caseSensitive = true ) const;

    /**
     *  Returns the value vector associated with the
     *  given key.
     *
     *  @exception EUnknownKey thrown if the given key is unknown
     */
    TVariantVector& GetValueVector( const CString& key );

    /**
     *  Returns the value vector associated with the
     *  given key.
     *
     *  @exception EUnknownKey thrown if the given key is unknown
     */
    const TVariantVector& GetValueVector( const CString& key ) const;

    /**
     *  Returns a string converted copy of the value vector associated with the
     *  given key.
     *
     *  @exception EUnknownKey thrown if the given key is unknown
     */
    TStringVector GetValueStringVector( const CString& key ) const;

    /**
     *  Returns the value vector associated with the
     *  given index.
     *
     *  @exception EIndexOutOfRange thrown if the given index is out of bounds
     */
    const TVariantVector& GetValueVector( const UInt32 index ) const;

    /**
     *  Attempt to assign or link all the values associated with the given key.
     *  Note that this non-const version will be potentially more performant since its able to link data vs a deep copy
     */
    bool TryGetValueVector( const CString& key, TVariantVector& outValue, bool linkIfPossible = true );

    /**
     *  Attempt to assign or link all the values associated with the given key.
     *  Note that this non-const version will be potentially more performant since its able to link data vs a deep copy
     */
    bool TryGetValueVector( const CVariant& key, TVariantVector& outValue, bool linkIfPossible = true );

    /**
     *  Attempt to assign or link all the values associated with the given key.
     *  Note that this non-const version will be potentially more performant since its able to link data vs a deep copy
     */
    bool TryGetValueVector( const char* key, TVariantVector& outValue, bool linkIfPossible = true );

    /**
     *  Attempt to assign all the values associated with the given key.
     */
    bool TryGetValueVector( const char* key, TVariantVector& outValue ) const;

    /**
     *  Attempt to assign all the values associated with the given key.
     */
    bool TryGetValueVector( const CString& key, TVariantVector& outValue ) const;

    /**
     *  Attempt to assign all the values associated with the given key.
     */
    bool TryGetValueVector( const CVariant& key, TVariantVector& outValue ) const;

    CString GetPair( const CString& key ) const;

    CString GetPair( const UInt32 index ) const;

    CString GetAllPairs( const UInt32 index, const CString& seperatorStr ) const;

    CString GetAllPairs( const CString& keyValueSeperatorStr        ,
                         const CString& kvPairSeperatorStr          ,
                         bool envelopElements = false               ,
                         const CString& envelopStr = CString::Empty ) const;

    /**
     *  Returns the key associated with the given index.
     *
     *  @exception EIndexOutOfRange thrown if the given key index is invalid
     */
    const CVariant& GetKey( const UInt32 index ) const;

    bool HasKey( const char* key ) const;

    bool HasKey( const CString& key ) const;

    bool HasKey( const CVariant& key ) const;

    bool HasKeyAndValue( const CString& key, const CString& value ) const;

    bool HasKeyAndValue( const CVariant& key, const CVariant& value ) const;

    void Delete( const char* key );

    void Delete( const CString& key );

    void Delete( const CVariant& key );

    void DeleteAll( void );

    UInt32 GetKeyCount( void ) const;

    UInt32 GetValueCount( void ) const;

    void SetAllowDuplicates( const bool allowDuplicates );

    bool GetAllowDuplicates( void ) const;

    void SetAllowMultipleValues( const bool allowMultipleValues );

    bool GetAllowMultipleValues( void ) const;

    void Clear( void );

    /**
     *      Attempts to store the given tree in the value list
     */
    virtual bool SaveConfig( CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Attempts to load data
     *
     *      @param treeroot pointer to the node that is to act as root of the data tree
     *      @return whether building the tree from the given file was successfull.
     */
    virtual bool LoadConfig( const CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     */
    virtual bool Serialize( CDataNode& domRootNode                        ,
                            const CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     *
     *  @param domRootNode Node that acts as root of the DOM data tree from which to deserialize
     *  @return whether deserializing the object data from the given DOM was successfull.
     */
    virtual bool Deserialize( const CDataNode& domRootNode                  ,
                              const CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  When loading from config data this will limit the scope
     *  of the information loaded to just the namespace given
     */
    void SetConfigNamespace( const CString& configNamespace );

    const CString& GetConfigNamespace( void ) const;

    /**
     *  When loading from config data this will limit the scope
     *  of the information loaded to just the keys that are
     *  namespaced with by having the exact 'configKeyNamespace' prefix
     */
    void SetConfigKeyNamespace( const CString& configKeyNamespace );

    const CString& GetConfigKeyNamespace( void ) const;

    TValueMap::const_iterator GetDataBeginIterator( void ) const;

    TValueMap::const_iterator GetDataEndIterator( void ) const;

    void CopySettingsFrom( const CValueList& other );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    GUCEF_DEFINE_MSGEXCEPTION( GUCEF_CORE_PUBLIC_CPP, EUnknownKey );
    GUCEF_DEFINE_MSGEXCEPTION( GUCEF_CORE_PUBLIC_CPP, EIndexOutOfRange );

    private:

    TValueMap m_list;
    bool m_allowDuplicates;
    bool m_allowMultipleValues;
    CString m_configNamespace;
    CString m_configKeyNamespace;
};

/*-------------------------------------------------------------------------*/

inline CString ToString( const CValueList& values ) { return values.GetAllPairs( "=", ",", true, "\"" ); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CVALUELIST_H */
