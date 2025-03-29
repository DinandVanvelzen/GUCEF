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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefCORE_CDataDrivenDStoreCodecFactory.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CDataDrivenDStoreCodecFactory::ClassTypeName = "GUCEF::CORE::CDataDrivenDStoreCodecFactory";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataDrivenDStoreCodecFactory::CDataDrivenDStoreCodecFactory( void )
    : CTAbstractFactoryWithParam< CString, CDataDrivenDStoreCodec, CDataDrivenDStoreCodecMetaPtr, MT::CMutex >()
    , CTONRegistry< CDataDrivenDStoreCodecMeta, MT::CMutex >()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataDrivenDStoreCodecFactory::~CDataDrivenDStoreCodecFactory()
{GUCEF_TRACE;
    
    UnregisterAllConcreteFactories();
    UnregisterAll();
}

/*-------------------------------------------------------------------------*/

CDataDrivenDStoreCodecFactory::TProductPtr
CDataDrivenDStoreCodecFactory::CreateCodec( const CString& dataDrivenCodecTypeName ,
                                            bool caseSensitive                     )
{GUCEF_TRACE;

    // First we need to check if we have meta-data for a codec with the given name in our registry
    CDataDrivenDStoreCodecMetaPtr codecMeta;
    if ( !TryLookup( dataDrivenCodecTypeName, codecMeta, caseSensitive ) || codecMeta.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DataDrivenDStoreCodecFactory:CreateCodec: No meta-data found for codec type: " + dataDrivenCodecTypeName );
        return TProductPtr(); // no meta-data found to produce codec
    }

    // Now we can create the codec using the meta-data
    TProductPtr newCodec = Create( codecMeta->GetBaseCodecTypeName(), codecMeta );
    if ( newCodec.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DataDrivenDStoreCodecFactory::CreateCodec: Failed to create codec of type \"" + 
            dataDrivenCodecTypeName + "\" using base codec with type name \"" + codecMeta->GetBaseCodecTypeName() + "\"" );
        return TProductPtr(); // failed to create codec even though we had the meta-data for it
    }
    
    // Initialize the codec with the meta-data
    if ( newCodec->GetDataDrivenDStoreCodecMeta().IsNULL() )
        newCodec->SetDataDrivenDStoreCodecMeta( codecMeta );

    return newCodec;
}

/*-------------------------------------------------------------------------*/

const CString& 
CDataDrivenDStoreCodecFactory::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

const MT::CILockable* 
CDataDrivenDStoreCodecFactory::AsLockable( void ) const
{GUCEF_TRACE;

    // we will use the base factory lock as THE lock to remove ambiguity
    return CTAbstractFactoryWithParam< CString, CDataDrivenDStoreCodec, CDataDrivenDStoreCodecMetaPtr, MT::CMutex >::AsLockable();
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CDataDrivenDStoreCodecFactory::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;
    
    // we will use the base factory lock as THE lock to remove ambiguity
    return CTAbstractFactoryWithParam< CString, CDataDrivenDStoreCodec, CDataDrivenDStoreCodecMetaPtr, MT::CMutex >::Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CDataDrivenDStoreCodecFactory::Unlock( void ) const
{GUCEF_TRACE;

    // we will use the base factory lock as THE lock to remove ambiguity
    return CTAbstractFactoryWithParam< CString, CDataDrivenDStoreCodec, CDataDrivenDStoreCodecMetaPtr, MT::CMutex >::Unlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
