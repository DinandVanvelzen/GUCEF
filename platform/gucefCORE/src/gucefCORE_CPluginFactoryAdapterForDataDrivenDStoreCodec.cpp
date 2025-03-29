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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#include "gucefCORE_CUriResourceAccessorFactory.h"
#define GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#endif /* GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H ? */

#ifndef GUCEF_CORE_CTVARIANTVECTORAPI_H
#include "gucefCORE_CTVariantMapAdapter_Cpp2C.h"
#define GUCEF_CORE_CTVARIANTVECTORAPI_H
#endif /* GUCEF_CORE_CTVARIANTVECTORAPI_H ? */

#include "gucefCORE_CPluginFactoryAdapterForDataDrivenDStoreCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataDrivenDStoreCodecPluginLink::CDataDrivenDStoreCodecPluginLink( CDataDrivenDStoreCodecMetaPtr codecMeta ,
                                                                    CDStoreCodecPlugin* codecPlugin         ,
                                                                    void* ddCodecData                       )
    : CDataDrivenDStoreCodec( codecMeta )
    , m_codecPlugin( codecPlugin )
    , m_ddCodecData( ddCodecData )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataDrivenDStoreCodecPluginLink::~CDataDrivenDStoreCodecPluginLink( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CICloneable* 
CDataDrivenDStoreCodecPluginLink::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool 
CDataDrivenDStoreCodecPluginLink::StoreDataTree( const CDataNode* tree, const CString& filename )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_codecPlugin )
    {
        return m_codecPlugin->StoreDataTree( tree, filename );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CDataDrivenDStoreCodecPluginLink::StoreDataTree( const CDataNode* tree, CIOAccess* file )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_codecPlugin )
    {
        return m_codecPlugin->StoreDataTree( tree, file );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CDataDrivenDStoreCodecPluginLink::BuildDataTree( CDataNode* treeroot, const CString& filename )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_codecPlugin )
    {
        return m_codecPlugin->BuildDataTree( treeroot, filename );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CDataDrivenDStoreCodecPluginLink::BuildDataTree( CDataNode* treeroot, CIOAccess* file )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_codecPlugin )
    {
        return m_codecPlugin->BuildDataTree( treeroot, file );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CString 
CDataDrivenDStoreCodecPluginLink::GetName( void ) const
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_codecPlugin )
    {
        return m_codecPlugin->GetName();
    }
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

CString 
CDataDrivenDStoreCodecPluginLink::GetTypeName( void ) const
{GUCEF_TRACE;

    if ( !m_codecMeta.IsNULL() )
    {
        return m_codecMeta->GetDataDrivenCodecTypeName();
    }
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

CString 
CDataDrivenDStoreCodecPluginLink::GetCopyright( void ) const
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_codecPlugin )
    {
        return m_codecPlugin->GetCopyright();
    }
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

TVersion 
CDataDrivenDStoreCodecPluginLink::GetVersion( void ) const
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_codecPlugin )
    {
        return m_codecPlugin->GetVersion();
    }
    return TVersion();
}

/*-------------------------------------------------------------------------*/

CPluginFactoryAdapterForDataDrivenDStoreCodec::CPluginFactoryAdapterForDataDrivenDStoreCodec( CDStoreCodecPlugin* codecPlugin )
    : CTFactoryBaseWithParam< CDataDrivenDStoreCodec, CDataDrivenDStoreCodecMetaPtr, MT::CMutex >()
    , m_codecPlugin( codecPlugin )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPluginFactoryAdapterForDataDrivenDStoreCodec::~CPluginFactoryAdapterForDataDrivenDStoreCodec()
{GUCEF_TRACE;

    m_codecPlugin = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CPluginFactoryAdapterForDataDrivenDStoreCodec::Clone( void ) const
{GUCEF_TRACE;

    // We do not support cloning of this factory due to potentially dangerous pointer references
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CPluginFactoryAdapterForDataDrivenDStoreCodec::TProductPtr
CPluginFactoryAdapterForDataDrivenDStoreCodec::Create( const CDataDrivenDStoreCodecMetaPtr& param )
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( GUCEF_NULL == m_codecPlugin || param.IsNULL() )
    {
        // preconditions not met
        return TProductPtr();
    }
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == m_codecPlugin->m_api.Create_Data_Driven_Codec )
    {
        // preconditions not met
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "PluginFactoryAdapterForDataDrivenDStoreCodec:Create: Codec plugin does not support data driven codec creation" );
        return TProductPtr();
    }

    TDataDrivenDStoreCodecMeta metaParamCAdapter;
    if ( !param->LinkCAdapter( &metaParamCAdapter ) )
    {
        // linking of C adapter failed
        return TProductPtr();
    }

    // In the case of a plugin we have to accomodate the constraints of plugin housed factories
    // They wont have access to all the same facilities and will need to operate through a C API
    // Hence we need to preload the data needed and provide access to the plugin through the C API

    CTVariantMapAdapterCpp2C< CString, CDynamicBuffer > resourceMapAdapter;
    CTVariantMapAdapterCpp2C< CString, CDynamicBuffer >::TLinkedMapType resourcesLoaded;
    resourceMapAdapter.SetMapAccessPtr( &resourcesLoaded );

    const CStringMap& resourcesNeeded = param->GetDataMap();
    CStringMap::const_iterator i = resourcesNeeded.begin();
    while ( i != resourcesNeeded.end() )
    {
        const CString& resourceName = i->first;
        const CString& resourceValue = i->second;

        bool resourceLoadSuccess = false;

        CUri resourceUri;
        if ( resourceUri.ParseUriInStringForm( resourceValue ) )
        {
            CUriResourceAccessorPtr uriAccessor = CCoreGlobal::Instance()->GetUriResourceAccessorFactory().CreateAccessor( resourceUri );
            if ( !uriAccessor.IsNULL() )
            {
                CDynamicBuffer& targetBuffer = resourcesLoaded[ resourceName ];
                
                if ( uriAccessor->GetResource( resourceUri, targetBuffer ) )
                {
                    // We have the resource in a buffer now, we can pass it to the plugin
                    resourceLoadSuccess = true;
                }
                else
                {
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginFactoryAdapterForDataDrivenDStoreCodec:Create: Failed to load resource into ram buffer for " + resourceUri );
                }
            }
            else
            {
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginFactoryAdapterForDataDrivenDStoreCodec:Create: Failed to parse Uri accessor for " + resourceUri );
            }
        }
        else
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginFactoryAdapterForDataDrivenDStoreCodec:Create: Failed to parse Uri string: " + resourceValue );
        }

        if ( !resourceLoadSuccess )
        {
            // Its all or nothing
            GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "PluginFactoryAdapterForDataDrivenDStoreCodec:Create: Failed to load resource \"" + 
                resourceName + "\" for base codec \"" + param->GetBaseCodecTypeName() + "\" and data codec \"" + param->GetDataDrivenCodecTypeName() + "\"");
            return TProductPtr();
        }

        ++i;
    }
    
    void* dataDrivenCodecPrivateData = GUCEF_NULL;
    UInt32 resultCode = m_codecPlugin->m_api.Create_Data_Driven_Codec( m_codecPlugin->m_plugdata            ,
                                                                       &metaParamCAdapter                   ,
                                                                       resourceMapAdapter.GetCStyleAccess() ,
                                                                       &dataDrivenCodecPrivateData          );

    if GUCEF_PREDICT_FALSE( 0 != resultCode )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "PluginFactoryAdapterForDataDrivenDStoreCodec:Create: Failed to create data driven codec \"" + 
            param->GetDataDrivenCodecTypeName() + "\" using base codec \"" + param->GetBaseCodecTypeName() + "\"");

        return TProductPtr();
    }

    CDataDrivenDStoreCodecPluginLinkPtr ddCodec( GUCEF_NEW CDataDrivenDStoreCodecPluginLink( param, m_codecPlugin, dataDrivenCodecPrivateData ) );

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "PluginFactoryAdapterForDataDrivenDStoreCodec:Create: Successfully created data driven codec \"" + 
        param->GetDataDrivenCodecTypeName() + "\" using base codec \"" + param->GetBaseCodecTypeName() + "\"");

    return ddCodec;
}

/*-------------------------------------------------------------------------*/

CString
CPluginFactoryAdapterForDataDrivenDStoreCodec::GetConcreteClassTypeName( void ) const
{GUCEF_TRACE;

    return "PluginFactoryAdapterForDataDrivenDStoreCodec";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
