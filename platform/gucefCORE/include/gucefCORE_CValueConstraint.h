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

#ifndef GUCEF_CORE_CVALUECONSTRAINT_H
#define GUCEF_CORE_CVALUECONSTRAINT_H

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

#ifndef GUCEF_CORE_CIFUNCTION_H
#include "gucefCORE_CIFunction.h"
#define GUCEF_CORE_CIFUNCTION_H
#endif /* GUCEF_CORE_CIFUNCTION_H ? */

#ifndef GUCEF_CORE_CFUNCTIONREGISTRY_H
#include "gucefCORE_CFunctionRegistry.h"
#define GUCEF_CORE_CFUNCTIONREGISTRY_H
#endif /* GUCEF_CORE_CFUNCTIONREGISTRY_H ? */

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
 *  Class to provide the ability to represent a constraint as an object 
 *  and provide the ability to check against said constraint
 */
class GUCEF_CORE_PUBLIC_CPP CValueConstraint : public CIDataNodeSerializable
{
    public:

    static const CString ClassTypeName;

    typedef GUCEF::vector< CValueConstraint >            ValueConstraintVector;

    /**
     *  This is what it is all about. This is the member function that will carry out the constraint check
     *  against the value given as 'valueToConstrain'
     */
    bool IsConstraintSatisfiedBy( const CVariant& valueToConstrain ) const;
        
    CValueConstraint( const CString& functionName                                   ,
                      const CVariant::VariantVector& params = CVariant::EmptyVector ,
                      Int32 constrainedValueParamIndex = 0                          ,
                      CFunctionRegistry* functionRegistry = GUCEF_NULL              ,
                      bool passIfFunctionExecIssue = true                           ,
                      UInt32 constraintResultIndex = 0                              );

    CValueConstraint( CIFunction* function                                          ,
                      const CVariant::VariantVector& params = CVariant::EmptyVector ,
                      Int32 constrainedValueParamIndex = 0                          ,
                      bool passIfFunctionExecIssue = true                           ,
                      UInt32 constraintResultIndex = 0                              );

    CValueConstraint( void );

    CValueConstraint( const CValueConstraint& src );
    
    virtual ~CValueConstraint();

    bool SetValueConstraint( const CString& functionName                                   ,
                             const CVariant::VariantVector& params = CVariant::EmptyVector ,
                             Int32 constrainedValueParamIndex = 0                          ,
                             CFunctionRegistry* functionRegistry = GUCEF_NULL              ,
                             bool passIfFunctionExecIssue = true                           ,
                             UInt32 constraintResultIndex = 0                              );

    bool SetValueConstraint( CIFunction* function                                          ,
                             const CVariant::VariantVector& params = CVariant::EmptyVector ,
                             Int32 constrainedValueParamIndex = 0                          ,
                             bool passIfFunctionExecIssue = true                           ,
                             UInt32 constraintResultIndex = 0                              );

    bool operator==( const CValueConstraint& other ) const;

    bool operator!=( const CValueConstraint& other ) const;

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

    CString m_functionName;
    CIFunction* m_function;
    mutable CVariant::VariantVector m_params;
    Int32 m_constrainedValueParamIndex;
    bool m_passIfFunctionExecIssue;
    mutable CVariant::VariantVector m_functionResults;
    UInt32 m_constraintResultIndex;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CVALUECONSTRAINT_H ? */
