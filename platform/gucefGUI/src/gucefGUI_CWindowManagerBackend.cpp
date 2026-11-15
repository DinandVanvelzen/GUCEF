/*
 *  guceCORE: GUCE module providing tie-in functionality between systems
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_GUI_CVIDEOSETTINGS_H
#include "gucefGUI_CVideoSettings.h"
#define GUCEF_GUI_CVIDEOSETTINGS_H
#endif /* GUCEF_GUI_CVIDEOSETTINGS_H ? */

#include "gucefGUI_CWindowManagerBackend.h"      /* definition of the class implemented here */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CWindowManagerBackend::PrimaryWindowContextCreatedEvent = "GUCEF::GUI::CWindowManager::PrimaryWindowContextCreatedEvent";
const CORE::CEvent CWindowManagerBackend::PrimaryWindowContextDestroyedEvent = "GUCEF::GUI::CWindowManager::PrimaryWindowContextDestroyedEvent";
const CORE::CEvent CWindowManagerBackend::WindowContextCreatedEvent = "GUCEF::GUI::CWindowManager::WindowContextCreatedEvent";
const CORE::CEvent CWindowManagerBackend::WindowContextDestroyedEvent = "GUCEF::GUI::CWindowManager::WindowContextDestroyedEvent";

const CString CWindowManagerBackend::GraphicsApiParameterName = "GraphicsApi";
const CString CWindowManagerBackend::GraphicsApiGL = "GL";
const CString CWindowManagerBackend::GraphicsApiNone = "None";
         
/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CWindowManagerBackend::CWindowManagerBackend( void )
    : CObservingNotifier()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CWindowManagerBackend::~CWindowManagerBackend()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWindowManagerBackend::RegisterEvents( void )
{GUCEF_TRACE;

    PrimaryWindowContextCreatedEvent.Initialize();
    PrimaryWindowContextDestroyedEvent.Initialize();
    WindowContextCreatedEvent.Initialize();
    WindowContextDestroyedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/
        
bool
CWindowManagerBackend::IsAvailable( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

CString
CWindowManagerBackend::GetAvailabilityDescription( void ) const
{GUCEF_TRACE;

    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

CString
CWindowManagerBackend::GetRequestedGraphicsApi( const CORE::CValueList* params )
{GUCEF_TRACE;

    if ( GUCEF_NULL == params || !params->HasKey( GraphicsApiParameterName ) )
        return CString::Empty;
    CString graphicsApi = params->GetValueAlways( GraphicsApiParameterName ).AsString( CString::Empty, true ).Trim( true ).Trim( false );
    if ( graphicsApi.Equals( GraphicsApiGL, false ) )
        return GraphicsApiGL;
    if ( graphicsApi.Equals( GraphicsApiNone, false ) )
        return GraphicsApiNone;
    return graphicsApi;
}

/*-------------------------------------------------------------------------*/

bool
CWindowManagerBackend::IsGraphicsApiSupported( const CString& graphicsApi ) const
{GUCEF_TRACE;

    return GraphicsApiGL == graphicsApi;
}

/*-------------------------------------------------------------------------*/

bool
CWindowManagerBackend::IsRequestedGraphicsApiSupported( const CString& backendName      ,
                                                        const CORE::CValueList* params ) const
{GUCEF_TRACE;

    const CString graphicsApi = GetRequestedGraphicsApi( params );
    if ( graphicsApi.IsNULLOrEmpty() || IsGraphicsApiSupported( graphicsApi ) )
        return true;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WindowManagerBackend: The window backend \"" << backendName << "\" cannot make windows for the graphics API \""
        << graphicsApi << "\" (window parameter " << GraphicsApiParameterName << "), no window is made" );
    return false;
}

/*-------------------------------------------------------------------------*/

TWindowContextPtr
CWindowManagerBackend::CreateWindowContextViaConfig( const GUCEF::CORE::CDataNode& params ,
                                                     const CString& title      /* = "" */ )
{GUCEF_TRACE;
     
    /*
     *      Attempt to get all the info for the preferred window setup
     */                
    const CORE::CDataNode* n = params.Find( "WindowSettings" );
    if ( GUCEF_NULL != n )
    {
        const CORE::CDataNode::TKeyValuePair* att;
        UInt32 width( 640 ), height( 480 );
        bool fullscreen( false );                        
            
        att = n->GetAttribute( "width" );
        if ( att ) 
            width = att->second.AsUInt32( width );
        att = n->GetAttribute( "height" );        
        if ( att ) 
            height = att->second.AsUInt32( height );
        att = n->GetAttribute( "fullscreen" );
        if ( att ) 
            fullscreen = att->second.AsBool( fullscreen );
            
        const CORE::CDataNode::TKeyValuePair* att2;
        const CORE::CString name( "name" );
        const CORE::CString value( "value" );
        CORE::CValueList valuelist;
        const CORE::CDataNode* cn;
        CORE::CDataNode::const_iterator i = n->ConstBegin();
        while ( i != n->ConstEnd() )
        {
            cn = (*i);
            if ( cn->GetName() == "WindowSetting" )
            {
                att = cn->GetAttribute( name );
                if ( GUCEF_NULL != att )
                {
                    att2 = cn->GetAttribute( value );
                    if ( GUCEF_NULL != att2 )
                    {
                        valuelist[ att->second ] = att2->second;
                    }        
                }
            }
            ++i;
        }
        if ( title.Length() > 0 )
        {
            valuelist[ "title" ] = title;
        }                                        
                                    
        return CreateWindowContext( title         ,
                                    width         ,
                                    height        ,
                                    fullscreen    ,                                                       
                                    valuelist     );                                          
    }
    return TWindowContextPtr();          
}

/*-------------------------------------------------------------------------*/

bool
CWindowManagerBackend::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CWindowManagerBackend::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CWindowManagerBackend::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GUI::CWindowManagerBackend";
    return classTypeName;
}
       
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace CORE ? */
} /* namespace GUCE ? */

/*-------------------------------------------------------------------------*/
