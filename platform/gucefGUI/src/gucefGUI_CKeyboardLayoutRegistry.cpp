/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#include "gucefMT_CScopeMutex.h"

#include "gucefGUI_CKeyboardLayoutRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CKeyboardLayoutRegistry::DefaultLayoutName = "US";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKeyboardLayoutRegistry::CKeyboardLayoutRegistry( void )
    : m_layouts()
    , m_lock()
{GUCEF_TRACE;

    Reset();
}

/*-------------------------------------------------------------------------*/

CKeyboardLayoutRegistry::~CKeyboardLayoutRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CKeyboardLayoutRegistry::Register( const CKeyboardLayout& layout )
{GUCEF_TRACE;

    if ( layout.GetName().IsNULLOrEmpty() || !layout.IsValid() )
        return false;

    MT::CScopeMutex lock( m_lock );
    m_layouts[ layout.GetName() ] = layout;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CKeyboardLayoutRegistry::Unregister( const CString& name )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return 0 != m_layouts.erase( name );
}

/*-------------------------------------------------------------------------*/

bool
CKeyboardLayoutRegistry::IsRegistered( const CString& name ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_layouts.find( name ) != m_layouts.end();
}

/*-------------------------------------------------------------------------*/

bool
CKeyboardLayoutRegistry::Get( const CString& name, CKeyboardLayout& layout ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TLayoutMap::const_iterator i = m_layouts.find( name );
    if ( i == m_layouts.end() )
        return false;
    layout = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

CKeyboardLayoutRegistry::TStringVector
CKeyboardLayoutRegistry::GetNames( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TStringVector names;
    for ( TLayoutMap::const_iterator i=m_layouts.begin(); i!=m_layouts.end(); ++i )
        names.push_back( (*i).first );
    return names;
}

/*-------------------------------------------------------------------------*/

void
CKeyboardLayoutRegistry::Reset( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_layouts.clear();
    m_layouts[ DefaultLayoutName ] = CKeyboardLayout::MakeUS();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
