/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */
 
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/ 

#include "gucefCORE_CPluginGroup.h"  /* definition of the class implemented here */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */ 

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/
    
CPluginGroup::CPluginGroup( void )
    : CTSharedObjCreator< CPluginGroup, MT::CMutex >( this )
    , m_plugins()        
    , m_pluginMetaData()
    , m_nextSequence( 0 )
    , m_priority( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPluginGroup::CPluginGroup( const CPluginGroup& src )
    : CTSharedObjCreator< CPluginGroup, MT::CMutex >( this )
    , m_plugins( src.m_plugins )                
    , m_pluginMetaData( src.m_pluginMetaData )
    , m_nextSequence( src.m_nextSequence )
    , m_priority( src.m_priority )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPluginGroup::~CPluginGroup()
{GUCEF_TRACE;

    m_plugins.clear();
    m_pluginMetaData.clear();
    m_priority = 0;
}

/*-------------------------------------------------------------------------*/

CPluginGroup&
CPluginGroup::operator=( const CPluginGroup& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_plugins = src.m_plugins;
        m_pluginMetaData = src.m_pluginMetaData;
        m_nextSequence = src.m_nextSequence;
        m_priority = src.m_priority;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CPluginGroup::IsBefore( const SEntry& a, const SEntry& b )
{GUCEF_TRACE;

    if ( a.priorityInGroup != b.priorityInGroup )
        return a.priorityInGroup < b.priorityInGroup;
    return a.sequence < b.sequence;
}

/*-------------------------------------------------------------------------*/

void
CPluginGroup::InsertInLoadOrder( TEntryList& entries, const SEntry& entry )
{GUCEF_TRACE;

    TEntryList::iterator i = entries.begin();
    while ( i != entries.end() && IsBefore( (*i), entry ) )
        ++i;
    entries.insert( i, entry );
}

/*-------------------------------------------------------------------------*/

CPluginGroup::TPluginList
CPluginGroup::GetPlugins( void ) const
{GUCEF_TRACE;

    TPluginList plugins;
    plugins.reserve( m_plugins.size() );
    for ( TEntryList::const_iterator i = m_plugins.begin(); i != m_plugins.end(); ++i )
        plugins.push_back( (*i).plugin );
    return plugins;
}

/*-------------------------------------------------------------------------*/

bool
CPluginGroup::HasPlugins( void ) const
{GUCEF_TRACE;

    return !m_plugins.empty();
}

/*-------------------------------------------------------------------------*/

CPluginGroup::TPluginMetaDataList
CPluginGroup::GetPluginMetaData( void ) const
{GUCEF_TRACE;

    TPluginMetaDataList metaData;
    metaData.reserve( m_pluginMetaData.size() );
    for ( TEntryList::const_iterator i = m_pluginMetaData.begin(); i != m_pluginMetaData.end(); ++i )
        metaData.push_back( (*i).metaData );
    return metaData;
}

/*-------------------------------------------------------------------------*/

void
CPluginGroup::GetModuleFilenamesInLoadOrder( TStringVector& moduleFilenames ) const
{GUCEF_TRACE;

    // both lists are in load order: merge them
    TEntryList::const_iterator p = m_plugins.begin();
    TEntryList::const_iterator m = m_pluginMetaData.begin();
    while ( p != m_plugins.end() || m != m_pluginMetaData.end() )
    {
        const bool takePlugin = m == m_pluginMetaData.end() || ( p != m_plugins.end() && IsBefore( (*p), (*m) ) );
        if ( takePlugin )
        {
            TPluginMetaDataPtr metaData = (*p).plugin->GetMetaData();
            moduleFilenames.push_back( metaData.IsNULL() ? CString::Empty : metaData->GetModuleFilename() );
            ++p;
        }
        else
        {
            moduleFilenames.push_back( (*m).metaData->GetModuleFilename() );
            ++m;
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CPluginGroup::GetHighestPriorityInGroup( void ) const
{GUCEF_TRACE;

    UInt32 highest = 0;
    if ( !m_plugins.empty() && m_plugins.back().priorityInGroup > highest )
        highest = m_plugins.back().priorityInGroup;
    if ( !m_pluginMetaData.empty() && m_pluginMetaData.back().priorityInGroup > highest )
        highest = m_pluginMetaData.back().priorityInGroup;
    return highest;
}

/*-------------------------------------------------------------------------*/

UInt32
CPluginGroup::AddPluginMetaData( const TPluginMetaDataStoragePtr& metaData, UInt32 priorityInGroup )
{GUCEF_TRACE;

    for ( TEntryList::const_iterator i = m_pluginMetaData.begin(); i != m_pluginMetaData.end(); ++i )
        if ( (*i).metaData == metaData )
            return (*i).priorityInGroup;

    SEntry entry;
    entry.metaData = metaData;
    entry.priorityInGroup = priorityInGroup > 0 ? priorityInGroup : GetHighestPriorityInGroup() + 1;
    entry.sequence = m_nextSequence++;
    InsertInLoadOrder( m_pluginMetaData, entry );
    return entry.priorityInGroup;
}

/*-------------------------------------------------------------------------*/

bool
CPluginGroup::RemovePluginMetaData( const TPluginMetaDataStoragePtr& metaData )
{GUCEF_TRACE;

    for ( TEntryList::iterator i = m_pluginMetaData.begin(); i != m_pluginMetaData.end(); ++i )
    {
        if ( (*i).metaData == metaData )
        {
            m_pluginMetaData.erase( i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CPluginGroup::ClearPluginMetaData( void )
{GUCEF_TRACE;

    m_pluginMetaData.clear();
}

/*-------------------------------------------------------------------------*/

bool
CPluginGroup::ReplacePluginMetaDataWithPlugin( const TPluginMetaDataStoragePtr& metaData, const TPluginPtr& plugin )
{GUCEF_TRACE;

    for ( TEntryList::iterator i = m_pluginMetaData.begin(); i != m_pluginMetaData.end(); ++i )
    {
        if ( (*i).metaData == metaData )
        {
            SEntry entry = (*i);
            m_pluginMetaData.erase( i );
            entry.metaData.Unlink();
            entry.plugin = plugin;
            InsertInLoadOrder( m_plugins, entry );
            return true;
        }
    }

    // meta data that the group did not have: the plugin comes after what the group has
    SEntry entry;
    entry.plugin = plugin;
    entry.priorityInGroup = GetHighestPriorityInGroup() + 1;
    entry.sequence = m_nextSequence++;
    InsertInLoadOrder( m_plugins, entry );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPluginGroup::ReplacePluginWithPluginMetaData( const TPluginPtr& plugin, const TPluginMetaDataStoragePtr& metaData )
{GUCEF_TRACE;

    for ( TEntryList::iterator i = m_plugins.begin(); i != m_plugins.end(); ++i )
    {
        if ( (*i).plugin == plugin )
        {
            SEntry entry = (*i);
            m_plugins.erase( i );
            entry.plugin.Unlink();
            entry.metaData = metaData;
            InsertInLoadOrder( m_pluginMetaData, entry );
            return true;
        }
    }
    AddPluginMetaData( metaData );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPluginGroup::RemovePlugin( const TPluginPtr& plugin )
{GUCEF_TRACE;

    for ( TEntryList::iterator i = m_plugins.begin(); i != m_plugins.end(); ++i )
    {
        if ( (*i).plugin == plugin )
        {
            m_plugins.erase( i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPluginGroup::HasPluginWithFileName( const CString& moduleFileName ) const
{GUCEF_TRACE;

    return !FindPluginWithModuleName( moduleFileName ).IsNULL() || !FindPluginMetaDataWithModuleName( moduleFileName ).IsNULL();
}

/*-------------------------------------------------------------------------*/

TPluginPtr 
CPluginGroup::FindPluginWithModuleName( const CString& moduleName ) const
{GUCEF_TRACE;

    for ( TEntryList::const_iterator i = m_plugins.begin(); i != m_plugins.end(); ++i )
    {
        TPluginMetaDataPtr metadata = (*i).plugin->GetMetaData();
        if ( !metadata.IsNULL() && moduleName == metadata->GetModuleFilename() )
            return (*i).plugin;
    }
    return TPluginPtr();
}

/*-------------------------------------------------------------------------*/

TPluginMetaDataPtr 
CPluginGroup::FindPluginMetaDataWithModuleName( const CString& moduleName ) const
{GUCEF_TRACE;

    for ( TEntryList::const_iterator i = m_pluginMetaData.begin(); i != m_pluginMetaData.end(); ++i )
    {
        if ( !(*i).metaData.IsNULL() && moduleName == (*i).metaData->GetModuleFilename() )
            return (*i).metaData;
    }
    return TPluginMetaDataPtr();
}

/*-------------------------------------------------------------------------*/

void 
CPluginGroup::SetGroupPriority( const UInt32 priority )
{GUCEF_TRACE;

    m_priority = priority;
}

/*-------------------------------------------------------------------------*/

UInt32 
CPluginGroup::GetGroupPriority( void ) const
{GUCEF_TRACE;

    return m_priority;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
