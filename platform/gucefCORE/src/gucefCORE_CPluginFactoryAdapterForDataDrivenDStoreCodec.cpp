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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

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

CDataDrivenDStoreCodecPluginLink::CDataDrivenDStoreCodecPluginLink( CDataDrivenDStoreCodecMetaPtr codecMeta                 ,
                                                                    CDStoreCodecPlugin* codecPlugin                         ,
                                                                    CPluginFactoryAdapterForDataDrivenDStoreCodec* factory  ,
                                                                    void* ddCodecData                                       )
    : CDataDrivenDStoreCodec( codecMeta )
    , m_codecPlugin( codecPlugin )
    , m_factory( factory )
    , m_ddCodecData( ddCodecData )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataDrivenDStoreCodecPluginLink::~CDataDrivenDStoreCodecPluginLink( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_factory )
        m_factory->ReleaseLink( *this );
}

/*-------------------------------------------------------------------------*/

void
CDataDrivenDStoreCodecPluginLink::DestroyPluginData( void )
{GUCEF_TRACE;

    // The plugin made the data of this codec when it created it, it destroys that data as well
    if ( GUCEF_NULL != m_codecPlugin && GUCEF_NULL != m_ddCodecData && GUCEF_NULL != m_codecPlugin->m_api.Destroy_Data_Driven_Codec )
    {
        TDataDrivenDStoreCodecMeta metaCAdapter;
        TDataDrivenDStoreCodecMeta* meta = GUCEF_NULL;
        CDataDrivenDStoreCodecMetaPtr codecMeta = GetDataDrivenDStoreCodecMeta();
        if ( !codecMeta.IsNULL() && codecMeta->LinkCAdapter( &metaCAdapter ) )
            meta = &metaCAdapter;

        if ( 0 != m_codecPlugin->m_api.Destroy_Data_Driven_Codec( m_codecPlugin->m_plugdata, meta, &m_ddCodecData ) )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataDrivenDStoreCodecPluginLink: The plugin failed to destroy the data of codec \"" <<
                ( codecMeta.IsNULL() ? CString::Empty : codecMeta->GetDataDrivenCodecTypeName() ) << "\"" );
        }
    }
    m_ddCodecData = GUCEF_NULL;
    m_codecPlugin = GUCEF_NULL;
    m_factory = GUCEF_NULL;
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
        return m_codecPlugin->StoreDataTree( &m_ddCodecData, tree, filename );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CDataDrivenDStoreCodecPluginLink::StoreDataTree( const CDataNode* tree, CIOAccess* file )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_codecPlugin )
    {
        return m_codecPlugin->StoreDataTree( &m_ddCodecData, tree, file );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CDataDrivenDStoreCodecPluginLink::BuildDataTree( CDataNode* treeroot, const CString& filename )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_codecPlugin )
    {
        return m_codecPlugin->BuildDataTree( &m_ddCodecData, treeroot, filename );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CDataDrivenDStoreCodecPluginLink::BuildDataTree( CDataNode* treeroot, CIOAccess* file )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_codecPlugin )
    {
        return m_codecPlugin->BuildDataTree( &m_ddCodecData, treeroot, file );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CDocumentStreamWriterPtr
CDataDrivenDStoreCodecPluginLink::CreateDocumentWriter( CIOAccess& output )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_codecPlugin )
    {
        return m_codecPlugin->CreateDocumentWriter( &m_ddCodecData, output );
    }
    return CDocumentStreamWriterPtr();
}

/*-------------------------------------------------------------------------*/

bool
CDataDrivenDStoreCodecPluginLink::ParseDocument( CIOAccess& input, CIDocumentParserCallbacks& callbacks )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_codecPlugin )
    {
        return m_codecPlugin->ParseDocument( &m_ddCodecData, input, callbacks );
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
    , m_links()
    , m_linksLock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPluginFactoryAdapterForDataDrivenDStoreCodec::~CPluginFactoryAdapterForDataDrivenDStoreCodec()
{GUCEF_TRACE;

    DetachAllLinks();
    m_codecPlugin = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CPluginFactoryAdapterForDataDrivenDStoreCodec::DetachAllLinks( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_linksLock );
    TLinkSet::iterator i = m_links.begin();
    while ( i != m_links.end() )
    {
        (*i)->DestroyPluginData();
        ++i;
    }
    m_links.clear();
}

/*-------------------------------------------------------------------------*/

void
CPluginFactoryAdapterForDataDrivenDStoreCodec::ReleaseLink( CDataDrivenDStoreCodecPluginLink& link )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_linksLock );
    TLinkSet::iterator i = m_links.find( &link );
    if ( i == m_links.end() )
        return;     // detached already
    m_links.erase( i );
    link.DestroyPluginData();
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
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginFactoryAdapterForDataDrivenDStoreCodec:Create: Failed to load resource into ram buffer for " << CRedactor::ForUrl().RedactUrl( resourceUri ) );
                }
            }
            else
            {
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginFactoryAdapterForDataDrivenDStoreCodec:Create: Failed to parse Uri accessor for " << CRedactor::ForUrl().RedactUrl( resourceUri ) );
            }
        }
        else
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PluginFactoryAdapterForDataDrivenDStoreCodec:Create: Failed to parse Uri string: " << CRedactor::ForUrl().RedactUrl( resourceValue ) );
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

    CDataDrivenDStoreCodecPluginLink* link = GUCEF_NEW CDataDrivenDStoreCodecPluginLink( param, m_codecPlugin, this, dataDrivenCodecPrivateData );
    {
        MT::CScopeMutex lock( m_linksLock );
        m_links.insert( link );
    }
    CDataDrivenDStoreCodecPluginLinkPtr ddCodec( link );

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
