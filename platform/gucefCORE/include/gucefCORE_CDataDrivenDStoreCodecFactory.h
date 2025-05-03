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

#ifndef GUCEF_CORE_CDATADRIVENDSTORECODECFACTORY_H
#define GUCEF_CORE_CDATADRIVENDSTORECODECFACTORY_H

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

#ifndef GUCEF_CORE_CTONREGISTRY_H
#include "CTONRegistry.h"         
#define GUCEF_CORE_CTONREGISTRY_H
#endif /* GUCEF_CORE_CTONREGISTRY_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CDSTORECODEC_H
#include "CDStoreCodec.h"         /* base class for data storage codecs */
#define GUCEF_CORE_CDSTORECODEC_H
#endif /* GUCEF_CORE_CDSTORECODEC_H ? */

#ifndef GUCEF_CORE_CDATADRIVENDSTORECODECMETA_H
#include "gucefCORE_CDataDrivenDStoreCodecMeta.h"
#define GUCEF_CORE_CDATADRIVENDSTORECODECMETA_H
#endif /* GUCEF_CORE_CDATADRIVENDSTORECODECMETA_H ? */

#ifndef GUCEF_CORE_CDATADRIVENDSTORECODEC_H
#include "gucefCORE_CDataDrivenDStoreCodec.h"
#define GUCEF_CORE_CDATADRIVENDSTORECODEC_H
#endif /* GUCEF_CORE_CDATADRIVENDSTORECODEC_H ? */

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
 *  Factory class for data driven storage codecs
 * 
 *  This style of codecs needs data in order to be constructed and be of use such as a schema or a data map
 *  In order to allow the data to be processed and thus evaluated for suitability for codec construction this
 *  functionality is provided via an abstract factory pattern such that externalized implementations can register 
 *  a factory for a specific codec type and provide the necessary logic for codec construction for that codec type
 *  
 *  This class acts as both a factory for creating data driven storage codecs as well as a registry for the codec meta data
 *  This is because one cannot function or be of use without the other.
 */
class GUCEF_CORE_PUBLIC_CPP CDataDrivenDStoreCodecFactory : public CTAbstractFactoryWithParam< CString, CDataDrivenDStoreCodec, CDataDrivenDStoreCodecMetaPtr, MT::CMutex > ,
                                                            public CTONRegistry< CDataDrivenDStoreCodecMeta, MT::CMutex > , 
                                                            public CIConfigurable
{
    public:

    static const CString ClassTypeName;

    typedef typename CTAbstractFactoryWithParam< CString, CDataDrivenDStoreCodec, CDataDrivenDStoreCodecMetaPtr, MT::CMutex >::TProductPtr TProductPtr;

    CDataDrivenDStoreCodecFactory( void );

    virtual ~CDataDrivenDStoreCodecFactory( void );

    /**
     *  Utility specialization which makes utilizing the factory easier by combining the registry lookup and codec creation
     */
    CDataDrivenDStoreCodecPtr CreateCodec( const CString& dataDrivenCodecTypeName ,
                                           bool caseSensitive = true              );

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Attempts to store the given tree in the file
     *  given according to the method of the codec metadata
     *
     *  @param cfg the data tree you wish to store the config
     *  @return wheter storing all the config information to the provided tree was successfull
     */
    virtual bool SaveConfig( CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Attempts to load data from the given file to the
     *  root node given. The root data will be replaced
     *  and any children the node may already have will be deleted.
     *
     *  @param cfg node that is to act as root of the config data tree
     *  @return whether loading required/mandatory settings from the given config was successfull
     */
    virtual bool LoadConfig( const CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    void SetAutoInstantiateShareableCodecs( bool autoInstantiateShareableCodecs );

    bool GetAutoInstantiateShareableCodecs( void ) const;
    
    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDataDrivenDStoreCodecFactory( const CDataDrivenDStoreCodecFactory& src );              /**< not implemented, not supported >*/
    CDataDrivenDStoreCodecFactory& operator=( const CDataDrivenDStoreCodecFactory& src );   /**< not implemented, not supported >*/

    bool m_autoInstantiateShareableCodecs;

};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATADRIVENDSTORECODECFACTORY_H ? */
