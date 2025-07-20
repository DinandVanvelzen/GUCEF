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

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h" /* macros that are GUCEF specific and generic macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "ETypes.h"           /* simple types used */
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef GUCEF_CORE_CITYPENAMED_H
#include "CITypeNamed.h"
#define GUCEF_CORE_CITYPENAMED_H
#endif /* GUCEF_CORE_CITYPENAMED_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#include "gucefCORE_CDataNodeSerializableSettings.h"
#define GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#endif /* GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H ? */

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

class CDataNode;

/*-------------------------------------------------------------------------*/

/**
 *  Abstract base class for adding a serialization API to descending classes.
 *  Serialize/Deserialize via the DataNode class as a DOM
 */
class GUCEF_CORE_PUBLIC_CPP CIDataNodeSerializable : public CICloneable ,
                                                     public CITypeNamed
{
    public:

    CIDataNodeSerializable( void );

    CIDataNodeSerializable( const CIDataNodeSerializable& src );

    virtual ~CIDataNodeSerializable() GUCEF_VIRTUAL_OVERRIDE;

    CIDataNodeSerializable& operator=( const CIDataNodeSerializable& src );

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     */
    virtual bool Serialize( CDataNode& domRootNode                        ,
                            const CDataNodeSerializableSettings& settings ) const = 0;

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     *
     *  @param domRootNode Node that acts as root of the DOM data tree from which to deserialize
     *  @return whether deserializing the object data from the given DOM was successful.
     */
    virtual bool Deserialize( const CDataNode& domRootNode                  ,
                              const CDataNodeSerializableSettings& settings ) = 0;

};

/*-------------------------------------------------------------------------*/

/**
 *  C++98 compatible SFINAE template helper
 *  Allows for checking for the existence of the CIDataNodeSerializable or compatible
 *  member function
 *           bool T::Serialize( CDataNode& domRootNode                        ,
 *                              const CDataNodeSerializableSettings& settings ) const
 */
template < class T >
struct TypeHasMemberFunctionForDataNodeSerialization
{
    // For the compile time comparison.
    typedef char    yes[1];
    typedef yes     no[2];

    template <typename U, U u> struct reallyHas;

    template < typename TestClass > static yes& test( reallyHas< bool (TestClass::*)( CDataNode& domRootNode, const CDataNodeSerializableSettings& settings ) const, &TestClass::Serialize >* /*unused*/ ) { static yes result; return result; }
    template < typename TestClass > static no&  test( ... ) { static no result; return result; }

    // The constant used as a return value for the test.
    enum { value = sizeof( test<T>(0) ) == sizeof( yes ) };
};

/*-------------------------------------------------------------------------*/

/**
 *  C++98 compatible SFINAE template helper
 *  Allows for checking for the existence of the CIDataNodeSerializable or compatible
 *  member function
 *           bool T::Deserialize( const CDataNode& domRootNode                  ,
 *                                const CDataNodeSerializableSettings& settings )
 */
template < class T >
struct TypeHasMemberFunctionForDataNodeDeserialization
{
    // For the compile time comparison.
    typedef char    yes[1];
    typedef yes     no[2];

    template <typename U, U u> struct reallyHas;

    template < typename TestClass > static yes& test( reallyHas< bool (TestClass::*)( const CDataNode& domRootNode, const CDataNodeSerializableSettings& settings ), &TestClass::Deserialize >* /*unused*/ ) { static yes result; return result; }
    template < typename TestClass > static no&  test( ... ) { static no result; return result; }

    // The constant used as a return value for the test.
    enum { value = sizeof( test<T>(0) ) == sizeof( yes ) };
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */
