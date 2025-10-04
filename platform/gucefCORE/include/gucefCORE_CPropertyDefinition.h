/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CPROPERTYDEFINITION_H
#define GUCEF_CORE_CPROPERTYDEFINITION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_CVALUEDEFINITION_H
#include "gucefCORE_CValueDefinition.h"
#define GUCEF_CORE_CVALUEDEFINITION_H
#endif /* GUCEF_CORE_CVALUEDEFINITION_H ? */

#ifndef GUCEF_CORE_CPROPERTY_H
#include "gucefCORE_CProperty.h"
#define GUCEF_CORE_CPROPERTY_H
#endif /* GUCEF_CORE_CPROPERTY_H ? */

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
 *  Class which describes a property as something that has a name with a value and a (unique?) id
 */
class GUCEF_CORE_PUBLIC_CPP CPropertyDefinition : public CIDataNodeSerializable
{
    public:

    static const CString ClassTypeName;

    typedef GUCEF::vector< CPropertyDefinition >  PropertyDefinitionVector;

    CValueDefinition name;
    CValueDefinition id;
    CValueDefinition value;

    /**
     *  member function that will carry out the constraint checks
     *  against the property given as 'propertyToConstrain' using the full set of constraints
     */
    bool AreConstraintsSatisfiedBy( const CProperty& propertyToConstrain ) const;

    CPropertyDefinition( void );

    CPropertyDefinition( const CPropertyDefinition& src );

    CPropertyDefinition& operator=( const CPropertyDefinition& src );

    bool operator==( const CPropertyDefinition& other ) const;

    bool operator!=( const CPropertyDefinition& other ) const;
    
    virtual ~CPropertyDefinition();

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

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CPROPERTYDEFINITION_H ? */
