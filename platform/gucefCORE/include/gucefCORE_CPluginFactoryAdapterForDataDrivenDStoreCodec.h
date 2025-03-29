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
                        
#ifndef GUCEF_CORE_CPLUGINFACTORYADAPTERFORDATADRIVENDSTORECODEC_H
#define GUCEF_CORE_CPLUGINFACTORYADAPTERFORDATADRIVENDSTORECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"    
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORYWITHPARAM_H
#include "CTAbstractFactoryWithParam.h"
#define GUCEF_CORE_CTABSTRACTFACTORYWITHPARAM_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORYWITHPARAM_H ? */

#ifndef GUCEF_CORE_CDATADRIVENDSTORECODECMETA_H
#include "gucefCORE_CDataDrivenDStoreCodecMeta.h"
#define GUCEF_CORE_CDATADRIVENDSTORECODECMETA_H
#endif /* GUCEF_CORE_CDATADRIVENDSTORECODECMETA_H ? */

#ifndef GUCEF_CORE_CDATADRIVENDSTORECODEC_H
#include "gucefCORE_CDataDrivenDStoreCodec.h"
#define GUCEF_CORE_CDATADRIVENDSTORECODEC_H
#endif /* GUCEF_CORE_CDATADRIVENDSTORECODEC_H ? */

#ifndef GUCEF_CORE_CDSTORECODECPLUGIN_H
#include "CDStoreCodecPlugin.h"
#define GUCEF_CORE_CDSTORECODECPLUGIN_H
#endif /* GUCEF_CORE_CDSTORECODECPLUGIN_H ? */

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

/**
 *  Internal implementation of a link to a data driven datanode codec implemented inside a plugin
 *  Each data set that results in a data driven codec will have a codec link
 *  This allows us to use the codec in a generic way beyond creation and destruction, without having to know the specifics of the codec
 * 
 *  Note that the DStore plugin related classes of this type are all tightly coupled as they need to efficiently 
 *  work together to provide the necessary functionality through a restrictive C interface
 */
class GUCEF_HIDDEN CDataDrivenDStoreCodecPluginLink : public CDataDrivenDStoreCodec
{
    public:

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool StoreDataTree( const CDataNode* tree, const CString& filename ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool StoreDataTree( const CDataNode* tree, CIOAccess* file ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool BuildDataTree( CDataNode* treeroot, const CString& filename ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool BuildDataTree( CDataNode* treeroot, CIOAccess* file ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CString GetTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CString GetCopyright( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual TVersion GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CDataDrivenDStoreCodecPluginLink( CDataDrivenDStoreCodecMetaPtr codecMeta ,
                                      CDStoreCodecPlugin* codecPlugin         ,
                                      void* ddCodecData                       );
    
    virtual ~CDataDrivenDStoreCodecPluginLink( void ) GUCEF_VIRTUAL_OVERRIDE;

    private: 

    CDataDrivenDStoreCodecPluginLink( const CDataDrivenDStoreCodecPluginLink& src );             /**< not implemented, not supported >*/
    CDataDrivenDStoreCodecPluginLink& operator=(const CDataDrivenDStoreCodecPluginLink& src );   /**< not implemented, not supported >*/

    private:

    CDStoreCodecPlugin* m_codecPlugin; /**< overall plugin adapter */
    void* m_ddCodecData;  /**< data we need to hold onto for the C API for this data driven codec */
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CDataDrivenDStoreCodecPluginLink, MT::CMutex > CDataDrivenDStoreCodecPluginLinkPtr;

/*-------------------------------------------------------------------------*/

/**
 *  Internal implementation of concrete factory class for data driven storage codecs specifically for plugins
 *  
 *  In the case of a plugin we have to accomodate the constraints of plugin housed factories
 *  They wont have access to all the same facilities and will need to operate through a C API
 * 
 *  Note that the DStore plugin related classes of this type are all tightly coupled as they need to efficiently 
 *  work together to provide the necessary functionality through a restrictive C interface
 */
class GUCEF_HIDDEN CPluginFactoryAdapterForDataDrivenDStoreCodec : public CTFactoryBaseWithParam< CDataDrivenDStoreCodec, CDataDrivenDStoreCodecMetaPtr, MT::CMutex >
{
    public:

    typedef CTFactoryBaseWithParam< CDataDrivenDStoreCodec, CDataDrivenDStoreCodecMetaPtr, MT::CMutex >     TFactoryBase;
    typedef typename TFactoryBase::TProductPtr                                                              TProductPtr;

    CPluginFactoryAdapterForDataDrivenDStoreCodec( CDStoreCodecPlugin* codecPlugin );
    
    virtual ~CPluginFactoryAdapterForDataDrivenDStoreCodec( void ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual TProductPtr Create( const CDataDrivenDStoreCodecMetaPtr& param  ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetConcreteClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    private: 

    CPluginFactoryAdapterForDataDrivenDStoreCodec( const CPluginFactoryAdapterForDataDrivenDStoreCodec& src );             /**< not implemented, not supported >*/
    CPluginFactoryAdapterForDataDrivenDStoreCodec& operator=(const CPluginFactoryAdapterForDataDrivenDStoreCodec& src );   /**< not implemented, not supported >*/

    private:

    CDStoreCodecPlugin* m_codecPlugin; /**< overall plugin adapter */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CPLUGINFACTORYADAPTERFORDATADRIVENDSTORECODEC_H ? */
