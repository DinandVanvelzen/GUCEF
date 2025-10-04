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

#include <set>

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
 */
class GUCEF_CORE_PRIVATE_CPP CPluginGroup : public CTSharedObjCreator< CPluginGroup, MT::CMutex >  
{
    public:

    typedef GUCEF::set< TPluginPtr > TPluginSet;
    typedef GUCEF::set< TPluginMetaDataStoragePtr > TPluginMetaDataSet;
    typedef typename CTSharedObjCreator< CPluginGroup, MT::CMutex >::TSharedPtrType TPluginGroupPtr;
    
    CPluginGroup( void );

    CPluginGroup( const CPluginGroup& src );

    virtual ~CPluginGroup();

    CPluginGroup& operator=( const CPluginGroup& src );

    TPluginSet& GetPlugins( void );

    const TPluginSet& GetPlugins( void ) const;

    TPluginPtr FindPluginWithModuleName( const CString& moduleName ) const;

    TPluginMetaDataPtr FindPluginMetaDataWithModuleName( const CString& moduleName ) const;

    TPluginMetaDataSet& GetPluginMetaData( void );

    const TPluginMetaDataSet& GetPluginMetaData( void ) const;

    bool HasPluginWithFileName( const CString& moduleFileName ) const;
    
    void SetGroupPriority( const UInt32 priority );

    UInt32 GetGroupPriority( void ) const;

    private:

    TPluginSet m_plugins;
    TPluginMetaDataSet m_pluginMetaData;
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
