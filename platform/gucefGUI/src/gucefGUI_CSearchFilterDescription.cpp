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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CSEARCHFILTERDESCRIPTION_H
#include "gucefGUI_CSearchFilterDescription.h"
#define GUCEF_GUI_CSEARCHFILTERDESCRIPTION_H
#endif /* GUCEF_GUI_CSEARCHFILTERDESCRIPTION_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/


CSearchFilterDescription::SChoice::SChoice( void )
    : value()
    , label()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSearchFilterDescription::SChoice::SChoice( const CString& choiceValue, const CString& choiceLabel )
    : value( choiceValue )
    , label( choiceLabel )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSearchFilterDescription::SGroup::SGroup( void )
    : id()
    , title()
    , kind( FILTERKIND_CHOICES )
    , choices()
    , unit()
    , tooltip()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSearchFilterDescription::CSearchFilterDescription( void )
    : m_groups()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CSearchFilterDescription::Clear( void )
{GUCEF_TRACE;

    m_groups.clear();
}

/*-------------------------------------------------------------------------*/

CSearchFilterDescription::TGroup&
CSearchFilterDescription::AddChoiceGroup( const CString& id, const CString& title )
{GUCEF_TRACE;

    TGroup group;
    group.id = id;
    group.title = title;
    group.kind = FILTERKIND_CHOICES;
    m_groups.push_back( group );
    return m_groups.back();
}

/*-------------------------------------------------------------------------*/

CSearchFilterDescription::TGroup&
CSearchFilterDescription::AddRangeGroup( const CString& id, const CString& title, const CString& unit )
{GUCEF_TRACE;

    TGroup group;
    group.id = id;
    group.title = title;
    group.kind = FILTERKIND_RANGE;
    group.unit = unit;
    m_groups.push_back( group );
    return m_groups.back();
}

/*-------------------------------------------------------------------------*/

bool
CSearchFilterDescription::AddChoice( const CString& groupId, const CString& value, const CString& label )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_groups.size(); ++i )
    {
        if ( m_groups[ i ].id == groupId && FILTERKIND_CHOICES == m_groups[ i ].kind )
        {
            m_groups[ i ].choices.push_back( TChoice( value, label.IsNULLOrEmpty() ? value : label ) );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CSearchFilterDescription::TGroupVector&
CSearchFilterDescription::GetGroups( void ) const
{GUCEF_TRACE;

    return m_groups;
}

/*-------------------------------------------------------------------------*/

const CSearchFilterDescription::TGroup*
CSearchFilterDescription::FindGroup( const CString& id ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_groups.size(); ++i )
        if ( m_groups[ i ].id == id )
            return &m_groups[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CSearchFilterDescription::operator==( const CSearchFilterDescription& other ) const
{GUCEF_TRACE;

    if ( m_groups.size() != other.m_groups.size() )
        return false;
    for ( size_t i=0; i<m_groups.size(); ++i )
    {
        const TGroup& a = m_groups[ i ];
        const TGroup& b = other.m_groups[ i ];
        if ( a.id != b.id || a.title != b.title || a.kind != b.kind || a.unit != b.unit || a.tooltip != b.tooltip || a.choices.size() != b.choices.size() )
            return false;
        for ( size_t c=0; c<a.choices.size(); ++c )
            if ( a.choices[ c ].value != b.choices[ c ].value || a.choices[ c ].label != b.choices[ c ].label )
                return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
