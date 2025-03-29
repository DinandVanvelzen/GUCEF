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

#ifndef GUCEF_CORE_CDATADRIVENDSTORECODECMETA_H
#define GUCEF_CORE_CDATADRIVENDSTORECODECMETA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"    
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"       
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"        
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_C_DATADRIVENDSTORECODECMETA_H
#include "gucefCORE_c_datadriven_dstorecodec_meta.h"
#define GUCEF_CORE_C_DATADRIVENDSTORECODECMETA_H
#endif /* GUCEF_CORE_C_DATADRIVENDSTORECODECMETA_H ? */

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
 *  Meta-data class containing the information that allows constructing a 
 *  data driven data storage codec by combining the base codec type name with resources
 *  and a data driven codec type name under which this logical codec will be registered.
 * 
 *  Some examples of data driven data storage codecs are:
 *      - protobuf schema driven data storage codecs for specific data types
 *      - kaitai schema driven data storage codecs for specific data types
 */
class GUCEF_CORE_PUBLIC_CPP CDataDrivenDStoreCodecMeta : public CIConfigurable ,
                                                         public CTSharedObjCreator< CDataDrivenDStoreCodecMeta, MT::CMutex >
{
    public:

    static const CString ClassTypeName; /**< The class type name used for runtime type identification */

    typedef typename CTSharedObjCreator< CDataDrivenDStoreCodecMeta, MT::CMutex >::TSharedPtrType CDataDrivenDStoreCodecMetaPtr;

    CDataDrivenDStoreCodecMeta( void );

    CDataDrivenDStoreCodecMeta( const CDataDrivenDStoreCodecMeta& src );          

    virtual ~CDataDrivenDStoreCodecMeta() GUCEF_VIRTUAL_OVERRIDE;

    CDataDrivenDStoreCodecMeta& operator=( const CDataDrivenDStoreCodecMeta& src );  

    void SetBaseCodecTypeName( const CString& codecTypeName );

    const CString& GetBaseCodecTypeName( void ) const;

    void SetDataDrivenCodecTypeName( const CString& codecTypeName );

    const CString& GetDataDrivenCodecTypeName( void ) const;

    void SetDataMap( const CStringMap& resources );

    const CStringMap& GetDataMap( void ) const;

    CStringMap& GetDataMap( void );

    void SetParams( const CStringMap& params );

    const CStringMap& GetParams( void ) const;

    CStringMap& GetParams( void );

    /**
     *  Sets whether the codec is shareable
     *  This means the codec is not tied to volatile resources or state in time and as such can be made globally available
     */
    void SetIsShareable( bool isSharable );
    
    /**
     *  Returns whether the codec is shareable
     *  This means the codec is not tied to volatile resources or state in time and as such can be made globally available
     */
    bool IsShareable( void ) const;

    /**
     *  This links to the contained meta-data from the C structure allowing to be passed through a C API
     *  Note that you MUST ensure that the C adapter is valid for the lifetime of this object which is achieved by 
     *  keeping the instance of this class that is linked to immutable and alive for the duration of the C API call
     *  Use with care!
     */
    bool LinkCAdapter( TDataDrivenDStoreCodecMeta* cAdapter ) const;

    virtual bool SaveConfig( CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;


    private:

    CString m_baseCodecTypeName;
    CString m_dataDrivenCodecTypeName;
    CStringMap m_resources;
    CStringMap m_params;
    bool m_isShareable;
};

/*-------------------------------------------------------------------------*/

typedef CDataDrivenDStoreCodecMeta::CDataDrivenDStoreCodecMetaPtr CDataDrivenDStoreCodecMetaPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATADRIVENDSTORECODECMETA_H ? */
