/*
 *  gucefMT: GUCEF module providing multithreading solutions
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_MT_CICLONEABLE_H
#define GUCEF_MT_CICLONEABLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_ETYPES_H
#include "gucefMT_ETypes.h"             /* simple types */
#define GUCEF_MT_ETYPES_H
#endif /* GUCEF_MT_ETYPES_H ? */

#ifndef GUCEF_MT_MACROS_H
#include "gucefMT_macros.h"             /* often used gucef macros */
#define GUCEF_MT_MACROS_H
#endif /* GUCEF_MT_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Interface class that adds a cloning interface to derived classes.
 *  Clone() should be implemented as:
 *      return new CMyClass( *this );
 */
class GUCEF_MT_PUBLIC_CPP CICloneable
{
    public:

    CICloneable( void );

    CICloneable( const CICloneable& src );

    virtual ~CICloneable();

    CICloneable& operator=( const CICloneable& src );

    virtual CICloneable* Clone( void ) const = 0;

    /**
     *  Names the concrete class of this object so a holder of a CICloneable pointer can check what it has
     *  without run time type information (RTTI can be disabled, so dynamic_cast is not an option).
     *  A class that is handed around as a CICloneable and checked by its users overrides this to return
     *  a name that is unique to the class, for example its qualified class name, the same string as the
     *  static CloneableTypeName() it offers. The default is NULL: the class does not tell.
     */
    virtual const char* GetCloneableTypeName( void ) const;

    /** Whether GetCloneableTypeName() says the given name (an exact match, a derived class has to say the name itself) */
    bool IsCloneableType( const char* typeName ) const;

    /**
     *  The object as the class T when it says it is a T, else NULL (also for a NULL object). The check is by
     *  name, so no RTTI is needed. T needs a static CloneableTypeName() and has to override GetCloneableTypeName()
     *  to return the same name.
     */
    template < class T >
    static const T* CloneableAs( const CICloneable* object )
    {
        return ( GUCEF_NULL != object && object->IsCloneableType( T::CloneableTypeName() ) ) ? static_cast< const T* >( object ) : GUCEF_NULL;
    }

    template < class T >
    static T* CloneableAs( CICloneable* object )
    {
        return ( GUCEF_NULL != object && object->IsCloneableType( T::CloneableTypeName() ) ) ? static_cast< T* >( object ) : GUCEF_NULL;
    }

};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MT_CICLONEABLE_H  ? */
