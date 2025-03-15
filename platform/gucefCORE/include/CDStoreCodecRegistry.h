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

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#define GUCEF_CORE_CDSTORECODECREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDE                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CMT_CMUTEX_H
#include "gucefMT_CMutex.h"               /* gucefMT mutex implementation */
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTREGISTRY_H
#include "CTRegistry.h"         /* registry template */
#define GUCEF_CORE_CTREGISTRY_H
#endif /* GUCEF_CORE_CTREGISTRY_H ? */

#ifndef GUCEF_CORE_CDSTORECODEC_H
#include "CDStoreCodec.h"         /* base class for data storage codecs */
#define GUCEF_CORE_CDSTORECODEC_H
#endif /* GUCEF_CORE_CDSTORECODEC_H ? */

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

class GUCEF_CORE_PUBLIC_CPP CDStoreCodecRegistry : public CTRegistry< CDStoreCodec, MT::CMutex >
{
    public:

    typedef typename CTRegistry< CDStoreCodec, MT::CMutex >::TRegisteredObjPtr TDStoreCodecPtr;

    virtual const CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Convenience helper function for the commonly used YAML codec
     */
    bool TryGetYamlCodec( TDStoreCodecPtr& yamlCodec ) const;

    /**
     *  Convenience helper function for the commonly used JSON codec
     */
    bool TryGetJsonCodec( TDStoreCodecPtr& jsonCodec ) const;

    /**
     *  Convenience helper function for the commonly used XML codec
     */
    bool TryGetXmlCodec( TDStoreCodecPtr& xmlCodec ) const;

    protected:
    
    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CCoreGlobal;

    CDStoreCodecRegistry( void );

    virtual ~CDStoreCodecRegistry();

    private:
    
    CDStoreCodecRegistry( const CDStoreCodecRegistry& src );
    
    CDStoreCodecRegistry& operator=( const CDStoreCodecRegistry& src );

    private:
    
    MT::CMutex _datalock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */
