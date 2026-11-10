/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#include "gucefAUDIO_CAudioDeviceInfo.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioDeviceInfo::CAudioDeviceInfo( void )
    : m_id()
    , m_name()
    , m_direction( AUDIODIRECTION_PLAYBACK )
    , m_isDefault( false )
    , m_preferredFormat()
    , m_maxChannelCount( 0 )
    , m_properties()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioDeviceInfo::CAudioDeviceInfo( const CString& id, const CString& name, TAudioDirection direction )
    : m_id( id )
    , m_name( name )
    , m_direction( direction )
    , m_isDefault( false )
    , m_preferredFormat()
    , m_maxChannelCount( 0 )
    , m_properties()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioDeviceInfo::CAudioDeviceInfo( const CAudioDeviceInfo& src )
    : m_id( src.m_id )
    , m_name( src.m_name )
    , m_direction( src.m_direction )
    , m_isDefault( src.m_isDefault )
    , m_preferredFormat( src.m_preferredFormat )
    , m_maxChannelCount( src.m_maxChannelCount )
    , m_properties( src.m_properties )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioDeviceInfo&
CAudioDeviceInfo::operator=( const CAudioDeviceInfo& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_id = src.m_id;
        m_name = src.m_name;
        m_direction = src.m_direction;
        m_isDefault = src.m_isDefault;
        m_preferredFormat = src.m_preferredFormat;
        m_maxChannelCount = src.m_maxChannelCount;
        m_properties = src.m_properties;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CAudioDeviceInfo::~CAudioDeviceInfo()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CAudioDeviceInfo::GetId( void ) const
{GUCEF_TRACE;

    return m_id;
}

/*-------------------------------------------------------------------------*/

void
CAudioDeviceInfo::SetId( const CString& id )
{GUCEF_TRACE;

    m_id = id;
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioDeviceInfo::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

void
CAudioDeviceInfo::SetName( const CString& name )
{GUCEF_TRACE;

    m_name = name;
}

/*-------------------------------------------------------------------------*/

TAudioDirection
CAudioDeviceInfo::GetDirection( void ) const
{GUCEF_TRACE;

    return m_direction;
}

/*-------------------------------------------------------------------------*/

void
CAudioDeviceInfo::SetDirection( TAudioDirection direction )
{GUCEF_TRACE;

    m_direction = direction;
}

/*-------------------------------------------------------------------------*/

bool
CAudioDeviceInfo::IsDefault( void ) const
{GUCEF_TRACE;

    return m_isDefault;
}

/*-------------------------------------------------------------------------*/

void
CAudioDeviceInfo::SetIsDefault( bool isDefault )
{GUCEF_TRACE;

    m_isDefault = isDefault;
}

/*-------------------------------------------------------------------------*/

const CAudioFormat&
CAudioDeviceInfo::GetPreferredFormat( void ) const
{GUCEF_TRACE;

    return m_preferredFormat;
}

/*-------------------------------------------------------------------------*/

void
CAudioDeviceInfo::SetPreferredFormat( const CAudioFormat& format )
{GUCEF_TRACE;

    m_preferredFormat = format;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioDeviceInfo::GetMaxChannelCount( void ) const
{GUCEF_TRACE;

    return m_maxChannelCount;
}

/*-------------------------------------------------------------------------*/

void
CAudioDeviceInfo::SetMaxChannelCount( UInt32 channels )
{GUCEF_TRACE;

    m_maxChannelCount = channels;
}

/*-------------------------------------------------------------------------*/

const CORE::CValueList&
CAudioDeviceInfo::GetProperties( void ) const
{GUCEF_TRACE;

    return m_properties;
}

/*-------------------------------------------------------------------------*/

CORE::CValueList&
CAudioDeviceInfo::GetProperties( void )
{GUCEF_TRACE;

    return m_properties;
}


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
