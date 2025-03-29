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

#ifndef GUCEF_CORE_CDSTORECODECPLUGIN_H
#define GUCEF_CORE_CDSTORECODECPLUGIN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_CIPLUGIN_H
#include "CIPlugin.h"
#define GUCEF_CORE_CIPLUGIN_H
#endif /* GUCEF_CORE_CIPLUGIN_H ? */

#ifndef GUCEF_CORE_CDSTORECODEC_H
#include "CDStoreCodec.h"
#define GUCEF_CORE_CDSTORECODEC_H
#endif /* GUCEF_CORE_CDSTORECODEC_H ? */

#ifndef GUCEF_CORE_CDATADRIVENDSTORECODEC_H
#include "gucefCORE_CDataDrivenDStoreCodec.h"
#define GUCEF_CORE_CDATADRIVENDSTORECODEC_H
#endif /* GUCEF_CORE_CDATADRIVENDSTORECODEC_H ? */

#ifndef GUCEF_CORE_CPLUGINMETADATA_H
#include "gucefCORE_CPluginMetaData.h"
#define GUCEF_CORE_CPLUGINMETADATA_H
#endif /* GUCEF_CORE_CPLUGINMETADATA_H ? */

#ifndef GUCEF_CORE_C_DSTORE_PLUGIN_API_H
#include "gucefCORE_c_dstore_plugin_api.h"
#define GUCEF_CORE_C_DSTORE_PLUGIN_API_H
#endif /* GUCEF_CORE_C_DSTORE_PLUGIN_API_H ? */

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

class CDStoreCodecPluginManager;
class CDataDrivenDStoreCodecPluginLink;
class CPluginFactoryAdapterForDataDrivenDStoreCodec;

/*-------------------------------------------------------------------------*/

/**
 *  Internal class for linking a DStoreCodec via a C plugin API
 * 
 *  Note that the DStore plugin related classes of this type are all tightly coupled as they need to efficiently 
 *  work together to provide the necessary functionality through a restrictive C interface
 */
class GUCEF_HIDDEN CDStoreCodecPlugin : public CDStoreCodec ,
                                        public CIPlugin
{
    public:

    CDStoreCodecPlugin( void );
    
    virtual ~CDStoreCodecPlugin() GUCEF_VIRTUAL_OVERRIDE;

    bool Link( void* modulePtr                   ,
               TPluginMetaDataPtr pluginMetaData );

    bool Unlink( void );

    virtual TPluginMetaDataPtr GetMetaData( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void* GetModulePointer( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Attempts to store the given tree in the file
     *      given according to the method of the codec metadata
     *
     *      @param tree the data tree you wish to store
     *      @param filename path and name of the file you wish to store the data in.
     *      @return wheter storing the tree was successfull
     */
   virtual bool StoreDataTree( const CDataNode* tree   ,
                               const CString& filename ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Attempts to store the given tree in the file
     *      given according to the method of the codec metadata
     *
     *      @param tree the data tree you wish to store
     *      @param file access to the recource to which the data will be written
     *      @return wheter storing the tree was successfull
     */
    virtual bool StoreDataTree( const CDataNode* tree   ,
                                CIOAccess* file         ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Attempts to load data from the given file to the
     *      root node given. The root data will be replaced
     *      and any children the node may already have will be deleted.
     *
     *      @param treeroot pointer to the node that is to act as root of the data tree
     *      @param filename path and name of the file from which the data tree information should be loaded
     *      @return whether building the tree from the given file was successfull.
     */
    virtual bool BuildDataTree( CDataNode* treeroot     ,
                                const CString& filename ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Attempts to load data from the given file to the
     *      root node given. The root data will be replaced
     *      and any children the node may already have will be deleted.
     *
     *      @param treeroot pointer to the node that is to act as root of the data tree
     *      @param file media independant access to the data source from which the data tree information should be loaded
     *      @return whether building the tree from the given file was successfull.
     */
    virtual bool BuildDataTree( CDataNode* treeroot ,
                                CIOAccess* file     ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Returns the codec type
     *      a dat file codec might return "dat" and an xml codec "xml" ect.
     *
     *      @return the codec type
     */
    virtual CString GetTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Returns whether this codec type is data driven
     *  Data driven codecs require a data map or schema to be able to perform their transformations
     */
    virtual bool IsCodecTypeDataDriven( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Returns a factory for creating data driven codecs if applicable. See IsCodecTypeDataDriven()
     */
    CPluginFactoryAdapterForDataDrivenDStoreCodec* GetDataDrivenCodecFactory( void ) const;

    /**
     *      Returns the name of the codec
     *      The author of the codec can give an codec a name that the user can
     *      obtain using this member function.
     *
     *      @return name of the codec
     */
    virtual CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetDescription( void ) const;

    /**
     *      Returns the copyright of the codec
     *      The author of the codec can give an codec a copyright notice that the user can
     *      obtain using this member function.
     *
     *      @return copyright notice of the codec
     */
    virtual CString GetCopyright( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Returns the codec version
     *
     *      @return the version of the codec
     */
    virtual TVersion GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsLoaded( void ) const;
    
    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CDataDrivenDStoreCodecPluginLink;

    bool StoreDataTree( void** codecData, const CDataNode* tree, const CString& filename );
    bool StoreDataTree( void** codecData, const CDataNode* tree, CIOAccess* file );
    bool BuildDataTree( void** codecData, CDataNode* treeroot, const CString& filename );
    bool BuildDataTree( void** codecData, CDataNode* treeroot, CIOAccess* file );

    private:
    
    CDStoreCodecPlugin( const CDStoreCodecPlugin& src ); /**< don't copy plugin objects */
    CDStoreCodecPlugin& operator=( const CDStoreCodecPlugin& src ); /**< don't copy plugin objects */

    void StoreNode( void** codecData   ,
                    const CDataNode* n ,
                    void** filedata    ); /**< recursive node storage algorithm */

    private:
    friend class CPluginFactoryAdapterForDataDrivenDStoreCodec;
    
    UInt32 _id;                       /**< codec id, typicly used by manager classes */
    TGucefCoreCDStorePluginApi m_api; /**< function pointers to plugin API */
    void* m_sohandle;                 /**< access to the so module */
    void* m_codecData;                /**< codec data pointer to be used by the plugin as needed */
    void* m_plugdata;                 /**< storage pointer to be used by the plugin as needed */
    TPluginMetaDataStoragePtr m_metaData;
    CPluginFactoryAdapterForDataDrivenDStoreCodec* m_ddCodecFactory; /**< factory for data driven codecs */   
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CDStoreCodecPlugin, MT::CMutex > TDStoreCodecPluginPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDSTORECODECPLUGIN_H ? */
