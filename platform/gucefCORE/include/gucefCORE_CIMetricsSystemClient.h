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

#ifndef GUCEF_CORE_CIMETRICSSYSTEMCLIENT_H
#define GUCEF_CORE_CIMETRICSSYSTEMCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"       /* module macro's */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CINAMEDINSTANCE_H
#include "CINamedInstance.h"
#define GUCEF_CORE_CINAMEDINSTANCE_H
#endif /* GUCEF_CORE_CINAMEDINSTANCE_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
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

class GUCEF_CORE_PUBLIC_CPP CIMetricsSystemClient : public CIConfigurable  ,
                                                    public CINamedInstance
{
    public:

    CIMetricsSystemClient( void );

    CIMetricsSystemClient( const CIMetricsSystemClient& src );

    virtual ~CIMetricsSystemClient();

    CIMetricsSystemClient& operator=( const CIMetricsSystemClient& src );

    virtual bool Open( void ) = 0;

    virtual bool Close( void ) = 0;

    /**
     *  Increments the key, at a given frequency rate
     */
    virtual void Increment( const CString& key, const Float32 frequency = 1.0f ) const = 0;

    /**
     *  Increments the key, at a given frequency rate
     */
    virtual void Decrement( const CString& key, const Float32 frequency = 1.0f ) const = 0;

    /**
     *   Adjusts the specified key by a given delta, at a given frequency rate
     */
    virtual void Count( const CString& key, const Int32 delta, const Float32 frequency = 1.0f ) const = 0;
    virtual void Count( const CString& key, const Int64 delta, const Float32 frequency = 1.0f ) const = 0;
    virtual void Count( const CString& key, const UInt32 delta, const Float32 frequency = 1.0f ) const = 0;
    virtual void Count( const CString& key, const UInt64 delta, const Float32 frequency = 1.0f ) const = 0;
    virtual void Count( const CString& key, const CVariant& delta, const Float32 frequency = 1.0f ) const = 0;

    /**
     *  Records a gauge for the key, with a given value, at a given frequency rate
     */
    virtual void Gauge( const CString& key, const Int32 value, const Float32 frequency = 1.0f ) const = 0;
    virtual void Gauge( const CString& key, const UInt32 value, const Float32 frequency = 1.0f ) const = 0;
    virtual void Gauge( const CString& key, const Int64 value, const Float32 frequency = 1.0f ) const = 0;
    virtual void Gauge( const CString& key, const UInt64 value, const Float32 frequency = 1.0f ) const = 0;
    virtual void Gauge( const CString& key, const Float32 value, const Float32 frequency = 1.0f ) const = 0;
    virtual void Gauge( const CString& key, const Float64 value, const Float32 frequency = 1.0f ) const = 0;

    /**
     *  Records a timing for a key, at a given frequency
     */
    virtual void Timing( const CString& key, const UInt32 ms, const Float32 frequency = 1.0f ) const = 0;
    virtual void Timing( const CString& key, const UInt64 ms, const Float32 frequency = 1.0f ) const = 0;
    virtual void Timing( const CString& key, const Float32 value, const Float32 frequency = 1.0f ) const = 0;
    virtual void Timing( const CString& key, const Float64 value, const Float32 frequency = 1.0f ) const = 0;

};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CIMETRICSSYSTEMCLIENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 16-02-2007 :
        - Dinand: Added this class

---------------------------------------------------------------------------*/
