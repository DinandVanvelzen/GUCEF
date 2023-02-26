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

#ifndef GUCEF_CORE_CVALUECONSTRAINTLIST_H
#define GUCEF_CORE_CVALUECONSTRAINTLIST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUECONSTRAINT_H
#include "gucefCORE_CValueConstraint.h"
#define GUCEF_CORE_CVALUECONSTRAINT_H
#endif /* GUCEF_CORE_CVALUECONSTRAINT_H ? */

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
 *  Class to provide the ability to represent a set of value constraints as an object 
 *  and provide the ability to check against said constraints
 */
class GUCEF_CORE_PUBLIC_CPP CValueConstraintList : public CIDataNodeSerializable
{
    public:

    static const CString ClassTypeName;
    
    typedef CValueConstraint::ValueConstraintVector   ValueConstraintVector;

    /**
     *  This is what it is all about. This is the member function that will carry out the constraint check
     *  against the value given as 'valueToConstrain' using the full set of constraints
     */
    bool AreConstraintsSatisfiedBy( const CVariant& valueToConstrain ) const;       

    CValueConstraintList( void );

    CValueConstraintList( const CValueConstraintList& src );
    
    virtual ~CValueConstraintList();

    ValueConstraintVector& GetConstraints( void );

    const ValueConstraintVector& GetConstraints( void ) const;

    bool operator==( const CValueConstraintList& other ) const;

    bool operator!=( const CValueConstraintList& other ) const;

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

    private:

    ValueConstraintVector m_constraints;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CVALUECONSTRAINTLIST_H ? */
