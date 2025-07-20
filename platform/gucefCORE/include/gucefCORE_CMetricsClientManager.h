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

#ifndef GUCEF_CORE_CMETRICSCLIENTMANAGER_H
#define GUCEF_CORE_CMETRICSCLIENTMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CREADWRITELOCK_H
#include "gucefMT_CReadWriteLock.h"
#define GUCEF_MT_CREADWRITELOCK_H
#endif /* GUCEF_MT_CREADWRITELOCK_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CIMETRICSSYSTEMCLIENT_H
#include "gucefCORE_CIMetricsSystemClient.h"
#define GUCEF_CORE_CIMETRICSSYSTEMCLIENT_H
#endif /* GUCEF_CORE_CIMETRICSSYSTEMCLIENT_H ? */

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


class GUCEF_CORE_PUBLIC_CPP CMetricsClientManager : public CIMetricsSystemClient
{
    public:

    typedef CTSharedPtr< CIMetricsSystemClient, MT::CMutex > CIMetricsSystemClientPtr;

    void AddMetricsClient( const CString& name, CIMetricsSystemClientPtr client );

    void RemoveMetricsClient( CIMetricsSystemClientPtr client );

    void RemoveMetricsClient( const CString& name );

    void ClearMetricsClients( void );

    virtual bool Open( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Increments the key, at a given frequency rate
     */
    virtual void Increment( const CString& key, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Increments the key, at a given frequency rate
     */
    virtual void Decrement( const CString& key, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *   Adjusts the specified key by a given delta, at a given frequency rate
     */
    virtual void Count( const CString& key, const Int32 delta, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Count( const CString& key, const Int64 delta, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Count( const CString& key, const UInt32 delta, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Count( const CString& key, const UInt64 delta, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Count( const CString& key, const CVariant& delta, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Records a gauge for the key, with a given value, at a given frequency rate
     */
    virtual void Gauge( const CString& key, const Int32 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Gauge( const CString& key, const UInt32 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Gauge( const CString& key, const Int64 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Gauge( const CString& key, const UInt64 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Gauge( const CString& key, const Float32 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Gauge( const CString& key, const Float64 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Records a timing for a key, at a given frequency
     */
    virtual void Timing( const CString& key, const UInt32 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Timing( const CString& key, const UInt64 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Timing( const CString& key, const Float32 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Timing( const CString& key, const Float64 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CCoreGlobal;

    CMetricsClientManager( void );

    virtual ~CMetricsClientManager();

    private:

    CMetricsClientManager( const CMetricsClientManager& src );              /**< not implemented, don't use */
    CMetricsClientManager& operator=( const CMetricsClientManager& src );   /**< not implemented, don't use */

    private:
    typedef std::map< CString, CIMetricsSystemClientPtr > TMetricsSystemClientPtrMap;

    TMetricsSystemClientPtrMap m_clients;
    MT::CReadWriteLock m_rwDataLock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CMETRICSCLIENTMANAGER_H ? */
