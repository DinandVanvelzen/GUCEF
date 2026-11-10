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

#include "gucefGUI_CWidgetHandler.h"

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

CWidgetHandler::SApplierChain::SApplierChain( void )
    : before()
    , main( GUCEF_NULL )
    , after()
{
}

/*-------------------------------------------------------------------------*/

CWidgetHandler::CWidgetHandler( void )
    : m_propertyMap()
    , m_commandMap()
{
}

/*-------------------------------------------------------------------------*/

CWidgetHandler::~CWidgetHandler()
{
}

/*-------------------------------------------------------------------------*/

void
CWidgetHandler::ApplyParent( CWidget& widget )
{
}

/*-------------------------------------------------------------------------*/

void
CWidgetHandler::ApplyLayout( CWidget& widget, const CRect& rect )
{
}

/*-------------------------------------------------------------------------*/

void
CWidgetHandler::ApplyChildOrder( CWidget& widget )
{
}

/*-------------------------------------------------------------------------*/

void
CWidgetHandler::ApplyState( CWidget& widget )
{
}

/*-------------------------------------------------------------------------*/

bool
CWidgetHandler::ApplyProperty( CWidget& widget, const CString& name, const CString& value )
{
    TPropertyMap::iterator i = m_propertyMap.find( name );
    if ( i == m_propertyMap.end() )
        return false;

    SApplierChain& chain = (*i).second;
    bool success = true;
    for ( UInt32 n=0; n<(UInt32)chain.before.size(); ++n )
        success = chain.before[ n ]( *this, widget, value ) && success;
    if ( GUCEF_NULL != chain.main )
        success = chain.main( *this, widget, value ) && success;
    for ( UInt32 n=0; n<(UInt32)chain.after.size(); ++n )
        success = chain.after[ n ]( *this, widget, value ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CWidgetHandler::RunCommand( CWidget& widget, const CString& command, const CORE::CValueList& arguments )
{
    TCommandMap::iterator i = m_commandMap.find( command );
    if ( i == m_commandMap.end() || GUCEF_NULL == (*i).second )
        return false;
    return (*i).second( *this, widget, arguments );
}

/*-------------------------------------------------------------------------*/

void*
CWidgetHandler::GetNativeHandle( const CWidget& widget ) const
{
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CWidgetHandler::MapProperty( const CString& name, TPropertyApplier applier )
{
    m_propertyMap[ name ].main = applier;
}

/*-------------------------------------------------------------------------*/

void
CWidgetHandler::PrependPropertyApplier( const CString& name, TPropertyApplier applier )
{
    GUCEF::vector< TPropertyApplier >& before = m_propertyMap[ name ].before;
    before.insert( before.begin(), applier );
}

/*-------------------------------------------------------------------------*/

void
CWidgetHandler::AppendPropertyApplier( const CString& name, TPropertyApplier applier )
{
    m_propertyMap[ name ].after.push_back( applier );
}

/*-------------------------------------------------------------------------*/

void
CWidgetHandler::ModifyPropertyApplier( const CString& name, TPropertyApplier applier )
{
    // replaces the mapped applier and drops the ones put before and after it
    SApplierChain& chain = m_propertyMap[ name ];
    chain.before.clear();
    chain.after.clear();
    chain.main = applier;
}

/*-------------------------------------------------------------------------*/

void
CWidgetHandler::MapCommand( const CString& name, TCommandRunner runner )
{
    m_commandMap[ name ] = runner;
}

/*-------------------------------------------------------------------------*/

CWidgetHandler::TStringSet
CWidgetHandler::GetMappedProperties( void ) const
{
    TStringSet names;
    TPropertyMap::const_iterator i = m_propertyMap.begin();
    while ( i != m_propertyMap.end() )
    {
        names.insert( (*i).first );
        ++i;
    }
    return names;
}

/*-------------------------------------------------------------------------*/

CWidgetHandler::TStringSet
CWidgetHandler::GetMappedCommands( void ) const
{
    TStringSet names;
    TCommandMap::const_iterator i = m_commandMap.begin();
    while ( i != m_commandMap.end() )
    {
        names.insert( (*i).first );
        ++i;
    }
    return names;
}

/*-------------------------------------------------------------------------*/

bool
CWidgetHandler::IsPropertyMapped( const CString& name ) const
{
    return m_propertyMap.find( name ) != m_propertyMap.end();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
