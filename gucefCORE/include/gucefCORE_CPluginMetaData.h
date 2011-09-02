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

#ifndef GUCEF_CORE_CPLUGINMETADATA_H
#define GUCEF_CORE_CPLUGINMETADATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIPLUGINMETADATA_H
#include "gucefCORE_CIPluginMetaData.h"
#define GUCEF_CORE_CIPLUGINMETADATA_H
#endif /* GUCEF_CORE_CIPLUGINMETADATA_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

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
 *  Default storage implementation for plugin metadata
 *  Relies on external source providing the actual information.
 */
class GUCEF_CORE_PUBLIC_CPP CPluginMetaData : public CIPluginMetaData ,
                                              public CIConfigurable
{
    public:

    CPluginMetaData( void );

    explicit CPluginMetaData( const CPluginMetaData& src );

    explicit CPluginMetaData( const CIPluginMetaData& src );

    virtual ~CPluginMetaData();

    CPluginMetaData& operator=( const CPluginMetaData& src );

    void SetDescription( const CString& description );

    /**
     *  Optional textual description of the plugin functionality
     */    
    virtual CString GetDescription( void ) const;
    
    void SetCopyright( const CString& description );
    
    /**
     *  Optional textual copyright information regarding the plugin
     */ 
    virtual CString GetCopyright( void ) const;

    void SetVersion( const TVersion& version );

    /**
     *  Version info for the plugin
     */
    virtual TVersion GetVersion( void ) const;

    void SetLoaderLogicTypeName( const CString& loaderLogicTypeName );
    
    /**
     *  Optional info on which which loader logic should be used when loading the plugin
     */
    virtual CString GetLoaderLogicTypeName( void ) const;

    void SetPluginType( const CString& pluginType );
    
    /**
     *  Info regarding the type of plugin this is. 
     *  PluginManagers can be added to the system to support the loading of different types
     *  of plugins. Defining the plugin type allows these managers to pick the plugins they 
     *  can manage
     */
    virtual CString GetPluginType( void ) const;

    void SetModuleFilename( const CString& moduleFilename );
    
    virtual CString GetModuleFilename( void ) const;

    void SetFullModulePath( const CString& description );
    
    /**
     *  Optional info regarding the full path of the module.
     *  If none is specified globally defined root paths can be used in combination 
     *  with the module filename to find the module
     */
    virtual CString GetFullModulePath( void ) const;

    /**
     *  Optionally parameters can be specified to be available for the plugin.
     *  How, when and if they values are passed to the plugin depends on the implementation
     *  of the plugin and plugin manager. This is merely a placeholder.
     */
    virtual void GetParams( CValueList& params ) const;

    /**
     *  Optionally parameters can be specified to be available for the plugin.
     *  How, when and if they values are passed to the plugin depends on the implementation
     *  of the plugin and plugin manager. This is merely a placeholder.
     */
    void SetParams( const CValueList& params );

    /**
     *  Attempts to store the given tree in the file
     *  given according to the method of the codec metadata
     *
     *  @param tree the data tree you wish to store
     *  @return wheter storing the tree was successfull
     */
    virtual bool SaveConfig( CDataNode& tree );

    /**
     *  Attempts to load data from the given file to the
     *  root node given. The root data will be replaced
     *  and any children the node may already have will be deleted.
     *
     *  @param treeroot pointer to the node that is to act as root of the data tree
     *  @return whether building the tree from the given file was successfull.
     */
    virtual bool LoadConfig( const CDataNode& treeroot );

    private:

    CString m_description;
    CString m_copyright;
    TVersion m_version;
    CString m_loaderLogicTypeName;
    CString m_pluginType;
    CString m_moduleFilename;
    CString m_modulePath;
    CValueList m_params;
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CPluginMetaData > TPluginMetaDataStoragePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CIPLUGINMETADATA_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 27-11-2004 :
        - Dinand: Initial implementation

---------------------------------------------------------------------------*/
