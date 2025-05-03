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

#ifndef GUCEF_CORE_CDATADRIVENDSTORECODEC_H
#define GUCEF_CORE_CDATADRIVENDSTORECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDSTORECODEC_H
#include "CDStoreCodec.h"       
#define GUCEF_CORE_CDSTORECODEC_H
#endif /* GUCEF_CORE_CDSTORECODEC_H ? */

#ifndef GUCEF_CORE_CDATADRIVENDSTORECODECMETA_H
#include "gucefCORE_CDataDrivenDStoreCodecMeta.h"
#define GUCEF_CORE_CDATADRIVENDSTORECODECMETA_H
#endif /* GUCEF_CORE_CDATADRIVENDSTORECODECMETA_H ? */

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
 *  Base class for data driven data storage codecs
 *  It simple adds context that its data driven plus access to the relevant meta data
 */
class GUCEF_CORE_PUBLIC_CPP CDataDrivenDStoreCodec : public CDStoreCodec
{
    public:

    CDataDrivenDStoreCodec( void );

    CDataDrivenDStoreCodec( CDataDrivenDStoreCodecMetaPtr codecMeta );

    virtual ~CDataDrivenDStoreCodec() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SetDataDrivenDStoreCodecMeta( CDataDrivenDStoreCodecMetaPtr newMetaData );

    virtual CDataDrivenDStoreCodecMetaPtr GetDataDrivenDStoreCodecMeta( void ) const;

    virtual bool IsCodecTypeDataDriven( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Returns the codec type
     *  a dat file codec might return "dat" and an xml codec "xml" ect.
     *
     *  @return the codec type
     */
    virtual CORE::CString GetTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    CDataDrivenDStoreCodecMetaPtr m_codecMeta;
    
    private:
    CDataDrivenDStoreCodec( const CDataDrivenDStoreCodec& src );             /**< no sense in an copied codec */
    CDataDrivenDStoreCodec& operator=( const CDataDrivenDStoreCodec& src );  /**< no sense in an copied codec */
};

/*-------------------------------------------------------------------------*/

typedef CTBasicSharedPtr< CDataDrivenDStoreCodec, MT::CMutex > CDataDrivenDStoreCodecPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATADRIVENDSTORECODEC_H ? */
