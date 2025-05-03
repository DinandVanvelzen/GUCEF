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

#ifndef GUCEF_CORE_CDATANODE_H 
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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
    , m_autoInstantiateShareableCodecs( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataDrivenDStoreCodecFactory::~CDataDrivenDStoreCodecFactory()
{GUCEF_TRACE;
    
    UnregisterAllConcreteFactories();
    UnregisterAll();
}

/*-------------------------------------------------------------------------*/

CDataDrivenDStoreCodecPtr
CDataDrivenDStoreCodecFactory::CreateCodec( const CString& dataDrivenCodecTypeName ,
                                            bool caseSensitive                     )
{GUCEF_TRACE;

    // First we need to check if we have meta-data for a codec with the given name in our registry
    CDataDrivenDStoreCodecMetaPtr codecMeta;
    if ( !TryLookup( dataDrivenCodecTypeName, codecMeta, caseSensitive ) || codecMeta.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DataDrivenDStoreCodecFactory:CreateCodec: No meta-data found for codec type: " + dataDrivenCodecTypeName );
        return CDataDrivenDStoreCodecPtr(); // no meta-data found to produce codec
    }

    // Now we can create the codec using the meta-data
    // Note that we use the base codec type for the factory to create the codec since the derived codec comes out of 
    // the factory as a product based on the data its given. That is the whole concept of a data-driven codec
    TProductPtr newCodec = Create( codecMeta->GetBaseCodecTypeName(), codecMeta );
    if ( newCodec.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DataDrivenDStoreCodecFactory::CreateCodec: Failed to create codec of type \"" + 
            dataDrivenCodecTypeName + "\" using base codec with type name \"" + codecMeta->GetBaseCodecTypeName() + "\"" );
        return CDataDrivenDStoreCodecPtr(); // failed to create codec even though we had the meta-data for it
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

/*-------------------------------------------------------------------------*/

bool
CDataDrivenDStoreCodecFactory::SaveConfig( CDataNode& cfg ) const
{GUCEF_TRACE;

    return false; // not implemented yet
}

/*-------------------------------------------------------------------------*/

void 
CDataDrivenDStoreCodecFactory::SetAutoInstantiateShareableCodecs( bool autoInstantiateShareableCodecs )
{GUCEF_TRACE;

    m_autoInstantiateShareableCodecs = autoInstantiateShareableCodecs;
}

/*-------------------------------------------------------------------------*/

bool 
CDataDrivenDStoreCodecFactory::GetAutoInstantiateShareableCodecs( void ) const
{GUCEF_TRACE;

    return m_autoInstantiateShareableCodecs;
}

/*-------------------------------------------------------------------------*/

bool
CDataDrivenDStoreCodecFactory::LoadConfig( const CDataNode& cfg )
{GUCEF_TRACE;

    m_autoInstantiateShareableCodecs = cfg.GetAttributeValueOrChildValueByName( "autoInstantiateShareableCodecs", m_autoInstantiateShareableCodecs ).AsBool( m_autoInstantiateShareableCodecs );
    
    const CDataNode* allCodecMetaCfg = cfg.Find( "codecMeta" );
    if ( GUCEF_NULL != allCodecMetaCfg )
    {
        CStringSet newSharableCodecsToInit;
        
        // Iterate over all codec meta-data and register them
        CDataNode::const_iterator it = allCodecMetaCfg->ConstBegin();            
        while ( it != allCodecMetaCfg->ConstEnd() )
        {
            const CDataNode* codecMetaCfg = (*it);
            if ( GUCEF_NULL != codecMetaCfg && codecMetaCfg->GetNodeType() == GUCEF_DATATYPE_OBJECT )
            {
                CDataDrivenDStoreCodecMetaPtr codecMeta = CDataDrivenDStoreCodecMeta::CreateSharedObj();
                if ( !codecMeta.IsNULL() )
                {
                    if ( codecMeta->LoadConfig( *codecMetaCfg ) )
                    {
                        // Register the codec meta-data with the registry
                        // This allows us to create codecs of this type later on
                        if ( TryRegister( codecMeta->GetDataDrivenCodecTypeName(), codecMeta ) )
                        {
                            if ( codecMeta->IsShareable() && m_autoInstantiateShareableCodecs )
                            {
                                newSharableCodecsToInit.insert( codecMeta->GetDataDrivenCodecTypeName() );
                            }
                        }
                    }
                    else
                    {
                        // we dont know if we were able to parse a name for the log statement, best effort
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DataDrivenDStoreCodecFactory:LoadConfig: Failed to load config for data driven codec meta-data. Possibly for codec type: \"" + codecMeta->GetDataDrivenCodecTypeName() + "\"" );
                    }
                }
            }
            ++it;
        }

        // Now instantiate codecs as applicable
        CStringSet::iterator i = newSharableCodecsToInit.begin();
        while ( i != newSharableCodecsToInit.end() )
        {
            const CString& codecTypeName = (*i);
            CDataDrivenDStoreCodecPtr codec = CreateCodec( codecTypeName, true );
            if ( !codec.IsNULL() )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "DataDrivenDStoreCodecFactory:LoadConfig: Successfully instantiated shareable codec of type: \"" + codecTypeName + "\"" );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DataDrivenDStoreCodecFactory:LoadConfig: Failed to instantiate shareable codec of type: \"" + codecTypeName + "\"" );
            }
            ++i;
        }
    }

    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
