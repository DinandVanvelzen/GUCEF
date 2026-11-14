/*
 *  gucefWEB: GUCEF module providing Web application functionality 
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_WEB_CWebGlobal_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWebGlobal_H
#endif /* GUCEF_WEB_CWebGlobal_H ? */

#ifndef GUCEF_WEB_CGLOBALHTTPCODECLINKS_H
#include "gucefWEB_CGlobalHttpCodecLinks.h"
#define GUCEF_WEB_CGLOBALHTTPCODECLINKS_H
#endif /* GUCEF_WEB_CGLOBALHTTPCODECLINKS_H ? */

#include "gucefWEB_CCodecBasedHTTPServerResource.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCodecBasedHTTPServerResource::CCodecBasedHTTPServerResource( void )
    : CDefaultHTTPServerResource()
    , CORE::CTSGNotifier()
    , m_allowCreate( false )
    , m_allowDeserialize( false )
    , m_allowSerialize( false )
{GUCEF_TRACE;

    TEventCallback callback( this, &CCodecBasedHTTPServerResource::OnGlobalMimeCodecsChanged );
    SubscribeTo( &CWebGlobal::Instance()->GetGlobalHttpCodecLinks() ,
                 CGlobalHttpCodecLinks::MimeCodecsChangedEvent      ,
                 callback                                           );

    InitCodecLinks();
}

/*-------------------------------------------------------------------------*/
    
CCodecBasedHTTPServerResource::CCodecBasedHTTPServerResource( const CCodecBasedHTTPServerResource& src )
    : CDefaultHTTPServerResource( src )
    , CORE::CTSGNotifier( src )
    , m_allowCreate( src.m_allowCreate )
    , m_allowDeserialize( src.m_allowDeserialize )
    , m_allowSerialize( src.m_allowSerialize )
{GUCEF_TRACE;

    TEventCallback callback( this, &CCodecBasedHTTPServerResource::OnGlobalMimeCodecsChanged );
    SubscribeTo( &CWebGlobal::Instance()->GetGlobalHttpCodecLinks() ,
                 CGlobalHttpCodecLinks::MimeCodecsChangedEvent      ,
                 callback                                           );

    InitCodecLinks();
}

/*-------------------------------------------------------------------------*/
    
CCodecBasedHTTPServerResource::~CCodecBasedHTTPServerResource()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/
    
CCodecBasedHTTPServerResource&
CCodecBasedHTTPServerResource::operator=( const CCodecBasedHTTPServerResource& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_allowCreate = src.m_allowCreate;
        m_allowDeserialize = src.m_allowDeserialize;
        m_allowSerialize = src.m_allowSerialize;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CCodecBasedHTTPServerResource::InitCodecLinks( void )
{GUCEF_TRACE;

    CHttpCodecLinks& codecLinks = CWebGlobal::Instance()->GetGlobalHttpCodecLinks();
    codecLinks.GetSupportedDeserializationMimeTypes( m_deserializationReps );
    codecLinks.GetSupportedSerializationMimeTypes( m_serializationReps );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCodecBasedHTTPServerResource::OnGlobalMimeCodecsChanged( CORE::CNotifier* notifier    ,
                                                          const CORE::CEvent& eventId  ,
                                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    InitCodecLinks();
}

/*-------------------------------------------------------------------------*/

CCodecBasedHTTPServerResource::TCreateState 
CCodecBasedHTTPServerResource::CreateResource( const CString& resourcePath                   ,
                                               const CString& transactionID                  ,
                                               const CORE::CDynamicBuffer& inputBuffer       ,
                                               const CString& representation                 ,
                                               const CString& params                         ,
                                               THTTPServerResourcePtr& resourceOutput        ,
                                               TStringVector& supportedRepresentationsOutput )
{GUCEF_TRACE;

    if ( !m_allowCreate )
        return CIHTTPServerResource::CREATESTATE_NOTSUPPORTED;

    CHttpCodecLinks& codecLinks = CWebGlobal::Instance()->GetGlobalHttpCodecLinks();
    CHttpCodecLinks::TMimeTypeCodecPtr codec = codecLinks.GetDeserializationCodec( representation );
    if ( !codec.IsNULL() )
    {        
        CORE::CDataNode rootNode;
        CORE::CDynamicBufferAccess bufferIoAccess( inputBuffer );
        if ( codec->BuildDataTree( &rootNode, &bufferIoAccess ) )
        {
            return CreateResource( resourcePath                   ,
                                   transactionID                  , 
                                   rootNode                       , 
                                   representation                 , 
                                   params                         ,
                                   resourceOutput                 , 
                                   supportedRepresentationsOutput );
        }
    }
    return CIHTTPServerResource::CREATESTATE_DESERIALIZATIONFAILED;
}

/*-------------------------------------------------------------------------*/

CCodecBasedHTTPServerResource::TCreateState 
CCodecBasedHTTPServerResource::CreateResource( const CString& resourcePath                   ,
                                               const CString& transactionID                  ,
                                               const CORE::CDataNode& input                  ,
                                               const CString& representation                 ,
                                               const CString& params                         ,
                                               THTTPServerResourcePtr& resourceOutput        ,
                                               TStringVector& supportedRepresentationsOutput )
{GUCEF_TRACE;

    // By default we don't allow or implement more complex interaction patterns
    // such as dynamic resources or collections
    return CIHTTPServerResource::CREATESTATE_FAILED;
}

/*-------------------------------------------------------------------------*/

bool 
CCodecBasedHTTPServerResource::Serialize( const CString& resourcePath        ,
                                          CORE::CDynamicBuffer& outputBuffer ,
                                          const CString& representation      ,
                                          const CString& params              )
{GUCEF_TRACE;

    if ( !m_allowSerialize )
        return false;
    
    CHttpCodecLinks& codecLinks = CWebGlobal::Instance()->GetGlobalHttpCodecLinks();
    CHttpCodecLinks::TMimeTypeCodecPtr codec = codecLinks.GetSerializationCodec( representation );
    if ( !codec.IsNULL() )
    {        
        CORE::CDataNode rootNode;
        if ( Serialize( resourcePath, rootNode, representation, params ) )
        {        
            CORE::CDynamicBufferAccess bufferIoAccess( &outputBuffer, false );
            if ( codec->StoreDataTree( &rootNode, &bufferIoAccess ) )
            {
                return true;
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CCodecBasedHTTPServerResource::Serialize( const CString& resourcePath         ,
                                          CORE::CDataNode& output             ,
                                          const CORE::CString& representation ,
                                          const CString& params               )
{
    return false;
}

/*-------------------------------------------------------------------------*/

CCodecBasedHTTPServerResource::TDeserializeState 
CCodecBasedHTTPServerResource::Deserialize( const CString& resourcePath             ,
                                            const CORE::CDynamicBuffer& inputBuffer ,
                                            const CString& representation           ,
                                            bool isDeltaUpdateOnly                  )
{GUCEF_TRACE;

    if ( !m_allowDeserialize )
        return CIHTTPServerResource::DESERIALIZESTATE_UNABLETOUPDATE;

    CHttpCodecLinks& codecLinks = CWebGlobal::Instance()->GetGlobalHttpCodecLinks();
    CHttpCodecLinks::TMimeTypeCodecPtr codec = codecLinks.GetDeserializationCodec( representation );
    if ( !codec.IsNULL() )
    {        
        CORE::CDataNode rootNode;
        CORE::CDynamicBufferAccess bufferIoAccess( inputBuffer );
        if ( codec->BuildDataTree( &rootNode, &bufferIoAccess ) )
        {
            return Deserialize( resourcePath, rootNode, representation, isDeltaUpdateOnly );
        }
    }
    return CIHTTPServerResource::DESERIALIZESTATE_UNABLETOUPDATE;
}

/*-------------------------------------------------------------------------*/

CCodecBasedHTTPServerResource::TDeserializeState 
CCodecBasedHTTPServerResource::Deserialize( const CString& resourcePath   ,
                                            const CORE::CDataNode& input  ,
                                            const CString& representation ,
                                            bool isDeltaUpdateOnly        )
{
    return CIHTTPServerResource::DESERIALIZESTATE_UNABLETOUPDATE;
}

/*-------------------------------------------------------------------------*/

bool 
CCodecBasedHTTPServerResource::IsCreateSupported( void ) const
{GUCEF_TRACE;

    return m_allowCreate;
}

/*-------------------------------------------------------------------------*/

bool 
CCodecBasedHTTPServerResource::IsSerializeSupported( void ) const
{GUCEF_TRACE;

    return m_allowSerialize;
}

/*-------------------------------------------------------------------------*/

bool 
CCodecBasedHTTPServerResource::IsDeserializeSupported( bool deltaOnly ) const
{GUCEF_TRACE;

    return m_allowDeserialize;
}

/*-------------------------------------------------------------------------*/

void 
CCodecBasedHTTPServerResource::SetIsCreateSupported( bool isSupported )
{GUCEF_TRACE;

    m_allowCreate = isSupported;
}

/*-------------------------------------------------------------------------*/

void 
CCodecBasedHTTPServerResource::SetIsDeserializeSupported( bool isSupported, bool deltaOnly )
{GUCEF_TRACE;

    // we do not differentiate delta Deserialize by default
    m_allowDeserialize = isSupported;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
