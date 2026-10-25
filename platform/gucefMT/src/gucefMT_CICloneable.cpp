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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#include "gucefMT_CICloneable.h"

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

CICloneable::CICloneable( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CICloneable::CICloneable( const CICloneable& src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CICloneable::~CICloneable()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CICloneable&
CICloneable::operator=( const CICloneable& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

const char*
CICloneable::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CICloneable::IsCloneableType( const char* typeName ) const
{GUCEF_TRACE;

    const char* ownName = GetCloneableTypeName();
    if ( GUCEF_NULL == ownName || GUCEF_NULL == typeName )
        return false;
    return ownName == typeName || 0 == strcmp( ownName, typeName );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
