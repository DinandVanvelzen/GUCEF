/*
 *  gucefCOM: GUCEF module providing communication implementations 
 *  for standardized protocols
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

#ifndef CDBFIELD_H
#define CDBFIELD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef DVSTRUTILS_H
#include "dvstrutils.h" /* my verry own string utils */
#define DVSTRUTILS_H
#endif /* DVSTRUTILS_H ? */

#ifndef GUCEFCOM_MACROS_H
#include "gucefCOM_macros.h"
#define GUCEFCOM_MACROS_H
#endif /* GUCEFCOM_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *	Forward declarations of classes used
 */
class CMySQLClient;	/* MySQL client implementation */

/*--------------------------------------------------------------------------*/

/**
 *	Inline implementation of a database field. Initial use is for the
 *	CMySQLClient as data storage.
 */
class GUCEF_COM_PUBLIC_CPP CDBField
{
	private:
        friend class CMySQLClient;
        char *value;

	public:
        const char *AsString( void ) { return value; }

        Int32 AsInteger( void ) { return CORE::Str_To_Int( value ); }

        CDBField( void ) { value = GUCEF_NULL; }
        virtual ~CDBField() { if(value != GUCEF_NULL) delete[] value; }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* CDBFIELD_H ? */

