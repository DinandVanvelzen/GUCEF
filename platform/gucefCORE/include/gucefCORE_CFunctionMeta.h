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

#ifndef GUCEF_CORE_CFUNCTIONMETA_H
#define GUCEF_CORE_CFUNCTIONMETA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_C_FUNCTION_DEFINITION_H
#include "gucefCORE_c_function_definition.h"
#define GUCEF_CORE_C_FUNCTION_DEFINITION_H
#endif /* GUCEF_CORE_C_FUNCTION_DEFINITION_H ? */

#ifndef GUCEF_CORE_CFUNCTIONPARAMMETA_H
#include "gucefCORE_CFunctionParamMeta.h"
#define GUCEF_CORE_CFUNCTIONPARAMMETA_H
#endif /* GUCEF_CORE_CFUNCTIONPARAMMETA_H ? */

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
 *  Class to hold meta-data for a runtime registered function
 * 
 *  Intended to work with the execution engine flow as a building block in a 
 *  generic scripting system
 * 
 *  This class provides an underlying C style structure
 *  this allows for easy integration with C style APIs
 *  which are a common way to interface with scripting languages
 */
class GUCEF_CORE_PUBLIC_CPP CFunctionMeta
{
    public:

    typedef GUCEF::vector< CFunctionParamMeta >    TFunctionParamMetaVector;

    CFunctionMeta( void );

    CFunctionMeta( const CFunctionMeta& src );

    CFunctionMeta( const TFunctionDefinition* c_style_function_meta );

    ~CFunctionMeta();

    CFunctionMeta& operator=( const CFunctionMeta& src );

    void SetParams( const TFunctionParamMetaVector& params );

    const TFunctionParamMetaVector& GetParams( void ) const;

    void SetIsFunctionConst( bool isParamConst );

    bool GetIsFunctionConst( void ) const;

    void SetFunctionName( const CString& paramName );

    const CString& GetFunctionName( void ) const;

    const TFunctionDefinition* GetCStyleFunctionMeta( void ) const;

    private:

    TFunctionDefinition m_functionMeta;
    TFunctionParamMetaVector m_paramsMeta;
    CString m_functionName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CFUNCTIONMETA_H ? */
