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

#include "gucefCOM_CEMailErrorEventData.h"

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

CEMailErrorEventData::CEMailErrorEventData( void )
    : CORE::CICloneable()
    , errorClass( MAILERR_NONE )
    , description()
    , serverCode( 0 )
    , enhancedCode()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailErrorEventData::CEMailErrorEventData( EMailErrorClass errorClassOfError, const CORE::CString& descriptionOfError )
    : CORE::CICloneable()
    , errorClass( errorClassOfError )
    , description( descriptionOfError )
    , serverCode( 0 )
    , enhancedCode()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailErrorEventData::CEMailErrorEventData( const CEMailErrorEventData& src )
    : CORE::CICloneable( src )
    , errorClass( src.errorClass )
    , description( src.description )
    , serverCode( src.serverCode )
    , enhancedCode( src.enhancedCode )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailErrorEventData::~CEMailErrorEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailErrorEventData&
CEMailErrorEventData::operator=( const CEMailErrorEventData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        errorClass = src.errorClass;
        description = src.description;
        serverCode = src.serverCode;
        enhancedCode = src.enhancedCode;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CEMailErrorEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CEMailErrorEventData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CEMailErrorEventData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CEMailErrorEventData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COM::CEMailErrorEventData";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
