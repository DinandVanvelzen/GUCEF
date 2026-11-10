/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef GUCEF_CORE_CTABLECODECREGISTRY_H
#define GUCEF_CORE_CTABLECODECREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTREGISTRY_H
#include "CTRegistry.h"
#define GUCEF_CORE_CTREGISTRY_H
#endif /* GUCEF_CORE_CTREGISTRY_H ? */

#ifndef GUCEF_CORE_CITABLECODEC_H
#include "gucefCORE_CITableCodec.h"
#define GUCEF_CORE_CITABLECODEC_H
#endif /* GUCEF_CORE_CITABLECODEC_H ? */

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
 *  The table codecs that are available, by type name ("csv", "parquet"). One codec per type name.
 *  Owned by CCoreGlobal: the built in CSV codec is registered at start up, table codec plugins register theirs
 *  through CTableCodecPluginManager. Registries are changed with TryRegister() and TryUnregister().
 */
class GUCEF_CORE_PUBLIC_CPP CTableCodecRegistry : public CTRegistry< CITableCodec, MT::CMutex >
{
    public:

    typedef CTRegistry< CITableCodec, MT::CMutex >::TRegisteredObjPtr TTableCodecPtr;

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The codec that lists the extension, either with or without the leading dot and in any case: "csv", ".Parquet".
     *  When more than one codec lists it, the one with the lowest type name is returned.
     */
    bool TryGetCodecForFileExtension( const CString& extension, TTableCodecPtr& codec ) const;

    /** The built in CSV codec */
    bool TryGetCsvCodec( TTableCodecPtr& codec ) const;

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CCoreGlobal;

    CTableCodecRegistry( void );

    virtual ~CTableCodecRegistry() GUCEF_VIRTUAL_OVERRIDE;

    CTableCodecRegistry( const CTableCodecRegistry& src );              /**< not implemented */
    CTableCodecRegistry& operator=( const CTableCodecRegistry& src );   /**< not implemented */

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

#endif /* GUCEF_CORE_CTABLECODECREGISTRY_H ? */
