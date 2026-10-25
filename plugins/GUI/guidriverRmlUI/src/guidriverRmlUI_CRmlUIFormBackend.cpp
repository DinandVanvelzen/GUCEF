/*
 *  guidriverRmlUI: GUI backend using RmlUi
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUICONTEXT_H
#include "guidriverRmlUI_CRmlUIContext.h"
#define GUCEF_GUIDRIVERRMLUI_CRMLUICONTEXT_H
#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUICONTEXT_H ? */

#include "guidriverRmlUI_CRmlUIFormBackend.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverRmlUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRmlUIFormBackend::CRmlUIFormBackend( CRmlUIContext& context )
    : GUI::CFormBackend()
    , m_context( &context )
    , m_root( GUCEF_NULL )
    , m_document( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRmlUIFormBackend::~CRmlUIFormBackend()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

void
CRmlUIFormBackend::Release( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_root )
    {
        // destroying the root also lets the context unload the document
        m_context->DestroyWidget( m_root );
        m_root = GUCEF_NULL;
    }
    m_document = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIFormBackend::Adopt( Rml::ElementDocument* document )
{GUCEF_TRACE;

    if ( GUCEF_NULL == document )
        return false;

    CString name = document->GetId().empty() ? CString( "RmlDocument" ) : CString( document->GetId().c_str() );
    m_root = m_context->CreateWidget( "Panel", name, GUCEF_NULL );
    if ( GUCEF_NULL == m_root )
    {
        m_context->GetRmlContext()->UnloadDocument( document );
        return false;
    }
    m_root->SetProperty( GUI::EWidgetProperty_Size, "1,0 1,0" );
    m_document = document;
    m_context->RegisterNativeDocument( m_root, document );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIFormBackend::LoadLayout( const CString& layoutStoragePath )
{GUCEF_TRACE;

    Release();
    if ( GUCEF_NULL == m_context->GetRmlContext() )
        return false;

    // RmlUi reads the document and the files it refers to through the VFS
    Rml::ElementDocument* document = m_context->GetRmlContext()->LoadDocument( layoutStoragePath.C_String() );
    if ( GUCEF_NULL == document )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIFormBackend: RmlUi could not load the document " << layoutStoragePath );
        return false;
    }
    return Adopt( document );
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIFormBackend::LoadLayout( CORE::CIOAccess& layoutStorage )
{GUCEF_TRACE;

    Release();
    if ( GUCEF_NULL == m_context->GetRmlContext() )
        return false;

    Rml::String rml;
    char buffer[ 4096 ];
    UInt32 read = 0;
    while ( 0 < ( read = layoutStorage.Read( buffer, 1, sizeof( buffer ) ) ) )
        rml.append( buffer, read );

    Rml::ElementDocument* document = m_context->GetRmlContext()->LoadDocumentFromMemory( rml );
    if ( GUCEF_NULL == document )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIFormBackend: RmlUi could not load the document from the stream" );
        return false;
    }
    return Adopt( document );
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIFormBackend::SaveLayout( const CString& layoutStoragePath )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIFormBackend::SaveLayout( CORE::CIOAccess& layoutStorage )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

GUI::CWidget*
CRmlUIFormBackend::GetRootWidget( void )
{GUCEF_TRACE;

    return m_root;
}

/*-------------------------------------------------------------------------*/

const GUI::CWidget*
CRmlUIFormBackend::GetRootWidget( void ) const
{GUCEF_TRACE;

    return m_root;
}

/*-------------------------------------------------------------------------*/

GUI::CWidget*
CRmlUIFormBackend::GetWidget( const CString& widgetName )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_root && m_root->GetName() == widgetName )
        return m_root;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CRmlUIFormBackend::GetWidgetVector( const CString& widgetName   ,
                                    TWidgetVector& widgetVector )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( widgetName );
    if ( GUCEF_NULL != widget )
        widgetVector.push_back( widget );
}

/*-------------------------------------------------------------------------*/

Rml::ElementDocument*
CRmlUIFormBackend::GetDocument( void ) const
{GUCEF_TRACE;

    return m_document;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverRmlUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
