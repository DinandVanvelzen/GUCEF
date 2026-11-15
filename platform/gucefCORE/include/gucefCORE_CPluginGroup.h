/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CPLUGINGROUP_H
#define GUCEF_CORE_CPLUGINGROUP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/


#ifndef GUCEF_CORE_CPLUGINMETADATA_H
#include "gucefCORE_CPluginMetaData.h"
#define GUCEF_CORE_CPLUGINMETADATA_H
#endif /* GUCEF_CORE_CPLUGINMETADATA_H ? */

#ifndef GUCEF_CORE_CIPLUGIN_H
#include "CIPlugin.h"
#define GUCEF_CORE_CIPLUGIN_H
#endif /* GUCEF_CORE_CIPLUGIN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Logical grouping class for plugins which allows plugins to be managed as groups
 *  This is just a data container class, PluginControl houses all the logic
 *
 *  The plugins of a group are kept in their load order: by their priority in the group, lowest first, and plugins with
 *  the same priority in the order they were added. The plugin control loads and registers them in that order and
 *  unregisters and unloads them in the reverse order. A plugin keeps its place when it moves from meta data to a
 *  registered plugin and back.
 */
class GUCEF_CORE_PRIVATE_CPP CPluginGroup : public CTSharedObjCreator< CPluginGroup, MT::CMutex >  
{
    public:

    typedef GUCEF::vector< TPluginPtr > TPluginList;
    typedef GUCEF::vector< TPluginMetaDataStoragePtr > TPluginMetaDataList;
    typedef GUCEF::vector< CString > TStringVector;
    typedef typename CTSharedObjCreator< CPluginGroup, MT::CMutex >::TSharedPtrType TPluginGroupPtr;
    
    CPluginGroup( void );

    CPluginGroup( const CPluginGroup& src );

    virtual ~CPluginGroup();

    CPluginGroup& operator=( const CPluginGroup& src );

    /** The registered plugins, in load order */
    TPluginList GetPlugins( void ) const;

    bool HasPlugins( void ) const;

    /** The meta data of the plugins that are not registered, in load order */
    TPluginMetaDataList GetPluginMetaData( void ) const;

    /** The module file names of all plugins of the group, registered or not, in load order */
    void GetModuleFilenamesInLoadOrder( TStringVector& moduleFilenames ) const;

    /**
     *  Adds the meta data of a plugin with its priority in the group. 0 places it after everything the group has.
     *  Meta data that is in the group already keeps its place. Returns the priority it has
     */
    UInt32 AddPluginMetaData( const TPluginMetaDataStoragePtr& metaData, UInt32 priorityInGroup = 0 );

    bool RemovePluginMetaData( const TPluginMetaDataStoragePtr& metaData );

    void ClearPluginMetaData( void );

    /** The meta data was registered as the plugin: the plugin takes its place in the load order */
    bool ReplacePluginMetaDataWithPlugin( const TPluginMetaDataStoragePtr& metaData, const TPluginPtr& plugin );

    /** The plugin was unregistered: the meta data takes its place in the load order */
    bool ReplacePluginWithPluginMetaData( const TPluginPtr& plugin, const TPluginMetaDataStoragePtr& metaData );

    bool RemovePlugin( const TPluginPtr& plugin );

    /** The highest priority of the plugins of the group, 0 when it has none */
    UInt32 GetHighestPriorityInGroup( void ) const;

    TPluginPtr FindPluginWithModuleName( const CString& moduleName ) const;

    TPluginMetaDataPtr FindPluginMetaDataWithModuleName( const CString& moduleName ) const;

    bool HasPluginWithFileName( const CString& moduleFileName ) const;
    
    void SetGroupPriority( const UInt32 priority );

    UInt32 GetGroupPriority( void ) const;

    private:

    /** A plugin of the group, registered (plugin) or not (metaData), with its place in the load order */
    struct SEntry
    {
        TPluginPtr plugin;
        TPluginMetaDataStoragePtr metaData;
        UInt32 priorityInGroup;
        UInt64 sequence;
    };
    typedef GUCEF::vector< SEntry > TEntryList;

    static void InsertInLoadOrder( TEntryList& entries, const SEntry& entry );
    static bool IsBefore( const SEntry& a, const SEntry& b );

    TEntryList m_plugins;
    TEntryList m_pluginMetaData;
    UInt64 m_nextSequence;
    UInt32 m_priority;
};

/*-------------------------------------------------------------------------*/

typedef CPluginGroup::TPluginGroupPtr TPluginGroupPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CPLUGINGROUP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 27-11-2004 :
        - Dinand: Initial implementation

---------------------------------------------------------------------------*/
