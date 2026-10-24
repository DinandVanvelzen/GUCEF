/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include "gucefCORE_CCodecParameterInfo.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCodecParameterInfo::CCodecParameterInfo( void )
    : m_name()
    , m_typeId( GUCEF_DATATYPE_UNKNOWN )
    , m_isRequired( false )
    , m_defaultValue()
    , m_description()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCodecParameterInfo::CCodecParameterInfo( const CString& name          ,
                                          UInt8 typeId                 ,
                                          bool isRequired              ,
                                          const CVariant& defaultValue ,
                                          const CString& description   )
    : m_name( name )
    , m_typeId( typeId )
    , m_isRequired( isRequired )
    , m_defaultValue( defaultValue )
    , m_description( description )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCodecParameterInfo::CCodecParameterInfo( const CCodecParameterInfo& src )
    : m_name( src.m_name )
    , m_typeId( src.m_typeId )
    , m_isRequired( src.m_isRequired )
    , m_defaultValue( src.m_defaultValue )
    , m_description( src.m_description )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCodecParameterInfo::~CCodecParameterInfo()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCodecParameterInfo&
CCodecParameterInfo::operator=( const CCodecParameterInfo& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_name = src.m_name;
        m_typeId = src.m_typeId;
        m_isRequired = src.m_isRequired;
        m_defaultValue = src.m_defaultValue;
        m_description = src.m_description;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CCodecParameterInfo::SetName( const CString& name )
{GUCEF_TRACE;

    m_name = name;
}

/*-------------------------------------------------------------------------*/

const CString&
CCodecParameterInfo::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

void
CCodecParameterInfo::SetTypeId( UInt8 typeId )
{GUCEF_TRACE;

    m_typeId = typeId;
}

/*-------------------------------------------------------------------------*/

UInt8
CCodecParameterInfo::GetTypeId( void ) const
{GUCEF_TRACE;

    return m_typeId;
}

/*-------------------------------------------------------------------------*/

void
CCodecParameterInfo::SetIsRequired( bool isRequired )
{GUCEF_TRACE;

    m_isRequired = isRequired;
}

/*-------------------------------------------------------------------------*/

bool
CCodecParameterInfo::GetIsRequired( void ) const
{GUCEF_TRACE;

    return m_isRequired;
}

/*-------------------------------------------------------------------------*/

void
CCodecParameterInfo::SetDefaultValue( const CVariant& defaultValue )
{GUCEF_TRACE;

    m_defaultValue = defaultValue;
}

/*-------------------------------------------------------------------------*/

const CVariant&
CCodecParameterInfo::GetDefaultValue( void ) const
{GUCEF_TRACE;

    return m_defaultValue;
}

/*-------------------------------------------------------------------------*/

void
CCodecParameterInfo::SetDescription( const CString& description )
{GUCEF_TRACE;

    m_description = description;
}

/*-------------------------------------------------------------------------*/

const CString&
CCodecParameterInfo::GetDescription( void ) const
{GUCEF_TRACE;

    return m_description;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
