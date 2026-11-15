/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#include "gucefWEB_CWebViewCapabilities.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CWebViewCapabilities::CapabilityCount;

namespace {

const char* CapabilityNames[ CWebViewCapabilities::CapabilityCount ] =
{
    "OffscreenFrames", "GpuFrames", "NativeWindow", "NativeChildWindow", "InjectPointer", "InjectKeyboard", "InjectText",
    "ImeComposition", "Cookies", "ScriptResult", "IsolatedWorlds", "UserAgentOverride", "ProfileDirectory", "EphemeralProfile",
    "ProxySettings", "NavigationPolicy", "DeferredDecisions", "NativeClipboard", "Headless", "TransparentBackground", "Zoom",
    "SecureFieldValue", "EditCommands"
};

}

/*-------------------------------------------------------------------------*/

CWebViewCapabilities::CWebViewCapabilities( void )
    : m_flags( CAP_NONE )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewCapabilities::CWebViewCapabilities( UInt32 flags )
    : m_flags( flags )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWebViewCapabilities::Has( UInt32 flags ) const
{GUCEF_TRACE;

    return ( m_flags & flags ) == flags;
}

/*-------------------------------------------------------------------------*/

void
CWebViewCapabilities::Set( UInt32 flags, bool present )
{GUCEF_TRACE;

    if ( present )
        m_flags |= flags;
    else
        m_flags &= ~flags;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewCapabilities::GetFlags( void ) const
{GUCEF_TRACE;

    return m_flags;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewCapabilities::GetMissing( UInt32 required ) const
{GUCEF_TRACE;

    return required & ~m_flags;
}

/*-------------------------------------------------------------------------*/

CString
CWebViewCapabilities::ToString( void ) const
{GUCEF_TRACE;

    return FlagsToString( m_flags );
}

/*-------------------------------------------------------------------------*/

CString
CWebViewCapabilities::FlagsToString( UInt32 flags )
{GUCEF_TRACE;

    CString text;
    for ( UInt32 i=0; i<CapabilityCount; ++i )
    {
        if ( 0 == ( flags & ( 1u << i ) ) )
            continue;
        if ( !text.IsNULLOrEmpty() )
            text += ",";
        text += CapabilityNames[ i ];
    }
    return text;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewCapabilities::FlagsFromString( const CString& names, UInt32* unknownNames )
{GUCEF_TRACE;

    UInt32 flags = CAP_NONE;
    UInt32 unknown = 0;
    CString::StringVector parts = names.ParseElements( ',', false );
    for ( CString::StringVector::iterator i=parts.begin(); i!=parts.end(); ++i )
    {
        const CString name = (*i).Trim( true ).Trim( false );
        if ( name.IsNULLOrEmpty() )
            continue;
        bool found = false;
        for ( UInt32 n=0; n<CapabilityCount && !found; ++n )
        {
            if ( name.Equals( CapabilityNames[ n ], false ) )
            {
                flags |= ( 1u << n );
                found = true;
            }
        }
        if ( !found )
            ++unknown;
    }
    if ( GUCEF_NULL != unknownNames )
        *unknownNames = unknown;
    return flags;
}

/*-------------------------------------------------------------------------*/

const char*
CWebViewCapabilities::GetFlagName( UInt32 singleFlag )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<CapabilityCount; ++i )
        if ( singleFlag == ( 1u << i ) )
            return CapabilityNames[ i ];
    return "";
}

/*-------------------------------------------------------------------------*/

bool
CWebViewCapabilities::operator==( const CWebViewCapabilities& other ) const
{GUCEF_TRACE;

    return m_flags == other.m_flags;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
