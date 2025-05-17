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

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#define GUCEF_CORE_CCODECREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTONREGISTRY_H
#include "CTONRegistry.h"
#define GUCEF_CORE_CTONREGISTRY_H
#endif /* GUCEF_CORE_CTONREGISTRY_H ? */

#ifndef GUCEF_CORE_CICODEC_H
#include "CICodec.h"
#define GUCEF_CORE_CICODEC_H
#endif /* GUCEF_CORE_CICODEC_H ? */

#ifndef GUCEF_CORE_CORECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_CORECODECTYPES_H
#endif /* GUCEF_CORE_CORECODECTYPES_H ? */

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

class CCoreGlobal;

/*-------------------------------------------------------------------------*/

class GUCEF_CORE_PUBLIC_CPP CCodecRegistry : public CTONRegistry< CTONRegistry< CICodec, MT::CMutex >, MT::CMutex >
{
    public:

    typedef CTONRegistry< CICodec, MT::CMutex >                  TCodecFamilyRegistry;
    typedef CTBasicSharedPtr< TCodecFamilyRegistry, MT::CMutex > TCodecFamilyRegistryPtr;
    typedef CTSharedPtr< TCodecFamilyRegistry, MT::CMutex >      TCodecFamilyRegistryTypedPtr;
    typedef TCodecFamilyRegistry::TRegisteredObjPtr              TICodecPtr;
    typedef TCodecFamilyRegistry::TRegisteredObjTypedPtr         TICodecTypedPtr;
    typedef TCodecFamilyRegistry::TRegisteredObjPtrVector        TICodecPtrVector;
    
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    TICodecPtr TryGetCodec( const CString& codecFamily , 
                            const CString& codecName   ) const;

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CCoreGlobal;

    CCodecRegistry( void );

    virtual ~CCodecRegistry();

    private:
    
    CCodecRegistry( const CCodecRegistry& src );              /**< not implemented, don't use */
    CCodecRegistry& operator=( const CCodecRegistry& src );   /**< not implemented, don't use */

    private:
    MT::CMutex m_dataLock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */
