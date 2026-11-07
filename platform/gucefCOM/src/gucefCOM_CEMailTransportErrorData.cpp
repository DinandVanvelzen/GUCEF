/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#include "gucefCOM_CEMailTransportErrorData.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEMailTransportErrorData::CEMailTransportErrorData( void )
    : CORE::CICloneable()
    , errorClass( MAILERR_CONNECTION )
    , description()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailTransportErrorData::CEMailTransportErrorData( EMailErrorClass errorClassOfError, const CORE::CString& descriptionOfError )
    : CORE::CICloneable()
    , errorClass( errorClassOfError )
    , description( descriptionOfError )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailTransportErrorData::CEMailTransportErrorData( const CEMailTransportErrorData& src )
    : CORE::CICloneable( src )
    , errorClass( src.errorClass )
    , description( src.description )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailTransportErrorData::~CEMailTransportErrorData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailTransportErrorData&
CEMailTransportErrorData::operator=( const CEMailTransportErrorData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        errorClass = src.errorClass;
        description = src.description;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CEMailTransportErrorData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CEMailTransportErrorData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CEMailTransportErrorData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CEMailTransportErrorData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COM::CEMailTransportErrorData";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
