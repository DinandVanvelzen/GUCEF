/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#include "gucefASSETSTORE_CAssetSettingsRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetSettingsRegistry::CAssetSettingsRegistry( void )
    : m_descriptors()
    , m_index()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetSettingsRegistry::~CAssetSettingsRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingsRegistry::Register( const CAssetSettingDescriptor& descriptor, CORE::CString& reason )
{GUCEF_TRACE;

    if ( !descriptor.IsValid( reason ) )
        return false;
    if ( m_index.find( descriptor.key ) != m_index.end() )
    {
        reason = "the setting " + descriptor.key + " is registered already";
        return false;
    }
    m_index[ descriptor.key ] = (UInt32) m_descriptors.size();
    m_descriptors.push_back( descriptor );
    return true;
}

/*-------------------------------------------------------------------------*/

const CAssetSettingDescriptor*
CAssetSettingsRegistry::Find( const CORE::CString& key ) const
{GUCEF_TRACE;

    GUCEF::map< CORE::CString, UInt32 >::const_iterator i = m_index.find( key );
    return i == m_index.end() ? GUCEF_NULL : &m_descriptors[ (*i).second ];
}

/*-------------------------------------------------------------------------*/

const CAssetSettingsRegistry::TDescriptorVector&
CAssetSettingsRegistry::GetDescriptors( void ) const
{GUCEF_TRACE;

    return m_descriptors;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetSettingsRegistry::GetCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_descriptors.size();
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingsRegistry::Normalize( const CORE::CString& key, const CORE::CString& text, CORE::CString& normalized, CORE::CString& problem ) const
{GUCEF_TRACE;

    const CAssetSettingDescriptor* descriptor = Find( key );
    if ( GUCEF_NULL == descriptor )
    {
        normalized.Clear();
        problem = "not a setting of this service";
        return false;
    }
    return descriptor->Normalize( text, normalized, problem );
}

/*-------------------------------------------------------------------------*/

void
CAssetSettingsRegistry::Clear( void )
{GUCEF_TRACE;

    m_descriptors.clear();
    m_index.clear();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
