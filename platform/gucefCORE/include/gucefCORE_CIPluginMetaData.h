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

#ifndef GUCEF_CORE_CIPLUGINMETADATA_H
#define GUCEF_CORE_CIPLUGINMETADATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"         /* often used gucef data structs */
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

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
 *  Interface class for plugin metadata
 */
class GUCEF_CORE_PUBLIC_CPP CIPluginMetaData
{
    public:

    CIPluginMetaData( void );

    CIPluginMetaData( const CIPluginMetaData& src );

    virtual ~CIPluginMetaData();

    CIPluginMetaData& operator=( const CIPluginMetaData& src );

    /**
     *  Optional textual description of the plugin functionality
     */    
    virtual CString GetDescription( void ) const = 0;

    /**
     *  Optional textual copyright information regarding the plugin
     */ 
    virtual CString GetCopyright( void ) const = 0;

    /**
     *  Version info for the plugin
     */
    virtual TVersion GetVersion( void ) const = 0;

    /**
     *  Optional info on which which loader logic should be used when loading the plugin
     */
    virtual CString GetLoaderLogicTypeName( void ) const = 0;

    /**
     *  Info regarding the type of plugin this is. 
     *  PluginManagers can be added to the system to support the loading of different types
     *  of plugins. Defining the plugin type allows these managers to pick the plugins they 
     *  can manage
     */
    virtual CString GetPluginType( void ) const = 0;

    virtual CString GetModuleFilename( void ) const = 0;

    /**
     *  Optional alternate filename
     */
    virtual CString GetAltModuleFilename( void ) const = 0;

    /**
     *  Optional info regarding the full path of the module.
     *  If none is specified globally defined root paths can be used in combination 
     *  with the module filename to find the module
     */
    virtual CString GetFullModulePath( void ) const = 0;

    /**
     *  Optionally parameters can be specified to be available for the plugin.
     *  How, when and if they values are passed to the plugin depends on the implementation
     *  of the plugin and plugin manager. This is merely a placeholder.
     */
    virtual void GetParams( CValueList& params ) const = 0;

    /**
     *  Resets all data to default creation state
     */
    virtual void Clear( void ) = 0;

    /**
     *  Returns the address of the module that houses the plugin if available
     *  Return shall be GUCEF_NULL if not available
     */
    virtual void* GetModulePointer( void ) const = 0;

    /**
     *  flag whether the plugin should be loaded immediately or if it should be loaded
     *  only when later requested to be loaded and thus treated as merely a placeholder
     */
    virtual bool GetLoadImmediately( void ) const = 0;

    /**
     *  flag whether the plugin is optional and loading it should be considered 'best effort'
     *  thus failure to load the plugin would not be considered an error and merely an informational event
     */
    virtual bool GetLoadFailAllowed( void ) const = 0;

    /**
     *  Optional comma-separated list of LinkBack module names this plugin depends on.
     *  A LinkBack module is any framework module (e.g. gucefPUBSUB) whose global singleton
     *  calls CPluginControl::ListModuleForLinkBack() on construction and
     *  CPluginControl::UnlistModuleForLinkBack() on destruction.
     *  When a listed module calls UnlistModuleForLinkBack(), CPluginControl will immediately
     *  unload all generic (C++ link-back) plugins that declared it here, preventing use-after-free
     *  crashes at process exit when plugin callbacks outlive the modules they link into.
     *  Example: "gucefMT,gucefCORE,gucefPUBSUB"
     *  No spaces should be used in the list.
     */
    virtual CString GetAllLinkBackModules( void ) const = 0;
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CIPluginMetaData, MT::CMutex > TPluginMetaDataPtr;
typedef CTSharedPtr< const CIPluginMetaData, MT::CMutex > TConstPluginMetaDataPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CIPLUGINMETADATA_H ? */
